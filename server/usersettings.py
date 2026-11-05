"""
Per-controller plugin settings chosen on the FDD (Settings button).

The server cannot reach a controller's computer, so it only keeps the choices. The plugin asks for them (/PluginSettings),
writes the changed lines into its own vnaaats.cfg and tells the controller to restart EuroScope. Only the options listed
in OPTIONS can be set here: never the key or the server address.
"""
import json
import time

# name -> allowed values (as the text written after "=" in vnaaats.cfg) and what is used when nothing is set
OPTIONS = {
    "autotype":  {"values": ("0", "1"), "default": "0"},
    "autoclick": {"values": ("0", "1"), "default": "1"},
    "highlight": {"values": ("0", "1"), "default": "1"},
    "retrack":   {"values": ("0", "1"), "default": "1"},
    "remotelog": {"values": ("0", "1", "2"), "default": "1"},
}


def clean(raw):
    """Keep only known options with allowed values. Returns (settings, error)."""
    if not isinstance(raw, dict):
        return None, "settings must be an object"
    out = {}
    for k, v in raw.items():
        if k not in OPTIONS:
            return None, "unknown setting: %s" % str(k)[:30]
        v = str(v).strip()
        if v not in OPTIONS[k]["values"]:
            return None, "%s must be one of %s" % (k, ", ".join(OPTIONS[k]["values"]))
        out[k] = v
    return out, None


class UserSettings:
    def __init__(self, store, now=time.time):
        self.store, self.now = store, now
        with store.lock:
            store.db.execute("CREATE TABLE IF NOT EXISTS user_settings (cid TEXT PRIMARY KEY, json TEXT NOT NULL, updated REAL NOT NULL)")
            store.db.commit()

    def get(self, cid):
        """{"settings": {...}, "stamp": int seconds} (settings empty and stamp 0 when the controller never chose any)."""
        with self.store.lock:
            row = self.store.db.execute("SELECT json, updated FROM user_settings WHERE cid=?", (str(cid),)).fetchone()
        if not row:
            return {"settings": {}, "stamp": 0}
        try:
            return {"settings": json.loads(row[0]), "stamp": int(row[1])}
        except ValueError:
            return {"settings": {}, "stamp": 0}

    def set(self, cid, raw):
        """Merge the given options into the saved ones. Returns (record, error)."""
        new, err = clean(raw)
        if err:
            return None, err
        cur = self.get(cid)["settings"]
        cur.update(new)
        t = int(self.now())
        with self.store.lock:
            self.store.db.execute("INSERT OR REPLACE INTO user_settings (cid, json, updated) VALUES (?,?,?)", (str(cid), json.dumps(cur), t))
            self.store.db.commit()
        return {"settings": cur, "stamp": t}, None
