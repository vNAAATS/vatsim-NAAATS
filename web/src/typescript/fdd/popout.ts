// Separate windows: in the Windows app a real window that can go on another screen; in a browser a normal pop-up window.
import { inApp } from "./app";

export type PopKind = "cpdlc" | "audio";
const SIZE: Record<PopKind, [number, number]> = { cpdlc: [780, 760], audio: [420, 560] };

export async function popOut(which: PopKind): Promise<string> {
  const [w, h] = SIZE[which];
  try {
    if (inApp()) {
      await (window as any).__TAURI__.core.invoke("open_popout", { which, width: w, height: h });
      return "";
    }
  } catch (e) { /* the app is older than this feature: use a normal window */ }
  const win = window.open("/fdd/popout?w=" + which, "vnaaats-pop-" + which, "popup=yes,width=" + w + ",height=" + h);
  return win ? "" : "The browser blocked the pop-up window. Allow pop-ups for this site.";
}
