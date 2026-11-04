/*
 *  Script for handling data uplink and downlink
 *  Created by Andrew Ogden
 */

/*
 *  Imports
 */
import { CronJob } from "cron";
import type { Aircraft, NatTrack, NatTrakData } from "./objects";
import { formatWaypoint } from "./waypoint";
import { inApp as appInApp, openExternal as appOpenExternal, natTrakOpen as appNatTrakOpen } from "./app";

/*
 *  Definitions
 */
// Json interface
interface JsonAcObj {
  callsign: string;
  type: string;
  assignedLevel: number;
  assignedMach: number;
  track: string;
  route: string;
  routeEtas: string;
  departure: string;
  arrival: string;
  direction: boolean;
  etd: string;
  selcal: string;
  datalinkConnected: boolean;
  isEquipped: boolean;
  state: string;
  relevant: boolean;
  targetMode: number;
  trackedBy: string;
  trackedById: string;
  lastUpdated: string;
  source?: string;
  cpdlc?: boolean;
  distNm?: number | null;
  ntSent?: boolean;
  cmAt?: number;
  manual?: any;
  scratch?: string;
  comm?: { kind: string; by: string; freq: string } | null;
  offline?: boolean | null;
  posReports?: Record<string, { est: string; act: string; by: string; at: string }>;
  updatedBy?: string;
  updatedAt?: string;
  history?: { at: string; by: string; changes: Record<string, [any, any]> }[];
}

/*
 *  Properties
 */
// Link constants
// API base: empty = same server that serves this page. Override at build time with VITE_API_BASE=https://host
const apiBase: string = (import.meta as any).env?.VITE_API_BASE ?? "";
const natTracksApi: string = apiBase + "/GetAllNatTracks?fdd=1"; // fdd=1: also the Concorde tracks
const allAircraftGet: string = apiBase + "/GetAllFlightData";
const allNatTrakGet: string = apiBase + "/GetNatTrakData";
const networkTrafficGet: string = apiBase + "/GetNetworkTraffic";
const vatsimStatusData: string = "https://data.vatsim.net/v3/vatsim-data.json";

// Data lists
export let trackError: boolean = false;
export let currentTMI: string = "";
export let currentNatTracks: Map<string, NatTrack> = new Map();
export let networkAircraft: Map<string, Map<string, Aircraft>> = new Map();
export let natTrakData: Map<string, NatTrakData> = new Map();
export let onlineControllers: string[];
export let aircraftCount: number = 0;
export let relevantAircraftCount: number = 0;
export let sortDescending: boolean = true;
// Selected aircraft
export let asel: Aircraft = null!;

// Crons
export let acUpdateJob;
export let tkUpdateJob;

/*
 *  Functions
 */
export function findAircraftLocal(callsign: string): Aircraft {
  let found: boolean = false;
  let ac: Aircraft = null!;

  // Loop through each of the aircraft maps to check for  matches
  networkAircraft.forEach((value: Map<string, Aircraft>, key: string) => {
    if (value.size < 1) return; // no data to check so skip
    // If not undefined then we have a match so set it
    if (
      value.get(callsign) != null &&
      value.get(callsign)?.Callsign != undefined
    ) {
      // we found it
      ac = value.get(callsign)!;
      found = true;
      return;
    }
    return;
  });

  // Return it
  // we didn't find it :(
  return found ? ac : null!;
}

// Re-point the selected aircraft at the freshly downloaded copy (after data was refreshed)
export function reselect(callsign: string): void {
  const found: Aircraft = findAircraftLocal(callsign);
  if (found != null) asel = found;
}

export function setAsel(callsign: string): void {
  // Check if exists and check to see if it matches
  if (asel != null && callsign === asel.Callsign) {
    return;
  }

  // Doesn't match so assign
  const found: Aircraft = findAircraftLocal(callsign);

  if (found != null) {
    asel = found;
  }
}

export function clearAsel(): void { asel = null; }

// Clock time (HHMM) at the first route point in flight order, "" if the flight has no times
export function entryClock(ac: Aircraft): string {
  for (const t of ac.PosCalc ?? []) {
    const d: string = (t ?? "").replace(/\D/g, "");
    if (d.length === 4) return d;
  }
  return "";
}

export async function parseNatTracks(): Promise<void> {
  // Dynamic data variable
  let res: string = "";

  // Get the data
  await fetch(natTracksApi).then(function (response: Response) {
    response.text().then(function (text: string) {
      // Parse to json
      res = text;
      const objArr: any = JSON.parse(res);

      let isUpdateReqd: boolean = false;

      // Safety check to see if the array is empty
      if (objArr.length != 0) {
        trackError = false;
        // Compare the TMIs if not first run
        if (currentTMI != "") {
          // If the TMI is different then we need to update the tracks
          currentTMI !== objArr?.[0]?.tmi
            ? (isUpdateReqd = true)
            : (isUpdateReqd = false);
        } else {
          // First run so request update
          isUpdateReqd = true;
        }
      } else {
        trackError = true;
      }

      // Skip next part if no update needed
      if (!isUpdateReqd) return;

      // Purge existing tracks
      currentNatTracks = new Map();
      // Now assign
      for (let i: number = 0; i < objArr.length; i++) {
        const track: NatTrack = {
          Identifier: objArr[i].id,
          TMI: objArr[i].tmi,
          Route: objArr[i].route,
          FlightLevels: objArr[i].flightLevels,
          Direction:
            objArr[i].direction == 1 || objArr[i].direction == 2
              ? objArr[i].direction - 1
              : objArr[i].direction,
          ValidFrom: parseInt(objArr[i].validFrom),
          ValidTo: parseInt(objArr[i].validTo),
          Supersonic: !!objArr[i].supersonic,
          Tango: !!objArr[i].tango,
          TangoDir: objArr[i].tangoDir ?? "both",
          BlockLevel: objArr[i].blockLevel ?? "",
          Use: objArr[i].use ?? "",
        };

        if (!track.Direction && !track.Supersonic && !track.Tango)
          track.Route = track.Route.reverse();

        console.log(track.Identifier)
        currentNatTracks.set(track.Identifier, track);
      }

      // Force svelte to update the dom
      currentNatTracks = currentNatTracks;

      // Assign TMI
      currentTMI = objArr[0].tmi;
    });
  });
  return;
}

// ---- sanity warnings: only "this level is not one of the levels for the track". Separation between aircraft is not judged here;
// that is the controller's job (and EuroScope has its own STCA) ---------------------------------------------------------------------
export let conflicts: Map<string, string[]> = new Map();

function etaMinutes(etas: string[]): number {
  const first = etas.find((e) => /^\d{4}$/.test(e));
  return first ? parseInt(first.slice(0, 2)) * 60 + parseInt(first.slice(2)) : -1;
}

function computeConflicts(): void {
  const next: Map<string, string[]> = new Map();
  const add = (cs: string, msg: string) => next.set(cs, [...(next.get(cs) ?? []), msg]);
  networkAircraft.forEach((group: Map<string, Aircraft>, track: string) => {
    if (track === "RR") return;
    const list = [...group.values()].filter((a) => a.Relevant && Date.now() - a.LastUpdated < 30 * 60 * 1000);
    const offered = currentNatTracks.get(track)?.FlightLevels ?? [];
    for (const a of list) {
      const lv = a.AssignedLevel;
      if (offered.length > 0 && lv > 0 && !offered.some((x) => (x >= 1000 ? x / 100 : x) === lv)) {
        add(a.Callsign, "FL" + lv + " is not one of the levels for track " + track);
      }
    }
  });
  conflicts = next;
}

// Forget everything on screen (signed out, no position, or the server said the data is not for this browser)
export function clearData(): void {
  networkAircraft = new Map();
  natTrakData = new Map();
  aircraftCount = 0;
  relevantAircraftCount = 0;
  asel = null!;
}

export async function populateAllAircraft() : Promise<void> {
  // Dynamic data variable
  let res: string = "";

  // Oceanic-bound traffic from the public VATSIM feed (filed flight plans). Failure here must never block the rest.
  let networkTraffic: JsonAcObj[] = [];
  try {
    const nr: Response = await fetch(networkTrafficGet);
    if (nr.status === 401) { clearData(); return; }   // signed out: the server sends nothing
    networkTraffic = await nr.json();
  } catch (e) {
    networkTraffic = [];
  }

  await fetch(allAircraftGet + (sortDescending ? "?sort=1" : "?sort=0")).then(
    function (response: Response) {
      if (response.status === 401) { clearData(); return; }   // not signed in with a position: no data
      response.text().then(function (text: string) {
        // Parse json
        res = text;
        const posted: JsonAcObj[] = JSON.parse(res);
        // Flights posted by controllers win over the same callsign from the VATSIM feed
        const postedCallsigns = new Set(posted.map((p) => p.callsign));
        const objArr: JsonAcObj[] = posted.concat(
          networkTraffic.filter((n) => !postedCallsigns.has(n.callsign))
        );

        // Purge map if new tracks found
        if (networkAircraft.size != currentNatTracks.size - 1) {
          // Bye bye
          networkAircraft = new Map();

          // Re-instantiate tracks
          currentNatTracks.forEach(function (value: NatTrack, key: string) {
            networkAircraft.set(key, new Map());
          });

          // And add random route for good measure :)
          networkAircraft.set("RR", new Map());
        } else {
          // Just reset the already existing aircraft maps
          networkAircraft.forEach(function (value: Map<string, Aircraft>, key: string) {
            networkAircraft.set(key, new Map());
          });
          // Just in case
          networkAircraft.set("RR", new Map());
        }

        // Set counter
        aircraftCount = objArr.length;
        relevantAircraftCount = 0;


        for (let i: number = 0; i < aircraftCount; i++) {
          // Parse route
          const routePoints: string[] = objArr[i].route.split(" ").filter((p: string) => p != "").map(formatWaypoint);
          const routeEtas: string[] = objArr[i].routeEtas.split(" ");
          
          // Handle for when tracks become outdated
          if (currentNatTracks.get(objArr[i].track) == null) {
            objArr[i].track = "RR";
          }

          // Concorde: typed-in actual / estimated times replace the calculated ones
          const isConc: boolean = /^CONC/i.test(objArr[i]?.type ?? "");
          const posReports = isConc ? objArr[i].posReports ?? {} : {};
          const posCalc: string[] = [...routeEtas];
          const shownEtas: string[] = isConc
            ? routePoints.map((fix: string, k: number) => posReports[fix]?.act || posReports[fix]?.est || routeEtas[k] || "--")
            : routeEtas;

          const ac: Aircraft = {
            Callsign: objArr[i]?.callsign,
            Type: objArr[i]?.type,
            AssignedLevel: objArr[i]?.assignedLevel,
            AssignedMach: objArr[i]?.assignedMach,
            Track: objArr[i]?.track,
            Route: routePoints,
            RouteEtas: shownEtas,
            Departure: objArr[i]?.departure,
            Arrival: objArr[i]?.arrival,
            Etd: objArr[i]?.etd,
            Selcal: objArr[i]?.selcal,
            Datalink: objArr[i]?.datalinkConnected,
            SectorID: objArr[i]?.trackedById,
            State: objArr[i]?.state,
            Relevant: objArr[i]?.relevant ?? false, // TEMPORARY TODO
            IsEquipped: objArr[i]?.isEquipped,
            TrackedBy: objArr[i]?.trackedBy,
            TargetMode: objArr[i]?.targetMode,
            Direction: objArr[i]?.direction,
            LastUpdated: Date.parse(objArr[i]?.lastUpdated),
            Source: objArr[i]?.source ?? "plugin",
            Cpdlc: objArr[i]?.cpdlc ?? false,
            DistNm: objArr[i]?.distNm ?? null,
            NtSent: objArr[i]?.ntSent ?? false,
            ContactMeAt: objArr[i]?.cmAt ? objArr[i].cmAt * 1000 : 0,
            Manual: !!objArr[i]?.manual,
            Scratch: objArr[i]?.scratch ?? "",
            Comm: objArr[i]?.comm ?? null,
            Offline: objArr[i]?.offline ?? null,
            Entry: routePoints[0] ?? "",
            Exit: routePoints[routePoints.length - 1] ?? "",
            EntryEta: etaMinutes(routeEtas),
            UpdatedBy: objArr[i]?.updatedBy ?? "",
            UpdatedAt: objArr[i]?.updatedAt ? Date.parse(objArr[i].updatedAt!) : 0,
            History: objArr[i]?.history ?? [],
            IsConc: isConc,
            PosFixes: [...routePoints],
            PosCalc: posCalc,
            PosReports: posReports,
          };
          // Check the track
          if (ac.Track != "RR" && currentNatTracks.has(ac.Track)) {
            networkAircraft.get(ac.Track)?.set(ac.Callsign, ac);
          } else {
            networkAircraft.get("RR")?.set(ac.Callsign, ac);
          }

          if (ac.Relevant) {
            relevantAircraftCount++;
          }
          // Reverse route if needed
          if (!ac.Direction && !currentNatTracks.get(ac.Track)?.Tango) {      // Tango routes keep the order flown
            ac.Route = ac.Route.reverse();
            ac.RouteEtas = ac.RouteEtas.reverse();
          }
        }
        computeConflicts();
        if (asel != null) reselect(asel.Callsign); // keep the selected flight plan current (edits by others)
      });
    }
  );
  return;
}

// ---- Concorde position reports ---------------------------------------------------------------------
function timeToMin(t: string): number {
  const d: string = (t ?? "").replace(/\D/g, "");
  return d.length === 4 ? parseInt(d.slice(0, 2)) * 60 + parseInt(d.slice(2)) : -1;
}

// The fix this Concorde is overdue to report at ("" if none): the first fix with no actual time whose estimate
// (typed, else calculated) passed more than 3 minutes ago. Estimates more than 90 minutes old are ignored.
export function concOverdueFix(ac: Aircraft): string {
  if (!ac.IsConc) return "";
  const n: Date = new Date();
  const nowMin: number = n.getUTCHours() * 60 + n.getUTCMinutes();
  for (let i: number = 0; i < ac.PosFixes.length; i++) {
    const fix: string = ac.PosFixes[i];
    const r = ac.PosReports[fix];
    if (r && r.act) continue; // already reported
    const est: number = timeToMin(r?.est || ac.PosCalc[i]);
    if (est < 0) continue; // no time to judge by
    const late: number = (((nowMin - est) % 1440) + 1440) % 1440;
    if (late > 720) return ""; // still in the future
    if (late > 90) continue; // stale
    return late > 3 ? fix : "";
  }
  return "";
}

export function concOverdueList(): string[] {
  const out: string[] = [];
  networkAircraft.forEach((m: Map<string, Aircraft>) => m.forEach((ac: Aircraft) => { if (concOverdueFix(ac) !== "") out.push(ac.Callsign); }));
  return out;
}

// ---- natTrak request alerts -------------------------------------------------------------------------
// A request is "pending" until a controller clears it on natTrak. New pending requests ping and flash
// until they are clicked (acknowledged) or cleared.
export let natTrakAcked: Set<string> = new Set();
const NATTRAK_SETTLE_MS = 60000;      // requests found in the first minute after opening the display are old ones: no flash, no ping
let natTrakStartedAt: number = 0;
let prevPending: Set<string> | null = null; // null until the first successful fetch (no ping for what is already there)

function readSetting(key: string, def: string): string {
  try { return localStorage.getItem(key) ?? def; } catch (e) { return def; }
}
function writeSetting(key: string, value: string): void {
  try { localStorage.setItem(key, value); } catch (e) {}
}
export let soundOn: boolean = readSetting("vnaaats.sound", "on") !== "off";   // audio alerts
export let visualOn: boolean = readSetting("vnaaats.visual", "on") !== "off"; // flashing / highlighting / tab title
export const tones: string[] = ["Ping", "Chime", "Beep"];
export let tone: string = tones.includes(readSetting("vnaaats.tone", "Ping")) ? readSetting("vnaaats.tone", "Ping") : "Ping";
let audioCtx: any = null;

// [frequency Hz, start offset s, length s, wave type]
const toneNotes: { [key: string]: [number, number, number, string][] } = {
  Ping: [[880, 0, 0.35, "sine"], [1320, 0.2, 0.35, "sine"]],
  Chime: [[660, 0, 0.3, "sine"], [880, 0.14, 0.3, "sine"], [1100, 0.28, 0.45, "sine"]],
  Beep: [[1000, 0, 0.18, "triangle"]],
};

export function playPing(): void {
  try {
    const AC = (window as any).AudioContext || (window as any).webkitAudioContext;
    if (!AC) return;
    audioCtx = audioCtx ?? new AC();
    if (audioCtx.state === "suspended") audioCtx.resume();
    const t0: number = audioCtx.currentTime;
    (toneNotes[tone] ?? toneNotes["Ping"]).forEach(([freq, delay, len, wave]) => {
      const osc = audioCtx.createOscillator();
      const gain = audioCtx.createGain();
      osc.type = wave;
      osc.frequency.value = freq;
      gain.gain.setValueAtTime(0.0001, t0 + delay);
      gain.gain.exponentialRampToValueAtTime(0.3, t0 + delay + 0.02);
      gain.gain.exponentialRampToValueAtTime(0.0001, t0 + delay + len);
      osc.connect(gain);
      gain.connect(audioCtx.destination);
      osc.start(t0 + delay);
      osc.stop(t0 + delay + len + 0.05);
    });
  } catch (e) {
    // audio blocked or unsupported - the flashing still works
  }
}

export function setSound(on: boolean): void {
  soundOn = on;
  writeSetting("vnaaats.sound", on ? "on" : "off");
  if (on) playPing(); // also unlocks audio in the browser
}

export function setVisual(on: boolean): void {
  visualOn = on;
  writeSetting("vnaaats.visual", on ? "on" : "off");
}

export function cycleTone(): void {
  tone = tones[(tones.indexOf(tone) + 1) % tones.length];
  writeSetting("vnaaats.tone", tone);
  playPing(); // preview the new tone (also unlocks audio)
}

// Opens natTrak in a small reusable pop-up window (must be called from a click). natTrak does not publish the
// request numbers, so this opens the pending list filtered to the request's track.
export function openNatTrak(track?: string): void {
  let url: string = "https://nattrak.vatsim.net/controllers/clx/pending";
  if (track) url += "?display[]=" + encodeURIComponent(track);
  if (appInApp()) { appNatTrakOpen().catch(() => void appOpenExternal(url)); return; }
  window.open(url, "natTrakWindow", "popup=yes,width=1000,height=900");
}

export function openNatTrakFor(callsign: string): void {
  const d = natTrakData.get(callsign.toUpperCase());
  openNatTrak(d?.nat);
}

export function ackNatTrak(callsign: string): void {
  natTrakAcked.add(callsign.toUpperCase());
  natTrakAcked = natTrakAcked;
}

// Aircraft that Bodo Oceanic (ENOB) controls or is working are not ours to clear: their natTrak requests are left out of PENDING.
export function isNatTrakIgnored(callsign: string): boolean {
  const ac: any = findAircraftLocal(callsign.toUpperCase());
  if (!ac) return false;
  return /^ENOB/i.test(ac.TrackedBy ?? "") || (ac.Comm?.kind === "oceanic" && /^ENOB/i.test(ac.Comm?.by ?? ""));
}

export function isNatTrakAlert(callsign: string): boolean {
  const cs: string = callsign.toUpperCase();
  const d = natTrakData.get(cs);
  return !!d && d.status === "PENDING" && !natTrakAcked.has(cs) && !isNatTrakIgnored(cs);
}

export async function natTrakFetch() : Promise<void> {
  try {
    const response: Response = await fetch(allNatTrakGet);
    if (response.status === 401) { natTrakData = new Map(); natTrakStartedAt = 0; prevPending = null; return; }
    const objArr: any[] = await response.json();
    if (!Array.isArray(objArr)) return;

    // Reset the list
    const fresh: Map<string, NatTrakData> = new Map();
    // Loop and assign
    for (let i: number = 0; i < objArr.length; i++) {
      const o = objArr[i];
      if (!o?.callsign) continue;
      // Mach arrives as "0.82"; the panel prints "M0" + 82
      const mach: number = Math.round(parseFloat(o.mach) * 100);
      // Clearance time arrives as an ISO timestamp; show it as 0811z
      let issued: string = o.clearance_issued ?? null;
      if (issued) {
        const d = new Date(issued);
        issued = isNaN(d.getTime())
          ? issued
          : String(d.getUTCHours()).padStart(2, "0") + String(d.getUTCMinutes()).padStart(2, "0") + "z";
      }
      fresh.set(String(o.callsign).toUpperCase(), {
        callsign: o.callsign,
        status: o.status,
        nat: o.nat,
        fix: o.fix,
        level: o.level,
        mach: isNaN(mach) ? 0 : mach,
        estimating_time: o.estimating_time,
        clearance_issued: issued,
        cleared_by: o.cleared_by ?? o.controller ?? o.issued_by ?? "",
        extra_info: o.extra_info,
      });
    }
    natTrakData = fresh;

    // Work out which requests are new since the last fetch
    const nowPending: Set<string> = new Set();
    fresh.forEach((v, k) => { if (v.status === "PENDING") nowPending.add(k); });
    let isNew: boolean = false;
    if (natTrakStartedAt === 0) natTrakStartedAt = Date.now();
    if (prevPending === null || Date.now() - natTrakStartedAt < NATTRAK_SETTLE_MS) {
      // Old requests (from before the controller opened the display, plus anything seen in the first minute) are marked as seen:
      // they stay in the PENDING box but do not flash or ping.
      nowPending.forEach((k) => natTrakAcked.add(k));
    } else if (prevPending !== null) {
      nowPending.forEach((k) => {
        if (!prevPending!.has(k)) {
          natTrakAcked.delete(k);
          isNew = true;
        }
      });
    }
    prevPending = nowPending;
    natTrakAcked.forEach((k) => { if (!nowPending.has(k)) natTrakAcked.delete(k); });
    if (isNew && soundOn) playPing();
  } catch (e) {
    // keep showing the last good data
  }
}

export async function parseOnlineControllers() : Promise<void> {
  let res: string = "";

  // Get the data
  await fetch(vatsimStatusData).then(function (response: Response) {
    response.text().then(function (text: string) {
        // Parse json
        res = text;
        const objArr: any[] = JSON.parse(res);

        // Reset the list
        natTrakData = new Map();
        // Loop and assign
        for(let i: number = 0; i < objArr.length; i++) {
          objArr[i].mach = objArr[i].mach * 100;
          natTrakData.set(objArr[i].callsign, objArr[i]);
        }
    })
  });
}

export function runDataFetcher() : void {
  console.log("Crons starting...");

  // First population
  parseNatTracks();
  populateAllAircraft();
  natTrakFetch();
  
  // Aircraft & natTrak cron (run every 5 seconds)
  acUpdateJob = setInterval(() => {
    populateAllAircraft();
    natTrakFetch();
  }, 5000);

  /* NB: We need to do the cron with each increment of 5 otherwise the methods run several times */
  // Tracks cron (run every 5 seconds)
  tkUpdateJob = setInterval(() => {
    parseNatTracks();
  }, 300000);
}

// Look up one aircraft by callsign (for the aircraft menu)
export function getAircraft(callsign: string): Aircraft | null {
  return findAircraftLocal(callsign) ?? null;
}

// Minutes from now to the first and last route estimate (negative = already past). null when there are no times.
export function etaSpan(ac: Aircraft): { entry: number; exit: number } | null {
  const n: Date = new Date();
  const nowMin: number = n.getUTCHours() * 60 + n.getUTCMinutes();
  const offs: number[] = [];
  const times: string[] = ac.IsConc ? ac.PosCalc : ac.RouteEtas;
  for (const t of times ?? []) {
    const m: number = timeToMin(t);
    if (m < 0) continue;
    offs.push((((m - nowMin + 720) % 1440) + 1440) % 1440 - 720);   // -720..+719, copes with midnight
  }
  return offs.length === 0 ? null : { entry: Math.min(...offs), exit: Math.max(...offs) };
}

export const NO_TIME_RANGE_NM: number = 200;

// Strip is wanted on the display: within `ahead` minutes of the oceanic entry, or less than `behind` minutes past the exit.
// Flights with no times, the selected flight, CPDLC-connected flights and pending natTrak requests always show.
// Flights an oceanic controller (CZQO, EGGX, NAT_FSS ...) is tracking stay on the display until untracked, then for OCEANIC_GRACE_MIN more minutes.
const OCEANIC_PREFIX = /^(CZQO|CZQX|EGGX|NAT|BIRD|KZWY|LPPO|ENOB)/i;
const OCEANIC_GRACE_MIN = 15;   // after handoff / untracked an oceanic flight stays (faded) this long
const oceanicSeen = new Map<string, number>();   // callsign -> last time (ms) an oceanic controller was tracking it
function oceanicHold(ac: Aircraft): boolean {
  const cs = ac.Callsign.toUpperCase();
  const tracked = (!!ac.TrackedBy && ac.TrackedBy !== "NATTRAK" && OCEANIC_PREFIX.test(ac.TrackedBy)) || (ac.Comm?.kind === "oceanic");
  if (tracked) { oceanicSeen.set(cs, Date.now()); return true; }
  const last = oceanicSeen.get(cs);
  return last !== undefined && Date.now() - last < OCEANIC_GRACE_MIN * 60000;
}

// --- single-position view: a Gander (CZQO_) or Shanwick (EGGX_) controller also sees the neighbour's traffic that is
// NEIGHBOUR_MIN or less from 30W, the boundary between the two. Anything else (NAT_FSS ...) sees everything.
export type Scope = "all" | "gander" | "shanwick";
export const NEIGHBOUR_MIN = 30;
export const FADE_KEEP_MIN = 15;
export function scopeForPosition(position: string | undefined | null): Scope {
  const p = (position ?? "").toUpperCase();
  if (p.startsWith("CZQO_") || p.startsWith("CZQX_")) return "gander";
  if (p.startsWith("EGGX_")) return "shanwick";
  return "all";
}

// Minutes from now until the flight is estimated at 30W (negative = already across). null = no 30W point or no time for it.
export function minutesTo30W(ac: Aircraft): number | null {
  const route: string[] = ac.Route ?? [];
  const times: string[] = ac.RouteEtas ?? [];
  const i = route.findIndex((r) => /^\d{2,4}(\.\d)?N?\/30(W)?$/i.test(String(r).trim()));
  if (i < 0 || i >= times.length) return null;
  const m: number = timeToMin(times[i]);
  if (m < 0) return null;
  const n: Date = new Date();
  const nowMin: number = n.getUTCHours() * 60 + n.getUTCMinutes();
  return (((m - nowMin + 720) % 1440) + 1440) % 1440 - 720;
}

// Where does the flight sit for a single-position controller? "own" = in your OCA, "neighbour" = the other OCA but
// close to 30W, "gone" = already handed over across 30W (kept faded for FADE_KEEP_MIN), "far" = neighbour's, not yet close.
function scopeState(ac: Aircraft, scope: Scope): "own" | "neighbour" | "gone" | "far" {
  if (scope === "all") return "own";
  const m = minutesTo30W(ac);
  if (m === null) return "own";
  const eb: boolean = !!ac.Direction;
  // Gander owns eastbound flights until 30W and westbound ones after it; Shanwick the other way round.
  const mineBefore: boolean = scope === "gander" ? eb : !eb;       // my flights are on my side of 30W while m > 0
  if (mineBefore) return m > 0 ? "own" : (m >= -FADE_KEEP_MIN ? "gone" : "far");
  return m > NEIGHBOUR_MIN ? "far" : (m > 0 ? "neighbour" : "own");
}

// What the list does with one flight: show it or not, and whether to draw it faded (handed off / past its exit point).
export function isTangoTrack(track: string): boolean {
  return !!currentNatTracks.get(track)?.Tango;
}

export function trafficView(ac: Aircraft, ahead: number, behind: number, scope: Scope): { show: boolean; faded: boolean } {
  const pinned = (asel != null && asel.Callsign === ac.Callsign) || ac.Cpdlc || ac.Manual;
  const tracked = (!!ac.TrackedBy && ac.TrackedBy !== "NATTRAK" && OCEANIC_PREFIX.test(ac.TrackedBy)) || (ac.Comm?.kind === "oceanic");
  // Tango routes are Shanwick's: a Gander position does not list them (unless it is tracking, selected, or on CPDLC)
  if (scope === "gander" && isTangoTrack(ac.Track) && !pinned && !tracked) return { show: false, faded: false };
  const st = scopeState(ac, scope);
  if (!pinned && !tracked && (st === "far")) return { show: false, faded: false };
  if (!inTrafficWindow(ac, ahead, behind) && !(st === "gone" || st === "neighbour")) return { show: false, faded: false };
  const sp = etaSpan(ac);
  const past: boolean = !!sp && sp.exit < 0;
  const handedOff: boolean = !tracked && oceanicSeen.has(ac.Callsign.toUpperCase());
  return { show: true, faded: st === "gone" || past || handedOff };
}

export function inTrafficWindow(ac: Aircraft, ahead: number, behind: number): boolean {
  if (asel != null && asel.Callsign === ac.Callsign) return true;
  if (ac.Cpdlc || ac.Manual) return true;
  if (oceanicHold(ac)) return true;
  if (natTrakData.get(ac.Callsign.toUpperCase())?.status === "PENDING") return true;
  if (ac.Offline === true) return false;     // not on the VATSIM network any more
  const sp = etaSpan(ac);
  // No times at all: use how far the pilot is from the route (VATSIM feed). Unknown position = show.
  if (!sp) return ac.DistNm == null || ac.DistNm <= NO_TIME_RANGE_NM;
  return sp.entry <= ahead && sp.exit >= -behind;
}


// Contact me was sent from the FDD: "" (nothing), "yellow" (sent, give the pilot time) or "red" (20 minutes on, still nobody
// tracking and the flight is still in oceanic airspace). Goes away once an oceanic controller tracks the flight.
export const CONTACT_ME_RED_MIN = 20;
export function contactMeState(ac: Aircraft): string {
  const at: number = ac.ContactMeAt ?? 0;
  if (!at) return "";
  const ageMin: number = (Date.now() - at) / 60000;
  if (ageMin < 0 || ageMin > 240) return "";
  const tb: string = ac.TrackedBy ?? "";
  if (ac.Comm?.kind === "oceanic" || (tb && tb !== "NATTRAK" && OCEANIC_PREFIX.test(tb))) return "";
  if (ageMin < CONTACT_ME_RED_MIN) return "yellow";
  const sp = etaSpan(ac);
  if (sp && (sp.entry > 0 || sp.exit < 0)) return "yellow";   // not in oceanic yet, or already out of it
  return "red";
}
