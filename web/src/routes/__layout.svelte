<script lang="ts">
  import "../styles/globals.postcss";
  import * as dataHandler from "../typescript/fdd/data";
  import config from "../config.json";
  import buildInfo from "../build-info.json";
  import * as site from "../typescript/site";
  import { onMount } from "svelte";
  import { inApp } from "../typescript/fdd/app";

  // The Homepage link would take FDD for Windows away from the display, so it is only shown in a browser
  const showHome = !inApp();

  // After a new version is published the old file names are gone. A page that was left open then fails with
  // "Failed to fetch dynamically imported module": reload once to pick up the new version.
  onMount(() => {
    const stale = (m: any) => /dynamically imported module|Importing a module script failed|error loading dynamically/i.test(String(m?.message ?? m ?? ""));
    const reload = () => {
      try { if (sessionStorage.getItem("vnaaats.staleReload")) return; sessionStorage.setItem("vnaaats.staleReload", "1"); } catch (e) { /* reload anyway once */ }
      location.reload();
    };
    window.addEventListener("unhandledrejection", (e) => { if (stale(e.reason)) reload(); });
    window.addEventListener("error", (e) => { if (stale(e.error ?? e.message)) reload(); });
    setTimeout(() => { try { sessionStorage.removeItem("vnaaats.staleReload"); } catch (e) { /* ignore */ } }, 15000);
  });

  // Server version (from /health) so bug reports can quote both numbers
  let serverVersion: string = "";
  onMount(async () => {
    try {
      const r = await fetch(((import.meta as any).env?.VITE_API_BASE ?? "") + "/health");
      serverVersion = (await r.json()).version ?? "";
    } catch (e) { /* ignore */ }
  });

  // Externals
  let count: number = 0;
  $: siteSection = site.siteSection;

  // DOM updater
  // Component updater (50ms)
  setInterval(() => {
    count = dataHandler.relevantAircraftCount;
    siteSection = site.siteSection;
  }, 50);
</script>

<div class="flex flex-col h-screen">
  {#if siteSection != 2}
    <div class="overflow-hidden"></div>
  {:else}
    <div class="flex justify-between items-center w-screen h-1/25 bg-blue-0 border-2 border-l-blue-10 border-t-blue-10 border-r-blue-10 border-b-blue-0">
      <div class="flex w-2/5">
        <button class="px-3 font-black hover:text-grey-300 select-none hover:cursor-default" title="Built {buildInfo.built}">
          Version
          {config.version}{serverVersion ? " \u00b7 server " + serverVersion : ""}
        </button>
        <div class="px-1/2 select-none hover:cursor-default">|</div>
        <div class="px-5 select-none hover:cursor-default">
          Aircraft Count: {count}
        </div>
      </div>
      <div class="flex flex-row-reverse px-3 2xl:w-1/5 xl:w-1/4 select-none hover:cursor-default">
        <div class="px-5 select-none hover:cursor-default">
          vNAAATS Flight Data Display
        </div>
      </div>
      <div class="flex flex-row-reverse w-2/5 select-none hover:cursor-default">
        {#if showHome}
          <a href="./">
            <button  class="px-5 font-black hover:text-grey-300 select-none">
              Homepage
            </button>
          </a>
        {/if}
        
        <!--<div class="px-3">Guest User</div>
        <div class="px-1/2 select-none hover:cursor-default">|</div>
        <button class="px-3 font-black hover:text-grey-300 select-none hover:cursor-default">
          Settings
        </button>
        <div class="px-1/2 select-none hover:cursor-default">•</div>
        <button class="px-3 font-black hover:text-grey-300 select-none hover:cursor-default">
          Manual Data Entry
        </button>-->
      </div>
    </div>
  {/if}
  <slot />
</div>
