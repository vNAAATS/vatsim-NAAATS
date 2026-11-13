<script lang="ts">
  export let identifier: string;
  export let label: string;
  export let startChecked: string = "false";
  export let font: string = "regular";
  export let bold: string = "true";
  export let textColour: string = "white";
  export let margin: string = "1";
  export let isDisabled: boolean = false;
  export let title: string = "";
  export let checked: boolean = startChecked === "true";

  let isBold: boolean = bold === "true" ? true : false;
</script>

<div class="flex flex-row items-center px-{parseInt(margin)}" {title}>
  <input
    type="checkbox"
    bind:checked
    class="select-none hover:cursor-default custom-check appearance-none outline-none h-3.5 w-3.5 border-2 
    border-l-blue-6 border-t-blue-6 border-r-blue-0 border-b-blue-0 bg-blue-3 checked:bg-blue-1
    checked:border-r-blue-6 checked:border-b-blue-6  checked:border-l-blue-0 checked:border-t-blue-0 {isDisabled ? "opacity-75" : ""}"
    id={identifier} disabled={isDisabled} />
  {#if checked}
    <label
      for={identifier}
      class="z-100 absolute justify-start select-none hover:cursor-default pointer-events-none"
      >✔</label
    >
  {/if}
  <label
    for={identifier}
    class="text-{textColour} font-{font} {isBold
      ? 'font-bold'
      : ''} {isDisabled ? "opacity-75" : ""} select-none hover:cursor-default px-1.5 pointer-events-none"
    >{label}</label
  >
</div>
