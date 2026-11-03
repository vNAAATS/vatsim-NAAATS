<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import * as dataHandler from "../../typescript/fdd/data";
  import * as sess from "../../typescript/fdd/session";
  import * as cp from "../../typescript/fdd/cpdlc";
  import * as pr from "../../typescript/fdd/posreports";
  import * as m from "../../typescript/fdd/acmenu";
  import { hideStrip } from "../../typescript/fdd/hide";

  const { session } = sess;
  export let callsign: string;
  const dispatch = createEventDispatcher();
  const { cpdlcStatus } = cp;

  let tab: string = "track";
  let lastCs: string = "";
  let level: string = "";
  let mach: string = "";
  let selcal: string = "";
  let scratch: string = "";
  let note: string = "";
  let copied: string = "";
  let busy: boolean = false;
  let controllers: m.Controller[] = [];
  let pickedCtrl: string = "";
  let wallopText: string = "";
  let ac: any = null;

  $: selcalWarn = /[^A-Za-z\-\s]/.test(selcal) ? "SELCAL is letters only" : "";   // any four letters are allowed; only warn about things that are not letters
  $: canEdit = sess.canEdit($session);
  openFor(callsign);

  function openFor(cs: string): void {
    ac = dataHandler.getAircraft(cs);
    tab = "track"; note = ""; copied = ""; busy = false; wallopText = ""; pickedCtrl = "";
    level = ac && ac.AssignedLevel ? String(ac.AssignedLevel) : "";
    mach = ac && ac.AssignedMach ? String(ac.AssignedMach).padStart(3, "0") : "";
    selcal = ac && ac.Selcal && ac.Selcal !== "N/A" ? String(ac.Selcal) : "";
    scratch = ac && ac.Scratch ? String(ac.Scratch) : "";
    m.loadControllers().then((c) => (controllers = c));
  }
  function close(): void { dispatch("close"); }
  function pickTab(t: string): void { tab = t; note = ""; copied = ""; }

  async function waitFor(id: number): Promise<void> {
    note = "Sent to EuroScope...";
    for (let i = 0; i < 40; i++) {
      await new Promise((r) => setTimeout(r, 1000));
      const s = await m.commandState(id);
      if (s.state === "ok") {
        note = "✔ " + (s.msg || "Done");
        await dataHandler.populateAllAircraft();
        ac = dataHandler.getAircraft(callsign);
        // EuroScope / the server can take a few seconds to report the change (tracker, headset): look again so no second click is needed
        for (const ms of [3000, 6000]) setTimeout(async () => { await dataHandler.populateAllAircraft(); ac = dataHandler.getAircraft(callsign); }, ms);
        return;
      }
      if (s.state === "failed") { note = "✖ " + s.msg; return; }
      if (s.state === "waiting") note = "Waiting - is your EuroScope running with the vNAAATS plugin and signed in as " + $session.position + "?";
    }
    note = "No answer from EuroScope yet.";
  }

  async function send(body: Record<string, string | number>): Promise<void> {
    busy = true; note = "";
    const r = await m.queueCommand(Object.assign({ callsign: callsign }, body));
    if (typeof r === "string") note = "✖ " + r;
    else await waitFor(r);
    busy = false;
  }

  // Ask the plugin to type the command into EuroScope. If that does not work, copy it instead and say so.
  async function typeIn(body: Record<string, string | number>, text: string): Promise<void> {
    copied = "";
    await send(body);
    if (note.charAt(0) === "\u2716") {
      const why = note;
      await copy(text);
      note = why + " - the command was copied instead, so paste it in.";
    }
  }

  async function copy(text: string): Promise<void> {
    copied = text;
    note = (await m.copyText(text)) ? "" : "Could not copy automatically - type it in yourself.";
  }

  function track(): void {
    if (selcal && !m.validSelcal(selcal)) { note = "✖ " + m.selcalProblem(selcal); return; }
    send({ type: "track", level, mach, selcal: selcal.toUpperCase() });
  }
  function untrack(): void {
    if (!confirm("Release tag for " + callsign + "?")) return;
    send({ type: "untrack" });
  }
  function transfer(): void {
    const c = controllers.find((x) => x.callsign === pickedCtrl);
    if (!c) { note = "✖ Choose a controller"; return; }
    send({ type: "transfer", target: c.callsign, freq: c.frequency });
  }
  async function saveSelcal(): Promise<void> {
    if (!m.validSelcal(selcal)) { note = "✖ " + m.selcalProblem(selcal); return; }
    const e = await sess.editFlight(callsign, { selcal: selcal.toUpperCase() });
    note = e ? "✖ " + e : "✔ SELCAL saved";
  }
  async function saveScratch(): Promise<void> {
    const t = scratch.toUpperCase().replace(/\s+/g, " ").trim();
    if (!/^[A-Z0-9 \/.,+*-]{0,15}$/.test(t)) { note = "\u2716 Note: up to 15 letters, numbers, spaces and / . , + * -"; return; }
    const e = await sess.editFlight(callsign, { scratch: t });
    note = e ? "\u2716 " + e : "\u2714 Note saved - it shows under the tag in EuroScope";
  }
  // Copy the .wallop and mark the aircraft's note WALLOPED (the EuroScope scratchpad), so everyone can see it was reported
  async function copyWallop(): Promise<void> {
    await copy(m.wallopCommand(callsign, wallopText));
    if (!canEdit) return;
    const old = (scratch || "").toUpperCase().replace(/\s+/g, " ").trim();
    if (old.indexOf("WALLOPED") >= 0) return;
    const t = old && ("WALLOPED " + old).length <= 15 ? "WALLOPED " + old : "WALLOPED";
    const e = await sess.editFlight(callsign, { scratch: t });
    if (!e) { scratch = t; note = (note ? note + " " : "") + "\u2714 Note set to " + t; }
  }
  function hideThis(): void {
    const cs = callsign;
    hideStrip(cs);
    if (dataHandler.asel && dataHandler.asel.Callsign === cs) dataHandler.clearAsel();
    close();
  }
  async function ntMark(sent: boolean): Promise<void> {
    const e = await m.markNatTrakSent(callsign, sent);
    note = e ? "\u2716 " + e : (sent ? "\u2714 Marked with a red * (it clears when the pilot requests clearance or an oceanic controller tracks the flight)" : "\u2714 Mark removed");
    if (!e) { await dataHandler.populateAllAircraft(); ac = dataHandler.getAircraft(callsign); }
  }
  function openCpdlc(): void { dataHandler.setAsel(callsign); cp.cpdlcPick.set(callsign); cp.cpdlcOpen.set(true); close(); }
  function openPos(): void { dataHandler.setAsel(callsign); pr.posOpen.set(true); close(); }

  $: isConc = !!ac && (ac.IsConc || ac.Type === "CONC");
  $: onCpdlc = !!callsign && cp.isConnected(callsign);
  $: tabs = [["track", trackedByMe ? "Update" : "Track"], ["transfer", "Transfer"], ["untrack", "Release Tag"], ["selcal", "SELCAL"], ["cpdlc", "CPDLC"], ["scratch", "Note"], ["contact", "Contact Me"], ["nt", "RCL REQ"], ["remove", "Hide"]] as [string, string][];
  $: trackedByMe = !!ac && !!ac.TrackedBy && !!$session.position && String(ac.TrackedBy).toUpperCase() === $session.position.toUpperCase();
  $: trackId = ac && ac.Track ? String(ac.Track) : "";
  $: blockLevel = trackId ? (dataHandler.currentNatTracks.get(trackId)?.BlockLevel ?? "") : "";
  const BTN = "border-2 border-l-blue-6 border-t-blue-6 border-r-blue-0 border-b-blue-0 bg-blue-3 active:bg-blue-1 hover:cursor-pointer select-none";
</script>

<div class="w-full mb-3 bg-blue-3 border-2 border-l-blue-10 border-t-blue-10 border-r-blue-0 border-b-blue-0 text-white font-regular" style="max-width: 52rem;">
      <div class="flex flex-row justify-between items-center px-3 py-2 bg-blue-4 font-bold select-none">
        <span>Aircraft menu - {callsign}<span class="ml-3 font-regular text-yellow-300">{trackId ? "Track " + trackId : "No track"}{blockLevel ? " - Block " + blockLevel : ""}</span></span>
        <button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={close}>Close</button>
      </div>
      <div class="p-3">
        <div class="flex flex-row flex-wrap items-center gap-x-2 gap-y-2 mb-4 select-none">
          {#each tabs as t}
            <span class="px-2 py-0.5 {BTN} {tab === t[0] ? 'bg-blue-1 border-l-blue-0 border-t-blue-0 border-r-blue-6 border-b-blue-6' : ''}" on:click={() => pickTab(t[0])}>{t[1]}</span>
          {/each}
          {#if isConc}
            <span class="px-2 py-0.5 {BTN}" on:click={openPos}>Position reports</span>
          {/if}
          <span class="ml-auto" />
          <span class="px-2 py-0.5 border-2 border-red-500 bg-red-700 font-bold hover:cursor-pointer {tab === 'sup' ? 'bg-red-900' : ''}" on:click={() => pickTab("sup")}>Supervisor</span>
        </div>

        {#if tab === "track"}
          <div class="flex flex-row flex-wrap items-start gap-x-4 gap-y-3">
            <button class="px-2 py-1 {BTN}" disabled={busy || !canEdit} on:click={track}>{trackedByMe ? "Update aircraft" : "Track aircraft"}</button>
            <label class="text-sm">FL <input class="w-16 px-1 text-black" bind:value={level} placeholder="optional" /></label>
            <label class="text-sm">Mach <input class="w-16 px-1 text-black" bind:value={mach} placeholder="optional" /></label>
            <div class="flex flex-col items-start">
              <label class="text-sm">SELCAL <input class="w-20 px-1 text-black uppercase" maxlength="4" bind:value={selcal} /></label>
              <button class="px-2 py-1 mt-2 {BTN}" disabled={busy || !canEdit || !m.validSelcal(selcal)} title="Types .selcal into EuroScope for this aircraft" on:click={() => typeIn({ type: "selcal", selcal: selcal.toUpperCase() }, m.selcalCommand(selcal))}>SELCAL Check</button>
            </div>
          </div>
        {:else if tab === "remove"}
          <div class="text-sm mb-2 text-yellow-300">Hide {callsign} from your display? Only you stop seeing it, and only for 12 hours. Nobody else is affected and EuroScope and VATSIM are not touched. To bring it back, use the Hidden list or type the callsign in Add callsign.</div>
          <button class="px-2 py-1 bg-red-700 border-2 border-red-300 font-bold" on:click={hideThis}>Yes, hide {callsign}</button>
          <button class="px-2 py-1 ml-2 {BTN}" on:click={() => pickTab("track")}>Cancel</button>
        {:else if tab === "untrack"}
          <button class="px-2 py-1 {BTN}" disabled={busy || !canEdit} on:click={untrack}>Release tag</button>
        {:else if tab === "transfer"}
          {#if controllers.length === 0}
            <div class="text-sm opacity-80">No adjacent ATC - Send to UNICOM</div>
          {:else}
            <select class="w-full px-1 py-0.5 text-black mb-3" bind:value={pickedCtrl}>
              <option value="">Choose controller</option>
              {#each controllers as c}
                <option value={c.callsign}>{c.callsign} {c.frequency} {c.name}</option>
              {/each}
            </select>
            <button class="px-2 py-1 {BTN}" disabled={busy || !canEdit || !pickedCtrl} on:click={transfer}>Transfer</button>
          {/if}
        {:else if tab === "selcal"}
          <div class="flex flex-row items-center">
            <input class="w-20 px-1 mr-2 text-black uppercase" maxlength="4" bind:value={selcal} />
            <button class="px-2 py-1 mr-2 {BTN}" disabled={busy || !canEdit || !m.validSelcal(selcal)} on:click={() => typeIn({ type: "selcal", selcal: selcal.toUpperCase() }, m.selcalCommand(selcal))}>Send</button>
            <button class="px-2 py-1 mr-2 {BTN}" disabled={!m.validSelcal(selcal)} on:click={() => copy(m.selcalCommand(selcal))}>Copy .selcal</button>
            <button class="px-2 py-1 {BTN}" disabled={!canEdit} on:click={saveSelcal}>Save code</button>
          </div>
        {:else if tab === "scratch"}
          <div class="text-sm mb-2 opacity-90">A short note (up to 15 characters) for this aircraft. It is the EuroScope scratchpad: it shows under the tag and goes with the aircraft to the next controller.</div>
          <div class="flex flex-row items-center">
            <input class="w-40 px-1 mr-2 text-black uppercase" maxlength="15" bind:value={scratch} on:keydown={(e) => { if (e.key === "Enter") saveScratch(); }} />
            <button class="px-2 py-1 {BTN}" disabled={!canEdit} on:click={saveScratch}>Save note</button>
          </div>
        {:else if tab === "contact"}
          <div class="text-sm mb-2 opacity-90">"Send" selects {callsign} in EuroScope and runs .contactme for you. If you copy it instead, click the aircraft in EuroScope first, then paste the command.</div>
          <button class="px-2 py-1 mr-2 {BTN}" disabled={busy || !canEdit} on:click={() => typeIn({ type: "contactme" }, m.contactMeCommand(callsign))}>Send</button>
          <button class="px-2 py-1 {BTN}" on:click={() => copy(m.contactMeCommand(callsign))}>Copy .contactme</button>
        {:else if tab === "sup"}
          <textarea class="w-full px-1 mb-2 text-black" rows="3" bind:value={wallopText} placeholder="Message to the supervisors" />
          <div class="flex flex-row">
            <button class="px-2 py-1 mr-2 {BTN}" on:click={() => (wallopText = m.NO_CONTACT_TEXT)}>No Contact</button>
            <button class="px-2 py-1 border-2 border-red-500 bg-red-700 font-bold hover:cursor-pointer" disabled={!wallopText.trim()} on:click={copyWallop}>Copy .wallop</button>
          </div>
        {:else if tab === "nt"}
          <div class="text-sm mb-2 opacity-90">"Send" types it into EuroScope for you (needs autotype=1 in vnaaats.cfg and plugin 1.5b13). Reminds the pilot to request oceanic clearance by CPDLC or on natTrak.</div>
          <div class="flex flex-row flex-wrap">
            <button class="px-2 py-1 mr-2 mb-1 {BTN}" disabled={busy || !canEdit} on:click={() => typeIn({ type: "natmsg" }, m.natTrakCommand(callsign))}>Send</button>
            <button class="px-2 py-1 mr-2 mb-1 {BTN}" on:click={() => copy(m.natTrakCommand(callsign))}>Copy natTrak message</button>
            <button class="px-2 py-1 mr-2 mb-1 {BTN}" disabled={!canEdit} on:click={() => ntMark(true)}>I have sent it - mark with red *</button>
            {#if ac && ac.NtSent}<button class="px-2 py-1 mb-1 {BTN}" disabled={!canEdit} on:click={() => ntMark(false)}>Remove mark</button>{/if}
          </div>
        {:else if tab === "cpdlc"}
          {#if onCpdlc}
            <button class="px-2 py-1 {BTN}" on:click={openCpdlc}>Open CPDLC for {callsign}</button>
          {:else}
            <div class="text-sm opacity-80">{callsign} is not connected to CPDLC.</div>
          {/if}
        {/if}

        {#if selcalWarn && (tab === "track" || tab === "selcal")}
          <div class="mt-3 p-1 text-sm text-white bg-red-700 border border-red-300">{selcalWarn}</div>
        {/if}
        {#if copied}
          <div class="mt-3 p-2 border-2 border-yellow-300 bg-blue-1 text-sm">
            <div class="font-bold text-yellow-300">Message copied</div>
            <div class="font-mono my-1 break-words">{copied}</div>
            <div>Now go to EuroScope, click in the command line, paste it (Ctrl+V) and press Enter to send it.</div>
          </div>
        {/if}
        {#if !canEdit && (tab === "track" || tab === "untrack" || tab === "transfer")}
          <div class="mt-3 text-sm text-yellow-300">Sign in and choose your position to use this.</div>
        {/if}
        {#if note}<div class="mt-3 text-sm break-words {note.charAt(0) === '\u2714' ? 'text-green-300' : 'text-yellow-300'}">{note}</div>{/if}
      </div>
</div>
