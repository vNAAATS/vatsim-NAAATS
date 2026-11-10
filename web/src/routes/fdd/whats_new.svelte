<script lang="ts">
  import { fade, fly } from "svelte/transition";
  import * as sess from "../../typescript/fdd/session";
  import * as app from "../../typescript/fdd/app";
  import * as wn from "../../typescript/fdd/whatsnew";
  import { vacs } from "../../typescript/fdd/vacs";
  const { session } = sess;
  const { setupOpen } = app;
  const { whatsNewOpen } = wn;

  let dismissed: boolean = false;
  // Once per announced release, after sign-in; never over the sign-in, the app setup or a ringing VACS call
  $: auto = !dismissed && $session.loggedIn && wn.unseen() && !$setupOpen && !$vacs.ringing;
  $: shown = $whatsNewOpen || auto;
  $: note = wn.latest();

  function close(): void { wn.markSeen(); dismissed = true; whatsNewOpen.set(false); }
  function act(a: string | undefined): void {
    if (a === "vacs") { close(); if (app.inApp()) app.setupOpen.set(true); else window.open("/guide", "_blank"); }
    if (a === "windows") window.open("/windows", "_blank");
  }
</script>

{#if shown}
  <div transition:fade={{ duration: 180 }} class="fixed inset-0 z-40 flex items-center justify-center bg-black bg-opacity-60">
    <div transition:fly={{ y: 24, duration: 220 }} class="w-full max-w-lg max-h-full overflow-y-auto bg-blue-2 border-2 border-blue-10 text-white font-web" role="dialog" aria-label="What's new">
      <div class="px-3 py-2 bg-blue-3 font-bold text-lg">What's new in {note.id} <span class="text-sm font-normal opacity-70">{note.date}</span></div>
      <ul class="m-3 space-y-3">
        {#each note.items as it}
          <li>
            <div class="font-bold">{it.title}</div>
            <div class="text-sm opacity-90">{it.text}
              {#if it.action === "vacs"}<span class="underline hover:cursor-pointer text-yellow-300" on:click={() => act("vacs")}>{app.inApp() ? "Set it up" : "How to set it up"}</span>{/if}
              {#if it.action === "windows"}<span class="underline hover:cursor-pointer text-yellow-300" on:click={() => act("windows")}>Find out more</span>{/if}
            </div>
          </li>
        {/each}
      </ul>
      <div class="px-3 pb-3 flex items-center gap-3">
        <button class="px-4 py-1 font-bold bg-blue-10 text-black" on:click={close}>Got it</button>
        <a class="underline text-sm opacity-80" href="/manual" target="_blank">User manual</a>
      </div>
    </div>
  </div>
{/if}
