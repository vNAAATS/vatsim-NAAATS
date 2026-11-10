<script lang="ts">
  import { fade, fly } from "svelte/transition";
  import * as dataHandler from "../../typescript/fdd/data";
  import * as sess from "../../typescript/fdd/session";
  import { posOpen, nowHHMM } from "../../typescript/fdd/posreports";
  import type { Aircraft } from "../../typescript/fdd/objects";

  const { session } = sess;
  let asel: Aircraft | null = null;
  let tick: number = 0;
  let message: string = "";
  let messageOk: boolean = true;
  let timer: any;
  import { onDestroy, onMount } from "svelte";
  onMount(() => { timer = setInterval(() => { asel = dataHandler.asel ?? null; tick++; }, 500); });
  onDestroy(() => clearInterval(timer));

  $: editable = sess.canEdit($session);
  $: overdue = asel && tick >= 0 ? dataHandler.concOverdueFix(asel) : "";

  function digits(t: string): string { return (t ?? "").replace(/\D/g, ""); }

  async function save(fix: string, field: "est" | "act", value: string): Promise<void> {
    if (!asel) return;
    const r = asel.PosReports[fix] ?? { est: "", act: "" };
    const est: string = field === "est" ? digits(value) : r.est;
    const act: string = field === "act" ? digits(value) : r.act;
    const err = await sess.editPosReport(asel.Callsign, fix, est, act);
    messageOk = err === "";
    message = err === "" ? "Saved" : err;
    setTimeout(() => (message = ""), 5000);
  }
  function onChange(e: Event, fix: string, field: "est" | "act"): void {
    save(fix, field, (e.target as HTMLInputElement).value);
  }
  function now(fix: string): void { save(fix, "act", nowHHMM()); }
  function at(ts: string): string { return ts ? ts.slice(11, 16) + "z" : ""; }
</script>

{#if $posOpen && asel && asel.IsConc}
  <div transition:fade={{ duration: 180 }} class="fixed inset-0 z-50 flex items-center justify-center" style="background: rgba(0,0,0,0.55);" on:click|self={() => posOpen.set(false)}>
    <div transition:fly={{ y: 24, duration: 220 }} class="w-full bg-blue-3 border-2 border-l-blue-10 border-t-blue-10 border-r-blue-0 border-b-blue-0 text-white font-regular" style="max-width: 36rem; max-height: 90vh; display: flex; flex-direction: column;">
      <div class="flex flex-row justify-between items-center px-3 py-2 bg-blue-4 font-bold select-none">
        <span>Position reports - {asel.Callsign}</span>
        <button class="px-3 py-0.5 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={() => posOpen.set(false)}>Close</button>
      </div>
      <div class="px-3 py-1 text-xs opacity-80">
        Type the time the pilot reports at each fix (Actual) and any revised estimate (Est), as four digits (1432). Calc is the calculated time.
        A fix flashes red when its time passed more than 3 minutes ago with no report.
      </div>
      <div class="overflow-y-auto px-3 pb-2">
        <div class="flex flex-row text-xs font-bold opacity-80 py-1 border-b border-blue-6">
          <div class="w-24">Fix</div><div class="w-16">Calc</div><div class="w-20">Est</div><div class="w-20">Actual</div><div class="flex-1">Entered by</div>
        </div>
        {#each asel.PosFixes as fix, i}
          <div class="flex flex-row items-center py-0.5 border-b border-blue-4 {overdue === fix && dataHandler.visualOn ? 'nt-flash text-black' : ''}">
            <div class="w-24 font-bold">{fix}</div>
            <div class="w-16 opacity-80">{(asel.PosCalc[i] ?? "--") === "" ? "--" : asel.PosCalc[i]}</div>
            <div class="w-20">
              <input class="w-16 px-1 text-black" maxlength="5" value={asel.PosReports[fix]?.est ?? ""} disabled={!editable} on:change={(e) => onChange(e, fix, "est")} />
            </div>
            <div class="w-20">
              <input class="w-16 px-1 text-black" maxlength="5" value={asel.PosReports[fix]?.act ?? ""} disabled={!editable} on:change={(e) => onChange(e, fix, "act")} />
            </div>
            <div class="flex-1 text-xs flex flex-row items-center justify-between">
              <span class="opacity-80">{asel.PosReports[fix] ? at(asel.PosReports[fix].at) + " " + asel.PosReports[fix].by : ""}</span>
              {#if editable}<span class="underline hover:cursor-pointer pl-2" title="Set Actual to the current time" on:click={() => now(fix)}>now</span>{/if}
            </div>
          </div>
        {/each}
        {#if asel.PosFixes.length === 0}<div class="py-2 opacity-70">This flight has no route fixes yet.</div>{/if}
      </div>
      <div class="px-3 py-1 text-xs">
        {#if message}<span class={messageOk ? "text-green-300" : "text-yellow-300"}>{message}</span>{/if}
        {#if !editable}<span class="opacity-70">View only - sign in and pick a position to enter times</span>{/if}
      </div>
    </div>
  </div>
{/if}
