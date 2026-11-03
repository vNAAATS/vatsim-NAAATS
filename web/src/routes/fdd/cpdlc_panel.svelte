<script lang="ts">
  import { onDestroy, onMount } from "svelte";
  import * as cp from "../../typescript/fdd/cpdlc";
  import * as dataHandler from "../../typescript/fdd/data";
  import * as sess from "../../typescript/fdd/session";

  const { cpdlcStatus, cpdlcOpen, cpdlcPick, cpdlcFreetexts, cpdlcAttention } = cp;
  const { session } = sess;

  const KEEP = 10;                                    // only the latest messages are shown
  let messages: cp.CpdlcMessage[] = [];
  let totalMsgs: number = 0;
  let loadError: string = "";
  let text: string = "";
  let ra: string = "WU";
  let replyTo: cp.CpdlcMessage | null = null;
  let sendMsg: string = "";
  let sendOk: boolean = true;
  let busy: boolean = false;
  let timer: any;
  let thread: HTMLElement;
  let lastSeenFor: string = "";
  let unicomAsk: boolean = false;
  let hoppieCode: string = "";
  let logonErr: string = "";
  let logonBusy: boolean = false;

  // Optional: remember the Hoppie code in THIS browser only (never on the server), per controller, so it need not be typed each time.
  const savedKey = (): string => "vnaaats.hoppie." + ($session.cid || "x");
  function loadSaved(): string { try { return localStorage.getItem(savedKey()) ?? ""; } catch (e) { return ""; } }
  function storeSaved(code: string): void { try { if (code) localStorage.setItem(savedKey(), code); else localStorage.removeItem(savedKey()); } catch (e) { /* not saved */ } }
  let remember: boolean = false;
  let hasSaved: boolean = false;
  let autoTried: string = "";
  async function signIn(): Promise<void> {
    logonBusy = true;
    const code = hoppieCode.trim();
    logonErr = await cp.logonCpdlc(code);
    logonBusy = false;
    if (logonErr === "") { if (remember) { storeSaved(code); hasSaved = true; } hoppieCode = ""; }
  }
  function forgetSaved(): void { storeSaved(""); hasSaved = false; remember = false; autoTried = "x"; }
  // Sign in by itself with the saved code, once per page visit and station
  $: if (canUse && $cpdlcStatus.enabled && !$cpdlcStatus.active && $cpdlcStatus.stationForPosition && !$cpdlcStatus.inUse
        && !($cpdlcStatus.lockedSeconds ?? 0) && autoTried !== $cpdlcStatus.stationForPosition && $session.cid) {
    autoTried = $cpdlcStatus.stationForPosition;
    const saved = loadSaved();
    hasSaved = saved !== "";
    if (saved) { logonBusy = true; cp.logonCpdlc(saved).then((e) => { logonBusy = false; if (e) logonErr = "Saved code did not work: " + e; }); }
  }
  function useFree(e: Event): void {
    const sel = e.target as HTMLSelectElement;
    const f = $cpdlcFreetexts[parseInt(sel.value)];
    if (f) { text = f.text; ra = f.ra === "NE" ? "NE" : "R"; replyTo = null; lvlVerb = null; }
    sel.value = "";
  }
  async function doUnicom(): Promise<void> {
    unicomAsk = false;
    busy = true;
    const err = await cp.sendUnicom(shown);
    busy = false;
    sendOk = err === "";
    sendMsg = err === "" ? "UNICOM message sent. The pilot is logged off when they answer WILCO/ROGER." : err;
    if (err === "") refresh();
    setTimeout(() => (sendMsg = ""), 8000);
  }
  // Force log out: step 0 closed, 1 type the callsign, 2 read the warnings and type LOG OUT
  let forceStep: number = 0;
  let forceCs: string = "";
  let forceConfirm: string = "";
  let forceErr: string = "";
  let forceBusy: boolean = false;
  function forceCancel(): void { forceStep = 0; forceCs = ""; forceConfirm = ""; forceErr = ""; }
  async function forceGo(): Promise<void> {
    forceBusy = true;
    forceErr = await cp.forceLogout(forceCs.trim(), forceConfirm);
    forceBusy = false;
    if (forceErr === "") { messages = []; forceCancel(); }
  }
  function mmss(n: number): string { return Math.floor(n / 60) + ":" + String(n % 60).padStart(2, "0"); }
  function signOut(): void { messages = []; cp.logoffCpdlc(); }

  $: connected = $cpdlcStatus.connected;
  $: canUse = sess.canEdit($session);
  $: if ($cpdlcOpen && shown) cp.clearAttention(shown);   // looking at it counts as seen
  // Which aircraft is shown: the one picked, else the one selected on the FDD if it is connected, else the first connected
  $: shown = $cpdlcPick && connected.some((c) => c.callsign === $cpdlcPick)
    ? $cpdlcPick
    : (dataHandler.asel && connected.some((c) => c.callsign === dataHandler.asel.Callsign) ? dataHandler.asel.Callsign : connected[0]?.callsign ?? "");

  async function refresh(): Promise<void> {
    if (!$cpdlcOpen || !shown || !canUse || !$cpdlcStatus.active) { return; }
    const r = await cp.loadMessages(shown);
    if (typeof r === "string") { loadError = r; return; }
    loadError = "";
    const grew = r.length !== totalMsgs || r[r.length - 1]?.id !== messages[messages.length - 1]?.id;
    totalMsgs = r.length;
    messages = r.slice(-KEEP);
    if (grew && thread) setTimeout(() => (thread.scrollTop = thread.scrollHeight), 30);
    if (messages.some((m) => m.dir === "DOWN" && !m.seen) && lastSeenFor !== shown + messages[messages.length - 1].id) {
      lastSeenFor = shown + messages[messages.length - 1].id;
      cp.markSeen(shown);
    }
  }

  onMount(() => { timer = setInterval(refresh, 3000); });
  onDestroy(() => clearInterval(timer));
  $: if ($cpdlcOpen && shown) { messages = []; totalMsgs = 0; replyTo = null; lvlVerb = null; lvlPick = 0; refresh(); }

  function pick(callsign: string): void { cpdlcPick.set(callsign); cp.clearAttention(callsign); }
  function useTemplate(t: { text: string; ra: string }): void { text = t.text; ra = t.ra; lvlVerb = null; }
  // Level messages: pick the verb (climb / descend / maintain / expect climb), then the flight level; the message is filled in for you.
  const LEVELS: number[] = [300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400, 410];
  let lvlVerb: { label: string; text: string; ra: string } | null = null;
  let lvlPick: number = 0;
  const LEVEL_LABELS = ["Climb and maintain", "Descend and maintain", "Maintain level", "Expect climb"];
  const LEVEL_SHORT: Record<string, string> = { "Climb and maintain": "Climb", "Descend and maintain": "Descend", "Maintain level": "Maintain", "Expect climb": "Expect climb" };
  function pickVerb(t: { label: string; text: string; ra: string }): void {
    lvlVerb = t; ra = t.ra; replyTo = null;
    text = lvlPick ? t.text.replace(/@FL\d+@/, "@FL" + lvlPick + "@") : t.text.replace(/@FL\d+@/, "").trimEnd() + " ";
  }
  function pickLevel(fl: number): void {
    if (!lvlVerb) return;
    lvlPick = fl;
    text = lvlVerb.text.replace(/@FL\d+@/, "@FL" + fl + "@");
  }
  function answer(m: cp.CpdlcMessage, t: { text: string; ra: string }): void { replyTo = m; text = t.text; ra = t.ra; }
  function since(ts: number): string {
    const m = Math.max(0, Math.floor((Date.now() / 1000 - ts) / 60));
    return m < 1 ? "just now" : m < 60 ? m + " min" : Math.floor(m / 60) + " h " + (m % 60) + " min";
  }
  function hhmm(ts: number): string {
    const d = new Date(ts * 1000);
    return String(d.getUTCHours()).padStart(2, "0") + String(d.getUTCMinutes()).padStart(2, "0") + "z";
  }

  async function send(): Promise<void> {
    if (!text.trim() || !shown) return;
    busy = true;
    const err = await cp.sendUplink(shown, text, ra, replyTo ? replyTo.min : null);
    busy = false;
    sendOk = err === "";
    sendMsg = err === "" ? "Sent" : err;
    if (err === "") { text = ""; replyTo = null; lvlVerb = null; lvlPick = 0; refresh(); }
    setTimeout(() => (sendMsg = ""), 5000);
  }
  function key(e: KeyboardEvent): void { if (e.key === "Enter" && !e.shiftKey) { e.preventDefault(); send(); } }

  const stateColour: Record<string, string> = { WILCO: "text-green-300", ROGER: "text-green-300", AFFIRM: "text-green-300", UNABLE: "text-red-400", NEGATIVE: "text-red-400", STANDBY: "text-yellow-300", SENT: "opacity-60", OPEN: "text-yellow-300", ANSWERED: "opacity-60" };

  // Panel width: drag the left edge to change it; remembered on this computer
  const WKEY = "vnaaats.cpdlcWidth";
  let panelW: number = 640;
  try { const w = parseInt(localStorage.getItem(WKEY) ?? ""); if (w >= 380) panelW = w; } catch (e) { /* default */ }
  let dragging: boolean = false;
  function dragStart(e: PointerEvent): void { dragging = true; (e.target as HTMLElement).setPointerCapture(e.pointerId); }
  function dragMove(e: PointerEvent): void {
    if (!dragging) return;
    panelW = Math.max(380, Math.min(window.innerWidth * 0.92, window.innerWidth - e.clientX));
  }
  function dragEnd(): void { dragging = false; try { localStorage.setItem(WKEY, String(Math.round(panelW))); } catch (e) { /* not kept */ } }

  // "More" menu (force log-out lives here)
  let moreOpen: boolean = false;
  function startForce(): void { moreOpen = false; forceStep = 1; }

  // Ready-made messages in groups
  const GROUPS: { name: string; labels: string[] }[] = [
    { name: "Level", labels: ["Climb and maintain", "Descend and maintain", "Maintain level", "Expect climb"] },
    { name: "Contact / squawk", labels: ["Contact", "Squawk"] },
    { name: "Replies", labels: ["Standby", "Roger", "Unable", "Affirm", "Negative"] },
  ];
  $: groupedTemplates = (() => {
    const used = new Set<string>();
    const out = GROUPS.map((g) => ({ name: g.name, items: cp.templates.filter((t) => g.labels.includes(t.label)) }));
    out.forEach((g) => g.items.forEach((t) => used.add(t.label)));
    const rest = cp.templates.filter((t) => !used.has(t.label));
    if (rest.length) out.push({ name: "Other", items: rest });
    return out.filter((g) => g.items.length > 0);
  })();
</script>

<!-- Pull-out CPDLC panel: slides in from the right edge. Aircraft list on the left, messages on the right. -->
<div class="fixed z-40 top-0 bottom-0 right-0 flex flex-row font-regular text-white" style="transform: translateX({$cpdlcOpen ? '0' : '100%'}); opacity: {$cpdlcOpen ? 1 : 0}; transition: transform 0.45s cubic-bezier(0.4, 0, 0.2, 1), opacity 0.45s ease; pointer-events: {$cpdlcOpen ? 'auto' : 'none'};">
  <button class="self-center -mr-px w-7 py-6 flex items-center justify-center font-bold text-sm bg-blue-4 border-2 border-r-0 border-blue-10 hover:bg-blue-10 hover:text-black select-none" style="writing-mode: vertical-rl; transform: rotate(180deg);" title="Close the CPDLC window" on:click={() => cpdlcOpen.set(false)}>Close ✕</button>
  <!-- drag this edge to make the window wider or narrower -->
  <div class="w-1.5 h-full cursor-col-resize bg-blue-10 opacity-40 hover:opacity-100 select-none touch-none" title="Drag to resize" on:pointerdown={dragStart} on:pointermove={dragMove} on:pointerup={dragEnd} on:pointercancel={dragEnd}></div>
  <div class="max-w-screen h-full flex flex-col bg-blue-2 border-l-2 border-blue-10 shadow-5xl" style="width: {panelW}px;">
    <div class="relative flex flex-row justify-between items-center px-3 py-2 bg-blue-3 font-bold select-none">
      <span>CPDLC {$cpdlcStatus.active && $cpdlcStatus.station ? "- station " + $cpdlcStatus.station : ""}</span>
      <span class="flex flex-row gap-2 shrink-0">
        {#if $cpdlcStatus.active}<button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black whitespace-nowrap" on:click={signOut}>Sign out of CPDLC</button>{/if}
        {#if canUse}<button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={() => (moreOpen = !moreOpen)}>More ▾</button>{/if}
        <button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={() => cpdlcOpen.set(false)}>Close</button>
      </span>
      {#if moreOpen}
        <div class="absolute right-12 top-full z-10 mt-1 w-72 bg-blue-3 border-2 border-blue-10 shadow-5xl font-regular font-normal text-sm">
          <button class="block w-full text-left px-3 py-2 hover:bg-blue-10 hover:text-black" on:click={startForce}>Force log out a CPDLC callsign...</button>
        </div>
      {/if}
    </div>
    {#if !$cpdlcStatus.enabled}
      <div class="p-3 text-sm opacity-80">CPDLC is switched off on this server.</div>
    {:else}
      {#if canUse && !$cpdlcStatus.active}
        <div class="p-3 border-b border-blue-4 text-sm">
          {#if !$cpdlcStatus.stationForPosition}
            <div>Your position has no CPDLC station, so you cannot use CPDLC from it.</div>
          {:else if $cpdlcStatus.inUse}
            <div class="text-yellow-300">{$cpdlcStatus.stationForPosition} is already being used by another controller.</div>
          {:else}
            <div class="font-bold mb-1">Sign in to CPDLC as {$cpdlcStatus.stationForPosition}</div>
            <div class="opacity-80 mb-2">Type your own Hoppie logon code. The server keeps it in memory only while you are signed in and never saves it. You can choose to remember it in this browser.</div>
            <form on:submit|preventDefault={signIn}>
              <input type="password" autocomplete="off" class="w-full px-2 py-1 text-black" placeholder="Your Hoppie code" bind:value={hoppieCode} />
              <label class="block mt-1 text-xs opacity-90"><input type="checkbox" bind:checked={remember} /> Remember my code on this computer (stored in this browser only)</label>
              <button class="mt-2 px-4 py-1 font-bold bg-blue-10 text-black" disabled={logonBusy || !hoppieCode.trim()}>{logonBusy ? "Checking..." : "Sign in"}</button>
              {#if hasSaved}<span class="ml-3 underline text-xs hover:cursor-pointer" on:click={forgetSaved}>Forget the saved code</span>{/if}
            </form>
            {#if logonErr}<div class="mt-1 text-yellow-300">{logonErr}</div>{/if}
          {/if}
        </div>
      {/if}
      {#if canUse && $cpdlcStatus.kickedFrom}
        <div class="p-2 border-b border-blue-4 text-sm text-yellow-300">You were force-logged-out of {$cpdlcStatus.kickedFrom} by a controller. Sign in again once the wait below is over.</div>
      {/if}
      {#if canUse && ($cpdlcStatus.lockedSeconds ?? 0) > 0}
        <div class="p-2 border-b border-blue-4 text-sm text-yellow-300">Sign-in to {$cpdlcStatus.stationForPosition} is blocked for {mmss($cpdlcStatus.lockedSeconds ?? 0)} after a forced log out.</div>
      {/if}
      {#if canUse && forceStep > 0}
        <div class="px-3 py-2 border-b border-blue-4 text-sm">
          {#if forceStep === 1}
            <div class="font-bold mb-1">Force log out a CPDLC callsign</div>
            <div class="opacity-80 mb-2">Type the CPDLC callsign to clear, for example CZQX. Use this only if the callsign is stuck because someone left without signing out.</div>
            <input class="w-full px-2 py-1 text-black uppercase" placeholder="CPDLC callsign" bind:value={forceCs} />
            <button class="mt-2 mr-2 px-4 py-1 font-bold bg-blue-10 text-black" disabled={forceCs.trim().length < 3} on:click={() => (forceStep = 2)}>Next</button>
            <button class="mt-2 px-4 py-1 border border-blue-6" on:click={forceCancel}>Cancel</button>
          {:else}
            <div class="font-bold text-yellow-300 mb-1">Warning: force log out of {forceCs.trim().toUpperCase()}</div>
            <ul class="list-disc ml-5 mb-2 text-yellow-300">
              <li>Everyone using {forceCs.trim().toUpperCase()} on this server is logged out of the Hoppie callsign straight away, including a controller who is working traffic on it.</li>
              <li>Aircraft connected through it lose their CPDLC connection and must log on again.</li>
              <li>Nobody, including you, can sign in to {forceCs.trim().toUpperCase()} for 3 minutes afterwards.</li>
              <li>This cannot be undone. Check with the other controller first.</li>
            </ul>
            <div class="mb-1">Type <b>LOG OUT</b> to confirm:</div>
            <input class="w-full px-2 py-1 text-black uppercase" placeholder="LOG OUT" bind:value={forceConfirm} />
            <button class="mt-2 mr-2 px-4 py-1 font-bold bg-red-600 text-white" disabled={forceBusy || forceConfirm.trim().toUpperCase() !== "LOG OUT"} on:click={forceGo}>{forceBusy ? "Working..." : "Force log out now"}</button>
            <button class="mt-2 px-4 py-1 border border-blue-6" on:click={forceCancel}>Cancel</button>
          {/if}
          {#if forceErr}<div class="mt-1 text-yellow-300">{forceErr}</div>{/if}
        </div>
      {/if}
      {#if $cpdlcStatus.error}
        <div class="px-3 py-1 text-xs text-yellow-300">Hoppie: {$cpdlcStatus.error}</div>
      {/if}
      <div class="flex-1 min-h-0 flex flex-col sm:flex-row">
        <!-- connected aircraft: up to 10 rows before the scroll bar; flashing when something new has come in -->
        <div class="sm:w-48 shrink-0 flex flex-col bg-blue-1 border-b sm:border-b-0 sm:border-r border-blue-4 csidebar">
          <div class="px-3 pt-3 pb-2 select-none border-b border-blue-4">
            <div class="flex flex-row items-center justify-between">
              <span class="text-xs font-bold tracking-widest opacity-80">CONNECTED</span>
              <span class="min-w-6 text-center px-1.5 text-xs font-bold bg-blue-10 text-black rounded-full">{connected.length}</span>
            </div>
            {#if $cpdlcStatus.autoAccept}<div class="mt-1.5 inline-block px-2 py-0.5 text-xs border border-green-300 text-green-300 rounded-sm" title="Logon requests are accepted automatically">&#10003; Auto accept</div>{/if}
          </div>
          <div class="clist overflow-y-auto flex flex-row sm:flex-col flex-wrap sm:flex-nowrap gap-1 p-2">
            {#each connected as c}
              <button class="flex flex-row items-center justify-between gap-2 pl-2 pr-2 py-1.5 text-left border-l-4 border-y border-r {c.callsign === shown ? 'bg-blue-3 border-blue-10 border-y-blue-6 border-r-blue-6' : 'bg-blue-2 border-l-blue-5 border-y-blue-3 border-r-blue-3 hover:bg-blue-3'} {(c.unread > 0 || $cpdlcAttention.has(c.callsign)) && !($cpdlcOpen && c.callsign === shown) ? 'nt-flash text-black' : ''}"
                title="Connected {since(c.since)}" on:click={() => pick(c.callsign)}>
                <span class="flex flex-col min-w-0">
                  <span class="text-sm font-bold leading-tight {c.callsign === shown ? 'text-blue-10' : ''}">{c.callsign}</span>
                  <span class="text-xs opacity-60 leading-tight">{since(c.since)}</span>
                </span>
                {#if c.unread > 0}<span class="px-1.5 text-xs font-bold bg-yellow-300 text-black rounded-full">{c.unread}</span>{/if}
              </button>
            {/each}
            {#if connected.length === 0}
              <span class="text-sm opacity-70 px-1">Nobody is connected.{$cpdlcStatus.station ? " Pilots send a logon request to " + $cpdlcStatus.station + "." : ""}</span>
            {/if}
          </div>
          {#if connected.length > 10}<div class="hidden sm:block px-3 py-1 text-xs opacity-60 select-none">Scroll for {connected.length - 10} more</div>{/if}
          <div class="hidden sm:block mt-auto px-3 py-2 text-xs opacity-60 border-t border-blue-4 select-none">A flashing aircraft has a new message or has just logged on.</div>
        </div>
        <div class="flex-1 min-w-0 min-h-0 flex flex-col">
          {#if !canUse}
            <div class="p-3 text-sm opacity-80">Sign in and choose a position to read and send CPDLC messages.</div>
          {:else if !$cpdlcStatus.active}
            <!-- sign-in form is shown above -->
          {:else if shown}
            <div class="flex flex-row items-center justify-between px-3 py-1 font-bold border-b border-blue-4 select-none">
              <span class="flex flex-col"><span class="text-base">{shown}</span>{#if connected.find((c) => c.callsign === shown)}<span class="text-xs font-normal opacity-70">connected {since(connected.find((c) => c.callsign === shown)?.since ?? 0)}{totalMsgs > KEEP ? " \u00b7 last " + KEEP + " of " + totalMsgs + " messages" : ""}</span>{/if}</span>
              <button class="px-2 py-0.5 text-xs font-bold border border-red-400 text-red-300 hover:bg-red-700 hover:text-white"
                title="Log this pilot off CPDLC from your station. The pilot can log on again."
                on:click={() => { if (confirm("Log " + shown + " off CPDLC from your station?")) { cp.disconnectPilot(shown).then((e) => { if (e) loadError = e; }); } }}>Log off pilot</button>
            </div>
            <div class="flex-1 min-h-0 overflow-y-auto px-2 py-1 text-sm" bind:this={thread}>
              {#if loadError}<div class="text-yellow-300">{loadError}</div>{/if}
              {#each messages as m}
                <div class="my-1.5 px-3 py-1.5 border-l-4 rounded-sm shadow-sm {m.dir === 'UP' ? 'border-blue-10 bg-blue-3' : 'border-yellow-300 bg-blue-4'}">
                  <div class="flex flex-row justify-between text-xs opacity-80 tracking-wide">
                    <span>{m.dir === "UP" ? "▲ UPLINK" : "▼ DOWNLINK"} {hhmm(m.ts)}{m.by && m.by !== "auto" ? " " + m.by : ""}</span>
                    <span class={stateColour[m.state] ?? ""}>{m.state === "LOGON" ? "" : m.state}{m.dir === "DOWN" && !m.seen ? " NEW" : ""}</span>
                  </div>
                  <div class="font-bold break-words">{m.readable}</div>
                  {#if m.dir === "DOWN" && m.state === "OPEN"}
                    <div class="flex flex-row flex-wrap gap-2 mt-1.5 text-sm">
                      <button class="px-3 py-1 font-bold bg-red-700 border border-red-300 hover:bg-red-600" on:click={() => answer(m, { text: "UNABLE", ra: "NE" })}>Unable</button>
                      <button class="px-3 py-1 font-bold bg-yellow-300 text-black hover:bg-yellow-200" on:click={() => answer(m, { text: "STANDBY", ra: "NE" })}>Standby</button>
                      <button class="px-3 py-1 font-bold bg-green-700 border border-green-300 hover:bg-green-600" on:click={() => answer(m, { text: "ROGER", ra: "NE" })}>Roger</button>
                      <button class="px-3 py-1 font-bold bg-blue-10 text-black hover:bg-blue-8" on:click={() => answer(m, { text: "", ra: "WU" })}>Reply...</button>
                    </div>
                  {/if}
                </div>
              {/each}
              {#if messages.length === 0 && !loadError}<div class="opacity-70 px-1">No messages yet.</div>{/if}
            </div>
            <div class="px-2 py-2 border-t-2 border-blue-10">
              <div class="flex flex-col gap-1 mb-2">
                {#each groupedTemplates as g}
                  <div class="flex flex-row flex-wrap items-center gap-1">
                    <span class="w-24 shrink-0 text-xs opacity-70 select-none">{g.name}</span>
                    {#if g.name === "Level"}
                      {#each g.items as t}
                        <button class="px-2 py-0.5 text-xs border {lvlVerb?.label === t.label ? 'bg-blue-10 text-black font-bold border-blue-10' : 'bg-blue-3 border-blue-6 hover:bg-blue-4'}" on:click={() => pickVerb(t)}>{LEVEL_SHORT[t.label] ?? t.label}</button>
                      {/each}
                    {:else}
                      {#each g.items as t}
                        <button class="px-2 py-0.5 text-xs bg-blue-3 border border-blue-6 hover:bg-blue-4" on:click={() => useTemplate(t)}>{t.label}</button>
                      {/each}
                    {/if}
                  </div>
                  {#if g.name === "Level"}
                    <div class="flex flex-row flex-wrap items-center gap-1 pl-24">
                      {#each LEVELS as fl}
                        <button class="w-11 py-0.5 text-xs font-bold border {lvlPick === fl && lvlVerb ? 'bg-yellow-300 text-black border-yellow-300' : 'bg-blue-2 border-blue-6 hover:bg-blue-4'} disabled:opacity-40 disabled:hover:bg-blue-2" disabled={!lvlVerb}
                          title={lvlVerb ? "FL" + fl : "Pick Climb, Descend, Maintain or Expect climb first"} on:click={() => pickLevel(fl)}>{fl}</button>
                      {/each}
                      {#if !lvlVerb}<span class="text-xs opacity-60 ml-1">Pick a message first, then a flight level</span>{/if}
                    </div>
                  {/if}
                {/each}
              </div>
              <div class="flex flex-row items-center gap-2 mb-1">
                <select class="text-black text-xs flex-1 min-w-0" title="Other standard messages" on:change={useFree}>
                  <option value="">More standard messages...</option>
                  {#each $cpdlcFreetexts as f, i}<option value={i}>{f.text.length > 60 ? f.text.slice(0, 58) + "..." : f.text}</option>{/each}
                </select>
                {#if !unicomAsk}
                  <button class="px-2 py-0.5 text-xs font-bold bg-yellow-300 text-black" on:click={() => (unicomAsk = true)}>UNICOM</button>
                {:else}
                  <span class="text-xs text-yellow-300">Send [{shown}] to UNICOM?</span>
                  <button class="px-2 py-0.5 text-xs font-bold bg-yellow-300 text-black" on:click={doUnicom}>Yes</button>
                  <button class="px-2 py-0.5 text-xs bg-blue-3 border border-blue-6" on:click={() => (unicomAsk = false)}>No</button>
                {/if}
              </div>
              {#if replyTo}<div class="text-xs text-yellow-300 mb-1">Replying to: {replyTo.readable} <span class="underline hover:cursor-pointer" on:click={() => (replyTo = null)}>cancel</span></div>{/if}
              <textarea class="w-full px-2 py-1 text-black uppercase" rows="2" maxlength="240" placeholder="Message to [{shown}] - @ marks the changeable parts" bind:value={text} on:keydown={key}></textarea>
              <div class="flex flex-row items-center gap-2 mt-1 text-sm">
                <label>Reply wanted
                  <select class="text-black" bind:value={ra}>
                    <option value="WU">Wilco / Unable</option>
                    <option value="AN">Affirm / Negative</option>
                    <option value="R">Roger</option>
                    <option value="NE">None</option>
                  </select>
                </label>
                <button class="ml-auto px-4 py-1 font-bold bg-blue-10 text-black" disabled={busy || !text.trim()} on:click={send}>Send</button>
              </div>
              {#if sendMsg}<div class="text-xs mt-1 {sendOk ? 'text-green-300' : 'text-yellow-300'}">{sendMsg}</div>{/if}
            </div>
          {:else}
            <div class="p-3 text-sm opacity-80">Select a connected aircraft on the left.</div>
          {/if}
        </div>
      </div>
    {/if}
  </div>
</div>

<style>
  /* about ten aircraft show before the scroll bar (each row is roughly 3.1rem with its gap) */
  .csidebar { max-height: 10rem; }
  .clist { max-height: 6.5rem; }
  @media (min-width: 640px) { .csidebar { max-height: none; } .clist { max-height: 30.7rem; } }
</style>
