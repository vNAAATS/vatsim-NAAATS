/*
 *  Strips hidden by this person on this browser only. Nobody else's display changes.
 *  Kept for 12 hours, then the strip comes back by itself.
 */
import { writable } from "svelte/store";

const KEY = "vnaaats.hidden";
const KEEP_MS = 12 * 3600 * 1000;

function load(): Record<string, number> {
  try {
    const raw = JSON.parse(localStorage.getItem(KEY) ?? "{}") as Record<string, number>;
    const out: Record<string, number> = {};
    for (const [cs, at] of Object.entries(raw)) if (typeof at === "number" && Date.now() - at < KEEP_MS) out[cs] = at;
    return out;
  } catch (e) { return {}; }
}

export const hidden = writable<Record<string, number>>(load());

function save(v: Record<string, number>): void {
  try { localStorage.setItem(KEY, JSON.stringify(v)); } catch (e) { /* storage blocked: it still works until the page is reloaded */ }
}

export function hideStrip(callsign: string): void {
  hidden.update((h) => { const n = { ...h, [callsign.toUpperCase()]: Date.now() }; save(n); return n; });
}

export function unhideStrip(callsign: string): void {
  hidden.update((h) => { const n = { ...h }; delete n[callsign.toUpperCase()]; save(n); return n; });
}
