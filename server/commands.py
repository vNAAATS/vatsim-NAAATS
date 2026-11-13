"""
Commands from the flight data display to a controller's EuroScope plugin.

A web page cannot talk to EuroScope, so the FDD puts a command in this queue (addressed to the controller's
position callsign, e.g. CZQO_CTR) and that controller's plugin collects it with /PluginCommands a few seconds later,
carries it out, and reports the result with /PluginCommandDone.  The FDD watches the result with /cmd/status.

Commands: track (start tracking + clearance, with level / Mach / SELCAL), untrack, transfer (handoff to a controller).
(EuroScope's own dot commands .selcal / .contactme / .wallop cannot be run by a plugin; the FDD copies them instead. .selcal and .contactme act on the selected aircraft, so the plugin selects it first.)
"""
import re
import threading
import time

TYPES = ("track", "untrack", "transfer", "selcal", "contactme", "natmsg", "planget", "planset", "highlight", "takeover")   # planget/planset/selcal/contactme/natmsg are typed into EuroScope by the plugin (autotype=1)
TTL_SECONDS = 60           # an unclaimed command is dropped after this (old clicks must not fire minutes later)
KEEP_SECONDS = 600         # finished commands stay readable this long
SELCAL_OK = re.compile(r"[A-Z]{4}")   # VATSIM pilots can use any four letters
CALLSIGN_OK = re.compile(r"[A-Z0-9_\-]{2,12}")


RELIEF_CPDLC_TEXT = "THERE IS A CONTROLLER CHANGE IN PROGRESS WHICH REQUIRES YOU TO RE LOG INTO THE CPDLC STATIONS ONCE YOU RECEIVE A LOG OFF MESSAGE"

NATMSG = ("Hello, you will soon be entering oceanic airspace. Please send your oceanic clearance request (RCL) by CPDLC, or via nattrak.vatsim.net, "
          "between 90 and 30 minutes before your oceanic entry point, and include the highest level you can accept (MAX FL). Squawk 2000 at the oceanic entry point.")

# The Auto RCL version: says it is automated and points to the website (the manual RCL REQ button keeps the plain NATMSG above)
NATMSG_AUTO = ("*THIS IS AN AUTOMATED MESSAGE* - Your flight plan has you entering the NAT Oceanic Airspace. "
               "You are required to send a request for oceanic clearance (RCL), between 90 and 30 minutes before your oceanic entry point. "
               "Include in this request the highest flight level you can accept (MAX FL). "
               "This can be sent via CPDLC, or via nattrak.vatsim.net. "
               "Set Squawk 2000 at the oceanic entry point and contact oceanic controller with your Callsign FL MACH and SELCAL code on first contact. "
               "Further details and assistance can be found at https://ganderoceanic.ca/")


PLAN_ICAO = re.compile(r"[A-Z0-9]{3,4}")
PLAN_AC = re.compile(r"[A-Z0-9/\-]{1,30}")
PLAN_ROUTE = re.compile(r"[A-Z0-9/ ._+\-]{0,1500}")
PLAN_REMARKS = re.compile(r"[ -~]{0,500}")      # printable ASCII only


def clean_plan(p):
    """Check the flight plan fields sent by the FDD edit window. Returns (plan dict, error)."""
    if not isinstance(p, dict):
        return None, "no flight plan sent"
    out = {}
    plan_type = str(p.get("planType", "I")).strip().upper()
    if plan_type not in ("I", "V"):
        return None, "flight rules must be IFR or VFR"
    out["planType"] = plan_type
    ac = str(p.get("acInfo", "")).strip().upper()
    if not PLAN_AC.fullmatch(ac):
        return None, "aircraft data looks like H/B77W/L (equipment letters allowed)"
    out["acInfo"] = ac
    for key, label, optional in (("origin", "departure airport", False), ("dest", "destination airport", False), ("alternate", "alternate airport", True)):
        v = str(p.get(key, "")).strip().upper()
        if v == "" and optional:
            out[key] = ""
            continue
        if not PLAN_ICAO.fullmatch(v):
            return None, "%s must be an airport code like EGLL" % label
        out[key] = v
    nums = (("tas", 0, 999, "TAS"), ("alt", 0, 60000, "altitude (feet)"), ("enrouteH", 0, 99, "enroute hours"), ("enrouteM", 0, 59, "enroute minutes"),
            ("fuelH", 0, 99, "fuel hours"), ("fuelM", 0, 59, "fuel minutes"))
    for key, lo, hi, label in nums:
        try:
            v = int(str(p.get(key, 0)).strip() or 0)
        except ValueError:
            return None, "%s must be a number" % label
        if not lo <= v <= hi:
            return None, "%s must be %d to %d" % (label, lo, hi)
        out[key] = v
    etd = str(p.get("etd", "")).strip()
    if not re.fullmatch(r"([01]\d|2[0-3])[0-5]\d", etd):
        return None, "departure time is four digits, for example 1435"
    out["etd"] = etd
    route = str(p.get("route", "")).strip().upper()
    if not PLAN_ROUTE.fullmatch(route):
        return None, "the route has characters that are not allowed (letters, numbers, / . + - and spaces)"
    out["route"] = route
    remarks = str(p.get("remarks", ""))
    if not PLAN_REMARKS.fullmatch(remarks):
        return None, "remarks may only use normal keyboard characters (max 500)"
    out["remarks"] = remarks
    return out, None


def clean_command(body):
    """Check a command from the FDD. Returns (type, callsign, args, error)."""
    typ = str(body.get("type", "")).lower()
    if typ not in TYPES:
        return None, None, None, "unknown command"
    cs = str(body.get("callsign", "")).strip().upper()
    if not re.fullmatch(r"[A-Z0-9]{2,8}", cs):
        return None, None, None, "bad callsign"
    args = {}
    if typ == "track":
        raw_level = str(body.get("level", "")).strip()
        raw_mach = str(body.get("mach", "")).strip()
        try:
            level = int(raw_level) if raw_level else 0               # blank = leave it as it is (0 = not given)
            mach = (int(raw_mach.lstrip("Mm.").lstrip("0") or 0) if raw_mach else 0)
        except ValueError:
            return None, None, None, "level and Mach must be numbers"
        if raw_level and not 1 <= level <= 999:
            return None, None, None, "level must be 1-999 (e.g. 350)"
        if raw_mach and not 1 <= mach <= 299:
            return None, None, None, "Mach must be e.g. 84 for M0.84"
        sel = re.sub(r"[-\s]", "", str(body.get("selcal", "")).upper())
        if sel and not SELCAL_OK.fullmatch(sel):
            return None, None, None, "SELCAL must be exactly 4 letters, like ABCD"
        args = {"level": level, "mach": mach, "selcal": sel}
    elif typ == "transfer":
        target = str(body.get("target", "")).strip().upper()
        if not CALLSIGN_OK.fullmatch(target):
            return None, None, None, "choose a controller to transfer to"
        freq = str(body.get("freq", "")).strip()
        args = {"target": target, "freq": freq if re.fullmatch(r"1[0-3]\d\.\d{1,3}", freq) else ""}
    elif typ == "selcal":
        sel = re.sub(r"[-\s]", "", str(body.get("selcal", "")).upper())
        if not SELCAL_OK.fullmatch(sel):
            return None, None, None, "SELCAL must be exactly 4 letters, like ABCD"
        args = {"text": ".selcal " + sel}
    elif typ == "contactme":
        args = {"text": ".contactme"}        # EuroScope's .contactme takes no callsign: it acts on the selected aircraft (the plugin selects it first)
    elif typ == "natmsg":
        args = {"text": ".msg " + cs + " " + (NATMSG_AUTO if body.get("auto") else NATMSG)}
    elif typ in ("planget", "highlight", "takeover"):      # highlight: the plugin circles the aircraft in white for a few seconds
        args = {}
    elif typ == "planset":
        plan, err = clean_plan(body.get("plan"))
        if err:
            return None, None, None, err
        args = {"plan": plan}
    return typ, cs, args, None


class CommandQueue:
    def __init__(self, now=time.time):
        self.now = now
        self.lock = threading.Lock()
        self.items = {}          # id -> dict
        self.next_id = 1

    def _clean(self):
        t = self.now()
        for i in [i for i, c in self.items.items()
                  if (c["state"] == "pending" and t - c["ts"] > TTL_SECONDS) or t - c["ts"] > KEEP_SECONDS + TTL_SECONDS]:
            del self.items[i]

    def add(self, target, typ, callsign, args, by=""):
        with self.lock:
            self._clean()
            cid = self.next_id
            self.next_id += 1
            self.items[cid] = {"id": cid, "target": target.upper(), "type": typ, "callsign": callsign, "args": args,
                               "by": by, "state": "pending", "msg": "", "ts": self.now()}
            return cid

    def pending_for(self, me):
        """Commands addressed to this controller callsign; they are marked delivered so they are handed out once."""
        out = []
        with self.lock:
            self._clean()
            for c in self.items.values():
                if c["state"] == "pending" and c["target"] == (me or "").upper():
                    c["state"] = "delivered"
                    c["delivered_ts"] = self.now()
                    out.append({"id": c["id"], "type": c["type"], "callsign": c["callsign"], "args": c["args"], "by": c["by"]})
        return out

    def done(self, cid, ok, msg=""):
        """Plugin result. Returns the command (for follow-up actions) or None."""
        with self.lock:
            c = self.items.get(cid)
            if not c or c["state"] not in ("delivered", "pending"):
                return None
            c["state"] = "ok" if ok else "failed"
            c["msg"] = (msg or "")[:(6000 if c["type"] == "planget" else 200)]    # planget carries the whole flight plan as JSON
            return dict(c)

    def status(self, cid):
        with self.lock:
            c = self.items.get(cid)
            if not c:
                return None
            state = c["state"]
            if state == "pending" and self.now() - c["ts"] > 20:
                state = "waiting"          # nobody has collected it: the plugin is not running / not that callsign
            return {"id": cid, "state": state, "msg": c["msg"], "type": c["type"], "callsign": c["callsign"]}
