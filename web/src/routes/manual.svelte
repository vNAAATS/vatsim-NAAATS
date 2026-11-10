<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("User manual");
    document.body.style.opacity = "100%";
  });
  const toc: [string, string][] = [
    ["overview", "1. How it all fits together"],
    ["start", "2. Starting a session"],
    ["layout", "3. The Flight Data Display screen"],
    ["topbar", "4. The top bars and filters"],
    ["strips", "5. Flight strips and their symbols"],
    ["menu", "6. The aircraft menu"],
    ["panel", "7. The flight plan panel"],
    ["nattrak", "8. natTrak requests and alerts"],
    ["cpdlc", "9. CPDLC"],
    ["tracks", "10. Tracks panel"],
    ["conc", "11. Concorde position reports"],
    ["es", "12. What you see in EuroScope"],
    ["cfg", "13. Settings in vnaaats.cfg"],
    ["flow", "14. A typical shift, step by step"],
    ["help", "15. Problems and questions"],
  ];
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-3xl w-full">
    <a href="https://vnaaats.com/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">vNAAATS V2 - user manual</h1>
    <div class="opacity-80 mb-6">Every feature of the Flight Data Display (FDD) and the EuroScope plugin, and how to use it. Not installed yet? Start with the <a class="underline" href="https://vnaaats.com/guide">install guide</a>.</div>

    <div class="p-4 mb-8 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-2">Contents</div>
      <ul class="space-y-1">
        {#each toc as t}<li><a class="underline" href={"#" + t[0]}>{t[1]}</a></li>{/each}
      </ul>
    </div>

    <h2 id="overview" class="text-2xl font-bold mb-2">1. How it all fits together</h2>
    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: how to use vNAAATS</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/vNAAATS-intro.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/vNAAATS-intro.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">About three minutes, with captions, an overview of the display. The flights shown are demo data.</div>
    </div>

    <p class="mb-3 opacity-90">vNAAATS has three parts that share one set of flight data:</p>
    <ul class="list-disc ml-6 space-y-1 mb-3">
      <li><b>The Flight Data Display (FDD)</b> - a web page at <a class="underline" href="https://vnaaats.com/fdd">vnaaats.com/fdd</a>. This is where you work. It lists every oceanic flight as a strip and lets you change flight data, track and transfer aircraft, send messages and run CPDLC.</li>
      <li><b>The EuroScope plugin</b> (VatsimNAAATS.dll) - shows the same aircraft on your radar and in EuroScope's flight plan windows, and carries out what you ask for on the FDD (track, untrack, transfer, SELCAL, contact me).</li>
      <li><b>The vNAAATS server</b> - keeps everything in step, so every controller sees the same level, Mach, track, SELCAL and notes within a few seconds.</li>
    </ul>
    <p class="mb-6 p-3 border-2 border-blue-10 bg-blue-3"><b>The FDD is your main screen.</b> Almost everything can be done from it, and most of what you do in EuroScope can also be done there. Keep EuroScope connected to VATSIM as your oceanic position with the plugin loaded, and use the FDD in a browser window next to it. (EuroScope still does what only it can do: radar, your own tracking and the flight plan windows.)</p>

    <h2 id="start" class="text-2xl font-bold mb-2">2. Starting a session</h2>
    <ol class="list-decimal ml-6 space-y-1 mb-3">
      <li>Start EuroScope, connect to VATSIM as your oceanic position (for example CZQO_CTR) and check the vNAAATS plugin is loaded.</li>
      <li>Open <a class="underline" href="https://vnaaats.com/fdd">vnaaats.com/fdd</a> in your browser.</li>
      <li>Sign in with the login you were given and choose your <b>position</b> (CZQO_CTR, EGGX_CTR and so on). The position is what other controllers see on "last changed" and "tracked by".</li>
    </ol>
    <p class="mb-3 opacity-90"><b>Use it as an app:</b> on Windows, install <b>FDD for Windows</b> from the <a class="underline" href="/download">download page</a>: it opens the display in its own window, looks after your plugin, airline list and font, and opens EuroScope, VACS and your audio client for you. Or, in Chrome or Edge, open the FDD and click the <i>install</i> icon at the right end of the address bar (or the three-dot menu, then <i>Install vNAAATS FDD</i> / <i>Apps</i>). It then opens in its own window with its own icon, which suits a second monitor. It is still the same website, so updates arrive automatically.</p>
    <p class="mb-3 opacity-90"><b>Nothing is shown until you have signed in and chosen a position.</b> This protects the data of pilots and controllers: the page stays covered, and the server sends no flight data, until a position has been chosen. There is no view-only mode. Use <i>change</i> to pick another position (the page is covered again until you do) and <i>sign out</i> when you finish. If your sign-in runs out while you work, the sign-in screen returns.</p>
    <p class="mb-6 opacity-90">Keep EuroScope connected while you work: the plugin asks the server every 2 seconds whether you have asked for something (track, transfer, SELCAL and so on), and does nothing if EuroScope is offline.</p>

    <h2 id="layout" class="text-2xl font-bold mb-2">3. The Flight Data Display screen</h2>
    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: how to use the Flight Data Display</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/training/vNAAATS-howto-1-FDD.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/training/vNAAATS-howto-1-FDD.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">Tracking, releasing and transferring tags, automatic hand-off to relief, contact me, supervisor request, and changing a flight level, speed or SELCAL. Sample flights, not a live display.</div>
    </div>

    <ul class="list-disc ml-6 space-y-1 mb-6">
      <li><b>Left (the big panel):</b> the filter bar, the controls bar, the natTrak PENDING and CLEARED boxes, and below them the flight strips.</li>
      <li><b>Right, top:</b> the <b>Flight Plan</b> panel for the selected aircraft.</li>
      <li><b>Right, middle:</b> natTrak information.</li>
      <li><b>Right, bottom:</b> the <b>Tracks</b> panel (today's NATs).</li>
      <li><b>Pop-ups:</b> the aircraft menu (click a callsign), the CPDLC drawer, and the Concorde position reports table.</li>
    </ul>

    <h2 id="topbar" class="text-2xl font-bold mb-2">4. The top bars and filters</h2>
    <h3 class="text-lg font-bold mt-4 mb-1">Filter bar (top)</h3>
    <ul class="list-disc ml-6 space-y-1 mb-3">
      <li><b>ALL TK / RR</b> - all tracks, random routings. The selected one is yellow. <b>Default</b> resets the filters.</li>
      <li><b>TRKD</b> only the flights you are tracking at your position, <b>EB</b> / <b>WB</b> eastbound / westbound, <b>RR</b> random routings only, <b>TANGO</b> Shanwick Tango routes only. With nothing ticked you see everything. Boxes in different groups narrow each other (TRKD + WB = your westbound flights); within a group either counts (EB + WB = both directions, RR + TANGO = both kinds). Hover over each box for its description.</li>
      <li>The two boxes on the right are the Zulu clock and the TMI (the day number of the NAT track message).</li>
    </ul>
    <h3 class="text-lg font-bold mt-4 mb-1">Header (top row and second row)</h3>
    <ul class="list-disc ml-6 space-y-2 mb-6">
      <li><b>Position menu</b> - click your position to <i>change position</i>, hand over to a relief controller, or <i>sign out</i>.</li>
      <li><b>VACS headphones</b> - flashes red with the caller's name when VACS has an incoming call. See the VACS call alert section below.</li>
      <li><b>Search or add</b> - one box. Type part of a callsign or a fix name to filter the list (Enter jumps to the first match). To bring a flight that is on VATSIM onto the display, type its full callsign and press <b>Add</b>; use it for a flight that did not appear by itself or to bring back one you hid. If it is not found, a yellow message tells you.</li>
      <li><b>Hidden (n)</b> - shown once you have hidden a strip. Pick a callsign from the list to <b>unhide</b> it.</li>
      
      <li><b>Show</b> - <i>entry -60 / exit +15 min</i> (the default) shows a flight from 60 minutes before its oceanic entry estimate until 15 minutes after its exit estimate, so the list stays short. Flights that have been handed off, or are past their exit point, stay for those 15 minutes <b>faded</b>. If you are signed in as a Gander (CZQO_) or Shanwick (EGGX_) position you also see the other OCA's flights that are 30 minutes or less from 30W; flights that have crossed 30W to the neighbour stay 15 minutes, faded. <i>All flights</i> shows everything. "n hidden" counts the flights the time window is keeping off the list. Flights with no times, the selected flight, CPDLC-connected flights, pending natTrak requests and flights tracked by an oceanic controller always show.</li>
      <li><b>Group</b> - by track, by entry fix, or by flight level (highest first, with the number of aircraft under each heading).</li>
      <li><b>Sort</b> - newest first, entry ETA, callsign, level or last changed.</li>
      <li><b>CPDLC button</b> - opens the CPDLC drawer. It shows how many aircraft are connected, an envelope with the unread count, and flashes "new logon" when a pilot has just logged on. See section 9.</li>
      <li><b>n shown</b> - how many strips are listed.</li>
      <li><b>Auto RCL</b> - the RCL reminder (section 8). Shows <i>Auto RCL: N waiting</i> when flights need a reminder and it is waiting for the sending controller to say go. Click it to open the box.</li>
      <li><b>Settings</b> - display options, plus a <b>What's new</b> button that re-opens the latest change notes (they also appear once after an update).</li>
      <li><b>Overdue</b> (flashing clock) - Concorde flights overdue a position report. See section 11.</li>
      <li><b>Warning triangle + number</b> - how many flights currently have a warning on their strip.</li>
    </ul>

    <h3 class="text-lg font-bold mt-4 mb-1">VACS call alert</h3>
    <p class="mb-3 opacity-90">If you use VACS for voice, the FDD can flash a headphones button and show a pop-up when someone calls you, so you do not miss a call while looking at the strips. Press <b>Accept</b> in the pop-up to answer, or <b>Dismiss</b> to hide it (the call keeps ringing in VACS). It talks to VACS on your own computer only; nothing goes to our server. To set it up, open VACS (2.1.0 or newer), go to <b>Advanced Settings</b>, tick <b>Enable remote control</b> and set <b>Listen addr.</b> to <b>127.0.0.1:9600</b>. FDD for Windows has it switched on already; in a browser click the headphones button to switch it on. If it says <i>VACS offline</i>, VACS is not running or remote control is off.</p>

    <h2 id="strips" class="text-2xl font-bold mb-2">5. Flight strips and their symbols</h2>
    <p class="mb-3 opacity-90">Each flight is one strip, and strips are grouped under the track or entry fix. The group heading shows the track's fixes, and each strip shows its estimated time over every fix, lined up underneath. Click the <b>callsign</b> to open the aircraft menu and select the aircraft (it also becomes the selected aircraft in EuroScope when you use a command on it). It works the other way too: click an aircraft in EuroScope and its strip is outlined in <b>yellow</b>, scrolled into view and shown even if a filter, the search box, the time window or Hide would normally keep it off the list (needs plugin 1.5b18 or newer, and the same position on the FDD as in EuroScope).</p>
    <p class="mb-3 opacity-90">Every strip shows: callsign, aircraft type, departure and arrival, assigned level, Mach (three digits, so Concorde shows M200), the track, the fix times, and symbols:</p>
    <ul class="list-disc ml-6 space-y-2 mb-3">
      <li><b>[CALLSIGN] in brackets</b> - the pilot is logged on to CPDLC with vNAAATS.</li>
      <li><b>* after the callsign</b> - the aircraft is <b>not</b> AGCS-equipped (worked out from the PBN and remarks of the flight plan).</li>
      <li><b>Red * after the callsign</b> - you sent the pilot the natTrak reminder (RCL REQ). It clears itself when the pilot requests clearance or an oceanic controller tracks the aircraft.</li>
      <li><b>Pencil</b> - something on this strip was changed in the last 10 minutes. Hover to see who and when.</li>
      <li><b>Warning triangle</b> - two aircraft are entering on the same track and level within 10 minutes of each other, or the level is not one of the track's flight levels. Hover for the reason.</li>
      <li><b>Headset icon</b> - who the pilot is talking to, taken from the radio frequency they have tuned on VATSIM:
        <ul class="list-disc ml-6 mt-1 space-y-1">
          <li>Bold green - an oceanic controller</li>
          <li>Faded green - a domestic controller</li>
          <li>Orange - UNICOM (122.800)</li>
          <li>Red, crossed out - online but not on any controller or UNICOM</li>
        </ul>
        Hover to see the callsign and frequency. If VATSIM has no radio information, it shows who is tracking the aircraft in EuroScope instead.</li>
      <li><b>Clipboard with a tick</b> - the clearance was processed on natTrak (hover for the time).</li>
      <li><b>Flashing strip</b> - a natTrak request is waiting, a CPDLC message or logon needs a look, or a Concorde is overdue. Flashing can be turned off (see section 8).</li>
    </ul>
    <p class="mb-6 opacity-90">Strips for flights tracked by an oceanic controller (CZQO, EGGX, NAT_FSS and so on) never disappear until the flight has been untracked for 15 minutes (shown faded once untracked). Flights whose pilot has left VATSIM are hidden unless they are selected, on CPDLC, or have a request pending. A filed <b>NAT track</b> such as <i>HOIST NATF GOMUP</i> is checked against today's tracks and replaced by that track's fixes between the filed entry and exit fix, so the strip sits under the right track. After you send <b>Contact me</b> the callsign cell turns <b>yellow</b>; if 20 minutes later nobody is tracking the flight and it is still in oceanic airspace it turns <b>red</b>, and it clears once an oceanic controller tracks it. <b>If EuroScope crashes</b> or you lose your connection, the plugin tracks your aircraft again about 20 seconds after you are back (within 5 minutes, never an aircraft another controller has taken, and not if someone with a different name has signed in on the FDD at your position). Put <i>retrack=0</i> in vnaaats.cfg to turn this off. Waypoints are always shown as four digits (for example 52/50), whatever format the flight plan used.</p>

    <h2 id="menu" class="text-2xl font-bold mb-2">6. The aircraft menu</h2>
    <p class="mb-3 opacity-90">Click a callsign. The heading shows the aircraft and the <b>track</b> it is on. The tabs along the top are, in order:</p>
    <ul class="list-disc ml-6 space-y-2 mb-3">
      <li><b>Track</b> - check or change the <b>level</b>, <b>Mach</b> and <b>SELCAL</b>, then <b>Track aircraft</b> (the button is on the left; the <b>SELCAL test</b> button sits under the SELCAL box). The plugin starts tracking the aircraft in EuroScope and the menu shows the result. If another controller is handing the aircraft to you, <b>Track also accepts that handoff</b>.</li>
      <li><b>Transfer</b> - pick the controller to hand the aircraft to and press Transfer. Only stations from the CPDLC station list are offered. If none are online it says <i>No adjacent ATC - Send to UNICOM</i> (use the UNICOM button in the CPDLC drawer).</li>
      <li><b>Release Tag</b> - releases the aircraft in EuroScope (you are asked to confirm).</li>
      <li><b>SELCAL</b> - enter or change the 4-letter code and use <b>Send</b> to run a SELCAL check in EuroScope, <b>Copy .selcal</b> to paste it yourself, or <b>Save code</b> to store it on the flight. Letters allowed: A-H, J-M, P-S (a red warning names any that are not).</li>
      <li><b>CPDLC</b> - opens the CPDLC drawer for this aircraft if the pilot is connected.</li>
      <li><b>Note</b> - type a short note (up to 15 characters) and press Save note: the EuroScope scratchpad (also in the flight plan panel).</li>
      <li><b>Contact Me</b> - <b>Send</b> runs EuroScope's contact-me command for the aircraft. <b>Copy .contactme</b> copies it.</li>
      <li><b>RCL REQ</b> - the reminder asking the pilot to request oceanic clearance on natTrak (CPDLC clearance is not available). <b>Send</b> types the message into EuroScope for you, <b>Copy</b> copies it, <b>I have sent it - mark with red *</b> puts the red * on the strip and the radar tag, and <b>Remove mark</b> clears it.</li>
      <li><b>Hide</b> - hides the strip <b>for you only</b>, in this browser, for 12 hours. Nobody else is affected and EuroScope and VATSIM are not touched. Bring it back from the <i>Hidden</i> list or with <i>Add callsign</i>.</li>
      <li><b>Position reports</b> - Concorde only. See section 11.</li>
      <li><b>Supervisor</b> (red) - write a message to the supervisors, or press <b>No Contact</b> for the standard text, then <b>Copy .wallop</b> and paste it in EuroScope.</li>
    </ul>
    <p class="mb-6 opacity-90">Track, untrack, transfer, SELCAL and contact me need you to be signed in with a position and EuroScope connected. <b>Send</b> buttons need <b>autotype=1</b> in vnaaats.cfg (section 13); without it use the <b>Copy</b> buttons and paste the command into EuroScope's command line (click in it, Ctrl+V, Enter). EuroScope's contact-me and SELCAL commands work on the <i>selected</i> aircraft and take no callsign, so the plugin selects the aircraft for you before it types them.</p>

    <h2 id="panel" class="text-2xl font-bold mb-2">7. The flight plan panel</h2>
    <p class="mb-3 opacity-90">Shows the flight you last clicked. The title shows its callsign (in brackets if it is on CPDLC). Boxes you can type in can be changed when you are signed in; the change is saved on the server, every other controller sees it within seconds, and your EuroScope flight plan window updates by itself.</p>
    <ul class="list-disc ml-6 space-y-2 mb-3">
      <li><b>ACID, Type, Depart, Dest</b> - read-only.</li>
      <li><b>Entry</b> - the time at the oceanic entry fix. Edit it and <b>all the other fix times move by the same amount</b>.</li>
      <li><b>Datalink</b> - green <i>Logged on</i> when the pilot is logged on to the vNAAATS CPDLC station, a faded red <i>Not logged on</i> otherwise.</li>
      <li><b>ATC</b> - the callsign of the controller tracking the flight (green), or who the pilot is tuned to on VATSIM: oceanic (green), domestic (pale green), red <i>UNICOM</i> or red <i>NONE</i>.</li>
      <li><b>Level, Mach, Track</b> - editable. A warning appears if the level is not one of the levels for the track. The display does not check separation between aircraft; that is the controller's job.</li>
      <li><b>Route (edit)</b> - amend the route. Latitude/longitude fixes must be written as two numbers over two numbers (51/20). Forms such as 51N20W are refused with a red warning as you type. Fixes you keep keep their times; new fixes have none. This changes the FDD and server only, <b>not</b> the flight plan in EuroScope or on VATSIM.</li>
      <li><b>Note</b> (up to 15 characters) - the EuroScope <b>scratchpad</b>. It shows under the tag in EuroScope, goes with the aircraft to the next controller, and anything typed in the scratchpad in EuroScope shows here. Letters, numbers, spaces and / . , + * - only. Clear it to remove the note.</li>
      <li><b>SELCAL</b> - on the route row, editable.</li>
      <li><b>Position reports</b> button - Concorde only.</li>
      <li><b>Last change HHMMz by POSITION</b> - who last changed level, Mach, track, SELCAL or the route.</li>
    </ul>
    <p class="mb-6 opacity-90">If the panel says <i>View only - sign in and pick a position to edit</i>, nothing can be changed yet: sign in and choose a position.</p>

    <h2 id="nattrak" class="text-2xl font-bold mb-2">8. natTrak requests and alerts</h2>
    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: how to use natTrak through the FDD</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/training/vNAAATS-howto-3-natTrak.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/training/vNAAATS-howto-3-natTrak.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">The pending and cleared lists, Prepare in natTrak, RCL processed and Send reclearance. Sample flights, not a live display.</div>
    </div>

    <p class="mb-3 opacity-90">Pilots request their oceanic clearance on natTrak. The FDD shows the requests in two boxes under the controls bar:</p>
    <ul class="list-disc ml-6 space-y-2 mb-3">
      <li><b>PENDING - needs action</b> - callsign, NAT, entry fix, level, Mach and ETA. A new request flashes (box row and strip) and plays a sound. Click the row to open natTrak on that track's pending list. <b>Open natTrak</b> opens it in a small pop-up.</li>
      <li><b>CLEARED</b> - clearances already issued, with the time. Clicking one selects that flight.</li>
    </ul>
    <p class="mb-6 opacity-90">At the top of the CLEARED box, <b>Visual</b> turns the flashing and tab-title alert on or off, <b>Audio</b> turns the sound on or off, and <b>Tone</b> cycles through Ping, Chime and Beep (it plays a preview). Your choices are remembered in your browser.</p>

    <h3 class="text-lg font-bold mt-4 mb-1">Automatic RCL REQ reminder</h3>
    <p class="mb-3 opacity-90">Switched <b>on by default</b>. The server watches every flight and, when one has <b>not</b> asked for clearance on natTrak, sends it the same RCL REQ message as the aircraft menu button, without anyone clicking. A flight is sent the reminder when:</p>
    <ul class="list-disc ml-6 space-y-1 mb-3">
      <li>its oceanic entry estimate is between <b>30 and 90 minutes</b> away (or, if it has no times, it is within <b>200 nm</b> of a fix on its route and still more than 30 minutes of flying from it). A request has to be in at least 30 minutes before the entry point (after that the pilot must use voice, and the flight is listed under "By voice" in the RCL panel), and a flight that has already passed its entry point (or been through oceanic airspace) does not need one, so neither is sent a reminder;</li>
      <li>it is online and moving, has no natTrak request, is not tracked by an oceanic controller, and has not been reminded already (by hand or automatically).</li>
    </ul>
    <ul class="list-disc ml-6 space-y-1 mb-3">
      <li>It never reminds a flight that is already inside, or through, the oceanic area (between 53W and 15W, using its position and direction).</li>
      <li>It waits <b>5 minutes</b> after an oceanic controller comes online before sending anything, and flights that are already in, or inside 30 minutes of, the oceanic area at that moment are marked as handled and never reminded. If nobody is online for 10 minutes the wait starts again.</li>
    </ul>
    <p class="mb-3 opacity-90">The message is typed into <b>EuroScope</b> by one oceanic controller who is online with the plugin and <b>autotype=1</b> (the first one alphabetically, if there are several). By default it <b>asks first</b>: when flights are due, a box appears on that controller's FDD listing each callsign with its estimated time and fix, and asks whether to send the reminders now. Choose <b>Send now</b> (do not switch windows until they finish), <b>Later</b> (asks again in 5 minutes) or <b>Skip</b>. They are then sent one at a time with the progress shown, and each strip gets the red * when its message has gone. The plugin checks EuroScope is still in front before every key, and stops (without sending) if you switch away. If a send fails it tries again after 5 minutes, up to three times. The ask-first behaviour and the on/off switch are in the Auto RCL settings, and are remembered.</p>

    <h3 class="text-xl font-bold mb-1">Shanwick Tango routes</h3>
    <p class="mb-6 opacity-90">Flights filed on a Shanwick Tango route (<b>T9</b>, <b>T290</b>, <b>T213</b>, <b>T13</b>, <b>T16</b>) are listed together in one group, <b>TANGO ROUTES</b> (each strip shows its route and fixes), below the NAT tracks and above RANDOM, and in the Tracks drawer under <i>Shanwick Tango routes</i>. A route is recognised when the filed route names the airway and one of its fixes, or both of its fixes. T9 is southbound only (even levels FL300-400, LASNO to BEGAS); T290 is northbound only (odd levels FL290-410, ADVAT to GELPO); T9 and T290 are exempt from the CPDLC mandate. T213 (TAMEL, BERUX) goes both ways; T13 (NILAV to OMOKO) is northbound and T16 (OMOKO to GONAN) southbound; CPDLC is required on T213, T13 and T16 between FL290 and FL410, and T13 and T16 continue into Santa Maria airspace. A level that does not fit T9 or T290 shows the usual level warning. They need an oceanic clearance like any other flight. Northbound counts as EB and southbound as WB for the EB / WB filter. A Gander position does not list them unless it is tracking one.</p>

    <h2 id="cpdlc" class="text-2xl font-bold mb-2">9. CPDLC</h2>
    <div class="p-4 mb-6 border-2 border-blue-10 bg-blue-3">
      <div class="font-bold mb-1">Video: how to use CPDLC</div>
      <video class="w-full bg-black" controls preload="none" playsinline src="/training/vNAAATS-howto-2-CPDLC.mp4">Your browser cannot play this video. <a class="underline" rel="external" href="/training/vNAAATS-howto-2-CPDLC.mp4">Download it</a>.</video>
      <div class="text-sm opacity-80 mt-2">Signing in with your Hoppie code, accepting pilots, level messages, clearance requests, UNICOM and the pop-out window. Sample flights, not a live display.</div>
    </div>

    <p class="mb-3 opacity-90">Pilots connect to the vNAAATS CPDLC station and you talk to them from the CPDLC drawer, opened with the <b>CPDLC</b> button in the controls bar.</p>
    <h3 class="text-lg font-bold mt-4 mb-1">Signing in</h3>
    <p class="mb-3 opacity-90">Each controller signs in with their <b>own Hoppie logon code</b>. The code is typed into the drawer, kept in the server's memory only while you are signed in, and never saved. Your position sets the station (CZQO = CZQX, EGGX = EGGX, NAT_FSS = NATX). Use <b>sign out of CPDLC</b> when you leave. Restarting the server logs everyone out of CPDLC, so sign in again if that happens.</p>
    <p class="mb-3 opacity-90"><b>Force log out.</b> If a station is stuck because someone left without signing out, use <i>Force log out a CPDLC callsign...</i> in the drawer. You type the callsign (for example CZQX), read the warnings, then type <b>LOG OUT</b>. The station is cleared at once, whoever is using it on this server is logged out of the Hoppie callsign and its aircraft lose their CPDLC connection, and nobody can sign in to that station for <b>3 minutes</b>. Check with the other controller first.</p>
    <h3 class="text-lg font-bold mt-4 mb-1">Working with pilots</h3>
    <ul class="list-disc ml-6 space-y-2 mb-3">
      <li><b>Connected aircraft</b> are listed at the top of the drawer. Click one to see the message thread.</li>
      <li><b>New logon:</b> when a pilot logs on, the alert sound plays <b>once</b>, their callsign flashes in the drawer and on the strip, and the CPDLC button flashes "new logon". It stops when you look at that aircraft.</li>
      <li><b>New message:</b> a sound plays, the button shows an envelope with the unread count, and the strip flashes until you open it.</li>
      <li><b>Replying:</b> quick buttons <i>unable</i>, <i>standby</i>, <i>roger</i>; the <i>reply...</i> choices (Wilco/Unable, Affirm/Negative, Roger, None) set the response type; or write your own text and press <b>Send</b>.</li>
      <li><b>Standard messages...</b> - the TopSky message list.</li>
      <li><b>Transfer</b> - when you hand an aircraft that is on CPDLC to another controller, the pilot is sent <i>CONTACT &lt;station&gt; ON &lt;frequency&gt;</i> and is logged off when they answer WILCO or ROGER.</li>
      <li><b>UNICOM</b> (asks you to confirm) - sends "THERE IS NO FURTHER ATC AVAILABLE. SERVICE TERMINATED. MONITOR ADVISORY ON 122.800", logs the pilot off when they answer WILCO or ROGER, and releases the tag in EuroScope.</li>
    </ul>
    <p class="mb-6 opacity-90">You need a controller position to send messages. Aircraft on CPDLC show their callsign in brackets, and the flight plan panel shows <i>CPDLC on</i>.</p>

    <h2 id="tracks" class="text-2xl font-bold mb-2">10. Tracks panel</h2>
    <p class="mb-6 opacity-90">Bottom right. The heading shows the TMI. Westbound and eastbound tracks are listed with their validity time once on the heading (for example <i>Westbound 1130-1900z</i>). Each track is two lines: its name and fixes, then its flight levels. The same fixes appear as the column headings over the strips.</p>

    <h2 id="conc" class="text-2xl font-bold mb-2">11. Concorde position reports</h2>
    <p class="mb-3 opacity-90">For aircraft type CONC, a <b>Position reports</b> button appears in the aircraft menu and flight plan panel. It opens a table with every fix on the route and three times: <b>Calc</b> (calculated), <b>Est</b> and <b>Actual</b>. Type times as four digits, or press <b>now</b> to fill the current time. Reported times replace the calculated times on the strip.</p>
    <p class="mb-6 opacity-90">A Concorde <b>flashes</b> (strip, button and table row), and the top bar shows a red <i>overdue</i> count, when the next fix's time passed more than 3 minutes ago with no report.</p>

    <h2 id="es" class="text-2xl font-bold mb-2">12. What you see in EuroScope</h2>
    <ul class="list-disc ml-6 space-y-2 mb-3">
      <li><b>Aircraft icons</b> on the radar point along the heading. <b>Filled</b> means cleared, <b>outline</b> means not cleared yet.</li>
      <li><b>[CALLSIGN]</b> in brackets on the tag - the pilot is on CPDLC.</li>
      <li><b>* under the callsign</b> - not AGCS equipped.</li>
      <li><b>Red *</b> - the natTrak reminder was sent from the FDD.</li>
      <li><b>Note under the tag</b> - the scratchpad. You can type it in EuroScope or on the FDD.</li>
      <li><b>Conflict alerts</b> flash for aircraft that are too close. A one-level difference is allowed 200 ft of tolerance, so aircraft exactly one level apart do not flash.</li>
      <li><b>Flight plan window:</b> the track, level, Mach and SELCAL boxes are the same data as the FDD. The Datalink box shows <i>CPDLC ON</i> or <i>CPDLC OFF</i>. The SELCAL box accepts four letters such as ASCM or AS-CM; clear it to go back to the code in the remarks. The <b>FDD</b> button in the menu bar opens the display.</li>
      <li><b>Clear / Unclear</b> - tracking an aircraft no longer changes its transponder code. Pilots who log on to CPDLC are sent a message to squawk <b>2000</b> within 10 minutes of entering the OCA.</li>
      <li><b>Handoff</b> - when you hand an aircraft that is on CPDLC to another controller (Co-ordination / Active Authorities > Transfer), the pilot is sent the contact message and logged off after WILCO/ROGER, as with the FDD Transfer.</li>
      <li><b>Updates:</b> a pop-up appears when a newer plugin is available and opens the download page. Replace <b>only the DLL</b> and keep your vnaaats.cfg.</li>
    </ul>
    <p class="mb-6 opacity-90">Anything you change in EuroScope shows on the FDD within seconds, and anything you change on the FDD appears in EuroScope in the same way.</p>

    <h2 id="cfg" class="text-2xl font-bold mb-2">13. Settings in vnaaats.cfg</h2>
    <p class="mb-3 opacity-90">A text file next to VatsimNAAATS.dll. Open it in Notepad, save, then restart EuroScope.</p>
    <ul class="list-disc ml-6 space-y-2 mb-6">
      <li><b>server=https://vnaaats.com</b> - the server (keep the https).</li>
      <li><b>key=...</b> - your access key from the Gander Oceanic members page. Keep it private.</li>
      <li><b>autotype=1</b> - lets the FDD <b>Send</b> buttons type into EuroScope for you. The plugin brings EuroScope to the front, clears the command line and types one command at a time. With <b>autotype=0</b>, or if Windows refuses, the command is copied for you to paste instead and the menu tells you so.</li>
    </ul>

    <h2 id="flow" class="text-2xl font-bold mb-2">14. A typical shift, step by step</h2>
    <ol class="list-decimal ml-6 space-y-2 mb-6">
      <li>Connect EuroScope to VATSIM as your position. Open the FDD, sign in and choose the same position.</li>
      <li>Check the <b>Tracks</b> panel for today's NATs and the <b>Show</b> setting for the strips you want.</li>
      <li>An aircraft approaching the oceanic boundary has not asked for clearance: open its menu and use <b>RCL REQ &gt; Send</b>, then <b>I have sent it - mark with red *</b>.</li>
      <li>The pilot requests clearance on natTrak: the PENDING box flashes and plays a sound. Click the row to open natTrak and issue the clearance. The flight moves to CLEARED.</li>
      <li>Check level, Mach, track and SELCAL in the <b>flight plan panel</b>. Fix anything wrong; add a <b>Note</b> if you want one under the tag.</li>
      <li>When the aircraft is yours: <b>Track</b> tab &gt; <b>Track aircraft</b> (or accept the handoff the same way).</li>
      <li>A pilot logs on to CPDLC: the sound plays, the callsign and button flash. Open the drawer, reply or send the standard messages you need.</li>
      <li>Aircraft leaving your area: <b>Transfer</b> tab &gt; pick the next controller. With no one online, use <b>UNICOM</b> in the CPDLC drawer.</li>
      <li>A flight you do not want to see: <b>Hide</b> it. It only disappears for you, and comes back from the <i>Hidden</i> list.</li>
    </ol>

    <h2 id="help" class="text-2xl font-bold mb-2">15. Problems and questions</h2>
    <ul class="list-disc ml-6 space-y-2 mb-6">
      <li><b>The page looks old or a new feature is missing:</b> hard refresh (Ctrl+Shift+R on Windows, Cmd+Shift+R on Mac). The header shows the display and server versions.</li>
      <li><b>"No data on remote":</b> the server is not answering. Try again in a minute and tell the Gander Oceanic team if it stays.</li>
      <li><b>Track, transfer, SELCAL or contact me does nothing:</b> check you are signed in with a position, EuroScope is connected as that position, and the plugin is loaded. The result (or the reason) is shown at the bottom of the aircraft menu.</li>
      <li><b>Send buttons only copy:</b> <b>autotype=1</b> is missing from vnaaats.cfg or Windows refused to let the plugin type. Use the Copy buttons.</li>
      <li><b>A flight is missing:</b> it may be outside the time window (see <b>Show</b>), hidden by you (see <b>Hidden</b>), or filtered. Use <b>Add callsign</b> to bring it in.</li>
      <li><b>No sound:</b> browsers only play sound after you have clicked somewhere on the page. Click once, and check <b>Audio</b> is ON in the natTrak box.</li>
      <li><b>Changes not saving:</b> you have not chosen a position, or the key in vnaaats.cfg is missing or wrong.</li>
      <li><b>Reporting a bug:</b> include the callsign, what you clicked, what you expected, what happened, the plugin version (first line of vNAAATS.log) and the log file. Report at <a class="underline" href="https://github.com/garythms/vatsim-naaats-v2/issues">GitHub issues</a> or to the Gander Oceanic team.</li>
    </ul>

    <p class="text-sm opacity-80">Independent community project, not run or endorsed by VATSIM. <a class="underline" href="/privacy">Privacy and data handling</a>.</p>
  </div>
</div>
