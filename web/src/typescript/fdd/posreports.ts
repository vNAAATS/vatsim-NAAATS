/*
 *  Concorde position reports: open/closed state of the pop-out table.
 */
import { writable } from "svelte/store";

export const posOpen = writable<boolean>(false);

export function nowHHMM(): string {
  const d = new Date();
  return String(d.getUTCHours()).padStart(2, "0") + String(d.getUTCMinutes()).padStart(2, "0");
}
