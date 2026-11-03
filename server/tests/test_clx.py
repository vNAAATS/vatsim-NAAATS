import os, sys, unittest, tempfile
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import clx
from datetime import datetime, timezone
import test_cpdlc as tc


class ClxTests(unittest.TestCase):
    REC = {"callsign": "KLM695", "arrival": "CYYZ", "route": "DOGAL 55/20 56/30 57/40 57/50 HOIST", "routeEtas": "1507 1520 1535 1550 1605", "track": "C", "assignedLevel": 400, "assignedMach": 85}
    NOW = datetime(2026, 11, 3, 15, 32, tzinfo=timezone.utc)

    def test_matches_natrak_wording(self):
        m = clx.build("clx", self.REC, {"nat": "C", "fix": "DOGAL", "level": "400", "mach": "0.85"}, "CZQO_FSS", self.NOW)
        self.assertEqual(m[0], "CLX 1532 031126 NAT CLRNCE / KLM695 CLRD TO CYYZ VIA DOGAL / NAT C / DOGAL 55/20 56/30 57/40 57/50 HOIST / FM DOGAL/1507 MNTN F400 M085 / SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA / END OF MESSAGE")

    def test_natrak_values_win(self):
        m = clx.build("clx", self.REC, {"nat": "D", "fix": "DOGAL", "level": "380", "mach": "0.83"}, "EGGX_CTR", self.NOW)
        self.assertIn("NAT D", m[0]); self.assertIn("MNTN F380 M083", m[0])
        self.assertTrue(m[1].startswith("CONTACT SHANWICK"))

    def test_processed_and_contact(self):
        m = clx.build("processed", self.REC, None, "NAT_FSS", self.NOW)
        self.assertEqual(m[0], "RCL PROCESSED - FLY FLIGHT PLAN ROUTE")
        self.assertTrue(m[1].startswith("CONTACT NAT FSS 5 MINS PRIOR ENTRY"))
        self.assertTrue(clx.contact_line("CZQO_CTR").startswith("CONTACT GANDER"))

    def test_random_route(self):
        m = clx.build("clx", dict(self.REC, track="RR"), None, "CZQO_CTR", self.NOW)
        self.assertIn("RANDOM ROUTE", m[0])

    def test_send_switch_is_off_by_default(self):
        import cpdlc
        hop, st = tc.make()
        self.assertFalse(cpdlc.RCL_SEND)
        self.assertIn("rclSend", st.status())

    def test_sent_state_and_limits(self):
        hop, st = tc.make()
        hop.queue = [("KLM695", "/data2/1//Y/REQUEST LOGON")]
        st.poll_once()
        hop.queue = [("KLM695", "/data2/2//Y/REQUEST OCEANIC CLEARANCE")]
        st.poll_once()
        r = st.rcl_requests()[0]
        self.assertIsNone(r["sent"])
        for m in clx.build("clx", self.REC, None, "CZQO_CTR", self.NOW):
            self.assertLessEqual(len(m), 240)
            st.send("KLM695", m, "NE", None, by="x")
        st.rcl_mark_sent(r["id"], "CZQO_CTR (Gary)", "clx")
        self.assertEqual(st.rcl_get(r["id"])["sent"]["kind"], "clx")


if __name__ == "__main__":
    unittest.main()
