/*
 *  TrackAudio: who spoke last on your frequencies. Talks to the TrackAudio program on THIS computer only (ws://127.0.0.1:49080/ws).
 *  Only callsign, frequency, time and length are kept (in this browser, on this computer). No audio is recorded or sent anywhere.
 */
import { writable, get } from "svelte/store";

export type Talk = { who: string; mine: boolean; freq: string; ts: number; secs: number };
export type TaState = { enabled: boolean; link: "off" | "up"; rx: { who: string; freq: string; since: number } | null; tx: boolean; history: Talk[] };

const EN_KEY = "vnaaats.trackaudio";
const HIST_KEY = "vnaaats.taHistory";
const MAX = 200;
const KEEP_SECONDS = 24 * 3600;
const PORT = 49080;

function loadHist(): Talk[] {
  try {
    const a = JSON.parse(localStorage.getItem(HIST_KEY) ?? "[]");
    const cut = Date.now() / 1000 - KEEP_SECONDS;
    return Array.isArray(a) ? a.filter((t: Talk) => t && t.ts > cut).slice(0, MAX) : [];
  } catch (e) { return []; }
}
function saveHist(h: Talk[]): void { try { localStorage.setItem(HIST_KEY, JSON.stringify(h.slice(0, MAX))); } catch (e) { /* not kept */ } }

export function isEnabled(): boolean { try { return localStorage.getItem(EN_KEY) === "1"; } catch (e) { return false; } }
// Highlight the aircraft that is transmitting in the flight list. On unless switched off (the choice is kept on this computer).
const HL_KEY = "vnaaats.taHighlight";
function loadHl(): boolean { try { return localStorage.getItem(HL_KEY) !== "0"; } catch (e) { return true; } }
export const highlightOn = writable<boolean>(loadHl());
export function setHighlight(on: boolean): void {
  try { localStorage.setItem(HL_KEY, on ? "1" : "0"); } catch (e) { /* not kept */ }
  highlightOn.set(on);
}
try { window.addEventListener("storage", (e) => { if (e.key === HL_KEY) highlightOn.set(loadHl()); }); } catch (e) { /* no window */ }
// Scroll the flight list to the aircraft that starts transmitting (only if its row is not already on screen). On unless switched off.
const SC_KEY = "vnaaats.taScroll";
function loadSc(): boolean { try { return localStorage.getItem(SC_KEY) !== "0"; } catch (e) { return true; } }
export const scrollOn = writable<boolean>(loadSc());
export function setScroll(on: boolean): void {
  try { localStorage.setItem(SC_KEY, on ? "1" : "0"); } catch (e) { /* not kept */ }
  scrollOn.set(on);
}
try { window.addEventListener("storage", (e) => { if (e.key === SC_KEY) scrollOn.set(loadSc()); }); } catch (e) { /* no window */ }
// Who is on the air right now (live) or just was (live = false, so the box can fade out)
export const talker = writable<{ who: string; live: boolean }>({ who: "", live: false });

export const ta = writable<TaState>({ enabled: false, link: "off", rx: null, tx: false, history: [] });

let ws: WebSocket | null = null;
let retry: any = null;
let running = false;
let txSince = 0;

function upd(f: (s: TaState) => void): void { ta.update((s) => { f(s); return { ...s }; }); }
function mhz(hz: any): string { const n = Number(hz); return n > 1000 ? (n / 1e6).toFixed(3) : ""; }

function addTalk(t: Talk): void {
  upd((s) => { s.history = [t, ...s.history].slice(0, MAX); saveHist(s.history); });
}

export function handle(raw: string): void {
  let m: any;
  try { m = JSON.parse(raw); } catch (e) { return; }
  const type = String(m?.type ?? "");
  const v = m?.value ?? m ?? {};
  const now = Date.now() / 1000;
  if (type === "kRxBegin") {
    upd((s) => { s.rx = { who: String(v.callsign ?? "?").toUpperCase(), freq: mhz(v.pFrequencyHz), since: now }; });
    talker.set({ who: String(v.callsign ?? "").toUpperCase(), live: true });
  } else if (type === "kRxEnd") {
    const cur = get(ta).rx;
    const who = String(v.callsign ?? cur?.who ?? "?").toUpperCase();
    const freq = mhz(v.pFrequencyHz) || cur?.freq || "";
    const since = cur && cur.who === who ? cur.since : now;
    upd((s) => { s.rx = null; });
    talker.set({ who, live: false });
    addTalk({ who, mine: false, freq, ts: since, secs: Math.max(1, Math.round(now - since)) });
  } else if (type === "kTxBegin") {
    txSince = now;
    upd((s) => { s.tx = true; });
  } else if (type === "kTxEnd") {
    upd((s) => { s.tx = false; });
    if (txSince) addTalk({ who: "You", mine: true, freq: "", ts: txSince, secs: Math.max(1, Math.round(now - txSince)) });
    txSince = 0;
  }
}

function connect(): void {
  if (!running) return;
  try {
    ws = new WebSocket("ws://127.0.0.1:" + PORT + "/ws");
  } catch (e) { schedule(); return; }
  ws.onopen = () => upd((s) => { s.link = "up"; });
  ws.onmessage = (e) => { if (typeof e.data === "string") handle(e.data); };
  ws.onclose = () => { upd((s) => { s.link = "off"; s.rx = null; s.tx = false; }); talker.update((t) => ({ who: t.who, live: false })); schedule(); };
  ws.onerror = () => { try { ws?.close(); } catch (e) { /* closing */ } };
}
function schedule(): void { if (running) { clearTimeout(retry); retry = setTimeout(connect, 5000); } }

export function start(): void {
  upd((s) => { s.enabled = isEnabled(); s.history = loadHist(); });
  if (!isEnabled() || running) return;
  running = true;
  connect();
}
export function stop(): void {
  running = false;
  clearTimeout(retry);
  try { ws?.close(); } catch (e) { /* closing */ }
  ws = null;
  upd((s) => { s.link = "off"; s.rx = null; s.tx = false; });
}
export function setEnabled(on: boolean): void {
  try { localStorage.setItem(EN_KEY, on ? "1" : "0"); } catch (e) { /* not kept */ }
  stop();
  upd((s) => { s.enabled = on; });
  if (on) start();
}
export function clearHistory(): void { upd((s) => { s.history = []; }); saveHist([]); }

/** A controller callsign (NAT_FSS, EGGX_CTR, ...). What a controller transmits is not "received" by you, so it is never the last RX; it still shows in the history. */
export function isAtc(who: string): boolean { return /_(CTR|FSS|TWR|APP|DEP|GND|DEL|ATIS|RMP|OBS|SUP|INS)$/.test(String(who ?? "").toUpperCase()); }
/** The last call received from a pilot. */
export function lastRx(s: TaState): Talk | null { return s.history.find((t) => !t.mine && !isAtc(t.who)) ?? null; }
export function hhmmss(ts: number): string {
  const d = new Date(ts * 1000);
  return [d.getUTCHours(), d.getUTCMinutes(), d.getUTCSeconds()].map((n) => String(n).padStart(2, "0")).join(":") + "Z";
}
