<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  const repo = "garythms/vatsim-NAAATS-v2";
  const installer = "/get/fdd-windows";
  let version: string = "";
  let zipUrl: string = "https://github.com/" + repo + "/releases/tag/latest-build";
  let direct: boolean = false;
  onMount(async () => {
    site.setTitle("Install guide");
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
        if (a) { zipUrl = a.browser_download_url; direct = true; }
      }
    } catch (e) { /* falls back to the release page */ }
  });
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-2xl w-full">
    <a href="https://vnaaats.com/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">vNAAATS V2 - first-time install guide</h1>
    <div class="opacity-80 mb-6">For controllers using EuroScope on Windows. Allow about 15 minutes. This guide uses <b>FDD for Windows</b>; if you prefer a browser, see "Without the app" at the end.</div>

    <div class="p-5 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="text-lg font-bold">FDD for Windows (<a class="underline text-sm" href="/windows">what is it?</a>)</div>
      <a href={installer} rel="external" class="inline-block my-2 px-5 py-2 font-bold bg-blue-10 text-black">Download FDD for Windows</a>
      <div class="text-sm opacity-80">The app installs and checks the plugin for you. Your <b>key</b> is not in it: get it from <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a> (certified controllers).</div>
    </div>

    <p class="mb-6 p-3 border-2 border-blue-10 bg-blue-3">After installing, read the <a class="underline font-bold" href="https://vnaaats.com/manual">user manual</a> to see how everything works. A short video of the EuroScope steps is on the <a class="underline" href="/download">download page</a>, and the app shows it during its setup.</p>

    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: FDD for Windows, the One Fits All Program</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/training/vNAAATS-howto-5-FDD-for-Windows.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/training/vNAAATS-howto-5-FDD-for-Windows.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">About four minutes: what the app does, why it is better than a browser, how to install it (including the Windows SmartScreen message), the one-time setup and how updates work.</div>
    </div>

    <h2 class="text-2xl font-bold mb-2">Before you start</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li>EuroScope installed and working, and a VATSIM controller rating.</li>
      <li>The official <b>Gander Oceanic sector package from AeroNav</b> (it also keeps your AIRAC up to date).</li>
      <li>Your <b>key</b> from <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a>. Do not share the key.</li>
      <li>Optional: <b>VACS</b> (voice between controllers, <a class="underline" href="https://vacs.network/">vacs.network</a>) and an audio client: <a class="underline" href="https://audio.vatsim.net/">Audio for VATSIM</a> or <a class="underline" href="https://github.com/pierr3/TrackAudio/releases">TrackAudio</a>.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">1. Get the Gander package</h2>
    <p class="mb-6 opacity-90">Download and run the Gander Oceanic package from <b>AeroNav</b>: <a class="underline" href="https://files.aero-nav.com/EXCXO">files.aero-nav.com/EXCXO</a>. Open EuroScope with that package once, so you can see the oceanic map, then close EuroScope. Remember where your <b>EXOCO</b> folder is (the folder that holds <b>Plugins</b>).</p>

    <h2 class="text-2xl font-bold mb-2">2. Install FDD for Windows</h2>
    <p class="mb-3 opacity-90">Run the installer you downloaded above. Windows may say <b>"Windows protected your PC"</b> because the installer is not code-signed yet: choose <b>More info</b>, then <b>Run anyway</b>. The app opens when it has finished.</p>

    <h2 class="text-2xl font-bold mb-2">3. The one-time setup</h2>
    <p class="mb-3 opacity-90">The first time it opens you see a short welcome and then the setup screen. It only appears once; afterwards the app opens straight to the display. (Open it again any time from <b>Settings &gt; App setup</b> in the display.) The screen has a how-to video at the top. Work down the steps:</p>
    <ol class="list-decimal ml-6 space-y-1 mb-3">
      <li><b>Your EXOCO folder</b> - choose the folder from step 1.</li>
      <li><b>The vNAAATS plugin</b> - the app looks for <b>VatsimNAAATS.dll</b> in EXOCO\Plugins\vNAAATS. If it is missing or old, press <b>Update now</b> with EuroScope closed. It also installs the airline list and the vNAAATS font next to the DLL.</li>
      <li><b>Your key</b> - get your API key at <a class="underline" href="/get-key">vnaaats.com/get-key</a> (Gander Oceanic controllers can also use their profile on the Gander Oceanic website) and paste it here. The app writes it into <b>vnaaats.cfg</b> for you.</li>
      <li><b>EuroScope</b> - point the app at <b>EuroScope.exe</b>. Leave "Open EuroScope for me when I choose my position" ticked.</li>
      <li><b>VACS</b> and <b>Audio client</b> (both optional) - point the app at the program. Each has a tick to open it for you after you choose your position.</li>
    </ol>
    <p class="mb-3 opacity-90"><b>VACS call alert (optional):</b> to see incoming VACS calls on the headphones button in the FDD, open <b>VACS</b> (version 2.1.0 or newer), go to <b>Advanced Settings</b> and tick <b>Enable remote control</b>. For safety, change the <b>Listen addr.</b> box beneath it to <b>127.0.0.1:9600</b>, so only your own computer can connect: the default lets any device on your network connect, with no password. The app then shows a green tick, and the headphones button flashes with a pop-up when someone calls you.</p>
    <p class="mb-6 opacity-90">When every line is green, press <b>Continue</b>. If the plugin DLL is not in your folder yet, get the plugin pack from the <a class="underline" href="/download">download page</a> (open "Not using the app?"), put <b>VatsimNAAATS.dll</b> in EXOCO\Plugins\vNAAATS, and run the setup again.</p>

    <h2 class="text-2xl font-bold mb-2">4. Load the plugin in EuroScope (once)</h2>
    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: loading vNAAATS into EuroScope</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/vNAAATS-install-howto.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/vNAAATS-install-howto.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">About a minute and a quarter: opening your profile, loading the sector file and plugin, then adding the vNAAATS display and the ASR file.</div>
    </div>

    <ol class="list-decimal ml-6 space-y-1 mb-6">
      <li>Open EuroScope with the Gander package.</li>
      <li>Other SET &gt; Plug-ins &gt; Load, and pick <b>VatsimNAAATS.dll</b> (the one in EXOCO\Plugins\vNAAATS that the app manages).</li>
      <li><b>Only vNAAATS should be loaded.</b> Remove any other plugin from that list while you work oceanic. (Discord is fine.)</li>
    </ol>

    <h2 class="text-2xl font-bold mb-2">5. Open the vNAAATS screen (once)</h2>
    <ol class="list-decimal ml-6 space-y-1 mb-3">
      <li>Open the file menu and choose <b>New vNAAATS radar screen</b>. You get a blank screen with a blue bar at the top.</li>
      <li>Open the file menu again, choose <b>Open</b>, and pick <b>vNAAATS.asr</b> (it is in the plugin pack zip on the <a class="underline" href="/download">download page</a>).</li>
      <li>If EuroScope warns about a different sector file, pick your <b>latest Gander (EXCXO) sector file</b>.</li>
      <li>The oceanic map appears. Choose <b>Save</b> so you never have to do this again.</li>
    </ol>
    <p class="mb-6 opacity-80 text-sm">If the screen stays blank, check that the Gander package is the one loaded and try the steps again in order.</p>

    <h2 class="text-2xl font-bold mb-2">6. Every day after that</h2>
    <ol class="list-decimal ml-6 space-y-1 mb-3">
      <li>Open <b>FDD for Windows</b>. It opens <b>EuroScope</b>, <b>VACS</b> and your audio client for you (each only if ticked in setup) and shows <b>Awaiting an active VATSIM connection</b>.</li>
      <li>In EuroScope, connect to VATSIM as your oceanic position (for example NAT_FSS, CZQO_CTR or EGGX_CTR) and open your saved vNAAATS screen.</li>
      <li>Within a few seconds the display opens by itself on that position. There is no sign-in or position to choose: your key and your live VATSIM connection are the sign-in. You are then asked about natTrak, which has to be signed in to each session.</li>
      <li>If an update is available the app tells you and asks before it downloads anything. Close EuroScope first, then press <b>Update now</b>. Your <b>vnaaats.cfg</b> and key are never touched.</li>
    </ol>
    <p class="mb-6 opacity-90">Check it works: open <b>vNAAATS.log</b> (next to the DLL); its first line shows the version. Nothing is shown on the display until you are connected to VATSIM on an oceanic position.</p>

    <h2 class="text-2xl font-bold mb-2">Let the display type commands for you (autotype)</h2>
    <p class="mb-6 opacity-90">The example config has <b>autotype=1</b> switched on. The Send buttons for SELCAL, contact me and RCL REQ in the display then type the command into EuroScope for you, and your EuroScope can send the <b>automatic RCL REQ reminders</b> (see the manual). If you would rather copy and paste, turn it off in <b>Settings</b> in the display (the automatic reminders then come from another controller's EuroScope).</p>

    <h2 class="text-2xl font-bold mb-2">If something goes wrong</h2>
    <ul class="list-disc ml-6 space-y-2 mb-6">
      <li><b>The installer will not run, or Windows blocks it:</b> choose <b>More info</b> then <b>Run anyway</b> on the "Windows protected your PC" screen. If your antivirus quarantines it, restore it and add an exception: the installer is not code-signed yet (see the <a class="underline" href="/code-signing">code signing policy</a>). No administrator rights are needed.</li>
      <li><b>The app opens to a blank or white window, or says it cannot reach vnaaats.com:</b> check your internet connection and that a firewall or VPN is not blocking <b>vnaaats.com</b>, then close and reopen the app. If it still stays blank, install the free <b>Microsoft WebView2 Runtime</b> from Microsoft and try again (Windows 11 already has it).</li>
      <li><b>The setup screen keeps coming back:</b> a red line in the setup says what is missing (usually the folder, key or plugin). Fix that line and press <b>Continue</b>. It only returns to full setup if the basics are no longer in place.</li>
      <li><b>"Update now" fails:</b> close EuroScope first. Windows will not let the app replace a plugin that EuroScope has open. The message shown tells you which file it could not write.</li>
      <li><b>A program says it "needs administrator rights" when the app opens it:</b> open it yourself once, or right-click the app and choose <b>Run as administrator</b>. Some audio clients ask Windows for this.</li>
      <li><b>I chose a position but nothing opened:</b> the programs are only opened for lines you ticked in <b>Settings &gt; App setup</b>, once per session. A notice at the bottom of the display says if one failed.</li>
      <li><b>The headphones (VACS) button stays grey:</b> in VACS open <b>Advanced Settings</b> and make sure <b>Enable remote control</b> is ticked, VACS is running, and the port matches the one in the headphones button (default 9600). Your browser or Windows may ask whether the FDD may reach devices on your network: choose Allow.</li>
      <li><b>The app says the plugin or font is missing or old:</b> close EuroScope and press <b>Update now</b> in the setup. If it cannot write the file, check EuroScope really is closed and that the folder is not read-only.</li>
      <li><b>The radar screen shows wrong letters:</b> <b>vNAAATS.ttf</b> is missing from the plugin folder. The app's <b>Update now</b> installs it.</li>
      <li><b>EuroScope, VACS or the audio client did not open by itself:</b> a small notice at the bottom of the display says why. Check the program is chosen in <b>Settings &gt; App setup</b>.</li>
      <li><b>EuroScope closes when it starts:</b> check <b>server=https://vnaaats.com</b> (with https) in <b>vnaaats.cfg</b>. Install the Microsoft Visual C++ 2015-2022 Redistributable (x86) if the plugin will not load at all.</li>
      <li><b>Wrong version in the log:</b> EuroScope is loading a different copy of the DLL. Other SET &gt; Plug-ins shows which folder it is using.</li>
      <li><b>The map is wrong or not oceanic:</b> you have another sector file loaded. Use the Gander package from AeroNav.</li>
      <li><b>No flights appear or changes do not save:</b> the key is missing, wrong or out of date (keys last 90 days). Get a new one from <a class="underline" href="https://ganderoceanic.ca/my/vnaaats">ganderoceanic.ca/my/vnaaats</a> and paste it in <b>Settings &gt; App setup</b>.</li>
      <li><b>Still stuck:</b> send <b>vNAAATS.log</b> to the Gander Oceanic team.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Without the app</h2>
    <p class="mb-3 opacity-90">You can use the display in any browser at <a class="underline" href="/fdd">vnaaats.com/fdd</a> and install the plugin by hand. Get the plugin pack from the <a class="underline" href="/download">download page</a>{version ? " (version " + version + ")" : ""}{direct ? "" : ""}: <a class="underline" href={zipUrl}>VatsimNAAATS-plugin.zip</a>. Unzip it somewhere you will remember. Put <b>VatsimNAAATS.dll</b>, <b>ICAO_Airlines.txt</b>, <b>vNAAATS.ttf</b> and your <b>vnaaats.cfg</b> in the <b>same folder</b>. For the config, rename <b>vnaaats.example.cfg</b> to <b>vnaaats.cfg</b>, open it in Notepad, set <b>server=https://vnaaats.com</b> and paste your <b>key=</b>. Then follow steps 4 and 5 above.</p>
    <p class="mb-6 opacity-90"><b>Updating by hand:</b> close EuroScope and replace <b>only VatsimNAAATS.dll</b>. Keep your existing vnaaats.cfg and your saved screen. Do not copy the example config over it or you will lose your key.</p>

    <p class="text-sm opacity-80">Independent community project, not run or endorsed by VATSIM.
      <a class="underline" href="/privacy">Privacy and data handling</a>.</p>
  </div>
</div>
