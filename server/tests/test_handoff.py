import os, sys, unittest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import handoff as h

ROUTE = [("A", 55, -40), ("RESNO", 55, -30), ("55/20", 55, -20), ("DOGAL", 54.5, -10)]
SHANNON = [{"callsign": "EISN_N_CTR", "frequency": "122.975"}, {"callsign": "EISN_ATIS", "frequency": "1"}]


class HandoffTests(unittest.TestCase):
    def test_exit_to_shannon_soon_with_controller(self):
        r = h.hint(ROUTE, (55, -11), SHANNON)
        self.assertEqual((r["facility"], r["fix"], r["controller"], r["soon"], r["unicom"]), ("EISN", "DOGAL", "EISN_N_CTR", True, False))

    def test_not_soon_when_far(self):
        r = h.hint(ROUTE, (55, -25), SHANNON)
        self.assertFalse(r["soon"])
        self.assertGreater(r["nm"], 400)

    def test_30w_crossing_goes_to_the_other_centre(self):
        r = h.hint(ROUTE, (55, -35), [{"callsign": "EGGX_CTR", "frequency": "127.9"}])
        self.assertEqual((r["facility"], r["fix"], r["controller"]), ("EGGX", "30W", "EGGX_CTR"))
        w = h.hint(list(reversed(ROUTE)), (55, -25), [{"callsign": "CZQO_CTR", "frequency": "131.7"}])
        self.assertEqual((w["facility"], w["fix"]), ("CZQO", "30W"))

    def test_combined_nat_position_has_no_30w_handoff(self):
        east = [("RESNO", 55, -40), ("55/30", 55, -30), ("55/20", 55, -20), ("DOGAL", 54.5, -10)]
        nat = [{"callsign": "NAT_FSS", "frequency": "127.9"}]
        r = h.hint(east, (55, -29), nat)                       # just past 30W: not leaving oceanic airspace, nothing to hand over yet
        self.assertEqual((r["facility"], r["fix"]), ("EISN", "DOGAL"))
        self.assertTrue(r["unicom"])                           # nobody online for Shannon, but that is at DOGAL, not at 30W
        self.assertGreater(r["nm"], 300)
        self.assertEqual(h.hint(east, (55, -35), nat)["fix"], "DOGAL")
        self.assertEqual(h.hint(east, (55, -35), [{"callsign": "NAT_1_FSS"}])["fix"], "DOGAL")
        self.assertEqual(h.hint(east, (55, -35), [{"callsign": "EGGX_CTR"}])["fix"], "30W")      # separate centres: still handed over

    def test_14w_track_end_points_go_to_shannon(self):
        r = h.hint([("A", 52.5, -30), ("RILED", 52.5, -14)], (52.5, -20), SHANNON)
        self.assertEqual((r["facility"], r["fix"], r["controller"]), ("EISN", "RILED", "EISN_N_CTR"))

    def test_nobody_online_means_unicom(self):
        r = h.hint(ROUTE, (55, -11), [])
        self.assertTrue(r["unicom"] and r["controller"] == "")

    def test_far_from_the_route_gives_nothing(self):
        self.assertIsNone(h.hint(ROUTE, (40, -11), SHANNON))
        self.assertIsNone(h.hint(ROUTE[:1], (55, -35), SHANNON))

    def test_unknown_exit_fix_gives_nothing(self):
        self.assertIsNone(h.hint([("A", 50, -20), ("ZZZZZ", 50, -10)], (50, -15), SHANNON))

    def test_short_code_of_the_online_position(self):
        r = h.hint([("46/50", 46, -50), ("ALLRY", 46, -52.5)], (46, -52), [{"callsign": "CZQM_CTR", "frequency": "132.200"}, {"callsign": "CZQX_CTR", "frequency": "132.100"}])
        self.assertEqual(h.CODES.get("CZQM_CTR"), "QM")
        self.assertEqual([o["callsign"] for o in r["online"]], ["CZQX_CTR", "CZQM_CTR"])   # Gander Domestic first, Moncton as the stand-in
        r2 = h.hint([("46/50", 46, -50), ("ALLRY", 46, -52.5)], (46, -52), [{"callsign": "CZQM_CTR", "frequency": "132.200"}])
        self.assertEqual((r2["controller"], r2["code"], r2["unicom"]), ("CZQM_CTR", "QM", False))
        r3 = h.hint([("46/50", 46, -50), ("ALLRY", 46, -52.5)], (46, -52), [{"callsign": "CZQX_APP", "frequency": "1"}])
        self.assertTrue(r3["unicom"])                                                        # approach is not a handoff target

    def test_gander_domestic_exit(self):
        r = h.hint([("46/50", 46, -50), ("ALLRY", 46, -52.5)], (46, -52), [{"callsign": "CZQX_CTR", "frequency": "132.1"}])
        self.assertEqual((r["facility"], r["controller"]), ("CZQX", "CZQX_CTR"))

if __name__ == "__main__":
    unittest.main()
