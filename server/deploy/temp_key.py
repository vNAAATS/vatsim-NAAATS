#!/usr/bin/env python3
"""Hand out a temporary vNAAATS key + Flight Data Display password for one controller (run on the server, as root).

    python3 /opt/vnaaats/deploy/temp_key.py 1017951 "Andrew Morkunas" 7

Only give one out after checking the person is shown as Certified on https://ganderoceanic.ca/roster.
It is not checked against the roster feed, lasts the number of days you give (default 7, max 30), and the roster
clean-up never withdraws it. To cancel early:  python3 temp_key.py --revoke 1017951
"""
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
for d in (os.path.join(HERE, ".."), "/opt/vnaaats"):
    if os.path.exists(os.path.join(d, "members.py")):
        sys.path.insert(0, d)
        break
import members as members_mod
from vnaaats_server import FlightStore


def main(argv):
    env = {}
    try:
        for line in open("/etc/vnaaats.env"):
            if "=" in line and not line.startswith("#"):
                k, v = line.strip().split("=", 1)
                env[k] = v.strip().strip('"')
    except OSError:
        pass
    db = env.get("VNAAATS_DB") or os.environ.get("VNAAATS_DB") or os.path.join(os.path.dirname(members_mod.__file__), "vnaaats.db")
    m = members_mod.MemberKeys(FlightStore(db))
    if len(argv) == 3 and argv[1] == "--revoke":
        print("revoked" if m.revoke(argv[2]) else "no active key for that CID")
        return 0
    if len(argv) < 2:
        print(__doc__)
        return 1
    cid = argv[1]
    name = argv[2] if len(argv) > 2 else cid
    days = max(1, min(30, int(argv[3]))) if len(argv) > 3 else 7
    out = m.issue(cid, name, days=days, manual=True)
    print("\nCID:       %s\nKey:       %s\nPassword:  %s\nExpires:   %s (%d days)\n" % (
        out["cid"], out["key"], out["password"], time.strftime("%d %b %Y %H:%M UTC", time.gmtime(out["expires"])), days))
    print("Send these to the controller privately. The key goes in vnaaats.cfg (key=...). The password is for the Flight Data Display.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
