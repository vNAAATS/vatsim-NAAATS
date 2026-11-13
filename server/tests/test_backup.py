import os, sqlite3, sys, tempfile, time, unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "deploy"))
import backup


class BackupTests(unittest.TestCase):
    def test_copy_is_readable_and_old_ones_are_pruned(self):
        with tempfile.TemporaryDirectory() as d:
            db = os.path.join(d, "v.db")
            c = sqlite3.connect(db); c.execute("create table t(a)"); c.execute("insert into t values (7)"); c.commit(); c.close()
            made = []
            for _ in range(3):
                made.append(backup.backup(db, os.path.join(d, "b"), keep=2))
                time.sleep(1.05)                                  # names carry the second
            left = sorted(os.listdir(os.path.join(d, "b")))
            self.assertEqual(len(left), 2)
            self.assertNotIn(os.path.basename(made[0]), left)     # the oldest went
            self.assertEqual(sqlite3.connect(made[-1]).execute("select a from t").fetchone()[0], 7)

    def test_missing_database_is_an_error(self):
        with self.assertRaises(SystemExit):
            backup.backup("/nonexistent/x.db", tempfile.gettempdir() + "/nb")


if __name__ == "__main__":
    unittest.main()
