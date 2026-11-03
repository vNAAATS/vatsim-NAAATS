<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("Oceanic training");
    document.body.style.opacity = "100%";
  });
  // The how-to videos. They are small files served from this site and only load when played.
  const howto = [
    { file: "vNAAATS-howto-1-FDD.mp4", title: "1. How to use the Flight Data Display", text: "Tracking, releasing and transferring tags, automatic hand-off to relief, contact me, supervisor request, and changing a flight level, speed or SELCAL." },
    { file: "vNAAATS-howto-2-CPDLC.mp4", title: "2. How to use CPDLC", text: "Signing in with your Hoppie code, accepting pilots, level messages, clearance requests, UNICOM and the pop-out window." },
    { file: "vNAAATS-howto-3-natTrak.mp4", title: "3. How to use natTrak through the FDD", text: "The pending and cleared lists, Prepare in natTrak, RCL processed and Send reclearance." },
  ];
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-3xl w-full">
    <a href="https://vnaaats.com/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">Oceanic training</h1>
    <div class="opacity-80 mb-6">For trainees working towards their OCA endorsement. Watch the introduction before your first session on the scopes.</div>

    <h2 class="text-2xl font-bold mb-2">Trainee introduction</h2>
    <video class="w-full bg-black mb-2" controls preload="metadata" playsinline src="/training/vNAAATS-trainee-intro.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/training/vNAAATS-trainee-intro.mp4">Download it</a>.</video>
    <p class="text-sm opacity-80 mb-6">About 5 minutes. It uses sample flights and is not a live display. It covers what you need, the oceanic area, vNAAATS and the Flight Data Display, CPDLC, and natTrak.</p>

    <h2 class="text-2xl font-bold mb-2">Before your session</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li>The latest stable vNAAATS and EuroScope pack. Follow the <a class="underline" href="/guide">install guide</a>, or use <a class="underline" href="/windows">FDD for Windows</a>.</li>
      <li>A <b>Hoppie ACARS code</b>, so that you can use CPDLC. It is free from <a class="underline" rel="external" href="https://www.hoppie.nl/acars/">hoppie.nl/acars</a>. Each controller uses their own code, and the server never stores it.</li>
      <li>Your VATSIM login for the <a class="underline" rel="external" href="https://nattrak.vatsim.net/">natTrak website</a>.</li>
      <li>Your vNAAATS key, as described in the install guide.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Callsigns</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li><b>Gander radio</b> for aircraft west of 30W, and <b>Shanwick radio</b> for aircraft east of 30W.</li>
      <li><b>NAT_FSS</b> acts as both. You control as NAT_FSS for training and for your OCA endorsement, so you say "Gander radio" or "Shanwick radio" depending on which side of 30W the aircraft is.</li>
      <li>The other callsigns are only used during events.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">The oceanic area</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li>The area is split at <b>30W</b>: Gander to the west, Shanwick to the east.</li>
      <li>The northern boundary is the solid blue line. North of it is the Reykjavik oceanic area, controlled by Icelandic controllers.</li>
      <li>To the south are the New York oceanic area (south west) and the Santa Maria oceanic area (south east).</li>
      <li>West is Canada (Moncton Centre). North east is the United Kingdom (Scottish). East is Ireland (Shannon). South east is France and Spain.</li>
      <li>There are transition areas for Canada and Ireland. Ireland is shown as a light green vertical line. For Canada, hand the aircraft off to Moncton about <b>30 nm before the oceanic exit point</b>.</li>
      <li>The current NAT tracks are drawn from the live track message, and move when the tracks change. The four supersonic routes are fixed.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">How the pieces fit together</h2>
    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li><b>EuroScope plugin:</b> manages your flight tags.</li>
      <li><b>Flight Data Display (FDD):</b> a browser page or Windows app. Track, release and transfer aircraft, change flight level, mach or SELCAL, send contact me and supervisor requests.</li>
      <li><b>CPDLC</b> is built into the FDD, using your own Hoppie code. Pilots log on to you and can also request their oceanic clearance.</li>
      <li><b>natTrak</b> is built into the FDD: the pending and cleared lists show what needs your action, and <i>Open natTrak</i> takes you to the website.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Working a clearance request</h2>
    <ol class="list-decimal ml-6 space-y-1 mb-3">
      <li>Log in to natTrak with your VATSIM details and open the <b>pending messages</b>.</li>
      <li>Work through the aircraft <b>in time order</b>, clicking <b>View</b> on each one.</li>
      <li>At the bottom right, natTrak shows either <i>no active conflicts detected within 10 minutes</i> or <i>conflict detected</i>. It checks for loss of separation against other aircraft on the same track, by flight level and speed.</li>
      <li>If there is no conflict, click <b>Move to processed list</b>, then review the next one.</li>
      <li>For a reclearance, choose the same NAT track, give the reason in the free text, and transmit the reclearance. The FDD updates automatically.</li>
    </ol>
    <p class="mb-6 p-3 border-2 border-blue-10 bg-blue-3"><b>In good time.</b> Pilots should file about 20 minutes before entering, but some do not. Watch your inbound list by the time each aircraft will enter the oceanic area, and check whether a request has come in. If it has not, prompt the pilot by CPDLC to request their oceanic clearance.</p>

    <h2 class="text-2xl font-bold mb-2">How-to videos</h2>
    <p class="opacity-80 mb-4">Short walkthroughs of the FDD. They use sample flights and are not a live display.</p>
    {#each howto as v}
      <div class="mb-6">
        <div class="font-bold mb-1">{v.title}</div>
        <video class="w-full bg-black mb-1" controls preload="none" playsinline src={"/training/" + v.file}>Your browser cannot play this video. <a class="underline" rel="external" href={"/training/" + v.file}>Download it</a>.</video>
        <div class="text-sm opacity-80">{v.text}</div>
      </div>
    {/each}

    <h2 class="text-2xl font-bold mb-2">More</h2>
    <ul class="list-disc ml-6 space-y-1 mb-10">
      <li><a class="underline" href="/manual">User manual</a> for the full detail.</li>
      <li><a class="underline" href="/guide">Install guide</a> and <a class="underline" href="/download">downloads</a>.</li>
    </ul>
  </div>
</div>
