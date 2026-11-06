<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("How To Use - Videos");
    document.body.style.opacity = "100%";
  });
  // Every video in one place. They are small files served from this site and only load when played.
  const groups: { heading: string; note: string; videos: { src: string; title: string; text: string }[] }[] = [
    {
      heading: "Getting started",
      note: "",
      videos: [
        { src: "/training/vNAAATS-howto-5-FDD-for-Windows.mp4", title: "FDD for Windows, the One Fits All Program", text: "What the app does, why it is better than a browser, installing it (including the Windows SmartScreen message), the one-time setup and how updates work." },
        { src: "/vNAAATS-install-howto.mp4", title: "Loading vNAAATS into EuroScope", text: "About a minute and a quarter: opening your profile, loading the sector file and plugin, then adding the vNAAATS display and the ASR file." },
        { src: "/vNAAATS-intro.mp4", title: "How to use vNAAATS", text: "About three minutes, with captions. An overview of the display. The flights shown are demo data." },
      ],
    },
    {
      heading: "Using the Flight Data Display",
      note: "Short walkthroughs using sample flights. They are not a live display.",
      videos: [
        { src: "/training/vNAAATS-howto-1-FDD.mp4", title: "1. How to use the Flight Data Display", text: "Tracking, releasing and transferring tags, automatic hand-off to relief, contact me, supervisor request, and changing a flight level, speed or SELCAL." },
        { src: "/training/vNAAATS-howto-2-CPDLC.mp4", title: "2. How to use CPDLC", text: "Signing in with your Hoppie code, accepting pilots, level messages, clearance requests, UNICOM and the pop-out window." },
        { src: "/training/vNAAATS-howto-3-natTrak.mp4", title: "3. How to use natTrak through the FDD", text: "The pending and cleared lists, Prepare in natTrak, RCL processed and Send reclearance." },
        { src: "/training/vNAAATS-howto-4-VACS-TrackAudio.mp4", title: "4. VACS and TrackAudio in FDD for Windows", text: "What VACS and TrackAudio are, setting up VACS remote control, answering VACS calls, who spoke last, and the transmitting-aircraft highlight." },
      ],
    },
    {
      heading: "Trainees",
      note: "",
      videos: [
        { src: "/training/vNAAATS-trainee-intro.mp4", title: "Trainee introduction", text: "For trainees working towards their OCA endorsement. Watch it before your first session on the scopes." },
      ],
    },
  ];
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-3xl w-full">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">How To Use - Videos</h1>
    <div class="opacity-80 mb-6">Every vNAAATS video in one place. For the written version see the <a class="underline" href="/guide">install guide</a> and the <a class="underline" href="/manual">user manual</a>.</div>

    {#each groups as g}
      <h2 class="text-2xl font-bold mb-2">{g.heading}</h2>
      {#if g.note}<p class="opacity-80 mb-4">{g.note}</p>{/if}
      {#each g.videos as v}
        <div class="mb-6">
          <div class="font-bold mb-1">{v.title}</div>
          <video class="w-full bg-black mb-1" controls preload="none" playsinline src={v.src}>Your browser cannot play this video. <a class="underline" rel="external" href={v.src}>Download it</a>.</video>
          <div class="text-sm opacity-80">{v.text}</div>
        </div>
      {/each}
    {/each}
  </div>
</div>
