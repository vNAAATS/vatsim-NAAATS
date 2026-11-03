"""
Builds the oceanic clearance messages a controller sends to a pilot who asked for it by CPDLC.
Nothing here sends anything: the controller sees the text, can edit it, and presses send.
"""
import re
from datetime import datetime, timezone

PROCESSED = "RCL PROCESSED - FLY FLIGHT PLAN ROUTE"


def _level(v):
    m = re.search(r"\d{2,3}", str(v or ""))
    return "F%03d" % int(m.group(0)) if m else ""


def _mach(v):
    """0.85 / '.85' / 85 / '085' -> M085"""
    s = str(v or "").strip()
    if not s:
        return ""
    try:
        f = float(s)
    except ValueError:
        return ""
    if f >= 10:
        f = f / 100.0
    return "M%03d" % int(round(f * 100))


def contact_line(position):
    """Which controller to tell the pilot to call, from the position that sends it (Gander, Shanwick or NAT FSS)."""
    p = (position or "").upper()
    if p.startswith(("CZQO", "CZQX")):
        name = "GANDER"
    elif p.startswith("EGGX"):
        name = "SHANWICK"
    else:
        name = "NAT FSS"
    return "CONTACT %s 5 MINS PRIOR ENTRY, STATE FL, MACH AND SELCAL" % name


def _entry(route, etas, fix):
    """(entry fix, its time HHMM, the route from that fix on) using natTrak's fix when it has one."""
    fixes = str(route or "").split()
    toks = str(etas or "").split()
    idx = fixes.index(fix) if fix and fix in fixes else None
    if idx is None:
        for i, t in enumerate(toks):
            if re.fullmatch(r"\d{4}", t) and i < len(fixes):
                idx = i
                break
    if idx is None:
        return (fix or ""), "", ""
    t = toks[idx] if idx < len(toks) and re.fullmatch(r"\d{4}", toks[idx]) else ""
    return fixes[idx], t, " ".join(fixes[idx:])


def build_clearance(rec, nt, now=None):
    """The CLX text. rec = flight record (our data), nt = natTrak's entry for the flight (wins where it has a value)."""
    rec, nt = rec or {}, nt or {}
    now = now or datetime.now(timezone.utc)
    nat = str(nt.get("nat") or rec.get("track") or "").upper()
    fix, eta, rte = _entry(rec.get("route", ""), rec.get("routeEtas", ""), str(nt.get("fix") or "").upper())
    lvl = _level(nt.get("level") or rec.get("assignedLevel"))
    mach = _mach(nt.get("mach") or rec.get("assignedMach"))
    cs = str(rec.get("callsign") or nt.get("callsign") or "").upper()
    dest = str(rec.get("arrival") or nt.get("destination") or "").upper()
    parts = ["CLX %s %s NAT CLRNCE" % (now.strftime("%H%M"), now.strftime("%d%m%y")),
             "%s CLRD TO %s VIA %s" % (cs, dest or "????", fix or "????")]
    if nat and nat not in ("RR", "RANDOM"):
        parts.append("NAT " + nat)
    else:
        parts.append("RANDOM ROUTE")
    if rte:
        parts.append(rte)
    parts.append("FM %s%s MNTN %s %s" % (fix or "????", "/" + eta if eta else "", lvl or "????", mach or "????"))
    parts.append("SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA")
    parts.append("END OF MESSAGE")
    return " / ".join(parts)


def build(kind, rec, nt, position, now=None):
    """The list of uplinks for 'clx' (reclearance) or 'processed' (no change)."""
    first = build_clearance(rec, nt, now) if kind == "clx" else PROCESSED
    return [first, contact_line(position)]
