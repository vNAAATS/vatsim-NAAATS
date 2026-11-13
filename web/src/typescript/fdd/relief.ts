/*
 *  Relief: when another controller takes over your position, every aircraft you track is handed to them.
 */
import { writable } from "svelte/store";
const apiBase: string = (import.meta as any).env?.VITE_API_BASE ?? "";

export const reliefOpen = writable<boolean>(false);

export type ReliefPlan = { position: string; pluginAlive: boolean; aircraft: { callsign: string; cpdlc: boolean }[]; message: string };

export async function loadPlan(): Promise<ReliefPlan | string> {
  try {
    const r = await fetch(apiBase + "/relief/plan", { credentials: "same-origin" });
    const d = await r.json().catch(() => ({}));
    return r.ok ? d : (d.error ?? "could not read your tracked aircraft");
  } catch (e) { return "cannot reach the server"; }
}

export async function startRelief(target: string, callsigns: string[]): Promise<{ commands: { callsign: string; id: number }[]; cpdlcTold: string[] } | string> {
  try {
    const r = await fetch(apiBase + "/relief/start", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ target, callsigns }), credentials: "same-origin" });
    const d = await r.json().catch(() => ({}));
    return r.ok ? d : (d.error ?? "could not start the relief");
  } catch (e) { return "cannot reach the server"; }
}
