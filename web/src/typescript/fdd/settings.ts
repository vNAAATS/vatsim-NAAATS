/*
 *  The Settings box: the plugin options kept in vnaaats.cfg, chosen on the FDD.
 *  The server stores them per controller; the controller's EuroScope plugin writes them into vnaaats.cfg (within about 2 minutes)
 *  and they take effect the next time EuroScope is started.
 */
import { writable } from "svelte/store";

const apiBase: string = (import.meta as any).env?.VITE_API_BASE ?? "";

export const settingsOpen = writable<boolean>(false);

export type SettingsData = { settings: Record<string, string>; stamp: number; defaults: Record<string, string> };

export async function loadSettings(): Promise<SettingsData | string> {
  try {
    const r = await fetch(apiBase + "/settings", { credentials: "same-origin" });
    const d = await r.json().catch(() => ({}));
    return r.ok ? d : (d.error ?? "could not load the settings");
  } catch (e) { return "cannot reach the server"; }
}

// Returns an error message, or "" on success
export async function saveSettings(settings: Record<string, string>): Promise<string> {
  try {
    const r = await fetch(apiBase + "/settings", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ settings }), credentials: "same-origin" });
    const d = await r.json().catch(() => ({}));
    return r.ok ? "" : (d.error ?? "could not save the settings");
  } catch (e) { return "cannot reach the server"; }
}
