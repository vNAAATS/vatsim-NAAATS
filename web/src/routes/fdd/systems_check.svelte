<script lang="ts">
  import { onMount } from "svelte";
  import { fade } from "svelte/transition";
  import * as app from "../../typescript/fdd/app";

  // "All systems go": after the welcome animation FDD for Windows asks the server how it is (the same answer as vnaaats.com/status) and shows it
  // for 5 seconds before the checks on this computer begin. It never blocks anything: if something is wrong it says so and carries on.
  const SECONDS = 5;
  const KEY = "vnaaats.systemsSeen";
  type Line = { ok: boolean; text: string };
  let show: boolean = false;
  let lines: Line[] = [];
  let go: boolean = false;
  let left: number = SECONDS;
  let timer: ReturnType<typeof setInterval> | null = null;

  function finish(): void {
    if (!show) return;
    show = false;
    if (timer) clearInterval(timer);
    app.finishSplash();
  }

  async function ask(): Promise<any | null> {
    const ctl = new AbortController();
    const t = setTimeout(() => ctl.abort(), 4000);
    try {
      const r = await fetch("/api/status", { cache: "no-store", signal: ctl.signal });
      return r.ok ? await r.json() : null;
    } catch (e) { return null; } finally { clearTimeout(t); }
  }

  function summarise(d: any | null): void {
    if (!d) {
      lines = [{ ok: false, text: "The vNAAATS server did not answer. The checks on this computer still run; try again in a minute if the display stays empty." }];
      go = false;
      return;
    }
    const now = d.now as number;
    const feedOk = !d.feed.enabled || (!d.feed.error && d.feed.updated && now - d.feed.updated < 120);
    const tracksOk = !!d.tracks.updated && now - d.tracks.updated < 3600;
    const atc = (d.atc as { callsign: string }[]).filter((c) => /^(CZQO|EGGX|NAT)/.test(c.callsign)).length;
    lines = [
      { ok: true, text: "Server " + d.version + " is running" },
      { ok: feedOk, text: feedOk ? "VATSIM data is up to date" : "VATSIM data is slow or not updating" },
      { ok: tracksOk, text: tracksOk ? d.tracks.count + " NAT tracks loaded" : "NAT tracks are out of date" },
      { ok: true, text: d.feed.oceanicFlights + " oceanic-bound flights" + (atc ? ", " + atc + " Gander/Shanwick position" + (atc === 1 ? "" : "s") + " online" : ", no Gander/Shanwick ATC online") },
    ];
    if (d.cpdlc.enabled) lines.push({ ok: true, text: "CPDLC is available" });
    go = lines.every((l) => l.ok);
  }

  onMount(async () => {
    if (!app.inApp()) { app.finishSplash(); return; }
    await app.welcomeDone;
    let seen = false;
    try { seen = sessionStorage.getItem(KEY) === "1"; sessionStorage.setItem(KEY, "1"); } catch (e) { /* show it */ }
    if (seen) { app.finishSplash(); return; }
    summarise(await ask());
    show = true;
    timer = setInterval(() => { left -= 1; if (left <= 0) finish(); }, 1000);
  });
</script>

{#if show}
  <div transition:fade={{ duration: 200 }} class="fixed inset-0 z-[75] flex items-center justify-center" style="background: rgba(11,27,48,0.96);">
    <div class="w-[30rem] max-w-full bg-blue-3 border-2 {go ? 'border-green-400' : 'border-yellow-300'} text-white font-regular p-5 text-center">
      <div class="text-3xl font-black tracking-wide {go ? 'text-green-300' : 'text-yellow-300'}">{go ? "ALL SYSTEMS GO" : "SOME SYSTEMS NEED ATTENTION"}</div>
      <ul class="my-4 text-sm text-left space-y-1">
        {#each lines as l}<li class={l.ok ? "text-green-300" : "text-yellow-300"}>{l.ok ? "✔" : "⚠"} {l.text}</li>{/each}
      </ul>
      <div class="h-1 bg-blue-4 mb-3"><div class="h-1 {go ? 'bg-green-400' : 'bg-yellow-300'}" style="width: {Math.max(0, left) / SECONDS * 100}%; transition: width 1s linear;"></div></div>
      <button class="px-4 py-1 font-bold bg-blue-10 text-black" on:click={finish}>Continue ({Math.max(0, left)})</button>
      <div class="mt-2 text-xs opacity-60">Live status any time: vnaaats.com/status</div>
    </div>
  </div>
{/if}
