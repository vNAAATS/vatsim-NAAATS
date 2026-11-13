"""
CPDLC ground station for the vNAAATS server, talking to the Hoppie ACARS network (https://www.hoppie.nl/acars).

Each controller signs in to their own station callsign (CZQX, EGGX, NATX ...) with THEIR OWN Hoppie logon code, typed on the
FDD. The code is kept in memory only while that controller is signed in (never written to disk). Pilots send "REQUEST LOGON" to the station callsign; we accept it (automatically unless VNAAATS_CPDLC_AUTOACCEPT=0), remember which aircraft are connected, keep
a message log, and let signed-in controllers send uplinks from the flight data display.

Hoppie rules this follows (https://www.hoppie.nl/acars/system/tech.html):
  * poll every 45-75 s at random, every ~20 s for a short while after something was sent or received
  * 15 s connection timeout, never retry straight away
  * one logon per station callsign at a time (a second client using the same callsign gets "callsign already in use")
  * polling takes messages OUT of Hoppie's queue, so this only works if nobody else polls the same station callsign
The CpdlcHub at the bottom of this file manages the stations: one running CpdlcStation per station callsign.

CPDLC packet:  /data2/<MIN>/<MRN>/<RA>/<message>      (MIN = sender's message number, MRN = number being replied to,
RA = reply wanted: WU wilco/unable, AN affirm/negative, R roger, NE none, Y/N for pilot downlinks)
Poll answer:   ok {FROM type {packet}} {FROM type {packet}} ...      (or just "ok", or "error {reason}")
"""
import logging
import math
import os
import random
import re
import sqlite3
import threading
import time
import urllib.parse
import urllib.request

log = logging.getLogger("vnaaats.cpdlc")

DEFAULT_URL = "https://www.hoppie.nl/acars/system/connect.html"
UPLINK_RA = ("WU", "AN", "R", "NE")
PING_SECONDS = 300
PING_MISSES_TO_DROP = 2
FAST_POLL_SECONDS = 120          # keep polling every ~20 s for this long after traffic
MAX_TEXT = 240
TELEX_PART = 220          # longest single telex sent; longer messages are split
TELEX_MAX_TOTAL = 800     # longest whole message send_telex accepts


def split_telex(text, size=TELEX_PART):
    """Cut a message into parts of at most `size` characters, at spaces."""
    parts, cur = [], ""
    for word in text.split(" "):
        if cur and len(cur) + 1 + len(word) > size:
            parts.append(cur)
            cur = word
        else:
            cur = (cur + " " + word) if cur else word
    if cur:
        parts.append(cur)
    return parts


REPLY_STATES = (("WILCO", "WILCO"), ("UNABLE", "UNABLE"), ("ROGER", "ROGER"), ("AFFIRM", "AFFIRM"),
                ("NEGATIVE", "NEGATIVE"), ("STANDBY", "STANDBY"))


OCA_REQUEST_RE = re.compile(r"\bOCEANIC\s+CLEARANCE\b|\bOCEANIC\s+CLX\b|\bOCA\s+CLEARANCE\b|\bREQUEST\s+CLEARANCE\b|\bOCEAN\s+REQUEST\b|\bOCEAN\s+CLEARANCE\b|\bOCEANIC\s+REQUEST\b|\bRCL\b|\bENTRY\s+POINT\b")
MSG_OCA_UNABLE = "UNABLE TO PROVIDE OCEANIC CLEARANCES VIA CPDLC. PLEASE VISIT NATTRAK.VATSIM.NET AND SUBMIT YOUR REQUEST THERE"
RCL_STATION_NAMES = {"CZQX": "CZQO", "EGGX": "EGGX", "NATX": "NAT_FSS"}      # Hoppie station -> the name the pilot sees when the entry area is not known


def rcl_received_text(oca):
    """What the pilot is told when the request arrives. oca = the area the flight enters (CZQO or EGGX)."""
    return "RCL RECEIVED BY %s" % (oca or "NAT_FSS")


def entry_contact_text(oca):
    """Sent once, at the oceanic entry point, to a pilot who is connected by CPDLC (needs ROGER)."""
    return "THIS IS AN AUTOMATED MESSAGE TO CONFIRM CPDLC CONTACT WITH %s CENTER" % ("GANDER" if oca == "CZQO" else "SHANWICK")


ENTRY_MSG = os.environ.get("VNAAATS_CPDLC_ENTRY_MSG", "on").lower() != "off"
NOREPLY_SECONDS = 180            # an uplink that needs an answer and has none after this long is marked "no reply" on the display
RCL_INTAKE = os.environ.get("VNAAATS_CPDLC_RCL", "on").lower() != "off"      # off = the old behaviour (refuse and point to natTrak)
RCL_SEND = os.environ.get("VNAAATS_CPDLC_RCL_SEND", "off").lower() == "on"      # the "RCL processed" / "Send reclearance" buttons: off until switched on (staged roll-out)
RCL_KEEP_SECONDS = 6 * 3600        # a request stays in the list this long unless dismissed
RCL_REPEAT_SECONDS = 1800          # a second request from the same aircraft inside this time updates the first
MSG_TOO_FAR = "UNABLE. CONTACT CONTROLLER ON VOICE 20 MILES BEFORE OCEANIC ENTRY POINT"
MSG_SQUAWK = "SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA"      # sent to every pilot who is accepted onto CPDLC (the plugin no longer sets the code itself)
SQUAWK_MSG = os.environ.get("VNAAATS_CPDLC_SQUAWK_MSG", "off").lower() == "on"      # off: the squawk instruction now comes with the "fly current flight plan" message after the request is processed
MSG_UNICOM = "THERE IS NO FURTHER ATC AVAILABLE. SERVICE TERMINATED. MONITOR ADVISORY ON 122.800"
LOGOFF_TEXT = "LOGOFF"          # UNVERIFIED against the live Hoppie network: what the ground sends to log a pilot off
NEAR_OCEANIC_NM = 1000           # pilots further than this from the oceanic airspace are refused a CPDLC logon
RELEASE_SECONDS = 180            # how long a "release the tag" request stays visible to the plugin
PENDING_SECONDS = 1800           # a handoff/UNICOM waiting for the pilot's WILCO is forgotten after this


# ----------------------------------------------------------------------------- station names and free texts (TopSky list)
def _load_topsky(path=None):
    path = path or os.path.join(os.path.dirname(os.path.abspath(__file__)), "topsky_cpdlc.txt")
    names, short, freetext = {}, {}, []
    try:
        section = ""
        for line in open(path, encoding="utf-8", errors="replace"):
            line = line.strip()
            if not line or line.startswith("//"):
                continue
            if line.startswith("["):
                section = line.upper()
                continue
            parts = line.split(":", 3)
            if section == "[STATIONS]" and parts[0].upper() == "LOGIN" and len(parts) == 4:
                names.setdefault(parts[1].strip().upper(), parts[2].strip())
                short.setdefault(parts[3].strip().upper(), parts[2].strip())
            elif section == "[FREETEXT]" and parts[0].upper() == "FREETEXT" and len(parts) == 3:
                freetext.append({"ra": parts[1].strip().upper(), "text": parts[2].strip()})
    except OSError:
        log.warning("topsky_cpdlc.txt not found: station names and free texts unavailable")
    return names, short, freetext


STATION_NAMES, STATION_SHORT, FREETEXTS = _load_topsky()
CALLSIGN_ALIASES = {"NAT": "NATX", "LFRR": "LFXX", "LFBB": "LFXX", "LFMM": "LFXX", "LFEE": "LFXX", "LFFF": "LFXX"}


def friendly_name(raw):
    """'SHANNON CTL' -> 'Shannon Control', 'NEW YORK RADIO' -> 'New York Radio'."""
    out = []
    for w in raw.replace("_", " ").split():
        u = w.upper()
        out.append("Control" if u in ("CTL", "CTR") else u.lower() if u in ("AND", "OF") else w.capitalize())
    return " ".join(out)


def station_display_name(callsign):
    """EuroScope controller callsign (EISN_CTR, NAT_FSS, EGPX_N_CTR ...) -> the full station name for pilots."""
    cs = (callsign or "").strip().upper()
    prefix = cs.split("_")[0]
    code = CALLSIGN_ALIASES.get(prefix, prefix)
    if code in STATION_NAMES:
        return friendly_name(STATION_NAMES[code])
    if prefix in STATION_SHORT:
        return friendly_name(STATION_SHORT[prefix])
    return friendly_name(cs) if cs else ""


def station_listed(callsign):
    """True if this controller callsign is one of the stations in topsky_cpdlc.txt (the only ones aircraft are transferred to)."""
    prefix = (callsign or "").strip().upper().split("_")[0]
    return CALLSIGN_ALIASES.get(prefix, prefix) in STATION_NAMES or prefix in STATION_SHORT


def nm_between(lat1, lon1, lat2, lon2):
    """Great circle distance in nautical miles."""
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dl = math.radians(lon2 - lon1)
    a = math.sin((p2 - p1) / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 3440.065 * 2 * math.asin(min(1.0, math.sqrt(a)))


def near_oceanic(lat, lon, tracks, limit_nm=NEAR_OCEANIC_NM):
    """True/False: within limit_nm of any waypoint of the active tracks. None if there are no tracks to judge by."""
    pts = [(p["latitude"], p["longitude"]) for t in tracks for p in t.get("route", [])]
    if not pts:
        return None
    return any(nm_between(lat, lon, a, b) <= limit_nm for a, b in pts)


# ----------------------------------------------------------------------------- protocol helpers
def parse_blocks(text):
    """'ok {A cpdlc {/data2/..}} {B telex {hi}}' -> ('ok', [(from, type, packet), ...]). Also handles 'error {reason}'."""
    text = (text or "").strip()
    head = text.split(" ", 1)[0].lower() if text else ""
    blocks, depth, start = [], 0, 0
    for i, ch in enumerate(text):
        if ch == "{":
            if depth == 0:
                start = i + 1
            depth += 1
        elif ch == "}" and depth > 0:
            depth -= 1
            if depth == 0:
                blocks.append(text[start:i])
    out = []
    for b in blocks:
        m = re.match(r"\s*(\S+)\s+(\S+)\s+\{(.*)\}\s*$", b, re.S)
        if m:
            out.append((m.group(1).upper(), m.group(2).lower(), m.group(3)))
    return head, out, blocks


MACH_RE = re.compile(r"\bM(?:ACH)?\s*0?\.?(\d{2})\d?\b")
LEVEL_RE = re.compile(r"\bF(?:L)?\s*(\d{3})\b")
MAXFL_RE = re.compile(r"\bMAX\s*(?:FL|F)?\s*(\d{3})\b")


def parse_rcl_text(text):
    """What a pilot asked for in an oceanic clearance request ('... REQ: M0.86 FL400'): Mach in hundredths (86) and level (400). None for anything not found."""
    t = (text or "").upper()
    mx = MAXFL_RE.search(t)
    if mx:
        t = t[:mx.start()] + " " + t[mx.end():]                  # "MAX FL380" is the highest acceptable level, not the requested one
    m, f = MACH_RE.search(t), LEVEL_RE.search(t)
    return {"mach": int(m.group(1)) if m else None, "level": int(f.group(1)) if f else None, "max": int(mx.group(1)) if mx else None}


def parse_cpdlc_packet(packet):
    """'/data2/15//WU/CONTACT @X@' -> dict(min=15, mrn=None, ra='WU', text='CONTACT @X@') or None."""
    parts = (packet or "").split("/", 5)
    if len(parts) < 6 or parts[1].lower() != "data2":
        return None
    try:
        mn = int(parts[2])
    except ValueError:
        return None
    try:
        mrn = int(parts[3]) if parts[3] else None
    except ValueError:
        mrn = None
    return {"min": mn, "mrn": mrn, "ra": parts[4].upper(), "text": parts[5].strip()}


def readable(text):
    """Hoppie marks variable parts with @; show them plainly."""
    return re.sub(r"\s+", " ", (text or "").replace("@", " ")).strip()


class HoppieClient:
    def __init__(self, url, logon, station, http=None):
        self.url, self.logon, self.station = url, logon, station
        self._http = http or self._real_http

    @staticmethod
    def _real_http(url, data):
        req = urllib.request.Request(url, data=data)             # POST, form encoded (keeps the logon code out of the URL)
        with urllib.request.urlopen(req, timeout=15) as r:
            return r.read().decode("utf-8", "replace")

    def request(self, to, type_, packet=""):
        form = urllib.parse.urlencode({"logon": self.logon, "from": self.station, "to": to, "type": type_, "packet": packet}).encode()
        return self._http(self.url, form)


# ----------------------------------------------------------------------------- the station
class CpdlcStation:
    def __init__(self, db_path, client, auto_accept=True, now=time.time, locate=None, on_release=None, facts=None):
        self.client = client
        self.auto_accept = auto_accept
        self.now = now
        self.locate = locate or (lambda callsign: None)          # callsign -> True (near oceanic) / False (too far) / None (unknown)
        self.facts = facts or (lambda callsign: {})              # callsign -> {"airborne", "skip", "entry" (CZQO/EGGX), "inside"} where known
        self.entry_sent = {}                                     # flight -> the area (CZQO/EGGX) whose contact message it last had from this station
        self.on_release = on_release or (lambda callsign: None)  # called when the tag should be released in EuroScope
        self.pending = {}                                        # callsign -> {"min", "kind", "ts"}: waiting for the pilot's WILCO/ROGER
        self.lock = threading.RLock()
        self.db = sqlite3.connect(db_path, check_same_thread=False, timeout=30)
        try: self.db.execute("PRAGMA journal_mode=WAL")
        except sqlite3.DatabaseError: pass
        self.db.execute("""CREATE TABLE IF NOT EXISTS cpdlc_messages (id INTEGER PRIMARY KEY AUTOINCREMENT, ts REAL NOT NULL,
            dir TEXT NOT NULL, callsign TEXT NOT NULL, min INTEGER, mrn INTEGER, ra TEXT, text TEXT NOT NULL,
            state TEXT NOT NULL DEFAULT '', seen INTEGER NOT NULL DEFAULT 1, by TEXT NOT NULL DEFAULT '')""")
        try:
            self.db.execute("ALTER TABLE cpdlc_messages ADD COLUMN station TEXT NOT NULL DEFAULT ''")
        except sqlite3.OperationalError:
            pass                                              # column already there
        self.db.execute("""CREATE TABLE IF NOT EXISTS cpdlc_rcl (id INTEGER PRIMARY KEY AUTOINCREMENT, ts REAL NOT NULL, station TEXT NOT NULL,
            callsign TEXT NOT NULL, text TEXT NOT NULL, state TEXT NOT NULL DEFAULT 'open')""")
        for col in ("sent_ts REAL", "sent_by TEXT", "sent_kind TEXT"):
            try:
                self.db.execute("ALTER TABLE cpdlc_rcl ADD COLUMN " + col)
            except sqlite3.OperationalError:
                pass                                          # column already there
        self.db.execute("CREATE TABLE IF NOT EXISTS cpdlc_conn2 (station TEXT NOT NULL, callsign TEXT NOT NULL, since REAL NOT NULL, PRIMARY KEY (station, callsign))")
        self.db.commit()
        self.stop_flag = False
        # Connections are NOT restored: a station only polls while a controller is signed in to it, so after a stop pilots must log on again.
        self.db.execute("DELETE FROM cpdlc_conn2 WHERE station=?", (client.station,))
        self.db.execute("DELETE FROM cpdlc_messages WHERE ts < ?", (self.now() - self.MESSAGE_KEEP,))     # days-old history is not kept
        # A new sign-in starts with nothing unread: whatever is left from earlier sessions is old.
        self.db.execute("UPDATE cpdlc_messages SET seen=1 WHERE station=? AND seen=0", (client.station,))
        self.db.commit()
        self.connected = {}
        self.next_min = 1
        self.fast_until = 0.0
        self.last_poll = None
        self.error = ""
        self.last_ping = 0.0

    # --- storage
    def _store(self, direction, callsign, mn, mrn, ra, text, state="", seen=1, by=""):
        with self.lock:
            cur = self.db.execute("INSERT INTO cpdlc_messages (ts, dir, callsign, min, mrn, ra, text, state, seen, by, station) VALUES (?,?,?,?,?,?,?,?,?,?,?)",
                                  (self.now(), direction, callsign, mn, mrn, ra, text, state, seen, by, self.client.station))
            self.db.commit()
            return cur.lastrowid

    def _row(self, r):
        return {"id": r[0], "ts": r[1], "dir": r[2], "callsign": r[3], "min": r[4], "mrn": r[5], "ra": r[6],
                "text": r[7], "readable": readable(r[7]), "state": r[8], "seen": bool(r[9]), "by": r[10]}

    COLS = "id, ts, dir, callsign, min, mrn, ra, text, state, seen, by"

    MESSAGE_MAX_AGE = 6 * 3600           # the drawer never shows anything older than this
    MESSAGE_KEEP = 2 * 86400             # and anything older than this is deleted when the station starts

    def messages(self, callsign=None, after=0, limit=200):
        with self.lock:
            floor = self.now() - self.MESSAGE_MAX_AGE
            if callsign:
                conn = self.connected.get(callsign.upper())
                if conn:                                          # connected now: this logon only (the request for it arrives just before the connection is made)
                    floor = max(floor, conn["since"] - 120)
                rows = self.db.execute("SELECT %s FROM cpdlc_messages WHERE station=? AND callsign=? AND id>? AND ts>=? ORDER BY id DESC LIMIT ?" % self.COLS,
                                       (self.client.station, callsign.upper(), after, floor, limit)).fetchall()
            else:
                rows = self.db.execute("SELECT %s FROM cpdlc_messages WHERE station=? AND id>? AND ts>=? ORDER BY id DESC LIMIT ?" % self.COLS, (self.client.station, after, floor, limit)).fetchall()
        return [self._row(r) for r in reversed(rows)]

    def mark_seen(self, callsign):
        with self.lock:
            self.db.execute("UPDATE cpdlc_messages SET seen=1 WHERE station=? AND callsign=? AND seen=0", (self.client.station, callsign.upper()))
            self.db.commit()

    def unread(self):
        with self.lock:
            # only aircraft that are connected now: nothing can be done about a message from one that is not
            return {c: n for c, n in self.db.execute("SELECT callsign, COUNT(*) FROM cpdlc_messages WHERE station=? AND seen=0 GROUP BY callsign", (self.client.station,))
                    if c in self.connected}

    def _set_connected(self, callsign, yes):
        with self.lock:
            if yes and callsign not in self.connected:
                self.connected[callsign] = {"since": self.now(), "misses": 0}
                self.db.execute("INSERT OR REPLACE INTO cpdlc_conn2 VALUES (?,?,?)", (self.client.station, callsign, self.connected[callsign]["since"]))
            elif yes:
                self.connected[callsign]["misses"] = 0
            elif callsign in self.connected:
                del self.connected[callsign]
                self.db.execute("DELETE FROM cpdlc_conn2 WHERE station=? AND callsign=?", (self.client.station, callsign))
            self.db.commit()

    def is_connected(self, callsign):
        with self.lock:
            return callsign.upper() in self.connected

    def connected_callsigns(self):
        with self.lock:
            return set(self.connected)

    def status(self):
        with self.lock:
            unread = self.unread()
            conn = [{"callsign": c, "since": v["since"], "unread": unread.get(c, 0)} for c, v in sorted(self.connected.items())]
            return {"enabled": True, "station": self.client.station, "autoAccept": self.auto_accept, "connected": conn,
                    "unread": unread, "lastPoll": self.last_poll, "error": self.error, "rclRequests": self.rcl_requests(), "rclSend": RCL_SEND}

    # --- oceanic clearance requests received by CPDLC (to be entered on natTrak by a controller)
    def rcl_requests(self):
        with self.lock:
            rows = self.db.execute("SELECT id, ts, callsign, text, sent_ts, sent_by, sent_kind FROM cpdlc_rcl WHERE station=? AND state='open' AND ts > ? ORDER BY ts",
                                   (self.client.station, self.now() - RCL_KEEP_SECONDS)).fetchall()
        return [{"id": r[0], "ts": r[1], "callsign": r[2], "text": readable(r[3]),
                 "sent": ({"ts": r[4], "by": r[5] or "", "kind": r[6] or ""} if r[4] else None)} for r in rows]

    def rcl_get(self, rid):
        for r in self.rcl_requests():
            if r["id"] == int(rid):
                return r
        return None

    def rcl_mark_sent(self, rid, by, kind):
        with self.lock:
            self.db.execute("UPDATE cpdlc_rcl SET sent_ts=?, sent_by=?, sent_kind=? WHERE id=? AND station=?",
                            (self.now(), by, kind, int(rid), self.client.station))
            self.db.commit()

    def rcl_dismiss(self, rid):
        with self.lock:
            cur = self.db.execute("UPDATE cpdlc_rcl SET state='dismissed' WHERE id=? AND station=? AND state='open'", (int(rid), self.client.station))
            self.db.commit()
            return cur.rowcount > 0

    def oca_name(self, callsign):
        """The area a flight enters (CZQO or EGGX): from its direction when known, otherwise from the station the pilot is talking to."""
        f = {}
        try:
            f = self.facts((callsign or "").upper()) or {}
        except Exception as e:                                # a broken lookup must never stop a reply going out
            log.warning("facts lookup failed for %s: %s", callsign, e)
        return f.get("entry") or RCL_STATION_NAMES.get(self.client.station, "NAT_FSS")

    def entry_tick(self, only=None):
        """Once per area, when a pilot connected by CPDLC is inside the oceanic area, tell them who has contact (again on crossing 30W into the other area). Needs ROGER.
        Never to an aircraft on the ground, one departing Iceland (Reykjavik's, not ours), or one we cannot place."""
        if not ENTRY_MSG:
            return
        with self.lock:
            names = [only.upper()] if only else list(self.connected.keys())
        for cs in names:
            if not self.is_connected(cs):
                continue
            try:
                f = self.facts(cs) or {}
            except Exception as e:
                log.warning("facts lookup failed for %s: %s", cs, e)
                continue
            if f.get("airborne") is not True or f.get("skip") or f.get("inside") is not True:
                continue
            # the area the aircraft is in now: crossing 30W between Shanwick and Gander (either way) gets a second message for the new area
            area = f.get("area") or f.get("entry") or self.oca_name(cs)
            if cs in self.entry_sent and (self.entry_sent[cs] == area or self.client.station != "NATX"):
                continue                                                 # the second message at 30W is only for the combined NAT_FSS (NATX) station
            text = entry_contact_text(area)
            with self.lock:
                done = self.db.execute("SELECT 1 FROM cpdlc_messages WHERE callsign=? AND dir='UP' AND text=? AND ts > ?",
                                       (cs, text, self.now() - 12 * 3600)).fetchone()
            self.entry_sent[cs] = area
            if done:
                continue
            mn = self._alloc_min()
            try:
                self._send_packet(cs, mn, None, "R", text)
            except RuntimeError as e:
                log.warning("entry-point message to %s failed: %s", cs, e)
                self.entry_sent.pop(cs, None)
                continue
            self._store("UP", cs, mn, None, "R", text, state="SENT", by="auto")

    def _rcl_request(self, sender, pkt):
        """A pilot asked for an oceanic clearance over CPDLC: keep it for the controllers, and tell the pilot it arrived."""
        cs = sender.upper()
        with self.lock:
            now = self.now()
            row = self.db.execute("SELECT id FROM cpdlc_rcl WHERE station=? AND callsign=? AND state='open' AND ts > ?",
                                  (self.client.station, cs, now - RCL_REPEAT_SECONDS)).fetchone()
            if row:
                self.db.execute("UPDATE cpdlc_rcl SET ts=?, text=? WHERE id=?", (now, pkt["text"], row[0]))
            else:
                self.db.execute("INSERT INTO cpdlc_rcl (ts, station, callsign, text) VALUES (?,?,?,?)", (now, self.client.station, cs, pkt["text"]))
            self.db.commit()
        self._store("DOWN", sender, pkt["min"], pkt["mrn"], pkt["ra"], pkt["text"], state="", seen=0)
        self._auto_reply(sender, rcl_received_text(self.oca_name(sender)), pkt["min"], "NE")

    # --- sending
    def _alloc_min(self):
        with self.lock:
            m = self.next_min
            self.next_min = m % 63 + 1
            return m

    def _send_packet(self, callsign, mn, mrn, ra, text):
        packet = "/data2/%d/%s/%s/%s" % (mn, "" if mrn is None else mrn, ra, text)
        answer = self.client.request(callsign, "cpdlc", packet)
        head = answer.strip().split(" ", 1)[0].lower()
        if head != "ok":
            raise RuntimeError("Hoppie refused the message: " + answer.strip()[:120])
        self.fast_until = self.now() + FAST_POLL_SECONDS

    def send(self, callsign, text, ra="WU", mrn=None, by=""):
        """Send an uplink. Raises ValueError for bad input, RuntimeError if Hoppie refuses."""
        cs = (callsign or "").strip().upper()
        text = re.sub(r"[^A-Z0-9 @./,:+\-]", "", (text or "").upper()).strip()      # Hoppie wants upper case letters and numbers
        ra = (ra or "NE").upper()
        if not re.fullmatch(r"[A-Z0-9]{2,8}", cs):
            raise ValueError("bad callsign")
        if not text or len(text) > MAX_TEXT:
            raise ValueError("message must be 1-%d characters" % MAX_TEXT)
        if ra not in UPLINK_RA:
            raise ValueError("reply type must be one of " + ", ".join(UPLINK_RA))
        if not self.is_connected(cs):
            raise ValueError("%s is not connected to CPDLC" % cs)
        mn = self._alloc_min()
        self._send_packet(cs, mn, mrn, ra, text)
        mid = self._store("UP", cs, mn, mrn, ra, text, state="SENT" if ra != "NE" else "", by=by)
        self.mark_seen(cs)                                    # answering counts as having read what they sent
        if mrn is not None:                                   # answering a pilot request closes it
            with self.lock:
                self.db.execute("UPDATE cpdlc_messages SET state='ANSWERED' WHERE callsign=? AND dir='DOWN' AND min=? AND state='OPEN'", (cs, mrn))
                self.db.commit()
        return self.messages(cs, after=mid - 1, limit=1)[0]

    def send_telex(self, callsign, text, by=""):
        """Send a plain telex to a pilot who is not connected by CPDLC (for example one who asked for a clearance by telex)."""
        cs = (callsign or "").strip().upper()
        text = re.sub(r"[^A-Z0-9 @./,:+\-]", "", (text or "").upper()).strip()
        if not re.fullmatch(r"[A-Z0-9]{2,8}", cs):
            raise ValueError("bad callsign")
        if not text or len(text) > TELEX_MAX_TOTAL:
            raise ValueError("message must be 1-%d characters" % TELEX_MAX_TOTAL)
        # a long message goes as several telexes, each cut at a sentence or word boundary, in order
        for part in split_telex(text):
            answer = self.client.request(cs, "telex", part)
            if parse_blocks(answer)[0] != "ok":
                raise RuntimeError("Hoppie refused the telex: " + (answer or "").strip()[:120])
            self._store("UP", cs, None, None, "", "TELEX: " + part, state="", by=by)

    # --- receiving
    def handle_cpdlc(self, sender, packet):
        pkt = parse_cpdlc_packet(packet)
        if not pkt:
            log.info("ignored non-CPDLC packet from %s", sender)
            return
        text = pkt["text"].upper()
        if "REQUEST LOGON" in text or text.strip() == "LOGON":
            self._store("DOWN", sender, pkt["min"], pkt["mrn"], pkt["ra"], pkt["text"], state="LOGON", seen=1)
            if self.auto_accept and self.locate(sender.upper()) is False:
                self._auto_reply(sender, MSG_TOO_FAR, pkt["min"], "NE")      # too far from the airspace: refuse
            elif self.auto_accept:
                self.accept_logon(sender, pkt["min"])
            else:
                self._store("DOWN", sender, None, None, "", "LOGON WAITING FOR ACCEPT", state="", seen=0)
            return
        if re.search(r"\bLOGOFF\b|\bLOG OFF\b", text):
            self._store("DOWN", sender, pkt["min"], pkt["mrn"], pkt["ra"], pkt["text"], state="", seen=1)
            self._set_connected(sender, False)
            return
        if OCA_REQUEST_RE.search(text):
            if RCL_INTAKE:                                    # keep it for a controller to enter on natTrak, and say it arrived
                self._rcl_request(sender, pkt)
                return
            self._store("DOWN", sender, pkt["min"], pkt["mrn"], pkt["ra"], pkt["text"], state="ANSWERED", seen=1)
            self._auto_reply(sender, MSG_OCA_UNABLE, pkt["min"], "R")
            return
        state = "OPEN" if pkt["ra"] == "Y" else ""
        # A message from an aircraft that is not logged on is old mail that was waiting at Hoppie: keep it in the log but do not alert.
        self._store("DOWN", sender, pkt["min"], pkt["mrn"], pkt["ra"], pkt["text"], state=state, seen=0 if self.is_connected(sender) else 1)
        if pkt["mrn"] is not None:                            # a reply to one of our uplinks
            for word, name in REPLY_STATES:
                if text.startswith(word):
                    with self.lock:
                        self.db.execute("UPDATE cpdlc_messages SET state=? WHERE callsign=? AND dir='UP' AND min=?", (name, sender, pkt["mrn"]))
                        self.db.commit()
                    break
            pend = self.pending.get(sender.upper())
            if pend and pend["min"] == pkt["mrn"]:
                if text.startswith("WILCO") or text.startswith("ROGER"):
                    self.finish(sender.upper())
                elif text.startswith("UNABLE"):
                    self.pending.pop(sender.upper(), None)

    def handle_telex(self, sender, packet):
        """A pilot sent a plain telex (not CPDLC). An oceanic clearance request is kept like a CPDLC one; anything else is logged and alerted."""
        text = (packet or "").strip()
        pkt = {"min": None, "mrn": None, "ra": "", "text": text}
        if OCA_REQUEST_RE.search(text.upper()) and RCL_INTAKE:
            cs = sender.upper()
            with self.lock:
                now = self.now()
                row = self.db.execute("SELECT id FROM cpdlc_rcl WHERE station=? AND callsign=? AND state='open' AND ts > ?",
                                      (self.client.station, cs, now - RCL_REPEAT_SECONDS)).fetchone()
                if row:
                    self.db.execute("UPDATE cpdlc_rcl SET ts=?, text=? WHERE id=?", (now, text, row[0]))
                else:
                    self.db.execute("INSERT INTO cpdlc_rcl (ts, station, callsign, text) VALUES (?,?,?,?)", (now, self.client.station, cs, text))
                self.db.commit()
            self._store("DOWN", sender, None, None, "", "TELEX: " + text, state="", seen=0)
            try:
                self.client.request(sender, "telex", rcl_received_text(self.oca_name(sender)))
            except Exception as e:
                log.warning("telex reply to %s failed: %s", sender, e)
            return
        self._store("DOWN", sender, None, None, "", "TELEX: " + text, state="", seen=0)

    def _auto_reply(self, callsign, text, their_min, ra):
        """Answer a pilot straight away (the message is logged as sent by 'auto')."""
        cs = callsign.upper()
        mn = self._alloc_min()
        try:
            self._send_packet(cs, mn, their_min, ra, text)
        except RuntimeError as e:
            log.warning("auto reply to %s failed: %s", cs, e)
            return
        self._store("UP", cs, mn, their_min, ra, text, state="", by="auto")

    # --- handoff and UNICOM: tell the pilot, and when they answer WILCO/ROGER log them off
    def _start_pending(self, cs, row, kind):
        with self.lock:
            now = self.now()
            for k in [k for k, v in self.pending.items() if now - v["ts"] > PENDING_SECONDS]:
                del self.pending[k]
            self.pending[cs] = {"min": row["min"], "kind": kind, "ts": now}

    def handoff(self, callsign, name, freq="", by=""):
        cs = (callsign or "").strip().upper()
        name = re.sub(r"[^A-Za-z0-9 ]", "", name or "").strip().upper()
        freq = freq.strip() if re.fullmatch(r"1[0-3]\d\.\d{1,3}", (freq or "").strip()) and freq.strip() != "199.998" else ""
        if not name:
            raise ValueError("no station name")
        text = "CONTACT %s%s, BYE." % (name, " ON " + freq if freq else "")
        row = self.send(cs, text, ra="WU", by=by)
        self._start_pending(cs, row, "handoff")
        return row

    def unicom(self, callsign, by=""):
        cs = (callsign or "").strip().upper()
        row = self.send(cs, MSG_UNICOM, ra="WU", by=by)
        self._start_pending(cs, row, "unicom")
        return row

    def finish(self, callsign):
        """The pilot acknowledged: log them off CPDLC (and, after UNICOM, ask the plugin to release the tag)."""
        cs = callsign.upper()
        pend = self.pending.pop(cs, None)
        if not pend:
            return
        mn = self._alloc_min()
        try:
            self._send_packet(cs, mn, None, "NE", LOGOFF_TEXT)
            self._store("UP", cs, mn, None, "NE", LOGOFF_TEXT, state="", by="auto")
        except RuntimeError as e:
            log.warning("logoff of %s failed: %s", cs, e)
        self._set_connected(cs, False)
        if pend["kind"] == "unicom":
            self.on_release(cs)

    def disconnect(self, callsign, by=""):
        """Controller's choice: log this pilot off CPDLC from our station now (a stuck or unwanted connection).
        Sends the LOGOFF uplink (if Hoppie is unreachable the pilot is still dropped from our list). Returns the stored message."""
        cs = (callsign or "").strip().upper()
        if not re.fullmatch(r"[A-Z0-9]{2,8}", cs):
            raise ValueError("bad callsign")
        if not self.is_connected(cs):
            raise ValueError("%s is not connected to your CPDLC station" % cs)
        self.pending.pop(cs, None)
        mn = self._alloc_min()
        row = None
        try:
            self._send_packet(cs, mn, None, "NE", LOGOFF_TEXT)
            row = self._store("UP", cs, mn, None, "NE", LOGOFF_TEXT, state="", by=by or "controller")
        except RuntimeError as e:
            log.warning("logoff of %s failed: %s", cs, e)
        self._set_connected(cs, False)
        return {"ok": True, "callsign": cs, "sent": row is not None}

    def accept_logon(self, callsign, their_min=None):
        cs = callsign.upper()
        mn = self._alloc_min()
        self._send_packet(cs, mn, their_min, "NE", "LOGON ACCEPTED")
        self._store("UP", cs, mn, their_min, "NE", "LOGON ACCEPTED", state="", by="auto" if self.auto_accept else "")
        self._set_connected(cs, True)
        if SQUAWK_MSG:
            self._auto_reply(cs, MSG_SQUAWK, None, "NE")      # tell the pilot about squawk 2000 (replaces the plugin setting the code)
        self.entry_tick(cs)                                   # already at the entry point? then they get the contact message now

    def poll_once(self):
        try:
            answer = self.client.request("SERVER", "poll", "")
        except Exception as e:                                # network trouble: skip this round, do not retry straight away
            self.error = "poll failed: %s" % e
            return 0
        head, blocks, raw = parse_blocks(answer)
        if head != "ok":
            self.error = "Hoppie: " + answer.strip()[:160]
            return 0
        self.error = ""
        self.last_poll = self.now()
        n = 0
        for sender, type_, packet in blocks:
            if type_ == "cpdlc":
                try:
                    self.handle_cpdlc(sender, packet)
                except Exception:
                    log.exception("could not handle a CPDLC message from %s", sender)
                n += 1
            elif type_ == "telex":
                try:
                    self.handle_telex(sender, packet)
                except Exception:
                    log.exception("could not handle a telex from %s", sender)
                n += 1
        if n:
            self.fast_until = self.now() + FAST_POLL_SECONDS
        try:
            self.entry_tick()
        except Exception:
            log.exception("entry-point check failed")
        return n

    def ping_online(self, calls):
        """Which of these callsigns are online on Hoppie right now (a ping is not a CPDLC logon: any pilot whose client polls Hoppie answers).
        Returns a set, or None if Hoppie could not be asked."""
        calls = sorted({str(c).upper() for c in calls if re.fullmatch(r"[A-Z0-9]{2,8}", str(c).upper())})
        if not calls:
            return set()
        try:
            answer = self.client.request("SERVER", "ping", " ".join(calls))
        except Exception as e:
            self.error = "ping failed: %s" % e
            return None
        head, _, raw = parse_blocks(answer)
        if head != "ok":
            return None
        return {c for c in re.findall(r"[A-Z0-9]{2,8}", " ".join(raw).upper()) if c in calls}

    def asked_rcl(self, callsign):
        """True if this aircraft has an open oceanic clearance request here (by CPDLC or telex)."""
        cs = str(callsign).upper()
        return any(r["callsign"] == cs for r in self.rcl_requests())

    def ping_connected(self):
        """Drop aircraft that stopped being online on Hoppie (two misses in a row)."""
        with self.lock:
            calls = sorted(self.connected)
        if not calls:
            return
        try:
            answer = self.client.request("SERVER", "ping", " ".join(calls))
        except Exception as e:
            self.error = "ping failed: %s" % e
            return
        head, _, raw = parse_blocks(answer)
        if head != "ok":
            return
        online = set(re.findall(r"[A-Z0-9]{2,8}", " ".join(raw).upper()))
        for c in calls:
            with self.lock:
                v = self.connected.get(c)
                if not v:
                    continue
                if c in online:
                    v["misses"] = 0
                else:
                    v["misses"] += 1
                    if v["misses"] >= PING_MISSES_TO_DROP:
                        log.info("%s is no longer online on Hoppie - connection dropped", c)
                        self._set_connected(c, False)

    def stop(self):
        """Stop polling and forget the logon code. Messages stay in the log."""
        self.stop_flag = True
        self.client.logon = ""
        with self.lock:
            self.connected.clear()
            self.db.execute("DELETE FROM cpdlc_conn2 WHERE station=?", (self.client.station,))
            self.db.commit()

    def loop(self):
        while not self.stop_flag:
            try:
                self.poll_once()
                if self.stop_flag:
                    break
                if self.now() - self.last_ping > PING_SECONDS:
                    self.last_ping = self.now()
                    self.ping_connected()
            except Exception:
                log.exception("CPDLC loop error")
            wait = random.uniform(18, 22) if self.now() < self.fast_until else random.uniform(45, 75)
            while wait > 0 and not self.stop_flag:
                time.sleep(min(1.0, wait))
                wait -= 1.0


# ----------------------------------------------------------------------------- the hub: one station per signed-in controller
FORCE_LOCKOUT_SECONDS = 180          # after a forced log-out nobody may sign in to that station for this long (Hoppie keeps the old logon "online" for a while)
DEFAULT_STATIONS = {"CZQO": "CZQX", "EGGX": "EGGX", "NAT": "NATX"}       # position prefix -> CPDLC station callsign


def parse_stations(text):
    """'CZQO_CTR=CZQX,EGGX_CTR=EGGX' -> {'CZQO_CTR': 'CZQX', ...}. Empty/None -> {}."""
    out = {}
    for part in (text or "").split(","):
        if "=" in part:
            k, v = part.split("=", 1)
            k, v = k.strip().upper(), v.strip().upper()
            if k and re.fullmatch(r"[A-Z0-9]{2,8}", v):
                out[k] = v
    return out


class CpdlcHub:
    def __init__(self, db_path, url=DEFAULT_URL, auto_accept=True, stations=None, make_client=None, is_session_alive=None, now=time.time, start_threads=True, locate=None, facts=None):
        self.db_path, self.url, self.auto_accept, self.now = db_path, url, auto_accept, now
        self.locate = locate
        self.facts = facts
        self.release_times = {}
        self.stations_map = stations or {}
        self.make_client = make_client or (lambda code, station: HoppieClient(self.url, code, station))
        self.is_session_alive = is_session_alive or (lambda token: True)
        self.start_threads = start_threads
        self.lock = threading.RLock()
        self.running = {}        # station callsign -> {"token", "obj"}
        self.locked_until = {}   # station callsign -> time before which nobody may log on (after a forced log-out)
        self.kicked = {}         # web session token -> station it was removed from
        self.enabled = True

    def station_for(self, position):
        pos = (position or "").upper()
        if not pos or pos == "OBSERVER" or pos.endswith("_DEL"):      # observers and delivery positions do not use CPDLC
            return None
        if pos in self.stations_map:
            return self.stations_map[pos]
        if self.stations_map:                      # an explicit list means only those positions
            return None
        return DEFAULT_STATIONS.get(pos.split("_")[0])

    def reap(self):
        """Stop stations whose controller's web session has ended."""
        with self.lock:
            for st, v in list(self.running.items()):
                if not self.is_session_alive(v["token"]):
                    log.info("CPDLC station %s stopped (controller signed out)", st)
                    v["obj"].stop()
                    del self.running[st]

    def start(self, token, position, code):
        """Returns (station, None) or (None, error text)."""
        station = self.station_for(position)
        if not station:
            return None, "your position (%s) has no CPDLC station" % (position or "none")
        code = (code or "").strip()
        if not re.fullmatch(r"[A-Za-z0-9]{6,40}", code):
            return None, "that does not look like a Hoppie logon code"
        self.reap()
        wait = self.lockout_seconds(station)
        if wait > 0:
            return None, "%s was force-logged-out. Wait %d:%02d before signing in again" % (station, wait // 60, wait % 60)
        with self.lock:
            self.kicked.pop(token, None)
            cur = self.running.get(station)
            if cur and cur["token"] != token:
                return None, "%s is already in use by another controller" % station
        client = self.make_client(code, station)
        try:                                       # check the code with a harmless ping before going live
            answer = client.request("SERVER", "ping", station)
        except Exception as e:
            return None, "could not reach Hoppie: %s" % e
        if answer.strip().split(" ", 1)[0].lower() != "ok":
            return None, "Hoppie said: " + answer.strip()[:120]
        with self.lock:
            cur = self.running.get(station)
            if cur:                                # same controller re-entering a code
                cur["obj"].client.logon = code
                return station, None
            obj = CpdlcStation(self.db_path, client, auto_accept=self.auto_accept, now=self.now, locate=self.locate, on_release=self._mark_released, facts=self.facts)
            self.running[station] = {"token": token, "obj": obj}
        if self.start_threads:
            threading.Thread(target=obj.loop, daemon=True).start()
        log.info("CPDLC station %s started", station)
        return station, None

    def known_stations(self):
        return set(self.stations_map.values()) if self.stations_map else set(DEFAULT_STATIONS.values())

    def lockout_seconds(self, station):
        with self.lock:
            return max(0, int(self.locked_until.get((station or "").upper(), 0) - self.now() + 0.999))

    def force_logout(self, callsign, by=""):
        """Remove whoever holds this CPDLC station from this server and block new sign-ins for FORCE_LOCKOUT_SECONDS.
        Returns (seconds_locked, error). Raises nothing."""
        cs = (callsign or "").strip().upper()
        if cs not in self.known_stations():
            return 0, "%s is not a CPDLC station here (known: %s)" % (cs or "blank", ", ".join(sorted(self.known_stations())))
        with self.lock:
            cur = self.running.pop(cs, None)
            if cur:
                cur["obj"].stop()                       # polling stops and the Hoppie code is forgotten
                self.kicked[cur["token"]] = cs
            self.locked_until[cs] = self.now() + FORCE_LOCKOUT_SECONDS
        log.warning("CPDLC station %s force-logged-out by %s (%s)", cs, by or "unknown", "was in use" if cur else "was not in use")
        return FORCE_LOCKOUT_SECONDS, ""

    def stop(self, token):
        with self.lock:
            for st, v in list(self.running.items()):
                if v["token"] == token:
                    v["obj"].stop()
                    del self.running[st]
                    log.info("CPDLC station %s stopped", st)

    def get(self, token):
        """The running station owned by this web session, or None."""
        with self.lock:
            for v in self.running.values():
                if v["token"] == token:
                    return v["obj"]
        return None

    def station_for_sender(self, position):
        """A running station to send from: the one that belongs to this position if it is signed in, otherwise any signed-in station."""
        with self.lock:
            mine = self.station_for(position)
            if mine and mine in self.running:
                return self.running[mine]["obj"]
            for v in self.running.values():
                return v["obj"]
        return None

    def is_connected(self, callsign):
        with self.lock:
            return any(v["obj"].is_connected(callsign) for v in self.running.values())

    def station_with(self, callsign):
        """The running station this aircraft is connected to, or None."""
        with self.lock:
            for v in self.running.values():
                if v["obj"].is_connected(callsign):
                    return v["obj"]
        return None

    def handoff(self, callsign, target, freq="", by=""):
        """Plugin handoff: tell the pilot to contact the next station. Returns the message, or raises ValueError."""
        st = self.station_with(callsign.upper())
        if not st:
            raise ValueError("%s is not connected to CPDLC" % callsign)
        return st.handoff(callsign, station_display_name(target), freq, by=by)

    def _mark_released(self, callsign):
        with self.lock:
            self.release_times[callsign.upper()] = self.now()

    def released(self, callsign):
        with self.lock:
            ts = self.release_times.get((callsign or "").upper())
            return bool(ts and self.now() - ts < RELEASE_SECONDS)

    def status(self, token=None, position=""):
        mine = self.get(token) if token else None
        station = self.station_for(position)
        with self.lock:
            others = sorted({c for v in self.running.values() if v["obj"] is not mine for c in v["obj"].connected_callsigns()})
            taken = bool(station and station in self.running and not mine)
        if mine:
            st = mine.status()
            st.update({"active": True, "position": position, "stationForPosition": station, "inUse": False, "lockedSeconds": self.lockout_seconds(station)})
            return st
        with self.lock:
            was_kicked = self.kicked.get(token) if token else None
        return {"enabled": True, "active": False, "station": station or "", "stationForPosition": station or "", "inUse": taken,
                "lockedSeconds": self.lockout_seconds(station), "kickedFrom": was_kicked or "",
                "connected": [{"callsign": c, "since": 0, "unread": 0} for c in others], "unread": {}, "lastPoll": None, "error": "", "rclRequests": []}
