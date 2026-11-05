#!/usr/bin/env python3
"""
vNAAATS server - replaces the old api.vnaaats.net backend. No third-party packages needed (Python 3.8+).

Endpoints used by the plugin (same names as the old backend):
  GET /GetTrackSource                      -> "0"
  GET /GetAllNatTracks[?event=true]        -> NAT tracks (converted from https://nattrak.vatsim.net/api/tracks)
  GET /GetSingleFlightData?callsign=XXX    -> [ {flight data} ]   (404 if unknown)
  GET /PostFlightData?code=KEY&callsign=.. -> store a flight           (needs the API key)
  GET /UpdateFlightData?code=KEY&callsign=..-> update a flight         (needs the API key)
  GET /PluginScratch?code=KEY&callsign=..&text=.. -> scratchpad note typed in EuroScope (needs the API key)
Extra:
  GET /GetAllFlightData[?sort=0|1]         -> every stored flight (flight data display)
  GET /GetNetworkTraffic                   -> oceanic-bound flights found in the public VATSIM data feed (not from the plugin)
  GET /GetNatTrakData                      -> natTrak clearances/requests (from https://nattrak.vatsim.net/api/plugins)
  GET /PostNamedFix?code=KEY&name=..&lat=..&lon=..  -> add a fix coordinates are missing for (admin page)
  If a "web" folder exists next to this file, the built flight data display is served from it.
  Controller login for the flight data display (editing): POST /auth/password, GET /auth/login + /auth/callback (VATSIM Connect),
  GET /auth/status, POST /auth/position, POST /auth/logout, POST /EditFlight (needs a login with a position).
  CPDLC (Hoppie ACARS ground station, see cpdlc.py): GET /cpdlc/status, GET /cpdlc/messages?callsign=XXX&after=N,
  POST /cpdlc/send {callsign,text,ra,mrn}, POST /cpdlc/seen {callsign}. Flight records carry "cpdlc": true when connected.
  GET /health      GET /unresolved (fixes we could not find coordinates for)   GET /  (simple info page)

Settings (environment variables): VNAAATS_API_KEY (required), VNAAATS_PORT (8080), VNAAATS_DB, VNAAATS_NATTRAK_URL,
VNAAATS_REFRESH_SECONDS (600), VNAAATS_BIND (127.0.0.1), VNAAATS_VATSIM_FEED (feed url; set to 'off' to disable),
VNAAATS_FEED_SECONDS (60), VNAAATS_NATTRAK_PLUGIN_URL (natTrak clearance API; 'off' to disable).
Login (editing from the web): VNAAATS_FDD_PASSWORD (shared password), VNAAATS_POSITIONS (comma list shown on the position screen),
VNAAATS_CONNECT_CLIENT_ID / VNAAATS_CONNECT_CLIENT_SECRET / VNAAATS_PUBLIC_URL (e.g. https://fdd.example.com) for VATSIM Connect,
VNAAATS_CPDLC=off (CPDLC is on by default; each controller types their own Hoppie code on the FDD), VNAAATS_CPDLC_STATIONS (position=station list,
default CZQO_*=CZQX, EGGX_*=EGGX, NAT_*=NATX), VNAAATS_HOPPIE_URL, VNAAATS_CPDLC_RCL_SEND (on = show the RCL processed / Send reclearance buttons; off by default), VNAAATS_CPDLC_AUTOACCEPT (1 = accept pilots' logon requests automatically, 0 = off).
VNAAATS_INSTRUCTOR_PASSWORD (10+ characters; switches on the hidden /trainee-key page where an instructor issues a 2-hour key for a student),
VNAAATS_CONNECT_BASE (https://auth.vatsim.net, sandbox: https://auth-dev.vatsim.net), VNAAATS_ALLOWED_CIDS (comma list; if empty
any VATSIM member rated VNAAATS_MIN_RATING or higher, default 2 = S1, may log in).
"""
import hashlib
import hmac
import http.cookies
import json
import logging
import math
import os
import re
import secrets
import sqlite3
import sys
import threading
import time
import urllib.parse
import urllib.request
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlparse

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import cpdlc as cpdlc_mod
import clx as clx_mod
import commands as commands_mod
import rcl_auto as rcl_auto_mod
import members as members_mod
import usersettings as usersettings_mod
import pluginfiles as pluginfiles_mod
import adminlog as adminlog_mod
log = logging.getLogger("vnaaats")

TRAINEE_KEY_HOURS = 2            # how long an instructor-issued trainee key lasts
NATTRAK_URL = os.environ.get("VNAAATS_NATTRAK_URL", "https://nattrak.vatsim.net/api/tracks")
REFRESH = int(os.environ.get("VNAAATS_REFRESH_SECONDS", "600"))
STALE_FLIGHT_SECONDS = 24 * 3600
SERVER_VERSION = "2.6.0"      # shown at /health so you can tell which version is running
VATSIM_FEED = os.environ.get("VNAAATS_VATSIM_FEED", "https://data.vatsim.net/v3/vatsim-data.json")
VATSIM_TRANSCEIVERS = os.environ.get("VNAAATS_VATSIM_TRANSCEIVERS", "https://data.vatsim.net/v3/transceivers-data.json")
FEED_SECONDS = int(os.environ.get("VNAAATS_FEED_SECONDS", "60"))
NATTRAK_PLUGIN_URL = os.environ.get("VNAAATS_NATTRAK_PLUGIN_URL", "https://nattrak.vatsim.net/api/plugins")
SESSION_COOKIE = "vnaaats_session"
SESSION_SECONDS = 12 * 3600
DEFAULT_POSITIONS = "CZQO_CTR,CZQO_DEL,EGGX_CTR,EGGX_DEL,NAT_FSS"
EDIT_FIELDS = ("assignedLevel", "assignedMach", "track", "selcal")       # what the web and the plugin both edit (route is web-only)
SELCAL_OK = re.compile(r"[A-Z]{4}")   # VATSIM pilots can use any four letters
SCRATCH_MAX = 15                                  # characters in the scratchpad note (EuroScope itself documents no limit)
SCRATCH_OK = re.compile(r"[A-Z0-9 /.,+*-]{0,%d}" % SCRATCH_MAX)


# ----------------------------------------------------------------------------- fixes
def load_fixes():
    fixes = {}
    for name in ("fixes.json", "extra_fixes.json"):          # extra_fixes.json wins
        path = os.path.join(HERE, name)
        if os.path.exists(path):
            with open(path, encoding="utf-8") as f:
                fixes.update({k.upper(): v for k, v in json.load(f).items() if not k.startswith("_")})
    return fixes


def token_to_latlon(token, fixes):
    """'56/20' -> (56.0, -20.0); '5730/30' -> (57.5, -30.0); named fix -> lookup; else None."""
    m = re.fullmatch(r"(\d{2})(\d{2})?/(\d{2,3})", token)
    if m:
        lat = int(m.group(1)) + (int(m.group(2)) / 60.0 if m.group(2) else 0.0)
        lon = int(m.group(3))
        return round(lat, 6), -float(lon)                    # NAT tracks are west of Greenwich
    v = fixes.get(token.upper()) or TANGO_FIXES.get(token.upper())
    return (v[0], v[1]) if v else None


def _gc_nm(lat1, lon1, lat2, lon2):
    """Great-circle distance in nautical miles."""
    p1, p2 = math.radians(lat1), math.radians(lat2)
    a = math.sin((p2 - p1) / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(math.radians(lon2 - lon1) / 2) ** 2
    return 3440.065 * 2 * math.asin(min(1.0, math.sqrt(a)))


def _bearing(lat1, lon1, lat2, lon2):
    p1, p2, dl = math.radians(lat1), math.radians(lat2), math.radians(lon2 - lon1)
    y = math.sin(dl) * math.cos(p2)
    x = math.cos(p1) * math.sin(p2) - math.sin(p1) * math.cos(p2) * math.cos(dl)
    return (math.degrees(math.atan2(y, x)) + 360) % 360


def estimate_etas(route, pilot, fixes, now=None):
    """Estimated times (HHMM, Zulu) over each fix of a route, for a flight nobody has handled in the plugin yet.
    From the pilot's position and ground speed in the VATSIM feed, straight along the route from the first fix still ahead.
    Fixes already passed, or ones that cannot be placed, get "--". Returns None when no estimate is possible (on the ground, climbing, no position)."""
    toks = str(route or "").split()
    try:
        lat, lon = float(pilot["latitude"]), float(pilot["longitude"])
        gs, hdg, alt = float(pilot.get("groundspeed") or 0), float(pilot.get("heading") or 0), float(pilot.get("altitude") or 0)
    except (KeyError, TypeError, ValueError):
        return None
    if not toks or gs < 150 or gs > 700 or alt < 10000:
        return None
    pts = [token_to_latlon(t, fixes) for t in toks]
    start = None
    for i, ll in enumerate(pts):
        if ll and abs((_bearing(lat, lon, ll[0], ll[1]) - hdg + 540) % 360 - 180) <= 100:
            start = i
            break
    if start is None:
        return None
    now = time.time() if now is None else now
    out, dist, prev = ["--"] * len(toks), 0.0, (lat, lon)
    for i in range(start, len(toks)):
        if not pts[i]:
            continue
        dist += cpdlc_mod.nm_between(prev[0], prev[1], pts[i][0], pts[i][1])
        prev = pts[i]
        if dist > 6000:
            break
        t = datetime.fromtimestamp(now + dist / gs * 3600, timezone.utc)
        out[i] = t.strftime("%H%M")
    return out if any(x != "--" for x in out) else None


OCEANIC_PREFIXES = tuple(x.strip().upper() for x in os.environ.get("VNAAATS_OCEANIC_PREFIXES", "CZQO,CZQX,EGGX,NAT,BIRD,KZWY,LPPO,ENOB").split(",") if x.strip())


def nt_stops_reminder(rec, natrak):
    """The red natTrak star goes away once the pilot has requested clearance, or an oceanic controller tracks the flight."""
    cs = str(rec.get("callsign", "")).upper()
    if natrak and cs in natrak.by_callsign():
        return True
    by = str(rec.get("trackedBy", "")).upper()
    return bool(by) and by.startswith(OCEANIC_PREFIXES)


def route_distance_nm(route, pos, fixes):
    """Distance in nm from a position (lat, lon) to the nearest fix on a route. None if either is unknown."""
    if not pos:
        return None
    best = None
    for tok in str(route or "").split():
        ll = token_to_latlon(tok, fixes)
        if ll:
            d = cpdlc_mod.nm_between(pos[0], pos[1], ll[0], ll[1])
            best = d if best is None or d < best else best
    return None if best is None else round(best)


# ----------------------------------------------------------------------------- tracks
def to_epoch_string(iso):
    """'2026-10-20T01:00:00.000000Z' -> '1792458000' (unix seconds as a string, which is what the plugin and web app expect)."""
    try:
        dt = datetime.fromisoformat(re.sub(r"\.\d+", "", iso).replace("Z", "+00:00"))
        if dt.tzinfo is None:
            dt = dt.replace(tzinfo=timezone.utc)
        return str(int(dt.timestamp()))
    except (ValueError, AttributeError):
        return ""


def convert_tracks(raw, fixes, unresolved):
    """nattrak JSON -> the format the plugin parses."""
    out, valid_from_dates = [], []
    for t in raw:
        if not t.get("active"):
            continue
        tokens = (t.get("last_routeing") or "").split()
        route = []
        for tok in tokens:
            ll = token_to_latlon(tok, fixes)
            if ll is None:
                unresolved[tok] = unresolved.get(tok, 0) + 1
                log.warning("No coordinates for fix %s (track %s)", tok, t.get("identifier"))
                continue
            route.append({"name": tok, "latitude": ll[0], "longitude": ll[1]})
        if len(route) < 2:
            continue
        direction = {"west": 1, "east": 2}.get(str(t.get("direction", "")).lower(), 0)
        vf = t.get("valid_from") or ""
        valid_from_dates.append(vf)
        out.append({
            "id": t["identifier"],
            "tmi": "",
            "direction": direction,
            "route": route,
            "flightLevels": [int(x) for x in (t.get("flight_levels") or [])],
            "validFrom": to_epoch_string(vf),
            "validTo": to_epoch_string(t.get("valid_to") or ""),
        })
    # TMI = day-of-year of the (earliest) validity start. UNVERIFIED against the official definition.
    tmi = ""
    dates = sorted(d for d in valid_from_dates if d)
    if dates:
        try:
            tmi = str(datetime.fromisoformat(dates[0].replace("Z", "+00:00")).timetuple().tm_yday)
        except ValueError:
            pass
    for t in out:
        t["tmi"] = tmi
    return out


class TrackCache:
    def __init__(self, fixes):
        self.fixes = fixes
        self.last_raw = []
        self.lock = threading.Lock()
        self.tracks = []
        self.unresolved = {}
        self.updated = None
        self.error = None

    def refresh(self, fetch=None):
        try:
            raw = fetch() if fetch else self._download()
            self.last_raw = raw
            unresolved = {}
            tracks = convert_tracks(raw, self.fixes, unresolved)
            with self.lock:
                self.tracks, self.unresolved = tracks, unresolved
                self.updated, self.error = time.time(), None
            log.info("Tracks refreshed: %d active", len(tracks))
        except Exception as e:                                # keep serving the last good copy
            with self.lock:
                self.error = str(e)
            log.error("Track refresh failed: %s", e)

    def add_fix(self, name, lat, lon):
        """Learn a fix and rebuild the track list from the last download straight away."""
        self.fixes[name.upper()] = [lat, lon]
        if self.last_raw:
            unresolved = {}
            tracks = convert_tracks(self.last_raw, self.fixes, unresolved)
            with self.lock:
                self.tracks, self.unresolved = tracks, unresolved

    @staticmethod
    def _download():
        req = urllib.request.Request(NATTRAK_URL, headers={"User-Agent": "vnaaats-server"})
        with urllib.request.urlopen(req, timeout=30) as r:
            return json.loads(r.read().decode("utf-8"))

    def loop(self):
        while True:
            self.refresh()
            time.sleep(REFRESH)


# ----------------------------------------------------------------------------- flights
def to_bool(v):
    return str(v).strip().lower() in ("1", "true", "yes", "y")


def is_conc(aircraft_type):
    """Concorde (ICAO type CONC) reports by voice, so controllers type its times in."""
    return str(aircraft_type or "").strip().upper().startswith("CONC")


def clean_time(value):
    """'1432', '14:32' or '1432z' -> '1432'; '' -> ''; anything else -> None."""
    t = re.sub(r"[:\sZz]", "", str(value or ""))
    if t == "":
        return ""
    if re.fullmatch(r"([01]\d|2[0-3])[0-5]\d", t):
        return t
    return None


def to_int(v, default=0):
    try:
        return int(float(v))
    except (TypeError, ValueError):
        return default


class FlightStore:
    def __init__(self, path):
        self.lock = threading.Lock()
        self.db = sqlite3.connect(path, check_same_thread=False, timeout=30)
        try: self.db.execute("PRAGMA journal_mode=WAL")
        except sqlite3.DatabaseError: pass
        self.db.execute("CREATE TABLE IF NOT EXISTS flights (callsign TEXT PRIMARY KEY, data TEXT NOT NULL, updated REAL NOT NULL)")
        self.db.execute("CREATE TABLE IF NOT EXISTS dismissed (callsign TEXT PRIMARY KEY, at REAL NOT NULL, by TEXT NOT NULL DEFAULT '')")
        self.db.execute("CREATE TABLE IF NOT EXISTS fixes (name TEXT PRIMARY KEY, lat REAL NOT NULL, lon REAL NOT NULL)")
        self.db.execute("""CREATE TABLE IF NOT EXISTS pos_reports (callsign TEXT NOT NULL, fix TEXT NOT NULL, est TEXT NOT NULL DEFAULT '',
            act TEXT NOT NULL DEFAULT '', by TEXT NOT NULL DEFAULT '', at TEXT NOT NULL DEFAULT '', ts REAL NOT NULL, PRIMARY KEY (callsign, fix))""")
        self.db.commit()

    # --- position reports for aircraft that report by voice (Concorde): estimate and actual time per route fix
    def pos_set(self, callsign, fix, est, act, by):
        cs, fix = callsign.strip().upper(), fix.strip().upper()
        now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        with self.lock:
            if not est and not act:
                self.db.execute("DELETE FROM pos_reports WHERE callsign=? AND fix=?", (cs, fix))
            else:
                self.db.execute("INSERT OR REPLACE INTO pos_reports VALUES (?,?,?,?,?,?,?)", (cs, fix, est, act, by, now, time.time()))
            self.db.commit()

    def pos_get(self, callsign):
        with self.lock:
            rows = self.db.execute("SELECT fix, est, act, by, at FROM pos_reports WHERE callsign=?", (callsign.strip().upper(),)).fetchall()
        return {f: {"est": e, "act": a, "by": b, "at": t} for f, e, a, b, t in rows}

    DISMISS_SECONDS = 12 * 3600       # a removed strip stays hidden this long unless it is added again

    def dismiss(self, callsign, by):
        cs = callsign.strip().upper()
        with self.lock:
            self.db.execute("DELETE FROM flights WHERE callsign=?", (cs,))
            self.db.execute("INSERT OR REPLACE INTO dismissed VALUES (?,?,?)", (cs, time.time(), by))
            self.db.commit()

    def undismiss(self, callsign):
        with self.lock:
            self.db.execute("DELETE FROM dismissed WHERE callsign=?", (callsign.strip().upper(),))
            self.db.commit()

    def dismissed_set(self):
        with self.lock:
            self.db.execute("DELETE FROM dismissed WHERE at < ?", (time.time() - self.DISMISS_SECONDS,))
            self.db.commit()      # never leave a write open: it would lock the CPDLC database connection out ("database is locked")
            rows = self.db.execute("SELECT callsign FROM dismissed").fetchall()
        return {r[0] for r in rows}

    def add_manual(self, rec, by):
        """Put a flight on the display by hand. If we already hold it, keep what we have and just mark it as added by hand."""
        cs = rec["callsign"]
        now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        with self.lock:
            self.db.execute("DELETE FROM dismissed WHERE callsign=?", (cs,))
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            out = json.loads(row[0]) if row else dict(rec, history=[])
            out["manual"] = {"by": by, "at": now}
            out["lastUpdated"] = now
            self.db.execute("INSERT OR REPLACE INTO flights VALUES (?,?,?)", (cs, json.dumps(out), time.time()))
            self.db.commit()
        return out

    def all_flights(self, newest_first=True):
        with self.lock:
            rows = self.db.execute("SELECT data FROM flights ORDER BY updated %s" % ("DESC" if newest_first else "ASC")).fetchall()
        return [json.loads(r[0]) for r in rows]

    def set_fix(self, name, lat, lon):
        with self.lock:
            self.db.execute("INSERT OR REPLACE INTO fixes VALUES (?,?,?)", (name.upper(), lat, lon))
            self.db.commit()

    def get_fixes(self):
        with self.lock:
            return {n: [la, lo] for n, la, lo in self.db.execute("SELECT name, lat, lon FROM fixes")}

    def upsert(self, q):
        cs = q.get("callsign", "").strip().upper()
        if not cs:
            return None
        g = lambda k: q.get(k, "")
        now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        rec = {
            "callsign": cs, "type": g("type"),
            "assignedLevel": to_int(g("level")), "assignedMach": to_int(g("mach")),
            "track": g("track"), "route": g("route"), "routeEtas": g("routeEtas"),
            "departure": g("departure"), "arrival": g("arrival"),
            "direction": to_bool(g("direction")), "etd": g("etd"), "selcal": g("selcal"),
            "datalinkConnected": to_bool(g("datalinkConnected")), "isEquipped": to_bool(g("isEquipped")),
            "state": g("state"), "relevant": to_bool(g("relevant")), "trackedBy": g("trackedBy"),
            "trackedById": g("trackedById"), "targetMode": to_int(g("targetMode")),
            "lastUpdated": now,
        }
        by = (g("by") or "Plugin")[:40]
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            old = json.loads(row[0]) if row else None
            for k in ("type", "departure", "arrival"):
                # an incomplete plan (blank or NOFP) from one plugin must not wipe real details another plugin already sent
                if old and str(rec[k]).strip().upper() in ("", "NOFP") and str(old.get(k, "")).strip().upper() not in ("", "NOFP"):
                    rec[k] = old[k]
            if old and old.get("webEditPending"):
                # a controller edited this on the web and the plugin has not fetched it yet: keep the web values
                for k in EDIT_FIELDS:
                    rec[k] = old.get(k, rec[k])
                rec["webEditPending"] = True
            if old and old.get("ntSent"):
                rec["ntSent"] = old["ntSent"]                 # natTrak reminder sent: survives plugin updates
            if old and old.get("cmAt"):
                rec["cmAt"] = old["cmAt"]                     # contact-me sent: survives plugin updates
            if old and old.get("manual"):
                rec["manual"] = old["manual"]                 # added by hand on the display: stays so
            if old and "scratch" in old:
                rec["scratch"] = old["scratch"]               # scratchpad note: changed only by set_scratch / web edit
            if old and old.get("etaWeb") and not old.get("routeWeb"):
                rec["routeEtas"], rec["etaWeb"] = old.get("routeEtas", rec["routeEtas"]), True   # entry time set on the web
            if old and old.get("routeWeb"):
                # the route was amended on the web (the plugin cannot read that): keep it
                rec["route"], rec["routeEtas"], rec["routeWeb"] = old.get("route", rec["route"]), old.get("routeEtas", rec["routeEtas"]), True
            self._track_changes(rec, old, by, now, ("route", "state") + EDIT_FIELDS)
            self.db.execute("INSERT OR REPLACE INTO flights VALUES (?,?,?)", (cs, json.dumps(rec), time.time()))
            self.db.commit()
        return rec

    @staticmethod
    def _track_changes(rec, old, by, now, fields):
        """Keep updatedBy / updatedAt / history only moving when something that matters actually changed."""
        changes = {}
        if old:
            for k in fields:
                if old.get(k) != rec.get(k):
                    changes[k] = [old.get(k), rec.get(k)]
            rec["history"] = old.get("history", [])
            if not changes:
                rec["updatedBy"], rec["updatedAt"] = old.get("updatedBy", by), old.get("updatedAt", now)
                return
        else:
            rec["history"] = []
        rec["updatedBy"], rec["updatedAt"] = by, now
        if changes:
            rec["history"] = (rec["history"] + [{"at": now, "by": by, "changes": changes}])[-10:]

    def edit(self, callsign, changes, by, base=None):
        """Apply a web edit. base is a record to start from when the flight is only known from the VATSIM feed."""
        cs = callsign.strip().upper()
        now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            old = json.loads(row[0]) if row else None
            if old is None and base is None:
                return None
            rec = dict(old if old is not None else base)
            rec["callsign"] = cs
            entry = changes.pop("entryEta", None)
            for k, v in changes.items():
                rec[k] = v
            if entry:
                rec["routeEtas"] = shift_entry_eta(str(rec.get("routeEtas", "")), entry, len(str(rec.get("route", "")).split()))
                rec["etaWeb"] = True
            if "route" in changes:           # keep the times of fixes that stay; new fixes have none
                prev = dict(zip(str((old or base).get("route", "")).split(), str((old or base).get("routeEtas", "")).split()))
                rec["routeEtas"] = " ".join(prev.get(f, "--") for f in changes["route"].split())
                rec["routeWeb"] = True
            rec["lastUpdated"] = now
            rec["webEditPending"] = True
            self._track_changes(rec, old if old is not None else dict(base, history=[]), by, now, EDIT_FIELDS)
            self.db.execute("INSERT OR REPLACE INTO flights VALUES (?,?,?)", (cs, json.dumps(rec), time.time()))
            self.db.commit()
        return rec

    def set_scratch(self, callsign, text, by):
        """Scratchpad note typed in EuroScope (plugin) - kept on the flight so the FDD shows it. Returns True if the flight is known."""
        cs = callsign.strip().upper()
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            if not row:
                return False
            rec = json.loads(row[0])
            if rec.get("scratch", "") != text:
                rec["scratch"] = text
                self.db.execute("UPDATE flights SET data=? WHERE callsign=?", (json.dumps(rec), cs))
                self.db.commit()
        return True

    def mark_nt(self, callsign, by, sent=True, base=None):
        """Remember (or forget) that the natTrak reminder was sent to this pilot. Returns the record or None."""
        cs = callsign.strip().upper()
        now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            if row:
                rec = json.loads(row[0])
            elif base is not None:
                rec = dict(base, history=[])
                rec["callsign"] = cs
                rec["lastUpdated"] = now
            else:
                return None
            if sent:
                rec["ntSent"] = {"by": by, "at": now}
            else:
                rec.pop("ntSent", None)
            self.db.execute("INSERT OR REPLACE INTO flights VALUES (?,?,?)", (cs, json.dumps(rec), time.time()))
            self.db.commit()
        return rec

    def mark_cm(self, callsign, base=None, now=None):
        """Remember that a .contactme was sent to this pilot (the strip turns yellow, then red). Returns the record or None."""
        cs = callsign.strip().upper()
        now = now or time.time()
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            if row:
                rec = json.loads(row[0])
            elif base is not None:
                rec = dict(base, history=[])
                rec["callsign"] = cs
                rec["lastUpdated"] = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
            else:
                return None
            rec["cmAt"] = now
            self.db.execute("INSERT OR REPLACE INTO flights VALUES (?,?,?)", (cs, json.dumps(rec), time.time()))
            self.db.commit()
        return rec

    def ack_web_edit(self, callsign):
        cs = callsign.strip().upper()
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (cs,)).fetchone()
            if not row:
                return
            rec = json.loads(row[0])
            if rec.get("webEditPending"):
                rec["webEditPending"] = False
                self.db.execute("UPDATE flights SET data=? WHERE callsign=?", (json.dumps(rec), cs))
                self.db.commit()

    def get(self, callsign):
        with self.lock:
            row = self.db.execute("SELECT data FROM flights WHERE callsign=?", (callsign.strip().upper(),)).fetchone()
        return json.loads(row[0]) if row else None

    def purge(self):
        with self.lock:
            self.db.execute("DELETE FROM flights WHERE updated < ?", (time.time() - STALE_FLIGHT_SECONDS,))
            self.db.execute("DELETE FROM pos_reports WHERE ts < ?", (time.time() - 24 * 3600,))
            self.db.commit()


# ----------------------------------------------------------------------------- network traffic
_SPEEDLEVEL = re.compile(r"/[NMK]\d{3,4}[FASM]\d{3,4}$")


# ----------------------------------------------------------------------------- Concorde (supersonic) tracks
# From the Gander Oceanic SOPs, Annex B 1.1.2: points are degrees.minutes (50.41 = 50 deg 41 min). Shown on the web display only.
CONCORDE_TRACK_DEFS = [
    ("SM", 1, "westbound only", [("SM15W", 50, 41, 15, 0), ("SM20W", 50, 50, 20, 0), ("SM30W", 50, 30, 30, 0), ("SM40W", 49, 16, 40, 0), ("SM50W", 47, 3, 50, 0),
                                 ("SM53W", 46, 10, 53, 0), ("SM60W", 44, 14, 60, 0), ("SM65W", 42, 46, 65, 0), ("SM67W", 42, 0, 67, 0)]),
    ("SN", 2, "eastbound only", [("SN67W", 40, 25, 67, 0), ("SN65W", 41, 40, 65, 0), ("SN60W", 43, 7, 60, 0), ("SN52W", 45, 10, 52, 30), ("SN50W", 45, 54, 50, 0),
                                 ("SN40W", 48, 10, 40, 0), ("SN30W", 49, 26, 30, 0), ("SN20W", 49, 49, 20, 0), ("SN15W", 49, 41, 15, 0)]),
    ("SO", 0, "overflow, either way (as coordinated)", [("SO15W", 48, 40, 15, 0), ("SO20W", 48, 48, 20, 0), ("SO30W", 48, 22, 30, 0), ("SO40W", 47, 4, 40, 0),
                                                        ("SO50W", 44, 45, 50, 0), ("SO52W", 44, 10, 52, 0), ("SO60W", 42, 0, 60, 0)]),
    ("SP", 0, "to the Caribbean, either way", [("SP15W", 48, 49, 14, 43), ("SP20W", 46, 49, 20, 0), ("SP45N", 45, 0, 23, 53), ("SP30W", 41, 36, 30, 0), ("SP40W", 34, 22, 40, 0)]),
]
CONC_FIXES = {n: (a + b / 60.0, -(c + d / 60.0)) for _, _, _, pts in CONCORDE_TRACK_DEFS for n, a, b, c, d in pts}
CONC_BLOCK = "FL470-600"


def concorde_tracks():
    """The four Concorde tracks in the same shape as the NAT tracks, plus supersonic/blockLevel/use for the web display."""
    return [{"id": tid, "tmi": "C", "direction": d, "supersonic": True, "blockLevel": CONC_BLOCK, "use": use,
             "route": [{"name": n, "latitude": round(CONC_FIXES[n][0], 4), "longitude": round(CONC_FIXES[n][1], 4)} for n, *_ in pts],
             "flightLevels": [], "validFrom": "", "validTo": ""} for tid, d, use, pts in CONCORDE_TRACK_DEFS]


# ----------------------------------------------------------------------------- Shanwick Tango routes
# Fixed RNAV routes in the south-east corner of the Shanwick OCA (between Northern Europe, Iberia and the Canaries). They are
# worked by Shanwick Oceanic and need an oceanic clearance like any other flight. Coordinates and rules: the Shanwick Tango route
# chart (positions in degrees, minutes, seconds as published) and the Gander Oceanic SOPs. Shown on the web display only.
def _dms(d, m=0.0, s=0.0):
    return d + m / 60.0 + s / 3600.0


TANGO_FIXES = {
    "OMOKO": (_dms(48, 50.2), -_dms(12, 0)),
    "TAMEL": (_dms(48, 43.7), -_dms(10, 29.8)),
    "LASNO": (_dms(48, 35.9), -_dms(9, 0)),
    "GELPO": (_dms(48, 38, 19), -_dms(9, 29, 19)),
    "ADVAT": (_dms(45, 0), -_dms(9, 28, 33)),
    "BEGAS": (_dms(45, 0), -_dms(9, 0)),
    "BERUX": (_dms(45, 0), -_dms(11, 0)),
    "NILAV": (_dms(45, 0), -_dms(13, 25)),
    "GONAN": (_dms(45, 0), -_dms(14, 0)),
}
_EVEN_300_400 = [fl * 100 for fl in range(300, 401, 20)]
_ODD_290_410 = [fl * 100 for fl in range(290, 411, 20)]
# id, rule in words, flight levels (feet, empty = no level rule published here), fixes in the direction of use
TANGO_ROUTE_DEFS = [
    ("T9", "southbound only, even levels FL300-400", _EVEN_300_400, ["LASNO", "BEGAS"]),
    ("T290", "northbound only, odd levels FL290-410", _ODD_290_410, ["ADVAT", "GELPO"]),
    ("T213", "both directions", [], ["TAMEL", "BERUX"]),
    ("T13", "northbound, continues into Santa Maria", [], ["NILAV", "OMOKO"]),
    ("T16", "southbound, continues into Santa Maria", [], ["OMOKO", "GONAN"]),
]


def tango_tracks():
    """The Tango routes in the same shape as the NAT tracks, plus tango:true and use for the web display."""
    return [{"id": tid, "tmi": "T", "direction": 0, "tango": True, "use": use,
             "tangoDir": "both" if use.startswith("both") else ("north" if use.startswith("north") else "south"),
             "route": [{"name": n, "latitude": round(TANGO_FIXES[n][0], 4), "longitude": round(TANGO_FIXES[n][1], 4)} for n in fixes],
             "flightLevels": list(levels), "validFrom": "", "validTo": ""} for tid, use, levels, fixes in TANGO_ROUTE_DEFS]


def tango_match(tokens):
    """The Tango route a filed route uses, or None. Returns (id, fixes in the order flown, northbound).
    A route matches when it names the airway (T9 ...) and at least one of that route's fixes, or names both of its fixes.
    The airway name alone is not enough: other parts of the world have a T9 too."""
    toks = [_SPEEDLEVEL.sub("", str(t).upper()) for t in tokens]
    best = None
    for tid, _use, _levels, fixes in TANGO_ROUTE_DEFS:
        pos = {f: toks.index(f) for f in fixes if f in toks}
        named = tid in toks
        if not ((named and pos) or len(pos) == len(fixes)):
            continue
        first = min(pos.values()) if pos else toks.index(tid)
        if best is None or first < best[0]:
            flown = sorted(pos, key=lambda f: pos[f]) if len(pos) > 1 else list(fixes)
            best = (first, tid, flown)
    if not best:
        return None
    _, tid, flown = best
    if len(flown) < 2:
        flown = next(list(f) for t, _u, _l, f in TANGO_ROUTE_DEFS if t == tid)
    north = TANGO_FIXES[flown[-1]][0] > TANGO_FIXES[flown[0]][0]
    return tid, flown, north


def oceanic_point(token):
    """Coordinate waypoints as used on the North Atlantic: '52/50', '5730/30', '52N050W', '5230N05000W'.
    Returns (name, lat, lon) or None."""
    t = _SPEEDLEVEL.sub("", token.upper())
    if t in CONC_FIXES:                                       # SM20W, SN30W ... (Concorde tracks)
        return t, CONC_FIXES[t][0], CONC_FIXES[t][1]
    m = re.fullmatch(r"(\d{2})(\d{2})N|N(\d{2})(\d{2})", t)   # ARINC 424 7.2.5: 5020N = 50N 020W, N5020 = 50.5N (half degree) 020W
    if m:
        if m.group(1):
            return t, float(m.group(1)), -float(m.group(2))
        return t, float(m.group(3)) + 0.5, -float(m.group(4))
    m = re.fullmatch(r"(\d{2})(\d{2})?/(\d{2,3})", t)
    if m:
        lat = int(m.group(1)) + (int(m.group(2)) / 60.0 if m.group(2) else 0.0)
        return t, lat, -float(m.group(3))
    m = re.fullmatch(r"(\d{2})(\d{2})?([NS])(\d{3})(\d{2})?([EW])", t)
    if m:
        lat = int(m.group(1)) + (int(m.group(2)) / 60.0 if m.group(2) else 0.0)
        lon = int(m.group(4)) + (int(m.group(5)) / 60.0 if m.group(5) else 0.0)
        if m.group(3) == "S":
            lat = -lat
        if m.group(6) == "W":
            lon = -lon
        return t, lat, lon
    return None


# ----------------------------------------------------------------------------- NAT track tokens in filed routes
NAT_TOKEN = re.compile(r"NAT([A-Z])")


def expand_nat_tokens(tokens, tracks, etas=None):
    """Pilots file  ENTRY NATF EXIT  (for example  HOIST NATF GOMUP). Replace the NATx token by the fixes of the
    active track x, from the filed entry fix to the filed exit fix, so the route has the oceanic coordinates.
    Returns (tokens, etas, expanded_letters). etas (a list the same length as tokens, or None) gets "--" for new fixes.
    A NATx token with no active track of that letter is dropped (it is not a real fix)."""
    by_id = {str(t.get("id", "")).upper(): [str(r["name"]).upper() for r in t.get("route", [])] for t in tracks}
    out, out_etas, done = [], ([] if etas is not None else None), []
    for i, tok in enumerate(tokens):
        m = NAT_TOKEN.fullmatch(_SPEEDLEVEL.sub("", tok.upper()))
        if not m:
            out.append(tok)
            if out_etas is not None:
                out_etas.append(etas[i] if i < len(etas) else "--")
            continue
        names = by_id.get(m.group(1))
        if not names:
            continue                                          # no such track today: drop the token
        prev = next((x.upper() for x in reversed(out) if x.upper() != "DCT"), None)
        nxt = next((x.upper() for x in tokens[i + 1:] if x.upper() != "DCT" and not NAT_TOKEN.fullmatch(_SPEEDLEVEL.sub("", x.upper()))), None)
        ip = names.index(prev) if prev in names else None
        inn = names.index(nxt) if nxt in names else None
        if ip is not None and inn is not None:
            inner = names[ip + 1:inn] if ip < inn else list(reversed(names[inn + 1:ip]))
        elif ip is not None:
            inner = names[ip + 1:]
        elif inn is not None:
            inner = names[:inn]
        else:
            inner = names
        for n in inner:
            out.append(n)
            if out_etas is not None:
                out_etas.append("--")
        done.append(m.group(1))
    return out, out_etas, done


def parse_level(alt):
    """Flight plan altitude ('39000', 'FL390', 'F390', '390', 'A050') -> flight level (390)."""
    digits = re.sub(r"\D", "", alt or "")
    if not digits:
        return 0
    n = int(digits)
    if alt.upper().startswith("A"):
        return n
    return n // 100 if n >= 1000 else n


def _plain_fix(token):
    """A named 5-letter fix (ALLRY, MALOT) - not DCT, an airway, a procedure or a coordinate."""
    t = _SPEEDLEVEL.sub("", token.upper())
    return t if re.fullmatch(r"[A-Z]{5}", t) and t != "DCT" else None


SELCAL_REMARK = re.compile(r"\bSEL(?:CAL)?\s*[/:= ]\s*([A-Z]{4})\b")


def selcal_from_remarks(remarks):
    """The SELCAL a pilot filed in the remarks of the VATSIM flight plan (SEL/ABCD, SELCAL ABCD), or "". Any four letters."""
    m = SELCAL_REMARK.search(str(remarks or "").upper())
    return m.group(1) if m else ""


def convert_network_pilot(pilot, tracks):
    """One pilot from the VATSIM data feed -> a flight record, or None if the route does not cross the ocean.
    The route is the oceanic coordinates plus the named entry fix before them and the exit fix after them."""
    fp = pilot.get("flight_plan") or {}
    route_str = fp.get("route") or ""
    tokens = expand_nat_tokens(route_str.split(), tracks)[0]  # HOIST NATF GOMUP -> HOIST <track F fixes> GOMUP
    idx = []                                                  # (index in route, (name, lat, lon)) of oceanic coordinates
    for i, tok in enumerate(tokens):
        p = oceanic_point(tok)
        if p and 30 <= p[1] <= 72 and -70 <= p[2] <= -4:
            idx.append((i, p))
    if len(idx) < 2:
        tg = tango_match(tokens)
        if tg:
            return _tango_record(pilot, fp, tg)
        if inactive_nat_token(route_str, tracks):                 # filed NATE but track E is not in use today: still ours to see (an RR flight)
            return _rr_record(pilot, fp, tracks)
        return None
    pts = [p for _, p in idx]
    entry = exit_ = None
    j = idx[0][0] - 1                                         # entry fix: the token before the first coordinate (skipping DCT)
    while j >= 0 and tokens[j].upper() == "DCT":
        j -= 1
    if j >= 0:
        entry = _plain_fix(tokens[j])
    j = idx[-1][0] + 1                                        # exit fix: the token after the last coordinate (skipping DCT)
    while j < len(tokens) and tokens[j].upper() == "DCT":
        j += 1
    if j < len(tokens):
        exit_ = _plain_fix(tokens[j])
    names = ([entry] if entry else []) + [p[0] for p in pts] + ([exit_] if exit_ else [])
    coord_names = [p[0] for p in pts]
    direction = pts[-1][2] > pts[0][2]                        # moving towards Europe = eastbound
    track = "RR"
    flown = {(round(p[1], 2), round(p[2], 2)) for p in pts}   # compare positions, not spelling (61/50 == 61N050W)
    for t in list(tracks) + concorde_tracks():                # on a track if it passes the whole track's coordinate fixes (Concorde tracks too)
        tpts = [oceanic_point(r["name"]) for r in t.get("route", [])]
        tpos = [(round(q[1], 2), round(q[2], 2)) for q in tpts if q]
        if len(tpos) >= 3 and all(pos in flown for pos in tpos):
            track = t["id"]
            break
    return {
        "callsign": pilot.get("callsign", "").upper(), "type": fp.get("aircraft_short") or "",
        "assignedLevel": parse_level(fp.get("altitude")), "assignedMach": 0,
        "track": track, "route": " ".join(names), "routeEtas": " ".join(["--"] * len(names)),
        "departure": fp.get("departure") or "", "arrival": fp.get("arrival") or "",
        "direction": direction, "etd": fp.get("deptime") or "", "selcal": selcal_from_remarks(fp.get("remarks")),
        "datalinkConnected": False, "isEquipped": True, "state": "", "relevant": True,
        "trackedBy": "", "trackedById": "", "targetMode": 3,
        "lastUpdated": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "source": "vatsim",
    }



def _tango_record(pilot, fp, tg):
    """A flight record for a pilot filed on a Tango route. Northbound counts as 'eastbound', southbound as 'westbound'."""
    tid, flown, north = tg
    return {
        "callsign": pilot.get("callsign", "").upper(), "type": fp.get("aircraft_short") or "",
        "assignedLevel": parse_level(fp.get("altitude")), "assignedMach": 0,
        "track": tid, "route": " ".join(flown), "routeEtas": " ".join(["--"] * len(flown)),
        "departure": fp.get("departure") or "", "arrival": fp.get("arrival") or "",
        "direction": north, "etd": fp.get("deptime") or "", "selcal": selcal_from_remarks(fp.get("remarks")),
        "datalinkConnected": False, "isEquipped": True, "state": "", "relevant": True,
        "trackedBy": "", "trackedById": "", "targetMode": 3,
        "lastUpdated": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "source": "vatsim",
    }


def inactive_nat_token(route_str, tracks):
    """True if the filed route has a NATx token for a track that is not in use today (so it could not be expanded)."""
    active = {str(t.get("id", "")).upper() for t in tracks}
    for tok in (route_str or "").upper().split():
        m = NAT_TOKEN.fullmatch(_SPEEDLEVEL.sub("", tok))
        if m and m.group(1) not in active:
            return True
    return False


def _rr_record(pilot, fp, tracks):
    """A random-route flight built from the plain fixes of the filed route (no oceanic coordinates to go on).
    The direction is a guess from where the flight starts (Europe = westbound)."""
    names = []
    for tok in expand_nat_tokens((fp.get("route") or "").upper().split(), tracks)[0]:
        if tok == "DCT" or re.fullmatch(r"[A-Z]{1,2}\d{1,3}[A-Z]?", tok) or re.fullmatch(r"[NMK]\d{3,4}[FAMS]\d{3,4}", tok):
            continue
        names.append(tok)
    dep = (fp.get("departure") or "").upper()
    return {
        "callsign": pilot.get("callsign", "").upper(), "type": fp.get("aircraft_short") or "",
        "assignedLevel": parse_level(fp.get("altitude")), "assignedMach": 0,
        "track": "RR", "route": " ".join(names), "routeEtas": " ".join(["--"] * len(names)),
        "departure": dep, "arrival": fp.get("arrival") or "",
        "direction": not dep[:1] in ("E", "L", "B"), "etd": fp.get("deptime") or "", "selcal": selcal_from_remarks(fp.get("remarks")),
        "datalinkConnected": False, "isEquipped": True, "state": "", "relevant": True,
        "trackedBy": "", "trackedById": "", "targetMode": 3,
        "lastUpdated": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "source": "vatsim",
    }


def convert_manual_pilot(pilot, tracks):
    """A flight added by hand: the same record as the VATSIM feed gives, but for any pilot with a flight plan.
    If the route does not have two oceanic coordinates the plain fixes of the route are used and the direction is a guess
    from where the flight starts (Europe = westbound)."""
    rec = convert_network_pilot(pilot, tracks)
    if rec:
        return rec
    fp = pilot.get("flight_plan") or {}
    if not fp:
        return None
    return _rr_record(pilot, fp, tracks)


class NatTrakFeed:
    """natTrak clearances and pending requests (public endpoint, no login). Keeps the last good copy."""
    def __init__(self, url=NATTRAK_PLUGIN_URL):
        self.url = url
        self.lock = threading.Lock()
        self.items, self.updated, self.error = [], None, None

    def refresh(self, fetch=None):
        try:
            if fetch:
                data = fetch()
            else:
                req = urllib.request.Request(self.url, headers={"User-Agent": "vnaaats-server"})
                with urllib.request.urlopen(req, timeout=30) as r:
                    data = json.loads(r.read().decode("utf-8"))
            if not isinstance(data, list):
                raise ValueError("unexpected natTrak response")
            with self.lock:
                self.items, self.updated, self.error = data, time.time(), None
            log.info("natTrak refreshed: %d messages", len(data))
        except Exception as e:
            with self.lock:
                self.error = str(e)
            log.error("natTrak refresh failed: %s", e)

    def loop(self):
        while True:
            self.refresh()
            time.sleep(FEED_SECONDS)

    def all(self):
        with self.lock:
            return list(self.items)

    def by_callsign(self):
        out = {}
        for it in self.all():                              # later entries (newest) win
            if it.get("callsign"):
                out[it["callsign"].upper()] = it
        return out


class NetworkTraffic:
    """Keeps the oceanic-bound flights from the public VATSIM data feed (flight plan data only)."""
    def __init__(self, cache, url=VATSIM_FEED, natrak=None):
        self.cache, self.url, self.natrak = cache, url, natrak
        self.lock = threading.Lock()
        self.flights, self.updated, self.error = [], None, None
        self.positions = {}      # callsign -> (lat, lon) for every pilot on the network (used by the CPDLC 1000 nm logon rule)
        self.controllers = []    # online ATC that aircraft can be transferred to: [{callsign, frequency, name, facility}]
        self.atc_freq = {}       # "133.500" -> callsign of every online controller (any position)
        self.pilots = {}         # callsign -> the pilot as in the VATSIM feed (for adding a flight by hand)
        self.tuned = {}          # pilot callsign -> set of frequencies (MHz string) the pilot has tuned, from the transceivers feed
        self.atc_all = {}        # "121.900" -> [callsigns of every online controller on it]: frequencies are NOT unique worldwide
        self.atc_range = {}      # controller callsign -> visual range in nm (from the feed)
        self.atc_calls = set()   # callsign of every controller online (any position, not ATIS)
        self.online_cids = set() # CID of everyone connected right now (pilots, controllers, observers)
        self.tx_pos = {}         # callsign -> [(lat, lon)] of its transceivers (from the transceivers feed)

    def _in_range(self, pilot, controller):
        """Could this pilot hear this controller? Unknown positions give True, so a feed without them behaves as before."""
        pp = self.positions.get(pilot)
        sites = self.tx_pos.get(controller)
        if not pp or not sites:
            return True
        reach = max(float(self.atc_range.get(controller) or 0), 60.0)       # at least ~60 nm, else the controller's own visual range
        return min(_gc_nm(pp[0], pp[1], s[0], s[1]) for s in sites) <= reach

    def is_offline(self, callsign):
        """True only when the feed is healthy (recent, has pilots) and this callsign is not in it. None if we cannot tell."""
        with self.lock:
            if not self.positions or not self.updated or time.time() - self.updated > 600:
                return None
            return (callsign or "").upper() not in self.positions

    def cid_online(self, cid):
        """True/False: is this VATSIM CID connected to the network right now? None when the feed is not healthy (cannot tell)."""
        with self.lock:
            if not self.online_cids or not self.updated or time.time() - self.updated > 600:
                return None
            return re.sub(r"\D", "", str(cid)) in self.online_cids

    def atc_online(self, callsign):
        """True/False: is this controller callsign online on the network right now? None when the feed is not healthy (cannot tell)."""
        with self.lock:
            if not self.atc_calls or not self.updated or time.time() - self.updated > 600:
                return None
            return (callsign or "").upper() in self.atc_calls

    def position(self, callsign):
        with self.lock:
            return self.positions.get((callsign or "").upper())

    def lookup_pilot(self, callsign):
        with self.lock:
            return self.pilots.get((callsign or "").upper())

    def comm_for(self, callsign):
        """Who the pilot is talking to: {"kind": "oceanic"|"domestic"|"unicom", "by": callsign or "", "freq": "133.500"} or None.
        Worked out from the frequencies the pilot has tuned (transceivers feed) matched to the online controllers."""
        with self.lock:
            freqs = self.tuned.get((callsign or "").upper())
            if not freqs:
                return None
            me = (callsign or "").upper()
            for f in sorted(freqs):
                cands = self.atc_all.get(f) or ([self.atc_freq[f]] if f in self.atc_freq else [])
                by = next((c for c in cands if self._in_range(me, c)), None)       # same frequency on the other side of the world is not "talking to"
                if by:
                    return {"kind": "oceanic" if by.startswith(OCEANIC_PREFIXES) else "domestic", "by": by, "freq": f}
            if "122.800" in freqs:
                return {"kind": "unicom", "by": "UNICOM", "freq": "122.800"}
            if (callsign or "").upper() in self.positions:      # online and tuned to nobody we know: not tracked by anyone
                return {"kind": "none", "by": "", "freq": ""}
        return None

    def refresh_transceivers(self, fetch=None):
        try:
            if fetch:
                data = fetch()
            else:
                req = urllib.request.Request(VATSIM_TRANSCEIVERS, headers={"User-Agent": "vnaaats-server"})
                with urllib.request.urlopen(req, timeout=30) as r:
                    data = json.loads(r.read().decode("utf-8"))
            tuned = {}
            sites = {}
            for row in data:
                cs = str(row.get("callsign", "")).upper()
                for t in row.get("transceivers", []):
                    try:
                        tuned.setdefault(cs, set()).add("%.3f" % (int(t["frequency"]) / 1e6))
                    except (KeyError, TypeError, ValueError):
                        pass
                    try:
                        sites.setdefault(cs, []).append((float(t["latDeg"]), float(t["lonDeg"])))
                    except (KeyError, TypeError, ValueError):
                        pass
            with self.lock:
                self.tuned = tuned
                self.tx_pos = sites
        except Exception as e:
            log.error("Transceivers refresh failed: %s", e)

    def refresh(self, fetch=None):
        try:
            if fetch:
                data = fetch()
            else:
                req = urllib.request.Request(self.url, headers={"User-Agent": "vnaaats-server"})
                with urllib.request.urlopen(req, timeout=30) as r:
                    data = json.loads(r.read().decode("utf-8"))
            with self.cache.lock:
                tracks = list(self.cache.tracks)
            out = []
            for pilot in data.get("pilots", []):
                rec = convert_network_pilot(pilot, tracks)
                if rec and rec["callsign"]:
                    out.append(rec)
            pos = {}
            raw = {}
            for pilot in data.get("pilots", []):
                try:
                    raw[str(pilot["callsign"]).upper()] = pilot
                except (KeyError, TypeError):
                    pass
            for pilot in data.get("pilots", []):
                try:
                    pos[str(pilot["callsign"]).upper()] = (float(pilot["latitude"]), float(pilot["longitude"]))
                except (KeyError, TypeError, ValueError):
                    pass
            ctrls = []
            atc = {}
            atc_all = {}
            atc_range = {}
            calls = set()
            for c in data.get("controllers", []):
                try:
                    if not str(c["callsign"]).upper().endswith("_ATIS"):
                        calls.add(str(c["callsign"]).upper())
                except (KeyError, TypeError):
                    pass
                try:
                    cs, fr = str(c["callsign"]).upper(), "%.3f" % float(c.get("frequency", 0))
                    if int(c.get("facility", 0)) >= 1 and not cs.endswith("_ATIS") and fr != "199.998":
                        atc[fr] = cs
                        atc_all.setdefault(fr, []).append(cs)
                        atc_range[cs] = float(c.get("visual_range") or 0)
                except (KeyError, TypeError, ValueError):
                    pass
                try:
                    if int(c.get("facility", 0)) in (1, 5, 6) and not str(c["callsign"]).upper().endswith("_ATIS"):
                        ctrls.append({"callsign": str(c["callsign"]).upper(), "frequency": str(c.get("frequency", "")),
                                      "name": str(c.get("name", "")), "facility": int(c["facility"])})
                except (KeyError, TypeError, ValueError):
                    pass
            cids = set()
            for who in list(data.get("pilots", [])) + list(data.get("controllers", [])):
                try:
                    cids.add(str(int(who["cid"])))
                except (KeyError, TypeError, ValueError):
                    pass
            with self.lock:
                self.online_cids = cids
                self.flights, self.updated, self.error = out, time.time(), None
                self.positions = pos
                self.pilots = raw
                self.atc_freq = atc
                self.atc_all, self.atc_range = atc_all, atc_range
                self.atc_calls = calls
                self.controllers = sorted(ctrls, key=lambda x: x["callsign"])
            log.info("Network traffic refreshed: %d oceanic flights", len(out))
            self.refresh_transceivers()
        except Exception as e:
            with self.lock:
                self.error = str(e)
            log.error("Network traffic refresh failed: %s", e)

    def loop(self):
        while True:
            self.refresh()
            time.sleep(FEED_SECONDS)

    def all(self):
        with self.lock:
            flights = [dict(f) for f in self.flights]
        with self.lock:
            pilots = self.pilots
        with self.cache.lock:
            fixes = dict(self.cache.fixes)
        for f in flights:                                  # no plugin times yet: work them out from the pilot's position and speed
            if all(x == "--" for x in str(f.get("routeEtas", "")).split()):
                est = estimate_etas(f.get("route", ""), pilots.get(f["callsign"]) or {}, fixes)
                if est:
                    f["routeEtas"] = " ".join(est)
                    f["etaCalc"] = True
        if not self.natrak:
            return flights
        clearances = self.natrak.by_callsign()
        with self.cache.lock:
            track_ids = {t["id"] for t in self.cache.tracks}
        for f in flights:                                  # fill in what the controller cleared on natTrak
            c = clearances.get(f["callsign"])
            if not c or c.get("status") != "CLEARED":
                continue
            if str(c.get("level", "")).isdigit():
                f["assignedLevel"] = int(c["level"])
            try:
                f["assignedMach"] = int(round(float(c["mach"]) * 100))
            except (TypeError, ValueError):
                pass
            if c.get("nat") in track_ids:
                f["track"] = c["nat"]
            f["trackedBy"] = "NATTRAK"
        return flights


# ----------------------------------------------------------------------------- login
class Auth:
    """Who may edit flights from the web. VATSIM Connect and/or a shared password. Sessions live in memory (a restart logs everyone out)."""

    def __init__(self, env=os.environ):
        self.password = env.get("VNAAATS_FDD_PASSWORD", "")
        if env.get("VNAAATS_SHARED_PASSWORD", "on").lower() == "off":
            self.password = ""            # shared password switched off: only personal passwords (and VATSIM Connect) work
        self.client_id = env.get("VNAAATS_CONNECT_CLIENT_ID", "")
        self.client_secret = env.get("VNAAATS_CONNECT_CLIENT_SECRET", "")
        self.public_url = env.get("VNAAATS_PUBLIC_URL", "").rstrip("/")
        self.connect_base = env.get("VNAAATS_CONNECT_BASE", "https://auth.vatsim.net").rstrip("/")
        self.allowed_cids = {c.strip() for c in env.get("VNAAATS_ALLOWED_CIDS", "").split(",") if c.strip()}
        self.min_rating = int(env.get("VNAAATS_MIN_RATING", "2"))
        self.positions = [p.strip().upper() for p in env.get("VNAAATS_POSITIONS", DEFAULT_POSITIONS).split(",") if p.strip()]
        self.lock = threading.Lock()
        self.sessions = {}        # token -> {cid, name, via, position, expires}
        self.states = {}          # oauth state -> expires
        self.failures = {}        # ip -> [timestamps of failed password attempts]

    @property
    def connect_enabled(self):
        return bool(self.client_id and self.client_secret and self.public_url)

    @property
    def enabled(self):
        return bool(self.password) or self.connect_enabled

    def _clean(self):
        now = time.time()
        for t in [t for t, s in self.sessions.items() if s["expires"] < now]:
            del self.sessions[t]
        for t in [t for t, e in self.states.items() if e < now]:
            del self.states[t]

    def names_at(self, position):
        """Lower-case names of the people signed in on the FDD at this position right now."""
        position = (position or "").strip().upper()
        now = time.time()
        with self.lock:
            return {s["name"].strip().lower() for s in self.sessions.values() if s["position"] == position and s["expires"] > now and s.get("name")}

    def create_session(self, cid, name, via):
        token = secrets.token_urlsafe(32)
        with self.lock:
            self._clean()
            self.sessions[token] = {"cid": cid, "name": name, "via": via, "position": "", "expires": time.time() + SESSION_SECONDS}
        return token

    def get(self, token):
        with self.lock:
            s = self.sessions.get(token or "")
            return dict(s) if s and s["expires"] > time.time() else None

    def set_position(self, token, position):
        position = (position or "").strip().upper()
        if position == "":                                   # "change position": go back to the position screen
            with self.lock:
                s = self.sessions.get(token or "")
                if s:
                    s["position"] = ""
                return bool(s)
        if not re.fullmatch(r"[A-Z0-9_]{3,16}", position):
            return False
        with self.lock:
            s = self.sessions.get(token or "")
            if not s:
                return False
            s["position"] = position
            return True

    def logout(self, token):
        with self.lock:
            self.sessions.pop(token or "", None)

    def throttled(self, ip):
        now = time.time()
        with self.lock:
            fails = [t for t in self.failures.get(ip, []) if now - t < 300]
            self.failures[ip] = fails
            return len(fails) >= 5

    def record_failure(self, ip):
        with self.lock:
            self.failures.setdefault(ip, []).append(time.time())

    def check_password(self, given):
        return bool(self.password) and hmac.compare_digest(given.encode(), self.password.encode())

    def new_state(self):
        state = secrets.token_urlsafe(16)
        with self.lock:
            self._clean()
            self.states[state] = time.time() + 600
        return state

    def use_state(self, state):
        with self.lock:
            return self.states.pop(state, 0) > time.time()

    def authorize_url(self):
        return self.connect_base + "/oauth/authorize?" + urllib.parse.urlencode({
            "client_id": self.client_id, "redirect_uri": self.public_url + "/auth/callback",
            "response_type": "code", "scope": "full_name vatsim_details", "state": self.new_state()})

    def exchange_code(self, code, http=None):
        """Trade the code VATSIM sent back for the member's details. Returns (cid, name) or raises ValueError."""
        http = http or _http_json
        tok = http(self.connect_base + "/oauth/token", {
            "grant_type": "authorization_code", "client_id": self.client_id, "client_secret": self.client_secret,
            "redirect_uri": self.public_url + "/auth/callback", "code": code})
        user = http(self.connect_base + "/api/user", None, tok.get("access_token", "")).get("data", {})
        cid = str(user.get("cid", ""))
        name = (user.get("personal") or {}).get("name_full", "") or cid
        try:
            rating = int(((user.get("vatsim") or {}).get("rating") or {}).get("id", -1))
        except (TypeError, ValueError):
            rating = -1
        if not cid:
            raise ValueError("VATSIM did not return a CID")
        if cid not in self.allowed_cids and (self.allowed_cids or rating < self.min_rating):
            raise ValueError("This VATSIM account is not allowed to edit (CID %s)" % cid)
        return cid, name


def _http_json(url, form=None, bearer=None):
    req = urllib.request.Request(url, data=urllib.parse.urlencode(form).encode() if form else None)
    req.add_header("Accept", "application/json")
    if bearer:
        req.add_header("Authorization", "Bearer " + bearer)
    with urllib.request.urlopen(req, timeout=15) as r:
        return json.loads(r.read().decode("utf-8"))


LATLON_OK = re.compile(r"\d{2}/\d{2}")                 # 51/20 = 51N 020W
NAMED_OK = re.compile(r"[A-Z]{3,5}")                      # named waypoint, e.g. SOMAX


def route_problem(text):
    """Why a route text is not accepted ("" if fine). Latitude/longitude fixes must be two numbers / two numbers (51/20)."""
    fixes = [f for f in str(text).upper().split() if f != "AIRCRAFT"]   # AIRCRAFT = the plugin's "present position" placeholder, not a fix
    if not fixes:
        return "type the route fixes separated by spaces, for example 52/50 52/40 52/30"
    if len(fixes) > 40:
        return "too many fixes"
    for f in fixes:
        if LATLON_OK.fullmatch(f) or NAMED_OK.fullmatch(f):
            continue
        if re.search(r"\d", f):
            return "%s is not accepted: write latitude/longitude as two numbers / two numbers, for example 51/20 (not 51N20W)" % f
        return "%s is not a fix name (3-5 letters, or two numbers / two numbers like 51/20)" % f
    return ""


EU_PREFIX, NA_PREFIX = ("E", "L"), ("K", "C")


def route_sanity(rec):
    """("", route problem) for a flight from the VATSIM feed. A route is trusted when it crosses the ocean the way the flight should:
    Europe -> North America runs westbound, North America -> Europe eastbound. A route that runs the wrong way, or a flight
    with both ends in Europe (or both in North America), is "wrong or invalid" and the controller is asked to check it."""
    dep, arr = str(rec.get("departure", "")).upper(), str(rec.get("arrival", "")).upper()
    if not str(rec.get("route", "")).strip():
        return "no route"
    track = str(rec.get("track", "")).upper()
    if len(track) > 1 and track.startswith("T"):
        return ""                                                   # Tango route: not judged by direction
    def region(code):
        return "EU" if code.startswith(EU_PREFIX) else "NA" if code.startswith(NA_PREFIX) else ""
    d, a = region(dep), region(arr)
    if d and a:
        if d == a:
            return "%s to %s does not cross the Atlantic" % (dep, arr)
        east = bool(rec.get("direction"))
        if d == "EU" and east:
            return "%s to %s should run westbound but the route runs eastbound" % (dep, arr)
        if d == "NA" and not east:
            return "%s to %s should run eastbound but the route runs westbound" % (dep, arr)
    return ""


def _hhmm_to_min(t):
    return int(t[:2]) * 60 + int(t[2:])


def shift_entry_eta(etas, new_entry, n_fixes):
    """Move the oceanic entry time to new_entry (HHMM) and shift every other fix time by the same amount.
    Times run in order along the flight, so the entry is the earliest of them (whichever way round the list is stored).
    A flight with no times yet gets the entry time on its first fix and nothing else."""
    parts = etas.split()
    valid = [i for i, t in enumerate(parts) if re.fullmatch(r"\d{4}", t)]
    if not valid:
        return " ".join([new_entry] + ["--"] * max(n_fixes - 1, 0))
    # unwrap midnight so the times form one run, then find the earliest
    mins, prev, day = {}, None, 0
    for i in valid:
        m = _hhmm_to_min(parts[i])
        if prev is not None and m + day * 1440 < prev - 720:
            day += 1
        elif prev is not None and m + day * 1440 > prev + 720:
            day -= 1
        mins[i] = m + day * 1440
        prev = mins[i]
    delta = _hhmm_to_min(new_entry) - (min(mins.values()) % 1440)
    if delta > 720:
        delta -= 1440
    elif delta < -720:
        delta += 1440
    out = list(parts)
    for i in valid:
        t = (mins[i] + delta) % 1440
        out[i] = "%02d%02d" % (t // 60, t % 60)
    return " ".join(out)


def validate_edit(body):
    """Check an edit from the web. Returns (changes, error)."""
    changes = {}
    if "level" in body:
        lv = to_int(body["level"], -1)
        if not 1 <= lv <= 999:
            return None, "level must be 1-999 (e.g. 350)"
        changes["assignedLevel"] = lv
    if "mach" in body:
        m = to_int(str(body["mach"]).lstrip("Mm.").lstrip("0") or "0", -1)
        if not 1 <= m <= 299:
            return None, "mach must be e.g. 84 for M0.84"
        changes["assignedMach"] = m
    if "track" in body:
        t = str(body["track"]).strip().upper()
        if not re.fullmatch(r"[A-Z]{1,3}", t):
            return None, "track must be 1-3 letters (or RR)"
        changes["track"] = t
    if "route" in body:
        bad = route_problem(body["route"])
        if bad:
            return None, bad
        changes["route"] = " ".join(str(body["route"]).upper().split())
    if "entryEta" in body:
        e = re.sub(r"\D", "", str(body["entryEta"]))
        if len(e) != 4 or int(e[:2]) > 23 or int(e[2:]) > 59:
            return None, "entry time must be four digits, 0000-2359 (e.g. 1530)"
        changes["entryEta"] = e
    if "selcal" in body:
        sc = re.sub(r"[-\s]", "", str(body["selcal"]).upper())
        if sc and not SELCAL_OK.fullmatch(sc):
            return None, "SELCAL must be exactly 4 letters, like ABCD"
        changes["selcal"] = sc
    if "scratch" in body:
        sp = " ".join(str(body["scratch"]).upper().split())
        if not SCRATCH_OK.fullmatch(sp):
            return None, "the note can be up to %d letters, numbers, spaces and / . , + - *" % SCRATCH_MAX
        changes["scratch"] = sp
    if not changes:
        return None, "nothing to change"
    return changes, None


# ----------------------------------------------------------------------------- http
MIME = {".html": "text/html", ".js": "application/javascript", ".css": "text/css", ".json": "application/json",
        ".png": "image/png", ".jpg": "image/jpeg", ".svg": "image/svg+xml", ".ico": "image/x-icon",
        ".webmanifest": "application/manifest+json", ".woff": "font/woff", ".ttf": "font/ttf", ".woff2": "font/woff2", ".txt": "text/plain", ".map": "application/json", ".mp4": "video/mp4", ".pdf": "application/pdf", ".zip": "application/zip"}


def rcl_enrich(reqs, natrak, store, network):
    """Adds to each CPDLC clearance request what the controller needs to enter it on natTrak: the flight's plan data and whether natTrak has it."""
    out = []
    nt = natrak.by_callsign() if natrak else {}
    for r in reqs or []:
        cs = r["callsign"].upper()
        rec = (store.get(cs) if store else None) or {}
        if not rec and network:
            rec = next((f for f in network.all() if str(f.get("callsign", "")).upper() == cs), {}) or {}
        n = nt.get(cs)
        issued = None
        if n and n.get("clearance_issued"):
            try:
                issued = datetime.fromisoformat(str(n["clearance_issued"]).replace("Z", "+00:00")).timestamp()
            except ValueError:
                issued = None
        item = dict(r)
        asked = cpdlc_mod.parse_rcl_text(r.get("text", ""))          # the level and Mach the pilot asked for in the request itself
        _fx, _tm, _rt = clx_mod._entry(rec.get("route", ""), rec.get("routeEtas", ""), str((n or {}).get("fix") or "").upper())
        item.update({"ntDown": bool(natrak and getattr(natrak, "error", None)), "entryFix": _fx, "entryTime": _tm, "dest": rec.get("arrival", ""), "route": rec.get("route", ""), "etas": rec.get("routeEtas", ""),
                     "level": asked["level"] or rec.get("assignedLevel"), "mach": asked["mach"] or rec.get("assignedMach"), "track": rec.get("track", ""),
                     "natTrak": ({"status": n.get("status", ""), "level": n.get("level"), "mach": n.get("mach"), "nat": n.get("nat"),
                                  "fix": n.get("fix"), "issued": issued} if n else None)})
        out.append(item)
    return out


def make_handler(cache, store, api_key, web_dir=None, network=None, natrak=None, auth=None, cpdlc=None, queue=None, rcl=None, members=None, issuer_secret="", roster=None, logs=None):
    logs = logs if logs is not None else adminlog_mod.LogHub()
    queue = queue if queue is not None else commands_mod.CommandQueue()
    auth = auth or Auth({})

    def with_cpdlc(rec):
        """Add "cpdlc": true/false (connected to our CPDLC station) to a flight record."""
        rec = dict(rec)
        by = str(rec.get("trackedBy", "")).upper()
        if by and by != "NATTRAK" and network and hasattr(network, "atc_online") and network.atc_online(by) is False:
            rec["trackedBy"], rec["trackedById"] = "", ""                    # that controller has logged off: nobody is tracking it now
        if str(rec.get("track", "")).upper() in ("", "RR"):                 # a flight on a Tango route is grouped under it
            tg = tango_match(str(rec.get("route", "")).split())
            if tg:
                rec["track"] = tg[0]
        rec["cpdlc"] = bool(cpdlc and cpdlc.is_connected(rec.get("callsign", "")))
        rec["cpdlcRelease"] = bool(cpdlc and cpdlc.released(rec.get("callsign", "")))   # tell the plugin to release the tag (after UNICOM)
        sent = rec.pop("ntSent", None)
        rec["ntSentInfo"] = sent
        rec["ntSent"] = bool(sent) and not nt_stops_reminder(rec, natrak)
        rec["offline"] = network.is_offline(rec.get("callsign", "")) if network else None
        rec["comm"] = network.comm_for(rec.get("callsign", "")) if network and hasattr(network, "comm_for") else None
        rec["distNm"] = route_distance_nm(rec.get("route", ""), network.position(rec.get("callsign", "")) if network else None, cache.fixes)
        if is_conc(rec.get("type", "")):
            rec["posReports"] = store.pos_get(rec.get("callsign", ""))
        return rec
    secure_cookie = auth.public_url.startswith("https://")
    shared_key_ok = os.environ.get("VNAAATS_SHARED_KEY", "on").lower() != "off"
    es_pick = {}                      # controller callsign -> {"callsign", "seq", "ts"}: the aircraft last clicked in that EuroScope
    es_pick_lock = threading.Lock()
    tracked_now = {}                  # controller callsign -> {"list": [callsigns], "ts"}: what that EuroScope was tracking last time it told us
    RETRACK_WINDOW = 300              # seconds: after a crash the controller gets the aircraft back if they return within this time
    plugin_seen = {}                  # controller callsign -> last time its plugin asked for commands (so we know it is alive)

    def key_problem(code):
        """"" when the plugin key is good, else why not: "unknown", "expired" or "revoked"."""
        code = code or ""
        if api_key and shared_key_ok and hmac.compare_digest(code.encode(), api_key.encode()):
            return ""
        if members:
            cid, why = members.check_key(code)
            return "" if cid else why
        return "unknown"

    usettings = usersettings_mod.UserSettings(store)
    plugin_files = pluginfiles_mod.PluginFiles(os.environ.get("VNAAATS_REPO", "garythms/vatsim-NAAATS-v2"))

    def issuer_ok(handler):
        sec = handler.headers.get("X-Issuer-Secret", "")
        return bool(issuer_secret) and len(issuer_secret) >= 24 and hmac.compare_digest(sec.encode(), issuer_secret.encode())

    class Handler(BaseHTTPRequestHandler):
        def handle_one_request(self):
            t0 = time.time()
            super().handle_one_request()
            d = time.time() - t0
            if d > 0.5 and getattr(self, "command", None):                 # slow requests are logged (path only, never the query with the key)
                log.warning("slow request %.2fs %s %s", d, self.command, (self.path or "").split("?")[0][:80])

        server_version = "vnaaats-server"

        def log_message(self, fmt, *args):
            log.info("%s %s", self.address_string(), re.sub(r"(?i)\b(code|key|logon|token|password)=[^&\s\"']*", r"\1=***", fmt % args))   # keys never go to the journal

        def _send(self, code, body, ctype="application/json"):
            data = body if isinstance(body, bytes) else (body if isinstance(body, str) else json.dumps(body)).encode("utf-8")
            self.send_response(code)
            self.send_header("Content-Type", ctype + "; charset=utf-8")
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("Access-Control-Allow-Origin", "*")      # the flight data display runs in a browser
            self.end_headers()
            self.wfile.write(data)

        def do_OPTIONS(self):
            self.send_response(204)
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "*")
            self.end_headers()

        def _client_ip(self):
            fwd = self.headers.get("X-Forwarded-For", "")
            return fwd.split(",")[-1].strip() if fwd else self.client_address[0]

        def _token(self):
            c = http.cookies.SimpleCookie(self.headers.get("Cookie", ""))
            return c[SESSION_COOKIE].value if SESSION_COOKIE in c else ""

        def _session(self):
            return auth.get(self._token())

        def _set_cookie(self, token, max_age=SESSION_SECONDS):
            return "%s=%s; Path=/; HttpOnly; SameSite=Lax; Max-Age=%d%s" % (SESSION_COOKIE, token, max_age, "; Secure" if secure_cookie else "")

        def _send_cookie(self, code, body, token, max_age=SESSION_SECONDS):
            data = json.dumps(body).encode("utf-8")
            self.send_response(code)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("Set-Cookie", self._set_cookie(token, max_age))
            self.end_headers()
            self.wfile.write(data)

        def _redirect(self, where, token=None):
            self.send_response(302)
            self.send_header("Location", where)
            self.send_header("Cache-Control", "no-store")
            if token:
                self.send_header("Set-Cookie", self._set_cookie(token))
            self.end_headers()

        def _read_ok(self, q):
            """Flight data is only for signed-in controllers (data protection). Open only on a server where nobody can sign in at all.
            The plugin may also read it with its key."""
            if not (auth.enabled or (members and members.any())):
                return True
            sess = self._session()
            if sess and sess["position"] and sess["position"] != "OBSERVER":
                return True
            code = q.get("code", "")
            return bool(code) and key_problem(code) == ""

        def _status(self, session):
            return {"editing": auth.enabled or bool(members and members.any()), "password": bool(auth.password) or bool(members and members.any()), "vatsim": auth.connect_enabled,
                    "loggedIn": bool(session), "name": session["name"] if session else "", "cid": session["cid"] if session else "",
                    "position": session["position"] if session else "", "positions": [x for x in auth.positions if x != "OBSERVER"]}

        def do_POST(self):
            p = urlparse(self.path).path.rstrip("/") or "/"
            q_post = {k: v[0] for k, v in parse_qs(urlparse(self.path).query, keep_blank_values=True).items()}
            try:
                if "application/json" not in self.headers.get("Content-Type", ""):
                    return self._send(415, {"error": "send JSON"})          # also blocks cross-site form posts
                n = int(self.headers.get("Content-Length", "0") or 0)
                if n > 20000:
                    return self._send(413, {"error": "too large"})
                try:
                    body = json.loads(self.rfile.read(n).decode("utf-8") or "{}")
                    if not isinstance(body, dict):
                        raise ValueError
                except ValueError:
                    return self._send(400, {"error": "bad JSON"})
                if p == "/PluginLog":                          # the plugin sends a copy of its log (batched)
                    bad = key_problem(q_post.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    lines = body.get("lines")
                    if not isinstance(lines, list):
                        return self._send(400, {"error": "lines must be a list"})
                    return self._send(200, logs.add(q_post.get("me", ""), q_post.get("v", ""), [x for x in lines if isinstance(x, str)],
                                                    ip=self._client_ip(), prev=bool(body.get("prev"))))
                if p == "/admin/log/detail":
                    if not logs.token_ok(self.headers.get("X-Admin-Token", ""), self._client_ip()):
                        return self._send(403, {"error": "not allowed"})
                    return self._send(200, logs.set_detail(body.get("me", ""), bool(body.get("on"))))
                if p.startswith("/admin/member/"):               # called by the Gander Oceanic website, not by browsers
                    if not issuer_ok(self):
                        return self._send(403, {"error": "not allowed"})
                    if not members:
                        return self._send(404, {"error": "member keys are not switched on"})
                    what = p[len("/admin/member/"):]
                    try:
                        if what == "issue":
                            if roster and roster.eligible(body.get("cid", "")) is False:
                                return self._send(403, {"error": "this CID is not certified on the roster"})
                            out = members.issue(body.get("cid", ""), body.get("name", ""))
                            log.info("Issued a personal key for CID %s", out["cid"])
                            return self._send(200, out)
                        if what == "info":
                            return self._send(200, members.info(body.get("cid", "")))
                        if what == "revoke":
                            return self._send(200, {"revoked": members.revoke(body.get("cid", ""))})
                        if what == "sync":
                            cids = body.get("cids", [])
                            if not isinstance(cids, list) or not cids:
                                return self._send(400, {"error": "send the list of certified CIDs (not empty)"})
                            gone = members.sync(cids)
                            log.info("Roster sync revoked %d member key(s)", len(gone))
                            return self._send(200, {"revoked": gone})
                        if what == "list":
                            return self._send(200, members.listing())
                    except ValueError as e:
                        return self._send(400, {"error": str(e)})
                    return self._send(404, {"error": "unknown"})
                if p == "/auth/password":
                    ip = self._client_ip()
                    if not auth.password and not (members and members.any()):
                        return self._send(404, {"error": "password login is not set up"})
                    if auth.throttled(ip):
                        return self._send(429, {"error": "too many wrong passwords - wait 5 minutes"})
                    name = str(body.get("name", "")).strip()[:40]
                    if not name:
                        return self._send(400, {"error": "enter your name or CID"})
                    who = members.check_login(name, str(body.get("password", ""))) if members else None
                    if who:
                        token = auth.create_session(who["cid"], who["name"], "member")
                        return self._send_cookie(200, self._status(auth.get(token)), token)
                    if not auth.password or not auth.check_password(str(body.get("password", ""))):
                        auth.record_failure(ip)
                        return self._send(401, {"error": "wrong password"})
                    token = auth.create_session(re.sub(r"\D", "", name)[:8], name, "password")
                    return self._send_cookie(200, self._status(auth.get(token)), token)
                if p == "/auth/temp-key":                      # temporary: 90-day key for certified controllers the roster feed misses. Off unless VNAAATS_TEMP_PASSPHRASE is set.
                    temp_pass = os.environ.get("VNAAATS_TEMP_PASSPHRASE", "")
                    if len(temp_pass) < 10 or not members:
                        return self._send(404, {"error": "not switched on"})
                    ip = self._client_ip()
                    if auth.throttled(ip):
                        return self._send(429, {"error": "too many wrong tries - wait 5 minutes"})
                    if not hmac.compare_digest(str(body.get("passphrase", "")).encode(), temp_pass.encode()):
                        auth.record_failure(ip)
                        return self._send(401, {"error": "wrong passphrase"})
                    cid = re.sub(r"\D", "", str(body.get("cid", "")))
                    if not 6 <= len(cid) <= 9:
                        return self._send(400, {"error": "enter your VATSIM CID (numbers only)"})
                    if network:                                 # proof it is you: that CID must be connected to VATSIM right now (observer, pilot or controller)
                        on = network.cid_online(cid)
                        if on is None:
                            return self._send(503, {"error": "the VATSIM connection list cannot be read right now - try again in a few minutes"})
                        if not on:
                            auth.record_failure(ip)
                            return self._send(403, {"error": "CID " + cid + " is not connected to VATSIM. Connect with your own CID (as an observer or a pilot is fine), wait a minute, then try again."})
                    if sum(1 for m in members.listing() if m["manual"] and not m["revoked"] and not m["expired"]) >= 40 and not members.info(cid).get("exists"):
                        return self._send(429, {"error": "the temporary keys are used up - contact info@vnaaats.com"})
                    out = members.issue(cid, str(body.get("name", "")).strip()[:60] or cid, days=90, manual=True)
                    log.info("Temporary 90-day key issued for CID %s from %s", cid, ip)
                    return self._send(200, {"key": out["key"], "password": out["password"], "expires": out["expires"], "cid": out["cid"]})
                if p == "/auth/trainee-key":                   # a trusted instructor issues a 2-hour key + FDD password for a student. Off unless VNAAATS_INSTRUCTOR_PASSWORD is set (10+ characters).
                    inst_pass = os.environ.get("VNAAATS_INSTRUCTOR_PASSWORD", "")
                    if len(inst_pass) < 10 or not members:
                        return self._send(404, {"error": "not switched on"})
                    ip = self._client_ip()
                    if auth.throttled(ip):
                        return self._send(429, {"error": "too many wrong tries - wait 5 minutes"})
                    if not hmac.compare_digest(str(body.get("passphrase", "")).encode(), inst_pass.encode()):
                        auth.record_failure(ip)
                        return self._send(401, {"error": "wrong password"})
                    instructor = str(body.get("instructor", "")).strip()[:60]
                    if not instructor:
                        return self._send(400, {"error": "enter your own name or CID, so we know who issued the key"})
                    cid = re.sub(r"\D", "", str(body.get("cid", "")))
                    if not 6 <= len(cid) <= 9:
                        return self._send(400, {"error": "enter the student's VATSIM CID (numbers only)"})
                    cur = members.info(cid)
                    if cur.get("exists") and not cur.get("revoked") and cur.get("expires", 0) > time.time() + TRAINEE_KEY_HOURS * 3600 + 60:
                        return self._send(409, {"error": "that CID already has a longer-lived key: the student can use it, and a trainee key would replace it"})
                    out = members.issue(cid, str(body.get("name", "")).strip()[:60] or cid, days=TRAINEE_KEY_HOURS / 24.0, manual=True)
                    log.info("Trainee %d-hour key issued for CID %s by %s from %s", TRAINEE_KEY_HOURS, cid, instructor, ip)
                    return self._send(200, {"key": out["key"], "password": out["password"], "expires": out["expires"], "cid": out["cid"], "hours": TRAINEE_KEY_HOURS})
                if p == "/auth/logout":
                    if cpdlc:
                        cpdlc.stop(self._token())
                    auth.logout(self._token())
                    return self._send_cookie(200, self._status(None), "", 0)
                if p == "/auth/position":
                    if str(body.get("position", "")).strip().upper() == "OBSERVER":
                        return self._send(400, {"error": "view-only mode has been removed: choose a controlling position"})
                    if not self._session() or not auth.set_position(self._token(), str(body.get("position", ""))):
                        return self._send(400, {"error": "log in first and give a position like CZQO_CTR"})
                    if cpdlc:
                        cpdlc.stop(self._token())                  # a new position means a different CPDLC station
                    return self._send(200, self._status(self._session()))
                if p in ("/cpdlc/send", "/cpdlc/seen", "/cpdlc/logon", "/cpdlc/logoff", "/cpdlc/unicom", "/cpdlc/force-logout", "/cpdlc/disconnect", "/cpdlc/rcl-dismiss", "/cpdlc/rcl-preview", "/cpdlc/rcl-send"):
                    if not cpdlc:
                        return self._send(404, {"error": "CPDLC is not switched on"})
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position to use CPDLC"})
                    token = self._token()
                    if p == "/cpdlc/logoff":
                        cpdlc.stop(token)
                        return self._send(200, {"ok": True})
                    if p == "/cpdlc/force-logout":
                        # two deliberate steps: the callsign typed out in full AND the confirm flag the page sets after its warnings
                        typed = str(body.get("callsign", "")).strip().upper()
                        if body.get("confirm") is not True or str(body.get("understand", "")).strip().upper() != "LOG OUT":
                            return self._send(400, {"error": "confirm the warnings first (type LOG OUT)"})
                        secs, err = cpdlc.force_logout(typed, by="%s (%s)" % (sess["position"], sess["name"]))
                        if err:
                            return self._send(400, {"error": err})
                        return self._send(200, {"ok": True, "callsign": typed, "lockedSeconds": secs})
                    if p == "/cpdlc/logon":
                        station, err = cpdlc.start(token, sess["position"], str(body.get("code", "")))
                        if err:
                            return self._send(400, {"error": err})
                        return self._send(200, {"ok": True, "station": station})
                    st = cpdlc.get(token)
                    if not st:
                        return self._send(409, {"error": "enter your Hoppie code first to sign in to CPDLC"})
                    if p == "/cpdlc/seen":
                        st.mark_seen(str(body.get("callsign", "")))
                        return self._send(200, {"ok": True})
                    if p == "/cpdlc/rcl-dismiss":
                        try:
                            ok = st.rcl_dismiss(int(body.get("id", 0)))
                        except (TypeError, ValueError):
                            return self._send(400, {"error": "bad request id"})
                        return self._send(200, {"ok": ok})
                    if p in ("/cpdlc/rcl-preview", "/cpdlc/rcl-send"):
                        if not cpdlc_mod.RCL_SEND:
                            return self._send(403, {"error": "sending clearances from the display is not switched on yet"})
                        kind = str(body.get("kind", ""))
                        if kind not in ("clx", "processed"):
                            return self._send(400, {"error": "kind must be clx or processed"})
                        try:
                            req = st.rcl_get(int(body.get("id", 0)))
                        except (TypeError, ValueError):
                            req = None
                        if not req:
                            return self._send(404, {"error": "that request is no longer in the list"})
                        cs = req["callsign"].upper()
                        nt = (natrak.by_callsign() if natrak else {}).get(cs)
                        if natrak and not nt and not getattr(natrak, "error", None):
                            return self._send(409, {"error": "%s is not on natTrak yet. Enter and check it there first." % cs})
                        rec = (store.get(cs) if store else None) or {}
                        rec = dict(rec, callsign=cs)
                        msgs = clx_mod.build(kind, rec, nt, sess["position"])
                        if p == "/cpdlc/rcl-preview":
                            return self._send(200, {"messages": msgs, "callsign": cs, "natTrak": bool(nt), "telex": not st.is_connected(cs)})
                        given = body.get("messages")
                        if isinstance(given, list):
                            msgs = [str(m).strip() for m in given if str(m).strip()]
                        if not msgs:
                            return self._send(400, {"error": "nothing to send"})
                        by = "%s (%s)" % (sess["position"], sess["name"])
                        try:
                            viatelex = not st.is_connected(cs)      # e.g. the pilot asked by telex: answer the same way
                            for m in msgs:
                                if viatelex:
                                    st.send_telex(cs, m, by=by)
                                else:
                                    st.send(cs, m, "NE", None, by=by)
                        except (ValueError, RuntimeError) as e:
                            return self._send(400, {"error": str(e)})
                        st.rcl_mark_sent(req["id"], by, kind)
                        return self._send(200, {"ok": True, "sent": len(msgs), "telex": viatelex})
                    if p == "/cpdlc/disconnect":
                        # log one pilot off CPDLC from this station (they stay connected on VATSIM; they can log on again)
                        try:
                            return self._send(200, st.disconnect(str(body.get("callsign", "")), by="%s (%s)" % (sess["position"], sess["name"])))
                        except ValueError as e:
                            return self._send(400, {"error": str(e)})
                    if p == "/cpdlc/unicom":
                        try:
                            return self._send(200, st.unicom(str(body.get("callsign", "")), by="%s (%s)" % (sess["position"], sess["name"])))
                        except ValueError as e:
                            return self._send(400, {"error": str(e)})
                        except RuntimeError as e:
                            return self._send(502, {"error": str(e)})
                    try:
                        mrn = body.get("mrn")
                        msg = st.send(str(body.get("callsign", "")), str(body.get("text", "")), str(body.get("ra", "WU")),
                                      int(mrn) if mrn not in (None, "") else None, by="%s (%s)" % (sess["position"], sess["name"]))
                    except ValueError as e:
                        return self._send(400, {"error": str(e)})
                    except RuntimeError as e:
                        return self._send(502, {"error": str(e)})
                    return self._send(200, msg)
                if p == "/settings":
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if sess.get("via") not in ("member", "vatsim") or not re.fullmatch(r"\d{1,9}", str(sess.get("cid", ""))):
                        return self._send(403, {"error": "settings need your own sign-in (personal password or VATSIM Connect), not the shared password"})
                    rec, err = usettings.set(sess["cid"], body.get("settings"))
                    if err:
                        return self._send(400, {"error": err})
                    return self._send(200, rec)
                if p == "/relief/start":                       # the confirmed relief: hand every aircraft of this position to the next controller
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    pos = sess["position"]
                    if not pos or pos == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position first"})
                    target = str(body.get("target", "")).strip().upper()
                    if not commands_mod.CALLSIGN_OK.fullmatch(target) or target == pos.upper():
                        return self._send(400, {"error": "choose the controller who is taking over"})
                    if network:
                        with network.lock:
                            online = {c["callsign"].upper(): c for c in network.controllers}
                        if target not in online:
                            return self._send(400, {"error": target + " is not online"})
                        freq = online[target].get("frequency", "")
                    else:
                        freq = ""
                    asked = [re.sub(r"[^A-Z0-9]", "", str(x).upper())[:8] for x in (body.get("callsigns") or [])][:100]
                    with es_pick_lock:
                        cur = tracked_now.get(pos.upper())
                    mine = set(cur["list"]) if cur and time.time() - cur["ts"] <= RETRACK_WINDOW else set()
                    cs_list = [c for c in asked if c in mine]            # only aircraft this position really has
                    by = ("RELIEF %s (%s)" % (pos, sess["name"]))[:60]
                    ids, told = [], []
                    for c in cs_list:
                        ids.append({"callsign": c, "id": queue.add(pos, "transfer", c, {"target": target, "freq": freq if re.fullmatch(r"1[0-3]\d\.\d{1,3}", str(freq)) else ""}, by=by)})
                        if cpdlc and cpdlc.is_connected(c):
                            try:
                                st = cpdlc.station_with(c)
                                st.send(c, commands_mod.RELIEF_CPDLC_TEXT, ra="NE", by=by)
                                told.append(c)
                            except (ValueError, RuntimeError):
                                pass
                    log.info("Relief of %s by %s started by %s: %d aircraft, %d told on CPDLC", pos, target, sess["name"], len(cs_list), len(told))
                    return self._send(200, {"commands": ids, "cpdlcTold": told})
                if p == "/cmd/queue":
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position first"})
                    typ, cs, args, err = commands_mod.clean_command(body)
                    if err:
                        return self._send(400, {"error": err})
                    cid = queue.add(sess["position"], typ, cs, args, by=("%s (%s)" % (sess["position"], sess["name"]))[:60])
                    return self._send(200, {"id": cid})
                if p == "/rcl/auto":
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position first"})
                    if not rcl:
                        return self._send(404, {"error": "automatic RCL REQ is not available on this server"})
                    act = str(body.get("action", ""))
                    if act:                                              # the answer to "send the reminders now?"
                        who = rcl.sender()
                        if not who or sess["position"].upper() != who:
                            return self._send(403, {"error": "only %s can answer: their EuroScope types the messages" % (who or "the oceanic controller who is online")})
                        if act == "send":
                            rcl.send_now()
                        elif act == "later":
                            rcl.later()
                        elif act == "skip":
                            rcl.skip()
                        elif act == "stop":
                            rcl.stop()
                        else:
                            return self._send(400, {"error": "unknown action"})
                        log.info("Auto RCL REQ: %s answered %s", sess["position"], act)
                        return self._send(200, rcl.status())
                    if "confirm" in body:
                        rcl.set_confirm(bool(body["confirm"]))
                        log.info("Auto RCL REQ ask-first switched %s by %s", "ON" if rcl.confirm else "OFF", sess["position"])
                        return self._send(200, rcl.status())
                    rcl.set_enabled(bool(body.get("on", True)))
                    log.info("Auto RCL REQ switched %s by %s", "ON" if rcl.enabled else "OFF", sess["position"])
                    return self._send(200, rcl.status())
                if p == "/NatTrakSent":
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position first"})
                    cs = str(body.get("callsign", "")).strip().upper()
                    if not re.fullmatch(r"[A-Z0-9]{2,8}", cs):
                        return self._send(400, {"error": "bad callsign"})
                    base = None
                    if network and not store.get(cs):
                        base = next((f for f in network.all() if f.get("callsign") == cs), None)
                    rec = store.mark_nt(cs, ("%s (%s)" % (sess["position"], sess["name"]))[:60], bool(body.get("sent", True)), base)
                    return self._send(200, {"ok": True}) if rec else self._send(404, {"error": "unknown flight"})
                if p == "/EditPosReport":
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position to enter position reports"})
                    cs = str(body.get("callsign", "")).strip().upper()
                    fix = str(body.get("fix", "")).strip().upper()
                    if not re.fullmatch(r"[A-Z0-9/]{2,12}", fix):
                        return self._send(400, {"error": "bad fix name"})
                    est, act = clean_time(body.get("est", "")), clean_time(body.get("act", ""))
                    if est is None or act is None:
                        return self._send(400, {"error": "times are four digits, for example 1432"})
                    rec = store.get(cs) or (next((f for f in network.all() if f.get("callsign") == cs), None) if network else None)
                    if not rec:
                        return self._send(404, {"error": "unknown flight"})
                    if not is_conc(rec.get("type", "")):
                        return self._send(400, {"error": "position reports are only for Concorde"})
                    store.pos_set(cs, fix, est, act, ("%s (%s)" % (sess["position"], sess["name"]))[:60])
                    return self._send(200, with_cpdlc(rec))
                if p in ("/AddFlight", "/RemoveFlight"):
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position first (observers are read-only)"})
                    cs = re.sub(r"[^A-Z0-9]", "", str(body.get("callsign", "")).upper())
                    if not cs:
                        return self._send(400, {"error": "type a callsign"})
                    by = ("%s (%s)" % (sess["position"], sess["name"]))[:60]
                    if p == "/RemoveFlight":
                        store.dismiss(cs, by)
                        return self._send(200, {"ok": True, "callsign": cs})
                    pilot = network.lookup_pilot(cs) if network else None
                    if not pilot:
                        return self._send(404, {"error": cs + " is not on the VATSIM network right now - check the callsign"})
                    with cache.lock:
                        tracks_now = list(cache.tracks)
                    rec = convert_manual_pilot(pilot, tracks_now)
                    if not rec:
                        return self._send(404, {"error": cs + " has no flight plan filed on VATSIM"})
                    return self._send(200, with_cpdlc(store.add_manual(rec, by)))
                if p == "/EditFlight":
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if not sess["position"] or sess["position"] == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position to edit (observers are read-only)"})
                    changes, err = validate_edit(body)
                    if err:
                        return self._send(400, {"error": err})
                    cs = str(body.get("callsign", "")).strip().upper()
                    base = None
                    if network and not store.get(cs):
                        base = next((f for f in network.all() if f.get("callsign") == cs), None)
                    by = "%s (%s)" % (sess["position"], sess["name"])
                    rec = store.edit(cs, changes, by[:60], base)
                    return self._send(200, rec) if rec else self._send(404, {"error": "unknown flight"})
                return self._send(404, {"error": "unknown endpoint"})
            except Exception as e:
                log.exception("request failed")
                return self._send(500, {"error": str(e)})

        def _static(self, path):
            """Serve the built web app (flight data display) if a web folder is configured."""
            if not web_dir:
                return False
            rel = path.lstrip("/") or "index.html"
            full = os.path.realpath(os.path.join(web_dir, rel))
            root = os.path.realpath(web_dir)
            if not (full == root or full.startswith(root + os.sep)):
                return False
            if os.path.isdir(full):
                full = os.path.join(full, "index.html")
            if not os.path.isfile(full):
                alt = full + ".html"                                    # /fdd -> fdd.html
                if os.path.isfile(alt):
                    full = alt
                else:
                    return False
            with open(full, "rb") as f:
                body = f.read()
            ext = os.path.splitext(full)[1].lower()
            if ext in (".mp4", ".pdf", ".zip"):                               # media: phones ask for byte ranges before they will play a video
                return self._send_media(body, MIME[ext])
            self._send(200, body, MIME.get(ext, "application/octet-stream"))
            return True

        def _send_media(self, body, ctype):
            size = len(body)
            m = re.fullmatch(r"bytes=(\d*)-(\d*)", self.headers.get("Range", "").strip())
            start, end, code = 0, size - 1, 200
            if m and (m.group(1) or m.group(2)):
                if m.group(1):
                    start = int(m.group(1))
                    end = min(int(m.group(2)), size - 1) if m.group(2) else size - 1
                else:
                    start, end = max(0, size - int(m.group(2))), size - 1
                if start > end or start >= size:
                    self.send_response(416)
                    self.send_header("Content-Range", "bytes */%d" % size)
                    self.send_header("Content-Length", "0")
                    self.end_headers()
                    return True
                code = 206
            part = body[start:end + 1]
            self.send_response(code)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(part)))
            self.send_header("Accept-Ranges", "bytes")
            self.send_header("Cache-Control", "public, max-age=3600")
            if code == 206:
                self.send_header("Content-Range", "bytes %d-%d/%d" % (start, end, size))
            self.end_headers()
            self.wfile.write(part)
            return True

        def do_GET(self):
            u = urlparse(self.path)
            q = {k: v[0] for k, v in parse_qs(u.query, keep_blank_values=True).items()}
            p = u.path.rstrip("/") or "/"
            try:
                if p == "/admin/log":                          # the page itself holds no data; the token unlocks it
                    return self._send(200, adminlog_mod.PAGE, "text/html")
                if p == "/admin/log/data":
                    if not logs.token_ok(self.headers.get("X-Admin-Token", ""), self._client_ip()):
                        return self._send(403, {"error": "not allowed"})
                    return self._send(200, logs.view(q.get("me", ""), q.get("level", ""), to_int(q.get("since"), 0), to_int(q.get("limit"), 400)))
                if p == "/get/fdd-windows":                    # the FDD for Windows installer: one stable address on this site
                    return self._redirect("https://github.com/%s/releases/download/fdd-windows/FDD-for-Windows-Setup.exe" % os.environ.get("VNAAATS_REPO", "garythms/vatsim-NAAATS-v2"))
                if p == "/GetTrackSource":
                    return self._send(200, "0", "text/plain")
                if p == "/GetAllNatTracks":
                    with cache.lock:
                        out = list(cache.tracks)
                    if q.get("fdd") == "1":                  # the web display also lists the Concorde tracks; EuroScope never gets them
                        out += concorde_tracks() + tango_tracks()
                    return self._send(200, out)
                if p == "/cmd/status":
                    if not self._session():
                        return self._send(401, {"error": "log in first"})
                    st = queue.status(to_int(q.get("id"), 0))
                    return self._send(200, st) if st else self._send(404, {"error": "unknown command"})
                if p in ("/plugin/latest.json", "/plugin/VatsimNAAATS.dll"):    # the latest released plugin, for the vNAAATS app (public: it is on GitHub too)
                    info, blob = plugin_files.latest()
                    if not info:
                        return self._send(503, {"error": "the latest plugin is not available right now"})
                    if p.endswith(".json"):
                        out = dict(info)
                        try:                                  # the airline list the website serves (the app installs it next to the DLL)
                            with open(os.path.join(web_dir, "ICAO_Airlines.txt"), "rb") as fh:
                                out["airlinesSha256"] = hashlib.sha256(fh.read()).hexdigest()
                        except (OSError, TypeError):
                            pass
                        try:                                  # the plugin's font (vNAAATS.ttf): the app installs it next to the DLL when it is missing
                            with open(os.path.join(web_dir, "vNAAATS.ttf"), "rb") as fh:
                                out["fontSha256"] = hashlib.sha256(fh.read()).hexdigest()
                        except (OSError, TypeError):
                            pass
                        return self._send(200, out)
                    return self._send(200, blob, ctype="application/octet-stream")
                if p == "/relief/plan":                        # what a relief would move: the aircraft this position tracks
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    pos = (sess["position"] or "").upper()
                    if not pos or pos == "OBSERVER":
                        return self._send(403, {"error": "choose a controlling position first"})
                    with es_pick_lock:
                        cur = tracked_now.get(pos)
                    alive = time.time() - plugin_seen.get(pos, 0) < 30
                    lst = list(cur["list"]) if cur and time.time() - cur["ts"] <= RETRACK_WINDOW else []
                    return self._send(200, {"position": pos, "pluginAlive": alive,
                                            "aircraft": [{"callsign": c, "cpdlc": bool(cpdlc and cpdlc.is_connected(c))} for c in lst],
                                            "message": commands_mod.RELIEF_CPDLC_TEXT})
                if p == "/GetControllers":
                    if not network:
                        return self._send(200, [])
                    with network.lock:
                        return self._send(200, [c for c in network.controllers if cpdlc_mod.station_listed(c["callsign"])])
                if p == "/PluginScratch":                     # the plugin reports a scratchpad note typed in EuroScope
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    text = " ".join(q.get("text", "").upper().split())[:SCRATCH_MAX]
                    text = "".join(ch for ch in text if SCRATCH_OK.fullmatch(ch) or ch == " ")
                    return self._send(200, {"ok": store.set_scratch(q.get("callsign", ""), text, "Plugin")})
                if p == "/PluginRoute":                       # the plugin asks for the filed route of an aircraft nobody has handled yet
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    cs = re.sub(r"[^A-Z0-9]", "", q.get("callsign", "").upper())[:8]
                    rec = next((f for f in network.all() if f.get("callsign") == cs), None) if network else None
                    if not rec:
                        return self._send(404, {"error": "not an oceanic flight on the network"})
                    why = route_sanity(rec)
                    if why:
                        return self._send(200, {"ok": False, "why": why})
                    return self._send(200, {"ok": True, "route": rec["route"], "track": rec.get("track", "RR")})
                if p == "/PluginAsel":                        # the plugin reports the aircraft selected in EuroScope
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    me = re.sub(r"[^A-Z0-9_\-]", "", q.get("me", "").upper())[:16]
                    cs = re.sub(r"[^A-Z0-9]", "", q.get("callsign", "").upper())[:8]
                    if me:
                        with es_pick_lock:
                            old = es_pick.get(me, {"seq": 0})
                            if old.get("callsign") != cs:
                                es_pick[me] = {"callsign": cs, "seq": old["seq"] + 1, "ts": time.time()}
                    return self._send(200, {"ok": True})
                if p == "/PluginTracked":                     # the plugin reports which aircraft this controller is tracking
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    me = re.sub(r"[^A-Z0-9_\-]", "", q.get("me", "").upper())[:16]
                    lst = [c for c in (re.sub(r"[^A-Z0-9]", "", x.upper())[:8] for x in q.get("list", "").split(",")) if c][:100]
                    if me:
                        with es_pick_lock:
                            old = tracked_now.get(me)
                            fresh = bool(old) and time.time() - old["ts"] <= RETRACK_WINDOW      # same sitting as the last report
                            owners = set(old["owners"]) if fresh else set()
                            owners |= auth.names_at(me)                           # who is signed in on the FDD at this position right now
                            tracked_now[me] = {"list": lst, "ts": time.time(), "owners": owners}
                    return self._send(200, {"ok": True})
                if p == "/PluginRetrack":                     # after a crash / reconnect: which aircraft did this controller have?
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    me = re.sub(r"[^A-Z0-9_\-]", "", q.get("me", "").upper())[:16]
                    with es_pick_lock:
                        cur = tracked_now.get(me)
                    ok = bool(cur) and time.time() - cur["ts"] <= RETRACK_WINDOW
                    if ok:
                        now_names = auth.names_at(me)
                        if now_names and cur["owners"] and not (now_names & cur["owners"]):
                            ok = False                                            # somebody else has signed in on this position on the FDD: not theirs to get back
                    return self._send(200, {"callsigns": cur["list"] if ok else []})
                if p == "/settings":                          # the FDD Settings box: this controller's plugin options
                    sess = self._session()
                    if not sess:
                        return self._send(401, {"error": "log in first"})
                    if sess.get("via") not in ("member", "vatsim") or not re.fullmatch(r"\d{1,9}", str(sess.get("cid", ""))):
                        return self._send(403, {"error": "settings need your own sign-in (personal password or VATSIM Connect), not the shared password"})
                    rec = usettings.get(sess["cid"])
                    return self._send(200, {"settings": rec["settings"], "stamp": rec["stamp"], "defaults": {k: v["default"] for k, v in usersettings_mod.OPTIONS.items()}})
                if p == "/PluginSettings":                    # the plugin writes these into vnaaats.cfg (needs a personal key: it carries the CID)
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    cid, _ = members.check_key(q.get("code", "")) if members else (None, "")
                    rec = usettings.get(cid) if cid else {"settings": {}, "stamp": 0}
                    return self._send(200, rec)
                if p == "/asel":                              # the FDD asks which aircraft its controller clicked in EuroScope
                    sess = self._session()
                    pos = (sess["position"] if sess else "").upper()
                    with es_pick_lock:
                        cur = es_pick.get(pos)
                        if sess and pos in tracked_now and time.time() - plugin_seen.get(pos, 0) < 15:
                            tracked_now[pos]["owners"].add(sess["name"].strip().lower())    # this person is on the FDD while the plugin is alive
                    return self._send(200, {"callsign": cur["callsign"] if cur else "", "seq": cur["seq"] if cur else 0})
                if p == "/PluginCommands":                    # polled by the plugin
                    with es_pick_lock:
                        plugin_seen[re.sub(r"[^A-Z0-9_\-]", "", q.get("me", "").upper())[:16]] = time.time()
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    if rcl:
                        rcl.note_poll(q.get("me", ""))
                    return self._send(200, queue.pending_for(q.get("me", "")))
                if p == "/PluginCommandDone":
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    ok = q.get("ok", "0") == "1"
                    c = queue.done(to_int(q.get("id"), 0), ok, q.get("msg", ""))
                    if c and ok and c["type"] == "contactme":                 # remember it: the strip turns yellow, then red
                        base = None
                        if network and not store.get(c["callsign"]):
                            base = next((f for f in network.all() if f.get("callsign") == c["callsign"]), None)
                        store.mark_cm(c["callsign"], base)
                    if c and rcl and c.get("by") == "AUTO":                  # the automatic RCL REQ reminder
                        rcl.done(c, ok, q.get("msg", ""))
                    if c and ok and c["type"] == "planset":                   # the plan was amended in EuroScope: show it on the FDD too
                        pl = (c.get("args") or {}).get("plan") or {}
                        ac_parts = str(pl.get("acInfo", "")).split("/")
                        ch = {"departure": pl.get("origin", ""), "arrival": pl.get("dest", ""), "etd": pl.get("etd", "")}
                        if len(ac_parts) >= 2 and ac_parts[1]:
                            ch["type"] = ac_parts[1]
                        store.edit(c["callsign"], {k: v for k, v in ch.items() if v}, by=c.get("by", "Plugin"))
                    if c and ok and c["type"] == "transfer" and str(c.get("by", "")).startswith("RELIEF"):    # relief: the next controller takes it now
                        queue.add(c["args"]["target"], "takeover", c["callsign"], {}, by=c["by"])
                    if c and ok and c["type"] == "takeover" and str(c.get("by", "")).startswith("RELIEF") and cpdlc:   # taken: log the pilot off so they log on to the new station
                        try:
                            st = cpdlc.station_with(c["callsign"])
                            if st:
                                st.disconnect(c["callsign"], by=c["by"])
                        except (ValueError, RuntimeError):
                            pass
                    if c and ok and c["type"] == "transfer" and cpdlc:        # handoff started: tell the pilot (if on CPDLC)
                        try:
                            cpdlc.handoff(c["callsign"], c["args"]["target"], c["args"].get("freq", ""), by=c["by"])
                        except (ValueError, RuntimeError):
                            pass
                    return self._send(200, {"ok": bool(c)})
                if p == "/rcl/auto":
                    return self._send(200, rcl.status() if rcl else {"on": False, "sender": "", "inflight": 0, "error": "not available"})
                if p == "/cpdlc/templates":
                    return self._send(200, cpdlc_mod.FREETEXTS)
                if p == "/CpdlcHandoff":                       # called by the plugin when a handoff is started in EuroScope
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    if not cpdlc:
                        return self._send(200, {"ok": False, "error": "CPDLC is off"})
                    try:
                        msg = cpdlc.handoff(q.get("callsign", ""), q.get("target", ""), q.get("freq", ""), by=q.get("by", ""))
                    except ValueError as e:
                        return self._send(200, {"ok": False, "error": str(e)})      # not connected is normal, not an error
                    except RuntimeError as e:
                        return self._send(502, {"ok": False, "error": str(e)})
                    return self._send(200, {"ok": True, "text": msg["text"]})
                if p == "/cpdlc/status":
                    if not cpdlc:
                        return self._send(200, {"enabled": False, "connected": [], "unread": {}})
                    sess = self._session()
                    st = cpdlc.status(self._token() if sess else None, sess["position"] if sess else "")
                    if st.get("rclRequests"):
                        st["rclRequests"] = rcl_enrich(st["rclRequests"], natrak, store, network)
                    return self._send(200, st)
                if p == "/cpdlc/messages":
                    if not cpdlc:
                        return self._send(404, {"error": "CPDLC is not switched on"})
                    if not self._session():
                        return self._send(401, {"error": "log in first"})
                    st = cpdlc.get(self._token())
                    if not st:
                        return self._send(200, [])
                    return self._send(200, st.messages(q.get("callsign") or None, to_int(q.get("after"), 0)))
                if p == "/auth/status":
                    return self._send(200, self._status(self._session()))
                if p == "/auth/login":
                    if not auth.connect_enabled:
                        return self._send(404, {"error": "VATSIM login is not set up"})
                    return self._redirect(auth.authorize_url())
                if p == "/auth/callback":
                    if not auth.connect_enabled or not auth.use_state(q.get("state", "")) or not q.get("code"):
                        return self._send(400, "Login failed or expired - go back and try again.", "text/plain")
                    try:
                        cid, name = auth.exchange_code(q["code"])
                    except Exception as e:
                        log.warning("VATSIM login refused: %s", e)
                        return self._send(403, "Login refused: %s" % e, "text/plain")
                    return self._redirect("/fdd", auth.create_session(cid, name, "vatsim"))
                if p == "/GetSingleFlightData":
                    rec = store.get(q.get("callsign", ""))
                    if rec:
                        store.ack_web_edit(q.get("callsign", ""))        # the plugin has now seen any web edit
                    return self._send(200, [with_cpdlc(rec)]) if rec else self._send(404, {"error": "not found"})
                if p in ("/PostFlightData", "/UpdateFlightData"):
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    if "NAT" in str(q.get("route", "")).upper():          # a filed NATx token: replace it by that track's fixes
                        with cache.lock:
                            tracks_now = list(cache.tracks)
                        toks = str(q.get("route", "")).split()
                        et = str(q.get("routeEtas", "")).split()
                        new, new_et, done = expand_nat_tokens(toks, tracks_now, et if len(et) == len(toks) else None)
                        if done or len(new) != len(toks):
                            q = dict(q, route=" ".join(new))
                            if new_et is not None:
                                q["routeEtas"] = " ".join(new_et)
                            elif "routeEtas" in q:
                                q["routeEtas"] = " ".join(["--"] * len(new))
                    rec = store.upsert(q)
                    return self._send(200, rec) if rec else self._send(400, {"error": "callsign required"})
                if p in ("/GetAllFlightData", "/GetNetworkTraffic", "/GetNatTrakData") and not self._read_ok(q):
                    return self._send(401, {"error": "log in first and choose a position"})
                if p == "/GetAllFlightData":
                    hidden = store.dismissed_set()
                    return self._send(200, [with_cpdlc(f) for f in store.all_flights(newest_first=q.get("sort", "1") != "0") if f.get("callsign") not in hidden])
                if p == "/GetNetworkTraffic":
                    hidden = store.dismissed_set()
                    return self._send(200, [with_cpdlc(f) for f in network.all() if f.get("callsign") not in hidden] if network else [])
                if p == "/GetNatTrakData":
                    return self._send(200, natrak.all() if natrak else [])
                if p == "/PostNamedFix":
                    bad = key_problem(q.get("code", ""))
                    if bad:
                        return self._send(401, {"error": "bad key", "reason": bad})
                    name = q.get("name", "").strip().upper()
                    try:
                        lat, lon = float(q.get("lat", "")), float(q.get("lon", ""))
                    except ValueError:
                        return self._send(400, {"error": "lat and lon must be numbers"})
                    if not re.fullmatch(r"[A-Z0-9]{2,8}", name) or not (-90 <= lat <= 90) or not (-180 <= lon <= 180):
                        return self._send(400, {"error": "invalid fix"})
                    store.set_fix(name, lat, lon)
                    cache.add_fix(name, lat, lon)
                    return self._send(200, {"name": name, "lat": lat, "lon": lon})
                if p == "/health":
                    with cache.lock:
                        return self._send(200, {"ok": True, "version": SERVER_VERSION, "tracks": len(cache.tracks), "tracksUpdated": cache.updated, "trackError": cache.error})
                if p == "/unresolved":
                    with cache.lock:
                        return self._send(200, cache.unresolved)
                if self._static(p):
                    return
                if p == "/":
                    return self._send(200, "<h1>vNAAATS server</h1><p>Backend for the vNAAATS EuroScope plugin.</p>", "text/html")
                return self._send(404, {"error": "unknown endpoint"})
            except Exception as e:
                log.exception("request failed")
                return self._send(500, {"error": str(e)})

    return Handler


def main():
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    api_key = os.environ.get("VNAAATS_API_KEY", "")
    if len(api_key) < 16:
        raise SystemExit("Set VNAAATS_API_KEY to a long random secret (16+ characters).")
    store = FlightStore(os.environ.get("VNAAATS_DB", os.path.join(HERE, "vnaaats.db")))
    fixes = load_fixes()
    fixes.update(store.get_fixes())                           # fixes added through the admin page win
    cache = TrackCache(fixes)
    web_dir = os.environ.get("VNAAATS_WEB_DIR", os.path.join(HERE, "web"))
    threading.Thread(target=cache.loop, daemon=True).start()
    natrak = None
    if NATTRAK_PLUGIN_URL.lower() != "off":
        natrak = NatTrakFeed()
        threading.Thread(target=natrak.loop, daemon=True).start()
    network = None
    if VATSIM_FEED.lower() != "off":
        network = NetworkTraffic(cache, natrak=natrak)
        threading.Thread(target=network.loop, daemon=True).start()
    threading.Thread(target=lambda: [store.purge() or time.sleep(3600) for _ in iter(int, 1)], daemon=True).start()
    host, port = os.environ.get("VNAAATS_BIND", "127.0.0.1"), int(os.environ.get("VNAAATS_PORT", "8080"))
    log.info("Listening on %s:%d", host, port)
    cpdlc = None
    auth = Auth()
    if os.environ.get("VNAAATS_CPDLC", "on").lower() != "off":
        def locate(callsign):                              # is this pilot within 1000 nm of the oceanic airspace?
            pos = network.position(callsign) if network else None
            if not pos:
                return None                                # not seen on the feed (yet): do not block
            with cache.lock:
                tracks = list(cache.tracks)
            return cpdlc_mod.near_oceanic(pos[0], pos[1], tracks)
        cpdlc = cpdlc_mod.CpdlcHub(os.environ.get("VNAAATS_DB", os.path.join(HERE, "vnaaats.db")),
                                   locate=locate,
                                   url=os.environ.get("VNAAATS_HOPPIE_URL", cpdlc_mod.DEFAULT_URL),
                                   auto_accept=os.environ.get("VNAAATS_CPDLC_AUTOACCEPT", "1") != "0",
                                   stations=cpdlc_mod.parse_stations(os.environ.get("VNAAATS_CPDLC_STATIONS", "")),
                                   is_session_alive=lambda t: auth.get(t) is not None)
        threading.Thread(target=lambda: [cpdlc.reap() or time.sleep(30) for _ in iter(int, 1)], daemon=True).start()
        log.info("CPDLC is available: each controller signs in with their own Hoppie code")
    else:
        log.info("CPDLC is OFF (VNAAATS_CPDLC=off)")
    log.info("Web editing: %s", "VATSIM Connect + password" if auth.connect_enabled and auth.password else "VATSIM Connect" if auth.connect_enabled else "password" if auth.password else "OFF (set VNAAATS_FDD_PASSWORD)")
    queue = commands_mod.CommandQueue()
    members = members_mod.MemberKeys(store)
    issuer_secret = os.environ.get("VNAAATS_ISSUER_SECRET", "")
    roster = None
    roster_url = os.environ.get("VNAAATS_ROSTER_URL", "https://ganderoceanic.ca/api/roster")
    if roster_url.lower() != "off":
        roster = members_mod.RosterWatch(members, roster_url)
        threading.Thread(target=roster.loop, daemon=True).start()
    log.info("Personal member keys: %s", "website can issue them" if len(issuer_secret) >= 24 else "off (set VNAAATS_ISSUER_SECRET, 24+ characters)")
    rcl = None
    if os.environ.get("VNAAATS_RCL_AUTO", "on").lower() != "off" and network:
        rcl = rcl_auto_mod.RclAuto(store, queue, network, natrak, lambda route, pos: route_distance_nm(route, pos, cache.fixes), OCEANIC_PREFIXES)
        log.info("Automatic RCL REQ reminders are available (%s)", "ON" if rcl.enabled else "switched off on the display")
    logs = adminlog_mod.LogHub(os.environ.get("VNAAATS_ADMIN_TOKEN", ""), os.environ.get("VNAAATS_LOG_DIR", os.path.join(HERE, "logs")))
    hook = os.environ.get("VNAAATS_ADMIN_WEBHOOK", "").strip()
    if hook:
        logs.notifier = adminlog_mod.Notifier(hook, summary_hours=float(os.environ.get("VNAAATS_SUMMARY_HOURS", "6") or 6),
                                         ping_user=os.environ.get("VNAAATS_ADMIN_PING", ""))
        threading.Thread(target=logs.notifier.run, daemon=True).start()
        log.info("Admin log alerts are ON (errors ping %s, summary every %s hours)", "you" if os.environ.get("VNAAATS_ADMIN_PING") else "@here", os.environ.get("VNAAATS_SUMMARY_HOURS", "6"))
    if not logs.token:
        log.info("Admin log viewer is OFF: set VNAAATS_ADMIN_TOKEN (16+ characters) in /etc/vnaaats.env to switch it on")
    ThreadingHTTPServer((host, port), make_handler(cache, store, api_key, web_dir if os.path.isdir(web_dir) else None, network, natrak, auth, cpdlc, queue, rcl, members, issuer_secret, roster, logs)).serve_forever()


if __name__ == "__main__":
    main()
