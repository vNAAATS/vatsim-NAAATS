<script lang="ts">
  import { onMount } from "svelte";
  // Old address kept so plugins that were already installed still reach the download page.
  onMount(() => { document.body.style.opacity = "100%"; window.location.replace("/download"); });
</script>

<div class="p-6 text-white font-regular">
  Moved: <a class="underline" href="/download">vnaaats.com/download</a>
</div>
