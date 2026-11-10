<script lang="ts">
  export let identifier: string;
  //export let hintText: string = "";
  export let value: string = "";
  export let label: string = "";
  export let labelOnTop: boolean = true;
  //export let labelPositionReverse: boolean = false;
  export let charWidth: number = 10;
  export let width: string = "";
  export let widthWhole: string = "";
  export let isDisabled: boolean = false;
  export let backColour: string = "";
  export let inputType: number = 0;
  export let inputForeColour: string = "text-white";
</script>

<div class="flex flex-{labelOnTop ? 'col' : 'row'} justify-content-end {!labelOnTop ? 'items-center' : ''} {isDisabled ? "select-none" : ""} font-bitmap font-thin px-2 {widthWhole}">
  <label for={identifier} class="font-regular font-bold text-sm {!labelOnTop ? 'px-2' : ''}">{label}</label>
  {#if inputType == 1}
    <input
      type="number"
      class="text-base appearance-none px-1 h-6 bg-blue-2 {inputForeColour} {backColour}
          border-2 border-l-blue-0 border-t-blue-0 border-r-blue-6 border-b-blue-6
          focus:outline-none focus:border-green-300 {width}"
      bind:value={value}
      id={identifier}
      maxlength={charWidth}
      disabled={isDisabled}/>
    {:else if inputType == 2}
    <input
    type="password"
    class="text-base appearance-none px-1 h-6 bg-blue-2 {inputForeColour} {backColour}
        border-2 border-l-blue-0 border-t-blue-0 border-r-blue-6 border-b-blue-6
        focus:outline-none focus:border-green-300 {width}"
    bind:value={value}
    id={identifier}
    maxlength={charWidth}
    disabled={isDisabled}/>
    {:else}
    <input
    type="text"
    class="text-base appearance-none px-1 h-6 bg-blue-2 {inputForeColour} {backColour}
        border-2 border-l-blue-0 border-t-blue-0 border-r-blue-6 border-b-blue-6
        focus:outline-none focus:border-green-300 {width}"
    bind:value={value}
    id={identifier}
    maxlength={charWidth}
    disabled={isDisabled}/>
    {/if}
</div>
