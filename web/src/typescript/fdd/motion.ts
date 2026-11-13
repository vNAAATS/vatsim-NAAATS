// Animation lengths for the FDD. Everything is short and quiet; with "reduce motion" switched on in Windows (or the browser) they are skipped.
export const reduced: boolean = (() => { try { return window.matchMedia("(prefers-reduced-motion: reduce)").matches; } catch (e) { return false; } })();
export const ms = (n: number): number => (reduced ? 0 : n);
