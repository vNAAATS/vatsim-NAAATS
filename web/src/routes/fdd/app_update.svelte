<script lang="ts">
  import { onMount } from "svelte";
  import * as app from "../../typescript/fdd/app";
  // FDD for Windows: when a newer version of the app exists, ask first. Nothing is downloaded until the controller presses Update.
  let info: { version: string; notes: string } | null = null;
  let busy = false;
  let err = "";
  const ASKED = "vnaaats.updateAsked";
  onMount(async () => {
    if (!app.inApp()) return;
    try { if (sessionStorage.getItem(ASKED) === "1") return; } catch (e) { /* ask anyway */ }
    await app.welcomeDone;
    info = await app.updateCheck();
  });
  function later(): void { try { sessionStorage.setItem(ASKED, "1"); } catch (e) { /* asked again next time */ } info = null; }
  async function go(): Promise<void> {
    busy = true; err = "";
    try { await app.updateInstall(); } catch (e) { err = "The update did not install: " + app.errText(e); busy = false; }
  }
</script>

{#if info}
  <div class="fixed inset-0 z-[80] flex items-center justify-center" style="background: rgba(11,27,48,0.92);">
    <div class="w-[28rem] max-w-full bg-blue-3 border-2 border-blue-10 text-white font-regular p-4">
      <div class="text-xl font-bold mb-2">A new version of FDD for Windows is ready</div>
      <div class="text-sm mb-3">Version {info.version}. Updating takes a moment and the app starts again by itself. Your key, sign-in and settings are kept.</div>
      {#if err}<div class="mb-2 text-sm text-yellow-300">{err}</div>{/if}
      <button class="w-full mb-2 py-2 font-bold bg-blue-10 text-black disabled:opacity-50" disabled={busy} on:click={go}>{busy ? "Updating... the app will restart" : "Update now"}</button>
      <button class="w-full py-1 text-sm border border-blue-6 bg-blue-4" disabled={busy} on:click={later}>Later</button>
    </div>
  </div>
{/if}
