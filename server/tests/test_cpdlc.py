import json, os, sys, tempfile, threading, time, unittest, urllib.error, urllib.parse, urllib.request
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import cpdlc as c
import vnaaats_server as s


class FakeHoppie:
    """Stands in for hoppie.nl: records what we send, plays back what 'pilots' sent."""
    def __init__(self):
        self.sent, self.queue, self.online, self.refuse = [], [], set(), False

    def __call__(self, url, data):
        f = {k: v[0] for k, v in urllib.parse.parse_qs(data.decode(), keep_blank_values=True).items()}
        self.sent.append(f)
        if self.refuse:
            return "error {callsign already in use}"
        if f["type"] == "poll":
            out, self.queue = self.queue, []
            return "ok " + " ".join("{%s %s {%s}}" % ((q[0], q[2] if len(q) > 2 else "cpdlc", q[1])) for q in out)
        if f["type"] == "ping":
            return "ok {%s}" % " ".join(x for x in f["packet"].split() if x in self.online)
        return "ok"


def make(auto=True, **kw):
    hop = FakeHoppie()
    st = c.CpdlcStation(os.path.join(tempfile.mkdtemp(), "c.db"), c.HoppieClient("http://x", "code", "CZQO", http=hop), auto_accept=auto, **kw)
    return hop, st


class ProtocolTests(unittest.TestCase):
    def test_parse_poll(self):
        head, blocks, _ = c.parse_blocks("ok {LOVV cpdlc {/data2/15//WU/CONTACT @WIEN SOUTH CTR@ @133.80}} {BAW1 telex {hello}}")
        self.assertEqual(head, "ok")
        self.assertEqual(blocks[0], ("LOVV", "cpdlc", "/data2/15//WU/CONTACT @WIEN SOUTH CTR@ @133.80"))
        self.assertEqual(blocks[1][1], "telex")
        self.assertEqual(c.parse_blocks("ok ")[1], [])
        self.assertEqual(c.parse_blocks("error {bad logon}")[0], "error")

    def test_parse_packet(self):
        self.assertEqual(c.parse_cpdlc_packet("/data2/15//WU/CONTACT @X@ 133/80"), {"min": 15, "mrn": None, "ra": "WU", "text": "CONTACT @X@ 133/80"})
        self.assertEqual(c.parse_cpdlc_packet("/data2/3/2/N/WILCO")["mrn"], 2)
        self.assertIsNone(c.parse_cpdlc_packet("hello"))
        self.assertEqual(c.readable("DESCEND TO @FL330@"), "DESCEND TO FL330")


class StationTests(unittest.TestCase):
    def test_logon_auto_accept_and_connected(self):
        hop, st = make()
        hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]
        st.poll_once()
        self.assertTrue(st.is_connected("BAW123"))
        acc = [x for x in hop.sent if x["type"] == "cpdlc"][0]
        self.assertEqual((acc["to"], acc["from"], acc["packet"]), ("BAW123", "CZQO", "/data2/1/5/NE/LOGON ACCEPTED"))
        self.assertEqual(st.status()["connected"][0]["callsign"], "BAW123")

    def test_telex_clearance_request_reaches_the_list(self):
        hop, st = make()
        hop.queue = [("BAW2606", "OCEAN REQUEST ENTRY POINT: VENER AT: 0812Z REQ: M0.85 FL370", "telex")]
        st.poll_once()
        reqs = st.status()["rclRequests"]
        self.assertEqual([r["callsign"] for r in reqs], ["BAW2606"])
        rep = [x for x in hop.sent if x["type"] == "telex"]
        self.assertEqual(rep[0]["to"], "BAW2606")
        self.assertIn("RCL RECEIVED", rep[0]["packet"])

    def test_send_telex_to_pilot_not_on_cpdlc(self):
        hop, st = make()
        st.send_telex("BAW2606", "CLX 1103 CLRD TO KMIA", by="NAT_FSS (A)")
        t = [x for x in hop.sent if x["type"] == "telex"][0]
        self.assertEqual((t["to"], t["packet"]), ("BAW2606", "CLX 1103 CLRD TO KMIA"))

    def test_other_telex_is_logged_and_unseen(self):
        hop, st = make()
        hop.queue = [("BAW2606", "HELLO CAN YOU HELP", "telex")]
        st.poll_once()
        self.assertEqual(st.status()["rclRequests"], [])

    def test_logon_accept_sends_the_squawk_message(self):
        hop, st = make()
        hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]
        st.poll_once()
        sent = [x["packet"] for x in hop.sent if x["type"] == "cpdlc"]
        self.assertEqual(sent[0], "/data2/1/5/NE/LOGON ACCEPTED")
        self.assertEqual(sent[1], "/data2/2//NE/SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA")

    def test_request_text_gives_mach_and_level(self):
        f = c.parse_rcl_text
        self.assertEqual(f("OCEAN REQUEST ENTRY POINT: VENER AT: 0812Z REQ: M0.85 FL370"), {"mach": 85, "level": 370})
        self.assertEqual(f("REQUEST CLEARANCE M086 F400"), {"mach": 86, "level": 400})
        self.assertEqual(f("OCEANIC CLEARANCE REQUEST"), {"mach": None, "level": None})

    def test_rcl_enrich_uses_the_asked_mach(self):
        class Nat:
            error = None
            def by_callsign(self): return {}
        class Store:
            def get(self, cs): return {"assignedLevel": 380, "assignedMach": 0, "route": "", "routeEtas": ""}
        out = s.rcl_enrich([{"id": 1, "ts": 1.0, "callsign": "BAW9", "text": "OCEAN REQUEST ENTRY POINT: DOGAL AT: 1138Z REQ: M0.86 FL400"}], Nat(), Store(), None)
        self.assertEqual((out[0]["mach"], out[0]["level"]), (86, 400))

    def test_old_messages_are_not_shown(self):
        clock = [1_000_000.0]
        hop, st = make(now=lambda: clock[0])
        st._store("DOWN", "BAW123", 1, None, "Y", "REQUEST LOGON", "", 1)          # yesterday's logon
        st._store("UP", "BAW123", 2, 1, "NE", "LOGON ACCEPTED", "", 1)
        clock[0] += 86400
        hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]                     # logs on again today
        st.poll_once()
        texts = [(x["text"], x["ts"]) for x in st.messages("BAW123")]
        self.assertTrue(texts and all(ts >= clock[0] - 120 for _, ts in texts), texts)   # nothing from yesterday
        self.assertTrue(any(t.startswith("REQUEST LOGON") for t, _ in texts))
        clock[0] += 7 * 3600                                                     # still connected, but the old part ages out
        self.assertEqual(st.messages(), [])

    def test_old_mail_is_not_unread_and_reply_clears(self):
        hop, st = make()
        hop.queue = [("OLD1", "/data2/9//Y/REQUEST LEVEL FL360")]       # waiting at Hoppie, pilot is not logged on
        st.poll_once()
        self.assertEqual(st.unread(), {})
        hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]
        st.poll_once()
        hop.queue = [("BAW123", "/data2/6//Y/REQUEST LEVEL FL380")]
        st.poll_once()
        self.assertEqual(st.unread(), {"BAW123": 1})
        st.send("BAW123", "ROGER", ra="NE")
        self.assertEqual(st.unread(), {})

    def test_manual_accept_waits(self):
        hop, st = make(auto=False)
        hop.queue = [("DLH1", "/data2/2//Y/REQUEST LOGON")]
        st.poll_once()
        self.assertFalse(st.is_connected("DLH1"))
        st.accept_logon("DLH1", 2)
        self.assertTrue(st.is_connected("DLH1"))

    def test_uplink_and_wilco(self):
        hop, st = make()
        hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]
        st.poll_once()
        with self.assertRaises(ValueError):
            st.send("NOPE1", "CLIMB TO FL350")
        with self.assertRaises(ValueError):
            st.send("BAW123", "CLIMB", ra="XX")
        m = st.send("BAW123", "climb to and maintain @fl350@", "WU", by="CZQO_CTR (Gary)")
        self.assertEqual((m["dir"], m["text"], m["state"], m["by"]), ("UP", "CLIMB TO AND MAINTAIN @FL350@", "SENT", "CZQO_CTR (Gary)"))
        self.assertEqual(hop.sent[-1]["packet"], "/data2/3//WU/CLIMB TO AND MAINTAIN @FL350@")
        hop.queue = [("BAW123", "/data2/6/%d/N/WILCO" % m["min"])]
        st.poll_once()
        self.assertEqual([x for x in st.messages("BAW123") if x["dir"] == "UP" and x["min"] == m["min"]][0]["state"], "WILCO")
        self.assertEqual(st.unread()["BAW123"], 1)
        st.mark_seen("BAW123")
        self.assertEqual(st.unread(), {})

    def test_pilot_request_opens_and_is_answered(self):
        hop, st = make()
        hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]
        st.poll_once()
        hop.queue = [("BAW123", "/data2/7//Y/REQUEST CLIMB TO FL370")]
        st.poll_once()
        req = [x for x in st.messages("BAW123") if x["text"].startswith("REQUEST CLIMB")][0]
        self.assertEqual((req["state"], req["seen"]), ("OPEN", False))
        st.send("BAW123", "UNABLE", "NE", mrn=req["min"])
        self.assertEqual([x for x in st.messages("BAW123") if x["id"] == req["id"]][0]["state"], "ANSWERED")
        self.assertEqual(hop.sent[-1]["packet"], "/data2/3/7/NE/UNABLE")

    def test_logoff_and_ping_drop(self):
        hop, st = make()
        for cs in ("A1", "B2"):
            hop.queue = [(cs, "/data2/1//Y/REQUEST LOGON")]
            st.poll_once()
        hop.queue = [("A1", "/data2/2//N/LOGOFF")]
        st.poll_once()
        self.assertEqual(st.connected_callsigns(), {"B2"})
        st.ping_connected()                       # B2 not online
        self.assertTrue(st.is_connected("B2"))
        st.ping_connected()
        self.assertFalse(st.is_connected("B2"))
        hop.online = {"B2"}
        hop.queue = [("B2", "/data2/3//Y/REQUEST LOGON")]
        st.poll_once()
        st.ping_connected(); st.ping_connected()
        self.assertTrue(st.is_connected("B2"))

    def test_errors_do_not_crash(self):
        hop, st = make()
        hop.refuse = True
        self.assertEqual(st.poll_once(), 0)
        self.assertIn("callsign already in use", st.error)
        hop.refuse = False
        hop.queue = [("X1", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        hop.refuse = True
        with self.assertRaises(RuntimeError):
            st.send("X1", "HELLO")

    def test_connections_cleared_on_restart(self):
        hop, st = make()
        hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        path = st.db.execute("PRAGMA database_list").fetchone()[2]
        st2 = c.CpdlcStation(path, c.HoppieClient("http://x", "code", "CZQO", http=hop))
        self.assertFalse(st2.is_connected("BAW9"))               # pilots must log on again after a stop

    def test_stop_ends_loop_and_forgets_code(self):
        hop, st = make()
        t = threading.Thread(target=st.loop, daemon=True)
        t.start()
        time.sleep(0.3)
        st.stop()
        t.join(3)
        self.assertFalse(t.is_alive())
        self.assertEqual(st.client.logon, "")


class AutomationTests(unittest.TestCase):
    def uplinks(self, hop):
        return [x["packet"] for x in hop.sent if x["type"] == "cpdlc"]

    def test_oceanic_clearance_request_is_refused_when_intake_is_off(self):
        old, c.RCL_INTAKE = c.RCL_INTAKE, False
        self.addCleanup(setattr, c, "RCL_INTAKE", old)
        hop, st = make()
        hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        hop.queue = [("BAW9", "/data2/2//Y/REQUEST OCEANIC CLEARANCE NAT A FL350 M84")]
        st.poll_once()
        last = self.uplinks(hop)[-1]
        self.assertIn("UNABLE TO PROVIDE OCEANIC CLEARANCES VIA CPDLC", last)
        self.assertIn("NATTRAK.VATSIM.NET", last)
        self.assertTrue(last.startswith("/data2/") and "/2/R/" in last)          # answers message 2, reply wanted (roger)
        m = [x for x in st.messages("BAW9") if x["dir"] == "DOWN" and "OCEANIC" in x["text"]][0]
        self.assertEqual((m["state"], m["seen"]), ("ANSWERED", True))            # no alert for the controller

    def test_oceanic_clearance_request_is_kept_and_acknowledged(self):
        hop, st = make()
        hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        hop.queue = [("BAW9", "/data2/2//Y/REQUEST OCEANIC CLEARANCE NAT A FL350 M84")]
        st.poll_once()
        last = self.uplinks(hop)[-1]
        self.assertIn("RCL RECEIVED - STANDBY FOR CLEARANCE", last)
        self.assertTrue("/2/NE/" in last)                                        # answers message 2, no reply wanted
        reqs = st.status()["rclRequests"]
        self.assertEqual([(r["callsign"], r["text"]) for r in reqs], [("BAW9", "REQUEST OCEANIC CLEARANCE NAT A FL350 M84")])
        m = [x for x in st.messages("BAW9") if x["dir"] == "DOWN" and "OCEANIC" in x["text"]][0]
        self.assertFalse(m["seen"])                                              # the controller is alerted

    def test_repeated_request_updates_the_same_entry_and_dismiss_removes_it(self):
        hop, st = make()
        hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        for n, txt in ((2, "REQUEST OCEANIC CLEARANCE NAT A"), (3, "REQUEST OCEANIC CLEARANCE NAT B")):
            hop.queue = [("BAW9", "/data2/%d//Y/%s" % (n, txt))]
            st.poll_once()
        reqs = st.status()["rclRequests"]
        self.assertEqual(len(reqs), 1)
        self.assertEqual(reqs[0]["text"], "REQUEST OCEANIC CLEARANCE NAT B")
        self.assertTrue(st.rcl_dismiss(reqs[0]["id"]))
        self.assertEqual(st.status()["rclRequests"], [])
        self.assertFalse(st.rcl_dismiss(reqs[0]["id"]))

    def test_rcl_enrich_adds_flight_data_and_natrak_state(self):
        class Nat:
            def by_callsign(self_):
                return {"BAW9": {"status": "CLEARED", "level": "350", "mach": "0.84", "nat": "A", "fix": "PIKIL", "clearance_issued": "2026-11-03T15:32:59.000000Z"}}
        class Store:
            def get(self_, cs):
                return {"arrival": "KJFK", "route": "PIKIL 57/20", "routeEtas": "1507", "assignedLevel": 350, "assignedMach": 84, "track": "A"} if cs == "BAW9" else None
        out = s.rcl_enrich([{"id": 1, "ts": 1.0, "callsign": "BAW9", "text": "X"}, {"id": 2, "ts": 2.0, "callsign": "DLH1", "text": "Y"}], Nat(), Store(), None)
        self.assertEqual(out[0]["dest"], "KJFK")
        self.assertEqual(out[0]["natTrak"]["status"], "CLEARED")
        self.assertGreater(out[0]["natTrak"]["issued"], 1.7e9)
        self.assertIsNone(out[1]["natTrak"])
        self.assertEqual(out[1]["dest"], "")

    def test_logon_distance_rule(self):
        for answer, connected in ((True, True), (None, True), (False, False)):
            hop, st = make(locate=lambda cs, a=answer: a)
            hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
            st.poll_once()
            self.assertEqual(st.is_connected("BAW9"), connected)
            if not connected:
                self.assertIn("UNABLE. CONTACT CONTROLLER ON VOICE 20 MILES BEFORE OCEANIC ENTRY POINT", self.uplinks(hop)[-1])
                self.assertNotIn("LOGON ACCEPTED", " ".join(self.uplinks(hop)))

    def test_near_oceanic(self):
        tracks = [{"route": [{"latitude": 52, "longitude": -15}, {"latitude": 52, "longitude": -30}]}]
        self.assertTrue(c.near_oceanic(52, -10, tracks))            # about 185 nm from the entry
        self.assertTrue(c.near_oceanic(51.5, -0.1, tracks))         # London, about 550 nm
        self.assertFalse(c.near_oceanic(55.7, 37.6, tracks))        # Moscow
        self.assertIsNone(c.near_oceanic(52, -10, []))

    def connect(self, **kw):
        hop, st = make(**kw)
        hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        return hop, st

    def test_handoff_then_wilco_logs_off(self):
        hop, st = self.connect()
        row = st.handoff("BAW9", "Shannon Control", "127.500", by="CZQO_CTR")
        self.assertIn("CONTACT SHANNON CONTROL ON 127.500, BYE.", self.uplinks(hop)[-1])
        self.assertTrue(st.is_connected("BAW9"))
        hop.queue = [("BAW9", "/data2/9/%d/N/WILCO" % row["min"])]
        st.poll_once()
        self.assertFalse(st.is_connected("BAW9"))
        self.assertIn("/NE/LOGOFF", self.uplinks(hop)[-1])

    def test_handoff_without_frequency_and_unable_keeps_connection(self):
        hop, st = self.connect()
        row = st.handoff("BAW9", "Brest Control", "199.998")
        self.assertIn("CONTACT BREST CONTROL, BYE.", self.uplinks(hop)[-1])
        hop.queue = [("BAW9", "/data2/9/%d/N/UNABLE" % row["min"])]
        st.poll_once()
        self.assertTrue(st.is_connected("BAW9"))
        self.assertNotIn("/NE/LOGOFF", " ".join(self.uplinks(hop)))

    def test_unicom_wilco_logs_off_and_asks_for_release(self):
        released = []
        hop, st = self.connect(on_release=released.append)
        row = st.unicom("BAW9", by="CZQO_CTR")
        self.assertIn("THERE IS NO FURTHER ATC AVAILABLE. SERVICE TERMINATED. MONITOR ADVISORY ON 122.800", self.uplinks(hop)[-1])
        hop.queue = [("BAW9", "/data2/9/%d/N/ROGER" % row["min"])]
        st.poll_once()
        self.assertFalse(st.is_connected("BAW9"))
        self.assertEqual(released, ["BAW9"])

    def test_controller_can_log_a_pilot_off(self):
        hop, st = self.connect()
        r = st.disconnect("baw9", by="CZQO_CTR")
        self.assertTrue(r["ok"])
        self.assertTrue(self.uplinks(hop)[-1].endswith("/NE/LOGOFF"))
        self.assertFalse(st.is_connected("BAW9"))
        with self.assertRaises(ValueError):
            st.disconnect("BAW9")                       # not connected any more
        with self.assertRaises(ValueError):
            st.disconnect("??")

    def test_other_reply_does_not_log_off(self):
        hop, st = self.connect()
        row = st.unicom("BAW9")
        hop.queue = [("BAW9", "/data2/9/%d/N/STANDBY" % row["min"])]
        st.poll_once()
        self.assertTrue(st.is_connected("BAW9"))

    def test_station_names(self):
        self.assertEqual(c.station_display_name("EISN_CTR"), "Shannon Control")
        self.assertEqual(c.station_display_name("EGPX_N_CTR"), "Scottish Control")
        self.assertEqual(c.station_display_name("NAT_FSS"), "Shanwick and Gander Radio")
        self.assertEqual(c.station_display_name("LFRR_CTR"), "Brest Control")
        self.assertTrue(any("UNICOM" in f["ra"] for f in c.FREETEXTS))


class HubTests(unittest.TestCase):
    def make_hub(self, alive=None):
        hop = FakeHoppie()
        hub = c.CpdlcHub(os.path.join(tempfile.mkdtemp(), "h.db"), make_client=lambda code, station: c.HoppieClient("http://x", code, station, http=hop),
                         is_session_alive=(lambda t: t in alive) if alive is not None else None, start_threads=False)
        return hop, hub

    def test_station_for_position(self):
        _, hub = self.make_hub()
        self.assertEqual([hub.station_for(p) for p in ("CZQO_CTR", "CZQO_DEL", "EGGX_CTR", "NAT_FSS", "OBSERVER", "FOO_CTR", "")],
                         ["CZQX", None, "EGGX", "NATX", None, None, None])
        self.assertEqual(c.parse_stations("czqo_ctr=czqx, x=")["CZQO_CTR"], "CZQX")

    def test_start_validates_and_blocks_second_controller(self):
        hop, hub = self.make_hub()
        self.assertIn("no CPDLC station", hub.start("t1", "FOO_CTR", "abcdef12")[1])
        self.assertIn("logon code", hub.start("t1", "CZQO_CTR", "x")[1])
        self.assertEqual(hub.start("t1", "CZQO_CTR", "abcdef12"), ("CZQX", None))
        self.assertEqual(hop.sent[-1]["logon"], "abcdef12")
        self.assertIn("already in use", hub.start("t2", "CZQO_CTR", "zzzzzz99")[1])
        self.assertEqual(hub.start("t1", "CZQO_CTR", "newcode99"), ("CZQX", None))      # same controller may re-enter the code
        self.assertEqual(hub.get("t1").client.logon, "newcode99")
        self.assertEqual(hub.start("t3", "EGGX_CTR", "otherone1")[0], "EGGX")
        hub.stop("t1")
        self.assertIsNone(hub.get("t1"))
        self.assertEqual(hub.start("t2", "CZQO_CTR", "zzzzzz99")[0], "CZQX")

    def test_bad_code_rejected(self):
        hop, hub = self.make_hub()
        hop.refuse = True
        self.assertIn("Hoppie said", hub.start("t1", "CZQO_CTR", "abcdef12")[1])
        self.assertIsNone(hub.get("t1"))

    def test_hub_handoff_and_release_flag(self):
        hop, hub = self.make_hub()
        hub.start("t1", "CZQO_CTR", "abcdef12")
        st = hub.get("t1")
        hop.queue = [("BAW9", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        self.assertTrue(hub.station_with("BAW9") is st)
        msg = hub.handoff("BAW9", "EISN_CTR", "127.500", by="x")
        self.assertIn("CONTACT SHANNON CONTROL ON 127.500, BYE.", msg["text"])
        with self.assertRaises(ValueError):
            hub.handoff("NOPE1", "EISN_CTR")
        self.assertFalse(hub.released("BAW9"))
        hub._mark_released("BAW9")
        self.assertTrue(hub.released("BAW9"))

    def test_reap_when_session_ends(self):
        alive = {"t1"}
        _, hub = self.make_hub(alive)
        hub.start("t1", "CZQO_CTR", "abcdef12")
        hub.reap()
        self.assertIsNotNone(hub.get("t1"))
        alive.clear()
        hub.reap()
        self.assertIsNone(hub.get("t1"))


class HttpTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        tmp = tempfile.mkdtemp()
        cls.hop = FakeHoppie()
        store = s.FlightStore(os.path.join(tmp, "f.db"))
        store.upsert({"callsign": "BAW123", "level": "350", "mach": "84", "track": "A", "relevant": "1"})
        store.upsert({"callsign": "DLH9", "level": "350", "mach": "84", "track": "A", "relevant": "1"})
        store.upsert({"callsign": "CONC1", "type": "CONC", "level": "500", "mach": "200", "track": "RR", "route": "52/50 52/40 52/30", "routeEtas": "1200 1215 1230", "relevant": "1"})
        cache = s.TrackCache(s.load_fixes())
        auth = s.Auth({"VNAAATS_FDD_PASSWORD": "pw"})
        cls.hub = c.CpdlcHub(os.path.join(tmp, "h.db"), make_client=lambda code, st: c.HoppieClient("http://x", code, st, http=cls.hop),
                             is_session_alive=lambda t: auth.get(t) is not None, start_threads=False)
        cls.srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(cache, store, "k" * 20, None, None, None, auth, cls.hub))
        threading.Thread(target=cls.srv.serve_forever, daemon=True).start()
        cls.base = "http://127.0.0.1:%d" % cls.srv.server_port
        cls.ck = cls.login_static("CZQO_CTR")
        assert cls.call("/cpdlc/logon", {"code": "abcdef12"}, cls.ck)[0] == 200
        cls.hop.queue = [("BAW123", "/data2/5//Y/REQUEST LOGON")]
        cls.hub.running["CZQX"]["obj"].poll_once()

    @classmethod
    def tearDownClass(cls):
        cls.srv.shutdown()

    @classmethod
    def call(cls, path, body=None, cookie=""):
        req = urllib.request.Request(cls.base + path, data=json.dumps(body).encode() if body is not None else None,
                                     headers={"Content-Type": "application/json", "Cookie": cookie})
        try:
            with urllib.request.urlopen(req) as r:
                return r.status, json.loads(r.read().decode()), r.headers.get("Set-Cookie", "")
        except urllib.error.HTTPError as e:
            return e.code, json.loads(e.read().decode()), ""

    @classmethod
    def login_static(cls, position):
        ck = cls.call("/auth/password", {"name": "Gary", "password": "pw"})[2].split(";")[0]
        if position:
            cls.call("/auth/position", {"position": position}, ck)
        return ck

    def login(self, position="CZQO_CTR"):
        return self.login_static(position)

    def test_flags_and_status(self):
        flights = {f["callsign"]: f for f in self.call("/GetAllFlightData", None, self.ck)[1]}
        self.assertTrue(flights["BAW123"]["cpdlc"])
        self.assertFalse(flights["DLH9"]["cpdlc"])
        self.assertTrue(self.call("/GetSingleFlightData?callsign=BAW123")[1][0]["cpdlc"])
        anon = self.call("/cpdlc/status")[1]
        self.assertEqual((anon["enabled"], anon["active"], anon["connected"][0]["callsign"]), (True, False, "BAW123"))
        mine = self.call("/cpdlc/status", None, self.ck)[1]
        self.assertEqual((mine["active"], mine["station"], mine["connected"][0]["callsign"]), (True, "CZQX", "BAW123"))

    def test_login_needed_for_messages_and_send(self):
        self.assertEqual(self.call("/cpdlc/messages?callsign=BAW123")[0], 401)
        self.assertEqual(self.call("/cpdlc/send", {"callsign": "BAW123", "text": "HELLO"})[0], 401)
        self.assertEqual(self.call("/cpdlc/send", {"callsign": "BAW123", "text": "HELLO"}, self.login(None))[0], 403)
        self.assertEqual(self.call("/cpdlc/logon", {"code": "abcdef12"}, self.login(None))[0], 403)

    def test_needs_code_first_and_station_in_use(self):
        ck = self.login("EGGX_DEL")
        self.assertEqual(self.call("/cpdlc/send", {"callsign": "BAW123", "text": "HELLO"}, ck)[0], 409)
        self.assertEqual(self.call("/cpdlc/messages?callsign=BAW123", None, ck)[1], [])
        busy = self.login("CZQO_CTR")                                   # CZQX is held by cls.ck
        code, body, _ = self.call("/cpdlc/logon", {"code": "zzzzzz99"}, busy)
        self.assertEqual(code, 400)
        self.assertIn("already in use", body["error"])
        self.assertTrue(self.call("/cpdlc/status", None, busy)[1]["inUse"])
        self.assertEqual(self.call("/cpdlc/logon", {"code": "bad"}, ck)[0], 400)

    def test_send_and_read(self):
        ck = self.ck
        code, msg, _ = self.call("/cpdlc/send", {"callsign": "BAW123", "text": "expect climb fl370 at 30w", "ra": "R"}, ck)
        self.assertEqual((code, msg["dir"], msg["ra"]), (200, "UP", "R"))
        self.assertEqual(self.call("/cpdlc/send", {"callsign": "DLH9", "text": "HELLO"}, ck)[0], 400)     # not connected
        self.assertEqual(self.call("/cpdlc/send", {"callsign": "BAW123", "text": "HELLO", "ra": "ZZ"}, ck)[0], 400)
        texts = [m["text"] for m in self.call("/cpdlc/messages?callsign=BAW123", None, ck)[1]]
        self.assertIn("EXPECT CLIMB FL370 AT 30W", texts)

    def test_unicom_button_and_plugin_handoff_endpoint(self):
        ck = self.ck
        code, msg, _ = self.call("/cpdlc/unicom", {"callsign": "BAW123"}, ck)
        self.assertEqual(code, 200)
        self.assertIn("NO FURTHER ATC", msg["text"])
        self.assertEqual(self.call("/cpdlc/unicom", {"callsign": "DLH9"}, ck)[0], 400)
        self.assertEqual(self.call("/CpdlcHandoff?code=wrong&callsign=BAW123&target=EISN_CTR")[0], 401)
        code, body, _ = self.call("/CpdlcHandoff?code=" + "k" * 20 + "&callsign=BAW123&target=EISN_CTR&freq=127.500&by=CZQO_CTR")
        self.assertEqual((code, body["ok"]), (200, True))
        self.assertIn("SHANNON CONTROL", body["text"])
        code, body, _ = self.call("/CpdlcHandoff?code=" + "k" * 20 + "&callsign=DLH9&target=EISN_CTR")
        self.assertEqual((code, body["ok"]), (200, False))           # not connected: harmless
        self.assertIn("cpdlcRelease", self.call("/GetSingleFlightData?callsign=BAW123")[1][0])
        self.assertTrue(any("CONTACT" in t["text"] or "UNICOM" in t["text"] for t in self.call("/cpdlc/templates")[1]))

    def test_concorde_position_reports(self):
        ck = self.ck
        self.assertEqual(self.call("/EditPosReport", {"callsign": "CONC1", "fix": "52/50", "act": "1203"})[0], 401)
        self.assertEqual(self.call("/EditPosReport", {"callsign": "CONC1", "fix": "52/50", "act": "1203"}, self.login(None))[0], 403)
        code, rec, _ = self.call("/EditPosReport", {"callsign": "CONC1", "fix": "52/50", "est": "12:00", "act": "1203z"}, ck)
        self.assertEqual(code, 200)
        self.assertEqual(rec["posReports"]["52/50"]["act"], "1203")
        self.assertEqual(rec["posReports"]["52/50"]["est"], "1200")
        self.assertIn("CZQO_CTR", rec["posReports"]["52/50"]["by"])
        self.assertEqual(self.call("/EditFlight", {"callsign": "CONC1", "mach": "200"}, ck)[0], 200)       # Mach 2.00 is allowed
        self.assertEqual(self.call("/EditPosReport", {"callsign": "CONC1", "fix": "52/40", "est": "9999"}, ck)[0], 400)
        self.assertEqual(self.call("/EditPosReport", {"callsign": "BAW123", "fix": "52/40", "est": "1210"}, ck)[0], 400)   # not Concorde
        self.assertEqual(self.call("/EditPosReport", {"callsign": "NOPE1", "fix": "52/40", "est": "1210"}, ck)[0], 404)
        flights = {f["callsign"]: f for f in self.call("/GetAllFlightData", None, ck)[1]}
        self.assertIn("52/50", flights["CONC1"]["posReports"])
        self.assertNotIn("posReports", flights["BAW123"])
        self.call("/EditPosReport", {"callsign": "CONC1", "fix": "52/50", "est": "", "act": ""}, ck)       # clearing removes the row
        self.assertEqual(self.call("/GetSingleFlightData?callsign=CONC1")[1][0]["posReports"], {})

    def test_position_change_stops_station(self):
        ck = self.login("NAT_FSS")
        self.assertEqual(self.call("/cpdlc/logon", {"code": "natcode12"}, ck)[1]["station"], "NATX")
        self.call("/auth/position", {"position": "EGGX_CTR"}, ck)
        self.assertNotIn("NATX", self.hub.running)

    def test_off_by_default(self):
        tmp = tempfile.mkdtemp()
        srv = s.ThreadingHTTPServer(("127.0.0.1", 0), s.make_handler(s.TrackCache(s.load_fixes()), s.FlightStore(os.path.join(tmp, "o.db")), "k" * 20))
        threading.Thread(target=srv.serve_forever, daemon=True).start()
        with urllib.request.urlopen("http://127.0.0.1:%d/cpdlc/status" % srv.server_port) as r:
            self.assertFalse(json.loads(r.read().decode())["enabled"])
        srv.shutdown()


if __name__ == "__main__":
    unittest.main()


class ForceLogoutTests(unittest.TestCase):
    def test_force_logout_blocks_signin_for_three_minutes(self):
        clock = [1000.0]
        hop = FakeHoppie()
        hub = c.CpdlcHub(os.path.join(tempfile.mkdtemp(), "f.db"), make_client=lambda code, st: c.HoppieClient("http://x", code, st, http=hop),
                         start_threads=False, now=lambda: clock[0])
        self.assertEqual(hub.start("t1", "CZQO_CTR", "abcdef12"), ("CZQX", None))
        self.assertIn("not a CPDLC station", hub.force_logout("ABCD", by="x")[1])
        secs, err = hub.force_logout("czqx", by="CZQO_CTR")
        self.assertEqual((secs, err), (180, ""))
        self.assertIsNone(hub.get("t1"))
        self.assertEqual(hub.status("t1", "CZQO_CTR")["kickedFrom"], "CZQX")
        self.assertIn("Wait 3:00", hub.start("t2", "CZQO_CTR", "zzzzzz99")[1])
        clock[0] += 100
        self.assertIn("Wait 1:20", hub.start("t2", "CZQO_CTR", "zzzzzz99")[1])
        self.assertEqual(hub.status("t2", "CZQO_CTR")["lockedSeconds"], 80)
        clock[0] += 81
        self.assertEqual(hub.start("t2", "CZQO_CTR", "zzzzzz99"), ("CZQX", None))
