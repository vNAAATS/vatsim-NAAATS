"""Real-time admin log: the plugin sends a copy of its log here; the admin reads it on /admin/log.

- Lines are kept in memory (last LINES_KEEP per controller) and appended to one file per day (kept DAYS_KEEP days).
- Secrets (keys, Hoppie logon codes) are removed here as well as in the plugin.
- Only someone holding VNAAATS_ADMIN_TOKEN can read. With no token set, the viewer is switched off (the plugin can still send)."""
import collections
import hmac
import json
import os
import re
import threading
import time
import urllib.request
from datetime import datetime, timezone

LINES_KEEP = 3000          # per controller, in memory
MAX_BATCH = 200            # lines per request
MAX_LINE = 600             # characters per line
RATE_PER_MIN = 600         # lines per controller per minute (more are dropped)
DAYS_KEEP = 7
DETAIL_SECONDS = 2 * 3600  # how long "send full detail" stays on for a controller

_SECRET = re.compile(r"(?i)\b(code|key|logon|password|token|passphrase)=[^&\s\"']*")
_ME = re.compile(r"[^A-Z0-9_\-]")
_CTRL = re.compile(r"[\x00-\x08\x0b-\x1f\x7f]")


def clean_line(text):
    text = _CTRL.sub(" ", str(text))[:MAX_LINE * 2]
    text = _SECRET.sub(lambda m: m.group(1) + "=***", text)
    return text[:MAX_LINE].rstrip()


def level_of(line):
    m = re.match(r"\[(INIT|LOG|WARN|ERR|EXC)\b", line)
    return m.group(1) if m else ""


class LogHub:
    def __init__(self, token="", directory=None):
        self.token = token if len(token) >= 16 else ""
        self.dir = directory
        self.lock = threading.Lock()
        self.buf = collections.defaultdict(lambda: collections.deque(maxlen=LINES_KEEP))
        self.info = {}              # me -> {"v", "last", "count", "errors", "ip"}
        self.detail_until = {}      # me -> unix time
        self.rate = {}              # me -> [window start, count]
        self.seq = 0
        self.fails = {}             # ip -> [window start, count]
        self.notifier = None
        if self.dir:
            try:
                os.makedirs(self.dir, exist_ok=True)
                self.prune()
            except OSError:
                self.dir = None

    # ---- plugin side
    def add(self, me, version, lines, ip="", prev=False):
        me = _ME.sub("", str(me).upper())[:16] or "UNKNOWN"
        now = time.time()
        out = []
        for raw in list(lines)[:MAX_BATCH]:
            line = clean_line(raw)
            if line:
                out.append(line)
        with self.lock:
            w = self.rate.get(me)
            if not w or now - w[0] >= 60:
                w = self.rate[me] = [now, 0]
            room = max(0, RATE_PER_MIN - w[1])
            out = out[:room]
            w[1] += len(out)
            inf = self.info.setdefault(me, {"v": "", "last": 0, "count": 0, "errors": 0, "ip": ""})
            inf.update(v=str(version)[:12], last=now, ip=str(ip)[:45])
            ts = datetime.fromtimestamp(now, timezone.utc).strftime("%H:%M:%S")
            for line in out:
                self.seq += 1
                lv = level_of(line)
                inf["count"] += 1
                if lv in ("ERR", "EXC"):
                    inf["errors"] += 1
                self.buf[me].append({"id": self.seq, "t": now, "ts": ts, "me": me, "lv": lv, "prev": bool(prev), "line": line})
            detail = self.detail_until.get(me, 0) > now
        self._write(me, now, out, prev)
        if self.notifier:
            self.notifier.note(me, version, out, prev)
        return {"ok": True, "n": len(out), "detail": detail}

    def _write(self, me, now, lines, prev):
        if not self.dir or not lines:
            return
        try:
            day = datetime.fromtimestamp(now, timezone.utc).strftime("%Y-%m-%d")
            with open(os.path.join(self.dir, "plugin-%s.log" % day), "a", encoding="utf-8") as f:
                for line in lines:
                    f.write("%s %s%s %s\n" % (datetime.fromtimestamp(now, timezone.utc).strftime("%H:%M:%S"), me, " (prev)" if prev else "", line))
        except OSError:
            pass

    def prune(self):
        cutoff = time.time() - DAYS_KEEP * 86400
        for name in os.listdir(self.dir):
            p = os.path.join(self.dir, name)
            if name.startswith("plugin-") and name.endswith(".log") and os.path.getmtime(p) < cutoff:
                try:
                    os.remove(p)
                except OSError:
                    pass

    # ---- admin side
    def token_ok(self, given, ip=""):
        if not self.token:
            return False
        now = time.time()
        with self.lock:
            f = self.fails.get(ip)
            if f and now - f[0] < 60 and f[1] >= 8:
                return False                       # too many wrong tries: wait a minute
        ok = hmac.compare_digest(str(given).encode(), self.token.encode())
        if not ok:
            with self.lock:
                f = self.fails.get(ip)
                if not f or now - f[0] >= 60:
                    f = self.fails[ip] = [now, 0]
                f[1] += 1
        return ok

    def set_detail(self, me, on):
        me = _ME.sub("", str(me).upper())[:16]
        with self.lock:
            if on:
                self.detail_until[me] = time.time() + DETAIL_SECONDS
            else:
                self.detail_until.pop(me, None)
        return {"me": me, "detail": bool(on)}

    def view(self, me="", level="", since=0, limit=400):
        me = _ME.sub("", str(me).upper())[:16]
        now = time.time()
        with self.lock:
            ctrl = [{"me": k, "v": v["v"], "ago": int(now - v["last"]), "count": v["count"], "errors": v["errors"], "ip": v["ip"],
                     "detail": self.detail_until.get(k, 0) > now} for k, v in sorted(self.info.items())]
            names = [me] if me else list(self.buf.keys())
            rows = [r for n in names for r in self.buf.get(n, ())
                    if r["id"] > since and (not level or (level == "problems" and r["lv"] in ("WARN", "ERR", "EXC")) or r["lv"] == level)]
        rows.sort(key=lambda r: r["id"])
        limit = max(1, min(int(limit), 1500))
        return {"controllers": ctrl, "lines": rows[-limit:], "last": self.seq}


def _normalise(line):
    """An error line without its time and numbers, so the same fault from many aircraft counts as one."""
    t = re.sub(r"^\[[A-Z]+ [0-9:.]+\]\s*", "", str(line))
    t = re.sub(r"\b[A-Z0-9]{2,7}\b", lambda m: "#" if re.search(r"\d", m.group(0)) else m.group(0), t)
    return re.sub(r"\d+", "#", t)[:160]


class Notifier:
    """Discord: a message with @here when plugins report errors (batched, repeats held back), and a plain summary every few hours.
    Sending happens on its own thread and never blocks the plugins; a failed post is dropped."""

    def __init__(self, webhook, post=None, summary_hours=6, cooldown=900, batch_seconds=60, ping_user=""):
        self.webhook = webhook
        # Who to ping on errors: a Discord user id (numeric) pings just that person; empty keeps @here.
        self.ping_user = re.sub(r"\D", "", ping_user or "")
        self.post = post or self._post
        self.summary_s = max(1, int(summary_hours * 3600))
        self.cooldown = cooldown
        self.batch_s = batch_seconds
        self.lock = threading.Lock()
        self.pending = []                       # (me, version, line) waiting for the next alert
        self.last_sent = {}                     # normalised error -> when it was last alerted
        self.suppressed = 0
        self.reset_stats()
        self.next_summary = (int(time.time()) // self.summary_s + 1) * self.summary_s

    def reset_stats(self):
        self.stats = {"since": time.time(), "controllers": set(), "versions": collections.Counter(), "lines": 0, "warn": 0, "err": 0,
                      "top": collections.Counter()}

    def note(self, me, version, lines, prev=False):
        """Called for every batch a plugin sends."""
        if prev:
            return
        with self.lock:
            st = self.stats
            if me not in st["controllers"]:
                st["controllers"].add(me)
                st["versions"][str(version) or "?"] += 1
            for line in lines:
                st["lines"] += 1
                lv = level_of(line)
                if lv == "WARN":
                    st["warn"] += 1
                elif lv in ("ERR", "EXC"):
                    st["err"] += 1
                    key = _normalise(line)
                    st["top"][key] += 1
                    if time.time() - self.last_sent.get(key, 0) >= self.cooldown:
                        self.last_sent[key] = time.time()
                        self.pending.append((me, str(version), line))
                    else:
                        self.suppressed += 1

    def _post(self, payload):
        req = urllib.request.Request(self.webhook, data=json.dumps(payload).encode(), headers={"Content-Type": "application/json", "User-Agent": "vnaaats-server"})
        urllib.request.urlopen(req, timeout=10).read()

    def ping_text(self):
        return "<@%s>" % self.ping_user if self.ping_user else "@here"

    def alert_text(self):
        with self.lock:
            items, self.pending = self.pending, []
            held, self.suppressed = self.suppressed, 0
        if not items:
            return None
        lines = ["%s **vNAAATS plugin errors** (%d new)" % (self.ping_text(), len(items))]
        for me, ver, line in items[:5]:
            lines.append("`%s` v%s: %s" % (me, ver, re.sub(r"^\[[A-Z]+ [0-9:.]+\]\s*", "", line)[:220].replace("`", "'")))
        if len(items) > 5:
            lines.append("...and %d more different errors" % (len(items) - 5))
        if held:
            lines.append("(%d repeats held back)" % held)
        allowed = {"users": [self.ping_user]} if self.ping_user else {"parse": ["everyone"]}
        return {"content": "\n".join(lines)[:1900], "allowed_mentions": allowed}

    def summary_text(self):
        with self.lock:
            st, self.stats = self.stats, None
        self.reset_stats()
        hours = max(1, round((time.time() - st["since"]) / 3600))
        lines = ["**vNAAATS log summary** (last %dh)" % hours,
                 "%d controllers reporting - versions: %s" % (len(st["controllers"]), ", ".join("%s x%d" % (v, n) for v, n in sorted(st["versions"].items())) or "none"),
                 "%d lines - %d warnings - %d errors" % (st["lines"], st["warn"], st["err"])]
        for key, n in st["top"].most_common(5):
            lines.append("%dx %s" % (n, key[:150].replace("`", "'")))
        if not st["top"]:
            lines.append("No errors reported.")
        return {"content": "\n".join(lines)[:1900]}

    def tick(self, now=None):
        """One pass of the loop (called every few seconds; tests call it directly)."""
        now = time.time() if now is None else now
        try:
            a = self.alert_text()
            if a:
                self.post(a)
            if now >= self.next_summary:
                self.next_summary = (int(now) // self.summary_s + 1) * self.summary_s
                self.post(self.summary_text())
        except Exception:
            pass

    def run(self):
        try:
            self.post({"content": "vNAAATS admin alerts are on. Errors ping %s (grouped, repeats held back); a plain summary comes every %g hours." % (self.ping_text(), self.summary_s / 3600.0)})
        except Exception:
            pass
        while True:
            time.sleep(self.batch_s)
            self.tick()


PAGE = """<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>vNAAATS admin log</title><meta name="robots" content="noindex">
<style>
:root{color-scheme:dark}body{margin:0;background:#101418;color:#d8dee4;font:13px/1.45 Consolas,Menlo,monospace}
header{position:sticky;top:0;background:#171c22;border-bottom:1px solid #2a323b;padding:8px 12px;display:flex;gap:8px;flex-wrap:wrap;align-items:center}
select,button,input{background:#222a33;color:#d8dee4;border:1px solid #34404c;border-radius:4px;padding:5px 8px;font:inherit}
button{cursor:pointer}button:hover{background:#2b3641}#login{max-width:320px;margin:20vh auto;display:flex;gap:8px}
#log{padding:8px 12px;white-space:pre-wrap;word-break:break-word}.r{padding:1px 0}.WARN{color:#e5c07b}.ERR,.EXC{color:#ff7b72}.INIT{color:#7ee0ff}.prev{opacity:.7;border-left:3px solid #6e7681;padding-left:6px}
.me{color:#79c0ff}.t{color:#6e7681}#st{color:#8b949e}.pill{border:1px solid #34404c;border-radius:10px;padding:1px 8px;cursor:pointer}.pill.on{background:#1f6feb;border-color:#1f6feb;color:#fff}.bad{color:#ff7b72}
</style></head><body>
<div id="login"><input id="tok" type="password" placeholder="Admin token" autocomplete="current-password"><button id="go">Open</button></div>
<div id="app" hidden>
<header><select id="who"><option value="">All controllers</option></select>
<select id="lvl"><option value="">All lines</option><option value="problems" selected>Warnings and errors</option><option value="ERR">Errors only</option></select>
<button id="pause">Pause</button><button id="clear">Clear screen</button><button id="det">Full detail: off</button><button id="dl">Download</button>
<span id="st"></span></header><div id="ctl" style="padding:6px 12px"></div><div id="log"></div></div>
<script>
const $=id=>document.getElementById(id);let tok="";try{tok=sessionStorage.getItem("vt")||""}catch(e){}
let last=0,paused=false,rows=[],ctrls=[];
async function api(path,body){const o={headers:{"X-Admin-Token":tok}};if(body){o.method="POST";o.headers["Content-Type"]="application/json";o.body=JSON.stringify(body)}
const r=await fetch(path,o);if(r.status==403){throw new Error("Wrong token")}if(!r.ok)throw new Error("Error "+r.status);return r.json()}
function esc(s){return s.replace(/[&<>]/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;"}[c]))}
function draw(){const lv=$("lvl").value,w=$("who").value;
 $("log").innerHTML=rows.filter(r=>(!w||r.me==w)&&(!lv||(lv=="problems"?["WARN","ERR","EXC"].includes(r.lv):r.lv==lv||(lv=="ERR"&&r.lv=="EXC")))).map(r=>`<div class="r ${r.lv} ${r.prev?"prev":""}"><span class="t">${r.ts}</span> <span class="me">${r.me}</span> ${esc(r.line)}</div>`).join("");
 if(!paused)window.scrollTo(0,document.body.scrollHeight)}
function drawCtl(){const s=$("who"),cur=s.value;s.innerHTML='<option value="">All controllers</option>'+ctrls.map(c=>`<option>${c.me}</option>`).join("");s.value=cur;
 $("ctl").innerHTML=ctrls.map(c=>`<span class="pill ${c.detail?"on":""}" data-me="${c.me}">${c.me} v${c.v} · ${c.ago<90?"live":Math.round(c.ago/60)+" min ago"} · ${c.errors} err${c.detail?" · full detail":""}</span> `).join("")||"<span id=st>No controller has sent a log yet.</span>";
 const c=ctrls.find(x=>x.me==s.value);$("det").textContent=c?"Full detail: "+(c.detail?"on":"off"):"Full detail: pick a controller"}
async function tick(){if(!tok)return;try{const d=await api("/admin/log/data?since="+last+"&limit=1500");ctrls=d.controllers;last=d.last;
 if(d.lines.length){rows=rows.concat(d.lines).slice(-4000)}drawCtl();draw();$("st").textContent="updated "+new Date().toLocaleTimeString()}catch(e){$("st").innerHTML='<span class=bad>'+e.message+'</span>';if(e.message=="Wrong token"){tok="";$("app").hidden=true;$("login").hidden=false}}}
async function open_(){tok=$("tok").value.trim()||tok;try{await api("/admin/log/data?limit=1");try{sessionStorage.setItem("vt",tok)}catch(e){}$("login").hidden=true;$("app").hidden=false;last=0;rows=[];tick()}catch(e){alert(e.message)}}
$("go").onclick=open_;$("tok").onkeydown=e=>{if(e.key=="Enter")open_()};
$("who").onchange=()=>{drawCtl();draw()};$("lvl").onchange=draw;
$("pause").onclick=()=>{paused=!paused;$("pause").textContent=paused?"Resume":"Pause"};$("clear").onclick=()=>{rows=[];draw()};
$("det").onclick=async()=>{const m=$("who").value;if(!m){alert("Pick a controller first");return}const c=ctrls.find(x=>x.me==m);await api("/admin/log/detail",{me:m,on:!(c&&c.detail)});tick()};
$("ctl").onclick=e=>{const m=e.target.dataset&&e.target.dataset.me;if(m){$("who").value=m;drawCtl();draw()}};
$("dl").onclick=()=>{const t=rows.map(r=>r.ts+" "+r.me+" "+r.line).join("\\n");const a=document.createElement("a");a.href=URL.createObjectURL(new Blob([t],{type:"text/plain"}));a.download="vnaaats-log.txt";a.click()};
if(tok){$("tok").value=tok;open_()}setInterval(tick,3000);
</script></body></html>"""


FDD_PAGE = """<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>vNAAATS admin: open the FDD</title>
<style>body{font:15px system-ui,sans-serif;background:#10161d;color:#dde;display:flex;min-height:100vh;align-items:center;justify-content:center;margin:0}
form{background:#18222d;padding:24px;border-radius:8px;width:320px}h1{font-size:17px;margin:0 0 6px}p{color:#9ab;font-size:13px;margin:0 0 14px}
input,button{width:100%;box-sizing:border-box;padding:9px;border-radius:5px;border:1px solid #345;background:#0d1319;color:#dde;font-size:14px}
button{margin-top:10px;background:#2a6fb0;border:0;cursor:pointer}#e{color:#f88;margin-top:8px;min-height:1em}</style></head><body>
<form id="f"><h1>Open the FDD (admin, view only)</h1><p>Shows the live display without a VATSIM connection. Nothing can be edited or sent from it.</p>
<input id="t" type="password" placeholder="Admin token" autocomplete="current-password" autofocus><button>Open</button><div id="e"></div></form>
<script>document.getElementById("f").onsubmit=async function(ev){ev.preventDefault();var e=document.getElementById("e");e.textContent="";
try{var r=await fetch("/admin/fdd/session",{method:"POST",headers:{"Content-Type":"application/json","X-Admin-Token":document.getElementById("t").value},body:"{}",credentials:"same-origin"});
if(r.ok)location.href="/fdd";else e.textContent=r.status==403?"Wrong token (or too many tries: wait a minute).":"Failed ("+r.status+")";}catch(x){e.textContent="Cannot reach the server."}};</script></body></html>
"""
