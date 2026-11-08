"""Next handoff for an oceanic flight: which neighbour it leaves to, how far away that is, and who is online to take it.

The exit-point lists come from the Shanwick & Gander Oceanic Control Procedures (Figure 8 for the Shanwick OCA, Figure 17 for the Gander OCA).
A flight that crosses 30W between the two oceanic areas is handed to the other oceanic centre there. Edit EXIT_FACILITY if the boundaries change.
"""
import json
import math
import os

def _load_codes():
    """callsign -> EuroScope short code (the position id in the sector file's [POSITIONS], e.g. CZQM_CTR -> QM). Made from the .ese."""
    try:
        with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "position_codes.json"), encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return {}
CODES = _load_codes()

SOON_NM = 50                    # "hand off soon" prompt distance
MAX_OFF_ROUTE_NM = 150          # further than this from the filed route and we do not guess
CROSS_LON = -30.0               # Shanwick / Gander boundary

# facility -> callsign prefixes of the positions that cover it
FACILITY = {
    "EGPX": ("EGPX", "SCO_"), "BIRD": ("BIRD",), "EISN": ("EISN",), "LFRR": ("LFRR", "LFXX"), "LECM": ("LECM",),
    "LPPO": ("LPPO",), "LPPC": ("LPPC",), "KZWY": ("KZWY", "NY_"), "CZQX": ("CZQX", "CZQM"), "CZUL": ("CZUL", "MTL_"),
    "CZEG": ("CZEG",), "BGGL": ("BGGL",), "EGGX": ("EGGX", "GGX"), "CZQO": ("CZQO", "ZQO"),
}
NAMES = {"EGPX": "Scottish", "BIRD": "Reykjavik", "EISN": "Shannon", "LFRR": "Brest", "LECM": "Madrid", "LPPO": "Santa Maria", "LPPC": "Lisboa",
         "KZWY": "New York Oceanic", "CZQX": "Gander Domestic", "CZUL": "Montreal", "CZEG": "Edmonton", "BGGL": "Nuuk", "EGGX": "Shanwick", "CZQO": "Gander Oceanic"}

def _facs(fac, names):
    return {n: fac for n in names.split()}
EXIT_FACILITY = {}
EXIT_FACILITY.update(_facs("BIRD", "RATSU"))                                         # the RATSU triangle is Reykjavik East
EXIT_FACILITY.update(_facs("EGPX", "ATSIX BALIX ERAKA GOMUP MIMKU NIBOG"))
EXIT_FACILITY.update(_facs("EISN", "SUNOT PIKIL RESNO DOGAL MALOT LIMRI DINIM SOMAX BEDRA OMOKO TAMEL LASNO"))   # includes the NOTA and SOTA
EXIT_FACILITY.update(_facs("LFRR", "ETIKI SEPAL SIVIR REGHI LAPEX RIVAX"))          # Brest (BOTA)
EXIT_FACILITY.update(_facs("LECM", "BEGAS"))
EXIT_FACILITY.update(_facs("LPPO", "GONAN PASAS DIXIS"))
EXIT_FACILITY.update(_facs("LPPC", "HIDRA MUDOS RASIX"))
EXIT_FACILITY.update(_facs("CZQX", "MUSVA KAGLY BERUS IKMAN GRIBS MIBNO PEPKI MOATT PRAWN PORGY SCROD HOIST JANJO ALLRY ELSIR JOOPY NICSO PORTI SUPRY RAFIN BOBTU"))
EXIT_FACILITY.update(_facs("CZEG", "AVPUT KENKI EPMAN"))


def _xy(lat, lon, lat0):
    return lon * 60.0 * math.cos(math.radians(lat0)), lat * 60.0

def _nm(a, b):
    p1, p2 = math.radians(a[0]), math.radians(b[0])
    h = math.sin((p2 - p1) / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(math.radians(b[1] - a[1]) / 2) ** 2
    return 3440.065 * 2 * math.asin(min(1.0, math.sqrt(h)))

def locate(points, pos):
    """points: [(name, lat, lon)]. Returns (segment index, fraction along it, nm off the route) of the closest point on the route, or None."""
    best = None
    for i in range(len(points) - 1):
        a, b = points[i], points[i + 1]
        lat0 = (a[1] + b[1]) / 2
        ax, ay = _xy(a[1], a[2], lat0); bx, by = _xy(b[1], b[2], lat0); px, py = _xy(pos[0], pos[1], lat0)
        dx, dy = bx - ax, by - ay
        t = 0.0 if dx == 0 and dy == 0 else max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy)))
        d = math.hypot(px - (ax + t * dx), py - (ay + t * dy))
        if best is None or d < best[2]:
            best = (i, t, d)
    return best

def _along(points, seg, t, tseg, tt):
    """nm along the route from (seg, t) forward to (tseg, tt)."""
    def pt(i, f):
        a, b = points[i], points[i + 1]
        return (a[1] + (b[1] - a[1]) * f, a[2] + (b[2] - a[2]) * f)
    if (tseg, tt) <= (seg, t):
        return 0.0
    if tseg == seg:
        return _nm(pt(seg, t), pt(tseg, tt))
    total = _nm(pt(seg, t), (points[seg + 1][1], points[seg + 1][2]))
    for i in range(seg + 1, tseg):
        total += _nm((points[i][1], points[i][2]), (points[i + 1][1], points[i + 1][2]))
    return total + _nm((points[tseg][1], points[tseg][2]), pt(tseg, tt))

def next_handoff(points, pos):
    """The next boundary this flight reaches. Returns {"facility", "fix", "nm"} or None when it cannot be told."""
    if len(points) < 2 or not pos:
        return None
    loc = locate(points, pos)
    if not loc or loc[2] > MAX_OFF_ROUTE_NM:
        return None
    seg, t, _ = loc
    events = []
    for i in range(len(points) - 1):                                   # the 30W crossing between the two oceanic centres
        a, b = points[i], points[i + 1]
        if a[2] != b[2] and (a[2] - CROSS_LON) * (b[2] - CROSS_LON) <= 0 and (i, (CROSS_LON - a[2]) / (b[2] - a[2])) > (seg, t):
            f = (CROSS_LON - a[2]) / (b[2] - a[2])
            fac = "CZQO" if b[2] < a[2] else "EGGX"                      # moving west: to Gander; moving east: to Shanwick
            events.append((_along(points, seg, t, i, f), fac, "30W"))
            break
    last = points[-1]
    fac = EXIT_FACILITY.get(last[0].upper())
    if fac:
        events.append((_along(points, seg, t, len(points) - 2, 1.0), fac, last[0].upper()))
    if not events:
        return None
    nm, fac, fix = min(events)
    return {"facility": fac, "fix": fix, "nm": int(round(nm))}

def pick_online(facility, controllers):
    """The online control positions that cover this facility (callsigns), the facility's own positions before stand-ins, then by callsign."""
    pre = FACILITY.get(facility, ())
    out = []
    for c in controllers:
        cs = str(c.get("callsign", "")).upper()
        if cs.startswith(pre) and cs.endswith(("_CTR", "_FSS")):
            out.append((min(i for i, x in enumerate(pre) if cs.startswith(x)), cs, c))
    return [c for _, _, c in sorted(out, key=lambda t: (t[0], t[1]))]

def hint(points, pos, controllers):
    """Everything the display needs for one flight, or None."""
    nh = next_handoff(points, pos)
    if not nh:
        return None
    online = pick_online(nh["facility"], controllers)
    def code(c):
        return CODES.get(str(c["callsign"]).upper(), "")
    nh.update({"name": NAMES.get(nh["facility"], nh["facility"]), "soon": nh["nm"] <= SOON_NM,
               "controller": online[0]["callsign"] if online else "", "frequency": online[0].get("frequency", "") if online else "",
               "code": code(online[0]) if online else "", "unicom": not online,
               "online": [{"callsign": c["callsign"], "code": code(c), "frequency": c.get("frequency", "")} for c in online[:6]]})
    return nh
