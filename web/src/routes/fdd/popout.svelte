<script lang="ts">
  import { onMount } from "svelte";
  import CpdlcPanel from "./cpdlc_panel.svelte";
  import AudioChip from "./audio_chip.svelte";
  import * as cp from "../../typescript/fdd/cpdlc";
  import * as sess from "../../typescript/fdd/session";
  import * as dataHandler from "../../typescript/fdd/data";
  import * as site from "../../typescript/site";

  let which: string = "cpdlc";
  onMount(() => {
    which = new URLSearchParams(location.search).get("w") === "audio" ? "audio" : "cpdlc";
    site.setTitle(which === "audio" ? "Who spoke last" : "CPDLC");
    document.body.style.opacity = "100%";
    sess.refreshSession();
    setInterval(sess.refreshSession, 30000);
    dataHandler.runDataFetcher();
    cp.startCpdlcPolling();
    cp.cpdlcOpen.set(true);
  });
</script>

<div class="h-screen w-screen bg-blue-2">
  {#if which === "audio"}
    <AudioChip standalone={true} />
  {:else}
    <CpdlcPanel popout={true} />
  {/if}
</div>
