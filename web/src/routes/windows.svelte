<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";

  const repo = "garythms/vatsim-NAAATS-v2";
  let size: string = "";
  let updated: string = "";
  onMount(async () => {
    site.setTitle("FDD for Windows");
    document.body.style.opacity = "100%";
    try {
      const r = await fetch("https://api.github.com/repos/" + repo + "/releases/tags/fdd-windows");
      if (r.ok) {
        const rel = await r.json();
        const a = (rel.assets ?? []).find((x: any) => String(x.name).endsWith(".exe"));
        if (a) {
          size = (a.size / 1048576).toFixed(1) + " MB";
          updated = new Date(a.updated_at).toLocaleDateString("en-GB", { day: "numeric", month: "long", year: "numeric" });
        }
      }
    } catch (e) { /* the page works without it */ }
  });
  const CARD = "p-5 mb-6 border-2 border-blue-10 bg-blue-3";
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-2xl w-full">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">FDD for Windows</h1>
    <div class="opacity-90 mb-6 text-lg">The vNAAATS Flight Data Display as a Windows app, with everything else you need to control oceanic ready at the click of one button.</div>

    <div class={CARD}>
      <a href="/get/fdd-windows" rel="external" class="inline-block px-6 py-3 text-lg font-bold bg-blue-10 text-black">Download FDD for Windows</a>
      <div class="text-sm opacity-80 mt-2">Free. Windows installer{size ? ", " + size : ""}{updated ? ", updated " + updated : ""}. No administrator rights needed.</div>
      <div class="text-xs opacity-80 mt-2">The installer is not code-signed yet, so Windows may say "Windows protected your PC". Choose <b>More info</b>, then <b>Run anyway</b>. <a class="underline" href="/code-signing">Why, and what is being done about it</a>.</div>
    </div>

    <h2 class="text-2xl font-bold mb-2">What is it?</h2>
    <p class="mb-6 opacity-90">It is the same Flight Data Display you use in a browser, in its own window with its own icon, plus a small helper that looks after your computer's side of vNAAATS. There is nothing to configure by hand: you answer a few questions once and it remembers.</p>

    <h2 class="text-2xl font-bold mb-2">Why use it?</h2>
    <div class="grid gap-3 mb-6 sm:grid-cols-2">
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">Always up to date</div><div class="text-sm opacity-90">The display updates by itself, with no new download. When the EuroScope plugin, airline list or font needs updating, the app tells you and asks before it changes anything.</div></div>
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">Opens your other programs</div><div class="text-sm opacity-90">Choose your position and it starts <b>EuroScope</b>, <b>VACS</b> and your audio client (<b>Audio for VATSIM</b> or <b>TrackAudio</b>) for you.</div></div>
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">One-time setup</div><div class="text-sm opacity-90">Point it at your EuroScope folder and paste your key once. It checks the plugin, saves the key into <b>vnaaats.cfg</b> for you, and never asks again unless something needs attention.</div></div>
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">Sign in once</div><div class="text-sm opacity-90">Tick <b>Remember me</b> and your sign-in is kept safely in the Windows Credential Manager on your computer, never on our server.</div></div>
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">Its own window</div><div class="text-sm opacity-90">A taskbar icon, and a window you can put on a second monitor, separate from all your browser tabs.</div></div>
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">VACS call alert</div><div class="text-sm opacity-90">A headphones button in the display flashes, with a pop-up and an Accept button, when VACS is calling you. It is on already; the app shows you the one VACS setting to tick.</div></div>
      <div class="p-3 border-2 border-blue-10 bg-blue-3"><div class="font-bold mb-1">Help built in</div><div class="text-sm opacity-90">The first-time setup shows the how-to video, and the app tells you plainly if a program could not be started and why.</div></div>
    </div>

    <h2 class="text-2xl font-bold mb-2">What you need</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6 opacity-90">
      <li><b>Windows 10 or Windows 11</b>, 64-bit. (It uses Microsoft's WebView2, which is already part of Windows 11 and up-to-date Windows 10. If yours is missing, the installer fetches it, so an internet connection is needed to install.)</li>
      <li><b>EuroScope</b> and the official <b>Gander Oceanic sector package</b> from <a class="underline" href="https://files.aero-nav.com/EXCXO">AeroNav</a>.</li>
      <li>Your own <b>API key</b>, free at <a class="underline" href="/get-key">vnaaats.com/get-key</a> for all oceanic controllers (Gander Oceanic controllers can also use <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a>). The display opens only while you are connected to VATSIM on an oceanic position. Keep your key private.</li>
      <li>Optional: <a class="underline" href="https://vacs.network/">VACS</a> and <a class="underline" href="https://audio.vatsim.net/">Audio for VATSIM</a> or <a class="underline" href="https://github.com/pierr3/TrackAudio/releases">TrackAudio</a> if you want the app to open them.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">How it works</h2>
    <ol class="list-decimal ml-6 space-y-1 mb-6 opacity-90">
      <li>Download and run the installer.</li>
      <li>On first launch, play the welcome and do the one-time setup (folder, plugin check, key, your programs).</li>
      <li>Load the plugin in EuroScope and open the vNAAATS screen once. The <a class="underline" href="/guide">install guide</a> walks you through it, with a video.</li>
      <li>From then on: open the app, sign in, choose your position. Everything else starts for you.</li>
    </ol>

    <div class={CARD}>
      <div class="font-bold mb-1">Prefer a browser?</div>
      <div class="text-sm opacity-90">The display works in any browser at <a class="underline" href="/fdd">vnaaats.com/fdd</a>. You then install the EuroScope plugin yourself: see the <a class="underline" href="/download">downloads page</a>.</div>
    </div>

    <p class="text-sm opacity-80">Independent community project, not run or endorsed by VATSIM. <a class="underline" href="/guide">Install guide</a> &middot; <a class="underline" href="/manual">User manual</a> &middot; <a class="underline" href="/privacy">Privacy and data handling</a> &middot; <a class="underline" href="/code-signing">Code signing policy</a>.</p>
  </div>
</div>
