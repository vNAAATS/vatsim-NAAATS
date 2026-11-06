vNAAATS V2 plugin - install guide and release notes
=========================================

INSTALL
1. Close EuroScope.
2. Put VatsimNAAATS.dll in your EuroScope Plugins folder (any folder is fine).
3. Copy vnaaats.example.cfg to the SAME folder as the DLL and rename it vnaaats.cfg.
4. Open vnaaats.cfg in Notepad and fill in:
      server=https://vnaaats.com
      key=<the key you were sent - keep it private>
      squawk=1
   (squawk=0 turns off automatic squawk 2000 when you start tracking an aircraft)
5. Start EuroScope > Other SET > Plug-ins > Load, pick VatsimNAAATS.dll.
6. If it will not load, install the Microsoft Visual C++ 2015-2022 Redistributable (x86).

NEW IN 1.4b3
- CPDLC handoff: when you hand an aircraft that is on CPDLC to another controller (flight plan window > Co-ordination / Active Authorities > Transfer), the pilot is sent "CONTACT <station name> ON <frequency>, BYE." and is logged off CPDLC when they reply WILCO/ROGER.
- UNICOM (button in the FDD CPDLC drawer): the pilot is told there is no further ATC, logged off when they reply, and the tag is released in EuroScope.

ALSO IN 1.4b2
- Radar targets are now aircraft icons (pointing along the heading) instead of stars and diamonds. Filled = cleared, outline = not cleared yet.

ALSO IN 1.4b1
- CPDLC: an aircraft connected to CPDLC shows its callsign in brackets on the radar tag, e.g. [BAW123].
- The server is now https://vnaaats.com (secure). The FDD is at https://vnaaats.com/fdd. If you used the old IP address in vnaaats.cfg, change it.
- SELCAL box in the flight plan window is editable (4 letters, A-H J-M P-S, e.g. ASCM or AS-CM).
  Clear the box to go back to the SEL/ code in the remarks.
- FDD button opens your own server's flight data display (<server>/fdd).
- Download/update links point to https://vnaaats.com/download.
- Your callsign is sent with every change, so the FDD shows who last changed a flight.
- If a controller changes level, Mach, track or SELCAL on the web FDD, your flight plan window updates by itself within a few seconds.

WHAT TO TEST
- Track list loads, flight plans open, Clear/Unclear works.
- Squawk 2000 assigned on oceanic clearance.
- Edit SELCAL, close and reopen the flight plan - the value should stay.
- The FDD (menu bar button) shows your aircraft.

REPORTING BUGS
https://github.com/garythms/vatsim-naaats-v2/issues
Include: callsign, what you clicked, what you expected, what happened, plugin version (shown when it loads),
and the log file (vnaaats log in the DLL folder or EuroScope folder) if you can find it.

NEW IN 1.5b1: FDD commands
Click a callsign on the web Flight Data Display for the aircraft menu. Track, Untrack and Transfer are carried out by this plugin in EuroScope
(it checks the server every 2 seconds, so keep EuroScope connected as your position). SELCAL test, Contact me and Supervisor copy the
EuroScope command for you to paste into the command line.

NEW IN 1.5b3: a red * beside the callsign on the radar tag means a natTrak reminder was sent to that pilot from the FDD (aircraft menu > NatTrak).
It disappears when the pilot requests clearance or an oceanic controller tracks the aircraft.

NEW IN 2.7.2: (1) Contact Me from the FDD now clicks the callsign on the radar tag, which is what EuroScope waits for after .contactme, so you no longer click it yourself. The plugin checks that the click reached the tag, tries once more if not, and tells the FDD "Contact Me NOT sent" if it still did not. (2) Auto RCL typing tries harder to bring EuroScope to the front (up to four attempts); backgroundtype=1 in vnaaats.cfg is an experiment that types without EuroScope in front. (3) A route drawn by right-clicking a callsign is erased after 10 seconds (routeseconds in vnaaats.cfg; 0 keeps it). (4) New contactbutton=left|middle|right in vnaaats.cfg for controllers whose EuroScope selects aircraft with another mouse button. Replace only VatsimNAAATS.dll and keep your vnaaats.cfg.
NEW IN 2.7.1: Fixes the version number. 2.7.0 still said it was 2.6.0 inside EuroScope and in the log, so it kept offering an update to itself. If you have 2.7.0 (or 2.6.0), replace only VatsimNAAATS.dll with this one and keep your vnaaats.cfg. Everything described under 2.7.0 is in this build.

NEW IN 2.7.0: (1) The plugin no longer assigns squawk 2000 when you track an aircraft (it upset adjacent domestic positions). Pilots signing on to CPDLC are now sent "SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA" instead. (2) Area Sel is now one locked area called OCA (the whole oceanic area) and Select is locked to Multi-role, so handed-over aircraft show straight away. (3) The plugin picks up commands from the FDD every second instead of every two. Replace only VatsimNAAATS.dll (the squawk setting in vnaaats.cfg is no longer used).

NEW IN 2.4.0: (1) Contact Me from the FDD now outlines the aircraft's EuroScope tag in yellow, red after 20 minutes if nobody is tracking it. (2) If an aircraft you were tracking disconnects and its pilot returns within 5 minutes, the plugin tracks it again with the same Flight Level, Mach and SELCAL (retrack=0 in vnaaats.cfg turns this off, together with the re-track after a crash). (3) Any four letters are accepted as a SELCAL, and a SELCAL filed in the VATSIM remarks (SEL/ABCD) is filled in on the FDD.

NEW IN 2.3.0: Aircraft nobody has handled yet now show the route filed with VATSIM, straight away, when it looks right (it crosses the Atlantic the right way for its departure and destination). The route box only says CHECK / EDIT ROUTE when the filed route is missing, runs the wrong way or does not cross the ocean. Needs server 2.3.0. Replace only VatsimNAAATS.dll.

NEW IN 2.2.1: When an aircraft has no route yet, the route box in the flight plan window now says CHECK / EDIT ROUTE instead of AIRCRAFT. Replace only VatsimNAAATS.dll.

NEW IN 2.2.0: Admin log. The plugin now sends a copy of its warnings and errors (and the end of the previous session's log after a crash) to the vNAAATS server, so problems can be found quickly. It never sends your key or any password, and it never slows EuroScope down (it sends in the background; if the server cannot be reached the lines are simply dropped). The admin can ask for full detail from one controller for two hours while chasing a fault. To switch it off, add remotelog=0 to vnaaats.cfg (remotelog=2 sends everything). Needs server 2.2.0. Replace only VatsimNAAATS.dll.

NEW IN 2.1.0: Edit the flight plan from the web display. When an aircraft's Type shows NOFP, click the Type box on the Flight Plan panel to open the whole flight plan (flight rules, aircraft data, departure, destination, alternate, level, TAS, times, route, remarks). The controller tracking the aircraft can change it and Save; EuroScope then sends the amended plan to the pilot. Everyone else sees it read only. Also: a flight plan EuroScope showed as NOFP is refreshed automatically once the plan has loaded. Needs server 2.1.0 and the new web display. Replace only VatsimNAAATS.dll.

NEW IN 2.0.7: TRACK on the web display now accepts a handoff that was offered to you (before it said "EuroScope would not let you track ... handoff offered to <you>"). Replace only VatsimNAAATS.dll.

NEW IN 2.0.6: Fixes "Waiting - is your EuroScope running with the vNAAATS plugin and signed in as ..." on the web display when you ARE signed in. The plugin only learned your callsign once an aircraft had appeared on your list, so a quiet screen never collected web commands (TRACK, TRANSFER, SELCAL and so on). It now knows your callsign as soon as you log on. Replace only VatsimNAAATS.dll.

NEW IN 2.0.5: Missing traffic fixed. With Area Sel = BDBX and Select = Multi-role (the whole area) every aircraft at or above FL250 between 43N-75N and 60W-10W now shows, even when EuroScope has no sector entry time for it. Logging on as NAT_FSS (the bandbox) selects BDBX and Multi-role for you. Use CZQX / EGGX or Delivery only when the area is split or you are running one sector or Delivery. Also, TRACK on the web display now loads an aircraft the radar has not listed yet instead of saying vNAAATS has not loaded it. Replace only VatsimNAAATS.dll.

NEW IN 2.0.4: (1) Clicking TRACK on the web display now shows you as the tracker straight away - the headset icon updates without a second click. (2) If an aircraft's flight plan reached EuroScope after vNAAATS first saw it, the blank Type / Depart / Dest are now filled in automatically. Replace only VatsimNAAATS.dll.

NEW IN 2.0.3: ADVAT (the start of Tango route T290) is now known as a Shanwick entry/exit point. The Shanwick Tango routes themselves are handled on the web display (server 2.0.2). Replace only VatsimNAAATS.dll.

NEW IN 2.0.2: the speed on the radar tag now shows the assigned Mach (for example M79) once a controller is tracking the aircraft and a Mach number has been entered. Before that, or if none is entered, it still shows the ground speed (N485). Replace only VatsimNAAATS.dll.

NEW IN 2.0.1: aircraft that are handed off or pass their exit point stay on the EuroScope display for 15 more minutes instead of disappearing. Needs server 2.0.1 for the other changes below (the web display shows them faded; neighbour traffic within 30 minutes of 30W; Auto RCL no longer reminds aircraft already in the OCA and waits 5 minutes after the session starts). Replace only VatsimNAAATS.dll.

NEW IN 2.0.0: the first full release of vNAAATS V2. It collects everything from the 1.5 builds: the live web Flight Data Display, CPDLC, natTrak clearances, Auto RCL, tracking and handoffs from the web, re-tracking after a crash, Concorde tracks and position reports, personal keys for certified oceanic controllers and the two crash fixes from 1.5b21 and 1.5b22. Replace only VatsimNAAATS.dll and keep your vnaaats.cfg. Needs server 2.0.0.

NEW IN 1.5b23: when EuroScope refuses a track or handoff accept from the web FDD, the message and the log now say who has the aircraft, who the handoff was offered to and who you are.

NEW IN 1.5b22: fixes a second EuroScope crash that could happen when an aircraft you had selected (with its route drawn) left your area and its data was removed. Removal now happens on the drawing thread, after the selection and route are cleared.

NEW IN 1.5b21: fixes a EuroScope crash that could happen while the flight plan window was open and the flight was edited on the web FDD (a route or level change). Changes from the server are now applied on the same thread that draws them. The log now notes "Web/server change applied to CALLSIGN".

NEW IN 1.5b20: the re-track after a crash now only happens if you are back within 5 minutes, and not if someone else has signed in on the FDD at your position (a different name). Needs server 0.19.2.

NEW IN 1.5b19: after a EuroScope crash or a lost connection, the plugin tracks again the aircraft you had (only ones nobody else is tracking, and only if you are back within 5 minutes). It starts about 20 seconds after you reconnect. retrack=0 in vnaaats.cfg turns it off. Needs server 0.19.0.

NEW IN 1.5b18: click an aircraft in EuroScope and its strip is outlined in yellow and scrolled into view on the FDD (needs server 0.17.0 / display 0.16.0).

NEW IN 1.5b17: fixes a lost space when typing into EuroScope (".msgBAW123 Hello..." instead of ".msg BAW123 Hello..."). The plugin now types one character at a time and presses a real Space key.

NEW IN 1.5b16: no feature changes - this build tests the automatic Discord update notice. Handoff accept via Track needs 1.5b15 or newer.

NEW IN 1.5b15: fixes a crash in 1.5b14 when new aircraft appeared (use 1.5b15, not 1.5b14).

NEW IN 1.5b14:
- Note / scratchpad: the note box on the FDD flight plan is the EuroScope scratchpad. It shows under the tag and goes with the aircraft to the next controller; notes typed in EuroScope show on the FDD.
- Track on the FDD now accepts a handoff that is being offered to you.
- Contact Me / SELCAL from the FDD: EuroScope's .contactme and .selcal act on the SELECTED aircraft and take no callsign. The plugin now selects the aircraft in EuroScope first.

NEW IN 1.5b13: typing into EuroScope (SELCAL / contact me / RCL REQ message) now works step by step on its own thread, clearing the command line first, so old text no longer gets mixed into the new command.

NEW IN 1.5b12: the "not equipped" * is now worked out again from the EuroScope flight plan each time, so an aircraft whose PBN/ remarks include B1 or B2 no longer keeps a stale star.

NEW IN 1.5b11: fixed a possible crash when reading an aircraft's equipment (PBN remarks at the end of a flight plan). A red-free * under the callsign on a tag means the aircraft is NOT AGCS equipped.

NEW IN 1.5b10: the update pop-up now opens the vNAAATS download page and says to replace only VatsimNAAATS.dll and keep vnaaats.cfg. The flight plan window "Datalink" box now shows CPDLC ON / CPDLC OFF (whether the pilot is logged on to the vNAAATS CPDLC station).

NEW IN 1.5b9: Send buttons (SELCAL, Contact, NatTrak message) now type one command at a time and replace anything already in the EuroScope command line, so two commands are no longer joined into one line.

NEW IN 1.5b8: conflict alerts no longer flash for aircraft one flight level apart (a 200 ft allowance is applied to the vertical separation, because real altitudes are never exactly on the level).

NEW IN 1.5b7 (and 1.5b6): fixed a crash on opening when the plugin could not reach the server (it now shows an error message instead, with a code).

NEW IN 1.5b5: start-up logging and protection (vNAAATS.log shows each start-up step). If EuroScope still crashes on opening, send vNAAATS.log.

vNAAATS.asr (in the zip)
This is the vNAAATS radar screen layout. Its sector-file lines are left blank, like the AeroNav ASR files, and it has no personal settings in it, so it should open with whatever Gander sector file you have loaded. Put it in your EuroScope ASR folder, or open it from Open SCT/ASR. If EuroScope still shows a "different sector file" warning, pick your latest Gander (EXCXO) sector file, then press Save once.

NEW IN 2.5.1: Relief - on the FDD, the Relief button hands every aircraft you track to the controller taking over; their plugin accepts them automatically. Replace the DLL on BOTH controllers.
NEW IN 2.5.2: (1) Squawk 2000 is now given as soon as you start tracking an aircraft (not at clearance). (2) Track from the FDD works without a flight level or Mach. (3) The FDD circle works without a flight plan. (4) Relief: the FDD Relief button hands all your tracked aircraft to the next controller. Replace the DLL.
NEW IN 2.5.0: (1) After .contactme the plugin clicks the aircraft for you (autoclick=0 turns off). (2) Clicking an aircraft on the FDD circles it in white for 3 seconds (highlight=0 turns off). (3) Log off pilot from CPDLC, natTrak legacy requests ignored, Tango routes grouped. (4) The FDD Settings button changes autotype/autoclick/highlight/retrack/squawk/remotelog here for you; the plugin writes them into vnaaats.cfg (and a settingsstamp= line, leave it) within about 2 minutes and tells you to close and reopen EuroScope. Replace the DLL.
NEW IN 2.4.1: (1) The cursor lat/long readout and ruler were wrong unless EuroScope was full screen (E30 over 30W) - fixed. (2) The selected aircraft is shown at the bottom centre of the radar with its radio callsign, aircraft type and route, e.g. BAW227 (Speedbird)   B788   EGLL - KBOS. It reads ICAO_Airlines.txt, which must sit in the SAME folder as the DLL (it is included in the zip). Replace the DLL and also copy ICAO_Airlines.txt next to it.

NEW IN 2.6.0: (1) Safer auto-typing into EuroScope: if you click into another program (Discord, a browser) while the plugin is typing a message, typing stops at once and the message is NOT sent (the FDD tells you), instead of carrying on in the other window. (2) Works with the new ask-first Auto RCL on the FDD (server 2.6.0). Replace the DLL.
