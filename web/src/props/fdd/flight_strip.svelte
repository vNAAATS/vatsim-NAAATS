<script lang="ts">
  import { circleInEs } from "../../typescript/fdd/acmenu";
  import { espicked, espickN } from "../../typescript/fdd/esselect";
  import * as utils from "../../typescript/fdd/utils";
  import * as dataHandler from "../../typescript/fdd/data";
  import { openMenu } from "../../typescript/fdd/acmenu";
  import { talker, highlightOn, scrollOn } from "../../typescript/fdd/trackaudio";

  // Elements for spread prop
  export let callsign: string;
  export let type: string;
  export let assignedLevel: number;
  export let blockLevel: string = ""; // on a Concorde track: the block level (e.g. FL470-600) replaces the single level
  export let assignedMach: number;
  export let track: string;
  export let route: string[];
  export let routeEtas: string[];
  export let departure: string;
  export let arrival: string;
  //export let selcal: string;
  //export let datalink: boolean;
  export let direction: boolean;
  export let isEquipped: boolean;
  export let tango: string = "";       // Shanwick Tango route id (T9 ...): the fixes row is shown with the id, like random routings
  export let faded: boolean = false;   // handed off / past its exit point: drawn dimmed
  export let alert: boolean = false; // natTrak request waiting for action: flash
  //export let state: string;
  //export let relevant: boolean;
  export let trackedBy: string;
  export let cols: number = 6;          // number of fix columns in the list (same for every row so everything lines up)
  export let comm: { kind: string; by: string; freq: string } | null = null;   // VATSIM radio: oceanic / domestic / unicom
  // Headset icon: bold green = oceanic controller, faded green = domestic controller, orange = UNICOM 122.800.
  // Taken from the frequency the pilot has tuned on VATSIM; if that is not known, falls back to who is tracking in EuroScope.
  $: radio = radioInfo(comm, trackedBy);
  function radioInfo(c: { kind: string; by: string; freq: string } | null, tb: string) {
    let kind = c?.kind ?? "", by = c?.by ?? "", freq = c?.freq ?? "";
    if (kind === "none") kind = "";
    if (!kind && tb && tb !== "NATTRAK") {
      by = tb;
      kind = /^(CZQO|CZQX|EGGX|NAT|BIRD|KZWY|LPPO|ENOB)/i.test(tb) ? "oceanic" : "domestic";
    }
    if (kind === "oceanic") return { tip: "Tracked by " + by + (freq ? " (" + freq + ")" : ""), color: "#15803d", opacity: 1, bold: 1.2 };
    if (kind === "domestic") return { tip: "Tracked by " + by + (freq ? " (" + freq + ")" : "") + " - domestic", color: "#16a34a", opacity: 0.5, bold: 0 };
    if (kind === "unicom") return { tip: "On UNICOM 122.800", color: "#c2410c", opacity: 1, bold: 0.6 };
    if (c?.kind === "none") return { tip: "Not tracked by any controller (not on UNICOM either)", color: "#dc2626", opacity: 1, bold: 0, none: true };
    return null;
  }
  export let cmAt: number = 0;           // when contact me was sent (ms)
  export let cmState: string = "";       // contact me sent: "yellow", or "red" after 20 minutes with nobody tracking
  export let handoff: { fix: string; nm: number; name: string; soon: boolean; controller: string; frequency: string; code?: string; unicom: boolean } | null = null;   // next boundary
  export let ntSent: boolean = false;    // natTrak reminder sent: red star
  export let cpdlc: boolean = false;      // connected to CPDLC: callsign shown as [BAW123]
  export let cpdlcAlert: boolean = false; // unread CPDLC message: flash
  export let concOverdue: boolean = false; // Concorde overdue a position report: flash
  export let warning: string = "";   // conflict / sanity warning text ("" = none)
  export let updatedBy: string = "";
  export let updatedAt: number = 0;

  // A pencil shows for 10 minutes after someone changed level / Mach / track / SELCAL
  // RCL (oceanic clearance) processed on natTrak: when, and by whom if natTrak says
  $: clearanceTip = (() => {
    const nt: any = dataHandler.natTrakData.get(callsign.toUpperCase());
    const when: string = nt?.clearance_issued ? " at " + nt.clearance_issued : "";
    const who: string = nt?.cleared_by ? " by " + nt.cleared_by : "";
    return "RCL processed" + when + who + " (natTrak)";
  })();
  $: recentlyChanged = updatedBy !== "" && updatedAt > 0 && Date.now() - updatedAt < 10 * 60 * 1000;
  $: tip = [warning ? "\u26A0 " + warning : "", updatedBy ? "Last change by " + updatedBy + " at " + new Date(updatedAt).toISOString().slice(11, 16) + "z" : ""].filter((x) => x).join("\n");
  //export let targetMode: number;
  //export let sectorId: string;

  function cmZ(ms: number): string { const d = new Date(ms); return String(d.getUTCHours()).padStart(2, "0") + String(d.getUTCMinutes()).padStart(2, "0") + "z"; }

  let colour: string = direction ? "bg-blue-10" : "bg-yellow-100";
  let darkerColour: string = direction ? "bg-blue-9" : "bg-yellow-200";
  let opacity: string = "bg-opacity-75";

  // Opacity
  let isActive: boolean = false;
  function stripMouseDown(): void {
    isActive = true;
  }
  // TrackAudio: this aircraft is transmitting (solid green box) or just stopped (the box fades out)
  $: sameTalker = $talker.who !== "" && $talker.who === callsign.toUpperCase();
  $: isTalker = $highlightOn && sameTalker;
  $: talking = isTalker && $talker.live;
  let stripEl: HTMLElement;
  // bring the row into view when this aircraft starts transmitting (nearest: does nothing if it is already visible)
  $: if (sameTalker && $talker.live && $scrollOn && stripEl) stripEl.scrollIntoView({ behavior: "smooth", block: "nearest" });
  function stripMouseUp(): void {
    isActive = false;
  }
</script>

<div bind:this={stripEl} id={"strip-" + callsign} style={faded ? "opacity: 0.45;" : ""} class={"hover:border-green-500 hover:border-2" + (isTalker ? " ta-box" : "") + (talking ? " ta-live" : "") + ($espicked === callsign ? ($espickN % 2 ? " es-pick-a" : " es-pick-b") : "")} title={tip}
  on:click={() => {
    if (dataHandler.isNatTrakAlert(callsign)) dataHandler.openNatTrakFor(callsign); // flashing request: open natTrak
    dataHandler.ackNatTrak(callsign);
    dataHandler.setAsel(callsign);
    circleInEs(callsign);
  }}>
  {#if track === "RR" || tango}
    <div class="flex flex-col items-center justify-end w-full h-8 {darkerColour} {isActive ? 'bg-opacity-75' : ''} text-black hover:cursor-default"
      on:mousedown={stripMouseDown}
      on:mouseup={stripMouseUp}
      on:mouseleave={stripMouseUp}>
      <div class="strip-grid w-full" style="--cols: {cols};">
        <div style="grid-column: 1 / span 4;" class="pl-2 font-bold">{tango}</div>
        {#each route as rte}
          <div class="select-none hover:cursor-default whitespace-nowrap overflow-hidden">{rte}</div>
        {/each}
      </div>
    </div>
  {/if}
  <div class="strip-grid w-full h-8 {colour} {isActive ? 'bg-opacity-75' : ''} {alert || cpdlcAlert || concOverdue ? 'nt-flash' : ''} text-black hover:cursor-default" style="--cols: {cols};"
    on:mousedown={stripMouseDown}
    on:mouseup={stripMouseUp}
    on:mouseleave={stripMouseUp}>
    <div class="flex flex-row items-center whitespace-nowrap overflow-hidden pl-2 hover:cursor-pointer select-none {cmState === 'yellow' ? 'bg-yellow-300' : cmState === 'red' ? 'bg-red-600 text-white' : ''}" title={cmState === "yellow" ? "Contact me sent at " + cmZ(cmAt) + " - waiting for the pilot" : cmState === "red" ? "Contact me sent at " + cmZ(cmAt) + " (over 20 minutes ago) and nobody is tracking this flight" : "Click for the aircraft menu"} on:click|stopPropagation={() => { dataHandler.setAsel(callsign); dataHandler.ackNatTrak(callsign); openMenu(callsign); }}>
      {(cpdlc ? "[" + callsign + "]" : callsign) + (!isEquipped ? "*" : "")}{#if handoff && handoff.soon}<span class="ml-1 px-1 font-bold bg-yellow-300 text-black border border-black" style="font-size: 0.7em; line-height: 1.3;" title={"Hand off soon: " + handoff.nm + " nm to " + handoff.fix + ", next is " + handoff.name + (handoff.unicom ? " (nobody online: UNICOM)" : " - " + handoff.controller + " " + handoff.frequency)}>HO {handoff.nm}nm &rarr; {handoff.unicom ? "UNICOM" : (handoff.code || handoff.controller)}</span>{/if}{#if ntSent}<span class="text-red-600 font-bold pl-0.5" style="font-size: 1.5em; line-height: 0.6;" title="natTrak reminder sent to the pilot">*</span>{/if}{#if warning}<span class="text-red-600 pl-1">&#9888;</span>{/if}{#if recentlyChanged}<span class="pl-1">&#9998;</span>{/if}
    </div>
    <div class="whitespace-nowrap overflow-hidden select-none" style="font-size: 0.88em;">{departure}/{arrival}</div>
    <div class="select-none" title={blockLevel ? "Block level " + blockLevel : ""}>{blockLevel ? "BLK" : assignedLevel}</div>
    <div class="select-none">{assignedMach > 0 ? "M" + String(assignedMach).padStart(3, "0") : ""}</div>
    {#each Array(cols) as _, i}
      <div class="select-none">{routeEtas[i] !== undefined && routeEtas[i] != "--" ? routeEtas[i] : ""}</div>
    {/each}
    <div class="whitespace-nowrap select-none">{type}</div>
    <div class="flex items-center justify-center select-none">
        {#if trackedBy === "NATTRAK"}
          <span title={clearanceTip} class="inline-flex" aria-label="Clearance processed on natTrak">
            <svg viewBox="0 0 24 24" width="20" height="20" fill="currentColor"><path d="M19 3h-4.2A3 3 0 0 0 12 1a3 3 0 0 0-2.8 2H5a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V5a2 2 0 0 0-2-2zm-7 0a1 1 0 1 1 0 2 1 1 0 0 1 0-2zm-2 14-4-4 1.4-1.4L10 14.2l6.6-6.6L18 9z" /></svg>
          </span>
        {:else if radio}
          <span title={radio.tip} class="inline-flex" style={"color: " + radio.color + "; opacity: " + radio.opacity + ";"} aria-label={radio.tip}>
            <svg viewBox="0 0 24 24" width="20" height="20" fill="currentColor" stroke="currentColor" stroke-width={radio.bold}><path d="M12 2a9 9 0 0 0-9 9v7a3 3 0 0 0 3 3h1v-8H5v-2a7 7 0 0 1 14 0v2h-2v8h2v1a2 2 0 0 1-2 2h-3v2h3a4 4 0 0 0 4-4V11a9 9 0 0 0-9-9z" />{#if radio.none}<path d="M3 3 21 21" stroke="currentColor" stroke-width="2.4" fill="none" />{/if}</svg>
          </span>
        {/if}
    </div>
  </div>
</div>
<style>
  /* TrackAudio talker box: solid while the aircraft is transmitting, then fades out over a couple of seconds */
  :global(.ta-box) { position: relative; }
  :global(.ta-box)::before {
    content: ""; position: absolute; inset: 0; pointer-events: none; z-index: 6;
    border: 4px solid #5cff7a; box-shadow: 0 0 10px #5cff7a, inset 0 0 6px #5cff7a;
    opacity: 0; transition: opacity 2.5s ease-out;
  }
  :global(.ta-live)::before { opacity: 1; transition: none; }
  /* Aircraft picked in EuroScope: a thick black ring with a white inner edge (stands out on blue and on yellow strips)
     that shrinks to a thin ring and stays. It is drawn over the strip (the strip's own colour would hide a shadow).
     Two identical animations alternate so a repeat click pulses again. */
  :global(.es-pick-a), :global(.es-pick-b) { position: relative; }
  :global(.es-pick-a)::after, :global(.es-pick-b)::after {
    content: ""; position: absolute; inset: 0; pointer-events: none; z-index: 5;
    border: 2px solid #000; box-shadow: inset 0 0 0 1px #fff;
  }
  :global(.es-pick-a)::after { animation: es-pulse-a 1.6s ease-out; }
  :global(.es-pick-b)::after { animation: es-pulse-b 1.6s ease-out; }
  @keyframes es-pulse-a {
    0%, 35% { border-width: 5px; box-shadow: inset 0 0 0 2px #fff; }
    100%    { border-width: 2px; box-shadow: inset 0 0 0 1px #fff; }
  }
  @keyframes es-pulse-b {
    0%, 35% { border-width: 5px; box-shadow: inset 0 0 0 2px #fff; }
    100%    { border-width: 2px; box-shadow: inset 0 0 0 1px #fff; }
  }

  /* One grid for every strip and the header rows, so the columns always line up and scale with the list font.
     Columns: callsign, dep/arr, level, Mach, <cols> fix times, aircraft type, tracking icon. */
  :global(.strip-grid) {
    display: grid;
    grid-template-columns: 7.4em 5.8em 2.5em 3.2em repeat(var(--cols, 6), minmax(2.9em, 1fr)) 3.2em 1.8em;
    align-items: center;
  }
</style>
