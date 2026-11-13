// Click a callsign anywhere (history lists, handoff messages, popped-out windows) to show its flight plan: the Flight Plan window follows the selected aircraft.
// A popped-out window cannot reach the main display directly, so it leaves the callsign in local storage and the main display picks it up.
import * as dataHandler from "./data";

const KEY = "vnaaats.showac";

/** Select the aircraft. Returns false when it is not on this display (not in the flight list), so the caller can say so. */
export function showFlightPlan(callsign: string): boolean {
  const cs = (callsign ?? "").trim().toUpperCase();
  if (!cs || cs === "YOU") return false;
  try { localStorage.setItem(KEY, JSON.stringify({ cs, ts: Date.now() })); } catch (e) { /* storage blocked: only this window follows */ }
  if (dataHandler.findAircraftLocal(cs) == null) return false;
  dataHandler.setAsel(cs);
  return true;
}

try {
  window.addEventListener("storage", (e) => {
    if (e.key !== KEY || !e.newValue) return;
    try { dataHandler.setAsel(JSON.parse(e.newValue).cs); } catch (x) { /* ignore a bad value */ }
  });
} catch (e) { /* no window */ }
