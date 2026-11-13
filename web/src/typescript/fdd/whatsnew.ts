/*
 *  "What's new" notes shown once per announced release, after sign-in. Add a NEW entry at the TOP of NOTES for a release worth announcing
 *  (a short list in plain words for controllers); the id is what is remembered per computer. Releases not listed here are not announced.
 */
import { writable } from "svelte/store";

export type Note = { id: string; date: string; items: { title: string; text: string; action?: "vacs" | "windows" }[] };

export const NOTES: Note[] = [
  {
    id: "2.9.0",
    date: "13 November 2026",
    items: [
      { title: "Voice position reports for any flight", text: "Not only Concorde. In the aircraft menu, press \"Voice reports\" for a flight that reports by voice (for example one that is not ADS equipped). It then gets a Position reports window where you type each estimate and actual time, and the strip flags it when a report is overdue. It switches itself off after 18 hours." },
      { title: "Sector pack installs itself", text: "FDD for Windows now checks for a new vNAAATS sector pack and installs it for you (close EuroScope first). Your own profile, screen layout and voice files are kept." },
      { title: "New start screen", text: "Nothing opens until the checks are done. You see whether the plugin, airline list, sector pack and your API key are up to date, then choose Yes to open EuroScope, VACS and your audio client. If your key has expired the app tells you and takes you to get a new one." },
      { title: "All systems go", text: "FDD for Windows now shows a quick ALL SYSTEMS GO screen for 5 seconds when it opens, so you can see the server, VATSIM data and tracks are healthy before anything starts. If something is wrong it tells you." },
      { title: "Clearer errors", text: "If something goes wrong the app now says what to do in plain words, with a Copy details link you can paste into Discord." },
      { title: "Server status page", text: "vnaaats.com/status shows whether the server, VATSIM data and tracks are working, how many flights and aircraft on CPDLC there are, and which oceanic ATC is online. Check it first if something looks wrong." },
      { title: "Smaller changes", text: "The squawk option is gone from vnaaats.cfg (the app tidies your file), the Yes button now opens every program that is not already running, and the user manual explains voice reports." },
    ],
  },
  {
    id: "2.8.0",
    date: "10 November 2026",
    items: [
      { title: "Oceanic clearance (RCL) follows NAT Doc 007", text: "An RCL is a request, not a clearance. It is answered \"RCL RECEIVED BY\" your area, then \"FLY CURRENT FLIGHT PLAN OR AS AMENDED BY ATC. SQUAWK 2000 AT THE OCEANIC ENTRY POINT\" when you process it. The request shows the MAX FL, and there are quick buttons for Resume normal speed and Contact by voice." },
      { title: "Automated entry message", text: "The CPDLC contact message is sent once per area, and again at 30W only on NAT_FSS (NATX)." },
      { title: "Handoff helper", text: "A reminder, not a way to take control. Both handoff buttons are always shown, and there is no prompt at 30W when a combined NAT_FSS position is online." },
      { title: "Auto RCL history and VACS calls", text: "Auto RCL has a History list. VACS shows who is calling, with a call log and a ring tone you can turn off." },
      { title: "New training videos", text: "Videos 6 (the RCL process) and 7 (the handoff helper) are on the Training page, and videos 1 to 5 and the overview have been re-recorded." },
    ],
  },
  {
    id: "2.7.0",
    date: "6 November 2026",
    items: [
      { title: "Auto RCL sends a telex first", text: "If the pilot is online on Hoppie, the oceanic clearance reminder goes as a telex (no CPDLC logon needed). If they have not asked 3 minutes later, or are not on Hoppie, the EuroScope message is sent as before. Aircraft on the ground or below 10,000 ft are never reminded. One controller must be signed in to CPDLC for the telex." },
      { title: "Contact Me works from the FDD", text: "The plugin now clicks the callsign on the tag for you, checks that the click landed, and tells you if Contact Me was not sent. Needs the new plugin (2.7.2)." },
      { title: "Aircraft menu keeps natTrak visible", text: "The menu opens over the strips only, so the flight plan and natTrak boxes stay in view." },
      { title: "natTrak Mach", text: "Prepare in natTrak now fills the Mach as .85, which natTrak accepts." },
      { title: "Smaller changes", text: "A green dot shows when CPDLC is online, the header is tidier, keys pasted with key= or quotes are accepted, and there is a new Get access page and a short video on the home page." },
    ],
  },
  {
    id: "2.6.0",
    date: "1 November 2026",
    items: [
      { title: "Tidier top of the display", text: "Change position, Relief and Sign out are now in one Position menu. VACS, CPDLC, Auto RCL and the alerts count are buttons at the top right, and Show, Group, Sort and one Search or add box are on the second row." },
      { title: "VACS call alert", text: "A headphones button flashes and shows a pop-up when someone calls you in VACS, with Accept and Dismiss. It needs 'Enable remote control' ticked in VACS (Advanced Settings).", action: "vacs" },
      { title: "FDD for Windows", text: "The new Windows app keeps the plugin, airline list and font up to date for you, saves your key into vnaaats.cfg, and opens EuroScope, VACS and your audio client when you choose your position. No more manual .dll downloads or .cfg editing.", action: "windows" },
      { title: "CPDLC window", text: "Connected aircraft are a list on the left, the window can be made wider or narrower, and the standard messages are grouped (Level, Contact, Replies)." },
      { title: "Auto RCL asks first", text: "The reminders to pilots are no longer typed without warning. The controller who sends them is asked first, with the list of aircraft and their entry times, and is told to keep EuroScope in front until they are sent. At most every 5 minutes." },
      { title: "Untracked aircraft", text: "Aircraft whose controller has logged off now show as untracked, so the menu offers Track instead of Update." },
    ],
  },
];

const KEY = "vnaaats.whatsNewSeen";
export const whatsNewOpen = writable<boolean>(false);   // set true to show the latest notes on demand (Settings > What's new)
export function latest(): Note { return NOTES[0]; }
export function seenId(): string { try { return localStorage.getItem(KEY) ?? ""; } catch (e) { return ""; } }
export function markSeen(): void { try { localStorage.setItem(KEY, latest().id); } catch (e) { /* ignore */ } }
export function unseen(): boolean { return seenId() !== latest().id; }
