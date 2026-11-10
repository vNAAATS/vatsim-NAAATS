import os, sys, unittest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import vnaaats_server as s

TRACKS = [
    {"id": "F", "direction": 2, "route": [{"name": n} for n in ("HOIST", "53/50", "54/40", "55/30", "GOMUP")]},
    {"id": "A", "direction": 1, "route": [{"name": n} for n in ("RESNO", "56/20", "56/30", "56/40", "57/50")]},
]
ROUTE = "WYNDE3 FNT HOCKE KAPUX YUC YBC HOIST NATF GOMUP BELOX TENSO"


class NatTokenTests(unittest.TestCase):
    def test_entry_exit_slice(self):
        t, _, done = s.expand_nat_tokens(ROUTE.split(), TRACKS)
        self.assertEqual(" ".join(t), "WYNDE3 FNT HOCKE KAPUX YUC YBC HOIST 53/50 54/40 55/30 GOMUP BELOX TENSO")
        self.assertEqual(done, ["F"])

    def test_only_entry_or_only_exit_or_neither(self):
        self.assertEqual(" ".join(s.expand_nat_tokens("HOIST NATF".split(), TRACKS)[0]), "HOIST 53/50 54/40 55/30 GOMUP")
        self.assertEqual(" ".join(s.expand_nat_tokens("NATF GOMUP".split(), TRACKS)[0]), "HOIST 53/50 54/40 55/30 GOMUP")
        self.assertEqual(" ".join(s.expand_nat_tokens(["NATF"], TRACKS)[0]), "HOIST 53/50 54/40 55/30 GOMUP")

    def test_mid_track_join_and_reversed(self):
        self.assertEqual(" ".join(s.expand_nat_tokens("54/40 NATF GOMUP".split(), TRACKS)[0]), "54/40 55/30 GOMUP")
        self.assertEqual(" ".join(s.expand_nat_tokens("GOMUP NATF HOIST".split(), TRACKS)[0]), "GOMUP 55/30 54/40 53/50 HOIST")

    def test_unknown_track_is_dropped_and_speed_suffix(self):
        self.assertEqual(s.expand_nat_tokens("HOIST NATZ GOMUP".split(), TRACKS)[0], ["HOIST", "GOMUP"])
        self.assertEqual(" ".join(s.expand_nat_tokens("HOIST NATF/N0480F360 GOMUP".split(), TRACKS)[0]), "HOIST 53/50 54/40 55/30 GOMUP")
        self.assertEqual(s.expand_nat_tokens("ABCDE DCT FGHIJ".split(), TRACKS)[0], ["ABCDE", "DCT", "FGHIJ"])

    def test_etas_stay_aligned(self):
        toks = "HOIST NATF GOMUP BELOX".split()
        t, e, _ = s.expand_nat_tokens(toks, TRACKS, ["1200", "--", "1330", "1400"])
        self.assertEqual(len(t), len(e))
        self.assertEqual((e[0], e[-2], e[-1]), ("1200", "1330", "1400"))

    def test_network_pilot_gets_the_track(self):
        pilot = {"callsign": "FBU760D", "flight_plan": {"route": ROUTE, "altitude": "36000", "departure": "KDTW", "arrival": "EDDF", "aircraft_short": "A333"}}
        rec = s.convert_network_pilot(pilot, TRACKS)
        self.assertIsNotNone(rec)
        self.assertEqual(rec["route"], "HOIST 53/50 54/40 55/30 GOMUP")
        self.assertEqual(rec["track"], "F")
        self.assertTrue(rec["direction"])                      # moving east


if __name__ == "__main__":
    unittest.main()


class InactiveTrackTests(unittest.TestCase):
    """A flight that filed NATE when track E is not in use must still reach the display (as RR), not vanish."""
    def pilot(self, route):
        return {"callsign": "TST1", "flight_plan": {"route": route, "departure": "EGLL", "arrival": "KJFK", "altitude": "35000", "aircraft_short": "B77W"}}

    def test_inactive_nat_token_still_listed(self):
        rec = s.convert_network_pilot(self.pilot("DCT NATE DCT"), [])
        self.assertIsNotNone(rec)
        self.assertEqual((rec["track"], rec["callsign"]), ("RR", "TST1"))

    def test_with_entry_and_exit_fixes(self):
        rec = s.convert_network_pilot(self.pilot("ALLRY NATE GISTI"), [{"id": "A", "route": []}])
        self.assertEqual(rec["route"], "ALLRY GISTI")

    def test_no_nat_token_still_ignored(self):
        self.assertIsNone(s.convert_network_pilot(self.pilot("DCT KOK DCT LGL"), []))
