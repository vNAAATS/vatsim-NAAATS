<script lang="ts">
  import { fade, fly } from "svelte/transition";
  import { flip } from "svelte/animate";
  import { menuStack, closeMenu, closeAllMenus } from "../../typescript/fdd/acmenu";
  import AcMenuCard from "./ac_menu_card.svelte";
</script>

<!-- Up to 3 aircraft menus: the newest is on top, so a controller can work on several aircraft at once -->
{#if $menuStack.length > 0}
  <div transition:fade={{ duration: 180 }} class="fixed top-0 bottom-0 left-0 w-3/5 z-50 overflow-y-auto" style="background: rgba(0,0,0,0.55);" on:click|self={closeAllMenus}>
    <!-- centred over the strips only (left 3/5), so the flight plan and natTrak boxes on the right stay visible; when 2 or 3 menus are taller than the window the whole thing scrolls -->
    <div class="min-h-full flex items-center justify-center px-2 py-4" on:click|self={closeAllMenus}>
      <div class="w-full" style="max-width: 52rem;">
        {#each $menuStack as cs (cs)}
          <div animate:flip={{ duration: 200 }} in:fly={{ y: -24, duration: 220 }} out:fade={{ duration: 120 }}>
            <AcMenuCard callsign={cs} on:close={() => closeMenu(cs)} />
          </div>
        {/each}
      </div>
    </div>
  </div>
{/if}
