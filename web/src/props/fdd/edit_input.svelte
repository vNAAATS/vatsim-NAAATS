<script lang="ts">
  // A text box in the flight plan panel that can be typed in. Saves on Enter or when you click away.
  export let label: string = "";
  export let value: string = "";           // what the server currently has
  export let width: string = "w-16";
  export let editable: boolean = false;
  export let upper: boolean = true;
  export let maxlength: number = 6;
  export let allowEmpty: boolean = false;   // an empty box is a real value (clears it)
  export let onCommit: (text: string) => void = () => {};

  let draft: string = value;
  let focused: boolean = false;
  $: if (!focused) draft = value;          // follow the server unless you are typing

  function commit(): void {
    focused = false;
    const t = draft.trim();
    if ((t !== "" || allowEmpty) && t !== value) onCommit(t);
    else draft = value;
  }
  function key(e: KeyboardEvent): void {
    if (e.key === "Enter") (e.target as HTMLInputElement).blur();
    if (e.key === "Escape") { draft = value; focused = false; (e.target as HTMLInputElement).blur(); }
  }
</script>

<div class="flex flex-col font-bitmap font-thin px-2">
  <label class="font-regular font-bold text-sm">{label}</label>
  <input
    type="text"
    class="text-base appearance-none px-1 h-6 {editable ? 'bg-blue-1 text-yellow-300' : 'bg-blue-2 text-white'}
      border-2 border-l-blue-0 border-t-blue-0 border-r-blue-6 border-b-blue-6 focus:outline-none focus:border-green-300 {width}"
    class:uppercase={upper}
    bind:value={draft}
    {maxlength}
    disabled={!editable}
    title={editable ? "Type a new value and press Enter" : "Sign in and choose a position to edit"}
    on:focus={() => (focused = true)}
    on:blur={commit}
    on:keydown={key} />
</div>
