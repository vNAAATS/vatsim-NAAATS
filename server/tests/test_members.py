import json, os, sys, tempfile, threading, time, unittest, urllib.error, urllib.request
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import members as mb
import vnaaats_server as s

SECRET = "i" * 30
SHARED = "k" * 20


class MemberTests(unittest.TestCase):
    def setUp(self):
        self.store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "m.db"))
        self.t = [1000.0]
        self.m = mb.MemberKeys(self.store, now=lambda: self.t[0])

    def test_issue_and_check(self):
        out = self.m.issue("1234567", "Jane Pilot")
        self.assertTrue(out["key"].startswith("1234567."))
        self.assertEqual(self.m.check_key(out["key"]), ("1234567", ""))
        self.assertEqual(self.m.check_key("1234567.wrongwrongwrongwrongwrong")[1], "unknown")
        self.assertEqual(self.m.check_key("nonsense")[1], "unknown")
        self.assertEqual(self.m.check_key("")[1], "unknown")
        with self.store.lock:                                   # only hashes are stored
            blob = json.dumps(self.store.db.execute("SELECT * FROM members").fetchall())
        self.assertNotIn(out["key"].split(".")[1], blob)
        self.assertNotIn(out["password"], blob)

    def test_expiry_revoke_reissue(self):
        out = self.m.issue("1234567", "Jane Pilot")
        self.t[0] += 91 * 86400
        self.assertEqual(self.m.check_key(out["key"])[1], "expired")
        new = self.m.issue("1234567", "Jane Pilot")              # asking again gives a fresh key and kills the old one
        self.assertEqual(self.m.check_key(new["key"])[0], "1234567")
        self.assertEqual(self.m.check_key(out["key"])[1], "unknown")
        self.assertTrue(self.m.revoke("1234567"))
        self.assertEqual(self.m.check_key(new["key"])[1], "revoked")
        self.assertFalse(self.m.revoke("1234567"))

    def test_login(self):
        out = self.m.issue("1234567", "Jane Pilot")
        self.assertEqual(self.m.check_login("jane pilot", out["password"])["cid"], "1234567")
        self.assertEqual(self.m.check_login("1234567", out["password"])["name"], "Jane Pilot")
        self.assertIsNone(self.m.check_login("1234567", "wrong"))
        self.assertIsNone(self.m.check_login("", out["password"]))
        self.m.revoke("1234567")
        self.assertIsNone(self.m.check_login("1234567", out["password"]))

    def test_sync(self):
        a, b = self.m.issue("111", "A"), self.m.issue("222", "B")
        self.assertEqual(self.m.sync(["111"]), ["222"])
        self.assertEqual(self.m.check_key(a["key"])[0], "111")
        self.assertEqual(self.m.check_key(b["key"])[1], "revoked")


class HttpTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "h.db"))
        cls.members = mb.MemberKeys(store)
        auth = s.Auth({"VNAAATS_FDD_PASSWORD": "sharedpw"})
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), store, SHARED, None, None, None, auth, None, None, None, cls.members, SECRET))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.url = "http://127.0.0.1:%d" % cls.srv.server_address[1]

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def call(self, path, body=None, secret=None):
        h = {"Content-Type": "application/json"}
        if secret:
            h["X-Issuer-Secret"] = secret
        req = urllib.request.Request(self.url + path, data=json.dumps(body).encode() if body is not None else None, headers=h)
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read() or b"{}")
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read() or b"{}")

    def test_issuer_secret_required(self):
        self.assertEqual(self.call("/admin/member/issue", {"cid": "1"})[0], 403)
        self.assertEqual(self.call("/admin/member/issue", {"cid": "1"}, secret="x" * 30)[0], 403)

    def test_key_flow_through_http(self):
        st, out = self.call("/admin/member/issue", {"cid": "9990001", "name": "Test Member"}, secret=SECRET)
        self.assertEqual(st, 200)
        ok = self.call("/PluginCommands?me=A&code=" + out["key"])
        self.assertEqual(ok[0], 200)
        self.assertEqual(self.call("/PluginCommands?me=A&code=" + SHARED)[0], 200)            # shared key still works
        bad = self.call("/PluginCommands?me=A&code=9990001.aaaaaaaaaaaaaaaaaaaaaaaa")
        self.assertEqual((bad[0], bad[1].get("reason")), (401, "unknown"))
        self.assertEqual(self.call("/PluginCommands?me=A&code=caf%C3%A9")[0], 401)              # non-ASCII is a clean refusal
        login = self.call("/auth/password", {"name": "9990001", "password": out["password"]})
        self.assertEqual((login[0], login[1]["name"]), (200, "Test Member"))
        self.assertEqual(self.call("/auth/password", {"name": "9990001", "password": "nope"})[0], 401)
        self.call("/admin/member/revoke", {"cid": "9990001"}, secret=SECRET)
        self.assertEqual(self.call("/PluginCommands?me=A&code=" + out["key"])[1].get("reason"), "revoked")
        self.assertEqual(self.call("/auth/password", {"name": "9990001", "password": out["password"]})[0], 401)

    def test_sync_refuses_empty_list(self):
        self.assertEqual(self.call("/admin/member/sync", {"cids": []}, secret=SECRET)[0], 400)


if __name__ == "__main__":
    unittest.main()


class RosterTests(unittest.TestCase):
    def setUp(self):
        self.store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "r.db"))
        self.m = mb.MemberKeys(self.store)
        self.data = [{"cid": i, "certification": "certified", "active": 1, "currency": 1} for i in range(1, 9)]
        self.data += [{"cid": 100, "certification": "not_certified", "active": 1}, {"cid": 101, "certification": "certified", "active": 0}]
        self.r = mb.RosterWatch(self.m, "x", fetch=lambda: self.data)

    def test_eligibility(self):
        self.assertTrue(self.r.eligible("3"))
        self.assertFalse(self.r.eligible("100"))          # not certified
        self.assertTrue(self.r.eligible("101"))           # certified but not active: still allowed
        self.assertFalse(self.r.eligible("999"))          # not on the roster

    def test_unreachable_roster_is_unknown(self):
        def boom(): raise OSError("down")
        r = mb.RosterWatch(self.m, "x", fetch=boom)
        self.assertIsNone(r.eligible("3"))

    def test_revoke_dropped(self):
        for c in ("1", "2", "3", "4"):
            self.m.issue(c, "M" + c)
        self.data = [e for e in self.data if e["cid"] != 4]
        self.assertEqual(self.r.revoke_dropped(), ["4"])
        self.assertEqual(self.m.check_key("x" * 3)[1], "unknown")
        self.assertTrue(self.m.info("3")["exists"] and not self.m.info("3")["revoked"])

    def test_broken_feed_revokes_nothing(self):
        for c in ("1", "2"):
            self.m.issue(c, "M")
        self.data = []
        self.assertEqual(self.r.revoke_dropped(), [])
        self.data = [{"cid": 1, "certification": "certified", "active": 1}]          # tiny answer
        self.assertEqual(self.r.revoke_dropped(), [])
        self.assertFalse(self.m.info("2")["revoked"])

    def test_mass_revoke_guard(self):
        for c in ("1", "2", "3", "4", "5", "6"):
            self.m.issue(c, "M")
        self.data = [e for e in self.data if e["cid"] in (1, 2, 100, 101, 7, 8, 5)]        # 3 of 6 members stay -> would drop 3 (50%)
        self.data += [{"cid": 90 + i, "certification": "certified", "active": 1} for i in range(5)]
        self.assertEqual(len(self.r.revoke_dropped()), 3)                                  # exactly half is allowed
        self.m.issue("11", "N"); self.m.issue("12", "N")
        self.data = [{"cid": 90 + i, "certification": "certified", "active": 1} for i in range(8)]
        self.assertEqual(self.r.revoke_dropped(), [])                                      # everyone dropped -> refused


class SharedSecretsOff(unittest.TestCase):
    def test_shared_password_off(self):
        a = s.Auth({"VNAAATS_FDD_PASSWORD": "abc", "VNAAATS_SHARED_PASSWORD": "off"})
        self.assertEqual(a.password, "")
        self.assertFalse(a.enabled)
        self.assertEqual(s.Auth({"VNAAATS_FDD_PASSWORD": "abc"}).password, "abc")


class PersonalOnly(unittest.TestCase):
    """Shared key and shared password switched off: personal ones still work and the FDD still offers a sign-in."""
    @classmethod
    def setUpClass(cls):
        store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "p.db"))
        cls.members = mb.MemberKeys(store)
        auth = s.Auth({"VNAAATS_SHARED_PASSWORD": "off", "VNAAATS_FDD_PASSWORD": "sharedpw"})
        os.environ["VNAAATS_SHARED_KEY"] = "off"
        try:
            cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), store, SHARED, None, None, None, auth, None, None, None, cls.members, SECRET))
        finally:
            os.environ.pop("VNAAATS_SHARED_KEY", None)
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.url = "http://127.0.0.1:%d" % cls.srv.server_address[1]
        cls.issued = cls.members.issue("1234567", "Gary Thomas")

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def call(self, path, body=None):
        req = urllib.request.Request(self.url + path, data=json.dumps(body).encode() if body is not None else None, headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read() or b"{}")
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read() or b"{}")

    def test_everything(self):
        self.assertTrue(self.call("/auth/status")[1]["editing"])                                       # sign-in screen still offered
        self.assertEqual(self.call("/auth/password", {"name": "Gary", "password": "sharedpw"})[0], 401)  # shared password refused
        self.assertEqual(self.call("/auth/password", {"name": "1234567", "password": self.issued["password"]})[0], 200)
        self.assertEqual(self.call("/PluginCommands?code=%s&me=CZQO_CTR" % SHARED)[0], 401)           # shared key refused
        self.assertEqual(self.call("/PluginCommands?code=%s&me=CZQO_CTR" % self.issued["key"])[0], 200)  # personal key accepted


class ManualKeyTest(unittest.TestCase):
    def test_manual_key_short_and_not_swept(self):
        import tempfile, os
        import members as mm
        from vnaaats_server import FlightStore
        d = tempfile.mkdtemp()
        m = mm.MemberKeys(FlightStore(os.path.join(d, "t.db")))
        out = m.issue("1017951", "Test Person", days=7, manual=True)
        self.assertAlmostEqual((out["expires"] - m.now()) / 86400, 7, delta=0.01)
        self.assertEqual(m.sync(["1"]), [])                      # roster sync leaves it alone
        self.assertEqual(m.check_key(out["key"])[0], "1017951")
        self.assertTrue(m.listing()[0]["manual"])


class TraineeKeyTest(unittest.TestCase):
    """The hidden instructor page: a 2-hour key for a student, behind its own password, and it never replaces a longer key."""
    @classmethod
    def setUpClass(cls):
        store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "t.db"))
        cls.members = mb.MemberKeys(store)
        auth = s.Auth({"VNAAATS_FDD_PASSWORD": "pw"})
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), store, SHARED, None, None, None, auth, None, None, None, cls.members, SECRET))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.url = "http://127.0.0.1:%d" % cls.srv.server_address[1]

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()
        os.environ.pop("VNAAATS_INSTRUCTOR_PASSWORD", None)

    def call(self, body):
        req = urllib.request.Request(self.url + "/auth/trainee-key", data=json.dumps(body).encode(), headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read() or b"{}")
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read() or b"{}")

    def test_flow(self):
        good = {"passphrase": "instructor-pass-1", "instructor": "Gary", "cid": "1400001", "name": "Student One"}
        os.environ.pop("VNAAATS_INSTRUCTOR_PASSWORD", None)
        self.assertEqual(self.call(good)[0], 404)                                  # off until the password is set
        os.environ["VNAAATS_INSTRUCTOR_PASSWORD"] = "instructor-pass-1"
        self.assertEqual(self.call(dict(good, passphrase="nope-nope-nope"))[0], 401)
        self.assertEqual(self.call(dict(good, instructor=""))[0], 400)
        self.assertEqual(self.call(dict(good, cid="12"))[0], 400)
        code, out = self.call(good)
        self.assertEqual(code, 200)
        self.assertEqual(out["hours"], 2)
        self.assertAlmostEqual(out["expires"] - time.time(), 7200, delta=30)
        self.assertEqual(self.members.check_key(out["key"])[0], "1400001")        # the key works, and the password signs in to the FDD
        self.assertEqual(self.members.check_login("1400001", out["password"])["cid"], "1400001")
        self.assertEqual(self.call(good)[0], 200)                                  # re-issuing a trainee key is fine
        full = self.members.issue("1400002", "Real Member")                        # a normal long-lived key must not be replaced
        self.assertEqual(self.call(dict(good, cid="1400002"))[0], 409)
        self.assertEqual(self.members.check_key(full["key"])[0], "1400002")
