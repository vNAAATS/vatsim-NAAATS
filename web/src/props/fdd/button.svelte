<script lang="ts">
  export let label: string;
  export let height: string = "10";
  export let width: string = "";
  export let font: string = "regular";
  export let bold: string = "true";
  export let textColour: string = "text-white";

  let isBold: boolean = bold === "true" ? true : false;
</script>

<!-- Standard button with hover styling -->
<button
  class="flex items-center justify-center h-{height} {width} px-2 font-{font} {textColour} active:border-r-blue-6 active:border-b-blue-6 
active:border-l-blue-0 active:border-t-blue-0 active:bg-blue-1 border-2 
border-l-blue-6 border-t-blue-6 border-r-blue-0 border-b-blue-0 bg-blue-3 select-none hover:cursor-default"
>
  {#if isBold}
    <strong>{label}</strong>
  {:else}
    {label}
  {/if}
</button>
