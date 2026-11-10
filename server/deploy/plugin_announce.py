#!/usr/bin/env python3
"""Post a Discord notice when a new plugin build is published on GitHub. Run every few minutes by a systemd timer.

Uses the same VNAAATS_DISCORD_WEBHOOK as announce.py (in /etc/vnaaats.env). Nothing is posted the first time it runs (it just
remembers the current version), so installing it never causes a surprise ping.  --force posts the current version once (a test).
Never fails loudly: any problem is just printed.
"""
import json, os, re, sys, urllib.request

REPO = os.environ.get("VNAAATS_REPO", "garythms/vatsim-NAAATS-v2")
ENV = os.environ.get("VNAAATS_ENV_FILE", "/etc/vnaaats.env")
STATE = os.environ.get("VNAAATS_PLUGIN_ANNOUNCED", "/var/lib/vnaaats-plugin-announced.json")
HOOK_PREFIXES = ("https://discord.com/api/webhooks/", "https://discordapp.com/api/webhooks/")


def read_env(path):
    out = {}
    try:
        for line in open(path, encoding="utf-8"):
            if "=" in line and not line.lstrip().startswith("#"):
                k, v = line.rstrip("\n").split("=", 1)
                out[k.strip()] = v.strip().strip('"').strip("'")
    except OSError:
        pass
    return out


def get(url):
    req = urllib.request.Request(url, headers={"User-Agent": "vnaaats-plugin-notice"})
    with urllib.request.urlopen(req, timeout=20) as r:
        return r.read().decode("utf-8")


def notes_for(readme, ver):
    """The 'NEW IN <ver>:' paragraph of the plugin README (without the heading)."""
    lines, on, out = readme.splitlines(), False, []
    for l in lines:
        if not on and re.match(r"^NEW IN %s\b" % re.escape(ver), l):
            on = True
            out.append(re.sub(r"^NEW IN [^:]*:?\s*", "", l))
            continue
        if on and l.strip() == "":
            break
        if on:
            out.append(l)
    return "\n".join(out).strip()


def main():
    force = "--force" in sys.argv
    hook = read_env(ENV).get("VNAAATS_DISCORD_WEBHOOK", "")
    if not hook.startswith(HOOK_PREFIXES):
        print("Plugin notice: no webhook set - skipped.")
        return
    try:
        rel = json.loads(get("https://api.github.com/repos/%s/releases/tags/latest-build" % REPO))
        sha = rel["target_commitish"]
        if not any(a.get("name") == "VatsimNAAATS.dll" for a in rel.get("assets", [])):
            print("Plugin notice: the build has no DLL yet - will try again later.")
            return
        ver = get("https://raw.githubusercontent.com/%s/%s/pluginversion.txt" % (REPO, sha)).strip()
        readme = get("https://raw.githubusercontent.com/%s/%s/VatsimNAAATS/README.txt" % (REPO, sha))
    except Exception as e:
        print("Plugin notice skipped: could not read GitHub (%s)" % e)
        return
    try:
        last = json.load(open(STATE)).get("version")
    except Exception:
        last = None
    if last is None and not force:
        json.dump({"version": ver}, open(STATE, "w"))
        print("Plugin notice: first run - remembering version %s without posting." % ver)
        return
    if last == ver and not force:
        return
    msg = "**vNAAATS plugin %s is out**\n" % ver
    text = notes_for(readme, ver)
    if text:
        msg += "\n" + text + "\n"
    msg += "\nUpdating? Close EuroScope and replace only VatsimNAAATS.dll (keep your vnaaats.cfg): https://vnaaats.com/download\nFirst time? https://vnaaats.com/guide"
    req = urllib.request.Request(hook, data=json.dumps({"content": msg[:1900], "allowed_mentions": {"parse": []}}).encode(),
                                 headers={"Content-Type": "application/json", "User-Agent": "vnaaats-plugin-notice"})
    try:
        urllib.request.urlopen(req, timeout=15).read()
        print("Plugin notice posted for %s." % ver)
        json.dump({"version": ver}, open(STATE, "w"))
    except Exception as e:
        print("Plugin notice failed (will try again): %s" % e)


if __name__ == "__main__":
    main()
