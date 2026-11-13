#!/usr/bin/env python3
"""Post a short notice to a Discord channel when a new server / display version has been installed.

Used by setup.sh. Does nothing unless VNAAATS_DISCORD_WEBHOOK=... is set in /etc/vnaaats.env, and only posts when the
version changed since the last notice. Never fails the install: any problem is just printed.

  announce.py <package folder>
The package folder may hold web/version.json ({"version": "0.12.0"}) and CHANGELOG-display.md (newest entry on top).
"""
import json, os, re, sys, urllib.request

ENV = os.environ.get("VNAAATS_ENV_FILE", "/etc/vnaaats.env")
STATE = os.environ.get("VNAAATS_ANNOUNCED", "/var/lib/vnaaats-announced.json")
HOOK_PREFIXES = ("https://discord.com/api/webhooks/", "https://discordapp.com/api/webhooks/")
HEALTH = os.environ.get("VNAAATS_HEALTH_URL", "http://127.0.0.1:8080/health")


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


def newest_changelog(path, max_chars=1400):
    """Text of the first '## version' section of the display changelog (bullets only)."""
    try:
        text = open(path, encoding="utf-8").read()
    except OSError:
        return ""
    m = re.search(r"^## .*?\n(.*?)(?=^## |\Z)", text, re.S | re.M)
    body = (m.group(1) if m else "").strip()
    return body[:max_chars].rstrip()


def main():
    pkg = sys.argv[1] if len(sys.argv) > 1 else "."
    hook = read_env(ENV).get("VNAAATS_DISCORD_WEBHOOK", "")
    if not hook.startswith(HOOK_PREFIXES):
        print("Discord notice: no webhook set (VNAAATS_DISCORD_WEBHOOK in /etc/vnaaats.env) - skipped.")
        return
    try:
        server_v = json.load(urllib.request.urlopen(HEALTH, timeout=10)).get("version", "")
    except Exception as e:
        print("Discord notice skipped: server did not answer (%s)" % e)
        return
    try:
        display_v = json.load(open(os.path.join(pkg, "web", "version.json"))).get("version", "")
    except Exception:
        display_v = ""
    now = {"server": server_v, "display": display_v}
    try:
        if json.load(open(STATE)) == now:
            print("Discord notice: versions unchanged - not posting again.")
            return
    except Exception:
        pass
    msg = "**vNAAATS update installed** - server %s%s\n" % (server_v, (", display " + display_v) if display_v else "")
    notes = newest_changelog(os.path.join(pkg, "CHANGELOG-display.md"))
    if notes:
        msg += "\n" + notes + "\n"
    msg += "\nHard refresh the page (Ctrl+Shift+R) to pick it up: https://vnaaats.com/fdd"
    req = urllib.request.Request(hook, data=json.dumps({"content": msg[:1900], "allowed_mentions": {"parse": []}}).encode(),
                                 headers={"Content-Type": "application/json", "User-Agent": "vnaaats-announce"})
    try:
        urllib.request.urlopen(req, timeout=15).read()
        print("Discord notice posted.")
        try:
            json.dump(now, open(STATE, "w"))
        except OSError:
            pass
    except Exception as e:
        print("Discord notice failed (the install is fine): %s" % e)


if __name__ == "__main__":
    main()
