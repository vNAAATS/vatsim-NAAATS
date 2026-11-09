<script lang="ts">
  import { showFlightPlan } from "../../typescript/fdd/selectac";
  import { fade, fly } from "svelte/transition";
  import { flip } from "svelte/animate";
  import { ms } from "../../typescript/fdd/motion";
  import { onMount, onDestroy } from "svelte";
  import * as sess from "../../typescript/fdd/session";
  import * as cp from "../../typescript/fdd/cpdlc";
  import * as m from "../../typescript/fdd/acmenu";
  import * as h from "../../typescript/fdd/handoff";

  const { session } = sess;
  const { handoffCards, handoffSoundOn, handoffAuto, autoLog } = h;
  let timer: any = null;
  let picked: Record<string, string> = {};   // callsign -> chosen position
  let note: Record<string, string> = {};
  let busy: Record<string, boolean> = {};
  let open: boolean = false;                  // the list of handoff messages (opened by clicking the flashing button)
  $: if ($handoffCards.length === 0) open = false;
  $: soonest = $handoffCards.length ? [...$handoffCards].sort((a, b) => a.nm - b.nm)[0] : null;

  onMount(() => { h.update($session.position ?? ""); timer = setInterval(() => h.update($session.position ?? ""), 2000); });
  onDestroy(() => { if (timer) clearInterval(timer); });

  function choice(c: h.HandoffCard): string { return picked[c.callsign] ?? c.online[0]?.callsign ?? ""; }
  async function transfer(c: h.HandoffCard): Promise<void> {
    const o = c.online.find((x) => x.callsign === choice(c));
    if (!o) return;
    busy[c.callsign] = true; note[c.callsign] = ""; busy = busy; note = note;
    const id = await m.queueCommand({ type: "transfer", callsign: c.callsign, target: o.callsign, freq: o.frequency });
    if (typeof id === "string") note[c.callsign] = "✖ " + id;
    else {
      note[c.callsign] = "Sending to EuroScope...";
      for (let i = 0; i < 10; i++) {
        await new Promise((r) => setTimeout(r, 1000));
        const s = await m.commandState(id);
        if (s.state === "ok") { h.dismissCard(c); busy[c.callsign] = false; return; }
        if (s.state === "failed") { note[c.callsign] = "✖ " + s.msg; break; }
      }
      if (!note[c.callsign].startsWith("✖")) note[c.callsign] = "✖ EuroScope did not answer. Hand off in EuroScope instead.";
    }
    busy[c.callsign] = false; busy = busy; note = note;
  }
  async function unicom(c: h.HandoffCard): Promise<void> {
    busy[c.callsign] = true; busy = busy;
    const e = await cp.sendUnicom(c.callsign);
    note[c.callsign] = e ? "✖ " + e : "";
    busy[c.callsign] = false; busy = busy; note = note;
    if (!e) h.dismissCard(c);
  }
</script>

<button class="tbtn border {$handoffAuto.on ? 'bg-blue-3 border-blue-6' : 'bg-blue-3 border-red-400 text-red-200'}" disabled={!sess.canEdit($session)}
  title={"Auto handoff is " + ($handoffAuto.on ? "ON" : "OFF") + ". On: for aircraft you track that are on CPDLC, the transfer is sent for you " + $handoffAuto.nm + " nm from the boundary when a position is online to take it; with nobody online the UNICOM message is sent at the boundary and the tag is released when the pilot answers. Needs this display to stay open. " + ($autoLog || "")}
  on:click={() => h.setHandoffAuto(!$handoffAuto.on)}><span class="inline-block w-2 h-2 rounded-full {$handoffAuto.on ? 'bg-green-300' : 'bg-red-400'}"></span>Auto handoff: {$handoffAuto.on ? "ON" : "OFF"}</button>
{#if $handoffAuto.on}
  <select class="tbtn border border-blue-6 bg-blue-3 text-white" value={String($handoffAuto.nm)} title="Distance from the boundary at which the transfer is sent" on:change={(e) => h.setHandoffAuto(true, parseInt(e.currentTarget.value, 10))}>
    {#each [10, 20, 30, 40, 50] as n}<option value={String(n)}>{n} nm</option>{/each}
  </select>
{/if}
{#if $autoLog}<span class="text-xs text-yellow-300 max-w-xs truncate" title={$autoLog}>{$autoLog}</span>{/if}

{#if $handoffCards.length > 0}
  <button class="tbtn border border-yellow-300 font-bold ho-flash text-black" aria-live="polite"
    title="Click to read the handoff message{$handoffCards.length === 1 ? '' : 's'} and act on {$handoffCards.length === 1 ? 'it' : 'them'}" on:click={() => (open = !open)}>
    &#8644; Handoff: {soonest ? soonest.callsign : ""}{$handoffCards.length > 1 ? " +" + ($handoffCards.length - 1) : ""}
  </button>
  {#if open}
    <div transition:fade={{ duration: ms(120) }} class="fixed inset-0 z-40" on:click={() => (open = false)}></div>
    <div class="fixed right-3 z-50 flex flex-col gap-2 w-96 max-w-[95vw] max-h-[70vh] overflow-y-auto font-web text-white" style="top: 13rem;" role="dialog" aria-label="Handoff messages">
      {#each $handoffCards as c (c.callsign + c.kind)}
        <div animate:flip={{ duration: ms(200) }} in:fly={{ x: 28, duration: ms(240) }} out:fade={{ duration: ms(160) }} class="bg-blue-2 border-2 border-yellow-300 shadow-lg" role="alertdialog" aria-label="Handoff {c.callsign}">
          <div class="px-3 py-1 bg-yellow-300 text-black font-bold flex items-center">
            <span>{c.kind === "near" ? "Handoff soon" : "Leaving oceanic airspace"}: <button class="underline" title="Show the flight plan" on:click={() => showFlightPlan(c.callsign)}>{c.callsign}</button></span>
            <button class="ml-auto px-2 text-xs border border-black" on:click={() => h.dismissCard(c)} aria-label="Dismiss">Dismiss</button>
          </div>
          <div class="p-3 text-sm">
            {#if c.kind === "near"}
              <div class="mb-2"><b>{c.callsign}</b> is nearing the sector boundary (<b>{c.nm} nm</b> to {c.fix}) to <b>{c.name}</b>.</div>
            {:else}
              <div class="mb-2"><b>{c.callsign}</b> is leaving oceanic airspace at {c.fix}.{c.online.length === 0 ? " No adjacent controller is online (" + c.name + ")." : ""}</div>
            {/if}
            <!-- both ways of handing the aircraft on are always here: transfer to a controller who is online, or hand off to UNICOM -->
            {#if c.online.length > 0}
              <select class="w-full px-1 py-0.5 text-black mb-2" value={choice(c)} on:change={(e) => { picked[c.callsign] = e.currentTarget.value; picked = picked; }}>
                {#each c.online as o}<option value={o.callsign}>{o.code ? o.code + " " : ""}{o.callsign} {o.frequency}</option>{/each}
              </select>
            {/if}
            <div class="flex flex-row flex-wrap gap-2">
              <button class="px-3 py-1 font-bold bg-blue-10 text-black disabled:opacity-40" disabled={c.online.length === 0 || busy[c.callsign] || !sess.canEdit($session)}
                title={c.online.length === 0 ? "Nobody is online to transfer to" : "Transfer in EuroScope to the controller chosen above"} on:click={() => transfer(c)}>Transfer</button>
              {#if c.cpdlc}
                <button class="px-3 py-1 font-bold bg-yellow-300 text-black disabled:opacity-40" disabled={busy[c.callsign] || !sess.canEdit($session)}
                  title="Send the CPDLC message to monitor UNICOM 122.800 and release the tag when the pilot answers" on:click={() => unicom(c)}>Hand off to UNICOM</button>
              {/if}
            </div>
            {#if !c.cpdlc}<div class="mt-2 opacity-90">Not on CPDLC: for UNICOM, tell the pilot on voice or text to monitor 122.800, then release the tag.</div>{/if}
            {#if c.error && !note[c.callsign]}<div class="mt-2 text-yellow-300">&#10006; Automatic handoff failed: {c.error}. Do it here or in EuroScope.</div>{/if}
            {#if note[c.callsign]}<div class="mt-2 text-yellow-300">{note[c.callsign]}</div>{/if}
            <label class="mt-2 flex items-center gap-2 text-xs opacity-80"><input type="checkbox" checked={$handoffSoundOn} on:change={(e) => h.setHandoffSound(e.currentTarget.checked)} /> Play a sound for handoff prompts</label>
          </div>
        </div>
      {/each}
    </div>
  {/if}
{/if}

<style>
  /* slow flash (about 2 seconds a cycle) so it catches the eye without being a strobe */
  :global(.ho-flash) { animation: hoflash 2s ease-in-out infinite; }
  @keyframes -global-hoflash {
    0%, 100% { background-color: #fde047; }
    50% { background-color: #a16207; color: #fff; }
  }
  @media (prefers-reduced-motion: reduce) { :global(.ho-flash) { animation: none; background-color: #fde047; } }
</style>
