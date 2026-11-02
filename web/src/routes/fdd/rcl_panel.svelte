<script lang="ts">
  import { fly } from "svelte/transition";
  import * as sess from "../../typescript/fdd/session";
  const { session, rclAuto, rclPanelOpen } = sess;

  let msg: string = "";
  let busy: boolean = false;
  $: me = ($session.position ?? "").toUpperCase();
  $: amSender = me !== "" && me === $rclAuto.sender;
  $: ask = $rclAuto.prompt && amSender;                                   // the box that asks "send the reminders now?"
  $: b = $rclAuto.batch;
  $: sending = $rclAuto.sending > 0 || $rclAuto.inflight > 0;             // typing in progress on the sender's EuroScope
  $: finished = !sending && b.total > 0 && (b.done + b.failed.length) >= b.total;
  let hidden: boolean = false;                                           // Close always works, even while "sending"
  $: if (!ask && !(sending && amSender)) hidden = false;
  $: show = ((ask || (sending && amSender)) && !hidden) || $rclPanelOpen;

  function when(w: sess.RclWaiting): string {
    if (w.minutes === null) return "no entry time yet";
    return (w.time ? "entry " + w.time + "Z" : "entry") + (w.fix ? " at " + w.fix : "") + ", in " + w.minutes + " min";
  }
  async function answer(a: "send" | "later" | "skip" | "stop"): Promise<void> {
    busy = true; msg = await sess.rclAnswer(a); busy = false;
  }
  async function toggleOn(): Promise<void> { msg = await sess.setRclAuto(!$rclAuto.on); }
  async function toggleConfirm(): Promise<void> { msg = await sess.setRclConfirm(!$rclAuto.confirm); }
  function close(): void { rclPanelOpen.set(false); hidden = true; msg = ""; }
</script>

{#if show}
  <div transition:fly={{ y: -16, duration: 200 }} class="fixed left-1/2 -translate-x-1/2 top-14 z-50 w-full max-w-lg bg-blue-2 border-2 {ask || (sending && amSender) ? 'border-yellow-300' : 'border-blue-10'} text-white font-web shadow-lg" role="dialog" aria-label="Automatic RCL REQ">
    <div class="px-3 py-1 bg-blue-3 font-bold flex items-center">
      <span>Auto RCL REQ</span>
      <span class="ml-2 text-sm font-normal opacity-70">{$rclAuto.on ? "on" : "off"}</span>
      <button class="ml-auto px-2 text-sm border border-blue-6" on:click={close} aria-label="Close">Close</button>
    </div>
    <div class="p-3 text-sm">
      {#if sending && amSender}
        <div class="font-bold text-base text-yellow-300">Sending reminder {Math.min(b.done + b.failed.length + 1, b.total)} of {b.total}...</div>
        <div class="mt-1">Please keep EuroScope in the front and <b>do not change windows</b> until they have all been sent.</div>
        <div class="mt-2 text-xs opacity-80">Nothing happening? Press Stop. It cancels the rest and this box goes away.</div>
        <button class="mt-2 px-3 py-1 border border-yellow-300" disabled={busy} on:click={() => answer("stop")}>Stop sending</button>
      {:else if ask}
        <div class="font-bold text-base">{$rclAuto.waiting.length} aircraft {$rclAuto.waiting.length === 1 ? "is" : "are"} en route to oceanic airspace and {$rclAuto.waiting.length === 1 ? "needs" : "need"} an RCL REQ reminder</div>
        <ul class="my-2 max-h-48 overflow-y-auto">
          {#each $rclAuto.waiting as w}<li class="flex justify-between gap-3 py-0.5 border-b border-blue-6"><b>{w.callsign}</b><span class="opacity-90">{when(w)}</span></li>{/each}
        </ul>
        <div>Send the reminder messages now? EuroScope will come to the front and type them one after another, so <b>please do not change windows</b> until they have all been sent.</div>
        <div class="mt-3 flex flex-wrap gap-2">
          <button class="px-4 py-1 font-bold bg-blue-10 text-black" disabled={busy} on:click={() => answer("send")}>Send now</button>
          <button class="px-3 py-1 border border-blue-6" disabled={busy} title="Ask again in 5 minutes" on:click={() => answer("later")}>Not now (ask in 5 min)</button>
          <button class="px-3 py-1 border border-blue-6" disabled={busy} title="Do not send reminders to these aircraft" on:click={() => answer("skip")}>Skip these</button>
        </div>
      {:else}
        {#if finished}
          <div class="mb-2 {b.failed.length ? 'text-yellow-300' : 'text-green-300'}">
            {b.failed.length === 0 ? "✔ All " + b.done + " reminders were sent." : b.done + " sent. Not sent (they will be offered again): " + b.failed.join(", ") + "."}
          </div>
        {/if}
        {#if $rclAuto.waiting.length > 0}
          <div class="mb-1 font-bold">Waiting ({$rclAuto.waiting.length}){$rclAuto.sender && !amSender ? ": " + $rclAuto.sender + " will be asked to send them" : ""}</div>
          <ul class="mb-2 max-h-40 overflow-y-auto">
            {#each $rclAuto.waiting as w}<li class="flex justify-between gap-3 py-0.5 border-b border-blue-6"><b>{w.callsign}</b><span class="opacity-90">{when(w)}</span></li>{/each}
          </ul>
        {:else}
          <div class="mb-2 opacity-80">No aircraft are waiting for a reminder.</div>
        {/if}
        {#if !$rclAuto.sender}<div class="mb-2 opacity-80">No oceanic controller with the plugin is online yet. Reminders start 5 minutes after one is.</div>{/if}
        <div class="flex flex-wrap items-center gap-3">
          <button class="px-3 py-1 font-bold border border-blue-6 {$rclAuto.on ? 'bg-blue-3' : 'bg-red-700'}" disabled={!sess.canEdit($session)} on:click={toggleOn}>Auto RCL: {$rclAuto.on ? "ON" : "OFF"}</button>
          <label class="flex items-center gap-2"><input type="checkbox" checked={$rclAuto.confirm} disabled={!sess.canEdit($session)} on:change={toggleConfirm} /> Ask me before sending (recommended)</label>
        </div>
        <div class="mt-2 text-xs opacity-70">Flights 20 to 45 minutes from oceanic entry that have not asked for clearance on natTrak get the reminder. With "ask first" off they are sent automatically, and EuroScope may come to the front without warning.</div>
        {#if $rclAuto.error}<div class="mt-1 text-yellow-300">Last problem: {$rclAuto.error}</div>{/if}
      {/if}
      {#if msg}<div class="mt-2 text-yellow-300">{msg}</div>{/if}
    </div>
  </div>
{/if}
