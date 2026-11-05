<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  let cid = "";
  let name = "";
  let passphrase = "";
  let busy = false;
  let error = "";
  let result: { key: string; password: string; expires: number; cid: string } | null = null;
  onMount(() => {
    site.setTitle("Temporary key");
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
        body: JSON.stringify({ cid, name, passphrase }),
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
    <h1 class="text-4xl font-bold mb-2">vNAAATS - temporary key</h1>
    <div class="p-4 mb-4 border-2 border-blue-10 bg-blue-3">
      <p class="mb-2"><b>Gander Oceanic certified controllers:</b> get your key and FDD password from your profile on the Gander Oceanic website, <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a>. You do not need this page.</p>
      <p><b>Oceanic controllers endorsed by VATSIM UK or Virtual New York ARTCC (NYACC):</b> follow the instructions below to get a temporary key.</p>
    </div>
    <p class="opacity-80 mb-6">The key and password from this page last <b>90 days</b>. Use only your own VATSIM CID, and only if you are shown as <b>Certified</b> on the <a class="underline" href="https://ganderoceanic.ca/roster">Gander Oceanic roster</a> (the roster also lists VATSIM UK and NYACC oceanic controllers).</p>
    {#if !result}
      <label class="block mb-3">VATSIM CID<input class="block w-full p-2 text-black" bind:value={cid} inputmode="numeric" autocomplete="off" /></label>
      <label class="block mb-3">Your name (optional)<input class="block w-full p-2 text-black" bind:value={name} autocomplete="off" /></label>
      <label class="block mb-4">Passphrase (you were sent this)<input class="block w-full p-2 text-black" type="password" bind:value={passphrase} autocomplete="off" /></label>
      {#if error}<div class="p-3 mb-4 border-2 border-red-500 bg-blue-3">{error}</div>{/if}
      <button class="px-5 py-2 font-bold bg-blue-10 text-black" disabled={busy} on:click={submit}>{busy ? "Please wait..." : "Get my key"}</button>
    {:else}
      <div class="p-4 border-2 border-blue-10 bg-blue-3 mb-4">
        <div class="mb-3"><div class="opacity-80 text-sm">Plugin key (put in vnaaats.cfg as key=...)</div><div class="font-mono break-all select-all">{result.key}</div></div>
        <div class="mb-3"><div class="opacity-80 text-sm">Flight Data Display password (sign in with your CID {result.cid})</div><div class="font-mono select-all">{result.password}</div></div>
        <div class="opacity-80 text-sm">Expires {new Date(result.expires * 1000).toUTCString()}</div>
      </div>
      <p class="opacity-90">Copy both now: the password is not shown again. Keep them private. After they expire you will need to get a normal key from the Gander Oceanic website.</p>
    {/if}
  </div>
</div>
