/*
 *  The vNAAATS app (a small Windows program that opens this display) can do things a web page cannot:
 *  find the plugin, replace an old DLL, save your key into vnaaats.cfg, start EuroScope and VACS.
 *  In a normal browser none of this exists and the display works as before.
 */
export type AppStatus = {
  app_version: string; root: string | null; euroscope: string | null; vacs: string | null; afv: string | null; trackaudio: string | null;
  dll_found: boolean; dll_path: string | null; dll_sha: string | null;
  latest_version: string | null; latest_sha: string | null; up_to_date: boolean | null;
  airlines_found: boolean; airlines_up_to_date: boolean | null; font_found?: boolean;
  cfg_exists: boolean; cfg_has_key: boolean; error: string | null;
};

function tauri(): any { try { return (window as any).__TAURI__; } catch (e) { return null; } }
export function inApp(): boolean { return !!tauri()?.core?.invoke; }

async function call<T>(cmd: string, args: Record<string, unknown> = {}): Promise<T> {
  return await tauri().core.invoke(cmd, args) as T;
}

export const status = (): Promise<AppStatus> => call<AppStatus>("status");
export const pickFolder = (): Promise<string | null> => call("pick_folder");
export const refreshCfg = (): Promise<boolean> => call("refresh_cfg");
export const updatePlugin = (): Promise<string> => call("update_plugin");
export const saveKey = (key: string): Promise<void> => call("save_key", { key });
export type ProgKind = "euroscope" | "vacs" | "afv" | "trackaudio";
export const pickExe = (kind: ProgKind): Promise<string | null> => call("pick_exe", { kind });
export const launch = (kind: ProgKind): Promise<void> => call("launch", { kind });

// Open a web page in the normal browser (pop-up windows do not work inside the app). Falls back to window.open outside the app.
// With strict set, a failure in the app is thrown so the caller can show it; otherwise it falls back to a normal window.
export async function openExternal(url: string, strict: boolean = false): Promise<void> {
  if (inApp()) {
    try { await call("open_url", { url }); return; } catch (e) { if (strict) throw e; }
  }
  window.open(url, "_blank");
}

// natTrak in the app's own window (it keeps the controller's sign-in). login=true opens natTrak's home page for the VATSIM sign-in,
// which moves on to the pending list by itself once signed in; login=false opens the pending list.
export const natTrakWindow = (login: boolean): Promise<void> => call("nattrak_open", { login });
export const natTrakSignedIn = async (): Promise<boolean> => { try { return !!(await call<boolean>("nattrak_signed_in")); } catch (e) { return false; } };
/** "yes" signed in, "no" not yet, "old" this copy of the app is too old to have the natTrak window (it must be reinstalled). */
export async function natTrakState(): Promise<"yes" | "no" | "old"> {
  try { return (await call<boolean>("nattrak_signed_in")) ? "yes" : "no"; } catch (e) { return "old"; }
}
/** A newer version of FDD for Windows, or null. Needs an app that has the updater (older copies answer with an error, treated as none). */
export async function updateCheck(): Promise<{ version: string; notes: string } | null> {
  try { return await call<{ version: string; notes: string } | null>("app_update_check"); } catch (e) { return null; }
}
/** Downloads and installs the newer version; the app then restarts by itself. */
export const updateInstall = (): Promise<void> => call("app_update_install");
export const APP_DOWNLOAD = "https://github.com/garythms/vatsim-NAAATS-v2/releases/download/fdd-windows/FDD-for-Windows-Setup.exe";

/** Opens natTrak in the app window: the sign-in page if the controller has not signed in yet, else the pending list. */
export async function natTrakOpen(): Promise<void> { await natTrakWindow(!(await natTrakSignedIn())); }

// "Show the setup screen again the next time FDD for Windows starts" (to change the audio client, say). The setup screen only appears
// at start-up for a first-time user, so this puts the controller back in that state.
export function setupAgainSet(on: boolean): void {
  try { if (on) localStorage.removeItem("vnaaats.appSetupDone"); else localStorage.setItem("vnaaats.appSetupDone", "1"); } catch (e) { /* not kept */ }
}
export function setupAgainGet(): boolean { try { return localStorage.getItem("vnaaats.appSetupDone") !== "1"; } catch (e) { return false; } }

// Settings for opening the programs automatically once a position is chosen (kept in this app's own storage)
export type AutoKind = ProgKind;
// Which audio client the controller uses: "afv" (Audio for VATSIM), "trackaudio" or "none"
export type AudioChoice = "afv" | "trackaudio" | "none";
export function audioGet(): AudioChoice { try { const v = localStorage.getItem("vnaaats.audio"); return v === "afv" || v === "trackaudio" ? v : "none"; } catch (e) { return "none"; } }
export function audioSet(c: AudioChoice): void { try { localStorage.setItem("vnaaats.audio", c); } catch (e) { /* not kept */ } }
export function autoGet(kind: AutoKind): boolean { try { return localStorage.getItem("vnaaats.auto." + kind) !== "0"; } catch (e) { return true; } }
export function autoSet(kind: AutoKind, on: boolean): void { try { localStorage.setItem("vnaaats.auto." + kind, on ? "1" : "0"); } catch (e) { /* not kept */ } }

// Programs that did not start by themselves after a position was chosen (shown as a small notice)
import { writable } from "svelte/store";
export const launchNotes = writable<string[]>([]);
const PROG_NAME: Record<string, string> = { euroscope: "EuroScope", vacs: "VACS", afv: "Audio for VATSIM", trackaudio: "TrackAudio" };

/** Called after a position is chosen: starts EuroScope, VACS and the chosen audio client once per app session, if ticked and located. */
export async function autoLaunch(): Promise<void> {
  if (!inApp()) return;
  try { if (sessionStorage.getItem("vnaaats.cfgrefreshed") !== "1") { sessionStorage.setItem("vnaaats.cfgrefreshed", "1"); await refreshCfg(); } } catch (e) { /* the file stays as it is */ }   // vnaaats.cfg gets the current comments and option list once per app session; values are kept
  let st: AppStatus;
  try { st = await status(); } catch (e) { return; }
  const audio = audioGet();
  const notes: string[] = [];
  for (const k of ["euroscope", "vacs", ...(audio === "none" ? [] : [audio])] as AutoKind[]) {
    if (!autoGet(k)) continue;
    if (!st[k]) { if (k === audio) notes.push(PROG_NAME[k] + " is not opened: its program has not been chosen yet (setup screen)."); continue; }
    const flag = "vnaaats.launched." + k;
    try { if (sessionStorage.getItem(flag) === "1") continue; } catch (e) { /* carry on */ }
    try { await launch(k); try { sessionStorage.setItem(flag, "1"); } catch (e) { /* carry on */ } }
    catch (e) { notes.push(PROG_NAME[k] + " did not start: " + errText(e)); }       // not marked as launched, so the next position choice tries again
  }
  if (notes.length) { launchNotes.set(notes); setTimeout(() => launchNotes.set([]), 15000); }
}

// Opens the setup screen again (from the settings window)
export const setupOpen = writable<boolean>(false);

// The welcome animation tells the setup screen when it has finished
let welcomeResolve: () => void = () => {};
export const welcomeDone: Promise<void> = new Promise<void>((r) => { welcomeResolve = r; });
export function finishWelcome(): void { welcomeResolve(); }

// The saved sign-in (kept in the Windows Credential Manager)
export const credGet = (): Promise<{ name: string; password: string } | null> => call("cred_get");
export const credSave = (name: string, password: string): Promise<void> => call("cred_save", { name, password });
export const credForget = (): Promise<void> => call("cred_forget");

export function errText(e: unknown): string { return typeof e === "string" ? e : (e as any)?.message ?? "something went wrong"; }

// Open natTrak's "new clearance" page with fields filled in. It never presses Create. In the app this is its own window that fills the form;
// in a browser it opens natTrak and the caller shows the values to copy.
export async function openNatTrak(fields: { id: string; value: string }[]): Promise<"filled" | "opened"> {
  if (inApp()) {
    try { await call("open_nattrak", { fields: fields.filter((f) => f.value).map((f) => [f.id, f.value]) }); return "filled"; } catch (e) { /* older app: just open it */ }
  }
  window.open("https://nattrak.vatsim.net/controllers/clx/create", "_blank");
  return "opened";
}
