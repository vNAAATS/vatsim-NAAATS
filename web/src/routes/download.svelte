<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";

  const repo = "garythms/vatsim-NAAATS-v2";
  let version: string = "";
  let published: string = "";
  let zipUrl: string = "https://github.com/" + repo + "/releases/tag/latest-build";
  let direct: boolean = false;
  let dllUrl: string = "";
  let note: string = "";
  const installer = "/get/fdd-windows";

  onMount(async () => {
    site.setTitle("Download FDD for Windows");
    document.body.style.opacity = "100%";
    try {
      const v = await fetch("https://raw.githubusercontent.com/" + repo + "/server-and-squawk/pluginversion.txt");
      if (v.ok) version = (await v.text()).trim();
    } catch (e) { /* the page still works without it */ }
    try {
      const r = await fetch("https://api.github.com/repos/" + repo + "/releases/tags/latest-build");
      if (r.ok) {
        const rel = await r.json();
        const a = (rel.assets ?? []).find((x: any) => x.name.endsWith(".zip"));
        const d = (rel.assets ?? []).find((x: any) => x.name.endsWith(".dll"));
        if (d) dllUrl = d.browser_download_url;
        if (a) { zipUrl = a.browser_download_url; direct = true; published = new Date(a.updated_at).toUTCString(); }
      }
    } catch (e) { note = "Could not check GitHub just now; the button opens the download page instead."; }
  });
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-2xl w-full">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">Download vNAAATS</h1>
    <div class="opacity-80 mb-6">Version 2 &ndash; Modernised by Gary Thomas. For EuroScope on Windows.</div>

    <!-- 1. The app: the way most controllers should start -->
    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-xs font-bold tracking-wider text-yellow-300 mb-1">RECOMMENDED</div>
      <div class="text-2xl font-bold">FDD for Windows</div>
      <p class="opacity-90 my-2">The Flight Data Display in its own window, with a one-time setup that gets everything else ready for you. It is the same display as in your browser, so updates to the display arrive by themselves.</p>
      <ul class="list-disc ml-6 space-y-1 mb-3 text-sm opacity-90">
        <li>Checks your plugin, airline list and font and updates them when you say so (you are asked first).</li>
        <li>Saves your key into <b>vnaaats.cfg</b> so you never edit the file by hand.</li>
        <li>Opens <b>EuroScope</b>, <b>VACS</b> and your audio client (<b>Audio for VATSIM</b> or <b>TrackAudio</b>) for you once you choose your position.</li>
        <li>Remembers your sign-in if you ask it to (kept in the Windows Credential Manager).</li>
        <li>The setup happens once. After that the app opens straight to the display.</li>
      </ul>
      <a href={installer} rel="external" class="inline-block px-5 py-2 mr-2 mb-2 font-bold bg-blue-10 text-black">Download FDD for Windows</a>
      <a href="/windows" class="inline-block px-5 py-2 mb-2 border-2 border-blue-10">What is it? Requirements</a>
      <div class="text-xs opacity-80 mt-2">Windows 10 or 11. The installer is not code-signed yet, so Windows may say "Windows protected your PC": choose <b>More info</b>, then <b>Run anyway</b>. See the <a class="underline" href="/code-signing">code signing policy</a> and <a class="underline" href="/privacy">privacy and data handling</a>.</div>
      <div class="text-sm mt-4"><b>First time?</b> Follow the <a class="underline" href="/guide">install guide</a>. In short: install the Gander sector package, install FDD for Windows, do the one-time setup, then load the plugin in EuroScope once.</div>
    </div>

    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: FDD for Windows, the One Fits All Program</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/training/vNAAATS-howto-5-FDD-for-Windows.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/training/vNAAATS-howto-5-FDD-for-Windows.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">About four minutes: what the app does, why it is better than a browser, how to install it (including the Windows SmartScreen message), the one-time setup and how updates work.</div>
    </div>

    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold mb-2">Video: loading vNAAATS into EuroScope</div>
      <video class="w-full bg-black" controls preload="metadata" playsinline src="/vNAAATS-install-howto.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/vNAAATS-install-howto.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">About a minute and a quarter: opening your profile, loading the sector file and plugin, then adding the vNAAATS display and the ASR file. The app shows this video in its first-time setup too.</div>
    </div>

    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold mb-2">Video: how to use vNAAATS</div>
      <video class="w-full bg-black" controls preload="metadata" playsinline src="/vNAAATS-intro.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/vNAAATS-intro.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">About two and a half minutes: what vNAAATS is and how to get started. The flights shown are demo data.</div>
    </div>

    <!-- 2. What you need -->
    <h2 class="text-2xl font-bold mb-2">What you need</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li>EuroScope, and the official <b>Gander Oceanic sector package</b> from AeroNav (below), which also keeps your AIRAC up to date.</li>
      <li>Your own <b>API key</b>, free at <a class="underline" href="/get-key">vnaaats.com/get-key</a> for all oceanic controllers (Gander Oceanic controllers can also use <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a>). The display opens only while you are connected to VATSIM on an oceanic position. Keep your key private.</li>
      <li>The display shows nothing until you have signed in and chosen a position, to protect pilot and controller data.</li>
    </ul>

    <div class="mb-6 p-4 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold">Gander Oceanic sector package (AeroNav)</div>
      <p class="opacity-90 my-2">The official sector files, kept up to date each AIRAC by AeroNav. Install it first, then install FDD for Windows (or the plugin by hand, below) and follow the <a href="/guide" class="underline">install guide</a>.</p>
      <a href="https://files.aero-nav.com/EXCXO" rel="external" class="inline-block px-5 py-2 font-bold bg-blue-10 text-black">Open files.aero-nav.com/EXCXO</a>
    </div>

    <!-- 3. Without the app -->
    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold">Not using the app? Install the plugin by hand</div>
      <p class="text-sm opacity-90 my-2">You can still use the display in any browser at <a href="/fdd" class="underline">vnaaats.com/fdd</a> and install the EuroScope plugin yourself.</p>
      <details>
        <summary class="cursor-pointer underline text-sm">Show the plugin downloads{version ? " (version " + version + ")" : ""}</summary>
        <div class="mt-3">
          {#if published}<div class="text-sm opacity-80 mb-2">Built {published}</div>{/if}
          <a href={zipUrl} class="inline-block px-5 py-2 mr-2 mb-2 font-bold bg-blue-10 text-black">{direct ? "Download the plugin pack (zip)" : "Open the download page"}</a>
          {#if dllUrl}<a href={dllUrl} class="inline-block px-5 py-2 mb-2 border-2 border-blue-10">VatsimNAAATS.dll only</a>{/if}
          {#if note}<div class="text-sm text-yellow-300 mt-1">{note}</div>{/if}
          <div class="text-sm opacity-80 mb-3">The pack holds the DLL, the screen file <b>vNAAATS.asr</b>, the airline list, the font, an example config and a README.</div>
          <div class="text-sm"><b>Also needed in the same folder as the DLL (all in the pack):</b></div>
          <div class="mt-2">
            <a href="/ICAO_Airlines.txt" rel="external" download class="inline-block px-4 py-1 mr-2 mb-2 border-2 border-blue-10">ICAO_Airlines.txt</a>
            <a href="/vNAAATS.ttf" rel="external" download class="inline-block px-4 py-1 mr-2 mb-2 border-2 border-blue-10">vNAAATS.ttf</a>
            <a href="/vnaaats.example.cfg" rel="external" download class="inline-block px-4 py-1 mb-2 border-2 border-blue-10">vnaaats.example.cfg</a>
          </div>
          <div class="text-sm opacity-80">ICAO_Airlines.txt shows the selected aircraft's radio callsign (e.g. BAW227 (Speedbird)) at the bottom centre of the radar. <b>vNAAATS.ttf</b> is the font the radar screen uses; without it the letters look wrong. The example config is for new installs only: rename it to <b>vnaaats.cfg</b> and add your key. Never copy it over an existing vnaaats.cfg.</div>
          <div class="text-sm mt-3"><b>Updating by hand:</b> close EuroScope and replace <b>only VatsimNAAATS.dll</b>. <b>Keep your existing vnaaats.cfg</b>. Make sure it is the DLL EuroScope actually loads (Other SET &gt; Plug-ins shows its folder). If it will not load, install the Microsoft Visual C++ 2015-2022 Redistributable (x86).</div>
        </div>
      </details>
    </div>

    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold">User guide (PDF)</div>
      <div class="text-sm opacity-80 mb-3">The install guide and the user manual in one file, for printing or offline. The <a href="/guide" class="underline">install guide</a> and <a href="/manual" class="underline">user manual</a> pages on this site are always the newest, so use them if the two ever differ.</div>
      <a href="/vNAAATS-user-guide.pdf" rel="external" class="inline-block px-5 py-2 mr-2 mb-2 font-bold bg-blue-10 text-black">Open the guide (PDF)</a>
      <a href="/vNAAATS-user-guide.pdf" rel="external" download class="inline-block px-5 py-2 mb-2 border-2 border-blue-10">Download</a>
    </div>

    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold">Icon key (PDF)</div>
      <div class="text-sm opacity-80 mb-3">What every symbol, colour and flashing state means on the display and in EuroScope. The <a href="/icons" class="underline">icon key page</a> is always the newest.</div>
      <a href="/vNAAATS-icon-key.pdf" rel="external" class="inline-block px-5 py-2 mr-2 mb-2 font-bold bg-blue-10 text-black">Open the icon key (PDF)</a>
      <a href="/vNAAATS-icon-key.pdf" rel="external" download class="inline-block px-5 py-2 mb-2 border-2 border-blue-10">Download</a>
    </div>

    <a href="/get-key" style="display:none" aria-hidden="true" tabindex="-1">.</a>
    <p class="text-sm opacity-80">Please report problems at
      <a class="underline" href="https://github.com/{repo}/issues">github.com/{repo}/issues</a>. Source code:
      <a class="underline" href="https://github.com/{repo}">github.com/{repo}</a>.</p>
  </div>
</div>
