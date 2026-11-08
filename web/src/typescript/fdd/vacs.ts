/*
 *  VACS call alert. VACS (vacs.network) has a remote-control WebSocket on this computer (default ws://localhost:9600/ws, switched off
 *  until the controller turns it on in VACS). We listen for call events, keep a short call log, and on the controller's click accept a ringing
 *  call, end a call or place a call to an online position. Nothing is sent to our server.
 *  The VACS API has no stability promise, so everything here fails quietly: on any problem the indicator simply goes grey.
 */
import { writable, get } from "svelte/store";
import * as app from "./app";

export type Party = { cid: string; position: string; callsign: string };
export type LogEntry = {
  callId: string; dir: "in" | "out"; who: string; cid: string; callsign: string; prio: boolean;
  at: number;                                // when it started ringing / when we placed it (ms)
  answeredAt: number;                        // 0 = not answered
  endedAt: number;                           // 0 = still going
  result: "ringing" | "answered" | "missed" | "no answer" | "ended" | "declined";
};
export type Online = { cid: string; callsign: string; position: string; freq: string };
export type VacsState = {
  enabled: boolean;
  link: "off" | "connecting" | "up";        // our WebSocket to VACS
  onPosition: boolean;                       // VACS itself is connected to a position
  ringing: { callId: string; from: string; prio: boolean; cid: string; callsign: string; at: number } | null;
  call: { callId: string; with: string; since: number } | null;
  dismissed: string;                         // callId whose pop-up was dismissed
  error: string;
  log: LogEntry[];                           // newest first, kept in this browser only
  online: Online[];                          // everyone VACS says is online, for the Call list
  callError: string;                         // why the last call-out failed, if it did
  busy: boolean;                             // Busy: new calls are declined automatically
  busyPrio: boolean;                         // ...except priority calls, which still ring
};

const KEY = "vnaaats.vacs";
const PORT = "vnaaats.vacsPort";
function lsGet(k: string): string | null { try { return localStorage.getItem(k); } catch (e) { return null; } }
function lsSet(k: string, v: string): void { try { localStorage.setItem(k, v); } catch (e) { /* ignore */ } }

export function isEnabled(): boolean { const v = lsGet(KEY); return v === null ? app.inApp() : v === "1"; }
export function getPort(): number { const n = parseInt(lsGet(PORT) ?? "", 10); return n > 0 && n < 65536 ? n : 9600; }

const LOGKEY = "vnaaats.vacsLog";
const BUSYKEY = "vnaaats.vacsBusy";
const BUSYPRIOKEY = "vnaaats.vacsBusyPrio";
function loadLog(): LogEntry[] {
  try {
    const a = JSON.parse(lsGet(LOGKEY) ?? "[]");
    if (!Array.isArray(a)) return [];
    // a call still "ringing" or going when the page was closed cannot be known any more
    return a.slice(0, 30).map((e: LogEntry) => (e.result === "ringing" ? { ...e, result: "missed" as const, endedAt: e.endedAt || e.at } : e.endedAt ? e : { ...e, endedAt: e.answeredAt || e.at, result: "ended" as const }));
  } catch (e) { return []; }
}
function saveLog(l: LogEntry[]): void { lsSet(LOGKEY, JSON.stringify(l.slice(0, 30))); }

const MUTEKEY = "vnaaats.vacsMute";
export const ringMuted = writable<boolean>(lsGet(MUTEKEY) === "1");
export function setRingMuted(m: boolean): void { ringMuted.set(m); lsSet(MUTEKEY, m ? "1" : "0"); }
let actx: AudioContext | null = null;
/** Browsers only allow sound after the page has been clicked once; this makes sure the audio context is ready by then. */
function audio(): AudioContext | null {
  try {
    if (!actx) actx = new (window.AudioContext || (window as any).webkitAudioContext)();
    if (actx.state === "suspended") void actx.resume();
    return actx;
  } catch (e) { return null; }
}
try { window.addEventListener("pointerdown", () => { audio(); }, { once: true }); } catch (e) { /* no window */ }
/** One telephone "ring ring": two bursts of a 440 + 480 Hz warble, played once per incoming call. */
export function ringOnce(): void {
  if (get(ringMuted)) return;
  const c = audio();
  if (!c || c.state !== "running") return;
  const t0 = c.currentTime + 0.02;
  const out = c.createGain(); out.gain.value = 0.18; out.connect(c.destination);
  for (const start of [0, 0.6]) {
    const g = c.createGain(); g.connect(out);
    g.gain.setValueAtTime(0, t0 + start);
    g.gain.linearRampToValueAtTime(1, t0 + start + 0.02);
    g.gain.setValueAtTime(1, t0 + start + 0.38);
    g.gain.linearRampToValueAtTime(0, t0 + start + 0.4);
    for (const f of [440, 480]) {
      const o = c.createOscillator(); o.type = "sine"; o.frequency.value = f; o.connect(g);
      const lfo = c.createOscillator(); lfo.frequency.value = 20; const d = c.createGain(); d.gain.value = 15; lfo.connect(d); d.connect(o.frequency);
      o.start(t0 + start); o.stop(t0 + start + 0.42); lfo.start(t0 + start); lfo.stop(t0 + start + 0.42);
    }
  }
}
export const vacs = writable<VacsState>({ enabled: false, link: "off", onPosition: false, ringing: null, call: null, dismissed: "", error: "", log: loadLog(), online: [], callError: "", busy: lsGet(BUSYKEY) === "1", busyPrio: lsGet(BUSYPRIOKEY) !== "0" });
const upd = (f: (s: VacsState) => void) => vacs.update((s) => { f(s); return { ...s }; });

let ws: WebSocket | null = null;
let timer: any = null;
let ping: any = null;
let wanted = false;
let delay = 5000;
let nextId = 1;
let me = "";                                 // our own VATSIM CID, to tell "ringing for me" from calls we placed
let mySource: any = null;                    // the "source" we place calls with (from VACS' own default call source)
const calls = new Map<string, string>();     // callId -> label of the other party
const clients = new Map<string, Online>();   // CID -> who VACS says is online

function who(cid: string, position: string): Party {
  const c = clients.get(cid);
  return { cid, position: position || c?.position || "", callsign: c?.callsign || "" };
}
function logStart(e: LogEntry): void {
  upd((s) => { s.log = [e, ...s.log.filter((x) => x.callId !== e.callId)].slice(0, 30); saveLog(s.log); });
}
function logChange(callId: string, f: (e: LogEntry) => void): void {
  upd((s) => { const e = s.log.find((x) => x.callId === callId); if (e) { f(e); saveLog(s.log); } });
}
function setOnline(): void { upd((s) => { s.online = [...clients.values()].filter((c) => c.cid !== me).sort((a, b) => (a.callsign || a.position).localeCompare(b.callsign || b.position)); }); }
function putClient(c: any): void {
  if (!c?.id) return;
  clients.set(String(c.id), { cid: String(c.id), callsign: String(c.displayName ?? ""), position: String(c.positionId ?? ""), freq: String(c.frequency ?? "") });
}
function endLog(callId: string, now: number): void {
  logChange(callId, (e) => {
    if (e.endedAt) return;
    e.endedAt = now;
    e.result = e.answeredAt ? "ended" : e.dir === "in" ? "missed" : "no answer";
  });
}

const EVENTS = ["signaling:call-invitation", "signaling:call-update", "signaling:call-end", "signaling:force-call-end", "signaling:accept-incoming-call",
  "signaling:outgoing-call", "signaling:call-reject", "signaling:connected", "signaling:disconnected", "webrtc:call-connected", "webrtc:call-disconnected", "auth:authenticated",
  "signaling:client-list", "signaling:client-connected", "signaling:client-disconnected"];

function label(src: any): string {
  return (src?.positionId || src?.stationId || src?.clientId || "unknown").toString();
}
function targetLabel(t: any): string {
  if (!t) return "";
  return (t.station || t.position || t.client || "").toString();
}

function handle(name: string, p: any): void {
  switch (name) {
    case "signaling:call-invitation": {
      if (!p?.callId) return;
      const from = label(p.source);
      const w = who(String(p.source?.clientId ?? ""), String(p.source?.positionId ?? p.source?.stationId ?? ""));
      calls.set(p.callId, from);
      const now = Date.now();
      const st = get(vacs);
      if (st.busy && !(p.prio && st.busyPrio) && !st.call) {
        // Busy: turn the call away without ringing; it is logged as declined
        send({ type: "invoke", id: "n" + nextId++, cmd: "signaling_end_call", args: { callId: p.callId } });
        logStart({ callId: p.callId, dir: "in", who: from, cid: w.cid, callsign: w.callsign, prio: !!p.prio, at: now, answeredAt: 0, endedAt: now, result: "declined" });
        break;
      }
      upd((s) => { s.ringing = { callId: p.callId, from, prio: !!p.prio, cid: w.cid, callsign: w.callsign, at: now }; });
      logStart({ callId: p.callId, dir: "in", who: from, cid: w.cid, callsign: w.callsign, prio: !!p.prio, at: now, answeredAt: 0, endedAt: 0, result: "ringing" });
      ringOnce();
      break;
    }
    case "signaling:accept-incoming-call": {
      const now = Date.now();
      upd((s) => { if (s.ringing && s.ringing.callId === p) { s.call = { callId: p, with: s.ringing.from, since: now }; s.ringing = null; } });
      logChange(String(p), (e) => { if (!e.answeredAt) { e.answeredAt = now; e.result = "answered"; } });
      break;
    }
    case "signaling:outgoing-call": {
      if (!p?.callId) return;
      const to = (p.targets ?? []).map(targetLabel).filter((x: string) => x).join(", ");
      calls.set(p.callId, to);
      const now = Date.now();
      upd((s) => { s.call = { callId: p.callId, with: to, since: 0 }; s.callError = ""; });
      const t0 = (p.targets ?? [])[0];
      const cid = String(t0?.client ?? "");
      logStart({ callId: p.callId, dir: "out", who: to, cid, callsign: clients.get(cid)?.callsign ?? "", prio: !!p.prio, at: now, answeredAt: 0, endedAt: 0, result: "ringing" });
      break;
    }
    case "webrtc:call-connected": {
      const now = Date.now();
      upd((s) => {
        if (s.ringing && s.ringing.callId === p?.callId) { s.call = { callId: p.callId, with: s.ringing.from, since: now }; s.ringing = null; }
        else if (p?.callId && (!s.call || s.call.callId === p.callId)) s.call = { callId: p.callId, with: s.call?.with || (calls.get(p.callId) ?? ""), since: now };
      });
      if (p?.callId) logChange(String(p.callId), (e) => { if (!e.answeredAt) { e.answeredAt = now; e.result = "answered"; } });
      break;
    }
    case "signaling:call-reject":
      upd((s) => { s.callError = "The other controller rejected the call."; });
      if (p?.callId) endLog(String(p.callId), Date.now());
      break;
    case "signaling:call-end":
    case "signaling:force-call-end":
      calls.delete(p);
      upd((s) => { if (s.ringing?.callId === p) s.ringing = null; if (s.call?.callId === p) s.call = null; });
      endLog(String(p), Date.now());
      break;
    case "signaling:client-list":
      clients.clear(); for (const c of Array.isArray(p) ? p : []) putClient(c); setOnline(); break;
    case "signaling:client-connected": putClient(p); setOnline(); break;
    case "signaling:client-disconnected": clients.delete(String(p)); setOnline(); break;
    case "webrtc:call-disconnected":
      break;                                  // one leg of a call; the call itself ends with signaling:call-end
    case "signaling:connected": upd((s) => { s.onPosition = true; }); break;
    case "signaling:disconnected": upd((s) => { s.onPosition = false; s.ringing = null; s.call = null; }); break;
    case "auth:authenticated": me = String(p ?? ""); setOnline(); break;
  }
}

function send(o: any): void { try { if (ws && ws.readyState === 1) ws.send(JSON.stringify(o)); } catch (e) { /* ignore */ } }

function open(): void {
  if (!wanted) return;
  upd((s) => { s.link = "connecting"; });
  let sock: WebSocket;
  try { sock = new WebSocket("ws://127.0.0.1:" + getPort() + "/ws"); } catch (e) { upd((s) => { s.link = "off"; s.error = "blocked"; }); again(); return; }
  ws = sock;
  sock.onopen = () => {
    delay = 5000;
    upd((s) => { s.link = "up"; s.error = ""; });
    for (const e of EVENTS) send({ type: "subscribe", event: e });
    send({ type: "invoke", id: "ready", cmd: "app_frontend_ready", args: {} });
    send({ type: "invoke", id: "state", cmd: "remote_get_session_state", args: {} });
    send({ type: "invoke", id: "sync", cmd: "remote_request_store_sync", args: {} });
    clearInterval(ping);
    ping = setInterval(() => send({ type: "ping" }), 20000);
  };
  sock.onmessage = (m) => {
    let d: any;
    try { d = JSON.parse(m.data as string); } catch (e) { return; }
    if (d.type === "event") handle(d.name, d.payload);
    else if (d.type === "response" && d.id === "state" && d.ok) {
      upd((s) => { s.onPosition = d.data?.connectionState === "connected"; });
      if (d.data?.clientId) me = String(d.data.clientId);
      for (const c of Array.isArray(d.data?.clients) ? d.data.clients : []) putClient(c);
      const ds = d.data?.defaultCallSources;
      const first = Array.isArray(ds) ? ds[0] : ds && typeof ds === "object" ? Object.values(ds)[0] : null;
      mySource = first && typeof first === "object" && (first as any).clientId ? first : null;
      setOnline();
    }
    else if (d.type === "response" && typeof d.id === "string" && d.id.startsWith("dial")) {
      if (!d.ok) upd((s) => { s.callError = "VACS did not place the call" + (d.error ? ": " + (typeof d.error === "string" ? d.error : d.error.message ?? "") : "."); });
    }
  };
  sock.onclose = () => {
    clearInterval(ping); if (ws === sock) ws = null;
    const open = get(vacs).log.filter((e) => !e.endedAt).map((e) => e.callId);
    clients.clear();
    upd((s) => { s.link = "off"; s.onPosition = false; s.ringing = null; s.call = null; s.online = []; });
    for (const id of open) endLog(id, Date.now());
    again();
  };
  sock.onerror = () => { /* onclose follows */ };
}

function again(): void {
  if (!wanted) return;
  clearTimeout(timer);
  timer = setTimeout(open, delay);
  delay = Math.min(delay * 2, 30000);
}

export function start(): void {
  wanted = isEnabled();
  upd((s) => { s.enabled = wanted; });
  if (!wanted || ws) return;
  open();
}
export function stop(): void {
  wanted = false;
  clearTimeout(timer); clearInterval(ping);
  try { ws?.close(); } catch (e) { /* ignore */ }
  ws = null;
  clients.clear();
  upd((s) => { s.link = "off"; s.onPosition = false; s.ringing = null; s.call = null; s.online = []; });
}
export function configure(enabled: boolean, port: number): void {
  lsSet(KEY, enabled ? "1" : "0");
  lsSet(PORT, String(port));
  stop();
  delay = 5000;
  start();
}
/** Rings an online controller from VACS. `cid` is their VATSIM CID as shown in the Call list; prio makes it a priority call. */
export function dial(cid: string, prio: boolean = false): void {
  const st = get(vacs);
  if (st.link !== "up" || !st.onPosition) { upd((s) => { s.callError = "VACS is not connected to a position."; }); return; }
  if (st.call || st.ringing) { upd((s) => { s.callError = "Finish the current call first."; }); return; }
  const mine = clients.get(me);
  const source = mySource ?? { clientId: me, ...(mine?.position ? { positionId: mine.position, stationId: mine.position } : {}) };
  upd((s) => { s.callError = ""; });
  send({ type: "invoke", id: "dial" + nextId++, cmd: "signaling_invite_to_call", args: { targets: [{ client: cid }], source, prio } });
}
export function hangup(): void {
  const c = get(vacs).call;
  if (c) send({ type: "invoke", id: "n" + nextId++, cmd: "signaling_end_call", args: { callId: c.callId } });
}
/** Turns a ringing call away. VACS has no "reject" command, so this asks it to end the call; if VACS ignores that the call keeps ringing there. */
export function decline(): void {
  const r = get(vacs).ringing;
  if (!r) return;
  send({ type: "invoke", id: "n" + nextId++, cmd: "signaling_end_call", args: { callId: r.callId } });
  upd((s) => { if (s.ringing?.callId === r.callId) s.ringing = null; });
  logChange(r.callId, (e) => { if (!e.endedAt) { e.endedAt = Date.now(); e.result = "declined"; } });
}
export function setBusy(on: boolean, prioThrough: boolean = get(vacs).busyPrio): void {
  lsSet(BUSYKEY, on ? "1" : "0"); lsSet(BUSYPRIOKEY, prioThrough ? "1" : "0");
  upd((s) => { s.busy = on; s.busyPrio = prioThrough; });
}
export function clearLog(): void { upd((s) => { s.log = []; saveLog([]); }); }
export function accept(): void {
  const r = get(vacs).ringing;
  if (r) send({ type: "invoke", id: "n" + nextId++, cmd: "signaling_accept_call", args: { callId: r.callId } });
}
export function dismiss(): void { upd((s) => { s.dismissed = s.ringing?.callId ?? ""; }); }
