<script lang="ts">
  import * as dataHandler from "../../typescript/fdd/data";
  import * as utils from "../../typescript/fdd/utils";
  import Titlebar from "../../props/fdd/titlebar.svelte";
  import TextInput from "../../props/fdd/text_input.svelte";
  import type { Aircraft, NatTrakData } from "../../typescript/fdd/objects";

  // Externals
  let asel: Aircraft;
  $: asel = dataHandler.asel;
  let natTrak: NatTrakData;
  $: natTrakData = {callsign: "",status: "--",  nat: "--", fix: "--", level: -1,
  mach: -1, estimating_time: "--", clearance_issued: "--", extra_info: "--"};
  
  // Component updater (50ms)
  setInterval(() => {
    asel = dataHandler.asel;
    if (asel != null && dataHandler.asel.Callsign.length > 0)
      natTrakData = dataHandler.natTrakData.get(dataHandler.asel.Callsign.toUpperCase()) ?? {callsign: "",status: "--",  nat: "--", fix: "--", level: -1,
  mach: -1, estimating_time: "--", clearance_issued: "--", extra_info: "--"};
  }, 50);
</script>

<div class="flex flex-col w-full h-8/27 border-2 border-l-blue-0 border-t-blue-0 border-r-blue-10 border-b-blue-10">
  <Titlebar label="natTrak Information - {asel == null ? 'No selection' : dataHandler.asel.Callsign}"/>
  <div class="px-2 pt-3 pb-3 text-xl font-bitmap tracking-widest font-black">
    Clearance Data
  </div>
  <div class="flex flex-row justify-start">
    <TextInput identifier="status" label="Status" isDisabled={true} value={natTrakData.callsign != "" ? natTrakData.status : "NO DATA"} width="w-6/7" widthWhole="w-1/4" 
    backColour={natTrakData.callsign != "" ? (natTrakData.status == "PENDING" ? "bg-yellow-300" : "bg-green-400") : "bg-red-600"} inputForeColour="text-black"></TextInput>
    <TextInput identifier="issuedAt" label="Clearance Issued" isDisabled={true} value={natTrakData.callsign != "" ? (natTrakData.clearance_issued ?? "NOT ISSUED") : "--"} width="w-5/7" widthWhole="w-1/4" ></TextInput>
    <TextInput identifier="nat" label="Track" isDisabled={true} value="{natTrakData.callsign != "" ? natTrakData.nat : "--"}" width="w-5/7" widthWhole="w-1/4"></TextInput>
  </div>
  <div class="flex flex-row justify-start pt-3">
    
    <TextInput identifier="level" label="Level" isDisabled={true} value={natTrakData.callsign != "" ? natTrakData.level.toString() : "--"} width="w-6/7" widthWhole="w-1/4"></TextInput>
    <TextInput identifier="mach" label="Mach" isDisabled={true} value={natTrakData.callsign != "" ? "M0" + natTrakData.mach.toString() : "--"} width="w-5/7" widthWhole="w-1/4"></TextInput>
    <TextInput identifier="fix" label="Fix" isDisabled={true} value="{natTrakData.callsign != "" ? utils.formatWaypoint(natTrakData.fix) : "--"}" width="w-5/7" widthWhole="w-1/4"></TextInput>
    <TextInput identifier="est" label="Estimation"isDisabled={true} value="{natTrakData.callsign != "" ? natTrakData.estimating_time : "--"}" width="w-6/7" widthWhole="w-1/4"></TextInput>
  </div>
  <div class="justify-start pt-4 pr-6">
    <TextInput identifier="info" label="Extra Information" isDisabled={true} value="{natTrakData.callsign != "" ? (natTrakData.extra_info ?? "NONE") : "--"}" width="w-full"></TextInput>
  </div>
</div>
