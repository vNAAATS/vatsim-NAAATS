<script lang="ts">
  import { fade, fly } from "svelte/transition";
  import * as rl from "../../typescript/fdd/relief";
  import * as acmenu from "../../typescript/fdd/acmenu";
  import * as sess from "../../typescript/fdd/session";

  const { reliefOpen } = rl;
  const { session } = sess;

  let plan: rl.ReliefPlan | null = null;
  let controllers: acmenu.Controller[] = [];
  let target: string = "";
  let error: string = "";
  let step: "pick" | "confirm" | "running" = "pick";
  let results: { callsign: string; state: string; msg: string }[] = [];
  let timer: any = null;

  async function open(): Promise<void> {
    error = ""; step = "pick"; results = []; target = ""; plan = null;
    const p = await rl.loadPlan();
    if (typeof p === "string") error = p; else plan = p;
    controllers = (await acmenu.loadControllers()).filter((c) => c.callsign.toUpperCase() !== $session.position.toUpperCase());
  }
  $: if ($reliefOpen) open(); else if (timer) { clearInterval(timer); timer = null; }

  async function go(): Promise<void> {
    if (!plan) return;
    const r = await rl.startRelief(target, plan.aircraft.map((a) => a.callsign));
    if (typeof r === "string") { error = r; return; }
    results = r.commands.map((c) => ({ callsign: c.callsign, state: "pending", msg: "" }));
    step = "running";
    const ids = r.commands;
    timer = setInterval(async () => {
      for (let i = 0; i < ids.length; i++) {
        if (results[i].state === "ok" || results[i].state === "failed") continue;
        const s = await acmenu.commandState(ids[i].id);
        results[i] = { callsign: ids[i].callsign, state: s.state, msg: s.msg };
      }
      results = results;
      if (results.every((x) => x.state === "ok" || x.state === "failed")) { clearInterval(timer); timer = null; }
    }, 3000);
  }
  $: cpdlcCount = plan ? plan.aircraft.filter((a) => a.cpdlc).length : 0;
</script>

{#if $reliefOpen}
  <div transition:fade={{ duration: 180 }} class="fixed inset-0 z-40 flex items-center justify-center bg-black bg-opacity-60" on:click|self={() => reliefOpen.set(false)}>
    <div transition:fly={{ y: 24, duration: 220 }} class="w-full max-w-xl max-h-full overflow-y-auto bg-blue-2 border-2 border-blue-10 text-white font-web">
      <div class="flex flex-row justify-between items-center px-3 py-1 bg-blue-3 font-bold">
        <span>Relief: hand over {$session.position}</span>
        <button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={() => reliefOpen.set(false)}>Close</button>
      </div>
      {#if error}<div class="m-3 text-yellow-300">{error}</div>{/if}
      {#if plan}
        {#if !plan.pluginAlive}
          <div class="m-3 text-yellow-300">Your EuroScope is not talking to vNAAATS right now, so the transfers cannot be carried out.</div>
        {/if}
        {#if step === "pick"}
          <div class="m-3 text-sm">
            You are tracking <b>{plan.aircraft.length}</b> aircraft{plan.aircraft.length ? ": " + plan.aircraft.map((a) => a.callsign).join(", ") : ""}.
            Choose the controller who is taking over (a second or relief callsign works too, as long as they are online in EuroScope).
          </div>
          <div class="mx-3 mb-3">
            <select class="w-full px-1 py-0.5 text-black" bind:value={target}>
              <option value="">Choose controller</option>
              {#each controllers as c}<option value={c.callsign}>{c.callsign} {c.frequency} {c.name}</option>{/each}
            </select>
          </div>
          <div class="px-3 pb-3">
            <button class="px-3 py-1 font-bold bg-blue-10 text-black" disabled={!target || plan.aircraft.length === 0 || !plan.pluginAlive} on:click={() => (step = "confirm")}>Continue</button>
          </div>
        {:else if step === "confirm"}
          <div class="m-3 p-2 border-2 border-yellow-300 text-sm">
            <b>Please confirm.</b> {plan.aircraft.length} aircraft will be handed from {plan.position} to <b>{target}</b> in EuroScope, and {target} will start tracking them.
            {#if cpdlcCount > 0}
              <div class="mt-2">{cpdlcCount} of them are on CPDLC ({plan.aircraft.filter((a) => a.cpdlc).map((a) => a.callsign).join(", ")}). They get this message now, and are logged off once {target} has them:</div>
              <div class="mt-1 italic opacity-90">{plan.message}</div>
            {/if}
          </div>
          <div class="px-3 pb-3 flex flex-row gap-3">
            <button class="px-3 py-1 font-bold bg-red-700 border-2 border-red-300" on:click={go}>Yes, hand over to {target}</button>
            <button class="px-3 py-1 font-bold border border-blue-6" on:click={() => (step = "pick")}>Back</button>
          </div>
        {:else}
          <div class="m-3 text-sm">
            {#each results as r}
              <div>{r.callsign}: {r.state === "ok" ? "handed over" : r.state === "failed" ? "failed - " + r.msg : r.state === "waiting" ? "waiting for your EuroScope" : "sending..."}</div>
            {/each}
            <div class="mt-2 opacity-80">{target} must have the vNAAATS plugin running to take the aircraft over. Any that fail stay with you.</div>
          </div>
        {/if}
      {:else if !error}
        <div class="m-3 opacity-80">Loading...</div>
      {/if}
    </div>
  </div>
{/if}
