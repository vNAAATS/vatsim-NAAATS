import json, os, time, sys, tempfile, threading, unittest, urllib.error, urllib.request
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import vnaaats_server as s

SAMPLE = [
 {"identifier": "V", "active": True, "last_routeing": "ALLRY 51/50 52/40 5330/30 53/20 MALOT GISTI",
  "valid_from": "2026-10-20T01:00:00.000000Z", "valid_to": "2026-10-20T08:00:00.000000Z",
  "concorde": 0, "flight_levels": [34000, 35000], "direction": "east"},
 {"identifier": "H", "active": False, "last_routeing": "MALOT GISTI", "valid_from": "x", "valid_to": "y", "flight_levels": [], "direction": "west"},
 {"identifier": "A", "active": True, "last_routeing": "ETARI 56/20 NEEKO", "valid_from": "2026-10-20T01:00:00Z",
  "valid_to": "2026-10-20T08:00:00Z", "flight_levels": [36000], "direction": "west"},
]
KEY = "k" * 20


class T(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        cache.refresh(fetch=lambda: SAMPLE)
        store = s.FlightStore(os.path.join(cls.tmp, "t.db"))
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, store, KEY))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port
        cls.cache = cache

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def get(self, path):
        try:
            with urllib.request.urlopen(self.base + path) as r:
                return r.status, r.read().decode()
        except urllib.error.HTTPError as e:
            return e.code, e.read().decode()

    def test_track_source(self):
        self.assertEqual(self.get("/GetTrackSource"), (200, "0"))

    def test_tracks_format(self):
        code, body = self.get("/GetAllNatTracks")
        self.assertEqual(code, 200)
        t = json.loads(body)
        self.assertEqual([x["id"] for x in t], ["V", "A"])          # inactive track dropped
        v = t[0]
        self.assertEqual(v["direction"], 2)
        self.assertEqual(t[1]["direction"], 1)
        self.assertEqual(v["flightLevels"], [34000, 35000])
        names = [p["name"] for p in v["route"]]
        names = [n for n in names if n != "ALLRY"]                      # ALLRY unknown until a fix is posted
        self.assertEqual(names, ["51/50", "52/40", "5330/30", "53/20", "MALOT", "GISTI"])
        r = [p for p in v["route"] if p["name"] != "ALLRY"]
        self.assertEqual((r[0]["latitude"], r[0]["longitude"]), (51.0, -50.0))
        self.assertEqual((r[2]["latitude"], r[2]["longitude"]), (53.5, -30.0))
        self.assertEqual((r[4]["latitude"], r[4]["longitude"]), (53.0, -15.0))
        self.assertEqual(v["tmi"], "293")
        self.assertEqual(v["validFrom"], "1792458000")                  # 2026-10-20 01:00Z as unix seconds
        self.assertEqual(v["validTo"], "1792483200")
        for k in ("id", "tmi", "direction", "route", "flightLevels", "validFrom", "validTo"):
            self.assertIn(k, v)

    def test_unresolved_reported(self):
        self.cache.fixes.pop("ZZZZZ", None)
        s.convert_tracks([{"identifier": "Q", "active": True, "last_routeing": "ZZZZZ 51/50 52/40"}], self.cache.fixes, u := {})
        self.assertIn("ZZZZZ", u)

    def test_flight_roundtrip(self):
        q = ("callsign=baw123&type=B772&level=350&mach=84&track=V&route=51/50 52/40&routeEtas=1,2&departure=EGLL&arrival=KJFK"
             "&direction=1&etd=1200&selcal=ABCD&datalinkConnected=true&isEquipped=1&state=CLEARED&relevant=1&targetMode=3"
             "&trackedBy=Shanwick&trackedById=EGGX")
        code, _ = self.get("/PostFlightData?code=%s&%s" % (KEY, q.replace(" ", "%20")))
        self.assertEqual(code, 200)
        code, body = self.get("/GetSingleFlightData?callsign=BAW123")
        self.assertEqual(code, 200)
        f = json.loads(body)[0]
        self.assertEqual(f["callsign"], "BAW123")
        self.assertEqual(f["assignedLevel"], 350)
        self.assertIs(f["datalinkConnected"], True)
        self.assertIs(f["isEquipped"], True)
        self.assertIs(f["direction"], True)
        self.assertEqual(f["targetMode"], 3)
        self.assertEqual(f["route"], "51/50 52/40")
        # update changes it
        self.get("/UpdateFlightData?code=%s&callsign=BAW123&level=370&state=ISSUED" % KEY)
        self.assertEqual(json.loads(self.get("/GetSingleFlightData?callsign=BAW123")[1])[0]["assignedLevel"], 370)

    def test_requires_key(self):
        self.assertEqual(self.get("/PostFlightData?callsign=X")[0], 401)
        self.assertEqual(self.get("/PostFlightData?code=wrong&callsign=X")[0], 401)
        self.assertEqual(self.get("/UpdateFlightData?code=&callsign=X")[0], 401)

    def test_all_flights_and_cors(self):
        self.get("/PostFlightData?code=%s&callsign=AAA1&level=340" % KEY)
        self.get("/PostFlightData?code=%s&callsign=BBB2&level=360" % KEY)
        with urllib.request.urlopen(self.base + "/GetAllFlightData?sort=1") as r:
            self.assertEqual(r.headers["Access-Control-Allow-Origin"], "*")
            names = [f["callsign"] for f in json.loads(r.read())]
        self.assertIn("AAA1", names)
        self.assertLess(names.index("BBB2"), names.index("AAA1"))      # newest first

    def test_named_fix_resolves_track(self):
        self.assertEqual(self.get("/PostNamedFix?name=ALLRY&lat=50.5&lon=-50.0")[0], 401)
        self.assertEqual(self.get("/PostNamedFix?code=%s&name=ALLRY&lat=abc&lon=1" % KEY)[0], 400)
        self.assertEqual(self.get("/PostNamedFix?code=%s&name=bad!&lat=1&lon=1" % KEY)[0], 400)
        code, body = self.get("/PostNamedFix?code=%s&name=allry&lat=50.5&lon=-50.0" % KEY)
        self.assertEqual(code, 200)
        v = [t for t in json.loads(self.get("/GetAllNatTracks")[1]) if t["id"] == "V"][0]
        self.assertEqual(v["route"][0]["name"], "ALLRY")
        self.assertEqual((v["route"][0]["latitude"], v["route"][0]["longitude"]), (50.5, -50.0))
        self.assertNotIn("ALLRY", json.loads(self.get("/unresolved")[1]))

    def test_public_status(self):
        s.SECTOR_RELEASE_URL = "off"                                   # no GitHub lookup in tests
        code, body = self.get("/api/status")                             # no key needed: it holds no keys, names or flight plans
        self.assertEqual(code, 200)
        d = json.loads(body)
        self.assertEqual(d["version"], s.SERVER_VERSION)
        self.assertGreaterEqual(d["uptimeSeconds"], 0)
        self.assertEqual(d["tracks"]["count"], 2)
        self.assertFalse(d["feed"]["enabled"])
        self.assertEqual(d["atc"], [])
        self.assertFalse(d["cpdlc"]["enabled"])
        self.assertIsNone(d["sectorPack"])
        self.assertNotIn(KEY, body)

    def test_nattrak_placeholder(self):
        self.assertEqual(self.get("/GetNatTrakData"), (200, "[]"))

    def test_unknown_flight_404(self):
        self.assertEqual(self.get("/GetSingleFlightData?callsign=NOPE1")[0], 404)

    def test_failed_refresh_keeps_last_tracks(self):
        def boom(): raise RuntimeError("down")
        self.cache.refresh(fetch=boom)
        self.assertEqual(len(json.loads(self.get("/GetAllNatTracks")[1])), 2)
        self.assertEqual(json.loads(self.get("/health")[1])["trackError"], "down")


class NetworkTests(unittest.TestCase):
    TRACKS = [{"id": "A", "route": [{"name": "ETARI"}, {"name": "56/20"}, {"name": "56/30"}, {"name": "56/40"}, {"name": "NEEKO"}]}]

    def pilot(self, cs, route, alt="39000", dep="EGLL", arr="KJFK"):
        return {"callsign": cs, "flight_plan": {"aircraft_short": "A333", "departure": dep, "arrival": arr,
                                                 "altitude": alt, "deptime": "1200", "route": route}}

    def test_oceanic_point(self):
        self.assertEqual(s.oceanic_point("52/50")[1:], (52.0, -50.0))
        self.assertEqual(s.oceanic_point("5730/30")[1:], (57.5, -30.0))
        self.assertEqual(s.oceanic_point("52N050W")[1:], (52.0, -50.0))
        self.assertEqual(s.oceanic_point("5230N05000W")[1:], (52.5, -50.0))
        self.assertEqual(s.oceanic_point("52/50/M082F390")[1:], (52.0, -50.0))
        self.assertIsNone(s.oceanic_point("ALLRY"))

    def test_levels(self):
        self.assertEqual(s.parse_level("39000"), 390)
        self.assertEqual(s.parse_level("FL390"), 390)
        self.assertEqual(s.parse_level("F390"), 390)
        self.assertEqual(s.parse_level(""), 0)

    def test_random_route(self):
        r = s.convert_network_pilot(self.pilot("BAW1", "DCT 52/50 52/40 52/30 DCT"), self.TRACKS)
        self.assertEqual(r["track"], "RR")
        self.assertTrue(r["direction"])                     # 50W -> 30W is heading east
        self.assertEqual(r["route"], "52/50 52/40 52/30")
        self.assertEqual(r["assignedLevel"], 390)

    def test_track_match_and_direction(self):
        r = s.convert_network_pilot(self.pilot("DLH1", "ETARI 56/20 56/30 56/40 NEEKO", dep="EDDF", arr="KJFK"), self.TRACKS)
        self.assertEqual(r["track"], "A")
        r2 = s.convert_network_pilot(self.pilot("DLH2", "56/40 56/30 56/20", dep="KJFK", arr="EDDF"), self.TRACKS)
        self.assertTrue(r2["direction"])


    def test_entry_and_exit_fixes_kept(self):
        r = s.convert_network_pilot(self.pilot("DLH401", "EDDF DCT ALLRY DCT 51N050W 52N040W 53N030W 53N020W DCT MALOT GISTI"), self.TRACKS)
        self.assertEqual(r["route"], "ALLRY 51N050W 52N040W 53N030W 53N020W MALOT")
        self.assertEqual(r["routeEtas"].split(), ["--"] * 6)
        self.assertEqual(r["track"], "RR")

    def test_entry_exit_ignores_airways_and_procedures(self):
        r = s.convert_network_pilot(self.pilot("X9", "SID1A N349B 52/50 52/40 52/30 L9 STAR2B"), self.TRACKS)
        self.assertEqual(r["route"], "52/50 52/40 52/30")

    def test_track_still_matches_with_fixes(self):
        r = s.convert_network_pilot(self.pilot("DLH1", "DCT ETARI 56/20 56/30 56/40 NEEKO DCT"), self.TRACKS)
        self.assertEqual((r["track"], r["route"]), ("A", "ETARI 56/20 56/30 56/40 NEEKO"))


    def test_track_matches_whatever_the_spelling(self):
        tracks = [{"id": "A", "route": [{"name": "SAURY"}, {"name": "61/50"}, {"name": "62/40"}, {"name": "62/30"}, {"name": "61/20"}, {"name": "BALIX"}]}]
        for route in ("SAURY DCT 61N050W 62N040W 62N030W 61N020W DCT BALIX",
                      "SAURY 6100N05000W 6200N04000W 6200N03000W 6100N02000W BALIX",
                      "SAURY 61/50 62/40 62/30 61/20 BALIX"):
            r = s.convert_network_pilot(self.pilot("DLH3V", route, dep="EDDF", arr="KDTW"), tracks)
            self.assertEqual(r["track"], "A", route)
        r = s.convert_network_pilot(self.pilot("X1", "61N050W 62N040W 62N030W 60N020W"), tracks)   # one point differs
        self.assertEqual(r["track"], "RR")

    def test_non_oceanic_ignored(self):
        self.assertIsNone(s.convert_network_pilot(self.pilot("X1", "DCT LAM DCT"), self.TRACKS))
        self.assertIsNone(s.convert_network_pilot({"callsign": "X2"}, self.TRACKS))
        self.assertIsNone(s.convert_network_pilot(self.pilot("X3", "26/138 27/139"), self.TRACKS))

    def test_endpoint_and_failure(self):
        cache = s.TrackCache(s.load_fixes())
        net = s.NetworkTraffic(cache)
        net.refresh(fetch=lambda: {"pilots": [self.pilot("BAW1", "52/50 52/40"), self.pilot("X1", "DCT")]})
        self.assertEqual([f["callsign"] for f in net.all()], ["BAW1"])
        def boom(): raise RuntimeError("feed down")
        net.refresh(fetch=boom)
        self.assertEqual(len(net.all()), 1)                  # keeps last good copy
        self.assertEqual(net.error, "feed down")


class NatTrakTests(unittest.TestCase):
    ITEMS = [{"callsign": "BAW1", "status": "CLEARED", "nat": "A", "fix": "ETARI", "level": "370", "mach": "0.82",
              "estimating_time": "09:05", "clearance_issued": "2026-10-20T08:18:59.000000Z", "extra_info": None},
             {"callsign": "BAW2", "status": "PENDING", "nat": "RR", "fix": "ALLRY", "level": "350", "mach": "0.80",
              "estimating_time": "09:10", "clearance_issued": None, "extra_info": None}]

    def test_feed_keeps_last_good(self):
        f = s.NatTrakFeed()
        f.refresh(fetch=lambda: self.ITEMS)
        self.assertEqual(len(f.all()), 2)
        def boom(): raise RuntimeError("down")
        f.refresh(fetch=boom)
        self.assertEqual(len(f.all()), 2)
        f.refresh(fetch=lambda: {"not": "a list"})
        self.assertEqual(len(f.all()), 2)

    def test_overlay_on_network_flights(self):
        cache = s.TrackCache(s.load_fixes())
        cache.refresh(fetch=lambda: SAMPLE)                # has track A
        nt = s.NatTrakFeed()
        nt.refresh(fetch=lambda: self.ITEMS)
        net = s.NetworkTraffic(cache, natrak=nt)
        pil = lambda cs: {"callsign": cs, "flight_plan": {"aircraft_short": "A333", "departure": "EGLL", "arrival": "KJFK",
                          "altitude": "30000", "deptime": "1200", "route": "52/20 52/30 52/40"}}
        net.refresh(fetch=lambda: {"pilots": [pil("BAW1"), pil("BAW2")]})
        got = {f["callsign"]: f for f in net.all()}
        self.assertEqual((got["BAW1"]["assignedLevel"], got["BAW1"]["assignedMach"], got["BAW1"]["track"]), (370, 82, "A"))
        self.assertEqual(got["BAW2"]["assignedLevel"], 300)    # pending is not a clearance: unchanged


class EditTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        cache.refresh(fetch=lambda: SAMPLE)
        cls.store = s.FlightStore(os.path.join(cls.tmp, "e.db"))
        nt = s.NatTrakFeed("x")
        net = s.NetworkTraffic(cache, natrak=nt)
        net.refresh(fetch=lambda: {"pilots": [{"callsign": "FEED1", "flight_plan": {"aircraft_short": "A333", "departure": "EGLL",
                  "arrival": "KJFK", "altitude": "30000", "deptime": "1200", "route": "52/20 52/30 52/40"}}]})
        cls.auth = s.Auth({"VNAAATS_FDD_PASSWORD": "secret-pass", "VNAAATS_POSITIONS": "CZQO_CTR,CZQO_DEL"})
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, cls.store, KEY, None, net, None, cls.auth))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def call(self, path, body=None, cookie=""):
        req = urllib.request.Request(self.base + path, data=json.dumps(body).encode() if body is not None else None,
                                     headers={"Content-Type": "application/json", "Cookie": cookie})
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read().decode()), r.headers.get("Set-Cookie", "")
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read().decode()), ""

    def login(self, position="CZQO_CTR"):
        code, body, ck = self.call("/auth/password", {"name": "Gary Thomas", "password": "secret-pass"})
        self.assertEqual(code, 200)
        cookie = ck.split(";")[0]
        if position:
            self.assertEqual(self.call("/auth/position", {"position": position}, cookie)[0], 200)
        return cookie

    def post_plugin(self, cs, level, by="CZQO_CTR", mach="84", track="A"):
        q = "/PostFlightData?code=%s&callsign=%s&level=%s&mach=%s&track=%s&by=%s" % (KEY, cs, level, mach, track, by)
        with urllib.request.urlopen(self.base + q) as r:
            return json.loads(r.read().decode())

    def test_edit_needs_login_and_position(self):
        self.assertEqual(self.call("/EditFlight", {"callsign": "X", "level": 350})[0], 401)
        self.assertEqual(self.call("/EditFlight", {"callsign": "X", "level": 350}, self.login(None))[0], 403)
        self.assertEqual(self.call("/EditFlight", {"callsign": "X", "level": 350}, self.login(None))[0], 403)

    def test_flight_data_needs_a_position(self):
        for path in ("/GetAllFlightData", "/GetNetworkTraffic", "/GetNatTrakData"):
            self.assertEqual(self.call(path)[0], 401)                              # not signed in
            self.assertEqual(self.call(path, None, self.login(None))[0], 401)      # signed in but no position yet
            self.assertEqual(self.call(path, None, self.login("CZQO_CTR"))[0], 200)
            self.assertEqual(self.call(path + "?code=" + KEY)[0], 200)             # the plugin key still works
            self.assertEqual(self.call(path + "?code=wrong")[0], 401)

    def test_voice_position_reports_for_any_aircraft(self):
        ck = self.login("CZQO_CTR")
        self.assertEqual(self.call("/SetVoiceReports", {"callsign": "FEED1", "on": True})[0], 401)                  # not signed in
        code, body, _ = self.call("/EditPosReport", {"callsign": "FEED1", "fix": "52/30", "est": "1432"}, ck)
        self.assertEqual(code, 400)                                                                                   # off by default for a non-Concorde
        code, body, _ = self.call("/SetVoiceReports", {"callsign": "FEED1", "on": True}, ck)
        self.assertEqual((code, body.get("voiceReports")), (200, True))
        code, body, _ = self.call("/EditPosReport", {"callsign": "FEED1", "fix": "52/30", "est": "1432", "act": "1433"}, ck)
        self.assertEqual(code, 200)
        self.assertEqual(body["posReports"]["52/30"]["act"], "1433")
        code, body, _ = self.call("/SetVoiceReports", {"callsign": "FEED1", "on": False}, ck)
        self.assertEqual((code, body.get("voiceReports")), (200, False))
        self.assertNotIn("posReports", body)                                                                          # switching off clears what was typed
        self.assertEqual(self.call("/EditPosReport", {"callsign": "FEED1", "fix": "52/30", "est": "1432"}, ck)[0], 400)
        self.assertEqual(self.call("/SetVoiceReports", {"callsign": "NOSUCH1", "on": True}, ck)[0], 404)

    def test_voice_position_reports_lapse(self):
        self.store.voice_set("LAPSE1", True, "x")
        self.assertTrue(self.store.voice_on("LAPSE1"))
        self.store.db.execute("UPDATE voice_reports SET ts=? WHERE callsign='LAPSE1'", (time.time() - 19 * 3600,))
        self.assertFalse(self.store.voice_on("LAPSE1"))

    def test_no_observer_mode(self):
        ck = self.login(None)
        code, body, _ = self.call("/auth/position", {"position": "OBSERVER"}, ck)
        self.assertEqual(code, 400)
        self.assertNotIn("OBSERVER", self.call("/auth/status", None, ck)[1]["positions"])
        self.assertEqual(self.call("/GetAllFlightData", None, ck)[0], 401)

    def test_wrong_password_and_throttle(self):
        self.assertEqual(self.call("/auth/password", {"name": "x", "password": "no"})[0], 401)
        codes = [self.call("/auth/password", {"name": "x", "password": "no"})[0] for _ in range(6)]
        self.assertEqual(codes[-1], 429)
        self.auth.failures.clear()

    def test_edit_validates(self):
        ck = self.login()
        self.post_plugin("VAL1", 350)
        for bad in ({"level": 5000}, {"mach": 0}, {"track": "A1"}, {"selcal": "AI1O"}, {}):
            self.assertEqual(self.call("/EditFlight", dict(bad, callsign="VAL1"), ck)[0], 400, bad)
        self.assertEqual(self.call("/EditFlight", {"callsign": "NOPE9", "level": 350}, ck)[0], 404)

    def test_edit_and_history_and_plugin_merge(self):
        ck = self.login()
        self.post_plugin("MRG1", 350)
        code, rec, _ = self.call("/EditFlight", {"callsign": "MRG1", "level": 370, "selcal": "as-cm"}, ck)
        self.assertEqual((code, rec["assignedLevel"], rec["selcal"]), (200, 370, "ASCM"))
        self.assertIn("CZQO_CTR (Gary Thomas)", rec["updatedBy"])
        self.assertEqual(rec["history"][-1]["changes"]["assignedLevel"], [350, 370])
        # the plugin posts its stale level before it has fetched the edit: the web value must survive
        self.assertEqual(self.post_plugin("MRG1", 350)["assignedLevel"], 370)
        # the plugin fetches (acknowledges) the edit, then its own changes win again
        with urllib.request.urlopen(self.base + "/GetSingleFlightData?callsign=MRG1") as r:
            self.assertEqual(json.loads(r.read().decode())[0]["assignedLevel"], 370)
        self.assertEqual(self.post_plugin("MRG1", 380, by="EGGX_CTR")["assignedLevel"], 380)
        self.assertEqual(self.post_plugin("MRG1", 380, by="Somebody")["updatedBy"], "EGGX_CTR")   # no change, so no new author

    def test_edit_feed_only_flight(self):
        ck = self.login()
        code, rec, _ = self.call("/EditFlight", {"callsign": "FEED1", "level": 360}, ck)
        self.assertEqual((code, rec["assignedLevel"], rec["departure"]), (200, 360, "EGLL"))

    def test_session_status_and_logout(self):
        ck = self.login()
        with urllib.request.urlopen(urllib.request.Request(self.base + "/auth/status", headers={"Cookie": ck})) as r:
            st = json.loads(r.read().decode())
        self.assertEqual((st["loggedIn"], st["position"], st["password"], st["vatsim"]), (True, "CZQO_CTR", True, False))
        self.call("/auth/logout", {}, ck)
        self.assertEqual(self.call("/EditFlight", {"callsign": "X", "level": 350}, ck)[0], 401)


class ConnectTests(unittest.TestCase):
    ENV = {"VNAAATS_CONNECT_CLIENT_ID": "id", "VNAAATS_CONNECT_CLIENT_SECRET": "sec", "VNAAATS_PUBLIC_URL": "https://fdd.example.com/"}

    def fake(self, cid, rating):
        def http(url, form=None, bearer=None):
            if url.endswith("/oauth/token"):
                return {"access_token": "tok"}
            return {"data": {"cid": cid, "personal": {"name_full": "Test User"}, "vatsim": {"rating": {"id": rating}}}}
        return http

    def test_authorize_url(self):
        a = s.Auth(self.ENV)
        self.assertTrue(a.connect_enabled)
        u = a.authorize_url()
        self.assertIn("redirect_uri=https%3A%2F%2Ffdd.example.com%2Fauth%2Fcallback", u)
        self.assertTrue(a.use_state(u.split("state=")[1]))

    def test_rating_gate_and_allowlist(self):
        a = s.Auth(self.ENV)
        self.assertEqual(a.exchange_code("c", self.fake("1234567", 5)), ("1234567", "Test User"))
        for rating in (1, 0, -1):                                     # observer, suspended, inactive
            with self.assertRaises(ValueError):
                a.exchange_code("c", self.fake("1234567", rating))
        b = s.Auth(dict(self.ENV, VNAAATS_ALLOWED_CIDS="1111111"))    # with an allow-list only those CIDs
        self.assertEqual(b.exchange_code("c", self.fake("1111111", 1))[0], "1111111")
        with self.assertRaises(ValueError):
            b.exchange_code("c", self.fake("1234567", 12))

    def test_disabled_without_settings(self):
        self.assertFalse(s.Auth({}).connect_enabled)
        self.assertFalse(s.Auth({}).enabled)


class MediaRangeTest(unittest.TestCase):
    def test_range_requests(self):
        tmp = tempfile.mkdtemp()
        with open(os.path.join(tmp, "v.mp4"), "wb") as f:
            f.write(bytes(range(100)))
        cache = s.TrackCache(s.load_fixes()); cache.refresh(fetch=lambda: SAMPLE)
        srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, s.FlightStore(os.path.join(tmp, "m.db")), KEY, tmp))
        threading.Thread(target=srv.serve_forever, daemon=True).start()
        base = "http://127.0.0.1:%d/v.mp4" % srv.server_port
        try:
            with urllib.request.urlopen(base) as r:
                self.assertEqual((r.status, len(r.read()), r.headers["Accept-Ranges"], r.headers["Content-Type"]), (200, 100, "bytes", "video/mp4"))
            with urllib.request.urlopen(urllib.request.Request(base, headers={"Range": "bytes=10-19"})) as r:
                self.assertEqual((r.status, r.headers["Content-Range"], r.read()), (206, "bytes 10-19/100", bytes(range(10, 20))))
            with urllib.request.urlopen(urllib.request.Request(base, headers={"Range": "bytes=90-"})) as r:
                self.assertEqual((r.status, len(r.read())), (206, 10))
            with self.assertRaises(urllib.error.HTTPError) as e:
                urllib.request.urlopen(urllib.request.Request(base, headers={"Range": "bytes=500-600"}))
            self.assertEqual(e.exception.code, 416)
        finally:
            srv.shutdown()


class NofpGuardTest(unittest.TestCase):
    def test_incomplete_plan_does_not_wipe_real_details(self):
        d = tempfile.mkdtemp()
        st = s.FlightStore(os.path.join(d, "t.db"))
        st.upsert({"callsign": "UAL19", "type": "B77W", "departure": "KEWR", "arrival": "EGLL"})
        rec = st.upsert({"callsign": "UAL19", "type": "NOFP", "departure": "", "arrival": "NOFP"})
        self.assertEqual((rec["type"], rec["departure"], rec["arrival"]), ("B77W", "KEWR", "EGLL"))
        rec = st.upsert({"callsign": "UAL19", "type": "A333", "departure": "KJFK", "arrival": "EGKK"})
        self.assertEqual((rec["type"], rec["departure"], rec["arrival"]), ("A333", "KJFK", "EGKK"))


if __name__ == "__main__":
    unittest.main()


class AselTest(unittest.TestCase):
    """Plugin reports the aircraft clicked in EuroScope; the FDD asks for it by session position."""
    def test_pick_roundtrip(self):
        tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        store = s.FlightStore(os.path.join(tmp, "a.db"))
        srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, store, KEY))
        threading.Thread(target=srv.serve_forever, daemon=True).start()
        base = "http://127.0.0.1:%d" % srv.server_port
        try:
            def get(p):
                try:
                    with urllib.request.urlopen(base + p) as r:
                        return r.status, json.loads(r.read().decode())
                except urllib.error.HTTPError as e:
                    return e.code, json.loads(e.read().decode() or "{}")
            self.assertEqual(get("/PluginAsel?code=wrong&me=CZQO_CTR&callsign=BAW1")[0], 401)
            self.assertEqual(get("/PluginAsel?code=%s&me=CZQO_CTR&callsign=baw123" % KEY)[0], 200)
            self.assertEqual(get("/PluginAsel?code=%s&me=CZQO_CTR&callsign=baw123" % KEY)[0], 200)   # same aircraft: no new pick
            self.assertEqual(get("/PluginAsel?code=%s&me=CZQO_CTR&callsign=DLH9" % KEY)[0], 200)
            self.assertEqual(get("/asel"), (200, {"callsign": "", "seq": 0}))      # no sign-in: nothing
        finally:
            srv.shutdown()


class ContactMeTest(unittest.TestCase):
    def test_mark_cm(self):
        store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "c.db"))
        self.assertIsNone(store.mark_cm("BAW1"))                                  # unknown flight, no base
        rec = store.mark_cm("baw1", {"callsign": "BAW1", "route": ""}, now=1000.0)
        self.assertEqual(rec["cmAt"], 1000.0)
        self.assertEqual(store.get("BAW1")["cmAt"], 1000.0)
        self.assertEqual(store.mark_cm("BAW1", now=2000.0)["cmAt"], 2000.0)       # existing record: later send replaces the time


class RetrackTest(unittest.TestCase):
    def test_journal(self):
        tmp = tempfile.mkdtemp()
        store = s.FlightStore(os.path.join(tmp, "r.db"))
        srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), store, KEY))
        threading.Thread(target=srv.serve_forever, daemon=True).start()
        base = "http://127.0.0.1:%d" % srv.server_port
        def get(p):
            try:
                with urllib.request.urlopen(base + p) as r:
                    return r.status, json.loads(r.read().decode())
            except urllib.error.HTTPError as e:
                return e.code, {}
        try:
            self.assertEqual(get("/PluginRetrack?code=%s&me=CZQO_CTR" % KEY), (200, {"callsigns": []}))
            self.assertEqual(get("/PluginTracked?code=bad&me=CZQO_CTR&list=BAW1")[0], 401)
            self.assertEqual(get("/PluginTracked?code=%s&me=czqo_ctr&list=baw1,DLH2,,x%%20y" % KEY)[0], 200)
            self.assertEqual(get("/PluginRetrack?code=%s&me=CZQO_CTR" % KEY)[1], {"callsigns": ["BAW1", "DLH2", "XY"]})
            self.assertEqual(get("/PluginRetrack?code=%s&me=EGGX_CTR" % KEY)[1], {"callsigns": []})
        finally:
            srv.shutdown()


class RetrackTakeoverTest(unittest.TestCase):
    def test_other_name_blocks_retrack(self):
        tmp = tempfile.mkdtemp()
        store = s.FlightStore(os.path.join(tmp, "t.db"))
        auth = s.Auth({"VNAAATS_FDD_PASSWORD": "pw"})
        srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), store, KEY, auth=auth))
        threading.Thread(target=srv.serve_forever, daemon=True).start()
        base = "http://127.0.0.1:%d" % srv.server_port
        def get(p):
            with urllib.request.urlopen(base + p) as r:
                return json.loads(r.read().decode())
        try:
            t1 = auth.create_session("1", "Gary Thomas", "password"); auth.set_position(t1, "CZQO_CTR")
            get("/PluginTracked?code=%s&me=CZQO_CTR&list=BAW1,DLH2" % KEY)
            self.assertEqual(get("/PluginRetrack?code=%s&me=CZQO_CTR" % KEY)["callsigns"], ["BAW1", "DLH2"])      # same person: fine
            auth.logout(t1)
            self.assertEqual(get("/PluginRetrack?code=%s&me=CZQO_CTR" % KEY)["callsigns"], ["BAW1", "DLH2"])      # nobody on the FDD: fine
            t2 = auth.create_session("2", "Someone Else", "password"); auth.set_position(t2, "CZQO_CTR")
            self.assertEqual(get("/PluginRetrack?code=%s&me=CZQO_CTR" % KEY)["callsigns"], [])                   # somebody else took the position
        finally:
            srv.shutdown()


class KeySignInTest(unittest.TestCase):
    """Anyone can get a key; the FDD signs in with it only while the CID is connected to VATSIM on an oceanic position."""
    @classmethod
    def setUpClass(cls):
        import members as mm
        cls.tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        cache.refresh(fetch=lambda: SAMPLE)
        store = s.FlightStore(os.path.join(cls.tmp, "k.db"))
        cls.members = mm.MemberKeys(store)
        cls.net = s.NetworkTraffic(cache)
        cls.net.slurper_url = "off"      # never reach the real VATSIM from a test
        cls.feed = {"pilots": [], "controllers": []}
        cls.net.refresh(fetch=lambda: cls.feed)
        auth = s.Auth({"VNAAATS_POSITIONS": "CZQO_CTR,EGGX_CTR,NAT_FSS"})
        cls.auth = auth
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, store, KEY, None, cls.net, None, auth, None, None, None, cls.members))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def test_live_lookup_signs_in_before_the_feed_shows_the_controller(self):
        self.setfeed()
        code, out = self.post("/auth/get-key", {"agree": True, "email": "live@example.com", "cid": "1700040", "name": "Live"})
        key = out["key"]
        self.assertEqual(self.post("/auth/key-login", {"key": key})[0], 202)                     # not connected anywhere
        self.net.slurper_cache.clear()
        orig = self.net.live_positions
        self.net.live_positions = lambda cid: orig(cid, fetch=lambda c: "%s,NAT_FSS,atc,128.000,600,0,0,0,0,0,0,0,0,0,0,\n%s,EGGX_ATIS,atc,121.0,0,0,0,0,0,0,0,0,0,0,0,\n" % (c, c))
        try:
            code, out = self.post("/auth/key-login", {"key": key})
            self.assertEqual(code, 200)                                                          # the feed does not list this CID yet
        finally:
            self.net.live_positions = orig
        self.net.slurper_cache.clear()
        rows = self.net.live_positions("1700040", fetch=lambda c: "%s,EGGX_ATIS,atc,121.0,0,0,0,0,0,0,0,0,0,0,0,\n%s,BAW1,pilot,0,0,0,0,0,0,0,0,0,0,0,0,\n" % (c, c))
        self.assertEqual(rows, [])                                                               # ATIS (range 0) and pilot rows do not count

    def test_key_check_for_the_app(self):
        code, out = self.post("/auth/get-key", {"agree": True, "email": "kc@example.com", "cid": "1700077", "name": "KC"})
        def ask(k):
            try:
                with urllib.request.urlopen(self.base + "/KeyCheck?code=" + k) as r:
                    return r.status, json.loads(r.read().decode())
            except urllib.error.HTTPError as e:
                return e.code, json.loads(e.read().decode())
        self.assertEqual(ask(out["key"]), (200, {"ok": True}))
        self.assertEqual(ask("1700077.nope"), (401, {"ok": False, "reason": "unknown"}))
        self.assertEqual(ask(""), (401, {"ok": False, "reason": "unknown"}))
        self.auth.failures.clear()                                        # wrong tries count toward the shared lock-out; do not leak into other tests

    def test_key_session_closes_an_hour_after_leaving_vatsim(self):
        a = s.Auth({})
        state = {"on": True}
        a.on_position = lambda cid: state["on"]
        tok = a.create_session("1700050", "X", "key")
        self.assertIsNotNone(a.get(tok))
        state["on"] = False
        self.assertIsNotNone(a.get(tok))                                   # just left: still open
        a.sessions[tok]["last_on"] = time.time() - 3000
        self.assertIsNotNone(a.get(tok))                                   # 50 minutes: still open
        a.sessions[tok]["last_on"] = time.time() - 3700
        self.assertIsNone(a.get(tok))                                      # over an hour: closed
        tok = a.create_session("1700050", "X", "key")
        state["on"] = None
        a.sessions[tok]["last_on"] = time.time() - 99999
        self.assertIsNotNone(a.get(tok))                                   # feed unreadable: never closes
        tok = a.create_session("1700051", "Y", "member")
        state["on"] = False
        a.sessions[tok]["last_on"] = time.time() - 99999
        self.assertIsNotNone(a.get(tok))                                   # only key sign-ins are affected

    def setfeed(self, pilots=(), controllers=()):
        self.__class__.feed = {"pilots": [{"cid": 9999999, "callsign": "FILL1"}] + list(pilots), "controllers": list(controllers)}   # someone is always online on the real network
        self.net.refresh(fetch=lambda: self.feed)

    def post(self, path, body):
        req = urllib.request.Request(self.base + path, data=json.dumps(body).encode(), headers={"Content-Type": "application/json"})
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read().decode())
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read().decode())

    def test_anyone_gets_a_key_and_it_is_replaced_only_by_the_connected_owner(self):
        self.setfeed()
        code, out = self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "1700001", "name": "Test"})
        self.assertEqual(code, 200)
        self.assertEqual(self.members.check_key(out["key"])[0], "1700001")
        self.assertAlmostEqual((out["expires"] - time.time()) / 86400, 90, delta=0.05)
        self.assertEqual(self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "1700001"})[0], 409)        # already has a key, and is not connected
        self.setfeed(pilots=[{"cid": 1700001, "callsign": "BAW1"}])
        self.assertEqual(self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "1700001"})[0], 200)        # connected: a new key replaces the old
        self.assertEqual(self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "12"})[0], 400)
        self.assertEqual(self.post("/auth/get-key", {"agree": True, "email": "nonsense", "cid": "1700009"})[0], 400)          # an email address is required
        self.assertEqual(self.post("/auth/get-key", {"email": "a@b.co", "cid": "1700009"})[0], 400)                          # the confirmation box is required
        mine = next(m for m in self.members.listing() if m["cid"] == "1700001")
        self.assertEqual((mine["email"], mine["ip"]), ("test@example.com", "127.0.0.1"))                                   # kept for following up misuse

    def test_key_sign_in_waits_for_a_position(self):
        self.setfeed()
        code, out = self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "1700002", "name": "Alex"})
        key = out["key"]
        self.assertEqual(self.post("/auth/key-login", {"key": "wrong"})[0], 401)
        code, d = self.post("/auth/key-login", {"key": key})
        self.assertEqual((code, d.get("waiting")), (202, True))                           # key good, not connected
        self.setfeed(controllers=[{"cid": 1700002, "callsign": "EGGX_OBS", "facility": 0}])
        self.assertEqual(self.post("/auth/key-login", {"key": key})[0], 202)             # an observer is not a position
        self.setfeed(controllers=[{"cid": 1700002, "callsign": "NAT_FSS", "facility": 6}])
        code, d = self.post("/auth/key-login", {"key": key})
        self.assertEqual(code, 200)
        self.assertEqual((d["loggedIn"], d["position"], d["cid"]), (True, "NAT_FSS", "1700002"))
        self.assertAlmostEqual((d["keyExpires"] - time.time()) / 86400, 90, delta=0.05)               # the display can warn before it runs out
        self.setfeed(controllers=[{"cid": 1700002, "callsign": "NY_FSS", "facility": 6}])
        self.assertEqual(self.post("/auth/key-login", {"key": key})[0], 202)             # not an oceanic position here

    def test_instructor_extra_and_relief_callsigns_sign_in_beside_the_main_position(self):
        calls = ["NAT_FSS", "NAT_I_FSS", "NAT__FSS", "CZQO_CTR", "CZQO_A_CTR", "CZQO_L_CTR", "CZQO_A_DEL", "CZQO_L_DEL", "EGGX_CTR", "EGGX_A_CTR", "EGGX_L_CTR", "EGGX_A_DEL", "EGGX_L_DEL", "CZQO__CTR", "EGGX__CTR"]
        self.setfeed()
        keys = {}
        for n, cs in enumerate(calls):
            s.TEMP_KEY_ISSUED.clear()                                                        # the per-address limit of 10 keys an hour is not what is being tested
            cid = str(1701000 + n)
            keys[cs] = (cid, self.post("/auth/get-key", {"agree": True, "email": "t@example.com", "cid": cid, "name": cs})[1]["key"])
        self.setfeed(controllers=[{"cid": int(cid), "callsign": cs, "facility": 6} for cs, (cid, _) in keys.items()])      # everyone on the network at once
        for cs, (cid, key) in keys.items():
            code, d = self.post("/auth/key-login", {"key": key})
            self.assertEqual((code, d.get("position")), (200, cs), cs)                       # each gets its own callsign as the position, whoever else is signed in
            self.assertTrue(self.net.cid_online(cid))
        s.TEMP_KEY_ISSUED.clear()

    def test_key_is_accepted_as_pasted_with_label_spaces_or_quotes(self):
        self.setfeed()
        key = self.post("/auth/get-key", {"agree": True, "email": "t@example.com", "cid": "1702000", "name": "P"})[1]["key"]
        for pasted in (key, "key=" + key, "KEY=" + key, "  key=" + key + "  \n", '"' + key + '"', "key=" + key[:12] + " " + key[12:]):
            self.assertEqual(self.members.check_key(pasted), ("1702000", ""), pasted)
        self.assertEqual(self.members.check_key("key=" + key[:-1] + ("a" if key[-1] != "a" else "b"))[1], "unknown")      # a wrong key is still refused
        s.TEMP_KEY_ISSUED.clear()

    def test_expired_key_says_so(self):
        self.setfeed()
        code, out = self.post("/auth/get-key", {"agree": True, "email": "a@b.co", "cid": "1700004"})
        with self.members.store.lock:
            self.members.store.db.execute("UPDATE members SET expires=? WHERE cid=?", (time.time() - 60, "1700004"))
            self.members.store.db.commit()
        code, d = self.post("/auth/key-login", {"key": out["key"]})
        self.assertEqual(code, 401)
        self.assertIn("expired", d["error"])
        self.assertIn("vnaaats.com/get-key", d["error"])

    def test_blocked_cid(self):
        self.setfeed()
        f = os.path.join(self.tmp, "blocked.txt")
        os.environ["VNAAATS_BLOCKED_CIDS"] = f
        try:
            code, out = self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "1700003"})
            self.assertEqual(code, 200)
            open(f, "w").write("# removed by the team\n1700003  # reason\n")
            self.assertEqual(self.post("/auth/get-key", {"agree": True, "email": "test@example.com", "cid": "1700003"})[0], 403)
            self.assertEqual(self.members.check_key(out["key"]), (None, "revoked"))
            self.setfeed(controllers=[{"cid": 1700003, "callsign": "NAT_FSS", "facility": 6}])
            self.assertEqual(self.post("/auth/key-login", {"key": out["key"]})[0], 401)
        finally:
            os.environ.pop("VNAAATS_BLOCKED_CIDS", None)


class ConcordeTrackTest(unittest.TestCase):
    def test_four_tracks_with_block_level(self):
        tr = s.concorde_tracks()
        self.assertEqual([t["id"] for t in tr], ["SM", "SN", "SO", "SP"])
        self.assertEqual({t["blockLevel"] for t in tr}, {"FL470-600"})
        self.assertEqual([t["direction"] for t in tr], [1, 2, 0, 0])
        sm20 = [r for r in tr[0]["route"] if r["name"] == "SM20W"][0]
        self.assertAlmostEqual(sm20["latitude"], 50 + 50 / 60.0, places=3)         # 50.50 means 50 deg 50 min
        sn52 = [r for r in tr[1]["route"] if r["name"] == "SN52W"][0]
        self.assertAlmostEqual(sn52["longitude"], -52.5, places=3)

    def test_plugin_track_list_has_no_concorde_tracks(self):
        class C:
            tracks = []
            lock = threading.Lock()
        tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        cache.refresh(fetch=lambda: SAMPLE)
        srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, s.FlightStore(os.path.join(tmp, "c.db")), KEY))
        threading.Thread(target=srv.serve_forever, daemon=True).start()
        try:
            base = "http://127.0.0.1:%d" % srv.server_port
            plain = json.loads(urllib.request.urlopen(base + "/GetAllNatTracks").read())
            web = json.loads(urllib.request.urlopen(base + "/GetAllNatTracks?fdd=1").read())
            self.assertFalse(any(t["id"] in ("SM", "SN", "SO", "SP") for t in plain))
            self.assertEqual([t["id"] for t in web][-9:], ["SM", "SN", "SO", "SP", "T9", "T290", "T213", "T13", "T16"])   # NAT, Concorde, Tango
        finally:
            srv.shutdown()

    def test_half_degree_and_concorde_points(self):
        self.assertEqual(s.oceanic_point("N5020"), ("N5020", 50.5, -20.0))          # ARINC 424 half degree
        self.assertEqual(s.oceanic_point("5020N"), ("5020N", 50.0, -20.0))          # whole degree
        self.assertEqual(s.oceanic_point("5030/30")[1:], (50.5, -30.0))
        self.assertAlmostEqual(s.oceanic_point("SM20W")[1], 50 + 50 / 60.0, places=3)

    def test_flight_on_a_concorde_track_gets_its_name(self):
        pts = " ".join(n for n, *_ in s.CONCORDE_TRACK_DEFS[0][3])
        rec = s.convert_network_pilot({"callsign": "BAW1", "flight_plan": {"aircraft_short": "CONC", "departure": "EGLL", "arrival": "KJFK",
                                       "altitude": "50000", "deptime": "1200", "route": "DCT " + pts + " DCT"}}, [])
        self.assertEqual(rec["track"], "SM")


class TangoRouteTest(unittest.TestCase):
    def test_routes_levels_and_positions(self):
        tr = {t["id"]: t for t in s.tango_tracks()}
        self.assertEqual(list(tr), ["T9", "T290", "T213", "T13", "T16"])
        self.assertTrue(all(t["tango"] and "supersonic" not in t and "blockLevel" not in t for t in tr.values()))
        self.assertEqual([r["name"] for r in tr["T9"]["route"]], ["LASNO", "BEGAS"])
        self.assertEqual(tr["T9"]["flightLevels"], [30000, 32000, 34000, 36000, 38000, 40000])      # even, FL300-400
        self.assertEqual(tr["T290"]["flightLevels"], [29000, 31000, 33000, 35000, 37000, 39000, 41000])   # odd, FL290-410
        self.assertEqual(tr["T213"]["flightLevels"], [])
        gelpo = [r for r in tr["T290"]["route"] if r["name"] == "GELPO"][0]
        self.assertAlmostEqual(gelpo["latitude"], 48 + 38 / 60.0 + 19 / 3600.0, places=3)        # 483819N
        self.assertAlmostEqual(gelpo["longitude"], -(9 + 29 / 60.0 + 19 / 3600.0), places=3)      # 0092919W
        omoko = [r for r in tr["T16"]["route"] if r["name"] == "OMOKO"][0]
        self.assertAlmostEqual(omoko["latitude"], 48 + 50.2 / 60.0, places=3)

    def test_matching_filed_routes(self):
        self.assertEqual(s.tango_match("EGLL DCT LASNO T9 BEGAS DCT LEMD".split()), ("T9", ["LASNO", "BEGAS"], False))
        self.assertEqual(s.tango_match("LEMD ADVAT T290 GELPO EGLL".split()), ("T290", ["ADVAT", "GELPO"], True))
        self.assertEqual(s.tango_match("BERUX DCT TAMEL".split())[1:], (["BERUX", "TAMEL"], True))     # both fixes, flown northbound
        self.assertEqual(s.tango_match("NILAV T13 OMOKO".split())[0], "T13")
        self.assertEqual(s.tango_match("OMOKO T16 GONAN".split())[0], "T16")
        self.assertIsNone(s.tango_match("A B T9 C".split()))              # the airway name alone is not enough
        self.assertIsNone(s.tango_match("LASNO DCT ABCDE".split()))       # one fix and no airway name
        self.assertIsNone(s.tango_match("EGLL DCT KJFK".split()))

    def test_feed_flight_on_a_tango_route(self):
        rec = s.convert_network_pilot({"callsign": "EZY12", "flight_plan": {"aircraft_short": "A320", "departure": "EGLL", "arrival": "GCTS",
                                       "altitude": "36000", "deptime": "1200", "route": "DCT LASNO T9 BEGAS DCT"}}, [])
        self.assertEqual((rec["track"], rec["route"], rec["direction"], rec["assignedLevel"]), ("T9", "LASNO BEGAS", False, 360))
        up = s.convert_network_pilot({"callsign": "RYR9", "flight_plan": {"aircraft_short": "B738", "departure": "GCTS", "arrival": "EGLL",
                                      "altitude": "35000", "route": "ADVAT T290 GELPO"}}, [])
        self.assertEqual((up["track"], up["direction"]), ("T290", True))
        self.assertIsNone(s.convert_network_pilot({"callsign": "X1", "flight_plan": {"route": "EGLL DCT KJFK"}}, []))

    def test_nat_flight_is_not_taken_for_a_tango_flight(self):
        rec = s.convert_network_pilot({"callsign": "BAW2", "flight_plan": {"departure": "EGLL", "arrival": "KJFK", "altitude": "36000",
                                       "route": "LASNO 50/20 52/30 52/40 LOMSI"}}, [])
        self.assertEqual(rec["track"], "RR")

    def test_distance_uses_tango_fix_positions(self):
        self.assertIsNotNone(s.token_to_latlon("ADVAT", {}))


class ZipServedTest(unittest.TestCase):
    def test_zip_has_a_type(self):
        self.assertEqual(s.MIME[".zip"], "application/zip")


class AdminLogTests(unittest.TestCase):
    TOKEN = "t" * 24

    @classmethod
    def setUpClass(cls):
        import adminlog
        cls.tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        store = s.FlightStore(os.path.join(cls.tmp, "t.db"))
        cls.hub = adminlog.LogHub(cls.TOKEN, os.path.join(cls.tmp, "logs"))
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, store, KEY, logs=cls.hub))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def call(self, path, body=None, token=None):
        h = {"Content-Type": "application/json"}
        if token is not None:
            h["X-Admin-Token"] = token
        req = urllib.request.Request(self.base + path, data=json.dumps(body).encode() if body is not None else None, headers=h)
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read().decode() or "null") if "json" in r.headers.get("Content-Type", "") else r.read().decode()
        except urllib.error.HTTPError as e:
            return e.code, None

    def test_plugin_needs_key(self):
        self.assertEqual(self.call("/PluginLog?code=wrong&me=X", {"lines": ["a"]})[0], 401)

    def test_roundtrip_and_redaction(self):
        code, out = self.call("/PluginLog?code=%s&me=czqo_ctr&v=2.2.0" % KEY, {"lines": [
            "[ERR 10:00:00.1] [X] Request URL: http://a/PostFlightData?code=SECRETKEY&callsign=AB1", "[LOG 10:00:01.1] hello"]})
        self.assertEqual((code, out["n"], out["detail"]), (200, 2, False))
        code, d = self.call("/admin/log/data?me=CZQO_CTR&level=problems", token=self.TOKEN)
        self.assertEqual(code, 200)
        self.assertEqual(len(d["lines"]), 1)
        self.assertNotIn("SECRETKEY", d["lines"][0]["line"])
        self.assertIn("code=***", d["lines"][0]["line"])
        self.assertEqual(d["controllers"][0]["errors"], 1)
        self.assertNotIn("SECRETKEY", open(os.path.join(self.tmp, "logs", os.listdir(os.path.join(self.tmp, "logs"))[0])).read())

    def test_admin_needs_token(self):
        self.assertEqual(self.call("/admin/log/data", token="nope")[0], 403)
        self.assertEqual(self.call("/admin/log/data")[0], 403)
        self.assertEqual(self.call("/admin/log/detail", {"me": "A", "on": True}, token="nope")[0], 403)

    def test_page_has_no_data(self):
        code, html = self.call("/admin/log")
        self.assertEqual(code, 200)
        self.assertIn("Admin token", html)

    def test_detail_flag_reaches_plugin(self):
        self.call("/PluginLog?code=%s&me=EGGX_CTR" % KEY, {"lines": ["[LOG 1] x"]})
        self.assertEqual(self.call("/admin/log/detail", {"me": "EGGX_CTR", "on": True}, token=self.TOKEN)[0], 200)
        self.assertTrue(self.call("/PluginLog?code=%s&me=EGGX_CTR" % KEY, {"lines": []})[1]["detail"])
        self.call("/admin/log/detail", {"me": "EGGX_CTR", "on": False}, token=self.TOKEN)
        self.assertFalse(self.call("/PluginLog?code=%s&me=EGGX_CTR" % KEY, {"lines": []})[1]["detail"])

    def test_viewer_off_without_token(self):
        import adminlog
        self.assertFalse(adminlog.LogHub("").token_ok(""))

    def test_rate_limit_and_batch_cap(self):
        import adminlog
        h = adminlog.LogHub("x" * 20)
        self.assertEqual(h.add("A", "1", ["l"] * 500)["n"], adminlog.MAX_BATCH)
        h.add("A", "1", ["l"] * 200); h.add("A", "1", ["l"] * 200)
        self.assertEqual(h.add("A", "1", ["l"] * 200)["n"], 0)


class EstimateEtaTests(unittest.TestCase):
    FIXES = {"JANJO": [57.0, -48.0]}

    def pilot(self, **kw):
        p = {"latitude": 55.0, "longitude": -20.0, "groundspeed": 480, "heading": 270, "altitude": 36000}
        p.update(kw)
        return p

    def test_times_run_forward_along_the_route(self):
        r = s.estimate_etas("5630/20 5630/30 56/40 JANJO", self.pilot(), self.FIXES, now=0)
        self.assertEqual(r[0], "0011")                    # 90nm ahead at 480kt
        times = [int(x[:2]) * 60 + int(x[2:]) for x in r]
        self.assertEqual(times, sorted(times))
        self.assertTrue(all(x != "--" for x in r))

    def test_passed_fixes_are_blank(self):
        r = s.estimate_etas("54/10 5630/20 5630/30", self.pilot(longitude=-15.0), self.FIXES, now=0)
        self.assertEqual(r[0], "--")                       # behind an aircraft flying west
        self.assertNotEqual(r[1], "--")

    def test_no_estimate_on_the_ground_or_without_a_position(self):
        self.assertIsNone(s.estimate_etas("56/30", self.pilot(groundspeed=0), self.FIXES))
        self.assertIsNone(s.estimate_etas("56/30", self.pilot(altitude=500), self.FIXES))
        self.assertIsNone(s.estimate_etas("56/30", {}, self.FIXES))

    def test_unknown_fix_is_skipped_not_fatal(self):
        r = s.estimate_etas("NOWHERE 56/30", self.pilot(), self.FIXES, now=0)
        self.assertEqual(r[0], "--")
        self.assertNotEqual(r[1], "--")

    def test_network_flights_get_times_and_plugin_times_win(self):
        feed = {"pilots": [], "controllers": []}
        n = s.NetworkTraffic(s.TrackCache({}))
        n.flights = [{"callsign": "AAA1", "route": "5630/20 5630/30", "routeEtas": "-- --"},
                     {"callsign": "BBB2", "route": "5630/20 5630/30", "routeEtas": "1200 1230"}]
        n.pilots = {"AAA1": self.pilot(), "BBB2": self.pilot()}
        out = {f["callsign"]: f for f in n.all()}
        self.assertTrue(out["AAA1"].get("etaCalc"))
        self.assertNotIn("--", out["AAA1"]["routeEtas"].split())
        self.assertEqual(out["BBB2"]["routeEtas"], "1200 1230")


class NotifierTests(unittest.TestCase):
    def make(self, **kw):
        import adminlog
        sent = []
        return adminlog.Notifier("http://x", post=sent.append, **kw), sent

    def test_error_alerts_with_here_and_warnings_do_not(self):
        n, sent = self.make()
        n.note("CZQO_CTR", "2.2.1", ["[WARN 10:00:00.1] slow", "[LOG 10:00:00.2] hi"])
        n.tick()
        self.assertEqual(sent, [])
        n.note("CZQO_CTR", "2.2.1", ["[ERR 10:00:01.1] Could not post aircraft BAW1: down"])
        n.tick()
        self.assertEqual(len(sent), 1)
        self.assertTrue(sent[0]["content"].startswith("@here"))
        self.assertEqual(sent[0]["allowed_mentions"], {"parse": ["everyone"]})
        self.assertIn("CZQO_CTR", sent[0]["content"])

    def test_ping_user_pings_only_that_person(self):
        n, sent = self.make(ping_user="<@123456789012345678>")
        n.note("A", "1", ["[ERR 1] boom"])
        n.tick()
        self.assertTrue(sent[0]["content"].startswith("<@123456789012345678>"))
        self.assertNotIn("@here", sent[0]["content"])
        self.assertEqual(sent[0]["allowed_mentions"], {"users": ["123456789012345678"]})

    def test_same_error_from_many_aircraft_is_held_back(self):
        n, sent = self.make()
        n.note("A", "1", ["[ERR 1] Could not post aircraft BAW1: down", "[ERR 2] Could not post aircraft DLH2: down", "[EXC 3] other fault"])
        n.tick()
        self.assertEqual(len(sent), 1)
        self.assertIn("2 new", sent[0]["content"])
        self.assertIn("1 repeats held back", sent[0]["content"])

    def test_summary_has_no_here_and_resets(self):
        n, sent = self.make(summary_hours=6)
        n.note("A", "2.2.1", ["[ERR 1] boom 1", "[LOG 2] x"])
        n.tick()
        sent.clear()
        n.tick(now=n.next_summary + 1)
        self.assertEqual(len(sent), 1)
        self.assertNotIn("@here", sent[0]["content"])
        self.assertIn("1 controllers", sent[0]["content"])
        self.assertIn("1 errors", sent[0]["content"])
        self.assertEqual(n.stats["err"], 0)

    def test_failed_post_does_not_raise(self):
        import adminlog
        def boom(_): raise OSError("discord down")
        n = adminlog.Notifier("http://x", post=boom)
        n.note("A", "1", ["[ERR 1] x"])
        n.tick()

    def test_prev_session_lines_never_alert(self):
        n, sent = self.make()
        n.note("A", "1", ["[ERR 1] old crash"], prev=True)
        n.tick()
        self.assertEqual(sent, [])


class InstallerTests(unittest.TestCase):
    def test_installer_copies_every_module_the_server_imports(self):
        """A module the server imports but deploy/setup.sh does not copy crashes the server on start (it did once: adminlog.py)."""
        import re as _re
        here = os.path.join(os.path.dirname(__file__), "..")
        src = open(os.path.join(here, "vnaaats_server.py")).read()
        setup = open(os.path.join(here, "deploy", "setup.sh")).read()
        for mod in _re.findall(r"^import (\w+)(?: as \w+)?$", src, _re.M):
            if os.path.exists(os.path.join(here, mod + ".py")):
                self.assertIn(mod + ".py", setup, "deploy/setup.sh does not copy %s.py" % mod)


class RouteSanityTests(unittest.TestCase):
    def rec(self, dep, arr, east, track="RR", route="JANJO 56/50 58/40 ETARI"):
        return {"departure": dep, "arrival": arr, "direction": east, "track": track, "route": route}

    def test_normal_flights_are_trusted(self):
        self.assertEqual(s.route_sanity(self.rec("EGLL", "KBOS", False)), "")
        self.assertEqual(s.route_sanity(self.rec("KJFK", "EGLL", True)), "")
        self.assertEqual(s.route_sanity(self.rec("CYYZ", "LFPG", True)), "")

    def test_wrong_way_is_flagged(self):
        self.assertIn("westbound", s.route_sanity(self.rec("EGLL", "KBOS", True)))
        self.assertIn("eastbound", s.route_sanity(self.rec("KJFK", "EGLL", False)))

    def test_both_ends_on_one_side_is_flagged(self):
        self.assertIn("does not cross", s.route_sanity(self.rec("EGLL", "EDDF", False)))
        self.assertIn("does not cross", s.route_sanity(self.rec("KJFK", "KBOS", True)))

    def test_unknown_ends_and_tango_are_not_judged(self):
        self.assertEqual(s.route_sanity(self.rec("TNCM", "EGLL", False)), "")
        self.assertEqual(s.route_sanity(self.rec("EGLL", "KBOS", True, track="T9")), "")

    def test_no_route(self):
        self.assertEqual(s.route_sanity(self.rec("EGLL", "KBOS", False, route="")), "no route")


class PluginRouteEndpointTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp()
        cache = s.TrackCache(s.load_fixes())
        store = s.FlightStore(os.path.join(cls.tmp, "t.db"))
        net = s.NetworkTraffic(cache)
        net.flights = [
            {"callsign": "GOOD1", "departure": "EGLL", "arrival": "KBOS", "direction": False, "track": "D", "route": "RIKAL 53/50 55/40 55/30 TOBOR", "routeEtas": "-- -- -- -- --"},
            {"callsign": "BAD1", "departure": "EGLL", "arrival": "KBOS", "direction": True, "track": "RR", "route": "A 53/50 B", "routeEtas": "-- -- --"}]
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, store, KEY, network=net))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def get(self, path):
        try:
            with urllib.request.urlopen(self.base + path) as r:
                return r.status, json.loads(r.read().decode())
        except urllib.error.HTTPError as e:
            return e.code, None

    def test_needs_key(self):
        self.assertEqual(self.get("/PluginRoute?code=bad&callsign=GOOD1")[0], 401)

    def test_good_route_is_given(self):
        code, d = self.get("/PluginRoute?code=%s&callsign=GOOD1" % KEY)
        self.assertEqual((code, d["ok"], d["track"]), (200, True, "D"))
        self.assertEqual(d["route"], "RIKAL 53/50 55/40 55/30 TOBOR")

    def test_wrong_way_route_is_refused_with_a_reason(self):
        code, d = self.get("/PluginRoute?code=%s&callsign=BAD1" % KEY)
        self.assertEqual((code, d["ok"]), (200, False))
        self.assertIn("westbound", d["why"])

    def test_unknown_aircraft_404(self):
        self.assertEqual(self.get("/PluginRoute?code=%s&callsign=NOPE9" % KEY)[0], 404)


class AdminFddTests(unittest.TestCase):
    """The admin view: the log-viewer token opens a read-only display with no VATSIM connection."""
    TOKEN = "admin-token-123456"

    @classmethod
    def setUpClass(cls):
        import adminlog
        tmp = tempfile.mkdtemp()
        logs = adminlog.LogHub(cls.TOKEN, os.path.join(tmp, "logs"))
        store = s.FlightStore(os.path.join(tmp, "a.db"))
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), store, KEY, None, None, None, s.Auth({}), None, None, None, None, "", None, logs))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    def _post(self, path, body, headers=None, cookie=""):
        h = {"Content-Type": "application/json"}
        h.update(headers or {})
        if cookie:
            h["Cookie"] = cookie
        req = urllib.request.Request(self.base + path, json.dumps(body).encode(), h, method="POST")
        try:
            r = urllib.request.urlopen(req)
            return r.status, json.loads(r.read() or b"{}"), r.headers.get("Set-Cookie", "")
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read() or b"{}"), ""

    def test_needs_the_token(self):
        self.assertEqual(self._post("/admin/fdd/session", {})[0], 403)
        self.assertEqual(self._post("/admin/fdd/session", {}, {"X-Admin-Token": "wrong"})[0], 403)

    def test_view_only(self):
        code, data, cookie = self._post("/admin/fdd/session", {}, {"X-Admin-Token": self.TOKEN})
        self.assertEqual(code, 200)
        self.assertEqual(data["position"], "ADMIN")
        ck = cookie.split(";")[0]
        for path in ("/EditFlight", "/AddFlight", "/RemoveFlight", "/cpdlc/logon", "/cpdlc/send", "/auth/position"):
            self.assertEqual(self._post(path, {"callsign": "X", "position": "CZQO_CTR"}, cookie=ck)[0], 403, path)
        self.assertEqual(self._post("/auth/logout", {}, cookie=ck)[0], 200)

    def test_page_and_position(self):
        self.assertIn(b"Admin token", urllib.request.urlopen(self.base + "/admin/fdd").read())
        self.assertEqual(self._post("/auth/position", {"position": "ADMIN"})[0], 400)
