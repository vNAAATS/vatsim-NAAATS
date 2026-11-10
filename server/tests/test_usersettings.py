import os, sys, tempfile, unittest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import usersettings as us
import vnaaats_server as s


class UserSettingsTests(unittest.TestCase):
    def setUp(self):
        self.store = s.FlightStore(os.path.join(tempfile.mkdtemp(), "u.db"))
        self.t = [5000.0]
        self.u = us.UserSettings(self.store, now=lambda: self.t[0])

    def test_nothing_chosen_yet(self):
        self.assertEqual(self.u.get("123"), {"settings": {}, "stamp": 0})

    def test_save_and_merge(self):
        rec, err = self.u.set("123", {"autotype": 1, "remotelog": "2"})
        self.assertIsNone(err)
        self.assertEqual(rec["settings"], {"autotype": "1", "remotelog": "2"})
        self.t[0] = 6000.0
        rec, _ = self.u.set("123", {"autoclick": "0"})
        self.assertEqual(rec["settings"], {"autotype": "1", "remotelog": "2", "autoclick": "0"})
        self.assertEqual(rec["stamp"], 6000)
        self.assertEqual(self.u.get("999")["settings"], {})               # per controller

    def test_only_known_options_and_values(self):
        for bad in ({"key": "x"}, {"server": "http://evil"}, {"remotelog": "3"}, {"autotype": "yes"}, "nope", None):
            rec, err = self.u.set("123", bad)
            self.assertIsNone(rec)
            self.assertTrue(err)
        self.assertEqual(self.u.get("123")["settings"], {})


if __name__ == "__main__":
    unittest.main()
