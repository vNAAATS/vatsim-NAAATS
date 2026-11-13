#!/usr/bin/env python3
"""Nightly copy of the vNAAATS database (flights, keys, sessions, voice-report flags).

Uses SQLite's own backup call, so it is safe while the server is running. Keeps the newest KEEP copies.
  python3 backup.py [database] [backup folder] [keep]
Defaults: /var/lib/vnaaats/vnaaats.db -> /var/lib/vnaaats/backups, keep 14.
"""
import glob, os, sqlite3, sys, time


def backup(db, folder, keep=14):
    if not os.path.isfile(db):
        raise SystemExit("No database at %s" % db)
    os.makedirs(folder, exist_ok=True)
    dest = os.path.join(folder, "vnaaats-%s.db" % time.strftime("%Y%m%d-%H%M%S", time.gmtime()))
    src = sqlite3.connect("file:%s?mode=ro" % db, uri=True)
    try:
        dst = sqlite3.connect(dest)
        try:
            src.backup(dst)
        finally:
            dst.close()
    finally:
        src.close()
    os.chmod(dest, 0o600)
    for old in sorted(glob.glob(os.path.join(folder, "vnaaats-*.db")))[:-max(1, keep)]:
        os.remove(old)
    return dest


if __name__ == "__main__":
    a = sys.argv[1:]
    print(backup(a[0] if a else "/var/lib/vnaaats/vnaaats.db", a[1] if len(a) > 1 else "/var/lib/vnaaats/backups", int(a[2]) if len(a) > 2 else 14))
