/*
 * Defines the objects used in the app
 * Created by Andrew Ogden
 */
export type Aircraft = {
  Callsign: string;
  Type: string;
  AssignedLevel: number;
  AssignedMach: number;
  Track: string;
  Route: string[];
  RouteEtas: string[];
  Departure: string;
  Arrival: string;
  Etd: string;
  Selcal: string;
  Datalink: boolean;
  SectorID: string;
  Direction: boolean;
  IsEquipped: boolean;
  TrackedBy: string;
  State: string;
  Relevant: boolean;
  TargetMode: number;
  LastUpdated: number;
  Comm: { kind: string; by: string; freq: string } | null; // who the pilot is tuned to on VATSIM: oceanic / domestic controller or UNICOM
  Scratch: string; // scratchpad note (same as the EuroScope scratchpad)
  Manual: boolean; // added by hand on the FDD (stays on the display)
  ContactMeAt?: number; // ms: when a .contactme was sent from the FDD (0 = never)
  NtSent: boolean; // natTrak reminder sent to the pilot (red star) - false once they request or an oceanic controller tracks
  Cpdlc: boolean; // connected to the CPDLC station
  Handoff?: { facility: string; fix: string; nm: number; name: string; soon: boolean; controller: string; frequency: string; code?: string; unicom: boolean; online?: { callsign: string; code: string; frequency: string }[] } | null; // next boundary, distance and who is online to take it
  DistNm: number | null; // nm from the pilot to the nearest route fix (VATSIM feed), null if unknown
  Offline: boolean | null; // true = the VATSIM feed is working and this pilot is not on it
  Entry: string; // first and last named/lat-lon point of the route in flight order (before any display reversal)
  Exit: string;
  EntryEta: number; // minutes since midnight at the first route point, -1 if unknown
  UpdatedBy: string; // who last changed level / Mach / track / SELCAL / route (position + name, or plugin callsign)
  UpdatedAt: number; // epoch ms, 0 if unknown
  History: { at: string; by: string; changes: Record<string, [any, any]> }[];
  IsConc: boolean; // Concorde: reports by voice, controllers type its times in
  PosFixes: string[]; // route fixes in flight order (position report rows)
  PosCalc: string[]; // calculated times for those fixes, same order
  PosReports: Record<string, { est: string; act: string; by: string; at: string }>;
  Source?: string; // 'plugin' = posted by a controller, 'vatsim' = filed flight plan from the VATSIM data feed
};

export type NatTrakData = {
  callsign: string;
  status: string;
  nat: string;
  fix: string;
  level: number;
  mach: number;
  estimating_time: string;
  clearance_issued: string;
  cleared_by?: string; // only if natTrak says who processed it
  extra_info: string;
}

export type Waypoint = {
  name: string;
  latitude: number;
  longitude: number;
};

export enum TrackDirection {
  Unknown = 0,
  Westbound = 1,
  Eastbound = 2,
}

export type NatTrack = {
  Identifier: string;
  TMI: string;
  Route: Waypoint[];
  FlightLevels: number[];
  Direction: number;
  ValidTo: number;
  ValidFrom: number;
  Supersonic?: boolean; // Concorde track (SM, SN, SO, SP)
  Tango?: boolean; // Shanwick Tango route (T9, T290, T213, T13, T16)
  TangoDir?: string; // "north" | "south" | "both"
  BlockLevel?: string; // e.g. FL470-600
  Use?: string; // e.g. westbound only
};
