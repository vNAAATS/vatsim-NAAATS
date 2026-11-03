/*
 *  CPDLC data for the FDD: which aircraft are connected, unread messages, the panel's open/closed state,
 *  and sending uplinks through the server (which talks to the Hoppie network).
 */
import { writable, get } from "svelte/store";
import * as dataHandler from "./data";

const apiBase: string = (import.meta as any).env?.VITE_API_BASE ?? "";

export type RclRequest = {
  id: number;
  ts: number;
  callsign: string;
  text: string;                    // what the pilot sent
  dest: string;
  route: string;
  etas: string;
  level: number | null;
  mach: number | null;
  track: string;
  natTrak: { status: string; level: string | null; mach: string | null; nat: string | null; fix: string | null; issued: number | null } | null;
};

export type CpdlcStatus = {
  enabled: boolean;
  station?: string;
  active?: boolean;               // this browser's controller is signed in to a CPDLC station
  stationForPosition?: string;    // the station that belongs to the chosen position
  inUse?: boolean;                // that station is being used by someone else
  lockedSeconds?: number;         // after a forced log-out: seconds before anyone may sign in to the station
  kickedFrom?: string;            // this browser was force-logged-out of that station
  autoAccept?: boolean;
  connected: { callsign: string; since: number; unread: number }[];
  unread: Record<string, number>;
  lastPoll?: number | null;
  error?: string;
  rclRequests?: RclRequest[];      // oceanic clearance requests that arrived by CPDLC and are still open
};

export type CpdlcMessage = {
  id: number;
  ts: number;
  dir: "UP" | "DOWN";
  callsign: string;
  min: number | null;
  mrn: number | null;
  ra: string;
  text: string;
  readable: string;
  state: string;
  seen: boolean;
  by: string;
};

export const cpdlcStatus = writable<CpdlcStatus>({ enabled: false, connected: [], unread: {} });
export const cpdlcOpen = writable<boolean>(false);
export const cpdlcPick = writable<string>("");     // callsign chosen in the panel
export type Freetext = { ra: string; text: string };
export const cpdlcFreetexts = writable<Freetext[]>([]);   // standard messages from the server (TopSky list)

let cpdlcAudio: HTMLAudioElement | null = null;
// The sound for a message from a pilot (static/cpdlc.wav). Respects the FDD sound switch.
export function playCpdlcSound(): void {
  if (!dataHandler.soundOn) return;
  try {
    cpdlcAudio = cpdlcAudio ?? new Audio("/cpdlc.wav");
    cpdlcAudio.currentTime = 0;
    const p = cpdlcAudio.play();
    if (p && p.catch) p.catch(() => dataHandler.playPing());   // autoplay blocked: fall back to the beep
  } catch (e) { /* no audio */ }
}

export async function loadFreetexts(): Promise<void> {
  try {
    const r = await fetch(apiBase + "/cpdlc/templates", { credentials: "same-origin" });
    if (r.ok) cpdlcFreetexts.set(await r.json());
  } catch (e) { /* keep the built-in buttons */ }
}

export async function sendUnicom(callsign: string): Promise<string> {
  const r = await post("/cpdlc/unicom", { callsign });
  if (r.ok) { refreshCpdlc(); return ""; }
  return r.data.error ?? "UNICOM message failed";
}

// Aircraft that have just logged on to CPDLC and have not been looked at yet: their button flashes and a sound plays once.
export const cpdlcAttention = writable<Set<string>>(new Set());
let knownConnected: Set<string> | null = null;      // null until the first answer, so aircraft already connected do not ping on page load
export function clearAttention(callsign: string): void {
  cpdlcAttention.update((a) => { if (!a.has(callsign)) return a; const n = new Set(a); n.delete(callsign); return n; });
}

let prevUnread: number = -1;     // -1 until the first answer, so old messages do not ping on page load
let timer: any = null;

export function unreadTotal(s: CpdlcStatus): number {
  return Object.values(s.unread ?? {}).reduce((a, b) => a + b, 0);
}

export async function refreshCpdlc(): Promise<void> {
  try {
    const r = await fetch(apiBase + "/cpdlc/status", { credentials: "same-origin" });
    if (!r.ok) return;
    const s: CpdlcStatus = await r.json();
    cpdlcStatus.set(s);
    const total = unreadTotal(s);
    if (prevUnread >= 0 && total > prevUnread) playCpdlcSound();
    prevUnread = total;
    // A pilot just logging on does NOT flash or ping: only a request or a reply (an unread message) does.
    const nowConnected = new Set(s.connected.map((c) => c.callsign));
    cpdlcAttention.update((a) => { const n = new Set([...a].filter((c) => nowConnected.has(c))); return n.size === a.size ? a : n; });
    knownConnected = nowConnected;
  } catch (e) { /* offline: keep what we have */ }
}

export function startCpdlcPolling(): void {
  if (timer) return;
  refreshCpdlc();
  loadFreetexts();
  timer = setInterval(refreshCpdlc, 4000);
}

export function stopCpdlcPolling(): void {
  if (timer) clearInterval(timer);
  timer = null;
}

export function isConnected(callsign: string): boolean {
  return get(cpdlcStatus).connected.some((c) => c.callsign === callsign);
}

export function unreadFor(callsign: string): number {
  return get(cpdlcStatus).unread?.[callsign] ?? 0;
}

export async function loadMessages(callsign: string): Promise<CpdlcMessage[] | string> {
  try {
    const r = await fetch(apiBase + "/cpdlc/messages?callsign=" + encodeURIComponent(callsign), { credentials: "same-origin" });
    const body = await r.json().catch(() => ({}));
    return r.ok ? body : body.error ?? "could not load messages";
  } catch (e) {
    return "cannot reach the server";
  }
}

async function post(path: string, body: any): Promise<{ ok: boolean; data: any }> {
  try {
    const r = await fetch(apiBase + path, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(body), credentials: "same-origin" });
    return { ok: r.ok, data: await r.json().catch(() => ({})) };
  } catch (e) {
    return { ok: false, data: { error: "cannot reach the server" } };
  }
}

// Returns an error message, or "" on success
export async function sendUplink(callsign: string, text: string, ra: string, mrn?: number | null): Promise<string> {
  const r = await post("/cpdlc/send", { callsign, text, ra, mrn: mrn ?? "" });
  if (r.ok) { refreshCpdlc(); return ""; }
  return r.data.error ?? "send failed";
}

// Sign in to the station for your position with your own Hoppie code. Returns an error message, or "" on success.
export async function logonCpdlc(code: string): Promise<string> {
  const r = await post("/cpdlc/logon", { code });
  if (r.ok) { prevUnread = -1; knownConnected = null; refreshCpdlc(); return ""; }
  return r.data.error ?? "could not sign in to CPDLC";
}

// Force a CPDLC station callsign off this server. Returns an error message, or "" on success.
export async function forceLogout(callsign: string, understand: string): Promise<string> {
  const r = await post("/cpdlc/force-logout", { callsign, confirm: true, understand });
  refreshCpdlc();
  return r.ok ? "" : (r.data.error ?? "could not force the log out");
}

// Log one pilot off CPDLC from your station (stuck or unwanted connection). Returns an error message, or "" on success.
export async function dismissRcl(id: number): Promise<string> {
  const r = await post("/cpdlc/rcl-dismiss", { id });
  refreshCpdlc();
  return r.ok ? "" : (r.data.error ?? "could not remove the request");
}

export async function disconnectPilot(callsign: string): Promise<string> {
  const r = await post("/cpdlc/disconnect", { callsign });
  refreshCpdlc();
  return r.ok ? "" : (r.data.error ?? "could not log the pilot off");
}

export async function logoffCpdlc(): Promise<void> {
  await post("/cpdlc/logoff", {});
  refreshCpdlc();
}

export async function markSeen(callsign: string): Promise<void> {
  await post("/cpdlc/seen", { callsign });
  refreshCpdlc();
}

// Ready-made uplinks (the text is filled in, the controller edits the @...@ parts). RA = reply wanted.
export const templates: { label: string; text: string; ra: string }[] = [
  { label: "Climb and maintain", text: "CLIMB TO AND MAINTAIN @FL350@", ra: "WU" },
  { label: "Descend and maintain", text: "DESCEND TO AND MAINTAIN @FL350@", ra: "WU" },
  { label: "Maintain level", text: "MAINTAIN @FL350@", ra: "WU" },
  { label: "Expect climb", text: "EXPECT CLIMB TO @FL350@", ra: "R" },
  { label: "Contact", text: "CONTACT @STATION@ @FREQUENCY@", ra: "WU" },
  { label: "Squawk", text: "SQUAWK @2000@", ra: "WU" },
  { label: "Standby", text: "STANDBY", ra: "NE" },
  { label: "Roger", text: "ROGER", ra: "NE" },
  { label: "Unable", text: "UNABLE", ra: "NE" },
  { label: "Affirm", text: "AFFIRM", ra: "NE" },
  { label: "Negative", text: "NEGATIVE", ra: "NE" },
];
