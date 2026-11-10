<script lang="ts">
  import { fade, fly } from "svelte/transition";
  import * as dataHandler from "../../typescript/fdd/data";
  import * as pe from "../../typescript/fdd/planedit";

  const { planEditCallsign } = pe;

  let lastCs: string = "";
  let loading: boolean = false;
  let busy: boolean = false;
  let error: string = "";
  let note: string = "";
  let mine: boolean = false;
  let trackedBy: string = "";
  let original: pe.Plan | null = null;
  let plan: pe.Plan | null = null;
  let levelText: string = "";
  let confirming: string[] = [];

  $: if ($planEditCallsign !== lastCs) { lastCs = $planEditCallsign; if ($planEditCallsign) open($planEditCallsign); }

  async function open(cs: string): Promise<void> {
    loading = true; busy = false; error = ""; note = ""; confirming = []; original = null; plan = null; mine = false; trackedBy = "";
    const r = await pe.loadPlan(cs);
    if ($planEditCallsign !== cs) return;      // closed meanwhile
    loading = false;
    if (r.error || !r.plan) { error = r.error ?? "No flight plan"; return; }
    mine = r.mine; trackedBy = r.trackedBy;
    original = r.plan;
    plan = { ...r.plan };
    levelText = pe.feetToLevel(plan.alt);
  }
  function close(): void { planEditCallsign.set(""); }

  $: changed = original && plan ? pe.changes(original, { ...plan, alt: pe.levelToFeet(levelText) ?? plan.alt }) : [];
  $: problem = plan ? pe.planProblem(plan, levelText) : "";

  function review(): void {
    if (!plan || problem) { note = problem; return; }
    confirming = changed;
    if (confirming.length === 0) note = "Nothing has been changed.";
  }
  async function save(): Promise<void> {
    if (!plan) return;
    busy = true; note = "";
    const out = { ...plan, alt: pe.levelToFeet(levelText) ?? plan.alt };
    const e = await pe.savePlan($planEditCallsign, out);
    busy = false;
    if (e) { note = "✖ " + e; confirming = []; return; }
    note = "✔ Flight plan sent to the network";
    confirming = [];
    await dataHandler.populateAllAircraft();
    dataHandler.reselect($planEditCallsign);
    setTimeout(close, 1200);
  }

  const BOX = "px-1 h-7 text-white bg-blue-2 border-2 border-l-blue-0 border-t-blue-0 border-r-blue-6 border-b-blue-6 focus:outline-none focus:border-green-300 disabled:opacity-70";
  const BTN = "border-2 border-l-blue-6 border-t-blue-6 border-r-blue-0 border-b-blue-0 bg-blue-3 active:bg-blue-1 hover:cursor-pointer select-none";
  $: ro = !mine || busy;
  const hot = (k: keyof pe.Plan): string => (original && plan && String(original[k] ?? "").trim() !== String(plan[k] ?? "").trim() ? "border-yellow-300" : "");
</script>

{#if $planEditCallsign}
  <div transition:fade={{ duration: 180 }} class="fixed inset-0 z-50 flex items-center justify-center" style="background: rgba(0,0,0,0.55);" on:click|self={close}>
    <div transition:fly={{ y: 24, duration: 220 }} class="w-full max-h-full overflow-y-auto bg-blue-3 border-2 border-l-blue-10 border-t-blue-10 border-r-blue-0 border-b-blue-0 text-white font-regular" style="max-width: 46rem;">
      <div class="flex flex-row justify-between items-center px-3 py-2 bg-blue-4 font-bold select-none">
        <span>Flight plan - {$planEditCallsign}</span>
        <button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={close}>Close</button>
      </div>
      <div class="p-3 text-sm">
        {#if loading}
          <div class="opacity-90">Asking EuroScope for the flight plan...</div>
        {:else if error}
          <div class="text-yellow-300 break-words">{error}</div>
        {:else if plan}
          {#if !mine}
            <div class="mb-3 p-2 text-black bg-yellow-300 font-bold">Read only. Only the controller tracking this aircraft ({trackedBy || "nobody"}) can change its flight plan{trackedBy ? "" : " - track it first"}.</div>
          {:else}
            <div class="mb-3 opacity-90">You are tracking this aircraft. Changed boxes turn yellow. Saving sends the amended flight plan to the pilot on VATSIM.</div>
          {/if}

          <div class="grid gap-x-3 gap-y-2 items-end" style="grid-template-columns: repeat(4, minmax(0, 1fr));">
            <label class="flex flex-col font-bold">Callsign<input class="{BOX} opacity-70" disabled value={$planEditCallsign} /></label>
            <label class="flex flex-col font-bold">Flight rules
              <select class="{BOX} {hot('planType')}" disabled={ro} bind:value={plan.planType}><option value="I">IFR</option><option value="V">VFR</option></select>
            </label>
            <label class="flex flex-col font-bold" style="grid-column: span 2 / span 2;">Aircraft data (like H/B77W/L)<input class="{BOX} {hot('acInfo')} uppercase" disabled={ro} maxlength="30" bind:value={plan.acInfo} /></label>

            <label class="flex flex-col font-bold">Departure<input class="{BOX} {hot('origin')} uppercase" disabled={ro} maxlength="4" bind:value={plan.origin} /></label>
            <label class="flex flex-col font-bold">Destination<input class="{BOX} {hot('dest')} uppercase" disabled={ro} maxlength="4" bind:value={plan.dest} /></label>
            <label class="flex flex-col font-bold">Alternate<input class="{BOX} {hot('alternate')} uppercase" disabled={ro} maxlength="4" bind:value={plan.alternate} /></label>
            <label class="flex flex-col font-bold">Level<input class="{BOX} uppercase {pe.levelToFeet(levelText) !== (original?.alt ?? 0) ? 'border-yellow-300' : ''}" disabled={ro} maxlength="6" placeholder="FL390" bind:value={levelText} /></label>

            <label class="flex flex-col font-bold">TAS (kt)<input class="{BOX} {hot('tas')}" disabled={ro} type="number" min="0" max="999" bind:value={plan.tas} /></label>
            <label class="flex flex-col font-bold">Departure time (Z)<input class="{BOX} {hot('etd')}" disabled={ro} maxlength="4" placeholder="1435" bind:value={plan.etd} /></label>
            <div class="flex flex-col font-bold">Enroute (h / min)
              <div class="flex flex-row"><input class="{BOX} {hot('enrouteH')} w-1/2" disabled={ro} type="number" min="0" max="99" bind:value={plan.enrouteH} /><input class="{BOX} {hot('enrouteM')} w-1/2" disabled={ro} type="number" min="0" max="59" bind:value={plan.enrouteM} /></div>
            </div>
            <div class="flex flex-col font-bold">Fuel (h / min)
              <div class="flex flex-row"><input class="{BOX} {hot('fuelH')} w-1/2" disabled={ro} type="number" min="0" max="99" bind:value={plan.fuelH} /><input class="{BOX} {hot('fuelM')} w-1/2" disabled={ro} type="number" min="0" max="59" bind:value={plan.fuelM} /></div>
            </div>
          </div>

          <label class="flex flex-col font-bold mt-2">Route
            <textarea class="{BOX} {hot('route')} h-auto uppercase" rows="3" disabled={ro} bind:value={plan.route} />
          </label>
          <label class="flex flex-col font-bold mt-2">Remarks
            <textarea class="{BOX} {hot('remarks')} h-auto" rows="3" disabled={ro} bind:value={plan.remarks} />
          </label>

          {#if problem && mine}<div class="mt-3 p-1 text-white bg-red-700 border border-red-300">{problem}</div>{/if}

          {#if confirming.length > 0}
            <div class="mt-3 p-2 border-2 border-yellow-300 bg-blue-1">
              <div class="font-bold text-yellow-300 mb-1">Send these changes to the pilot's flight plan?</div>
              {#each confirming as c}<div class="font-mono break-words">{c}</div>{/each}
              <div class="mt-2 flex flex-row">
                <button class="px-3 py-1 mr-2 font-bold {BTN}" disabled={busy} on:click={save}>{busy ? "Sending..." : "Yes, send"}</button>
                <button class="px-3 py-1 {BTN}" disabled={busy} on:click={() => (confirming = [])}>Back</button>
              </div>
            </div>
          {:else}
            <div class="mt-3 flex flex-row">
              {#if mine}<button class="px-3 py-1 mr-2 font-bold {BTN}" disabled={busy || !!problem || changed.length === 0} on:click={review}>Save</button>{/if}
              <button class="px-3 py-1 {BTN}" on:click={close}>{mine ? "Cancel" : "Close"}</button>
            </div>
          {/if}
        {/if}
        {#if note}<div class="mt-3 break-words {note.charAt(0) === '✔' ? 'text-green-300' : 'text-yellow-300'}">{note}</div>{/if}
      </div>
    </div>
  </div>
{/if}
