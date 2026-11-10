<script lang="ts">
  import { onMount } from "svelte";
  import * as app from "../../typescript/fdd/app";

  // "Welcome to vNAAATS": a short opening animation with a little tune, played once each time the app is opened.
  let show: boolean = false;
  let out: boolean = false;
  const { launchNotes } = app;
  let audio: HTMLAudioElement | null = null;
  const KEY = "vnaaats.welcomeSeen";

  // The sound is a recorded file (jet flyby, "Gander Radio, SELCAL ...", SELCAL tones, chime), about 7 seconds.
  function tune(): void {
    try { audio = new Audio("/welcome.mp3"); audio.volume = 0.9; void audio.play().catch(() => { /* no sound is fine */ }); } catch (e) { /* no sound is fine */ }
  }

  function finish(): void {
    if (out) return;
    out = true;
    try { if (audio) { audio.pause(); } } catch (e) { /* nothing playing */ }
    setTimeout(() => { show = false; app.finishWelcome(); }, 600);
  }
  onMount(() => {
    let seen = false;
    try { seen = sessionStorage.getItem(KEY) === "1"; sessionStorage.setItem(KEY, "1"); } catch (e) { /* play it */ }
    if (!app.inApp() || seen) { app.finishWelcome(); return; }
    show = true;
    tune();
    setTimeout(finish, 7200);
  });
</script>

{#if $launchNotes.length}
  <div class="fixed bottom-4 left-1/2 -translate-x-1/2 z-[70] max-w-xl px-4 py-3 bg-blue-3 border-2 border-yellow-300 text-white text-sm font-regular shadow-5xl" on:click={() => launchNotes.set([])}>
    {#each $launchNotes as n}<div>{n}</div>{/each}
    <div class="mt-1 text-xs opacity-70">Click to close.</div>
  </div>
{/if}

{#if show}
  <!-- svelte-ignore a11y-click-events-have-key-events -->
  <div class="wl-root fixed inset-0 z-[80] flex flex-col items-center justify-center select-none cursor-pointer" class:wl-out={out} on:click={finish}>
    <div class="wl-title">
      <div class="wl-word wl-a">vNAAATS</div>
      <div class="wl-word wl-b" aria-hidden="true">vNAAATS</div>
      <div class="wl-word wl-c" aria-hidden="true">vNAAATS</div>
    </div>
    <div class="wl-lane">
      <div class="wl-line"></div>
      <svg class="wl-plane" viewBox="0 0 24 24" width="30" height="30"><path fill="currentColor" d="M21 16v-2l-8-5V3.5a1.5 1.5 0 0 0-3 0V9l-8 5v2l8-2.5V19l-2 1.5V22l3.5-1 3.5 1v-1.5L13 19v-5.5z" transform="rotate(90 12 12)" /></svg>
    </div>
    <div class="wl-hello">Welcome to vNAAATS</div>
    <div class="wl-sub">The Virtual North Atlantic Advanced Air Traffic System</div>
  </div>
{/if}

<style>
  .wl-root { background: radial-gradient(ellipse at center, #12294a 0%, #0b1b30 60%, #050d19 100%); color: #fff; transition: opacity 0.6s ease; font-family: sans-serif; }
  .wl-root.wl-out { opacity: 0; }
  .wl-title { position: relative; font-size: min(14vw, 7.5rem); font-weight: 800; letter-spacing: 0.04em; line-height: 1; }
  .wl-word { animation: wl-reveal 0.8s 0s both ease-out; }
  .wl-b, .wl-c { position: absolute; inset: 0; mix-blend-mode: screen; opacity: 0; }
  .wl-b { color: #4fd1ff; animation: wl-glitch-b 0.7s 0.05s both steps(1); }
  .wl-c { color: #ff4f7a; animation: wl-glitch-c 0.7s 0.05s both steps(1); }
  .wl-lane { position: relative; width: min(70vw, 34rem); height: 34px; margin-top: 0.6rem; }
  .wl-line { position: absolute; left: 0; top: 16px; height: 2px; background: #fff; width: 0; animation: wl-draw 1.0s 0.7s forwards ease-in-out; }
  .wl-plane { position: absolute; left: 0; top: 0; color: #fff; opacity: 0; transform: translateX(0); animation: wl-fly 1.0s 0.7s forwards ease-in-out; }
  .wl-hello { margin-top: 1.4rem; font-size: 1.9rem; letter-spacing: 0.12em; opacity: 0; animation: wl-fade 0.8s 1.6s forwards ease-out; }
  .wl-sub { margin-top: 0.5rem; font-size: 0.95rem; opacity: 0; letter-spacing: 0.06em; animation: wl-fade2 0.8s 2.0s forwards ease-out; }
  @keyframes wl-reveal { 0% { clip-path: inset(0 100% 0 0); opacity: 0; } 100% { clip-path: inset(0 0 0 0); opacity: 1; } }
  @keyframes wl-glitch-b { 0% { opacity: 1; transform: translate(-6px, 2px); clip-path: inset(10% 0 60% 0); } 25% { opacity: 1; transform: translate(5px, -3px); clip-path: inset(55% 0 15% 0); } 50% { opacity: 1; transform: translate(-3px, 0); clip-path: inset(30% 0 40% 0); } 75% { opacity: 1; transform: translate(4px, 2px); clip-path: inset(70% 0 5% 0); } 100% { opacity: 0; } }
  @keyframes wl-glitch-c { 0% { opacity: 1; transform: translate(6px, -2px); clip-path: inset(60% 0 10% 0); } 25% { opacity: 1; transform: translate(-5px, 3px); clip-path: inset(5% 0 70% 0); } 50% { opacity: 1; transform: translate(3px, 0); clip-path: inset(45% 0 30% 0); } 75% { opacity: 1; transform: translate(-4px, -2px); clip-path: inset(20% 0 55% 0); } 100% { opacity: 0; } }
  @keyframes wl-draw { to { width: 100%; } }
  @keyframes wl-fly { 0% { opacity: 0; transform: translateX(0); } 10% { opacity: 1; } 90% { opacity: 1; } 100% { opacity: 0; transform: translateX(calc(min(70vw, 34rem) - 30px)); } }
  @keyframes wl-fade { from { opacity: 0; transform: translateY(8px); } to { opacity: 1; transform: none; } }
  @keyframes wl-fade2 { from { opacity: 0; } to { opacity: 0.8; } }
</style>
