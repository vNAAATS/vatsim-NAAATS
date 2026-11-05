"""
Personal plugin keys and FDD passwords for members.

The Gander Oceanic website (which knows who is on the roster, and where the member is signed in with VATSIM)
asks this server to issue a member a key and a password:   POST /admin/member/issue  (protected by VNAAATS_ISSUER_SECRET).
  * the KEY goes in the member's vnaaats.cfg (key=...). It looks like  <cid>.<secret>  and is only valid for DAYS days.
  * the PASSWORD is typed on the FDD sign-in page together with the member's name or CID.
Only hashes are stored here, so a copy of the database does not leak working keys or passwords.
A nightly call to /admin/member/sync with the CIDs that are still certified revokes everyone else.
"""
import hashlib
import os
import hmac
import re
import secrets
import threading
import time

DAYS = 90
PBKDF_ROUNDS = 100_000
LAST_USED_EVERY = 300          # seconds: how often last_used is written for a busy key
PW_ALPHABET = "abcdefghjkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789"    # no look-alike characters


def _hash_key(secret):
    return hashlib.sha256(secret.encode()).hexdigest()


def _hash_pw(password, salt):
    return hashlib.pbkdf2_hmac("sha256", password.encode(), bytes.fromhex(salt), PBKDF_ROUNDS).hex()


class MemberKeys:
    def __init__(self, store, now=time.time):
        self.store, self.now = store, now
        self.touched = {}                                   # cid -> last time last_used was written
        self.lock = threading.Lock()
        with store.lock:
            store.db.execute("""CREATE TABLE IF NOT EXISTS members (cid TEXT PRIMARY KEY, name TEXT NOT NULL,
                key_hash TEXT NOT NULL, pw_salt TEXT NOT NULL, pw_hash TEXT NOT NULL,
                created REAL NOT NULL, expires REAL NOT NULL, revoked INTEGER NOT NULL DEFAULT 0, last_used REAL)""")
            try:
                store.db.execute("ALTER TABLE members ADD COLUMN manual INTEGER NOT NULL DEFAULT 0")   # 1 = temporary key handed out by the admin by hand
            except Exception:
                pass                                                                                  # column already there
            for col in ("email", "issued_ip"):                                                       # who asked for the key (kept for following up misuse)
                try:
                    store.db.execute("ALTER TABLE members ADD COLUMN %s TEXT NOT NULL DEFAULT ''" % col)
                except Exception:
                    pass
            store.db.commit()

    def any(self):
        with self.store.lock:
            return self.store.db.execute("SELECT 1 FROM members WHERE revoked=0 LIMIT 1").fetchone() is not None

    # --- issuing
    @staticmethod
    def blocked_file():
        return os.environ.get("VNAAATS_BLOCKED_CIDS", "/var/lib/vnaaats/blocked-cids.txt")

    def blocked(self):
        """CIDs on the blocked list (one per line, # for notes). Read each time, so a change takes effect at once."""
        try:
            with open(self.blocked_file(), encoding="utf-8") as f:
                return {d for d in (re.sub(r"\D", "", ln.split("#")[0]) for ln in f) if d}
        except OSError:
            return set()

    def is_blocked(self, cid):
        return re.sub(r"\D", "", str(cid))[:9] in self.blocked()

    def issue(self, cid, name, days=None, manual=False, email="", ip=""):
        cid = re.sub(r"\D", "", str(cid))[:9]
        if cid and cid in self.blocked():
            raise ValueError("this CID is blocked")
        name = " ".join(str(name or "").split())[:60]
        if not cid:
            raise ValueError("a CID is required")
        key = "%s.%s" % (cid, secrets.token_urlsafe(24))
        password = "".join(secrets.choice(PW_ALPHABET) for _ in range(12))
        salt = secrets.token_hex(16)
        t = self.now()
        expires = t + (days if days else DAYS) * 86400
        with self.store.lock:
            self.store.db.execute("INSERT OR REPLACE INTO members (cid, name, key_hash, pw_salt, pw_hash, created, expires, revoked, last_used, manual, email, issued_ip) VALUES (?,?,?,?,?,?,?,0,NULL,?,?,?)",
                                  (cid, name or cid, _hash_key(key.split(".", 1)[1]), salt, _hash_pw(password, salt), t, expires, 1 if manual else 0, str(email or "")[:120], str(ip or "")[:64]))
            self.store.db.commit()
        return {"key": key, "password": password, "expires": expires, "cid": cid, "name": name or cid}

    # --- checking
    def check_key(self, code):
        """Returns (cid, "") for a good key, or (None, reason) with reason "unknown", "expired" or "revoked"."""
        m = re.fullmatch(r"(\d{1,9})\.([A-Za-z0-9_\-]{16,64})", str(code or ""))
        if not m:
            return None, "unknown"
        cid, secret = m.groups()
        with self.store.lock:
            row = self.store.db.execute("SELECT key_hash, expires, revoked FROM members WHERE cid=?", (cid,)).fetchone()
        if not row or not hmac.compare_digest(row[0], _hash_key(secret)):
            return None, "unknown"
        if row[2] or cid in self.blocked():
            return None, "revoked"
        if row[1] < self.now():
            return None, "expired"
        self._touch(cid)
        return cid, ""

    def _touch(self, cid):
        t = self.now()
        with self.lock:
            if t - self.touched.get(cid, 0) < LAST_USED_EVERY:
                return
            self.touched[cid] = t
        with self.store.lock:
            self.store.db.execute("UPDATE members SET last_used=? WHERE cid=?", (t, cid))
            self.store.db.commit()

    def check_login(self, who, password):
        """FDD sign-in with the member's CID or name and personal password. Returns {"cid","name"} or None."""
        who = " ".join(str(who or "").split())
        if not who or not password:
            return None
        digits = re.sub(r"\D", "", who)
        with self.store.lock:
            if who.isdigit():
                rows = self.store.db.execute("SELECT cid, name, pw_salt, pw_hash, expires, revoked FROM members WHERE cid=?", (digits,)).fetchall()
            else:
                rows = self.store.db.execute("SELECT cid, name, pw_salt, pw_hash, expires, revoked FROM members WHERE lower(name)=lower(?)", (who,)).fetchall()
        for cid, name, salt, pw_hash, expires, revoked in rows:
            if hmac.compare_digest(pw_hash, _hash_pw(password, salt)) and not revoked and expires >= self.now() and cid not in self.blocked():
                self._touch(cid)
                return {"cid": cid, "name": name}
        return None

    # --- admin
    def info(self, cid):
        cid = re.sub(r"\D", "", str(cid))[:9]
        with self.store.lock:
            row = self.store.db.execute("SELECT name, created, expires, revoked, last_used FROM members WHERE cid=?", (cid,)).fetchone()
        if not row:
            return {"exists": False}
        name, created, expires, revoked, last_used = row
        return {"exists": True, "name": name, "created": created, "expires": expires, "revoked": bool(revoked),
                "expired": expires < self.now(), "lastUsed": last_used}

    def revoke(self, cid):
        cid = re.sub(r"\D", "", str(cid))[:9]
        with self.store.lock:
            cur = self.store.db.execute("UPDATE members SET revoked=1 WHERE cid=? AND revoked=0", (cid,))
            self.store.db.commit()
        return cur.rowcount > 0

    def sync(self, active_cids):
        """Revoke every member whose CID is not in the list of members who are still certified."""
        keep = {re.sub(r"\D", "", str(c)) for c in active_cids}
        with self.store.lock:
            rows = [r[0] for r in self.store.db.execute("SELECT cid FROM members WHERE revoked=0 AND manual=0")]
        gone = [c for c in rows if c not in keep]
        for c in gone:
            self.revoke(c)
        return gone

    def listing(self):
        with self.store.lock:
            rows = self.store.db.execute("SELECT cid, name, created, expires, revoked, last_used, manual, email, issued_ip FROM members ORDER BY name").fetchall()
        t = self.now()
        return [{"cid": c, "name": n, "created": cr, "expires": ex, "revoked": bool(rv), "expired": ex < t, "lastUsed": lu, "manual": bool(mn), "email": em, "ip": ip} for c, n, cr, ex, rv, lu, mn, em, ip in rows]


class RosterWatch:
    """Reads the Gander Oceanic roster feed (public JSON: [{"cid":..,"certification":"certified","active":1,..}, ...])
    and keeps the set of CIDs that are certified (endorsed on the roster; active or not). Used to refuse a key to anyone else and, every few hours,
    to withdraw the keys of members who have dropped off. A bad or tiny answer never revokes anything."""
    MIN_ENTRIES = 5            # a roster smaller than this is treated as a broken answer
    MAX_REVOKE_FRACTION = 0.5  # never withdraw more than half of the keys in one go

    def __init__(self, members, url, fetch=None, now=time.time):
        self.members, self.url, self.now = members, url, now
        self.fetch = fetch or self._http
        self.lock = threading.Lock()
        self.ok_cids, self.updated, self.error = None, None, ""

    @staticmethod
    def _http_get(url):
        import json
        import urllib.request
        req = urllib.request.Request(url, headers={"User-Agent": "vnaaats-server"})
        with urllib.request.urlopen(req, timeout=30) as r:
            return json.loads(r.read().decode("utf-8"))

    def _http(self):
        return self._http_get(self.url)

    def refresh(self):
        """Fetch the roster. Returns True when we now hold a good copy."""
        try:
            data = self.fetch()
            if not isinstance(data, list) or len(data) < self.MIN_ENTRIES:
                raise ValueError("roster answer looks wrong (%s entries)" % (len(data) if isinstance(data, list) else "no"))
            ok = {str(e["cid"]) for e in data if isinstance(e, dict) and e.get("certification") == "certified"}      # certified is enough: active status does not matter
        except Exception as e:
            with self.lock:
                self.error = str(e)
            return False
        with self.lock:
            self.ok_cids, self.updated, self.error = ok, self.now(), ""
        return True

    def eligible(self, cid):
        """True/False from the roster; None when we cannot tell (no good copy and the fetch failed)."""
        with self.lock:
            stale = self.ok_cids is None or self.now() - (self.updated or 0) > 3600
        if stale:
            self.refresh()
        with self.lock:
            if self.ok_cids is None:
                return None
            return re.sub(r"\D", "", str(cid)) in self.ok_cids

    def revoke_dropped(self):
        """Withdraw keys of members no longer certified. Returns the CIDs revoked."""
        if not self.refresh():
            return []
        with self.lock:
            ok = set(self.ok_cids)
        live = [m["cid"] for m in self.members.listing() if not m["revoked"] and not m["manual"]]
        gone = [c for c in live if c not in ok]
        if live and len(gone) > len(live) * self.MAX_REVOKE_FRACTION and len(gone) > 2:
            with self.lock:
                self.error = "refused to revoke %d of %d keys in one go - check the roster feed" % (len(gone), len(live))
            return []
        for c in gone:
            self.members.revoke(c)
        return gone

    def loop(self, every=6 * 3600):
        import logging
        log = logging.getLogger("vnaaats")
        while True:
            try:
                gone = self.revoke_dropped()
                if gone:
                    log.info("Roster check withdrew %d member key(s)", len(gone))
                if self.error:
                    log.warning("Roster check: %s", self.error)
            except Exception:
                log.exception("roster check failed")
            time.sleep(every)
