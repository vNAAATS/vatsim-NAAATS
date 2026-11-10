<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("Get access");
    document.body.style.opacity = "100%";
  });
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-2xl w-full">
    <p class="mb-4"><a class="underline opacity-80" href="/">&larr; Homepage</a></p>
    <h1 class="text-4xl font-bold mb-2">vNAAATS - how to get access</h1>
    <p class="opacity-80 mb-6">The Flight Data Display (FDD), FDD for Windows and the EuroScope plugin are for oceanic controllers. Access is a personal key plus a live VATSIM connection: there is no password to remember, and nobody has to approve you each time.</p>

    <h2 class="text-2xl font-bold mb-2">Who can have access</h2>
    <p class="mb-6">Controllers who are <b>certified or endorsed</b> to control the oceanic positions: Gander Oceanic, VATSIM UK (Shanwick), Virtual New York ARTCC (NYACC) and any other division endorsed to control them.</p>

    <h2 class="text-2xl font-bold mb-3">Four steps</h2>
    <ol class="space-y-4 mb-6">
      <li class="p-4 border-2 border-blue-10 bg-blue-3">
        <div class="font-bold text-lg mb-1">1. Get your key</div>
        <p class="mb-2">Fill in the short form with your VATSIM CID, name and email address. The key lasts <b>90 days</b>. Gander Oceanic controllers can also take it from their profile on the Gander Oceanic website.</p>
        <a class="inline-block px-4 py-2 font-bold bg-blue-10 text-black" href="/get-key">Get your API key</a>
      </li>
      <li class="p-4 border-2 border-blue-10 bg-blue-3">
        <div class="font-bold text-lg mb-1">2. Install FDD for Windows (or use a browser)</div>
        <p class="mb-2">FDD for Windows is the recommended way: it brings the display, EuroScope, VACS and your audio together in one app and updates itself. A browser works too, for example on an iPad.</p>
        <a class="underline mr-4" href="/windows">FDD for Windows</a><a class="underline mr-4" href="/fdd">Use in a browser</a><a class="underline" href="/guide">Install guide</a>
      </li>
      <li class="p-4 border-2 border-blue-10 bg-blue-3">
        <div class="font-bold text-lg mb-1">3. Paste your key once</div>
        <p>Paste it into the sign-in screen, or into <b>Settings &gt; App setup</b> in FDD for Windows. The EuroScope plugin reads it from <span class="font-mono">vnaaats.cfg</span> (<span class="font-mono">key=...</span>). Paste it exactly as it was given to you.</p>
      </li>
      <li class="p-4 border-2 border-blue-10 bg-blue-3">
        <div class="font-bold text-lg mb-1">4. Connect to VATSIM on an oceanic position</div>
        <p>The display waits on <b>"Awaiting an active VATSIM connection"</b> and opens by itself, on your position, a few seconds after you connect. An observer connection does not count.</p>
      </li>
    </ol>

    <h2 class="text-2xl font-bold mb-2">Positions that open the display</h2>
    <ul class="list-disc pl-5 space-y-1 mb-6">
      <li><b>NAT_FSS</b> - the oceanic bandbox, with <b>NAT_I_FSS</b> for instructors and <b>NAT__FSS</b> for a relief taking over</li>
      <li><b>CZQO_</b> (Gander Radio and Delivery), including the extra positions CZQO_A to CZQO_L and a relief CZQO__CTR</li>
      <li><b>EGGX_</b> (Shanwick Radio and Delivery), including the extra positions EGGX_A to EGGX_L and a relief EGGX__CTR</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Good to know</h2>
    <ul class="list-disc pl-5 space-y-2 mb-6">
      <li>The key alone opens nothing. It works only while <b>you</b> are connected to VATSIM, as yourself, on one of the positions above.</li>
      <li>The display closes an hour after you leave VATSIM, and opens again by itself when you reconnect.</li>
      <li>A banner warns you in the last 30 days before the key expires. Come back to <a class="underline" href="/get-key">Get your API key</a> while connected to VATSIM to renew it.</li>
      <li><b>Never share your key</b> or ask for one for someone else's CID. Your CID, email address and IP address are recorded when a key is issued (see the <a class="underline" href="/privacy">privacy page</a>), and unauthorised use is reported to the VATSIM supervisors.</li>
      <li>CPDLC is separate: you also need your own Hoppie logon code, signed in on the FDD, and only one controller can use each CPDLC station.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">It says my key is not known</h2>
    <ul class="list-disc pl-5 space-y-1 mb-6">
      <li>Copy the whole key again, with nothing missing at the end.</li>
      <li>The key belongs to the CID it was issued for. Connect to VATSIM with that same CID.</li>
      <li>Lost it? Connect to VATSIM and ask for a new one on the key page.</li>
    </ul>

    <p class="opacity-80">Learn how it all works in the <a class="underline" href="/manual">user manual</a> and the <a class="underline" href="/videos">How To Use videos</a>.</p>
  </div>
</div>
