/*
 *  Flight plan edit window: read the flight plan from the controller's EuroScope and send changes back.
 *  Only the controller tracking the aircraft can save (EuroScope allows nothing else); the plugin checks that again.
 */
import { writable } from "svelte/store";
import { queueCommand, commandState } from "./acmenu";

export const planEditCallsign = writable<string>("");   // "" = window closed

export type Plan = {
  planType: string; acInfo: string; origin: string; dest: string; alternate: string;
  tas: number; alt: number; etd: string; enrouteH: number; enrouteM: number; fuelH: number; fuelM: number;
  route: string; remarks: string;
};
export type PlanRead = { plan?: Plan; mine: boolean; trackedBy: string; error?: string };

// "NOFP" is what EuroScope shows when a field has no data
export function isNofp(v: string | undefined | null): boolean {
  const t = (v ?? "").trim().toUpperCase();
  return t === "" || t === "NOFP";
}

async function waitFor(id: number, seconds: number): Promise<{ state: string; msg: string }> {
  let last = { state: "pending", msg: "" };
  for (let i = 0; i < seconds; i++) {
    await new Promise((r) => setTimeout(r, 1000));
    last = await commandState(id);
    if (last.state === "ok" || last.state === "failed") return last;
  }
  return last;
}

const num = (v: any): number => { const n = parseInt(String(v ?? "0"), 10); return isNaN(n) ? 0 : n; };

// Ask EuroScope for the whole flight plan.
export async function loadPlan(callsign: string): Promise<PlanRead> {
  const id = await queueCommand({ type: "planget", callsign });
  if (typeof id === "string") return { mine: false, trackedBy: "", error: id };
  const s = await waitFor(id, 25);
  if (s.state === "waiting" || s.state === "pending" || s.state === "delivered") {
    return { mine: false, trackedBy: "", error: "No answer from EuroScope - is it running with the vNAAATS plugin and signed in as your position?" };
  }
  if (s.state === "failed") return { mine: false, trackedBy: "", error: s.msg || "EuroScope could not give the flight plan" };
  try {
    const j = JSON.parse(s.msg);
    return {
      mine: !!j.mine, trackedBy: String(j.trackedBy ?? ""),
      plan: {
        planType: String(j.planType ?? "I").toUpperCase() === "V" ? "V" : "I",
        acInfo: String(j.acInfo ?? ""), origin: isNofp(j.origin) ? "" : String(j.origin), dest: isNofp(j.dest) ? "" : String(j.dest),
        alternate: String(j.alternate ?? ""), tas: num(j.tas), alt: num(j.alt), etd: String(j.etd ?? "").padStart(4, "0").slice(-4),
        enrouteH: num(j.enrouteH), enrouteM: num(j.enrouteM), fuelH: num(j.fuelH), fuelM: num(j.fuelM),
        route: String(j.route ?? ""), remarks: String(j.remarks ?? ""),
      },
    };
  } catch (e) { return { mine: false, trackedBy: "", error: "The flight plan came back in a form I could not read." }; }
}

// Level typed as FL390, 390 or 39000 -> feet
export function levelToFeet(text: string): number | null {
  const t = text.trim().toUpperCase().replace(/^FL/, "");
  if (!/^\d{1,5}$/.test(t)) return null;
  const n = parseInt(t, 10);
  return n <= 600 ? n * 100 : n;
}
export function feetToLevel(ft: number): string { return ft > 0 ? "FL" + String(Math.round(ft / 100)).padStart(3, "0") : ""; }

// Problems with the typed values ("" = fine). Same rules as the server.
export function planProblem(p: Plan, levelText: string): string {
  if (!/^[A-Z0-9]{3,4}$/.test(p.origin.trim().toUpperCase())) return "Departure must be an airport code, like KEWR";
  if (!/^[A-Z0-9]{3,4}$/.test(p.dest.trim().toUpperCase())) return "Destination must be an airport code, like EGLL";
  if (p.alternate.trim() !== "" && !/^[A-Z0-9]{3,4}$/.test(p.alternate.trim().toUpperCase())) return "Alternate must be an airport code or empty";
  if (!/^[A-Z0-9\/\-]{1,30}$/.test(p.acInfo.trim().toUpperCase())) return "Aircraft data looks like H/B77W/L";
  if (levelToFeet(levelText) === null || levelToFeet(levelText)! > 60000) return "Level looks like FL390";
  if (p.tas < 0 || p.tas > 999) return "TAS must be 0 to 999";
  if (!/^([01]\d|2[0-3])[0-5]\d$/.test(p.etd)) return "Departure time is four digits, like 1435";
  if (p.enrouteH < 0 || p.enrouteH > 99 || p.enrouteM < 0 || p.enrouteM > 59 || p.fuelH < 0 || p.fuelH > 99 || p.fuelM < 0 || p.fuelM > 59) return "Enroute and fuel times: hours 0-99, minutes 0-59";
  if (!/^[A-Za-z0-9\/ ._+\-]{0,1500}$/.test(p.route)) return "The route may only use letters, numbers, / . + - and spaces";
  if (!/^[ -~]{0,500}$/.test(p.remarks)) return "Remarks may only use normal keyboard characters";
  return "";
}

const LABELS: Record<string, string> = { planType: "Flight rules", acInfo: "Aircraft data", origin: "Departure", dest: "Destination", alternate: "Alternate", tas: "TAS", alt: "Level", etd: "Departure time", enrouteH: "Enroute hours", enrouteM: "Enroute minutes", fuelH: "Fuel hours", fuelM: "Fuel minutes", route: "Route", remarks: "Remarks" };

// What will change (for the confirmation)
export function changes(before: Plan, after: Plan): string[] {
  const out: string[] = [];
  for (const k of Object.keys(LABELS) as (keyof Plan)[]) {
    const a = String(before[k] ?? "").trim(), b = String(after[k] ?? "").trim();
    if (a !== b) out.push(LABELS[k] + ": " + (a || "(empty)") + "  ->  " + (b || "(empty)"));
  }
  return out;
}

// Send the changes to EuroScope. Returns an error message, or "" when EuroScope has sent the amended plan.
export async function savePlan(callsign: string, p: Plan): Promise<string> {
  const id = await queueCommand({
    type: "planset", callsign,
    plan: {
      planType: p.planType, acInfo: p.acInfo.trim().toUpperCase(), origin: p.origin.trim().toUpperCase(), dest: p.dest.trim().toUpperCase(),
      alternate: p.alternate.trim().toUpperCase(), tas: p.tas, alt: p.alt, etd: p.etd, enrouteH: p.enrouteH, enrouteM: p.enrouteM,
      fuelH: p.fuelH, fuelM: p.fuelM, route: p.route.trim().toUpperCase(), remarks: p.remarks,
    } as any,
  } as any);
  if (typeof id === "string") return id;
  const s = await waitFor(id, 25);
  if (s.state === "ok") return "";
  if (s.state === "failed") return s.msg || "EuroScope would not change the flight plan";
  return "No answer from EuroScope yet - check the flight plan in a moment.";
}
