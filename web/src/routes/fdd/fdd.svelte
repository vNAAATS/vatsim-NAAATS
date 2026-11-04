<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import * as utils from "../../typescript/fdd/utils";
  import Button from "../../props/fdd/button.svelte";
  import Checkbox from "../../props/fdd/checkbox.svelte";
  import FlightStrip from "../../props/fdd/flight_strip.svelte";
  import { settingsOpen } from "../../typescript/fdd/settings";
  import { reliefOpen } from "../../typescript/fdd/relief";
  import * as acmenu from "../../typescript/fdd/acmenu";
  import * as dataHandler from "../../typescript/fdd/data";
  import * as site from "../../typescript/site";
  import * as sess from "../../typescript/fdd/session";
  import { hidden, unhideStrip } from "../../typescript/fdd/hide";
  import * as cp from "../../typescript/fdd/cpdlc";
  import * as app from "../../typescript/fdd/app";
  import * as esSel from "../../typescript/fdd/esselect";
  import { espicked } from "../../typescript/fdd/esselect";
  import VacsChip from "./vacs_chip.svelte";
  import AudioChip from "./audio_chip.svelte";
  import RclPanel from "./rcl_panel.svelte";

  const { session } = sess;
  const { cpdlcStatus, cpdlcOpen, cpdlcAttention } = cp;
  const { rclPanelOpen } = sess;
  $: rclAsk = $rclAuto.prompt && ($session.position ?? "").toUpperCase() === $rclAuto.sender;
  $: cpdlcUnread = cp.unreadTotal($cpdlcStatus);

  // Initialisation of data
  let isLoading: boolean = true;
  let noData: boolean = true;
  // natTrak sign-in (FDD for Windows): done from here once the controller is online on VATSIM, so natTrak sees them on the network first.
  const natApp = app.inApp();
  let natSigned = false;
  let natTimer: any = null;
  async function natCheck(): Promise<void> { if (!natSigned) natSigned = await app.natTrakSignedIn(); }
  function natClick(): void {
    if (natApp) app.natTrakWindow(!natSigned).catch(() => dataHandler.openNatTrak());
    else dataHandler.openNatTrak();
  }
  onMount(async () => {
    if (natApp) { void natCheck(); natTimer = setInterval(natCheck, 2000); }
    // Bye bye homepage
    site.setSiteSection(2);

    // Start the fetcher
    dataHandler.runDataFetcher();
    cp.startCpdlcPolling();
    esSel.startEsSelect();
    sess.refreshRclAuto();
    rclTimer = setInterval(sess.refreshRclAuto, 5000);

    // Title
    site.setTitle("Flight Data Display");

  });

  let rclTimer: any = null;
  let rclMsg: string = "";
  const { rclAuto } = sess;
  async function toggleRcl(): Promise<void> {
    rclMsg = await sess.setRclAuto(!$rclAuto.on);
    if (rclMsg) setTimeout(() => (rclMsg = ""), 6000);
  }
  onDestroy(async () => {
    if (natTimer) clearInterval(natTimer);
    if (rclTimer) clearInterval(rclTimer);
    esSel.stopEsSelect();
    // Hello homepage
    site.setSiteSection(0);
    clearInterval(dataHandler.acUpdateJob);
    clearInterval(dataHandler.tkUpdateJob);
    cp.stopCpdlcPolling();
  });

  // Track filter for the strip list (ALL TK / RR)
  let filter: string = "ALL";
  // Filter tick boxes. Nothing ticked = show everything. Different groups narrow each other (Tracked AND direction AND level);
  // within a group any ticked box counts (EB or WB, HL or LL), so EB + WB shows both directions.
  let showMine: boolean = false;
  let showEb: boolean = false;
  let showWb: boolean = false;
  let showRr: boolean = false;      // RR: random routings only
  let showTango: boolean = false;   // TANGO: Shanwick Tango routes only
  function typeMatch(ac: any, track: string, mine: boolean, eb: boolean, wb: boolean, rr: boolean, tg: boolean): boolean {
    if (mine) {
      const me: string = ($session.position ?? "").toUpperCase();
      if (!me || (ac.TrackedBy ?? "").toUpperCase() !== me) return false;
    }
    if ((eb || wb) && !((eb && ac.Direction) || (wb && !ac.Direction))) return false;
    if ((rr || tg) && !((rr && track === "RR") || (tg && dataHandler.isTangoTrack(track)))) return false;
    return true;
  }
  // Search, grouping and sorting
  let search: string = "";
  let groupBy: string = "TRACK";   // TRACK | ENTRY
  // Traffic window: only flights within 60 min of the oceanic entry, until 15 min after the exit ("ALL" shows everything)
  const WIN_AHEAD: number = 60;
  const WIN_BEHIND: number = 15;   // minutes past the exit point (or the handoff) a flight stays, faded
  let windowMode: string = "WINDOW";
  try { if (localStorage.getItem("vnaaats.window") === "ALL") windowMode = "ALL"; } catch (e) { /* ignore */ }
  function setWindowMode(v: string): void { windowMode = v; try { localStorage.setItem("vnaaats.window", v); } catch (e) { /* ignore */ } }
  let sortBy: string = "DEFAULT";  // DEFAULT | ETA | CALLSIGN | LEVEL | UPDATED
  function searchMatch(ac: any, s: string): boolean {
    const q = s.trim().toUpperCase();
    if (q === "" || isPicked(ac)) return true;
    return [ac.Callsign, ac.Departure, ac.Arrival, ac.Track, ac.Entry, ac.Exit, ac.Type].some((v: string) => (v ?? "").toUpperCase().includes(q));
  }
  function sorter(by: string): (a: any, b: any) => number {
    switch (by) {
      case "ETA": return (a, b) => (a.EntryEta < 0 ? 9999 : a.EntryEta) - (b.EntryEta < 0 ? 9999 : b.EntryEta);
      case "CALLSIGN": return (a, b) => a.Callsign.localeCompare(b.Callsign);
      case "LEVEL": return (a, b) => b.AssignedLevel - a.AssignedLevel;
      case "UPDATED": return (a, b) => b.UpdatedAt - a.UpdatedAt;
      default: return () => 0;
    }
  }
  type Group = { key: string; label: string; track: string; rows: any[]; kind?: string };
  function buildGroups(_tick: number, _win: string, f: string, mine: boolean, eb: boolean, wb: boolean, rr: boolean, tg: boolean, s: string, g: string, by: string, _hid: Record<string, number>, _pick: string, _pos: string): Group[] {
    const out: Group[] = [];
    const cmp = sorter(by);
    if (g === "LEVEL") {
      const byLevel = new Map<number, any[]>();
      dataHandler.networkAircraft.forEach((m, track) => {
        for (const ac of m.values()) {
          if (!matches(ac, track, f, mine, eb, wb, rr, tg) || !searchMatch(ac, s)) continue;
          const k = ac.AssignedLevel > 0 ? ac.AssignedLevel : 0;
          byLevel.set(k, [...(byLevel.get(k) ?? []), ac]);
        }
      });
      [...byLevel.keys()].sort((a, b) => b - a).forEach((k) => out.push({ key: "FL" + k, label: k > 0 ? "FL" + k : "No level", track: "", kind: "LEVEL", rows: byLevel.get(k)!.sort(cmp) }));
      return out;
    }
    if (g === "ENTRY") {
      const byEntry = new Map<string, any[]>();
      dataHandler.networkAircraft.forEach((m, track) => {
        for (const ac of m.values()) {
          if (!matches(ac, track, f, mine, eb, wb, rr, tg) || !searchMatch(ac, s)) continue;
          const k = ac.Entry || "?";
          byEntry.set(k, [...(byEntry.get(k) ?? []), ac]);
        }
      });
      [...byEntry.keys()].sort().forEach((k) => out.push({ key: k, label: k, track: "", rows: byEntry.get(k)!.sort(cmp) }));
      return out;
    }
    // NAT tracks (and Concorde tracks) first, then ONE group for all the Shanwick Tango routes, then RANDOM
    let tangoRows: any[] = [];
    let rrGroup: Group | null = null;
    dataHandler.networkAircraft.forEach((m, track) => {
      const rows = [...m.values()].filter((ac) => isPicked(ac) || (matches(ac, track, f, mine, eb, wb, rr, tg) && searchMatch(ac, s))).sort(cmp);
      if (rows.length === 0) return;
      if (dataHandler.isTangoTrack(track)) { tangoRows = tangoRows.concat(rows); return; }
      const grp: Group = { key: track, label: track != "RR" ? track : "RANDOM", track, rows };
      if (track === "RR") rrGroup = grp; else out.push(grp);
    });
    if (tangoRows.length > 0) out.push({ key: "TANGO", label: "TANGO ROUTES", track: "TANGO", rows: tangoRows.sort(cmp) });
    if (rrGroup) out.push(rrGroup);
    return out;
  }
  $: groups = buildGroups(tickN, windowMode, filter, showMine, showEb, showWb, showRr, showTango, search, groupBy, sortBy, $hidden, $espicked, $session.position);
  $: hiddenCount = (tickN >= 0 && windowMode === "WINDOW") ? (() => { let n = 0; dataHandler.networkAircraft.forEach((m) => m.forEach((ac: any) => { if (ac.Relevant && utils.isRecent(ac.LastUpdated) && !dataHandler.trafficView(ac, WIN_AHEAD, WIN_BEHIND, dataHandler.scopeForPosition($session.position)).show) n++; })); return n; })() : 0;
  // Number of fix columns: the longest route among the shown flights and track headers, so every row lines up
  $: cols = Math.min(10, Math.max(5, ...groups.map((g) => Math.max(g.track && g.track !== "RR" ? (dataHandler.currentNatTracks.get(g.track)?.Route.length ?? 0) : 0, ...g.rows.map((r: any) => r.RouteEtas.length)))));
  $: shownCount = groups.reduce((n, g) => n + g.rows.length, 0);
  $: concOverdueN = dataHandler.concOverdueList().length + 0 * tickN;
  $: conflictCount = [...dataHandler.conflicts.keys()].length + 0 * tickN;
  let addCs: string = "";
  let addMsg: string = "";
  async function addStrip(): Promise<void> {
    const cs = addCs.trim().toUpperCase();
    if (!cs) return;
    if ($hidden[cs]) { unhideStrip(cs); addCs = ""; addMsg = ""; dataHandler.setAsel(cs); return; }   // hidden by you earlier: just bring it back
    if (dataHandler.getAircraft(cs)) { addCs = ""; addMsg = ""; dataHandler.setAsel(cs); return; }
    addMsg = "Adding...";
    const e = await sess.addFlight(cs);
    addMsg = e ? e : "";
    if (!e) { addCs = ""; dataHandler.setAsel(cs); }
  }
  let posMenu: boolean = false;
  // One box for search and add: Enter picks the first match; with no match, and permission to edit, it adds the callsign
  async function searchAdd(): Promise<void> {
    if (!search.trim()) return;
    addCs = search;
    await addStrip();
    if (addCs === "") search = "";
  }
  function searchKey(e: KeyboardEvent): void {
    if (e.key === "Enter" && groups.length > 0 && groups[0].rows.length > 0) dataHandler.setAsel(groups[0].rows[0].Callsign);
    else if (e.key === "Enter" && sess.canEdit($session)) searchAdd();
    if (e.key === "Escape") search = "";
  }
  function setFilter(f: string): void { filter = f; }
  function resetFilters(): void { filter = "ALL"; showMine = showEb = showWb = showRr = showTango = false; search = ""; setWindowMode("WINDOW"); groupBy = "TRACK"; sortBy = "DEFAULT"; }
  // The aircraft just clicked in EuroScope is always on the list, whatever the filters, search, window or Hide say
  function isPicked(ac: any): boolean { return !!$espicked && ac.Callsign.toUpperCase() === $espicked && !!ac.Relevant; }
  function matches(ac: any, track: string, f: string, mine: boolean, eb: boolean, wb: boolean, rr: boolean, tg: boolean): boolean {
    if (isPicked(ac)) return true;
    if (!ac.Relevant || !utils.isRecent(ac.LastUpdated)) return false;
    if ($hidden[ac.Callsign.toUpperCase()]) return false;     // hidden by this person only
    if (!typeMatch(ac, track, mine, eb, wb, rr, tg)) return false;
    if (windowMode === "WINDOW" && !dataHandler.trafficView(ac, WIN_AHEAD, WIN_BEHIND, dataHandler.scopeForPosition($session.position)).show) return false;
    switch (f) {
      case "RR": return track == "RR";
      default: return true;
    }
  }
  function groupHasMatch(track: string, f: string, mine: boolean, eb: boolean, wb: boolean, rr: boolean, tg: boolean): boolean {
    const m = dataHandler.networkAircraft.get(track);
    if (!m) return false;
    for (const ac of m.values()) if (matches(ac, track, f, mine, eb, wb, rr, tg)) return true;
    return false;
  }

  // natTrak requests box + alerts
  let tickN: number = 0;
  let pendingList: any[] = [];
  let clearedList: any[] = [];
  let alertCount: number = 0;
  let soundOn: boolean = dataHandler.soundOn;
  let visualOn: boolean = dataHandler.visualOn;
  let tone: string = dataHandler.tone;
  function refreshNatTrak(): void {
    const all: any[] = [...dataHandler.natTrakData.values()];
    pendingList = all.filter((d) => d.status === "PENDING" && !dataHandler.isNatTrakIgnored(d.callsign));
    clearedList = all.filter((d) => d.status !== "PENDING").sort((a, b) => String(b.clearance_issued).localeCompare(String(a.clearance_issued)));
    alertCount = pendingList.filter((d) => dataHandler.isNatTrakAlert(d.callsign)).length;
    soundOn = dataHandler.soundOn;
    visualOn = dataHandler.visualOn;
    tone = dataHandler.tone;
    document.title = (visualOn && alertCount > 0 ? "(" + alertCount + ") " : "") + "Flight Data Display - vNAAATS";
  }
  function pickRequest(callsign: string, openWindow: boolean = false): void {
    if (openWindow) dataHandler.openNatTrakFor(callsign);
    dataHandler.ackNatTrak(callsign);
    dataHandler.setAsel(callsign);
    refreshNatTrak();
  }
  function toggleSound(): void {
    dataHandler.setSound(!dataHandler.soundOn);
    refreshNatTrak();
  }
  const circleNote = acmenu.circleNote;
  let circleOn: boolean = acmenu.circleOn;
  function toggleCircle(): void {
    acmenu.setCircle(!acmenu.circleOn);
    circleOn = acmenu.circleOn;
  }
  function toggleVisual(): void {
    dataHandler.setVisual(!dataHandler.visualOn);
    refreshNatTrak();
  }
  function changeTone(): void {
    dataHandler.cycleTone();
    refreshNatTrak();
  }

  // Externals
  $: acListKeys = [...dataHandler.networkAircraft.keys()];
  $: tmi = dataHandler.currentTMI;

  // Time string
  $: timeString = utils.parseTime(new Date());

  // DOM updater (1 second)
  setInterval(() => {
    // Reset externals
    acListKeys = [...dataHandler.networkAircraft.keys()];
    tmi = dataHandler.currentTMI;

    // natTrak requests / flashing
    tickN++;
    refreshNatTrak();

    // Update time
    timeString = utils.parseTime(new Date());

    // Handle 'loading' and 'no content'
    if (dataHandler.networkAircraft.size > 1 || dataHandler.aircraftCount > 0) {
      // tracks are downloaded so we are no longer 'loading'
      isLoading = false;
      // Re-check every tick whether any aircraft exist
      let any: boolean = false;
      dataHandler.networkAircraft.forEach(function (value, key) {
        if (value.size > 0) any = true;
      });
      noData = !any;
    }
  }, 1000);
</script>

<!-- The main flight data panel with flight strips -->
<div class="flex flex-col w-full border-b-2 border-r-2 border-blue-10">
  <div class="flex flex-row flex-wrap gap-y-2 justify-around items-center w-full min-h-[3.5rem] border-blue-10 border-b-2 text-white py-3 font-regular font-light">
    <div class="flex flex-row items-center justify-start 2xl:scale-100 xl:scale-85">
      <div class="px-0.5 hover:cursor-default">
        <div class="flex items-center justify-between min-w-5 2xl:h-9 xl:h-8 px-1 border-2 border-r-blue-8 border-b-blue-8 border-l-blue-0 border-t-blue-0">
          <Checkbox identifier={"mine"} label={"TRKD"} title={"Tracked: only the flights you are tracking at your position"} bind:checked={showMine} />
          <Checkbox identifier={"eb"} label={"EB"} title={"Eastbound flights"} bind:checked={showEb} />
          <Checkbox identifier={"wb"} label={"WB"} title={"Westbound flights"} bind:checked={showWb} />
          <Checkbox identifier={"rr"} label={"RR"} title={"Random routings only"} bind:checked={showRr} />
          <Checkbox identifier={"tango"} label={"TANGO"} title={"Shanwick Tango routes only"} bind:checked={showTango} />
        </div>
      </div>
    </div>
    <div class="flex flex-row items-center justify-end 2xl:w-1/5 xl:w-1/6 2xl:scale-100 xl:scale-85 xl:ml-5">
      <div class="flex-row px-3">
        <div class="select-none flex items-center h-10 px-3 text-2xl border-2 hover:cursor-default bg-blue-2 border-r-blue-8 border-b-blue-8 border-l-blue-0 border-t-blue-0">
          {timeString}
        </div>
      </div>
      <div class="flex-row px-3">
        <div class="select-none flex items-center h-10 px-3 text-2xl border-2 hover:cursor-default bg-blue-2 border-r-blue-8 border-b-blue-8 border-l-blue-0 border-t-blue-0">
          {tmi != "" ? tmi : "000"}
        </div>
      </div>
    </div>
    <div class="flex flex-row-reverse items-center 2xl:scale-100 xl:scale-85">
      <div class="px-3" on:click={resetFilters}>
        <Button label="Default" />
      </div>
    </div>
  </div>
  <!-- row 1: who I am and what is live (VACS, CPDLC, Auto RCL, alerts) -->
  <div class="flex flex-row flex-wrap items-center gap-x-3 gap-y-1 px-2 pt-1 w-full text-white font-regular text-sm select-none">
    <div class="relative">
      {#if $session.loggedIn && $session.position}
        <span class="font-bold text-green-300">{$session.position}</span> {$session.name}
        <button class="ml-1 px-2 py-0 font-bold border border-blue-6 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={() => (posMenu = !posMenu)} title="Change position, relief, sign out">Position &#9662;</button>
        {#if posMenu}
          <div class="fixed inset-0 z-40" on:click={() => (posMenu = false)}></div>
          <div class="absolute left-0 top-full mt-1 z-50 min-w-max border-2 border-blue-10 bg-blue-2 shadow-lg">
            <div class="px-4 py-1 hover:bg-blue-10 hover:text-black hover:cursor-pointer" on:click={() => { posMenu = false; sess.choosePosition(""); }}>Change position</div>
            <div class="px-4 py-1 hover:bg-blue-10 hover:text-black hover:cursor-pointer" title="Hand all your tracked aircraft to the controller relieving you" on:click={() => { posMenu = false; reliefOpen.set(true); }}>Relief / handover</div>
            <div class="px-4 py-1 hover:bg-blue-10 hover:text-black hover:cursor-pointer" on:click={() => { posMenu = false; sess.logout(); }}>Sign out</div>
          </div>
        {/if}
      {:else if $session.editing}
        <span class="opacity-70">Signed out</span>
        <span class="underline hover:cursor-pointer ml-1" on:click={() => location.reload()}>sign in</span>
      {:else}
        <span class="opacity-70">Sign-in is not switched on for this server</span>
      {/if}
    </div>
    <div class="flex-1"></div>
    <VacsChip />
    <AudioChip />
    <RclPanel />
    {#if $cpdlcStatus.enabled}
      <button class="px-2 py-0.5 border border-blue-6 {(cpdlcUnread > 0 || $cpdlcAttention.size > 0) && visualOn && tickN >= 0 ? 'nt-flash text-black font-bold' : 'bg-blue-3'}"
        title="Open the CPDLC panel" on:click={() => cpdlcOpen.set(!$cpdlcOpen)}>
        CPDLC{$cpdlcStatus.connected.length > 0 ? " [" + $cpdlcStatus.connected.length + "]" : ""}{cpdlcUnread > 0 ? " \u2709 " + cpdlcUnread : ""}{$cpdlcAttention.size > 0 ? " new logon" : ""}
      </button>
    {/if}
    {#if $rclAuto.sender !== "" || $rclAuto.on}
      <button class="px-2 py-0.5 border border-blue-6 {rclAsk && visualOn && tickN >= 0 ? 'nt-flash text-black font-bold' : $rclAuto.on ? 'bg-blue-3' : 'bg-red-700'}"
        title={($rclAuto.on ? "Automatic RCL REQ is ON: flights 20 to 45 minutes from oceanic entry (or within 200 nm if they have no times) that have not asked for clearance on natTrak are sent the reminder from " + ($rclAuto.sender || "an oceanic controller's EuroScope (none online right now)") + "." + ($rclAuto.confirm ? " The controller is asked before anything is typed." : "") : "Automatic RCL REQ is OFF.") + " Click for details and settings." + ($rclAuto.error ? " Last problem: " + $rclAuto.error : "")}
        on:click={() => rclPanelOpen.set(!$rclPanelOpen)}>Auto RCL: {$rclAuto.on ? ($rclAuto.confirm && $rclAuto.waiting.length > 0 ? $rclAuto.waiting.length + " waiting" : "ON") : "OFF"}{$rclAuto.on && $rclAuto.sender === "" ? " (no sender)" : ""}</button>
    {/if}
    {#if concOverdueN > 0}
      <span class="px-1 font-bold {visualOn && tickN >= 0 ? 'nt-flash text-black' : 'text-yellow-300'}" title="Concorde flights overdue a position report">&#9201; {concOverdueN} overdue</span>
    {/if}
    {#if conflictCount > 0}
      <span class="px-2 py-0.5 border border-yellow-300 text-yellow-300 font-bold" title="Flights with a warning are marked on their strip">&#9888; {conflictCount} alert{conflictCount === 1 ? "" : "s"}</span>
    {/if}
    <button class="px-2 py-0.5 font-bold border border-blue-6 bg-blue-3" title="Plugin options (vnaaats.cfg)" on:click={() => settingsOpen.set(true)}>Settings</button>
  </div>
  <!-- row 2: how the list is shown (show, group, sort), counts, and search or add -->
  <div class="flex flex-row flex-wrap items-center gap-x-3 gap-y-1 px-2 py-1 w-full border-b-2 border-blue-10 text-white font-regular text-sm select-none">
    <label title="Show flights within 60 minutes of oceanic entry, until 15 minutes after the exit (shown faded)">Show
      <select class="text-black" value={windowMode} on:change={(e) => setWindowMode(e.currentTarget.value)}>
        <option value="WINDOW">entry -60 / exit +15 min</option>
        <option value="ALL">all flights</option>
      </select>
    </label>
    <label>Group
      <select class="text-black" bind:value={groupBy}>
        <option value="TRACK">by track</option>
        <option value="ENTRY">by entry fix</option>
        <option value="LEVEL">by flight level</option>
      </select>
    </label>
    <label>Sort
      <select class="text-black" bind:value={sortBy}>
        <option value="DEFAULT">newest first</option>
        <option value="ETA">entry ETA</option>
        <option value="CALLSIGN">callsign</option>
        <option value="LEVEL">level</option>
        <option value="UPDATED">last changed</option>
      </select>
    </label>
    {#if Object.keys($hidden).length > 0}
      <select class="text-black w-28" value="" on:change={(e) => { unhideStrip(e.currentTarget.value); e.currentTarget.value = ""; }} title="Strips you have hidden - pick one to bring it back">
        <option value="">Hidden ({Object.keys($hidden).length})</option>
        {#each Object.keys($hidden) as h}<option value={h}>Unhide {h}</option>{/each}
      </select>
    {/if}
    <div class="flex-1"></div>
    <span class="opacity-80" title={hiddenCount > 0 ? hiddenCount + " flights are outside the time window" : ""}><b>{shownCount}</b> shown{#if hiddenCount > 0} &middot; <b>{hiddenCount}</b> hidden{/if}</span>
    {#if addMsg}<span class="text-yellow-300">{addMsg}</span>{/if}
    <input class="px-2 py-0.5 w-64 text-black uppercase" placeholder={sess.canEdit($session) ? "Search or add: callsign / fix" : "Search callsign / fix"} bind:value={search} on:keydown={searchKey}
      title="Type to search the list. If the flight is not on the list, press Enter or Add to add a flight that is on VATSIM by its callsign." />
    {#if sess.canEdit($session)}
      <button class="px-2 py-0.5 bg-blue-3 border border-blue-6" title="Add this callsign if it is on VATSIM but not on the list" on:click={searchAdd}>Add</button>
    {/if}
  </div>
  <!-- natTrak requests: PENDING (needs action) and CLEARED -->
  <div class="flex flex-row w-full border-b-2 border-blue-10 text-white font-regular" style="height: 9.5rem; font-size: 0.8rem;">
    <div class="flex flex-col w-1/2 border-r-2 border-blue-10">
      <div class="flex flex-row justify-between px-2 py-0.5 bg-blue-3 font-bold select-none">
        <span>natTrak PENDING ({pendingList.length}) - needs action</span>
        <span class="underline text-yellow-300 hover:cursor-pointer font-bold" on:click={natClick}>{natApp && !natSigned ? "LOGIN" : "Open natTrak"}</span>
      </div>
      {#if natApp && !natSigned}
        <div class="px-2 py-1 bg-yellow-900 text-yellow-100 border-b border-yellow-500 text-xs">To use natTrak within the FDD you must log in to natTrak with your VATSIM details <b>once you have connected to the network through EuroScope</b>. Please click the natTrak <b>LOGIN</b> button.</div>
      {/if}
      <div class="flex-1 min-h-0 overflow-y-auto">
        {#if pendingList.length == 0}
          <div class="px-2 py-1 opacity-70 select-none">No pending requests</div>
        {/if}
        {#each pendingList as p}
          <div class="flex flex-row px-2 py-0.5 border-b border-blue-4 hover:cursor-pointer {visualOn && dataHandler.isNatTrakAlert(p.callsign) && tickN >= 0 ? 'nt-flash text-black' : ''}"
            title="Click to open natTrak and select the aircraft"
            on:click={() => pickRequest(p.callsign, true)}>
            <div class="w-1/5 font-bold">{p.callsign}</div>
            <div class="w-1/12">{p.nat}</div>
            <div class="w-1/6">{utils.formatWaypoint(p.fix)}</div>
            <div class="w-1/6">FL{p.level}</div>
            <div class="w-1/6">M{p.mach > 0 ? "0" + p.mach : "--"}</div>
            <div class="w-1/6">{p.estimating_time}z</div>
          </div>
        {/each}
      </div>
    </div>
    <div class="flex flex-col w-1/2">
      <div class="flex flex-row justify-between px-2 py-0.5 bg-blue-3 font-bold select-none">
        <span>natTrak CLEARED ({clearedList.length})</span>
        <span class="flex flex-row gap-x-3">
          <span class="underline hover:cursor-pointer" title="Flashing strips and tab title" on:click={toggleVisual}>Visual: {visualOn ? "ON" : "OFF"}</span>
          <span class="underline hover:cursor-pointer" title="Sound when a new request arrives" on:click={toggleSound}>Audio: {soundOn ? "ON" : "OFF"}</span>
          <span class="underline hover:cursor-pointer" title="Clicking an aircraft here circles it in white in EuroScope for a few seconds" on:click={toggleCircle}>Circle: {circleOn ? "ON" : "OFF"}</span>
          {#if $circleNote}<span class="text-yellow-300 ml-2">{$circleNote}</span>{/if}
          <span class="underline hover:cursor-pointer" title="Change the alert sound (plays a preview)" on:click={changeTone}>Tone: {tone}</span>
        </span>
      </div>
      <div class="flex-1 min-h-0 overflow-y-auto">
        {#each clearedList as c}
          <div class="flex flex-row px-2 py-0.5 border-b border-blue-4 hover:cursor-pointer" on:click={() => pickRequest(c.callsign)}>
            <div class="w-1/5 font-bold">{c.callsign}</div>
            <div class="w-1/12">{c.nat}</div>
            <div class="w-1/6">{utils.formatWaypoint(c.fix)}</div>
            <div class="w-1/6">FL{c.level}</div>
            <div class="w-1/6">M{c.mach > 0 ? "0" + c.mach : "--"}</div>
            <div class="w-1/6">{c.clearance_issued ?? ""}</div>
          </div>
        {/each}
      </div>
    </div>
  </div>
  <div
    class="flex flex-col flex-1 min-h-0 font-bitmap font-light text-lg overflow-y-scroll overflow-x-hidden"
    style="font-size: clamp(0.65rem, 1.2vw, 1.125rem);">
    <!-- Aircraft component cards -->
    {#if isLoading}
      <div class="select-none flex flex-row w-full h-full justify-items-center justify-center text-2xl my-10 hover:cursor-default">
        Loading data...
      </div>
    {:else if noData}
      <div class="select-none flex flex-row w-full h-full justify-items-center justify-center text-2xl my-10 hover:cursor-default">
        No data on remote.
      </div>
    {:else}
      {#each groups as g (g.key)}
        <div class="strip-grid w-full bg-grey-500 bg-opacity-70 select-none hover:cursor-default" style="--cols: {cols};">
          <div class="px-3 whitespace-nowrap">{g.label}</div>
          {#if g.kind === "LEVEL"}
            <div class="whitespace-nowrap" style="grid-column: 2 / span 4;">{g.rows.length} aircraft</div>
          {:else if g.track == "TANGO"}
            <div class="whitespace-nowrap overflow-hidden" style="grid-column: 4 / -1;">SHANWICK TANGO ROUTES: T9 T290 T213 T13 T16</div>
          {:else if g.track == "RR" || g.track == ""}
            <div class="whitespace-nowrap" style="grid-column: 4 / span 4;">{g.track == "RR" ? "RANDOM ROUTINGS" : "ENTRY FIX"}</div>
          {:else}
            {#if dataHandler.currentNatTracks.get(g.track).Tango}
              <div class="whitespace-nowrap overflow-hidden" style="grid-column: 2 / -1;">Tango - {dataHandler.currentNatTracks.get(g.track).Use} - {dataHandler.currentNatTracks.get(g.track).Route.map((w) => w.name).join(" ")}</div>
            {:else if dataHandler.currentNatTracks.get(g.track).Supersonic}
              <div class="whitespace-nowrap overflow-hidden" style="grid-column: 2 / -1;">Block {dataHandler.currentNatTracks.get(g.track).BlockLevel} - {dataHandler.currentNatTracks.get(g.track).Route.map((w) => w.name).join(" ")}</div>
            {:else}
            <div style="grid-column: 2 / span 3;" />
            {#each dataHandler.currentNatTracks.get(g.track).Route as waypoint}
              <div class="whitespace-nowrap overflow-hidden" style={waypoint.name.length > 5 ? "font-size: 0.82em;" : ""}>{waypoint.name}</div>
            {/each}
            {/if}
          {/if}
        </div>
        {#each g.rows as ac (ac.Callsign)}
          <FlightStrip 
            cols={cols}
            callsign={ac.Callsign}
            type={ac.Type}
            assignedLevel={ac.AssignedLevel}
            blockLevel={dataHandler.currentNatTracks.get(ac.Track)?.BlockLevel ?? ""}
            assignedMach={ac.AssignedMach}
            track={ac.Track}
            route={ac.Route}
            routeEtas={ac.RouteEtas}
            departure={ac.Departure}
            arrival={ac.Arrival}
            trackedBy={ac.TrackedBy}
            isEquipped={ac.IsEquipped}
            alert={visualOn && dataHandler.isNatTrakAlert(ac.Callsign) && tickN >= 0}
            ntSent={ac.NtSent}
            cmAt={ac.ContactMeAt ?? 0}
            cmState={tickN >= 0 ? dataHandler.contactMeState(ac) : ""}
            comm={ac.Comm}
            cpdlc={ac.Cpdlc}
            cpdlcAlert={visualOn && (($cpdlcStatus.unread?.[ac.Callsign] ?? 0) > 0 || $cpdlcAttention.has(ac.Callsign)) && tickN >= 0}
            concOverdue={visualOn && tickN >= 0 && dataHandler.concOverdueFix(ac) !== ""}
            warning={(dataHandler.conflicts.get(ac.Callsign) ?? []).join("; ")}
            updatedBy={ac.UpdatedBy}
            updatedAt={ac.UpdatedAt}
            direction={ac.Direction ?? false}
            tango={dataHandler.isTangoTrack(ac.Track) ? ac.Track : ""}
            faded={tickN >= 0 && windowMode === "WINDOW" && dataHandler.trafficView(ac, WIN_AHEAD, WIN_BEHIND, dataHandler.scopeForPosition($session.position)).faded}
          />
        {/each}
      {/each}
    {/if}
  </div>
</div>

<style>
  :global(.nt-flash) {
    animation: ntflash 1s steps(1, end) infinite;
  }
  @keyframes -global-ntflash {
    0% { background-color: #ef4444; }
    50% { background-color: #fde047; }
  }
</style>
