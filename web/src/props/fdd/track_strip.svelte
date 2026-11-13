<script lang="ts">
  import type { Waypoint } from "../../typescript/fdd/objects";

  export let id: string;
  export let route: Waypoint[];
  export let flightLevels: number[];
  export let direction: number;
  export let blockLevel: string = ""; // Concorde tracks: e.g. FL470-600
  export let use: string = "";
  export let tango: boolean = false; // Shanwick Tango route: green strips
</script>

<!-- One track: line 1 = name and fixes, line 2 = flight levels. The validity time is shown once on the Westbound / Eastbound heading. -->
<div class="flex flex-col text-black select-none hover:cursor-default">
  <div class="flex flex-row px-3 py-1 {tango ? 'bg-green-200' : direction ? 'bg-blue-10' : 'bg-yellow-100'}">
    <div class="flex flex-row flex-shrink-0 font-bold" style="width: 2.5em;">{id}</div>
    <div class="flex flex-row flex-1 min-w-0 whitespace-nowrap">
      {#each route as wp}
        <div class="pr-2.5">{wp.name}</div>
      {/each}
    </div>
  </div>
  <div class="flex flex-row px-3 py-1 {tango ? 'bg-green-300' : direction ? 'bg-blue-9' : 'bg-yellow-300'} border-b-2 border-black border-opacity-30">
    <div class="flex flex-row flex-shrink-0" style="width: 2.5em;" />
    <div class="flex flex-row flex-1 min-w-0 whitespace-nowrap">
      {#if blockLevel}
        <div class="pr-2.5">Block {blockLevel}{use ? " - " + use : ""}</div>
      {:else if use}
        <div class="pr-2.5">{use}{flightLevels.length ? " -" : ""}</div>
      {/if}
      {#each flightLevels as level}
        <div class="pr-2.5">{level / 100}</div>
      {/each}
    </div>
  </div>
</div>
