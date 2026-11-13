/*
 *  Handoff prompter. For aircraft YOU are tracking, the server says how far each is from its next boundary and who is online to take it
 *  (Aircraft.Handoff). At 30 nm a card pops up with a chime and the transfer controls; with nobody online it waits until the aircraft is
 *  at the boundary and then says it is leaving oceanic airspace (UNICOM). Dismissing a card keeps it away for that aircraft.
 */
import { writable, get } from "svelte/store";
import * as dataHandler from "./data";
import * as acmenu from "./acmenu";
import * as cp from "./cpdlc";

export const NEAR_NM = 30;       // pop up with the controllers online
export const LEAVE_NM = 3;       // nobody online: pop up at the boundary
const CLEAR_NM = 45;             // an aircraft this far from the boundary again is forgotten (so a later approach can alert again)

export type HandoffCard = { callsign: string; kind: "near" | "leaving"; nm: number; fix: string; name: string; cpdlc: boolean; online: { callsign: string; code: string; frequency: string }[]; error?: string };
export const handoffCards = writable<HandoffCard[]>([]);
export const handoffSoundOn = writable<boolean>((() => { try { return localStorage.getItem("vnaaats.handoffSound") !== "off"; } catch (e) { return true; } })());
export function setHandoffSound(on: boolean): void {
  handoffSoundOn.set(on);
  try { localStorage.setItem("vnaaats.handoffSound", on ? "on" : "off"); } catch (e) { /* not saved */ }
}

/*
 *  Auto handoff (switch on the toolbar, off by default, remembered on this computer). For aircraft you track that are on CPDLC:
 *   - at the chosen distance from the boundary, with a position online to take it: the transfer is sent for you (the plugin also sends the pilot
 *     "CONTACT <station> ON <frequency>");
 *   - at the boundary with nobody online: the UNICOM message is sent, and the tag is released when the pilot answers (as the UNICOM button does).
 *  Each action is tried ONCE per aircraft. If it fails, the card comes up with the error so you can do it by hand. It only runs while this
 *  display is open and signed in to your position.
 */
export type AutoSetting = { on: boolean; nm: number };
function loadAuto(): AutoSetting {
  try { const v = JSON.parse(localStorage.getItem("vnaaats.autoHandoff") ?? ""); return { on: !!v.on, nm: [10, 20, 30, 40, 50].includes(v.nm) ? v.nm : NEAR_NM }; } catch (e) { return { on: false, nm: NEAR_NM }; }
}
export const handoffAuto = writable<AutoSetting>(loadAuto());
export function setHandoffAuto(on: boolean, nm: number = get(handoffAuto).nm): void {
  handoffAuto.set({ on, nm });
  try { localStorage.setItem("vnaaats.autoHandoff", JSON.stringify({ on, nm })); } catch (e) { /* not saved */ }
}
const AUTOKEY = "vnaaats.autoHandoffTried";
function triedBefore(key: string): boolean {       // shared by every tab of this browser, so two open displays do not both act
  try {
    const now = Date.now();
    const o: Record<string, number> = JSON.parse(localStorage.getItem(AUTOKEY) ?? "{}");
    Object.keys(o).forEach((k) => { if (now - o[k] > 3 * 3600 * 1000) delete o[k]; });
    if (o[key]) return true;
    o[key] = now;
    localStorage.setItem(AUTOKEY, JSON.stringify(o));
    return false;
  } catch (e) { return autoMem.has(key) ? true : (autoMem.add(key), false); }
}
const autoMem = new Set<string>();
const autoFailed = new Map<string, string>();     // callsign -> why the automatic action failed (its card then shows)
export const autoLog = writable<string>("");      // last automatic action, shown beside the switch

// Aircraft you tracked that the controller has since released or transferred by hand: auto handoff leaves them alone from then on.
const handledByHand = new Set<string>();
const wasMine = new Set<string>();
function findAc(cs: string): any {
  let found: any = null;
  dataHandler.networkAircraft.forEach((group) => group.forEach((ac: any) => { if (ac.Callsign === cs) found = ac; }));
  return found;
}
/** True while the tag is still yours: checked again just before an automatic action, in case you released or transferred it a moment ago. */
function stillMine(cs: string, me: string): boolean {
  const ac = findAc(cs);
  return !handledByHand.has(cs) && !!ac && String(ac.TrackedBy ?? "").toUpperCase() === me;
}

async function autoTransfer(cs: string, to: { callsign: string; frequency: string }, me: string): Promise<void> {
  if (!stillMine(cs, me)) return;
  const id = await acmenu.queueCommand({ type: "transfer", callsign: cs, target: to.callsign, freq: to.frequency });
  if (typeof id === "string") { autoFailed.set(cs, id); autoLog.set("Auto handoff of " + cs + " failed: " + id); return; }
  for (let i = 0; i < 10; i++) {
    await new Promise((r) => setTimeout(r, 1000));
    const s = await acmenu.commandState(id);
    if (s.state === "ok") { autoLog.set("Auto handoff: " + cs + " transferred to " + to.callsign); return; }
    if (s.state === "failed") { autoFailed.set(cs, s.msg); autoLog.set("Auto handoff of " + cs + " failed: " + s.msg); return; }
  }
  autoFailed.set(cs, "EuroScope did not answer. Hand off in EuroScope instead.");
  autoLog.set("Auto handoff of " + cs + ": EuroScope did not answer");
}
async function autoUnicom(cs: string, me: string): Promise<void> {
  if (!stillMine(cs, me)) return;
  const e = await cp.sendUnicom(cs);
  if (e) { autoFailed.set(cs, e); autoLog.set("Auto UNICOM for " + cs + " failed: " + e); } else autoLog.set("Auto handoff: " + cs + " sent to UNICOM (tag is released when the pilot answers)");
}

const done = new Set<string>();      // callsign|kind already shown (or dismissed)
let actx: AudioContext | null = null;
/** Two rising notes, different from the VACS ring and the CPDLC sound. */
export function chime(): void {
  if (!dataHandler.soundOn || !get(handoffSoundOn)) return;
  try {
    actx = actx ?? new (window.AudioContext || (window as any).webkitAudioContext)();
    if (actx.state === "suspended") void actx.resume();
    if (actx.state !== "running") return;
    const t0 = actx.currentTime + 0.02;
    [[660, 0], [880, 0.22]].forEach(([f, d]) => {
      const o = actx!.createOscillator(), g = actx!.createGain();
      o.type = "triangle"; o.frequency.value = f; o.connect(g); g.connect(actx!.destination);
      g.gain.setValueAtTime(0, t0 + d); g.gain.linearRampToValueAtTime(0.25, t0 + d + 0.02); g.gain.exponentialRampToValueAtTime(0.001, t0 + d + 0.5);
      o.start(t0 + d); o.stop(t0 + d + 0.52);
    });
  } catch (e) { /* no audio */ }
}

try { window.addEventListener("pointerdown", () => { try { actx = actx ?? new (window.AudioContext || (window as any).webkitAudioContext)(); void actx.resume(); } catch (e) { /* no audio */ } }, { once: true }); } catch (e) { /* no window */ }

export function dismiss(callsign: string): void {
  handoffCards.update((l) => l.filter((c) => c.callsign !== callsign));
}

/** Call every couple of seconds with your position callsign (e.g. CZQO_A_CTR). */
export function update(position: string): void {
  const me = (position ?? "").toUpperCase();
  const cards: HandoffCard[] = [];
  const seen = new Set<string>();
  if (me && me !== "OBSERVER") {
    dataHandler.networkAircraft.forEach((group) => group.forEach((ac: any) => {
      const h = ac.Handoff;
      const mine = String(ac.TrackedBy ?? "").toUpperCase() === me;
      if (mine) { if (!handledByHand.has(ac.Callsign)) wasMine.add(ac.Callsign); }
      else if (wasMine.has(ac.Callsign)) { handledByHand.add(ac.Callsign); }   // it was yours and now is not: you released or transferred it
      if (!h || !mine || handledByHand.has(ac.Callsign)) return;
      seen.add(ac.Callsign);
      if (h.nm > CLEAR_NM) { done.delete(ac.Callsign + "|near"); done.delete(ac.Callsign + "|leaving"); return; }
      const auto = get(handoffAuto);
      const nearAt = auto.on ? Math.max(auto.nm, NEAR_NM) : NEAR_NM;       // the card still comes at 30 nm or earlier if auto is set further out
      const kind = !h.unicom && h.nm <= nearAt ? "near" : h.unicom && h.nm <= LEAVE_NM ? "leaving" : null;
      if (!kind) return;
      const key = ac.Callsign + "|" + kind;
      if (auto.on && ac.Cpdlc && !autoFailed.has(ac.Callsign)) {
        if (kind === "near" && h.nm <= auto.nm && h.online?.length) { if (!triedBefore("T|" + ac.Callsign)) void autoTransfer(ac.Callsign, h.online[0], me); return; }
        if (kind === "leaving") { if (!triedBefore("U|" + ac.Callsign)) void autoUnicom(ac.Callsign, me); return; }
        if (kind === "near") return;                                       // waiting for the chosen distance: no card yet
      }
      cards.push({ callsign: ac.Callsign, kind, nm: h.nm, fix: h.fix, name: h.name, cpdlc: !!ac.Cpdlc, online: h.online ?? [], error: autoFailed.get(ac.Callsign) });
      if (!done.has(key)) { done.add(key); chime(); }
    }));
  }
  const dismissed = new Set<string>(get(dismissedStore));
  const shown = cards.filter((c) => !dismissed.has(c.callsign + "|" + c.kind));
  handoffCards.set(shown);
  [...dismissed].forEach((k) => { if (!seen.has(k.split("|")[0])) dismissedStore.update((s) => s.filter((x) => x !== k)); });
}
const dismissedStore = writable<string[]>([]);
export function dismissCard(c: HandoffCard): void {
  dismissedStore.update((s) => [...s, c.callsign + "|" + c.kind]);
  dismiss(c.callsign);
}
