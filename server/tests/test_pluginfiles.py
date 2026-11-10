import json, os, sys, unittest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import pluginfiles


def fake_http(calls):
    dll = b"MZ" + b"\0" * 100
    def http(url):
        calls.append(url)
        if "api.github.com" in url:
            return json.dumps({"tag_name": "v2.5.1", "assets": [{"name": "VatsimNAAATS.dll", "browser_download_url": "https://github.com/me/repo/releases/download/v2.5.1/VatsimNAAATS.dll"}]}).encode()
        return dll
    return http


class PluginFilesTests(unittest.TestCase):
    def test_fetch_and_cache(self):
        calls, now = [], [1000.0]
        pf = pluginfiles.PluginFiles("me/repo", http=fake_http(calls), now=lambda: now[0])
        info, blob = pf.latest()
        self.assertEqual((info["version"], info["size"], blob[:2]), ("2.5.1", 102, b"MZ"))
        n = len(calls)
        pf.latest()
        self.assertEqual(len(calls), n)                       # cached
        now[0] += 1000
        pf.latest()
        self.assertGreater(len(calls), n)                     # refreshed after the cache time

    def test_rejects_non_dll_and_foreign_url(self):
        def bad(url):
            if "api.github.com" in url:
                return json.dumps({"tag_name": "v1", "assets": [{"name": "VatsimNAAATS.dll", "browser_download_url": "https://evil.example/x.dll"}]}).encode()
            return b"MZ"
        info, blob = pluginfiles.PluginFiles("me/repo", http=bad).latest()
        self.assertIsNone(info)
        def notdll(url):
            if "api.github.com" in url:
                return json.dumps({"tag_name": "v1", "assets": [{"name": "VatsimNAAATS.dll", "browser_download_url": "https://github.com/me/repo/a.dll"}]}).encode()
            return b"<html>"
        self.assertIsNone(pluginfiles.PluginFiles("me/repo", http=notdll).latest()[0])

    def test_failure_keeps_old_copy(self):
        calls, now = [], [0.0]
        pf = pluginfiles.PluginFiles("me/repo", http=fake_http(calls), now=lambda: now[0])
        pf.latest()
        pf.http = lambda url: (_ for _ in ()).throw(OSError("down"))
        now[0] += 2000
        self.assertEqual(pf.latest()[0]["version"], "2.5.1")


if __name__ == "__main__":
    unittest.main()
