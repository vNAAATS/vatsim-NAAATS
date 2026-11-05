<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  let cid = "";
  let name = "";
  let email = "";
  let agree = false;
  let busy = false;
  let error = "";
  let result: { key: string; expires: number; cid: string } | null = null;
  onMount(() => {
    site.setTitle("Get your API key");
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
      const r = await fetch("/auth/get-key", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ cid, name, email, agree }),
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
    <h1 class="text-4xl font-bold mb-2">vNAAATS - get your API key</h1>
    <p class="opacity-80 mb-4">For all oceanic controllers: Gander Oceanic, VATSIM UK, Virtual New York ARTCC (NYACC) and any other division endorsed to control the oceanic positions. The key lasts <b>90 days</b>.</p>
    <div class="p-4 mb-4 border-2 border-red-500 bg-blue-3">
      <p class="font-bold text-lg mb-2">WARNING - authorised controllers only</p>
      <ul class="list-disc pl-5 space-y-1">
        <li>This key is for controllers who are <b>certified or endorsed</b> to control oceanic airspace. Do not request a key if you are not.</li>
        <li><b>Never</b> request a key for someone else's CID, and never share your key.</li>
        <li>Your <b>CID, email address and IP address</b> are recorded when you ask for a key.</li>
        <li>Anyone controlling an oceanic position without authorisation is <b>reported to the VATSIM supervisors</b>, and the key is blocked.</li>
        <li>The key does not open the display by itself: it opens only while you are <b>connected to VATSIM on an oceanic position</b>, as you.</li>
      </ul>
    </div>
    {#if !result}
      <label class="block mb-3">VATSIM CID<input class="block w-full p-2 text-black" bind:value={cid} inputmode="numeric" autocomplete="off" /></label>
      <label class="block mb-3">Your name<input class="block w-full p-2 text-black" bind:value={name} autocomplete="name" /></label>
      <label class="block mb-3">Email address<input class="block w-full p-2 text-black" type="email" bind:value={email} autocomplete="email" /></label>
      <label class="flex items-start gap-2 mb-4"><input class="mt-1" type="checkbox" bind:checked={agree} /> <span>I am a certified oceanic controller, this is my own CID, and I understand the warning above.</span></label>
      {#if error}<div class="p-3 mb-4 border-2 border-red-500 bg-blue-3">{error}</div>{/if}
      <button class="px-5 py-2 font-bold bg-blue-10 text-black" disabled={busy || !agree || !cid || !email} on:click={submit}>{busy ? "Please wait..." : "Get my API key"}</button>
      <p class="opacity-70 text-sm mt-4">Gander Oceanic controllers can also get a key from their profile on the Gander Oceanic website, <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a>.</p>
    {:else}
      <div class="p-4 border-2 border-blue-10 bg-blue-3 mb-4">
        <div class="mb-3"><div class="opacity-80 text-sm">Your API key (paste it into the display, or into Settings &gt; App setup in FDD for Windows)</div><div class="font-mono break-all select-all">{result.key}</div></div>
        <div class="opacity-80 text-sm">Expires {new Date(result.expires * 1000).toUTCString()}</div>
      </div>
      <p class="opacity-90">Copy it now: it is not shown again. Keep it private. If you lose it, connect to VATSIM with your own CID and ask again here for a new one.</p>
      <p class="opacity-80 mt-3 text-sm">The plugin needs it too: put it in vnaaats.cfg as key=... (FDD for Windows does this for you in Settings &gt; App setup).</p>
    {/if}
  </div>
</div>
