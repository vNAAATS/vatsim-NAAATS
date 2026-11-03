<script lang="ts">
  import { onMount, onDestroy } from "svelte";
  import * as site from "../typescript/site";
  // Hidden page for trusted instructors: not linked anywhere and not indexed. Needs the instructor password.
  let passphrase = "";
  let instructor = "";
  let cid = "";
  let name = "";
  let busy = false;
  let error = "";
  let result: { key: string; password: string; expires: number; cid: string; hours: number } | null = null;
  let left = "";
  let timer: any = null;
  let copied = "";
  onMount(() => {
    site.setTitle("Trainee key");
    document.body.style.opacity = "100%";
    const m = document.createElement("meta");
    m.name = "robots";
    m.content = "noindex, nofollow";
    document.head.appendChild(m);
    try { instructor = sessionStorage.getItem("vnaaats.instructor") ?? ""; } catch (e) { /* fine without it */ }
  });
  onDestroy(() => { if (timer) clearInterval(timer); });
  function tick() {
    if (!result) return;
    const s = Math.max(0, Math.floor(result.expires - Date.now() / 1000));
    left = s <= 0 ? "expired" : Math.floor(s / 3600) + "h " + String(Math.floor((s % 3600) / 60)).padStart(2, "0") + "m left";
  }
  async function submit() {
    error = "";
    busy = true;
    try {
      const r = await fetch("/auth/trainee-key", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ passphrase, instructor, cid, name }),
      });
      const j = await r.json();
      if (!r.ok) error = j.error || "Something went wrong";
      else {
        result = j;
        try { sessionStorage.setItem("vnaaats.instructor", instructor); } catch (e) { /* fine */ }
        tick();
        timer = setInterval(tick, 20000);
      }
    } catch (e) {
      error = "Could not reach the server";
    }
    busy = false;
  }
  async function copy(what: string, text: string) {
    try { await navigator.clipboard.writeText(text); copied = what; setTimeout(() => (copied = ""), 1500); } catch (e) { /* select-all still works */ }
  }
  function another() {
    result = null; cid = ""; name = ""; left = "";
    if (timer) { clearInterval(timer); timer = null; }
  }
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-xl w-full">
    <h1 class="text-4xl font-bold mb-2">vNAAATS - trainee key</h1>
    <p class="opacity-80 mb-6">For instructors. Issues a key and FDD password for a student, valid for <b>2 hours</b> from now. Issue it on the day of the live session, shortly before it starts. Give it only to that student.</p>
    {#if !result}
      <label class="block mb-3">Instructor password<input class="block w-full p-2 text-black" type="password" bind:value={passphrase} autocomplete="off" /></label>
      <label class="block mb-3">Your name or CID (so we know who issued it)<input class="block w-full p-2 text-black" bind:value={instructor} autocomplete="off" /></label>
      <label class="block mb-3">Student's VATSIM CID<input class="block w-full p-2 text-black" bind:value={cid} inputmode="numeric" autocomplete="off" /></label>
      <label class="block mb-4">Student's name (optional)<input class="block w-full p-2 text-black" bind:value={name} autocomplete="off" /></label>
      {#if error}<div class="p-3 mb-4 border-2 border-red-500 bg-blue-3">{error}</div>{/if}
      <button class="px-5 py-2 font-bold bg-blue-10 text-black" disabled={busy} on:click={submit}>{busy ? "Please wait..." : "Issue 2-hour key"}</button>
    {:else}
      <div class="p-4 border-2 border-blue-10 bg-blue-3 mb-4">
        <div class="mb-1 opacity-80 text-sm">For CID {result.cid}. Expires {new Date(result.expires * 1000).toUTCString()} ({left})</div>
        <div class="mt-3 mb-3"><div class="opacity-80 text-sm">Plugin key (vnaaats.cfg: key=...)</div><div class="font-mono break-all select-all">{result.key}</div>
          <button class="mt-1 px-3 py-1 text-sm bg-blue-10 text-black font-bold" on:click={() => copy("key", result ? result.key : "")}>{copied === "key" ? "Copied" : "Copy key"}</button></div>
        <div class="mb-1"><div class="opacity-80 text-sm">FDD password (the student signs in with their CID {result.cid})</div><div class="font-mono select-all">{result.password}</div>
          <button class="mt-1 px-3 py-1 text-sm bg-blue-10 text-black font-bold" on:click={() => copy("pw", result ? result.password : "")}>{copied === "pw" ? "Copied" : "Copy password"}</button></div>
      </div>
      <p class="opacity-90 mb-4">Pass these on now: the password is not shown again. After 2 hours they stop working.</p>
      <button class="px-5 py-2 font-bold bg-blue-10 text-black" on:click={another}>Issue for another student</button>
    {/if}
  </div>
</div>
