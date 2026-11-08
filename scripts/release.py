#!/usr/bin/env python3
"""One-step release prep: python3 scripts/release.py --server 2.8.0 --display 2.8.0 [--plugin 2.8.0] [--whatsnew] [--date 2026-11-09]

Run it on a branch after the work is merged (or in the PR itself). It
  * sets SERVER_VERSION, web/src/config.json "version" and, with --plugin, PLUGIN_VERSION + pluginversion.txt
  * renames "## Unreleased" in web/CHANGELOG.md to "## <display> - <date>" (the Discord announcer reads the first section)
  * with --plugin, adds a "NEW IN <plugin>:" note to VatsimNAAATS/README.txt built from the changelog lines that start "Plugin:"
  * with --whatsnew, adds a "what's new" popup entry at the top of web/src/typescript/fdd/whatsnew.ts from the changelog lines
Then commit, merge, and push master to server-and-squawk as usual. Nothing is committed by this script. Review `git diff` first.
"""
import argparse, datetime, json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def P(*a): return os.path.join(ROOT, *a)
def read(p):
    with open(p, encoding="utf-8", newline="") as f: return f.read()
def write(p, s):
    with open(p, "w", encoding="utf-8", newline="") as f: f.write(s)
def sub1(p, pattern, repl, what):
    s = read(p)
    n, c = re.subn(pattern, repl, s, count=1, flags=re.M)
    if c != 1: sys.exit("Could not find %s in %s" % (what, p))
    write(p, n)

def clean(b):
    return re.sub(r"\s*(Server deploy needed[^.]*\.|Plugin update needed\.?)\s*$", "", b.strip()).strip()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--server", required=True); ap.add_argument("--display", required=True)
    ap.add_argument("--plugin"); ap.add_argument("--whatsnew", action="store_true")
    ap.add_argument("--date", default=datetime.date.today().isoformat())
    a = ap.parse_args()
    for v in (a.server, a.display, a.plugin):
        if v and not re.fullmatch(r"\d+\.\d+\.\d+", v): sys.exit("Versions look like 2.8.0, not %r" % v)
    d = datetime.date.fromisoformat(a.date)

    sub1(P("server", "vnaaats_server.py"), r'^SERVER_VERSION = "[^"]*"', 'SERVER_VERSION = "%s"' % a.server, "SERVER_VERSION")
    sub1(P("web", "src", "config.json"), r'("version"\s*:\s*)"[^"]*"', r'\g<1>"%s"' % a.display, "version")

    cl = P("web", "CHANGELOG.md"); text = read(cl)
    m = re.search(r"^## Unreleased[ \t]*\n(.*?)(?=^## |\Z)", text, re.M | re.S)
    if not m: sys.exit("No '## Unreleased' section in web/CHANGELOG.md. Add the changes there first.")
    body = m.group(1)
    bullets = [clean(b[2:]) for b in body.splitlines() if b.startswith("- ")]
    if not bullets: sys.exit("The Unreleased section has no bullet lines.")
    write(cl, text.replace("## Unreleased", "## %s - %s" % (a.display, a.date), 1))

    if a.plugin:
        sub1(P("VatsimNAAATS", "Constants.h"), r'(const string PLUGIN_VERSION = )"[^"]*"', r'\g<1>"%s"' % a.plugin, "PLUGIN_VERSION")
        write(P("pluginversion.txt"), a.plugin)
        items = [re.sub(r"^Plugin:\s*", "", b) for b in bullets if b.startswith("Plugin:")]
        if not items: print("Note: no 'Plugin:' lines in the changelog, so the README note says only the version changed. Edit it.")
        note = " ".join("(%d) %s" % (i + 1, t) for i, t in enumerate(items)) if items else "Maintenance release."
        rd = P("VatsimNAAATS", "README.txt"); s = read(rd)
        nl = "\r\n" if "\r\n" in s else "\n"
        k = re.search(r"^NEW IN \d+\.\d+\.\d+", s, re.M)
        if not k: sys.exit("Could not find the previous NEW IN note in README.txt")
        text_note = "NEW IN %s: %s Replace only VatsimNAAATS.dll and keep your vnaaats.cfg." % (a.plugin, note)
        write(rd, s[:k.start()] + text_note + nl + nl + s[k.start():])

    if a.whatsnew:
        web_items = [b for b in bullets if not b.startswith("Plugin:")] or bullets
        out = []
        for b in web_items:
            title = re.split(r"[:.]", b, maxsplit=1)[0][:60].strip()
            out.append("      { title: %s, text: %s }," % (json.dumps(title), json.dumps(b)))
        entry = '  {\n    id: "%s",\n    date: "%d %s %d",\n    items: [\n%s\n    ],\n  },\n' % (a.display, d.day, d.strftime("%B"), d.year, "\n".join(out))
        wn = P("web", "src", "typescript", "fdd", "whatsnew.ts"); s = read(wn)
        i = s.index("export const NOTES: Note[] = [\n") + len("export const NOTES: Note[] = [\n")
        write(wn, s[:i] + entry + s[i:])
        print("Added a what's new entry. Trim the wording for controllers if needed.")
    print("Done. Review `git diff`, then commit and merge. Discord posts go out when master is pushed to server-and-squawk.")

main()
