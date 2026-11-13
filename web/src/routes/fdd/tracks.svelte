<script lang="ts">
  import { CronJob } from "cron";
  import type { NatTrack } from "../../typescript/fdd/objects";
  import * as dataHandler from "../../typescript/fdd/data";
  import Titlebar from "../../props/fdd/titlebar.svelte";
  import TrackStrip from "../../props/fdd/track_strip.svelte";

  // Get tracks and sort
  let tracksRaw: string[] = Array.from(dataHandler.currentNatTracks.keys()).sort();
  let eastbound: NatTrack[];
  let westbound: NatTrack[];
  let supersonic: NatTrack[];
  let tango: NatTrack[];
  let tangoGroups: { label: string; tracks: NatTrack[] }[] = [];
  $: supersonic = [];
  $: tango = [];
  $: eastbound = [];
  $: westbound = [];

  // Validity as one short text, e.g. 1130-1900z (several, separated by "/", if the tracks differ)
  const hhmm = (sec: number): string => { const d = new Date(sec * 1000); return String(d.getUTCHours()).padStart(2, "0") + String(d.getUTCMinutes()).padStart(2, "0"); };
  function validity(tracks: NatTrack[]): string {
    const set: string[] = [];
    for (const t of tracks) {
      if (isNaN(t.ValidFrom) || isNaN(t.ValidTo)) continue;
      const v: string = hhmm(t.ValidFrom) + "-" + hhmm(t.ValidTo) + "z";
      if (!set.includes(v)) set.push(v);
    }
    return set.join(" / ");
  }

  // DOM elements to update
  $: tmi = dataHandler.currentTMI;

  // DOM updater
  let acJob = new CronJob(
    "*/1 * * * * *",
    function () {
      // Set TMI
      tmi = dataHandler.currentTMI;

      // Reset the arrays
      eastbound = [];
      westbound = [];
      supersonic = [];
      tango = [];
      let tracksRaw = Array.from(dataHandler.currentNatTracks.keys()).sort();

      for (let i: number = 0; i < tracksRaw.length; i++) {
        //console.log();
        if (dataHandler.currentNatTracks.get(tracksRaw[i]).Tango) {
          tango.push(dataHandler.currentNatTracks.get(tracksRaw[i]));
        } else if (dataHandler.currentNatTracks.get(tracksRaw[i]).Supersonic) {
          supersonic.push(dataHandler.currentNatTracks.get(tracksRaw[i]));
        } else if (dataHandler.currentNatTracks.get(tracksRaw[i]).Direction == 1) {
          eastbound.push(dataHandler.currentNatTracks.get(tracksRaw[i]));
        } else {
          westbound.push(dataHandler.currentNatTracks.get(tracksRaw[i]));
        }
      }
      tangoGroups = [
        { label: "Northbound", tracks: tango.filter((t) => t.TangoDir === "north") },
        { label: "Southbound", tracks: tango.filter((t) => t.TangoDir === "south") },
        { label: "Bi-directional", tracks: tango.filter((t) => t.TangoDir !== "north" && t.TangoDir !== "south") },
      ].filter((g) => g.tracks.length > 0);
    },
    null,
    true
  );
</script>

<div class="flex flex-col w-full h-10/27 border-2 border-t-blue-0 border-l-blue-0 font-bitmap text-xl font-thin">
  <Titlebar label="Tracks - TMI: {tmi}" />
  <div class="overflow-y-scroll h-full">
    <div class="flex" />
    {#if westbound.length > 0}
    <div class="flex flex-row justify-center w-full bg-grey-500 bg-opacity-70 select-none hover:cursor-default">
      Westbound {validity(westbound)}
    </div>
    {/if}
    {#each westbound as track}
      <TrackStrip
        id={track.Identifier}
        route={track?.Route}
        flightLevels={track.FlightLevels}
        direction={track.Direction}
      />
    {/each}
    {#if eastbound.length > 0}
    <div class="flex flex-row justify-center w-full bg-grey-500 bg-opacity-70 select-none hover:cursor-default">
      Eastbound {validity(eastbound)}
    </div>
    {/if}
    {#each eastbound as track}
      <TrackStrip
        id={track.Identifier}
        route={track.Route}
        flightLevels={track.FlightLevels}
        direction={track.Direction}
      />
    {/each}
    {#if supersonic.length > 0}
    <div class="flex flex-row justify-center w-full bg-grey-500 bg-opacity-70 select-none hover:cursor-default">
      Supersonic (Concorde)
    </div>
    {/if}
    {#each supersonic as track}
      <TrackStrip
        id={track.Identifier}
        route={track.Route}
        flightLevels={track.FlightLevels}
        direction={track.Direction}
        blockLevel={track.BlockLevel}
        use={track.Use}
      />
    {/each}
    {#each tangoGroups as grp}
      <div class="flex flex-row justify-center w-full bg-grey-500 bg-opacity-70 select-none hover:cursor-default">
        Shanwick Tango routes - {grp.label}
      </div>
      {#each grp.tracks as track}
        <TrackStrip
          id={track.Identifier}
          route={track.Route}
          flightLevels={track.FlightLevels}
          direction={track.Direction}
          use={track.Use}
          tango={true}
        />
      {/each}
    {/each}
  </div>
</div>
