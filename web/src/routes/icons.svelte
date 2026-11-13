<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("Icon Key");
    document.body.style.opacity = "100%";
    // the key's own styles, scoped to .ik, added while this page is open
    const el = document.createElement("style");
    el.textContent = css;
    document.head.appendChild(el);
    return () => el.remove();
  });
  const css = ".ik{--bg:#071428;--panel:#0e2340;--ink:#e8f1fb;--muted:#9db3cc;--line:#23406a;--accent:#7cc4ff;--chip:#173258;--ok:#4ade80;--warn:#fb923c;--bad:#f87171;--st-east:#8ec9f5;--st-west:#fbe98f;--st-ink:#000;--f-head:inherit;--f-body:inherit;--f-mono:ui-monospace,Menlo,Consolas,monospace;--flash:#facc15}\n.ik header{margin-bottom:28px}\n.ik h1{font-family:var(--f-head);font-size:clamp(30px,5vw,44px);line-height:1.08;margin:0 0 10px;letter-spacing:-.02em;text-wrap:balance}\n.ik header p{margin:0;color:var(--muted);max-width:62ch}\n.ik nav{display:flex;flex-wrap:wrap;gap:8px;margin:20px 0 0}\n.ik nav a{color:var(--accent);text-decoration:none;border:1px solid var(--line);padding:5px 12px;border-radius:999px;font-size:14px;font-weight:600}\n.ik nav a:hover,.ik nav a:focus-visible{background:var(--chip);outline:2px solid var(--accent);outline-offset:1px}\n.ik section{margin-top:44px}\n.ik h2{font-family:var(--f-head);font-size:26px;margin:0 0 6px;letter-spacing:-.01em}\n.ik .lead{color:var(--muted);margin:0 0 16px;max-width:70ch}\n.ik .sample{background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:14px;margin:0 0 14px;overflow-x:auto}\n.ik .sample .cap{font-size:12px;letter-spacing:.08em;text-transform:uppercase;color:var(--muted);margin:0 0 8px;font-weight:600}\n.ik .key{display:grid;grid-template-columns:minmax(150px,230px) 1fr;border-top:1px solid var(--line)}\n.ik .key>div{padding:10px 8px;border-bottom:1px solid var(--line);min-width:0}\n.ik .key .s{display:flex;align-items:center;gap:8px;flex-wrap:wrap}\n.ik .key .m b{font-weight:700}\n.ik .key .m span{color:var(--muted)}\n\n@media (max-width:560px){.key{grid-template-columns:1fr}.key .s{border-bottom:0;padding-bottom:0}.key .m{padding-top:2px}}.ik /* strip mocks */\n.strip{display:grid;grid-template-columns:150px 82px 44px 50px repeat(3,50px) 44px 28px;align-items:center;height:32px;color:var(--st-ink);\n  font-family:var(--f-mono);font-size:12px;font-weight:500;min-width:560px;gap:0;white-space:nowrap}\n.ik .strip>div{padding:0 4px;overflow:hidden}\n.ik .strip>div:first-child{padding-left:8px;font-weight:700}\n.ik .east{background:var(--st-east)}\n.ik .west{background:var(--st-west)}\n.ik .faded{opacity:.45}\n.ik .pick{outline:2px solid #000;outline-offset:-2px;box-shadow:inset 0 0 0 3px #fff}\n.ik .talk{outline:4px solid #5cff7a;outline-offset:-4px;box-shadow:0 0 10px #5cff7a}\n.ik .flash{animation:fl 1.1s steps(2,jump-none) infinite}\n\n@keyframes fl{0%{filter:none}50%{filter:brightness(.78) saturate(1.3)}}\n@media (prefers-reduced-motion:reduce){.flash,.ho-flash{animation:none}}.ik .sw{display:inline-block;width:56px;height:20px;border-radius:2px;border:1px solid rgba(0,0,0,.35);flex:none}\n.ik .cs{font-family:var(--f-mono);font-weight:700;font-size:14px;background:var(--st-east);color:#000;padding:2px 7px;border-radius:2px;white-space:nowrap}\n.ik .cs.y{background:#fde047}\n.ik .cs.r{background:#dc2626;color:#fff}\n.ik .hs{display:inline-flex;width:22px;height:22px}\n.ik .redstar{color:#dc2626;font-weight:700;font-size:1.5em;line-height:.6}\n.ik .hochip{font-family:var(--f-mono);font-size:11px;font-weight:700;background:#fde047;color:#000;border:1px solid #000;padding:0 4px;line-height:1.3}\n.ik /* top bar mocks */\n.bar{display:flex;flex-wrap:wrap;gap:8px;align-items:center;font-size:13px}\n.ik .tb{display:inline-flex;align-items:center;gap:6px;padding:3px 10px;border:1px solid #4a8ac0;background:#14304f;color:#fff;font-weight:700;font-size:13px;line-height:1.4;white-space:nowrap}\n.ik .tb.y{border-color:#fde047;color:#fde047;background:transparent}\n.ik .tb.ho{border-color:#fde047;background:#fde047;color:#000}\n.ik .ho-flash{animation:fl 1.1s steps(2,jump-none) infinite}\n.ik .tb.red{background:#dc2626;border-color:#fca5a5}\n.ik .tb.blue{border-color:#7cc4ff;color:#7cc4ff}\n.ik .tb.grn{background:#15803d;border-color:#86efac}\n.ik .tb.up{border-color:#4a8ac0}\n.ik .dot{display:inline-block;width:9px;height:9px;border-radius:50%}\n.ik .pill{display:inline-block;font-size:12px;font-weight:700;padding:1px 7px;color:#fff;border:1px solid}\n.ik .p-ok{background:#15803d;border-color:#86efac}\n.ik .p-pend{background:#14304f;border-color:#7cc4ff}\n.ik .p-warn{background:#fde047;color:#000;border-color:#fde047}\n.ik .p-bad{background:#b91c1c;border-color:#fca5a5}\n.ik .p-out{background:transparent;color:#7cc4ff;border-color:#7cc4ff}\n.ik .tag{font-family:var(--f-mono);background:#0a1a2e;color:#e8f1fb;padding:8px 12px;border-radius:3px;font-size:13px;line-height:1.35;display:inline-block;border:1px solid #223d5e}\n.ik .ac{display:inline-block;vertical-align:middle}\n.ik footer{margin-top:48px;color:var(--muted);font-size:14px;border-top:1px solid var(--line);padding-top:16px;max-width:70ch}\n.ik .key .s:has(.tb,.pill){background:#0e2340;padding-left:12px}\n.ik .bar:has(.tb){background:#0e2340;padding:10px;border-radius:4px}\n.ik .sub{}\n";
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-4xl w-full ik">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <div class="my-3 p-4 border-2 border-blue-10 bg-blue-3 flex flex-wrap items-center justify-between gap-3">
      <div><b>Icon key (PDF)</b><div class="text-sm opacity-80">The same key as a file to download or print.</div></div>
      <div><a href="/vNAAATS-icon-key.pdf" rel="external" class="inline-block px-5 py-2 mr-2 font-bold bg-blue-10 text-black">Open (PDF)</a><a href="/vNAAATS-icon-key.pdf" rel="external" download class="inline-block px-5 py-2 border-2 border-blue-10">Download</a></div>
    </div>

<header>
  <h1 class="text-4xl font-bold mt-3 mb-1">vNAAATS icon key</h1>
  <p>What each symbol, colour and flashing state means on the Flight Data Display and in EuroScope. Hover over most of them in the FDD to see the detail.</p>
  <nav aria-label="Sections">
    <a href="#strips">Flight strips</a><a href="#callsign">Callsign marks</a><a href="#radio">Headset icon</a><a href="#topbar">Top bar</a><a href="#cpdlc">CPDLC and requests</a><a href="#es">EuroScope</a>
  </nav>
</header>

<section id="strips">
  <h2>Flight strip colours and outlines</h2>
  <p class="lead">One strip per flight, grouped under its track. Columns left to right: callsign, departure/arrival, level, Mach, estimates over each fix, aircraft type, headset or clearance icon.</p>
  <div class="sample"><p class="cap">Example strips</p>
    <div class="strip east"><div>BAW112</div><div>EGLL/KJFK</div><div>350</div><div>M082</div><div>1837</div><div>1912</div><div>2004</div><div>B77W</div><div></div></div>
    <div class="strip west"><div>ACA871</div><div>CYYZ/EGLL</div><div>370</div><div>M080</div><div>1841</div><div>1923</div><div>2015</div><div>A333</div><div></div></div>
  </div>
  <div class="key">
    <div class="s"><span class="sw" style="background:var(--st-east)"></span></div><div class="m"><b>Blue strip</b> <span>Eastbound flight.</span></div>
    <div class="s"><span class="sw" style="background:var(--st-west)"></span></div><div class="m"><b>Yellow strip</b> <span>Westbound flight.</span></div>
    <div class="s"><span class="sw" style="background:var(--st-east);opacity:.45"></span></div><div class="m"><b>Faded strip</b> <span>The flight has been handed off or is past its exit point. It stays on the list for 15 minutes after being untracked.</span></div>
    <div class="s"><span class="sw pick" style="background:var(--st-east)"></span></div><div class="m"><b>Black and white outline</b> <span>This is the aircraft you picked in EuroScope. The strip scrolls into view even if a filter would hide it.</span></div>
    <div class="s"><span class="sw talk" style="background:var(--st-east)"></span></div><div class="m"><b>Green glowing box</b> <span>The aircraft is transmitting on your TrackAudio frequency. The box fades out when it stops.</span></div>
    <div class="s"><span class="sw flash" style="background:var(--st-east)"></span></div><div class="m"><b>Flashing strip</b> <span>A natTrak request is waiting, a CPDLC message or logon needs a look, or a Concorde is overdue a position report. Click the strip to deal with it. Flashing can be switched off with <i>Visual</i> at the bottom.</span></div>
    <div class="s"><span class="sw" style="background:#fde047"></span></div><div class="m"><b>Yellow callsign cell</b> <span>You sent <i>Contact me</i> to this pilot.</span></div>
    <div class="s"><span class="sw" style="background:#dc2626"></span></div><div class="m"><b>Red callsign cell</b> <span>Twenty minutes after Contact me, nobody is tracking the flight and it is still in oceanic airspace.</span></div>
  </div>
</section>

<section id="callsign">
  <h2>Marks beside the callsign</h2>
  <div class="sample"><p class="cap">What you may see</p>
    <div class="bar"><span class="cs">BAW112</span><span class="cs">[BAW112]</span><span class="cs">BAW112*</span><span class="cs">BAW112<span class="redstar">*</span></span><span class="cs">BAW112 <span style="color:#dc2626">&#9888;</span></span><span class="cs">BAW112 &#9998;</span><span class="cs">BAW112 <span class="hochip">HO 28nm &rarr; CZQO</span></span></div>
  </div>
  <div class="key">
    <div class="s"><span class="cs">[BAW112]</span></div><div class="m"><b>Square brackets</b> <span>The pilot is logged on to CPDLC with vNAAATS.</span></div>
    <div class="s"><span class="cs">BAW112*</span></div><div class="m"><b>Black star</b> <span>The aircraft is not AGCS equipped, worked out from the PBN and remarks in the flight plan.</span></div>
    <div class="s"><span class="cs">BAW112<span class="redstar">*</span></span></div><div class="m"><b>Red star</b> <span>You (or Auto RCL) sent the pilot the natTrak reminder. It clears when the pilot requests clearance or an oceanic controller tracks the aircraft. It shows even if the aircraft is still 90 minutes out.</span></div>
    <div class="s"><span class="cs"><span style="color:#dc2626">&#9888;</span></span></div><div class="m"><b>Red warning triangle</b> <span>Two aircraft enter on the same track and level within 10 minutes, or the level is not one of the track's levels. Hover for the reason.</span></div>
    <div class="s"><span class="cs">&#9998;</span></div><div class="m"><b>Pencil</b> <span>Level, Mach, track or SELCAL changed in the last 10 minutes. Hover to see who and when.</span></div>
    <div class="s"><span class="hochip">HO 28nm &rarr; CZQO</span></div><div class="m"><b>Yellow HO chip</b> <span>The aircraft is within the handoff distance of a boundary. It shows the distance and who is next, or UNICOM if nobody is online.</span></div>
  </div>
</section>

<section id="radio">
  <h2>Headset icon and clearance icon</h2>
  <p class="lead">The last column of every strip. The headset comes from the radio frequency the pilot has tuned on VATSIM. Hover to see the callsign and frequency.</p>
  <div class="key">
    <div class="s"><svg class="hs" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor" stroke-width="1.2" style="color:#15803d"><path d="M12 2a9 9 0 0 0-9 9v7a3 3 0 0 0 3 3h1v-8H5v-2a7 7 0 0 1 14 0v2h-2v8h2v1a2 2 0 0 1-2 2h-3v2h3a4 4 0 0 0 4-4V11a9 9 0 0 0-9-9z"/></svg></div><div class="m"><b>Bold green headset</b> <span>Talking to an oceanic controller.</span></div>
    <div class="s"><svg class="hs" viewBox="0 0 24 24" fill="currentColor" style="color:#16a34a;opacity:.5"><path d="M12 2a9 9 0 0 0-9 9v7a3 3 0 0 0 3 3h1v-8H5v-2a7 7 0 0 1 14 0v2h-2v8h2v1a2 2 0 0 1-2 2h-3v2h3a4 4 0 0 0 4-4V11a9 9 0 0 0-9-9z"/></svg></div><div class="m"><b>Faded green headset</b> <span>Talking to a domestic controller.</span></div>
    <div class="s"><svg class="hs" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor" stroke-width=".6" style="color:#c2410c"><path d="M12 2a9 9 0 0 0-9 9v7a3 3 0 0 0 3 3h1v-8H5v-2a7 7 0 0 1 14 0v2h-2v8h2v1a2 2 0 0 1-2 2h-3v2h3a4 4 0 0 0 4-4V11a9 9 0 0 0-9-9z"/></svg></div><div class="m"><b>Orange headset</b> <span>On UNICOM 122.800.</span></div>
    <div class="s"><svg class="hs" viewBox="0 0 24 24" fill="currentColor" stroke="currentColor" style="color:#dc2626"><path d="M12 2a9 9 0 0 0-9 9v7a3 3 0 0 0 3 3h1v-8H5v-2a7 7 0 0 1 14 0v2h-2v8h2v1a2 2 0 0 1-2 2h-3v2h3a4 4 0 0 0 4-4V11a9 9 0 0 0-9-9z"/><path d="M3 3 21 21" stroke-width="2.2"/></svg></div><div class="m"><b>Red crossed-out headset</b> <span>Online but not on any controller or UNICOM.</span></div>
    <div class="s"><svg class="hs" viewBox="0 0 24 24" fill="currentColor" style="color:var(--ink)"><path d="M19 3h-4.2A3 3 0 0 0 12 1a3 3 0 0 0-2.8 2H5a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V5a2 2 0 0 0-2-2zm-7 0a1 1 0 1 1 0 2 1 1 0 0 1 0-2zm-2 14-4-4 1.4-1.4L10 14.2l6.6-6.6L18 9z"/></svg></div><div class="m"><b>Clipboard with a tick</b> <span>The RCL was processed on natTrak. Hover for the time and who processed it.</span></div>
  </div>
</section>

<section id="topbar">
  <h2>Top bar</h2>
  <div class="sample"><p class="cap">Buttons and indicators</p>
    <div class="bar">
      <span class="tb"><span class="dot" style="background:#86efac"></span>CPDLC [3]</span>
      <span class="tb ho ho-flash">&#8644; Handoff: BAW112 +1</span>
      <span class="tb y">&#9888; 2 warnings</span>
      <span class="tb y">&#9201; 1 overdue</span>
      <span class="tb"><span class="dot" style="background:#4ade80"></span>Auto handoff: ON</span>
    </div>
  </div>
  <div class="key">
    <div class="s"><span class="tb"><span class="dot" style="background:#86efac"></span>CPDLC [3]</span></div><div class="m"><b>CPDLC button, green dot</b> <span>CPDLC is online. The number is how many pilots are connected. Click to open the CPDLC panel.</span></div>
    <div class="s"><span class="tb ho ho-flash">&#8644; Handoff: BAW112 +1</span></div><div class="m"><b>Flashing yellow Handoff button</b> <span>Aircraft are near a boundary and need a handoff. The number after the plus is how many others are waiting. Click to read the cards and act. It is a reminder, not an automatic action.</span></div>
    <div class="s"><span class="tb"><span class="dot" style="background:#4ade80"></span>Auto handoff: ON</span><span class="tb"><span class="dot" style="background:#f87171"></span>OFF</span></div><div class="m"><b>Auto handoff dot</b> <span>Green: the transfer is sent for you at the chosen distance (10 to 50 nm), once per aircraft. Red: you do the handoffs yourself.</span></div>
    <div class="s"><span class="tb y">&#9888; 2 warnings</span></div><div class="m"><b>Yellow warning button</b> <span>Click to jump to the next flight with a warning. Click again for the next one.</span></div>
    <div class="s"><span class="tb y">&#9201; 1 overdue</span></div><div class="m"><b>Overdue count</b> <span>Concorde flights more than 3 minutes past their next fix with no position report. The strip flashes too.</span></div>
  </div>

  <div class="sub">
  <h2 style="margin-top:32px;font-size:20px">VACS button</h2>
  <div class="key">
    <div class="s"><span class="tb">VACS</span></div><div class="m"><b>Plain button</b> <span>Call alerts are switched off. Click to set up.</span></div>
    <div class="s"><span class="tb up"><svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="#7fe0a2" stroke-width="2"><path d="M4 15v-3a8 8 0 0 1 16 0v3"/><rect x="3" y="14" width="4" height="6" rx="1.5"/><rect x="17" y="14" width="4" height="6" rx="1.5"/></svg>VACS</span></div><div class="m"><b>Green headphones</b> <span>Connected to VACS on your position. If it says <i>no position</i>, VACS is not yet connected to a position.</span></div>
    <div class="s"><span class="tb red">Call: CZQX_FSS</span></div><div class="m"><b>Flashing red</b> <span>Incoming call. A pop-up offers Accept, Decline or Dismiss.</span></div>
    <div class="s"><span class="tb blue">In call: EGGX 0:42</span></div><div class="m"><b>Blue with a timer</b> <span>You are on a call.</span></div>
    <div class="s"><span class="tb">VACS: Busy</span></div><div class="m"><b>Busy</b> <span>Incoming calls are declined automatically. Click to turn it off.</span></div>
    <div class="s"><span class="tb">VACS offline</span></div><div class="m"><b>Offline</b> <span>VACS is not running or Remote Control is off. Click for help.</span></div>
    <div class="s"><span class="pill p-bad">2</span></div><div class="m"><b>Red number badge</b> <span>Missed calls in the last hour.</span></div>
  </div>

  </div>
  <div class="sub">
  <h2 style="margin-top:32px;font-size:20px">TrackAudio button</h2>
  <div class="key">
    <div class="s"><span class="tb up"><span class="dot" style="background:#9ca3af"></span>No link</span></div><div class="m"><b>Grey dot</b> <span>Not connected to TrackAudio.</span></div>
    <div class="s"><span class="tb up"><span class="dot" style="background:#86efac"></span>Waiting</span></div><div class="m"><b>Green dot</b> <span>Connected to TrackAudio.</span></div>
    <div class="s"><span class="tb grn">BAW112</span></div><div class="m"><b>Green button</b> <span>A pilot spoke last on your frequencies. The name shows who.</span></div>
    <div class="s"><span class="tb" style="background:#fde047;color:#000;border-color:#fde047">You</span></div><div class="m"><b>Yellow button</b> <span>You are transmitting.</span></div>
  </div>
  </div>
</section>

<section id="cpdlc">
  <h2>CPDLC and request panels</h2>
  <p class="lead">The small coloured labels beside each request or message.</p>
  <div class="key">
    <div class="s"><span class="pill p-ok">natTrak: cleared FL350 NAT E</span></div><div class="m"><b>Green</b> <span>natTrak has issued a clearance for this request.</span></div>
    <div class="s"><span class="pill p-pend">natTrak: pending</span></div><div class="m"><b>Blue</b> <span>The request is on natTrak, waiting for a controller.</span></div>
    <div class="s"><span class="pill p-warn">not in natTrak</span></div><div class="m"><b>Yellow</b> <span>The pilot asked by CPDLC but has no request on natTrak yet. A controller has to enter it there.</span></div>
    <div class="s"><span class="pill p-bad">not in natTrak 6m</span></div><div class="m"><b>Red</b> <span>Still not on natTrak after 5 minutes.</span></div>
    <div class="s"><span class="pill p-warn" style="color:#000">NO REPLY</span></div><div class="m"><b>Yellow NO REPLY</b> <span>The pilot has not answered a message after 3 minutes. Use <i>Contact by voice</i>.</span></div>
    <div class="s"><b style="font-size:13px">NEW</b></div><div class="m"><b>NEW</b> <span>A message from the pilot that you have not looked at yet.</span></div>
    <div class="s"><span class="pill p-out">&#10003; Sent</span></div><div class="m"><b>Tick with text</b> <span>The message went to the pilot.</span></div>
    <div class="s"><span class="pill" style="color:#86efac;border-color:#86efac;background:transparent">&#10003; Auto accept</span></div><div class="m"><b>Green tick</b> <span>Logon requests are accepted for you.</span></div>
    <div class="s"><span class="pill p-warn" style="color:#000">MAX FL390</span></div><div class="m"><b>MAX FL line</b> <span>The highest level the pilot can accept at the entry point. Without one, the requested level is treated as the maximum.</span></div>
  </div>
</section>

<section id="es">
  <h2>In EuroScope</h2>
  <div class="sample"><p class="cap">Tag, filled and outline aircraft</p>
    <div class="bar" style="gap:18px;align-items:center">
      <span><svg class="ac" width="34" height="34" viewBox="0 0 24 24" fill="#7cc4ff" stroke="#7cc4ff"><path d="M21 16v-2l-8-5V3.5a1.5 1.5 0 0 0-3 0V9l-8 5v2l8-2.5V19l-2 1.5V22l3.5-1 3.5 1v-1.5L13 19v-5.5z"/></svg> Cleared</span>
      <span><svg class="ac" width="34" height="34" viewBox="0 0 24 24" fill="none" stroke="#7cc4ff" stroke-width="1.3"><path d="M21 16v-2l-8-5V3.5a1.5 1.5 0 0 0-3 0V9l-8 5v2l8-2.5V19l-2 1.5V22l3.5-1 3.5 1v-1.5L13 19v-5.5z"/></svg> Not cleared</span>
      <span class="tag">[BAW112]<br>350 M082<br><span style="opacity:.75">scratchpad note</span></span>
      <span class="tag">ACA871*<br>370 M080</span>
      <span class="tag">DLH401<span style="color:#f87171;font-weight:700"> *</span><br>360 M083</span>
    </div>
  </div>
  <div class="key">
    <div class="s"><svg class="ac" width="26" height="26" viewBox="0 0 24 24" fill="#7cc4ff" stroke="#7cc4ff"><path d="M21 16v-2l-8-5V3.5a1.5 1.5 0 0 0-3 0V9l-8 5v2l8-2.5V19l-2 1.5V22l3.5-1 3.5 1v-1.5L13 19v-5.5z"/></svg></div><div class="m"><b>Filled aircraft icon</b> <span>Cleared.</span></div>
    <div class="s"><svg class="ac" width="26" height="26" viewBox="0 0 24 24" fill="none" stroke="#7cc4ff" stroke-width="1.3"><path d="M21 16v-2l-8-5V3.5a1.5 1.5 0 0 0-3 0V9l-8 5v2l8-2.5V19l-2 1.5V22l3.5-1 3.5 1v-1.5L13 19v-5.5z"/></svg></div><div class="m"><b>Outline aircraft icon</b> <span>Not cleared yet. Icons point along the aircraft's heading.</span></div>
    <div class="s"><span class="tag" style="padding:2px 8px">[BAW112]</span></div><div class="m"><b>Callsign in brackets</b> <span>The pilot is on CPDLC.</span></div>
    <div class="s"><span class="tag" style="padding:2px 8px">ACA871*</span></div><div class="m"><b>Star under the callsign</b> <span>Not AGCS equipped.</span></div>
    <div class="s"><span class="tag" style="padding:2px 8px;color:#f87171;font-weight:700">*</span></div><div class="m"><b>Red star</b> <span>The natTrak reminder was sent from the FDD.</span></div>
    <div class="s"><span class="tag" style="padding:2px 8px;opacity:.85">note</span></div><div class="m"><b>Note under the tag</b> <span>The scratchpad. Type it in EuroScope or on the FDD.</span></div>
    <div class="s"><b>Flashing tags</b></div><div class="m"><span>Conflict alert: aircraft are too close. Exactly one level apart does not flash.</span></div>
    <div class="s"><b>CPDLC ON / OFF</b></div><div class="m"><span>Shown in the Datalink box of the flight plan window.</span></div>
  </div>
</section>

<footer>Based on the vNAAATS manual and the Flight Data Display itself. Colours here match the FDD; exact shades on your screen may differ slightly with your display settings. The EuroScope samples are schematic: the real tags and icons use your radar screen's own fonts and colours. Full detail is in the manual at vnaaats.com/manual.</footer>

  </div>
</div>
