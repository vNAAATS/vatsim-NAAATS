/*
 *  Aircraft menu (click a callsign): commands for EuroScope and copy-and-paste EuroScope dot commands.
 */
import { writable } from "svelte/store";

const apiBase: string = (import.meta as any).env?.VITE_API_BASE ?? "";

// The open aircraft menus, newest first (at most 3). Empty = no menu open.
export const MAX_MENUS = 3;
export const menuStack = writable<string[]>([]);
export function openMenu(callsign: string): void {
  const cs = (callsign || "").toUpperCase();
  if (!cs) return;
  menuStack.update((l) => [cs, ...l.filter((x) => x !== cs)].slice(0, MAX_MENUS));
}
export function closeMenu(callsign: string): void { menuStack.update((l) => l.filter((x) => x !== callsign)); }
export function closeAllMenus(): void { menuStack.set([]); }

export type Controller = { callsign: string; frequency: string; name: string; facility: number };

export async function loadControllers(): Promise<Controller[]> {
  try {
    const r = await fetch(apiBase + "/GetControllers", { credentials: "same-origin" });
    return r.ok ? await r.json() : [];
  } catch (e) { return []; }
}

// Copy text to the clipboard. Returns true if it worked.
export async function copyText(text: string): Promise<boolean> {
  try {
    await navigator.clipboard.writeText(text);
    return true;
  } catch (e) {
    try {
      const ta = document.createElement("textarea");
      ta.value = text;
      ta.style.position = "fixed";
      ta.style.opacity = "0";
      document.body.appendChild(ta);
      ta.select();
      const ok = document.execCommand("copy");
      document.body.removeChild(ta);
      return ok;
    } catch (e2) { return false; }
  }
}

export const NO_CONTACT_TEXT = "This aircraft is within my controlling area but has not responded to contact me requests";

export function selcalCommand(code: string): string { return ".selcal " + code.toUpperCase().replace(/[^A-Z]/g, ""); }
// EuroScope's .contactme takes no callsign: it acts on the aircraft you click or have selected
export function contactMeCommand(_callsign: string): string { return ".contactme"; }
export function wallopCommand(callsign: string, text: string): string {
  const t = text.trim().replace(/"/g, "'");
  return ".wallop " + callsign + " " + (t.indexOf(" ") >= 0 ? '"' + t + '"' : t);
}
export function validSelcal(code: string): boolean { return /^[A-Z]{4}$/.test(code.toUpperCase().replace(/[-\s]/g, "")); }   // any four letters: VATSIM pilots are not limited to the real-world SELCAL letters
// Why a SELCAL code is not valid ("" if it is). Any four letters are fine: VATSIM pilots are not limited to the real-world SELCAL letters.
export function selcalProblem(code: string): string {
  const c = code.toUpperCase().replace(/[-\s]/g, "");
  return /^[A-Z]{4}$/.test(c) ? "" : "SELCAL must be exactly 4 letters, like ABCD";
}

// Send a command to the controller's EuroScope. Returns an error message, or the command id as a number.
export async function queueCommand(body: Record<string, string | number>): Promise<string | number> {
  try {
    const r = await fetch(apiBase + "/cmd/queue", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(body), credentials: "same-origin" });
    const d = await r.json().catch(() => ({}));
    return r.ok ? d.id : (d.error ?? "could not send the command");
  } catch (e) { return "cannot reach the server"; }
}

// Clicking an aircraft on the FDD circles it in white in EuroScope for a few seconds (switch it off with "Circle" on the FDD).
function readCircle(): boolean { try { return localStorage.getItem("vnaaats.circle") !== "off"; } catch (e) { return true; } }
export let circleOn: boolean = readCircle();
export function setCircle(on: boolean): void {
  circleOn = on;
  try { localStorage.setItem("vnaaats.circle", on ? "on" : "off"); } catch (e) { /* not saved, still works for this visit */ }
}
export const circleNote = writable<string>("");     // why the circle did not appear ("" = nothing to say)
let circleNoteTimer: any = null;
function noteCircle(text: string): void {
  circleNote.set(text);
  if (circleNoteTimer) clearTimeout(circleNoteTimer);
  if (text) circleNoteTimer = setTimeout(() => circleNote.set(""), 10000);
}
export async function circleInEs(callsign: string): Promise<void> {
  if (!circleOn || !callsign) return;
  try {
    const id = await queueCommand({ type: "highlight", callsign });
    if (typeof id === "string") { noteCircle("Circle: " + id); return; }
    // wait up to 8 seconds for EuroScope to answer
    for (let i = 0; i < 8; i++) {
      await new Promise((r) => setTimeout(r, 1000));
      const s = await commandState(id);
      if (s.state === "ok") { if (s.msg.indexOf("switched off") >= 0) noteCircle("Circle: " + s.msg); return; }
      if (s.state === "failed") { noteCircle("Circle: " + s.msg); return; }
    }
    noteCircle("Circle: your EuroScope did not answer. It needs the vNAAATS plugin 2.5.0 or newer, running with the vNAAATS display open. Turn the Circle off here if you do not want it.");
  } catch (e) { /* the circle is a nicety: never break a click */ }
}

// Wait for EuroScope's answer. state: ok | failed | waiting | pending | delivered
export async function commandState(id: number): Promise<{ state: string; msg: string }> {
  try {
    const r = await fetch(apiBase + "/cmd/status?id=" + id, { credentials: "same-origin" });
    if (!r.ok) return { state: "failed", msg: "lost track of the command" };
    return await r.json();
  } catch (e) { return { state: "pending", msg: "" }; }
}

export function natTrakCommand(callsign: string): string {
  return ".msg " + callsign + " Hello, you will soon be entering oceanic airspace. Please request your oceanic clearance by CPDLC, or via nattrak.vatsim.net, no later than 20 minutes before your oceanic entry point. Squawk 2000 upon entering the OCA.";
}

// Remember (sent = true) or forget (false) that the natTrak reminder was sent. Returns an error message or "".
export async function markNatTrakSent(callsign: string, sent: boolean): Promise<string> {
  try {
    const r = await fetch(apiBase + "/NatTrakSent", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ callsign, sent }), credentials: "same-origin" });
    const d = await r.json().catch(() => ({}));
    return r.ok ? "" : (d.error ?? "could not save");
  } catch (e) { return "cannot reach the server"; }
}
