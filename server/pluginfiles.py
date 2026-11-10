"""
The latest released plugin DLL, for the vNAAATS app (it checks the controller's DLL and replaces an old one).
The server fetches it from the GitHub release page, keeps a copy for a while and serves it from /plugin/.
"""
import hashlib
import json
import threading
import time
import urllib.request

CACHE_SECONDS = 900
MAX_BYTES = 20 * 1024 * 1024


class PluginFiles:
    def __init__(self, repo, http=None, now=time.time):
        self.repo = repo
        self.http = http or self._real_http
        self.now = now
        self.lock = threading.Lock()
        self.info = None          # {"version", "sha256", "size"}
        self.data = b""
        self.checked = 0.0

    @staticmethod
    def _real_http(url):
        req = urllib.request.Request(url, headers={"User-Agent": "vnaaats-server", "Accept": "application/vnd.github+json"})
        with urllib.request.urlopen(req, timeout=30) as r:
            return r.read(MAX_BYTES + 1)

    def _refresh(self):
        rel = json.loads(self.http("https://api.github.com/repos/%s/releases/latest" % self.repo).decode("utf-8"))
        asset = next((a for a in rel.get("assets", []) if a.get("name") == "VatsimNAAATS.dll"), None)
        if not asset:
            raise ValueError("the latest release has no VatsimNAAATS.dll")
        url = asset.get("browser_download_url", "")
        if not url.startswith("https://github.com/%s/" % self.repo):
            raise ValueError("unexpected download address")
        blob = self.http(url)
        if not blob or len(blob) > MAX_BYTES or blob[:2] != b"MZ":
            raise ValueError("that is not a DLL")
        self.data = blob
        self.info = {"version": str(rel.get("tag_name", "")).lstrip("v"), "sha256": hashlib.sha256(blob).hexdigest(), "size": len(blob)}

    def latest(self):
        """(info, bytes) or (None, b"") when it cannot be fetched and nothing is cached."""
        with self.lock:
            if self.info is None or self.now() - self.checked > CACHE_SECONDS:
                self.checked = self.now()                     # a failure is retried after the cache time, not on every request
                try:
                    self._refresh()
                except Exception:
                    pass
            return self.info, self.data
