<script lang="ts">
  import { onMount } from "svelte";
  import * as sess from "../../typescript/fdd/session";
  import * as app from "../../typescript/fdd/app";

  const { session } = sess;
  let name: string = "";
  let password: string = "";
  let custom: string = "";
  let error: string = "";
  let busy: boolean = false;

  // Remember me: the app keeps the name and password in the Windows Credential Manager; Chrome keeps them in its own password manager.
  let remember: boolean = false;
  let savedName: string = "";                       // app: a sign-in is saved for this name
  const browserCreds: boolean = typeof window !== "undefined" && "PasswordCredential" in window && !app.inApp();
  async function loadSaved(): Promise<void> {
    if (!app.inApp()) return;
    try { savedName = (await app.credGet())?.name ?? ""; } catch (e) { savedName = ""; }
  }
  async function useSaved(): Promise<void> {
    busy = true; error = "";
    try {
      if (app.inApp()) {
        const c = await app.credGet();
        if (!c) { savedName = ""; busy = false; return; }
        error = await sess.loginPassword(c.name, c.password);
        if (error) error += " (the saved password may have changed: sign in again with Remember me ticked)";
      } else {
        const c: any = await (navigator.credentials as any).get({ password: true, mediation: "optional" });
        if (c?.id && c?.password) error = await sess.loginPassword(c.id, c.password);
      }
    } catch (e) { error = "could not use the saved sign-in"; }
    busy = false;
  }
  async function forget(): Promise<void> {
    try { await app.credForget(); } catch (e) { /* nothing saved */ }
    savedName = "";
  }

  // Key sign-in: with a saved key the display waits for a live VATSIM connection on an oceanic position, then opens by itself
  let keyInput: string = "";
  let savedKeyText: string = sess.getKey();
  let waiting: boolean = false;
  let keyError: string = "";
  let launched: boolean = false;
  let keyTimer: any = null;
  async function tryKey(): Promise<void> {
    if (!savedKeyText || $session.loggedIn) return;
    waiting = true;
    if (!launched) { launched = true; void app.autoLaunch(); }       // start EuroScope, VACS and the audio client so you can connect
    const r = await sess.loginKey(savedKeyText);
    if (r.state === "in") { waiting = false; keyError = ""; return; }
    if (r.state === "error") { keyError = r.msg; if (/not valid/.test(r.msg)) { waiting = false; stopKey(); } return; }
    keyError = "";
  }
  function startKey(): void { stopKey(); void tryKey(); keyTimer = setInterval(tryKey, 5000); }
  function stopKey(): void { if (keyTimer) { clearInterval(keyTimer); keyTimer = null; } }
  function saveKey(): void {
    const k = keyInput.trim();
    if (!k) return;
    sess.rememberKey(k); savedKeyText = k; keyInput = ""; keyError = ""; startKey();
  }
  function forgetKey(): void { sess.rememberKey(""); savedKeyText = ""; waiting = false; keyError = ""; stopKey(); }
  $: if ($session.loggedIn) { waiting = false; stopKey(); }

  onMount(() => {
    loadSaved();
    sess.refreshSession().then(() => { if (savedKeyText && !$session.loggedIn) startKey(); });
    // Ask again now and then, so a session that has run out brings the sign-in screen back
    const t = setInterval(sess.refreshSession, 30000);
    return () => { clearInterval(t); stopKey(); };
  });

  // The flight data is only sent to a signed-in controller with a position, so until then the whole page is covered
  $: show = $session.editing && (!$session.loggedIn || $session.position === "");

  async function doPassword(): Promise<void> {
    busy = true; error = "";
    const typedName = name, typedPassword = password;
    error = await sess.loginPassword(name, password);
    if (!error && remember) {
      try {
        if (app.inApp()) { await app.credSave(typedName, typedPassword); savedName = typedName.trim(); }
        else if (browserCreds) await (navigator.credentials as any).store(new (window as any).PasswordCredential({ id: typedName, password: typedPassword, name: typedName }));
      } catch (e) { /* signed in anyway; just not saved */ }
    }
    password = "";
    busy = false;
  }
  // A position is chosen straight away. natTrak is signed in to later, from the natTrak box on the display, once the controller is online on VATSIM.
  async function pick(p: string): Promise<void> {
    error = await sess.choosePosition(p);
    if (!error && p !== "") void app.autoLaunch();
  }
</script>

{#if show}
  <div class="fixed inset-0 z-50 flex items-center justify-center" style="background: #0b1b30;">
    <div class="w-96 max-w-full bg-blue-3 border-2 border-l-blue-10 border-t-blue-10 border-r-blue-0 border-b-blue-0 text-white font-regular p-4">
      <div class="text-xl font-bold mb-1">vNAAATS Flight Data Display</div>
      {#if !$session.loggedIn}
          {#if savedKeyText}
            <div class="mb-2 text-lg font-bold text-yellow-300">Awaiting an active VATSIM connection</div>
            <div class="mb-3 text-sm opacity-90">Connect to VATSIM in EuroScope on your oceanic position (for example NAT_FSS, CZQO_CTR or EGGX_CTR). The display opens by itself, on that position, as soon as you are connected.</div>
            {#if keyError}<div class="mb-3 text-sm text-yellow-300">{keyError}</div>{/if}
            <div class="text-sm mb-1 opacity-80">{waiting ? "Checking every few seconds..." : "Stopped."}</div>
            <button class="mt-2 mr-4 text-sm underline opacity-80" on:click={forgetKey}>Use a different key</button>
          {:else}
            <div class="mb-3 text-sm opacity-90">Enter your vNAAATS key. You do not have one yet? Get it at <a class="underline" href="/get-key">vnaaats.com/get-key</a> (Gander Oceanic controllers can also use ganderoceanic.ca/my/vnaaats).</div>
            <form class="flex gap-2 mb-1" on:submit|preventDefault={saveKey}>
              <input class="flex-1 px-2 py-1 text-black" placeholder="Your key" bind:value={keyInput} autocomplete="off" />
              <button class="px-3 font-bold bg-blue-10 text-black" disabled={!keyInput.trim()}>Continue</button>
            </form>
            {#if keyError}<div class="mb-2 text-sm text-yellow-300">{keyError}</div>{/if}
          {/if}
      {:else}
        <div class="mb-3 text-sm opacity-80">Hello {$session.name}. Which position are you working?</div>
        <div class="grid grid-cols-2 gap-2 mb-3">
          {#each $session.positions as p}
            <button class="py-2 font-bold bg-blue-10 text-black" on:click={() => pick(p)}>{p}</button>
          {/each}
        </div>
        <form class="flex gap-2" on:submit|preventDefault={() => custom && pick(custom)}>
          <input class="flex-1 px-2 py-1 text-black uppercase" placeholder="Other position e.g. CZQO_E_CTR" bind:value={custom} maxlength="16" />
          <button class="px-3 font-bold bg-blue-10 text-black" disabled={!custom}>Use</button>
        </form>
        <button class="mt-3 px-3 py-1 text-sm font-bold border-2 border-blue-10 bg-blue-4 hover:bg-blue-10 hover:text-black" on:click={sess.logout}>Sign out</button>
      {/if}
      {#if error}<div class="mt-2 text-sm text-yellow-300">{error}</div>{/if}
    </div>
  </div>
{/if}
