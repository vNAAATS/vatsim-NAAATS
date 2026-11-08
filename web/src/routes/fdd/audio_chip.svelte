<script lang="ts">
  import { onMount, onDestroy } from "svelte";
  import * as t from "../../typescript/fdd/trackaudio";
  import * as dataHandler from "../../typescript/fdd/data";
  import { popOut } from "../../typescript/fdd/popout";
  const { ta, highlightOn, scrollOn } = t;

  export let standalone: boolean = false;      // shown as a whole window (pop-out) instead of a chip
  let open: boolean = false;
  let tick: number = 0;
  let timer: any;
  onMount(() => { t.start(); timer = setInterval(() => (tick = tick + 1), 1000); });
  onDestroy(() => { clearInterval(timer); });

  $: last = t.lastRx($ta);
  $: ago = (tick, last ? Math.max(0, Math.floor(Date.now() / 1000 - last.ts)) : 0);
  function agoText(s: number): string { return s < 60 ? s + "s ago" : s < 3600 ? Math.floor(s / 60) + " min ago" : Math.floor(s / 3600) + " h ago"; }
  $: rxPilot = $ta.rx && !t.isAtc($ta.rx.who) ? $ta.rx : null;      // a controller's call is not "received" by you
  $: text = !$ta.enabled ? "Audio" : $ta.link !== "up" ? "TrackAudio offline" : rxPilot ? "RX: " + rxPilot.who : $ta.tx ? "TX" : last ? "Last RX: " + last.who + " " + agoText(ago) : "TrackAudio";
  function select(who: string): void { if (who && who !== "You") dataHandler.setAsel(who); }
  async function pop(): Promise<void> { await popOut("audio"); open = false; }
</script>

{#if !standalone}
<div class="relative inline-block">
  <button class="tbtn border w-56 overflow-hidden whitespace-nowrap {rxPilot ? 'bg-green-700 border-green-300 font-bold' : $ta.tx ? 'bg-yellow-300 text-black border-yellow-300 font-bold' : $ta.link === 'up' ? 'bg-blue-3 border-blue-6' : 'bg-blue-3 border-blue-6 opacity-60'}"
    title="Who spoke last on your frequencies (TrackAudio). Click for the list." on:click={() => (open = !open)}>
    <span class="inline-block shrink-0 w-2 h-2 rounded-full {$ta.link === 'up' ? 'bg-green-300' : 'bg-gray-400'}"></span><span class="truncate">{text}</span>
  </button>
  {#if open}
    <div class="absolute left-0 top-full mt-1 z-50 w-80 max-w-[95vw] border-2 border-blue-10 bg-blue-2 text-white shadow-lg font-regular text-sm">
      <svelte:self standalone={true} />
    </div>
  {/if}
</div>
{:else}
  <div class="bg-blue-2 text-white font-regular text-sm h-full flex flex-col">
    <div class="px-3 py-2 bg-blue-3 font-bold flex flex-row justify-between items-center select-none">
      <span>Who spoke last (TrackAudio)</span>
      <button class="px-2 py-0.5 text-xs border border-blue-10 hover:bg-blue-10 hover:text-black" on:click={pop} title="Open in its own window">Pop out</button>
    </div>
    <div class="px-3 py-2 border-b border-blue-4 text-xs opacity-90">
      {#if !$ta.enabled}
        Not switched on. This shows the callsign, frequency and time of each call heard on your TrackAudio frequencies. No audio is recorded and nothing leaves this computer.
        <button class="mt-2 block px-3 py-1 font-bold bg-blue-10 text-black" on:click={() => t.setEnabled(true)}>Switch on</button>
      {:else if $ta.link !== "up"}
        TrackAudio is not answering on this computer (127.0.0.1:49080). Start TrackAudio; this reconnects by itself.
        <button class="mt-2 block px-3 py-1 border border-blue-6" on:click={() => t.setEnabled(false)}>Switch off</button>
      {:else}
        Connected to TrackAudio. Kept on this computer only, for 24 hours.
      {/if}
    </div>
    <div class="flex-1 min-h-0 overflow-y-auto">
      {#each $ta.history.slice(0, 50) as h}
        <div class="flex flex-row items-center justify-between px-3 py-1.5 border-b border-blue-4">
          <span class="flex flex-row items-center gap-2">
            <span class="px-1 text-xs border {h.mine ? 'border-blue-6' : t.isAtc(h.who) ? 'border-blue-10 text-blue-10' : 'border-green-300 text-green-300'}">{h.mine ? "TX" : t.isAtc(h.who) ? "ATC" : "RX"}</span>
            {#if h.mine}<b>You</b>{:else if t.isAtc(h.who)}<b>{h.who}</b>{:else}<button class="font-bold underline" title="Select this aircraft" on:click={() => select(h.who)}>{h.who}</button>{/if}
            {#if h.freq}<span class="text-xs opacity-70">{h.freq}</span>{/if}
          </span>
          <span class="text-xs opacity-80">{t.hhmmss(h.ts)} - {h.secs}s</span>
        </div>
      {/each}
      {#if $ta.history.length === 0}<div class="px-3 py-3 opacity-70">Nothing heard yet.</div>{/if}
    </div>
    {#if $ta.enabled}
      <label class="px-3 py-2 flex flex-row items-center gap-2 border-t border-blue-4 text-xs select-none" title="Draws a green box around the aircraft's row in the flight list while it is transmitting, then fades it out">
        <input type="checkbox" checked={$highlightOn} on:change={(e) => t.setHighlight(e.currentTarget.checked)} />
        Highlight the aircraft that is transmitting
      </label>
      <label class="px-3 py-2 flex flex-row items-center gap-2 border-t border-blue-4 text-xs select-none" title="Scrolls the flight list to the aircraft that starts transmitting, if its row is not already on screen">
        <input type="checkbox" checked={$scrollOn} on:change={(e) => t.setScroll(e.currentTarget.checked)} />
        Scroll the list to the aircraft that is transmitting
      </label>
      <div class="px-3 py-2 flex flex-row gap-2 border-t border-blue-4">
        <button class="px-3 py-0.5 text-xs border border-blue-6 hover:bg-blue-4" on:click={t.clearHistory}>Clear history</button>
        <button class="px-3 py-0.5 text-xs border border-blue-6 hover:bg-blue-4" on:click={() => t.setEnabled(false)}>Switch off</button>
      </div>
    {/if}
  </div>
{/if}
