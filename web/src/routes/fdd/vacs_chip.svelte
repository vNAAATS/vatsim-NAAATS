<script lang="ts">
  import { onMount, onDestroy } from "svelte";
  import * as v from "../../typescript/fdd/vacs";
  const { vacs } = v;

  let open: boolean = false;
  let en: boolean = v.isEnabled();
  let port: string = String(v.getPort());
  let blink: boolean = true;
  let blinkTimer: any = null;

  onMount(() => { v.start(); blinkTimer = setInterval(() => (blink = !blink), 500); });
  onDestroy(() => { v.stop(); clearInterval(blinkTimer); });

  function apply(): void {
    const p = parseInt(port, 10);
    v.configure(en, p > 0 && p < 65536 ? p : 9600);
    open = false;
  }

  $: ringing = $vacs.ringing;
  $: showPop = !!ringing && $vacs.dismissed !== ringing.callId;
  $: state = !$vacs.enabled ? "disabled" : ringing ? "ring" : $vacs.call ? "call" : $vacs.link === "up" ? "up" : "off";
  $: text = state === "disabled" ? "VACS" : state === "ring" ? "Call: " + ringing!.from : state === "call" ? "In call" + ($vacs.call!.with ? ": " + $vacs.call!.with : "")
    : state === "up" ? ($vacs.onPosition ? "VACS" : "VACS (no position)") : "VACS offline";
  $: tip = state === "disabled" ? "VACS call alert is switched off. Click to set it up."
    : state === "off" ? "Cannot reach VACS. Switch on Remote Control in VACS (Settings) and check the port. Click for help."
    : state === "up" ? ($vacs.onPosition ? "Connected to VACS. You will see incoming calls here. Click for options." : "Connected to VACS, but VACS is not connected to a position yet.")
    : "";
</script>

<div class="relative inline-block">
  <button class="tbtn border {state === 'ring' ? (blink ? 'bg-red-600 border-red-300 text-white font-bold' : 'bg-blue-3 border-red-300 font-bold') : state === 'call' ? 'bg-blue-3 border-blue-10 text-blue-10 font-bold' : state === 'up' ? 'bg-blue-3 border-blue-6' : 'bg-blue-3 border-blue-6 opacity-60'}"
    title={tip} on:click={() => (open = !open)} aria-label="VACS call indicator">
    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke={state === "up" ? "#7fe0a2" : "currentColor"} stroke-width="2" aria-hidden="true"><path d="M4 15v-3a8 8 0 0 1 16 0v3" /><rect x="3" y="14" width="4" height="6" rx="1.5" /><rect x="17" y="14" width="4" height="6" rx="1.5" /></svg>
    <span>{text}</span>
  </button>

  {#if showPop}
    <div class="absolute right-0 top-full mt-1 z-50 w-72 p-3 border-2 border-red-300 bg-blue-2 text-white shadow-lg" role="alert">
      <div class="font-bold text-base">VACS: incoming {ringing?.prio ? "PRIORITY " : ""}call</div>
      <div class="my-1">from <b>{ringing?.from}</b></div>
      <button class="mr-2 mt-1 px-3 py-0.5 font-bold bg-green-300 text-black" on:click={v.accept}>Accept</button>
      <button class="mt-1 px-3 py-0.5 border border-blue-6" on:click={v.dismiss}>Dismiss</button>
      <div class="mt-1 text-xs opacity-70">Dismiss only hides this box; the call keeps ringing in VACS.</div>
    </div>
  {/if}

  {#if open}
    <div class="absolute right-0 top-full mt-1 z-50 w-80 p-3 border-2 border-blue-10 bg-blue-2 text-white text-sm shadow-lg">
      <div class="font-bold mb-1">VACS call alert</div>
      <div class="mb-2 opacity-90">Shows when VACS has an incoming call. It listens to VACS on this computer only; nothing is sent to our server.</div>
      <label class="flex items-center gap-2 mb-2"><input type="checkbox" bind:checked={en} /> Switch the alert on</label>
      <label class="flex items-center gap-2 mb-2">VACS port <input class="w-20 px-1 text-black" bind:value={port} inputmode="numeric" /></label>
      <div class="mb-2 text-xs opacity-80">In VACS (2.1.0 or newer) open <b>Advanced Settings</b> and tick <b>Enable remote control</b>; it is off by default and uses port 9600. For safety set <b>Listen addr.</b> to <b>127.0.0.1:9600</b> so only this computer can connect. Your browser may ask to allow access to devices on your network: choose Allow.</div>
      <button class="px-3 py-0.5 font-bold bg-blue-10 text-black" on:click={apply}>Save</button>
      <button class="ml-2 px-3 py-0.5 border border-blue-6" on:click={() => (open = false)}>Close</button>
    </div>
  {/if}
</div>
