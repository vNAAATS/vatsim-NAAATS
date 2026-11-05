<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  let cid = "";
  let name = "";
  let busy = false;
  let error = "";
  let result: { key: string; password: string; expires: number; cid: string } | null = null;
  onMount(() => {
    site.setTitle("Get your key");
    document.body.style.opacity = "100%";
    const m = document.createElement("meta");
    m.name = "robots";
    m.content = "noindex";
    document.head.appendChild(m);
  });
  async function submit() {
    error = "";
    busy = true;
    try {
      const r = await fetch("/auth/temp-key", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ cid, name }),
      });
      const j = await r.json();
      if (!r.ok) error = j.error || "Something went wrong";
      else result = j;
    } catch (e) {
      error = "Could not reach the server";
    }
    busy = false;
  }
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-xl w-full">
    <h1 class="text-4xl font-bold mb-2">vNAAATS - get your key</h1>
    <div class="p-4 mb-4 border-2 border-blue-10 bg-blue-3">
      <p class="mb-2"><b>Gander Oceanic certified controllers:</b> you can also get your key and FDD password from your profile on the Gander Oceanic website, <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a>.</p>
      <p><b>Everyone else (VATSIM UK, NYACC and other oceanic controllers):</b> get your key here.</p>
    </div>
    <p class="opacity-80 mb-2">The key lasts <b>90 days</b>. Use only your own VATSIM CID.</p>
    <p class="mb-6">The key is not a login on its own. The Flight Data Display opens only while you are <b>connected to VATSIM on an oceanic position</b> (for example NAT_FSS, CZQO_CTR or EGGX_CTR): start the display, enter your key, connect in EuroScope, and it opens by itself.</p>
    {#if !result}
      <label class="block mb-3">VATSIM CID<input class="block w-full p-2 text-black" bind:value={cid} inputmode="numeric" autocomplete="off" /></label>
      <label class="block mb-4">Your name (optional)<input class="block w-full p-2 text-black" bind:value={name} autocomplete="off" /></label>
      {#if error}<div class="p-3 mb-4 border-2 border-red-500 bg-blue-3">{error}</div>{/if}
      <button class="px-5 py-2 font-bold bg-blue-10 text-black" disabled={busy} on:click={submit}>{busy ? "Please wait..." : "Get my key"}</button>
    {:else}
      <div class="p-4 border-2 border-blue-10 bg-blue-3 mb-4">
        <div class="mb-3"><div class="opacity-80 text-sm">Your key (paste it into the display, or into Settings &gt; App setup in FDD for Windows)</div><div class="font-mono break-all select-all">{result.key}</div></div>
        <div class="opacity-80 text-sm">Expires {new Date(result.expires * 1000).toUTCString()}</div>
      </div>
      <p class="opacity-90">Copy it now: it is not shown again. Keep it private. If you lose it, connect to VATSIM with your own CID and ask again here for a new one.</p>
      <p class="opacity-80 mt-3 text-sm">The plugin also needs it: put it in vnaaats.cfg as key=... (FDD for Windows does this for you in Settings &gt; App setup).</p>
    {/if}
  </div>
</div>
