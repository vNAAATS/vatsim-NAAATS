<script lang="ts">
  import { onMount } from "svelte";
  import * as app from "../../typescript/fdd/app";
  import * as sess from "../../typescript/fdd/session";
  import * as vc from "../../typescript/fdd/vacs";
  const { vacs } = vc;

  let show: boolean = false;
  let mode: "full" | "update" | "ready" = "full";   // full setup, the "an update / key is needed" question, or the "ready to open?" question
  let firstTime: boolean = true;
  let st: app.AppStatus | null = null;
  let busy: boolean = false;
  let msg: string = "";
  let key: string = "";
  let autoES: boolean = true;
  let autoVacs: boolean = true;
  let audio: app.AudioChoice = "none";
  let autoAudio: boolean = true;

  const SEEN = "vnaaats.appSetupSeen";
  function seen(): boolean { try { return sessionStorage.getItem(SEEN) === "1"; } catch (e) { return false; } }

  let problem: string = "";                         // why the app did not answer
  async function refresh(): Promise<void> {
    try {
      st = await Promise.race([app.status(), new Promise<never>((_, no) => setTimeout(() => no("the app did not answer within 15 seconds"), 15000))]);
      problem = "";
    } catch (e) { problem = app.errText(e); console.error("vNAAATS app:", e); }
  }
  // The one-time setup is remembered. After that the screen only appears when something needs attention:
  // an update (the controller is asked first), or something that stopped being right (folder moved, key gone...).
  const DONE = "vnaaats.appSetupDone";
  function isDone(): boolean { try { return localStorage.getItem(DONE) === "1"; } catch (e) { return false; } }
  function prefs(): void {
    autoES = app.autoGet("euroscope"); autoVacs = app.autoGet("vacs");
    audio = app.audioGet(); autoAudio = app.autoGet(audio === "none" ? "afv" : audio);
  }
  onMount(async () => {
    if (!app.inApp() || seen()) return;
    await app.welcomeDone;
    await refresh();
    prefs();
    firstTime = !isDone();
    const basicsOk = !!st && st.root !== null && st.dll_found && st.cfg_has_key;
    if (!firstTime && basicsOk) {
      const filesOk = st!.airlines_up_to_date !== false && st!.font_found !== false;       // (not the reactive value: it may not have updated yet)
      const keyBad = ["unknown", "expired", "revoked"].includes(st!.key_status ?? "");
      mode = (!filesOk || st!.up_to_date === false || st!.sector_up_to_date === false || keyBad) ? "update" : "ready";   // something to fix: ask first; all good: "ready to open?"
      show = true;
      return;
    }
    mode = "full";
    show = true;
  });
  const { setupOpen } = app;
  $: if ($setupOpen && !show) { mode = "full"; firstTime = false; refresh().then(() => { prefs(); show = true; }); }

  $: listOk = !!st && st.airlines_up_to_date !== false && st.font_found !== false;
  $: keyBad = !!st && ["unknown", "expired", "revoked"].includes(st.key_status ?? "");
  $: pluginOk = !!st && st.dll_found && st.up_to_date !== false && listOk;
  $: sectorOk = !st || st.sector_up_to_date !== false;
  $: allGood = !!st && pluginOk && sectorOk && !keyBad;
  // the programs the "Yes" button will open (ticked on the setup screen and located)
  $: toOpen = !st ? [] : ([["euroscope", "EuroScope"], ["vacs", "VACS"], ...(audio === "none" ? [] : [[audio, AUDIO_NAME[audio]]])] as string[][]).filter(([k]) => app.autoGet(k as app.AutoKind) && !!(st as any)[k]).map(([, n]) => n);
  $: ready = !!st && st.root !== null && st.dll_found && st.up_to_date === true && listOk && st.sector_up_to_date !== false && st.cfg_has_key && !keyBad;

  async function run(f: () => Promise<unknown>, ok: string = ""): Promise<void> {
    busy = true; msg = "";
    try { await f(); if (ok) msg = ok; } catch (e) { msg = "✖ " + app.errText(e); }
    await refresh();
    busy = false;
  }
  const folder = () => run(() => app.pickFolder());
  // The plugin first (if it or its files need it), then the sector pack. Both need EuroScope closed; the app says so if it is open.
  const update = () => run(async () => {
    const parts: string[] = [];
    if (st && (st.up_to_date === false || st.airlines_up_to_date === false || st.font_found === false)) parts.push(await app.updatePlugin());
    if (st && st.sector_up_to_date === false) parts.push(await app.updateSector());
    msg = "Updated: " + (parts.join(" and ") || "everything is up to date") + ".";
  });
  const saveKey = () => run(async () => { await app.saveKey(key); sess.rememberKey(key); key = ""; }, "Key saved.");
  const VACS_URL = "https://vacs.network/";
  const pick = (k: app.ProgKind) => run(async () => {
    msg = "A file window should open (it can appear behind this one)...";
    const r = await app.pickExe(k);
    msg = r ? "" : "Nothing was chosen.";
  });
  const AUDIO_URL: Record<string, string> = { afv: "https://audio.vatsim.net/", trackaudio: "https://github.com/pierr3/TrackAudio/releases" };
  const AUDIO_NAME: Record<string, string> = { afv: "Audio for VATSIM", trackaudio: "TrackAudio" };
  function chooseAudio(c: app.AudioChoice): void { audio = c; app.audioSet(c); if (c !== "none") autoAudio = app.autoGet(c); }
  const getAudio = () => run(() => app.openExternal(AUDIO_URL[audio], true), "Opening the download page in your browser...");
  const getVacs = () => run(() => app.openExternal(VACS_URL, true), "Opening the VACS page in your browser...");
  function done(): void {
    if (ready) { try { localStorage.setItem(DONE, "1"); } catch (e) { /* the setup shows again next time */ } }
    if (firstTime) { firstTime = false; msg = ""; refresh().then(() => { mode = "ready"; }); return; }      // the first setup ends with the same "ready to open?" question
    try { sessionStorage.setItem(SEEN, "1"); } catch (e) { /* shown again next time */ }
    show = false;
    app.setupOpen.set(false);
  }
  // Nothing (EuroScope, VACS, audio) is started until the controller says yes on the "ready" screen.
  async function openNow(): Promise<void> {
    busy = true;
    try { await app.autoLaunch(); } catch (e) { /* the notice shows what did not start */ }
    try { sessionStorage.setItem(SEEN, "1"); } catch (e) { /* asked again next time */ }
    busy = false; show = false;
  }
  function notNow(): void { msg = ""; refresh().then(() => { mode = "ready"; }); }          // updates skipped: still ask before opening anything
  function later(): void { try { sessionStorage.setItem(SEEN, "1"); } catch (e) { /* asked again next time */ } show = false; }
  let updated: boolean = false;
  async function doUpdate(): Promise<void> { await update(); updated = !msg.startsWith("✖"); }
  const getKey = () => run(() => app.openExternal("https://vnaaats.com/access", true), "Opening vnaaats.com/access in your browser...");
  const KEY_WHY: Record<string, string> = { unknown: "was not recognised", expired: "has expired", revoked: "has been withdrawn" };
  const STEP = "mb-3 p-2 border border-blue-6 bg-blue-4";
  const BTN = "px-3 py-1 font-bold bg-blue-10 text-black";
</script>

{#if show && mode === "update"}
  <div class="fixed inset-0 z-[60] flex items-center justify-center" style="background: rgba(11,27,48,0.92);">
    <div class="w-[30rem] max-w-full bg-blue-3 border-2 border-blue-10 text-white font-regular p-4">
      <div class="text-xl font-bold mb-2">{updated ? "Updated" : "Before you start"}</div>
      {#if keyBad}
        <div class="mb-3 p-2 border-2 border-red-400 text-sm">
          <b class="text-red-300">API key update is required.</b> Your key {KEY_WHY[st?.key_status ?? "unknown"] ?? "is not accepted"}. The FDD will not work with EuroScope without a working key.
          <div class="mt-2 flex gap-2">
            <button class="px-3 py-1 {BTN}" disabled={busy} on:click={getKey}>Get a new key now</button>
          </div>
          <div class="mt-2 text-xs opacity-80">It opens vnaaats.com/access in your browser. Copy the new key, paste it here and save:</div>
          <form class="flex gap-2 mt-1" on:submit|preventDefault={saveKey}>
            <input class="flex-1 px-2 py-1 text-black" placeholder="1234567.abc..." bind:value={key} autocomplete="off" />
            <button class={BTN} disabled={busy || !key}>Save</button>
          </form>
        </div>
      {/if}
      {#if st && !updated && !(pluginOk && sectorOk)}
        <div class="text-sm mb-1">An update is available:</div>
        <ul class="list-disc ml-5 mb-3 text-sm">
          {#if st.up_to_date === false}<li>A newer vNAAATS plugin{st.latest_version ? " (" + st.latest_version + ")" : ""}</li>{/if}
          {#if st.airlines_up_to_date === false}<li>A newer airline list (ICAO_Airlines.txt)</li>{/if}
          {#if st.font_found === false}<li>The vNAAATS font (vNAAATS.ttf) is missing</li>{/if}
          {#if st.sector_up_to_date === false}<li>A newer sector pack{st.sector_name ? " (" + st.sector_name + ")" : ""}</li>{/if}
        </ul>
      {/if}
      {#if updated}
        <div class="mb-3 text-green-300">✔ {msg.replace("Updated: ", "Updated ")}</div>
        <button class="w-full py-2 font-bold bg-blue-10 text-black" disabled={busy} on:click={notNow}>Continue</button>
      {:else}
        {#if st && st.euroscope_running && !(pluginOk && sectorOk)}<div class="mb-2 text-sm text-yellow-300">EuroScope is open. Close it first, then press <b>Update now</b>.</div>{/if}
        {#if !(pluginOk && sectorOk)}<div class="mb-3 text-sm opacity-90">Close EuroScope first, then press <b>Update now</b>. Your vnaaats.cfg (and your key) is not touched.</div>{/if}
        {#if msg}<div class="mb-2 text-sm text-yellow-300">{msg}</div>{/if}
        <div class="flex gap-2">
          {#if !(pluginOk && sectorOk)}<button class="flex-1 py-2 {BTN}" disabled={busy} on:click={doUpdate}>{busy ? "Updating..." : "Update now"}</button>{/if}
          <button class="flex-1 py-2 border border-blue-6" disabled={busy} on:click={notNow}>{pluginOk && sectorOk ? "Continue" : "Not now"}</button>
        </div>
      {/if}
    </div>
  </div>
{:else if show && mode === "ready"}
  <div class="fixed inset-0 z-[60] flex items-center justify-center" style="background: rgba(11,27,48,0.92);">
    <div class="w-[30rem] max-w-full bg-blue-3 border-2 border-blue-10 text-white font-regular p-4">
      <div class="text-xl font-bold mb-2">{allGood ? "You are up to date" : "Check before you start"}</div>
      {#if st}
        <ul class="mb-3 text-sm space-y-1">
          <li class={pluginOk ? "text-green-300" : "text-yellow-300"}>{pluginOk ? "✔ Plugin" + (st.latest_version ? " " + st.latest_version : "") + " and airline list are the latest" : "⚠ The plugin or its files are not up to date"}</li>
          {#if st.sector_up_to_date !== null && st.sector_up_to_date !== undefined}
            <li class={sectorOk ? "text-green-300" : "text-yellow-300"}>{sectorOk ? "✔ Sector pack is the latest" : "⚠ A newer sector pack is available (not installed)"}</li>
          {/if}
          {#if st.key_status === "ok"}<li class="text-green-300">✔ Your API key works</li>
          {:else if keyBad}<li class="text-red-300">✖ Your API key {KEY_WHY[st.key_status ?? "unknown"] ?? "is not accepted"}</li>
          {:else if !st.cfg_has_key}<li class="text-yellow-300">⚠ No API key saved yet</li>{/if}
        </ul>
        {#if keyBad}
          <div class="mb-3 p-2 border-2 border-red-400 text-sm">
            <b class="text-red-300">API key update is required.</b> The FDD will not work with EuroScope without it.
            <div class="mt-2"><button class={"px-3 py-1 " + BTN} disabled={busy} on:click={getKey}>Get a new key now</button></div>
            <form class="flex gap-2 mt-2" on:submit|preventDefault={saveKey}>
              <input class="flex-1 px-2 py-1 text-black" placeholder="Paste the new key" bind:value={key} autocomplete="off" />
              <button class={BTN} disabled={busy || !key}>Save</button>
            </form>
          </div>
        {/if}
      {/if}
      {#if msg}<div class="mb-2 text-sm text-yellow-300">{msg}</div>{/if}
      {#if toOpen.length}
        <div class="mb-3 text-sm">{allGood ? "No updates are needed." : "Some things above need attention."} Ready to open {toOpen.join(", ")} now?</div>
        <div class="flex gap-2">
          <button class="flex-1 py-2 {BTN}" disabled={busy} on:click={openNow}>{busy ? "Opening..." : allGood ? "Yes, open now" : "Open anyway"}</button>
          <button class="flex-1 py-2 border border-blue-6" disabled={busy} on:click={later}>Not now</button>
        </div>
      {:else}
        <div class="mb-3 text-sm">{allGood ? "No updates are needed." : "Some things above need attention."} Nothing is set to open automatically (change that under Settings, App setup).</div>
        <button class="w-full py-2 font-bold {BTN}" on:click={later}>Continue</button>
      {/if}
    </div>
  </div>
{:else if show}
  <div class="fixed inset-0 z-[60] flex items-center justify-center overflow-y-auto" style="background: #0b1b30;">
    <div class="w-[34rem] max-w-full max-h-full overflow-y-auto bg-blue-3 border-2 border-blue-10 text-white font-regular p-4">
      <div class="text-xl font-bold mb-1">FDD for Windows - {firstTime ? "one-time setup" : "setup"}</div>
      <div class="mb-3 text-sm opacity-80">{firstTime ? "This only needs doing once. Next time the app opens straight to the display and only asks you if an update is needed." : "A quick check that this computer is ready."}{st ? " (app " + st.app_version + ")" : ""}</div>
      <details class="mb-3 border border-blue-6 bg-blue-4 p-2" open={firstTime}>
        <summary class="cursor-pointer font-bold text-sm">Watch the how-to video (about a minute): getting vNAAATS working in EuroScope</summary>
        <video class="w-full mt-2 bg-black" controls preload="none" playsinline src="/vNAAATS-install-howto.mp4">The video could not be played.</video>
        <button class="mt-2 underline text-xs" on:click={() => app.openExternal("https://vnaaats.com/guide")}>Open the full install guide in your browser</button>
      </details>
      {#if !st && problem}
        <div class="mb-2 text-yellow-300 text-sm break-words">The app could not check this computer: {problem}</div>
        <div class="flex gap-2">
          <button class={BTN} on:click={refresh}>Try again</button>
          <button class="px-3 py-1 border border-blue-6" on:click={done}>Continue anyway</button>
        </div>
      {:else if !st}
        <div>Checking...</div>
      {:else}
        <div class={STEP}>
          <b>1. Your EXOCO folder</b>
          {#if st.root}<div class="text-sm opacity-80 break-all">{st.root}</div>{/if}
          <button class="{BTN} mt-1" disabled={busy} on:click={folder}>{st.root ? "Choose a different folder" : "Choose your EXOCO folder"}</button>
        </div>
        {#if st.root}
          <div class={STEP}>
            <b>2. The vNAAATS plugin</b>
            {#if !st.dll_found}
              <div class="text-yellow-300 text-sm">VatsimNAAATS.dll was not found in that folder. Put it in EXOCO/Plugins/vNAAATS, or choose another folder.</div>
            {:else if st.up_to_date === true && listOk}
              <div class="text-green-300 text-sm">✔ Plugin{st.latest_version ? " " + st.latest_version : ""}, airline list and font are up to date</div>
            {:else if st.up_to_date === false || !listOk}
              <div class="text-yellow-300 text-sm">
                {#if st.up_to_date === false}A newer plugin{st.latest_version ? " (" + st.latest_version + ")" : ""} is available.{/if}
                {#if st.airlines_up_to_date === false}{st.airlines_found ? "A newer" : "No"} airline list (ICAO_Airlines.txt){st.airlines_found ? " is available" : " is installed"}.{/if}
                {#if st.font_found === false}The vNAAATS font (vNAAATS.ttf) is not in the plugin folder, so the screen would show the wrong letters.{/if}
                Close EuroScope first, then update.
              </div>
              <button class="{BTN} mt-1" disabled={busy} on:click={update}>Update now</button>
            {:else}
              <div class="text-yellow-300 text-sm">Could not check for the latest plugin{st.error ? ": " + st.error : ""}.</div>
            {/if}
          </div>
          {#if st.sector_up_to_date !== null && st.sector_up_to_date !== undefined}
            <div class={STEP}>
              <b>Sector pack</b>
              {#if st.sector_up_to_date}
                <div class="text-green-300 text-sm">✔ Sector pack{st.sector_name ? " " + st.sector_name : ""} is up to date</div>
              {:else}
                <div class="text-yellow-300 text-sm">A newer sector pack{st.sector_name ? " (" + st.sector_name + ")" : ""} is available. Close EuroScope first, then update.</div>
                <button class="{BTN} mt-1" disabled={busy} on:click={update}>Update now</button>
              {/if}
            </div>
          {/if}
          {#if st.dll_found}
            <div class={STEP}>
              <b>3. Your key</b>
              {#if st.cfg_has_key}
                <div class="text-green-300 text-sm">✔ A key is saved in vnaaats.cfg</div>
              {:else}
                <div class="text-yellow-300 text-sm">No key in vnaaats.cfg yet. Get your API key at vnaaats.com/get-key and paste it here.</div>
                <form class="flex gap-2 mt-1" on:submit|preventDefault={saveKey}>
                  <input class="flex-1 px-2 py-1 text-black" placeholder="1234567.abc..." bind:value={key} autocomplete="off" />
                  <button class={BTN} disabled={busy || !key}>Save</button>
                </form>
              {/if}
            </div>
          {/if}
        {/if}
        <div class={STEP}>
          <b>{st.root ? "4" : "2"}. EuroScope</b>
          {#if st.euroscope}<div class="text-sm opacity-80 break-all">{st.euroscope}</div>{/if}
          <div class="flex gap-2 mt-1">
            <button class="px-3 py-1 border border-blue-6" disabled={busy} on:click={() => pick("euroscope")}>{st.euroscope ? "Change" : "Find EuroScope.exe"}</button>
          </div>
          <label class="flex items-center gap-2 mt-2 text-sm"><input type="checkbox" bind:checked={autoES} on:change={() => app.autoSet("euroscope", autoES)} /> Open EuroScope for me when I say yes on the start screen</label>
        </div>
        <div class={STEP}>
          <b>{st.root ? "5" : "3"}. VACS (voice calls between controllers, optional)</b>
          {#if st.vacs}<div class="text-sm opacity-80 break-all">{st.vacs}</div>{/if}
          <div class="flex gap-2 mt-1">
            <button class="px-3 py-1 border border-blue-6" disabled={busy} on:click={() => pick("vacs")}>{st.vacs ? "Change" : "Find the VACS program"}</button>
            {#if !st.vacs}<button class="px-3 py-1 underline" disabled={busy} on:click={getVacs}>Get VACS</button>{/if}
          </div>
          {#if !st.vacs}<div class="text-xs opacity-70 mt-1 break-all">Or open this address yourself: {VACS_URL}</div>{/if}
          <label class="flex items-center gap-2 mt-2 text-sm"><input type="checkbox" bind:checked={autoVacs} on:change={() => app.autoSet("vacs", autoVacs)} /> Open VACS for me when I say yes on the start screen</label>
          <div class="mt-2 p-2 border border-blue-6 text-sm">
            <b>Incoming call alert in the FDD (optional)</b>
            {#if $vacs.enabled && $vacs.link === "up"}
              <div class="text-green-300 mt-1">&#10004; Connected to VACS. You will see incoming VACS calls on the headphones button at the top of the display.</div>
            {:else}
              <div class="mt-1 opacity-90">The FDD can flash a headphones button and show a pop-up when someone calls you in VACS (needs VACS 2.1.0 or newer). To switch it on:</div>
              <ol class="list-decimal ml-5 mt-1 space-y-0.5 opacity-90">
                <li>Open <b>VACS</b>, go to the <b>Advanced Settings</b> page and tick <b>Enable remote control</b>. It is off by default.</li>
                <li>For safety, change the <b>Listen addr.</b> box under it to <b>127.0.0.1:9600</b>, so only this computer can connect (the default 0.0.0.0:9600 lets any device on your network in, with no password).</li>
                <li>Come back here and press <b>Check again</b>.</li>
              </ol>
              <button class="mt-2 px-3 py-1 border border-blue-6" on:click={() => vc.configure(true, vc.getPort())}>Check again</button>
              <span class="ml-2 text-xs opacity-70">{$vacs.link === "connecting" ? "Trying..." : "Not connected yet. You can skip this and set it up later from the headphones button."}</span>
            {/if}
          </div>
        </div>
        <div class={STEP}>
          <b>{st.root ? "6" : "4"}. Audio client (voice with pilots)</b>
          <div class="flex flex-wrap gap-x-4 gap-y-1 mt-1 text-sm">
            <label><input type="radio" name="audioc" checked={audio === "afv"} on:change={() => chooseAudio("afv")} /> Audio for VATSIM</label>
            <label><input type="radio" name="audioc" checked={audio === "trackaudio"} on:change={() => chooseAudio("trackaudio")} /> TrackAudio</label>
            <label><input type="radio" name="audioc" checked={audio === "none"} on:change={() => chooseAudio("none")} /> Neither</label>
          </div>
          {#if audio !== "none"}
            {#if st[audio]}<div class="text-sm opacity-80 break-all mt-1">{st[audio]}</div>{/if}
            <div class="flex gap-2 mt-1">
              <button class="px-3 py-1 border border-blue-6" disabled={busy} on:click={() => pick(audio)}>{st[audio] ? "Change" : "Find " + AUDIO_NAME[audio]}</button>
              {#if !st[audio]}<button class="px-3 py-1 underline" disabled={busy} on:click={getAudio}>Get {AUDIO_NAME[audio]}</button>{/if}
            </div>
            {#if !st[audio]}<div class="text-xs opacity-70 mt-1 break-all">Or open this address yourself: {AUDIO_URL[audio]}</div>{/if}
            <label class="flex items-center gap-2 mt-2 text-sm"><input type="checkbox" bind:checked={autoAudio} on:change={() => app.autoSet(audio === "none" ? "afv" : audio, autoAudio)} /> Open {AUDIO_NAME[audio]} for me when I say yes on the start screen</label>
          {/if}
        </div>
        {#if msg}<div class="mb-2 text-sm text-yellow-300">{msg}</div>{/if}
        <button class="w-full py-2 font-bold {ready ? 'bg-blue-10 text-black' : 'border border-blue-6'}" on:click={done}>{ready ? "Continue" : "Continue anyway"}</button>
      {/if}
    </div>
  </div>
{/if}
