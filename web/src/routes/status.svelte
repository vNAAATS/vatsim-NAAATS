<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";

  type Status = {
    version: string; now: number; uptimeSeconds: number;
    tracks: { count: number; updated: number | null; error: string | null };
    feed: { enabled: boolean; updated: number | null; error: string | null; oceanicFlights: number; pilotsOnline: number };
    atc: { callsign: string; frequency: string }[];
    cpdlc: { enabled: boolean; connected: number };
    flightsOnDisplay: number;
    sectorPack: { name: string; size: number; updated: string } | null;
  };

  let st: Status | null = null;
  let failed: boolean = false;
  let checked: string = "";

  async function load(): Promise<void> {
    try {
      const r = await fetch(((import.meta as any).env?.VITE_API_BASE ?? "") + "/api/status", { cache: "no-store" });
      if (!r.ok) throw new Error(String(r.status));
      st = await r.json();
      failed = false;
    } catch (e) { failed = true; }
    checked = new Date().toISOString().slice(11, 19) + "Z";
  }

  onMount(() => {
    site.setTitle("Status");
    document.body.style.opacity = "100%";
    load();
    const t = setInterval(load, 15000);
    return () => clearInterval(t);
  });

  function ago(sec: number | null, now: number): string {
    if (!sec) return "never";
    const d = Math.max(0, Math.round(now - sec));
    if (d < 90) return d + " s ago";
    if (d < 5400) return Math.round(d / 60) + " min ago";
    return Math.round(d / 3600) + " h ago";
  }
  function up(sec: number): string {
    const d = Math.floor(sec / 86400), h = Math.floor((sec % 86400) / 3600), m = Math.floor((sec % 3600) / 60);
    return (d ? d + " d " : "") + (h || d ? h + " h " : "") + m + " min";
  }
  // green when updated recently, amber when a bit old, red when it has an error or is stale
  function feedState(f: Status["feed"], now: number): string {
    if (!f.enabled) return "off";
    if (f.error || !f.updated) return "bad";
    return now - f.updated > 120 ? "warn" : "ok";
  }
  function trackState(t: Status["tracks"], now: number): string {
    if (!t.updated) return "bad";
    if (t.error) return "warn";
    return now - t.updated > 3600 ? "warn" : "ok";
  }
  const word: Record<string, string> = { ok: "Working", warn: "Slow or out of date", bad: "Problem", off: "Switched off" };
  const MHZ = (f: string) => { const n = parseFloat(f); return isNaN(n) ? f : n.toFixed(3); };
  $: fs = st ? feedState(st.feed, st.now) : "off";
  $: ts = st ? trackState(st.tracks, st.now) : "ok";
  $: oceanic = st ? st.atc.filter((c) => /^(CZQO|EGGX|NAT)/.test(c.callsign)) : [];
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-3xl w-full">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">vNAAATS status</h1>
    <p class="opacity-80 mb-6">Live from the vNAAATS server. It refreshes by itself every 15 seconds{checked ? " (last checked " + checked + ")" : ""}.</p>

    {#if failed && !st}
      <div class="p-4 border-2 border-red-400 bg-blue-3"><b>The server did not answer.</b> It may be restarting; try again in a minute. If it stays like this, tell the team on Discord.</div>
    {:else if st}
      {#if failed}<div class="mb-4 p-3 border-2 border-yellow-300 bg-blue-3 text-sm">Could not refresh just now. Showing the last answer.</div>{/if}
      <div class="grid gap-3 sm:grid-cols-2">
        <div class="p-4 border-2 border-blue-10 bg-blue-3">
          <div class="text-sm opacity-70">Server</div>
          <div class="text-xl font-bold">Version {st.version}</div>
          <div class="text-sm opacity-80">Running for {up(st.uptimeSeconds)}</div>
        </div>
        <div class="p-4 border-2 border-blue-10 bg-blue-3">
          <div class="text-sm opacity-70">VATSIM data</div>
          <div class="text-xl font-bold"><span class="inline-block w-3 h-3 rounded-full mr-2 {fs === 'ok' ? 'bg-green-400' : fs === 'warn' ? 'bg-yellow-300' : fs === 'bad' ? 'bg-red-400' : 'bg-blue-6'}"></span>{word[fs]}</div>
          <div class="text-sm opacity-80">{st.feed.enabled ? "Updated " + ago(st.feed.updated, st.now) : "Not used on this server"}</div>
        </div>
        <div class="p-4 border-2 border-blue-10 bg-blue-3">
          <div class="text-sm opacity-70">NAT tracks</div>
          <div class="text-xl font-bold"><span class="inline-block w-3 h-3 rounded-full mr-2 {ts === 'ok' ? 'bg-green-400' : ts === 'warn' ? 'bg-yellow-300' : 'bg-red-400'}"></span>{st.tracks.count} track{st.tracks.count === 1 ? "" : "s"} loaded</div>
          <div class="text-sm opacity-80">Updated {ago(st.tracks.updated, st.now)}{st.tracks.error ? ". Last refresh failed, using the previous tracks." : ""}</div>
        </div>
        <div class="p-4 border-2 border-blue-10 bg-blue-3">
          <div class="text-sm opacity-70">CPDLC</div>
          <div class="text-xl font-bold"><span class="inline-block w-3 h-3 rounded-full mr-2 {st.cpdlc.enabled ? 'bg-green-400' : 'bg-blue-6'}"></span>{st.cpdlc.enabled ? "Available" : "Switched off"}</div>
          <div class="text-sm opacity-80">{st.cpdlc.enabled ? st.cpdlc.connected + " aircraft connected right now" : "Not used on this server"}</div>
        </div>
      </div>

      <h2 class="text-2xl font-bold mt-8 mb-2">Traffic</h2>
      <div class="p-4 border-2 border-blue-10 bg-blue-3 grid gap-2 sm:grid-cols-3">
        <div><div class="text-3xl font-bold">{st.feed.oceanicFlights}</div><div class="text-sm opacity-80">oceanic-bound flights on VATSIM</div></div>
        <div><div class="text-3xl font-bold">{st.flightsOnDisplay}</div><div class="text-sm opacity-80">flights on the display</div></div>
        <div><div class="text-3xl font-bold">{st.feed.pilotsOnline}</div><div class="text-sm opacity-80">pilots on the whole network</div></div>
      </div>

      <h2 class="text-2xl font-bold mt-8 mb-2">Oceanic and neighbouring ATC online</h2>
      <div class="p-4 border-2 border-blue-10 bg-blue-3">
        {#if !st.feed.enabled}
          <div class="opacity-80">Not available on this server.</div>
        {:else if !st.atc.length}
          <div class="opacity-80">No oceanic ATC online right now.</div>
        {:else}
          <ul class="grid gap-x-6 gap-y-1 sm:grid-cols-2">
            {#each st.atc as c}
              <li class="flex justify-between"><b class={oceanic.includes(c) ? "text-yellow-300" : ""}>{c.callsign}</b><span class="opacity-80">{MHZ(c.frequency)}</span></li>
            {/each}
          </ul>
          <div class="text-xs opacity-60 mt-2">Yellow is Gander or Shanwick. The others are the neighbours aircraft are handed to.</div>
        {/if}
      </div>

      {#if st.sectorPack}
        <h2 class="text-2xl font-bold mt-8 mb-2">Sector pack</h2>
        <div class="p-4 border-2 border-blue-10 bg-blue-3">
          <div class="font-bold">{st.sectorPack.name}</div>
          <div class="text-sm opacity-80">Published {st.sectorPack.updated.slice(0, 10)} ({(st.sectorPack.size / 1048576).toFixed(1)} MB). FDD for Windows installs it for you.</div>
        </div>
      {/if}
    {:else}
      <div class="opacity-80">Loading&hellip;</div>
    {/if}
  </div>
</div>
