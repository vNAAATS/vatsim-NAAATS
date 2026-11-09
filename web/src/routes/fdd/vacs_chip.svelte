<script lang="ts">
  import { fly } from "svelte/transition";
  import { ms } from "../../typescript/fdd/motion";
  import { onMount, onDestroy } from "svelte";
  import * as v from "../../typescript/fdd/vacs";
  const { vacs } = v;

  let open: boolean = false;
  let tab: "calls" | "dial" | "setup" = "calls";
  let en: boolean = v.isEnabled();
  const { ringMuted } = v;
  let port: string = String(v.getPort());
  let find: string = "";
  let blink: boolean = true;
  let blinkTimer: any = null;
  let now: number = Date.now();
  let clock: any = null;

  onMount(() => { v.start(); blinkTimer = setInterval(() => (blink = !blink), 500); clock = setInterval(() => (now = Date.now()), 1000); });
  onDestroy(() => { v.stop(); clearInterval(blinkTimer); clearInterval(clock); });

  function apply(): void {
    const p = parseInt(port, 10);
    v.configure(en, p > 0 && p < 65536 ? p : 9600);
    open = false;
  }
  function hhmmss(t: number): string { return new Date(t).toISOString().slice(11, 19) + "Z"; }
  function hhmm(t: number): string { return new Date(t).toISOString().slice(11, 16) + "Z"; }
  function dur(ms: number): string {
    const s = Math.max(0, Math.round(ms / 1000));
    return s >= 3600 ? Math.floor(s / 3600) + "h " + String(Math.floor((s % 3600) / 60)).padStart(2, "0") + "m" : Math.floor(s / 60) + ":" + String(s % 60).padStart(2, "0");
  }
  function name(e: { callsign: string; who: string; cid: string }): string { return e.callsign || e.who || e.cid || "unknown"; }
  // positions people usually ring first from an oceanic position
  const PINNED = ["EGGX", "CZQX", "BIRD", "LPPO", "ENOB", "EISN", "EGPX", "CZQM", "CZQO", "NAT", "EGTT", "EGPF", "KZNY", "CZUL"];
  function pinned(c: v.Online): boolean { const k = (c.callsign || c.position).toUpperCase(); return PINNED.some((p) => k.startsWith(p)); }

  $: ringing = $vacs.ringing;
  $: showPop = !!ringing && $vacs.dismissed !== ringing.callId;
  $: state = !$vacs.enabled ? "disabled" : ringing ? "ring" : $vacs.call ? "call" : $vacs.link === "up" ? ($vacs.busy ? "busy" : "up") : "off";
  $: text = state === "disabled" ? "VACS" : state === "ring" ? "Call: " + (ringing!.callsign || ringing!.from) : state === "call" ? "In call" + ($vacs.call!.with ? ": " + $vacs.call!.with : "")
    : state === "busy" ? "VACS: Busy" : state === "up" ? ($vacs.onPosition ? "VACS" : "VACS (no position)") : "VACS offline";
  $: tip = state === "disabled" ? "VACS call alert is switched off. Click to set it up."
    : state === "off" ? "Cannot reach VACS. Switch on Remote Control in VACS (Settings) and check the port. Click for help."
    : state === "busy" ? "Busy is on: incoming calls are declined automatically. Click to turn it off."
    : state === "up" ? ($vacs.onPosition ? "Connected to VACS. Incoming calls show here. Click for the call log and to call someone." : "Connected to VACS, but VACS is not connected to a position yet.")
    : "";
  $: missed = $vacs.log.filter((e) => e.dir === "in" && e.result === "missed" && now - e.at < 3600_000).length;
  $: q = find.trim().toUpperCase();
  $: list = $vacs.online.filter((c) => !q || (c.callsign + " " + c.position + " " + c.cid).toUpperCase().includes(q));
  $: top = list.filter(pinned);
  $: rest = list.filter((c) => !pinned(c));
</script>

<div class="relative inline-block">
  <button class="tbtn border {state === 'ring' ? (blink ? 'bg-red-600 border-red-300 text-white font-bold' : 'bg-blue-3 border-red-300 font-bold') : state === 'call' ? 'bg-blue-3 border-blue-10 text-blue-10 font-bold' : state === 'up' ? 'bg-blue-3 border-blue-6' : state === 'busy' ? 'bg-yellow-600 border-yellow-300 text-black font-bold' : 'bg-blue-3 border-blue-6 opacity-60'}"
    title={tip} on:click={() => { open = !open; if (open && $vacs.link !== "up") tab = "setup"; else if (open && tab === "setup") tab = "calls"; }} aria-label="VACS call indicator">
    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke={state === "up" ? "#7fe0a2" : "currentColor"} stroke-width="2" aria-hidden="true"><path d="M4 15v-3a8 8 0 0 1 16 0v3" /><rect x="3" y="14" width="4" height="6" rx="1.5" /><rect x="17" y="14" width="4" height="6" rx="1.5" /></svg>
    <span>{text}</span>
    {#if state === "call" && $vacs.call && $vacs.call.since}<span class="font-mono">{dur(now - $vacs.call.since)}</span>{/if}
    {#if missed > 0 && state !== "ring"}<span class="ml-1 px-1 rounded bg-red-600 text-white text-xs font-bold" title="{missed} missed call{missed === 1 ? '' : 's'} in the last hour">{missed}</span>{/if}
  </button>

  {#if showPop}
    <div class="absolute left-0 top-full mt-1 z-50 w-80 max-w-[95vw] p-3 border-2 border-red-300 bg-blue-2 text-white shadow-lg" role="alert">
      <div class="font-bold text-base">VACS: incoming {ringing?.prio ? "PRIORITY " : ""}call</div>
      <div class="my-1 text-lg"><b>{ringing?.callsign || ringing?.from}</b></div>
      <div class="text-sm opacity-90">
        {#if ringing?.callsign && ringing?.from && ringing.from !== ringing.callsign}Position {ringing.from}<br />{/if}
        {#if ringing?.cid}VATSIM CID {ringing.cid}<br />{/if}
        Rang at {ringing ? hhmmss(ringing.at) : ""} ({ringing ? Math.max(0, Math.round((now - ringing.at) / 1000)) : 0} s ago)
      </div>
      <button class="mr-2 mt-2 px-3 py-0.5 font-bold bg-green-300 text-black" on:click={v.accept}>Accept</button>
      <button class="mr-2 mt-2 px-3 py-0.5 font-bold bg-red-600 text-white" title="Turns the call away" on:click={v.decline}>Decline</button>
      <button class="mt-2 px-3 py-0.5 border border-blue-6" on:click={v.dismiss}>Dismiss</button>
      <div class="mt-1 text-xs opacity-70">Decline turns the call away. Dismiss only hides this box; the call keeps ringing in VACS.</div>
    </div>
  {/if}

  {#if open}
    <div transition:fly={{ y: -6, duration: ms(140) }} class="absolute left-0 top-full mt-1 z-50 w-96 max-w-[95vw] border-2 border-blue-10 bg-blue-2 text-white text-sm shadow-lg">
      <div class="flex border-b border-blue-6">
        <button class="px-3 py-1 {tab === 'calls' ? 'bg-blue-3 font-bold' : 'opacity-80'}" on:click={() => (tab = "calls")}>Calls</button>
        <button class="px-3 py-1 {tab === 'dial' ? 'bg-blue-3 font-bold' : 'opacity-80'}" on:click={() => (tab = "dial")}>Call someone</button>
        <button class="px-3 py-1 {tab === 'setup' ? 'bg-blue-3 font-bold' : 'opacity-80'}" on:click={() => (tab = "setup")}>Setup</button>
        <button class="ml-auto px-3 py-1 opacity-80" on:click={() => (open = false)} aria-label="Close">×</button>
      </div>

      {#if tab === "calls"}
        <div class="p-3">
          <div class="mb-2 p-2 border border-blue-6 flex items-center gap-2 {$vacs.busy ? 'bg-yellow-600 text-black' : ''}">
            <div class="flex-1"><b>Busy</b> <span class="text-xs">{$vacs.busy ? "On: new calls are declined automatically" : "Off: calls ring as normal"}</span></div>
            <button class="px-2 py-0.5 font-bold border {$vacs.busy ? 'border-black' : 'border-blue-6'}" on:click={() => v.setBusy(!$vacs.busy)}>{$vacs.busy ? "Turn off" : "Turn on"}</button>
          </div>
          {#if $vacs.busy}
            <label class="flex items-center gap-2 mb-2 text-xs"><input type="checkbox" checked={$vacs.busyPrio} on:change={(e) => v.setBusy(true, e.currentTarget.checked)} /> Still ring me for priority calls</label>
          {/if}
          {#if $vacs.call}
            <div class="mb-2 p-2 border border-blue-10 bg-blue-3 flex items-center">
              <div class="flex-1"><b>In call{$vacs.call.with ? " with " + $vacs.call.with : ""}</b>{#if $vacs.call.since} <span class="font-mono">{dur(now - $vacs.call.since)}</span>{/if}</div>
              <button class="px-2 py-0.5 font-bold bg-red-600 text-white" on:click={v.hangup}>End call</button>
            </div>
          {/if}
          {#if $vacs.log.length === 0}
            <div class="opacity-80">No calls yet. Calls you take, miss or place while FDD is open will be listed here, newest first.</div>
          {:else}
            <div class="max-h-72 overflow-y-auto">
              {#each $vacs.log as e (e.callId)}
                <div class="flex items-center gap-2 py-1 border-b border-blue-6">
                  <span class="w-5 text-center font-bold {e.dir === 'in' ? (e.result === 'missed' || e.result === 'declined' ? 'text-red-300' : 'text-green-300') : 'text-blue-10'}" title={e.dir === "in" ? "Incoming" : "Outgoing"}>{e.dir === "in" ? (e.result === "missed" || e.result === "declined" ? "✕" : "↙") : "↗"}</span>
                  <div class="flex-1 min-w-0">
                    <div class="truncate"><b>{name(e)}</b>{#if e.prio} <span class="ml-1 text-red-300 text-xs font-bold">PRIORITY</span>{/if}</div>
                    <div class="text-xs opacity-75 truncate">{e.who && e.who !== name(e) ? e.who + " · " : ""}{e.cid ? "CID " + e.cid : ""}</div>
                  </div>
                  <div class="text-right text-xs">
                    <div class="font-mono">{hhmm(e.at)}</div>
                    <div class="opacity-80">{e.result === "ringing" ? "ringing" : e.result === "declined" ? "declined" : e.result === "answered" ? "in call" : e.answeredAt ? dur((e.endedAt || now) - e.answeredAt) : e.result}</div>
                  </div>
                  {#if e.cid && e.cid !== ""}<button class="px-1.5 py-0.5 border border-blue-6 text-xs" title="Call back" disabled={!$vacs.onPosition || !!$vacs.call || !!$vacs.ringing} on:click={() => v.dial(e.cid)}>Call</button>{/if}
                </div>
              {/each}
            </div>
            <button class="mt-2 text-xs underline opacity-80" on:click={v.clearLog}>Clear list</button>
          {/if}
        </div>

      {:else if tab === "dial"}
        <div class="p-3">
          {#if $vacs.link !== "up" || !$vacs.onPosition}
            <div class="opacity-90">VACS must be running and connected to a position to place a call. See Setup.</div>
          {:else}
            <input class="w-full px-2 py-1 mb-2 text-black" placeholder="Search callsign, position or CID" bind:value={find} />
            {#if $vacs.callError}<div class="mb-2 p-1 text-red-300">{$vacs.callError}</div>{/if}
            {#if list.length === 0}
              <div class="opacity-80">{$vacs.online.length === 0 ? "Nobody listed as online yet. VACS sends the list a moment after it connects." : "No match."}</div>
            {:else}
              <div class="max-h-72 overflow-y-auto">
                {#each top as c (c.cid)}
                  <div class="flex items-center gap-2 py-1 border-b border-blue-6">
                    <div class="flex-1 min-w-0"><b>{c.callsign || c.position}</b> <span class="text-xs opacity-75">{c.position && c.position !== c.callsign ? c.position : ""} {c.freq}</span></div>
                    <button class="px-2 py-0.5 font-bold bg-green-300 text-black" disabled={!!$vacs.call || !!$vacs.ringing} on:click={() => v.dial(c.cid)}>Call</button>
                  </div>
                {/each}
                {#if top.length && rest.length}<div class="mt-2 mb-1 text-xs uppercase opacity-70">Everyone else online</div>{/if}
                {#each rest as c (c.cid)}
                  <div class="flex items-center gap-2 py-1 border-b border-blue-6">
                    <div class="flex-1 min-w-0 truncate"><b>{c.callsign || c.position}</b> <span class="text-xs opacity-75">{c.position && c.position !== c.callsign ? c.position : ""} {c.freq}</span></div>
                    <button class="px-2 py-0.5 border border-blue-6" disabled={!!$vacs.call || !!$vacs.ringing} on:click={() => v.dial(c.cid)}>Call</button>
                  </div>
                {/each}
              </div>
            {/if}
            <div class="mt-2 text-xs opacity-70">Rings the controller through VACS, the same as clicking them in VACS. Needs VACS 2.1.0 or newer.</div>
          {/if}
        </div>

      {:else}
        <div class="p-3">
          <div class="font-bold mb-1">VACS call alert</div>
          <div class="mb-2 opacity-90">Shows incoming calls, keeps a call log and lets you ring people. It talks to VACS on this computer only; nothing is sent to our server.</div>
          <label class="flex items-center gap-2 mb-2"><input type="checkbox" bind:checked={en} /> Switch the alert on</label>
          <label class="flex items-center gap-2 mb-2"><input type="checkbox" checked={!$ringMuted} on:change={(e) => v.setRingMuted(!e.currentTarget.checked)} /> Ring once when a call comes in <button class="px-2 border border-blue-6" on:click={() => { v.setRingMuted(false); v.ringOnce(); }}>Test</button></label>
          <label class="flex items-center gap-2 mb-2">VACS port <input class="w-20 px-1 text-black" bind:value={port} inputmode="numeric" /></label>
          <div class="mb-2 text-xs opacity-80">In VACS (2.1.0 or newer) open <b>Advanced Settings</b> and tick <b>Enable remote control</b>; it is off by default and uses port 9600. For safety set <b>Listen addr.</b> to <b>127.0.0.1:9600</b> so only this computer can connect. Your browser may ask to allow access to devices on your network: choose Allow.</div>
          <button class="px-3 py-0.5 font-bold bg-blue-10 text-black" on:click={apply}>Save</button>
          <button class="ml-2 px-3 py-0.5 border border-blue-6" on:click={() => (open = false)}>Close</button>
        </div>
      {/if}
    </div>
  {/if}
</div>
