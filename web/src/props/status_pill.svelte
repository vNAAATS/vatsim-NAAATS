<script lang="ts">
  import { onMount } from "svelte";

  // A one-line "how is the server" pill for the homepage; it links to the full status page
  let level: "ok" | "warn" | "off" = "off";
  let text: string = "Checking status…";

  async function load(): Promise<void> {
    const ctl = new AbortController();
    const t = setTimeout(() => ctl.abort(), 5000);
    try {
      const r = await fetch(((import.meta as any).env?.VITE_API_BASE ?? "") + "/api/status", { cache: "no-store", signal: ctl.signal });
      if (!r.ok) throw new Error(String(r.status));
      const d = await r.json();
      const now = d.now as number;
      const feedOk = !d.feed.enabled || (!d.feed.error && d.feed.updated && now - d.feed.updated < 120);
      const tracksOk = !!d.tracks.updated && now - d.tracks.updated < 3600;
      const atc = (d.atc as { callsign: string }[]).filter((c) => /^(CZQO|EGGX|NAT)/.test(c.callsign)).length;
      level = feedOk && tracksOk ? "ok" : "warn";
      text = (level === "ok" ? "All systems go" : "Some systems need attention") +
        (d.feed.enabled ? " · " + (atc ? atc + " Gander/Shanwick position" + (atc === 1 ? "" : "s") + " online" : "no Gander/Shanwick ATC online") : "");
    } catch (e) {
      level = "off"; text = "Status unavailable";
    } finally { clearTimeout(t); }
  }

  onMount(() => {
    load();
    const t = setInterval(load, 60000);
    return () => clearInterval(t);
  });
</script>

<a href="/status" class="inline-flex items-center gap-2 mt-3 px-3 py-1 rounded-full border text-sm font-regular text-white bg-black bg-opacity-40 hover:bg-opacity-60 transition duration-300 {level === 'ok' ? 'border-green-400' : level === 'warn' ? 'border-yellow-300' : 'border-blue-6'}" title="Open the full status page">
  <span class="inline-block w-2.5 h-2.5 rounded-full {level === 'ok' ? 'bg-green-400' : level === 'warn' ? 'bg-yellow-300' : 'bg-blue-6'}"></span>{text}
</a>
