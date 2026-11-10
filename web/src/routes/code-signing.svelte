<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("Code signing policy");
    document.body.style.opacity = "100%";
  });
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-2xl w-full">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">vNAAATS - code signing policy</h1>
    <div class="opacity-80 mb-6">Last updated 31 October 2026</div>

    <p class="p-4 mb-6 border-2 border-blue-10 bg-blue-3"><b>Status:</b> the FDD for Windows installer is being set up for free code signing. Until that is approved, the installer is not signed and Windows may show a "Windows protected your PC" notice (choose "More info" then "Run anyway"). This page describes how signing will work.</p>

    <h2 class="text-2xl font-bold mb-2">Attribution</h2>
    <p class="mb-6 opacity-90">Free code signing provided by <a class="underline" href="https://signpath.io/">SignPath.io</a>, certificate by <a class="underline" href="https://signpath.org/">SignPath Foundation</a>.</p>

    <h2 class="text-2xl font-bold mb-2">What is signed</h2>
    <p class="mb-6 opacity-90">Only the Windows installer and program of FDD for Windows (the vNAAATS app), built from the public source code at <a class="underline" href="https://github.com/garythms/vatsim-NAAATS-v2">github.com/garythms/vatsim-NAAATS-v2</a> (folder <code>launcher</code>) by the automated GitHub build. Nothing else is signed, and nothing from other projects is signed under this policy.</p>

    <h2 class="text-2xl font-bold mb-2">Team and roles</h2>
    <ul class="list-disc pl-6 mb-6 space-y-2 opacity-90">
      <li><b>Author and committer:</b> Gary Thomas (<a class="underline" href="https://github.com/garythms">garythms</a>).</li>
      <li><b>Reviewer</b> of changes that come from anyone else: Gary Thomas.</li>
      <li><b>Approver</b> of each signing request: Gary Thomas.</li>
    </ul>
    <p class="mb-6 opacity-90">Every signing request is approved by hand, and two-factor sign-in is required for the source repository and for SignPath.</p>

    <h2 class="text-2xl font-bold mb-2">Privacy</h2>
    <p class="mb-6 opacity-90">This program will not transfer any information to other networking systems unless specifically requested by the user or the person installing or operating it. What the app and website do with data is described on the <a class="underline" href="/privacy">privacy and data handling</a> page.</p>

    <p class="text-sm opacity-80"><a class="underline" href="/">Back to vNAAATS</a></p>
  </div>
</div>
