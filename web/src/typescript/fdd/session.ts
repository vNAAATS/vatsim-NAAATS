/*
 *  Who is using the FDD, which position they are working, and sending edits to the server.
 */
import { writable } from "svelte/store";
import * as dataHandler from "./data";

const apiBase: string = (import.meta as any).env?.VITE_API_BASE ?? "";

export type Session = {
  editing: boolean;   // the server has some way to log in
  password: boolean;
  vatsim: boolean;
  loggedIn: boolean;
  name: string;
  cid: string;
  position: string;
  positions: string[];
  keyExpires?: number;   // when the API key used to sign in runs out (unix seconds; 0 = not signed in with a key)
};

export const emptySession: Session = { editing: false, password: false, vatsim: false, loggedIn: false, name: "", cid: "", position: "", positions: [] };
export const session = writable<Session>(emptySession);

export function canEdit(s: Session): boolean {
  return s.loggedIn && s.position !== "";
}

async function post(path: string, body: any): Promise<{ ok: boolean; data: any }> {
  try {
    const r = await fetch(apiBase + path, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(body), credentials: "same-origin" });
    return { ok: r.ok, data: await r.json().catch(() => ({})) };
  } catch (e) {
    return { ok: false, data: { error: "cannot reach the server" } };
  }
}

export async function refreshSession(): Promise<void> {
  try {
    const r = await fetch(apiBase + "/auth/status", { credentials: "same-origin" });
    if (r.ok) session.set(await r.json());
  } catch (e) { /* offline: keep what we have */ }
}

// Returns an error message, or "" on success
export async function loginPassword(name: string, password: string): Promise<string> {
  const r = await post("/auth/password", { name, password });
  if (r.ok) { session.set(r.data); return ""; }
  return r.data.error ?? "login failed";
}

// ---- key sign-in: the key from vnaaats.com/get-key (or the Gander Oceanic website) identifies you; the display opens once you are
// connected to VATSIM on an oceanic position. The key is kept in this browser / app only.
export function getKey(): string { try { return localStorage.getItem("vnaaats.key") ?? ""; } catch (e) { return ""; } }
export function rememberKey(key: string): void { try { if (key) localStorage.setItem("vnaaats.key", key.trim()); else localStorage.removeItem("vnaaats.key"); } catch (e) { /* not kept */ } }

export type KeyResult = { state: "in" | "waiting" | "error"; msg: string; cid?: string };
export async function loginKey(key: string): Promise<KeyResult> {
  const r = await post("/auth/key-login", { key });
  if (r.ok && r.data.waiting) return { state: "waiting", msg: "", cid: r.data.cid };
  if (r.ok) { session.set(r.data); return { state: "in", msg: "" }; }
  return { state: "error", msg: r.data.error ?? "sign-in failed" };
}

export async function choosePosition(position: string): Promise<string> {
  const r = await post("/auth/position", { position });
  if (r.ok) { session.set(r.data); if (position === "") dataHandler.clearData(); else dataHandler.populateAllAircraft(); return ""; }
  return r.data.error ?? "could not set the position";
}

export async function logout(): Promise<void> {
  await post("/auth/logout", {});
  dataHandler.clearData();                          // nothing stays on the screen once you sign out
  await refreshSession();
}

export function vatsimLogin(): void {
  window.location.href = apiBase + "/auth/login";
}

// changes: any of level, mach, track, selcal. Returns an error message, or "" on success.
// Concorde position report: est / act are four digits ("" clears). Returns an error message or "".
export async function editPosReport(callsign: string, fix: string, est: string, act: string): Promise<string> {
  const r = await post("/EditPosReport", { callsign, fix, est, act });
  if (!r.ok) {
    if (r.data.error && /log in/.test(r.data.error)) await refreshSession();
    return r.data.error ?? "could not save";
  }
  await dataHandler.populateAllAircraft();
  dataHandler.reselect(callsign);
  return "";
}

export async function editFlight(callsign: string, changes: Record<string, string | number>): Promise<string> {
  const r = await post("/EditFlight", Object.assign({ callsign }, changes));
  if (!r.ok) {
    if (r.data.error && /log in/.test(r.data.error)) await refreshSession();
    return r.data.error ?? "edit failed";
  }
  await dataHandler.populateAllAircraft();        // show it straight away
  dataHandler.reselect(callsign);
  return "";
}

// Add a flight by callsign (looked up on VATSIM). Returns an error message, or "" on success.
export async function addFlight(callsign: string): Promise<string> {
  const r = await post("/AddFlight", { callsign });
  if (!r.ok) {
    if (r.data.error && /log in/.test(r.data.error)) await refreshSession();
    return r.data.error ?? "could not add the flight";
  }
  await dataHandler.populateAllAircraft();
  dataHandler.reselect(callsign.toUpperCase());
  return "";
}

// Hide a strip from the display. Returns an error message, or "" on success.
export async function removeFlight(callsign: string): Promise<string> {
  const r = await post("/RemoveFlight", { callsign });
  if (!r.ok) {
    if (r.data.error && /log in/.test(r.data.error)) await refreshSession();
    return r.data.error ?? "could not remove the flight";
  }
  await dataHandler.populateAllAircraft();
  return "";
}

// ---- automatic RCL REQ reminder (switch on the server, shown in the controls bar)
export type RclWaiting = { callsign: string; minutes: number | null; fix: string; time: string };
export type RclAuto = {
  on: boolean; sender: string; inflight: number; error: string;
  confirm: boolean;                       // ask the sending controller before anything is typed
  waiting: RclWaiting[];                  // due, waiting for the controller's answer
  prompt: boolean;                        // time to ask (at most every 5 minutes)
  sending: number;                        // released and not yet typed
  batch: { total: number; done: number; failed: string[] };
  history?: { ts: number; callsign: string; kind: string; detail: string }[];   // newest first
};
export const rclAuto = writable<RclAuto>({ on: false, sender: "", inflight: 0, error: "", confirm: true, waiting: [], prompt: false, sending: 0, batch: { total: 0, done: 0, failed: [] }, history: [] });
export const rclPanelOpen = writable<boolean>(false);

export async function refreshRclAuto(): Promise<void> {
  try {
    const r = await fetch(apiBase + "/rcl/auto", { credentials: "same-origin" });
    if (r.ok) { const d = await r.json(); rclAuto.update((old) => ({ ...old, ...d })); }
  } catch (e) { /* keep the last answer */ }
}

export async function setRclConfirm(confirm: boolean): Promise<string> {
  const r = await post("/rcl/auto", { confirm });
  if (r.ok) { rclAuto.update((old) => ({ ...old, ...r.data })); return ""; }
  return r.data.error ?? "could not change the setting";
}

// The answer to "send the reminders now?": send | later | skip
export async function rclAnswer(action: "send" | "later" | "skip" | "stop"): Promise<string> {
  const r = await post("/rcl/auto", { action });
  if (r.ok) { rclAuto.update((old) => ({ ...old, ...r.data })); return ""; }
  return r.data.error ?? "could not send the answer";
}

export async function setRclAuto(on: boolean): Promise<string> {
  const r = await post("/rcl/auto", { on });
  if (r.ok) { rclAuto.update((old) => ({ ...old, ...r.data })); return ""; }
  return r.data.error ?? "could not change the setting";
}
