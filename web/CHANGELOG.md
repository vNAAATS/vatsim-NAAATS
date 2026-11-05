# Flight data display changelog

Version shown in the header (`src/config.json`). Bump it whenever the display changes.

## Unreleased
- The "N alerts" button is now clickable: it jumps to the flight with the warning, outlines its strip and selects it; click again for the next one (suggested by a controller).
- Strip list: each Shanwick Tango route (T9, T13, T16, T213, T290) is now its own group, like the NAT tracks, with its route in the header, instead of one TANGO ROUTES group. Concorde tracks were already one group each. Tango groups come after the NAT and Concorde tracks, before RANDOM.
- EuroScope plugin: the Area Sel box (CZQX / EGGX / BDBX) is gone. It is now one locked area called OCA (the whole oceanic area, what BDBX was), because the CZQX and EGGX visibility ranges were too small. The Select box (Delivery / OCA Enroute / Multi-role) is locked to Multi-role as well. An older saved .asr is forced to both.
- Squawk 2000 is no longer set by the plugin when you track an aircraft (it upset adjacent domestic positions): the plugin setting and its cfg key are gone. Instead every pilot accepted onto CPDLC is automatically sent "SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA" (VNAAATS_CPDLC_SQUAWK_MSG=off turns it off). Plugin 2.7.0.
- Aircraft menu, Track tab: the Mach box is now required. The faint text in it is the Mach on natTrak, which is used if you leave the box blank; the FL box shows the natTrak level the same way.
- Prepare in natTrak (CPDLC clearance requests): the Mach and level the pilot asked for in the request text (REQ: M0.86 FL400) now fill the natTrak form; Mach was left empty before.
- The TrackAudio chip keeps a fixed width so it no longer moves between RX and TX with long callsigns.
- Quicker response to FDD commands: the plugin checks for FDD commands every second (was 2) and the display checks the result twice a second (was once). The server now logs any request slower than half a second. Server deploy and plugin 2.7.0 needed.
- FDD for Windows now updates itself: at start-up it checks for a newer version of the app, and if there is one asks "Update now / Later"; Update downloads the signed installer, installs it and restarts the app. Also fixes the natTrak window commands (nattrak_open, nattrak_signed_in) not being allowed through the app permissions, which made natTrak open in the browser even in the new app. Everyone needs to install the app once more to get this; after that it updates itself. Server deploy needed (web files).
- FDD for Windows: the app itself does not update by itself, so a copy installed before the in-app natTrak window existed still opened natTrak in the web browser. The natTrak box now notices this, says the app is out of date and offers the download of the new FDD for Windows installer. Server deploy needed (web files).
- CPDLC: if you have a saved Hoppie code, a box now asks "Connect to CPDLC now?" once per visit instead of signing you in by itself, with the note that if another user is already logged in to CPDLC they must log off first. The natTrak notice also says you must be connected to EuroScope / the VATSIM network before connecting to natTrak. Server deploy needed (web files).
- Settings (FDD for Windows): a new "Audio client" box shows whether Audio for VATSIM or TrackAudio is chosen and has a "Change audio client at next start" button. The setup screen then opens the next time FDD for Windows starts, where the audio client can be changed (Cancel undoes it). Server deploy needed (web files).
- FDD for Windows, natTrak sign-in moved: the question about signing in to natTrak no longer appears when you pick a position. Instead the natTrak box on the display shows a notice and a LOGIN button (shown instead of "Open natTrak") telling you to log in with your VATSIM details once you are connected to the network through EuroScope. LOGIN opens natTrak in a window of the app; when you are signed in the window moves on to the pending list and the button becomes "Open natTrak", which reopens that same window. The "Prepare in natTrak" window uses the same sign-in. Needs a new installer build (launcher change) plus a server deploy.
- FDD for Windows: if natTrak answers 403 in the "Prepare in natTrak" window, a red banner now explains that the signed-in account is not an active oceanic controller on natTrak and what to do. Needs a new installer build (launcher change).
- Send reclearance / RCL processed: if the pilot is not logged on to CPDLC (for example they asked for their clearance by telex), the messages are now sent to them as a plain telex instead of failing with "not connected to CPDLC". The window says so before you press Send. Server deploy needed.
- CPDLC: a new oceanic clearance request plays its own rising three-note chime (once per request) instead of the normal message sound. A "chime" tick-box in the clearance requests list switches it off and on (kept on this computer). Clearance requests sent by pilots as a plain Hoppie telex now reach the same list (server), and CPDLC logon is accepted up to 1000 nm from the tracks. Server deploy needed.
- /training: fourth how-to video, VACS and TrackAudio in FDD for Windows. Server deploy needed (web files).
- TrackAudio: the aircraft that is transmitting gets a green box around its row in the flight list while it talks, and the box fades out when it stops. On by default; the tick-box "Highlight the aircraft that is transmitting" in the TrackAudio (headphones) window switches it off and on (kept on this computer). The list also scrolls to the aircraft that starts transmitting if its row is off screen (second tick-box, "Scroll the list to the aircraft that is transmitting", on by default). Needs TrackAudio switched on in the FDD. Server deploy needed (web files only).
- RCL reminder message (RCL REQ Send, Auto RCL, Copy natTrak message) no longer says CPDLC clearance is unavailable: it now asks the pilot to request their oceanic clearance by CPDLC, or via nattrak.vatsim.net, no later than 20 minutes before the oceanic entry point, then "Squawk 2000 upon entering the OCA." Server deploy needed.
- Hidden instructor page /trainee-key (not linked, not indexed): a trusted instructor enters the instructor password (server setting VNAAATS_INSTRUCTOR_PASSWORD, 10+ characters; page is off until it is set), their own name and the student's CID, and gets a plugin key + FDD password for the student that works for 2 hours. It never replaces a longer-lived key for the same CID, and who issued each key is written to the server log. Server deploy needed.
- Homepage: a "Pages" drop-down next to the buttons lists the other pages of the site.
- New /training page for oceanic trainees: the trainee introduction video (area, callsigns, vNAAATS, FDD, CPDLC, natTrak), a before-your-session checklist, how to work a clearance request, and the three how-to videos. Server deploy needed (web files).
- Staged roll-out: the "RCL processed" / "Send reclearance" buttons are OFF until the server setting VNAAATS_CPDLC_RCL_SEND=on is added to /etc/vnaaats.env (then restart). "Prepare in natTrak", the clearance-request list, TrackAudio (off until switched on) and pop-outs are unaffected.
- natTrak clearance flow for CPDLC requests: each request has "Prepare in natTrak" (FDD for Windows opens natTrak with the form filled in and never presses Create; in a browser a box of values to copy), and "RCL processed" / "Send reclearance". The send buttons unlock once the flight is on natTrak, show an editable preview, and send from our Hoppie station: RCL PROCESSED - FLY FLIGHT PLAN ROUTE, or the CLX message (NAT CLRNCE ... FM fix/time MNTN level Mach, SQUAWK 2000 WITHIN 10 MINS OF ENTERING OCA, END OF MESSAGE), plus a contact line for Gander, Shanwick or NAT FSS from your position. Sent state shows who sent it. Server deploy needed; the natTrak window needs the new FDD for Windows installer.
- TrackAudio: new "Last RX" button (callsign, frequency, time and length of each call heard, and your own transmissions), history kept on your computer for 24 hours, no audio recorded. Switch it on from the button; TrackAudio must be running.
- Pop-out windows: the CPDLC window and the TrackAudio history can be opened in their own window (FDD for Windows) or browser pop-up, for another screen.
- CPDLC oceanic clearance requests: a pilot who asks for an oceanic clearance over CPDLC is now told "RCL RECEIVED - STANDBY FOR CLEARANCE" (instead of "UNABLE ... use natTrak") and the request appears in a new list at the top of the CPDLC window, marked "not in natTrak" (red after 5 minutes), "natTrak: pending" or "natTrak: cleared". Server switch VNAAATS_CPDLC_RCL=off restores the old refusal. Server deploy needed.
- CPDLC: clicking a flight level now sends the level message straight away (tick-box "Send on click" under the level buttons switches this off, so the button only fills the message box).
- CPDLC: flight-level quick buttons (FL300 to FL410). Pick Climb, Descend, Maintain or Expect climb, then a level, and the message is filled in. The long list of standard messages is now a small "More standard messages" drop-down.
- CPDLC window redesign: the connected list now shows 10 aircraft before the scroll bar (it was stuck at about two) with a count badge, an auto-accept tag and how long each has been connected; only the latest 10 messages are shown; tidier message cards and header.
- Fix: the Auto RCL "Sending reminder" box could get stuck (after switching Auto RCL off, or if EuroScope never answered) and its Close button did nothing. Released reminders now expire if nothing is typed, switching off cancels them, Close always works, and there is a Stop sending button.
- Auto RCL now asks first (switchable): flights en route to oceanic airspace that need the RCL REQ reminder wait in a list. At most every 5 minutes (and not until 5 minutes after the oceanic controller's session starts) the sending controller gets a box listing them with their entry time and fix, and Send now / Not now / Skip these. While sending, a box says "keep EuroScope in the front and do not change windows". The Auto RCL button flashes and shows how many are waiting. Plugin 2.6.0 also stops typing at once if you switch to another window.
- New "What's new" box: shown once per announced release after sign-in (and any time from Settings > What's new). Notes live in src/typescript/fdd/whatsnew.ts; add an entry at the top for a release worth announcing. Display version 2.6.0.
- VACS call alert: the app setup now explains how to switch on Remote Control in VACS (Advanced Settings, address 127.0.0.1:9600 recommended) and shows a green tick when connected; added to the install guide and troubleshooting.
- FDD header tidied: position, relief and sign out now sit in one "Position" menu; VACS, CPDLC, Auto RCL and alerts are status buttons on the top row; Show, Group, Sort and one "Search or add" box are on the second row.
- Home page: the feature boxes now have a one-line description each and include FDD for Windows and VACS Call Alerts. User manual updated for the new header, VACS call alert and ask-first Auto RCL.
- New VACS call indicator (headphones button): shows when VACS has an incoming call with a pop-up (Accept or Dismiss) and who you are talking to. It listens to VACS on your own computer only (VACS Remote Control must be switched on, default port 9600); on by default in FDD for Windows, optional in a browser.
- The PDF user guide is regenerated from the current install guide and user manual.
- Install guide: more troubleshooting for FDD for Windows (installer blocked, blank window, setup keeps returning, update fails, administrator rights, nothing opened).
- New "FDD for Windows" page (vnaaats.com/windows): what it is, why use it, Windows requirements and a download button. The home page links to it, and the download everywhere now comes from one stable address on this site (vnaaats.com/get/fdd-windows).
- FDD for Windows is now a normal release called "FDD for Windows" (download link on the download page and install guide) instead of the "test build".
- FDD for Windows: the app is now called "FDD for Windows" on the website. The download page puts the app first (plugin-only install under "Not using the app?"), and the install guide is rewritten around the app. The one-time setup is remembered; after that the app opens straight to the display and only asks you when an update is available. The setup shows the how-to video and can be reopened from Settings > App setup.
- vNAAATS app: programs that do not start by themselves after you choose a position now say why (small notice at the bottom), and are tried again next time; programs that ask Windows for administrator rights (Audio for VATSIM can) now start with the normal Windows permission prompt.
- vNAAATS app: also installs the vNAAATS font (vNAAATS.ttf) next to the plugin DLL when it is missing, so the EuroScope screen shows the right letters. The plugin pack now includes the font.
- vNAAATS app: new setup step "Audio client" (Audio for VATSIM or TrackAudio): find its program and it opens by itself when you choose your position, like EuroScope and VACS.
- CPDLC window: connected aircraft are now a list down the left (with unread counts), the window can be dragged wider or narrower (remembered), standard messages are grouped (Level / Contact / Replies), Unable / Standby / Roger / Reply are proper buttons, and Force log out moved into a "More" menu.
- Aircraft menu / filter bar: more room between the buttons at half-screen width; removed a "Default" button that did nothing (the right-hand one still resets the filters).
- Aircraft whose tracking controller has logged off now show as untracked, so the menu says "Track" and not "Update" (server).
- vNAAATS app: "Get VACS" now opens the VACS website (vacs.network).
- vNAAATS app: now uses the vNAAATS logo as its icon (program file, installer, taskbar and window).
- vNAAATS app: "Find VACS" now says what it is doing (and when nothing was chosen); "Get VACS" shows an error and the address if it cannot open your browser; messages from Update now no longer vanish.
- New "Code signing policy" page, an app section on the download page and app wording on the privacy page (needed to apply for free code signing).
- vNAAATS app: "Welcome to vNAAATS" opening animation with a short radio-style sound (jet, "Cell Cal, Alpha Bravo Charlie Golf", SELCAL tones, chime).
- vNAAATS app: EuroScope and VACS now open by themselves once you choose your position (tick boxes on the setup screen replace the Open buttons).
- vNAAATS app: natTrak and Get VACS links now open in your normal browser (they did nothing inside the app).
- Aircraft menu: back in the middle of the screen (the stacked menus had moved it to the top).
- vNAAATS app: fixed "Command status not allowed by ACL" (the app now explicitly allows its own commands for vnaaats.com pages).
- vNAAATS app: if the app does not answer, the setup screen now says why (after 15 seconds) and offers Try again / Continue anyway instead of staying on "Checking...".

## 2.5.2 - 2026-10-31
- Aircraft menu: if a menu is open and you click another aircraft in EuroScope, that aircraft's menu opens above it, so you can work on up to 3 at once (the oldest closes when a fourth is picked). Each has its own Close button.
- Aircraft menu, Track tab: you can press Track without a flight level or Mach (blank keeps what is already set; the plugin takes EuroScope's own figures). Once you are tracking the aircraft the tab and button say Update. "SELCAL test (copy)" is now "SELCAL Check" and sends .selcal to EuroScope (needs autotype=1). The plugin part needs the next plugin version.
- Aircraft menu, Supervisor tab: "Copy .wallop" also sets the aircraft's note to WALLOPED (in front of any note already there).
- Flights that filed a NAT track which is not in use today (for example NATE when there is no track E) now appear on the display as random-route (RR) flights instead of being missing. Server change only.
- Windows now open and close with a short animation: the CPDLC window slides out slowly and fades, and the other windows (settings, relief, aircraft menu, flight plan edit, position reports) fade and glide in and out.
- Close, Sign out and Change are now proper buttons (they were underlined text), including "Sign out of CPDLC" and "Close" in the CPDLC window. The CPDLC window also has a vertical Close tab on its left edge.
- Sign-in: tick "Remember me on this computer" to save your name and password (in the vNAAATS app they go in the Windows Credential Manager; in Chrome, its own password manager), then sign in with one click. "Forget the saved sign-in" removes it.
- vNAAATS app (test build): a small Windows program that opens this display and, on start-up, checks your EXOCO folder, updates an old plugin DLL, saves your key into vnaaats.cfg and starts EuroScope and VACS. The normal browser display is unchanged.
- Squawk 2000 is now given as soon as you start tracking an aircraft (plugin), not when the clearance is sent. Domestic reassign it when the aircraft is back in radar service. Aircraft you were already tracking when EuroScope started are left alone.
- vNAAATS app: the setup check also installs the current ICAO_Airlines.txt next to the plugin DLL and replaces an old one, in the same Update button as the plugin.
- Circle in EuroScope: if the circle does not appear, a yellow note next to "Circle: ON" now says why (EuroScope not answering / plugin older than 2.5.0 / aircraft not on your radar). The plugin no longer needs a flight plan to draw the circle.

## 2.5.1 - 2026-10-31
- Relief button: when another controller takes over your position, press Relief, choose who is taking over (a relief callsign works too) and confirm. Every aircraft you track is handed to them in EuroScope and they start tracking it (needs the new plugin on both sides). Aircraft on CPDLC are told "There is a controller change in progress which requires you to re log into the CPDLC stations once you receive a log off message" and are logged off once the new controller has them.
- natTrak sign-in box: now shown every time you choose a position (each controlling session), not just the first time in a browser.


## 2.5.0 - 2026-10-31
- CPDLC: a "Log off pilot" button in the drawer logs the selected pilot off CPDLC from your station. A pilot who has only just logged on no longer flashes or pings: only a request or a reply does.
- natTrak PENDING: requests that were already waiting when you opened the display (and anything arriving in the first minute) stay in the box but do not flash or ping.
- Shanwick Tango routes: the "CPDLC required" wording is gone and the routes are grouped Northbound, Southbound and Bi-directional.
- Flight Plan window: narrower, so it fits at half screen without a sideways scroll bar.
- Clicking an aircraft on the FDD circles it in white in EuroScope for 3 seconds (needs plugin 2.5.0; "Circle: ON/OFF" next to Audio, or highlight=0 in vnaaats.cfg).
- Plugin 2.5.0: after typing .contactme the plugin clicks the aircraft for you (autoclick=0 turns it off).
- Settings button: choose the plugin options (autotype, autoclick, circle, retrack, squawk, remote log) on the FDD. Your EuroScope writes them into vnaaats.cfg within about 2 minutes and tells you to close and reopen EuroScope. Needs a personal key or VATSIM sign-in.
- CPDLC: tick "Remember my code on this computer" and the Hoppie code is kept in this browser only and used to sign in automatically ("Forget the saved code" removes it).
- natTrak PENDING: requests for aircraft tracked by ENOB (Bodø Oceanic) are ignored.

## 2.4.1 - 2026-10-29
- Flight strip picked in EuroScope: a thick black ring with a white inner edge appears for about half a second, then shrinks to a thin ring and stays. It stands out on blue and yellow strips, and pulses again if you click the same aircraft again.
- Download page: new video showing how to load vNAAATS into EuroScope (profile, sector file, plugin, display, ASR file).
- Download page and guide: the sector files now link to the official AeroNav package (files.aero-nav.com/EXCXO) instead of a zip hosted here. The page also offers ICAO_Airlines.txt (updated, AIRAC 2611) and the current example config.
- Plugin: the cursor latitude/longitude readout (and the ruler/QDM tool) now use the radar window's own pixels, so the longitude is correct when EuroScope is not full screen (it showed e.g. E30 over 30W).
- Plugin: the selected aircraft is shown at the bottom left of the radar with its radio callsign, e.g. BAW227 (Speedbird), read from ICAO_Airlines.txt next to the DLL. No change to the FDD.

## 2.4.0 - 2026-10-29
- SELCAL: any four letters are accepted (VATSIM pilots are not limited to the real-world letters) and the warning about letters is gone. A SELCAL filed in the VATSIM flight plan remarks (SEL/ABCD) now fills the SELCAL box for you.
- CPDLC drawer: only the current logon is shown (never more than 6 hours); days-old messages are no longer listed.
- Server 2.4.0: a pilot tuned to a frequency that a far-away controller also uses (for example Cairo Ground on 121.900) is no longer shown as talking to that controller.

## 2.3.0 - 2026-10-29
- Server 2.3.0 gives the plugin the VATSIM-filed route of aircraft nobody has handled yet (new /PluginRoute), but only when it looks right: Europe to North America must run westbound, North America to Europe eastbound, and both ends on one side of the Atlantic is refused. Needs plugin 2.3.0. No change to the display.

## 2.2.2 - 2026-10-29
- Server 2.2.2: Discord alerts from the admin log. Set VNAAATS_ADMIN_WEBHOOK in /etc/vnaaats.env: plugin errors post to that channel with @here (grouped, repeats held back for 15 minutes), and a plain summary posts every 6 hours (VNAAATS_SUMMARY_HOURS). Warnings never ping. No change to the display.

## 2.2.1 - 2026-10-29
- Server 2.2.1 works out estimated times for every aircraft in the VATSIM feed that nobody has handled in the plugin yet (from its position, heading and ground speed, straight along its route), so the fix columns under each track are no longer blank. Times are rough (no wind) and are replaced by the plugin's own times as soon as a controller handles the aircraft. Aircraft on the ground or still climbing get none. No change to the display itself.

## 2.2.0 - 2026-10-29
- No change to the display itself. The server gains the admin log (/admin/log) fed by plugin 2.2.0.

## 2.1.0 - 2026-10-29
- Flight plan window: when an aircraft's Type shows NOFP (or is blank) the Type box is outlined in yellow and clicking it opens the whole flight plan in a window in the same layout as EuroScope's Flight plan setting dialog (flight rules, aircraft data, departure, destination, alternate, level, TAS, departure time, enroute and fuel times, route, remarks). Only the controller tracking the aircraft can save; everyone else sees it read only. Saving shows what will change and asks you to confirm, then EuroScope sends the amended plan to the pilot. Needs plugin 2.1.0.

## 2.0.5 - 2026-10-29
- Flight Plan panel: the plugin's "AIRCRAFT" placeholder (present position, when a flight has no route) is no longer shown as a route fix, so it no longer triggers "AIRCRAFT is not a fix name".

## 2.0.4 - 2026-10-28
- After you send TRACK (or release / transfer) from the aircraft menu, the strip list now refreshes again 3 and 6 seconds later, so the headset icon and tracker update without clicking a second time.

## 2.0.4 - 2026-10-28
- After you send TRACK (or release / transfer) from the aircraft menu, the strip list now refreshes again 3 and 6 seconds later, so the headset icon and tracker update without clicking a second time.
- Download page: the sector pack, guide PDF and intro video links no longer show "Not found" when clicked (the page was treating them as page links); they now download directly.

## 2.0.3 - 2026-10-27
- Removed the "N min from <callsign>" warning (same track, same level, entering within 10 minutes). It compared estimated entry times only, so it could flag aircraft that were miles apart; judging separation is the controller's job. The "level is not one of the levels for track X" warning stays.
- Flight Plan panel: a warning (for example "FL330 is not one of the levels for track V") is now a solid yellow banner at the top of the panel instead of small text at the bottom, and the panel grows to fit instead of scrolling.
- Flight Plan panel: Datalink moved to the Spd / FL row so the panel fits without side scrolling, and it now reads "Logged on" (green) or "Not logged on" (faded red) instead of "CPDLC on" / "No CPDLC".
- Filter boxes are now TRKD, EB, WB, RR and TANGO (HL / LL and the ALL TK / RR buttons are gone). RR shows random routings only and TANGO the Shanwick Tango routes only; ticking both shows both kinds.
- Shanwick Tango routes (T9, T290, T213, T13, T16) are now on the list in one group, TANGO ROUTES (each strip shows its route name and fixes), below the NAT tracks (and Concorde tracks) and above RANDOM. A flight that files one of these routes (the route name with one of its fixes, or both of its fixes) appears on the group automatically; they are also listed in the Tracks drawer under "Shanwick Tango routes" with their rules: T9 southbound only, even levels FL300-400; T290 northbound only, odd levels FL290-410; T213 both ways; T13 northbound; T16 southbound. A level that does not belong to the route (T9 and T290) shows the usual "not one of the levels" warning.
- Northbound Tango flights count as EB and southbound ones as WB for the EB / WB filter.
- Tango flights are Shanwick's: a Gander (CZQO_) position does not list them unless it is tracking them. The 30W neighbour rule does not apply to them.

## 2.0.2 - 2026-10-26
- Flights that are handed off (an oceanic controller stopped tracking) or past their exit point now stay on the list for 15 minutes, drawn faded.
- If you are signed in as a Gander (CZQO_) or Shanwick (EGGX_) position, flights of the other OCA that are 30 minutes or less from 30W are also listed, as normal. Flights that have crossed 30W to the neighbour stay 15 minutes, faded. Other positions (NAT_FSS ...) see everything as before.
- CPDLC: "Force log out a CPDLC callsign..." in the CPDLC drawer. Two warning steps (type the callsign, then type LOG OUT); the station is cleared and nobody can sign in to it for 3 minutes.
- Auto RCL (server 2.0.1): never sent to aircraft already inside or through the oceanic area; waits 5 minutes after an oceanic controller comes online; flights already at/in the OCA at that moment are never reminded.

## 2.0.1 - 2026-10-26
- New filter boxes replace PST / FaD / WST / TERM: TRKD (flights you are tracking at your position), EB, WB, HL (above FL340) and LL (FL340 and below). Nothing ticked shows everything; the groups (tracked, direction, level) narrow each other and within a group either box counts. The HL / LL buttons are gone (they are now boxes); ALL TK and RR stay.

## 2.0.0 - 2026-10-26
- The ready-made sector pack (Gander sector files, plugin 2.0.0, vNAAATS radar screen and profile) can be downloaded from /download and is mentioned in the guide.
- First full release of vNAAATS V2 (server 2.0.0, plugin 2.0.0). The display, server and plugin now share the version number 2.0.0; no features changed from 0.20.0 / 0.21.0 / 1.5b23.

## 0.20.0 - 2026-10-26
- Concorde tracks (server 0.21.0): SM (westbound), SN (eastbound), SO (overflow) and SP (Caribbean) are listed in a "Supersonic (Concorde)" group in the track panel, from the Gander Oceanic SOPs (Annex B 1.1.2). A Concorde that files their points (SM20W, SN30W ...) or the same coordinates is put on its track and in its own group. They are web display only: EuroScope is not sent them.
- Block level: flights on a Concorde track show the block level (B470-600) instead of a single level on the strip, and "Block FL470-600" in the aircraft menu and track panel.
- Half-degree points are no longer shown as whole degrees: 5230/30 and 5230N03000W stay 5230/30 (52 degrees 30 minutes). ARINC 424 forms N5020 (50 30N 020W) and 5020N (50N 020W) are understood too.

## 0.19.0 - 2026-10-25
- Data protection (server 0.20.0): view-only mode ("Just watch" and the Observer position) is gone. The whole page is covered until you have signed in and chosen a position, and the server sends no flight, natTrak or network traffic data to anyone who has not. Signing out or choosing "change" clears the screen. If a sign-in runs out the sign-in screen comes back.
- The download page now has the intro video and the user guide as a PDF. Guide and manual updated: key and sign-in come from ganderoceanic.ca/my/vnaaats, new aircraft menu order.
- Server: video and PDF files are served with byte ranges so they play on phones.

## 0.18.7 - 2026-10-24
- "Beta" wording removed ahead of v2.0: the header says "Version", the plugin download page is now /download (the old /beta address redirects there so older plugins still work), and the homepage button and CPDLC tile no longer say beta.

## 0.18.6 - 2026-10-24
- Aircraft menu: buttons reordered (Track, Transfer, Release Tag, SELCAL, CPDLC, Note, Contact Me, RCL REQ, Hide, then Supervisor). "Untrack" is now "Release Tag". The track ID the aircraft is on shows in the menu heading. On the Track tab the Track aircraft button is on the left and the SELCAL test button sits under the SELCAL box.

## 0.18.5 - 2026-10-24
- iPhone and iPad: "Add to Home Screen" now gives the vNAAATS icon and name and opens full screen like an app (Safari: Share, then Add to Home Screen).

## 0.18.4 - 2026-10-24
- Server 0.19.6: the shared beta key and the shared FDD password can now be switched off (VNAAATS_SHARED_KEY=off, VNAAATS_SHARED_PASSWORD=off in the server settings). Personal keys and passwords keep working, and the sign-in screen is still shown when only personal passwords exist.

## 0.18.3 - 2026-10-24
- Personal keys (server 0.19.5): a key is available to anyone who is certified (endorsed) on the Gander Oceanic roster. Active status no longer matters.

## 0.18.2 - 2026-10-24
- Automatic RCL REQ reminder (server 0.19.4): only sent while the flight is 20 to 45 minutes from its oceanic entry point. Natrak refuses a new request closer than 20 minutes, and a flight already past its entry point (or through oceanic airspace) needs none. With no times on the flight, the distance and speed are used for the same 20-minute rule.

## 0.18.1 - 2026-10-23
- Contact me: the time it was sent is shown in the Flight Plan panel ("Contact me sent at 1854z (12 min ago)", red once 20 minutes pass with no track) and in the strip's tooltip.

## 0.18.0 - 2026-10-23
- The FDD can be installed as an app (server 0.19.3): in Chrome or Edge use the install icon in the address bar. It opens in its own window with its own icon. Nothing is stored for offline use, so it is always the newest version.

## 0.17.5 - 2026-10-23
- Privacy page: contact email info@vnaaats.com, why data is used, how long it is kept, where the server is, and your rights (GDPR-style, for everyone).

## 0.17.4 - 2026-10-23
- Privacy page: says the roster check covers the oceanic controllers of Gander Oceanic, VATSIM UK and Virtual New York ARTCC (all listed on the Gander roster).

## 0.17.3 - 2026-10-23
- The privacy page (/privacy) is now a proper page of the site. Before, opening it from a link inside the site gave "Not found: /privacy". It now also describes personal keys, the roster check and the re-track memory.

## 0.17.2 - 2026-10-23
- Re-track after a crash (server 0.19.2, plugin 1.5b20): only if you are back within 5 minutes, and not if someone with a different name has signed in on the FDD at your position.

## 0.17.1 - 2026-10-23
- Server 0.19.0 + plugin 1.5b19: if EuroScope crashes or you lose your connection, the plugin tracks your aircraft again about 20 seconds after you reconnect (within 5 minutes, never an aircraft another controller has taken, and not if a different name has signed in on the FDD at your position). retrack=0 in vnaaats.cfg turns it off.

## 0.17.0 - 2026-10-23
- Contact me sent from the FDD (server 0.18.0): the aircraft's callsign cell turns yellow. If 20 minutes later no oceanic controller is tracking it and it is still in oceanic airspace (by its route times), it turns red. It clears as soon as an oceanic controller tracks the flight (or after 4 hours).

## 0.16.0 - 2026-10-23
- Click an aircraft in EuroScope and the FDD follows: its strip is outlined in yellow, shown on the list even if the time window, a filter, the search box or Hide would have kept it off, scrolled to the middle and becomes the selected aircraft (Flight Plan panel). Needs plugin 1.5b18 and server 0.17.0, and you must be signed in to the FDD with the same position callsign as your EuroScope.

## 0.15.0 - 2026-10-23
- NAT track in the filed route (server 0.16.0): a route such as "... HOIST NATF GOMUP ..." is now checked against today's tracks. NATF is replaced by track F's fixes between the filed entry and exit fix (or the whole track if only one end is given), the strip gets the oceanic coordinates and sits under that track. A NAT letter with no active track is dropped. Applies to flights from the VATSIM feed, flights added by callsign, and flights sent by the plugin.

## 0.14.2 - 2026-10-23
- Server 0.15.1: the server now reads the Gander Oceanic roster feed (ganderoceanic.ca/api/roster) itself. A personal key is only issued to a CID that is certified and active, and every 6 hours keys of members who have dropped off are withdrawn. A broken or empty feed never withdraws anything.

## 0.14.1 - 2026-10-23
- Personal keys and passwords (server 0.15.0): each certified, active controller requests their own plugin key and FDD password from their myCZQO page on ganderoceanic.ca ("vNAAATS key"). Keys last 90 days and stop working if the controller leaves the roster. Sign in on the FDD with your name or CID and that password. The shared key and password still work for now.

## 0.14.0 - 2026-10-23
- Automatic RCL REQ reminder (server 0.14.0, on by default): a flight with no natTrak request, within 45 minutes of its oceanic entry estimate or within 200 nm of a fix on its route, is sent the RCL REQ message automatically, typed in by an online oceanic controller's EuroScope (needs plugin 1.5b13+ and autotype=1). Retries after 5 minutes (3 tries). "Auto RCL: ON/OFF" in the controls bar switches it for everyone.
- New "Note" tab in the aircraft menu to set the scratchpad note. The flight plan panel scrolls when the window is short, and its Note box is labelled plainly "Note".
- The example config now has autotype=1.

## 0.13.1 - 2026-10-23
- CPDLC: signing in no longer brings up old messages that were waiting at Hoppie. Unread now counts only aircraft that are logged on, and starts at zero for each new sign-in. Sending a reply marks that aircraft's messages as read, so the flashing stops. Needs server 0.13.1.
- After you pick a position for the first time in a browser, a message explains the one-off natTrak sign-in and opens nattrak.vatsim.net in a new tab, where you press Sign in yourself.

## 0.13.0 - 2026-10-23
- New user manual at /manual: every feature of the display and the plugin, with a contents list and a step-by-step shift. Linked from the install guide and the download page.

## 0.12.0 - 2026-10-23
- Note (scratchpad): a "Note" box on the flight plan panel (up to 15 characters). It is the EuroScope scratchpad, so it shows under the tag in EuroScope and goes with the aircraft to the next controller. Notes typed in EuroScope show here too.
- CPDLC: when a pilot logs on, the alert sound plays once, their callsign flashes in the CPDLC window and on the strip, and the CPDLC button flashes ("new logon") until you look at that aircraft.
- Contact Me: EuroScope's .contactme takes no callsign (it works on the aircraft you click), so Send now selects the aircraft in EuroScope first. Copy gives plain ".contactme".
- Track now also accepts a handoff that is being offered to you (needs plugin 1.5b15).

## 0.11.4 - 2026-10-22
- Two downloads: the download page now gives just the plugin DLL for people updating; the install guide has the full first-time pack (zip).

## 0.11.3 - 2026-10-22
- New install guide at /guide (linked from the download page), with the AeroNav Gander package link and a download button for the newest plugin pack.

## 0.11.2 - 2026-10-22
- The aircraft menu "NatTrak" tab is now "RCL REQ" (the clearance-request reminder message).

## 0.11.1 - 2026-10-22
- Flight plan panel: ACID and Datalink boxes narrower so the ATC box is not cut off.

## 0.11.0 - 2026-10-22
- "Remove" is now "Hide": it hides the strip for you only (this browser, 12 hours). Nobody else's display changes. Bring a strip back from the "Hidden" list in the top bar or by typing its callsign in Add callsign.
- Sector and State boxes merged into one "ATC" box: the callsign of the controller tracking the flight (green), or who the pilot is tuned to on VATSIM (oceanic green, domestic pale green), red UNICOM, or red NONE.

## 0.10.2 - 2026-10-22
- The "Entry time" box is now labelled "Entry".

## 0.10.1 - 2026-10-22
- Flight plan panel: Dest now sits next to Depart; "Etd" replaced by the editable "Entry time" (time at the oceanic entry fix); SELCAL moved to the Route row beside the route box.
- State box now shows CTL when an ATC position is tracking the flight, "--" otherwise (it used to show a fixed "UC").

## 0.10.0 - 2026-10-22
- Remove a strip: aircraft menu > Remove, with a warning first. The strip stays hidden for 12 hours unless someone adds it again.
- Add a strip: type a callsign in "Add callsign" (top bar) and press Add. The flight is looked up on VATSIM and stays on the display.
- Entry time: edit the "Entry" box in the flight plan panel and all the other fix times move by the same amount.

## 0.9.10 - 2026-10-22
- Flight plan panel: removed the blue callsign box and the blue route/times list (the Route (edit) box now uses the space).
- "Datalink" now means CPDLC: green "CPDLC on" when the pilot is logged on to the vNAAATS CPDLC station, red "No CPDLC" otherwise (it used to say Connected when a natTrak request existed).

## 0.9.9 - 2026-10-21
- A flight tracked by an oceanic controller (CZQO, EGGX, NAT_FSS ...) is never hidden by the traffic window: its strip stays until it is untracked and for 5 minutes afterwards. (The 5 minutes is counted by the open browser page, so a page refresh restarts the count.)

## 0.9.8 - 2026-10-21
- Headset icon: aircraft that are online but not on any controller or UNICOM now show a red crossed-out headset ("Not tracked by any controller").
- Transfer list only offers the stations in the TopSky list (topsky_cpdlc.txt), not every controller online. Needs server 0.11.1. With none online the Transfer tab says "No adjacent ATC - Send to UNICOM".

## 0.9.7 - 2026-10-21
- Headset icon now shows who the pilot is talking to, taken from the VATSIM radio frequency they have tuned: bold green = oceanic controller, faded green = domestic controller, orange = UNICOM 122.800. Hover shows e.g. "Tracked by NAT_FSS (133.500)". If VATSIM has no radio info it falls back to who is tracking the aircraft in EuroScope. Needs server 0.11.0.

## 0.9.6 - 2026-10-21
- "Group" now also has "by flight level": flights are grouped under FL400, FL380, ... highest first, each heading showing how many aircraft.

## 0.9.5 - 2026-10-21
- Tracks panel: the validity time is shown once on the heading ("Westbound 1130-1900z") instead of on every track. Each track is two lines: its name and fixes, then its flight levels. (Also fixed minutes 09 being shown as "9".)

## 0.9.4 - 2026-10-21
- Strip list rebuilt on one grid shared by every strip and header row: all fix times (including the exit fix) always show and line up under their fix names, at any window width. The columns scale with the list text, and the dead space between callsign / route / level is gone.

## 0.9.3 - 2026-10-21
- The tracking controller is now a headset icon (hover: "Tracked by NAT_FSS"). Flights cleared on natTrak show a clipboard-tick icon instead of the word NATTRAK (hover: "RCL processed at 1613z (natTrak)"; natTrak does not currently say which controller processed it, so the name only shows if natTrak starts sending it).
- Aircraft type and the tracking icon have their own space, so they no longer overlap; long rows are clipped instead of running into the right-hand column.

## 0.9.2 - 2026-10-21
- SELCAL is checked against the 16 SELCAL letters (A-H, J-M, P-S). A red warning in the menu says which letters are not allowed, e.g. "SELCAL cannot use W".

## 0.9.0 - 2026-10-21
- "Send" buttons for SELCAL test, Contact me and the natTrak message: the plugin (1.5b4, with `autotype=1` in vnaaats.cfg) brings EuroScope to the front and types the command into its command line. If that is switched off or Windows refuses, the command is copied instead and the menu says so. Supervisor stays copy-and-paste.

## 0.8.0 - 2026-10-21
- NatTrak tab in the aircraft menu: copies the EuroScope message (.msg CALLSIGN Hello, you will soon be entering oceanic airspace...) and "I have sent it - mark with red *" puts a red * next to the callsign on the strip (and, with plugin 1.5b3, on the EuroScope tag). The * clears by itself when the pilot requests clearance on natTrak or an oceanic controller (CZQO, CZQX, EGGX, NAT, BIRD, KZWY, LPPO, ENOB) tracks the flight. "Remove mark" clears it by hand.

## 0.7.5 - 2026-10-21
- Strip columns now have fixed widths (departure/arrival, level, Mach, each ETA) and the track header row uses the same widths, so level/Mach and the fix names no longer run into each other and the times line up under their fixes. The tracking controller (e.g. NAT_FSS) is no longer cut off.

## 0.7.4 - 2026-10-21
- Flights whose pilot is no longer on the VATSIM network are hidden (when the VATSIM feed is working). The selected flight, CPDLC-connected flights and pending natTrak requests still show. If the feed is unavailable, nothing is hidden for this reason.

## 0.7.3 - 2026-10-21
- Flights with no times at all are now judged by where the pilot is (live VATSIM position): they only show when within 200 nm of a fix on their route. If the pilot's position is not known they still show. (Server 0.9.1 sends the distance.)

## 0.7.2 - 2026-10-21
- Route can be amended in the flight plan panel ("Route (edit)"). Latitude/longitude fixes must be two numbers / two numbers (51/20); 51N20W and similar are refused with a red warning as you type. Fixes you keep keep their times; new fixes have none. The change is on the display and server only - EuroScope's own flight plan is not changed.

## 0.7.1 - 2026-10-21
- Traffic window: by default the strip list only shows flights within 60 minutes of their oceanic entry estimate, until 5 minutes after their exit estimate. A "Show" box in the top bar switches to all flights (remembered in your browser) and a count shows how many are hidden. Flights with no times, the selected flight, CPDLC-connected flights and pending natTrak requests always show.

## 0.7.0 - 2026-10-21
- Click a callsign for the aircraft menu: Track (level, Mach, SELCAL), Untrack (asks to confirm), Transfer (online controllers), SELCAL, Contact me, Supervisor request (with a "No Contact" quick text), CPDLC, and Position reports (Concorde only).
- Track, Untrack and Transfer are sent to your EuroScope (plugin 1.5b1 or newer) and the result is shown in the menu.
- SELCAL test, Contact me and Supervisor copy the EuroScope command (.selcal / .contactme / .wallop) to paste into the command line, because plugins cannot run those themselves.

## 0.6.0 - 2026-10-21
- Concorde (type CONC) position reports: a "Position reports" button in the flight plan panel opens a table of every route fix with Calc, Est and Actual times (four digits, typed in by the controller; "now" fills the current time). Typed times replace the calculated times on the strip.
- A Concorde flashes (strip, flight plan button, red "overdue" count in the top bar, and the row in the table) when the next fix's time passed more than 3 minutes ago with no reported time.
- Mach is shown with three digits (M084, M200) so Concorde's Mach 2.00 displays correctly; the server now accepts Mach up to 2.99.

## 0.5.0 - 2026-10-21
- CPDLC sound for messages from pilots (your own sound file), instead of the beep.
- "Standard messages..." list in the CPDLC drawer (the TopSky list) and a UNICOM button (asks to confirm): sends "THERE IS NO FURTHER ATC AVAILABLE. SERVICE TERMINATED. MONITOR ADVISORY ON 122.800", logs the pilot off when they answer WILCO/ROGER and releases the tag in EuroScope.

## 0.4.0 - 2026-10-21
- New page /beta: always links the newest plugin beta download, with install steps. Homepage has a "Download plugin beta" button.
- CPDLC: aircraft connected to CPDLC show their callsign in brackets, e.g. [BAW123], on the strip and in the flight plan panel.
- New CPDLC drawer (button in the info bar, with unread count): connected aircraft, message thread, quick replies (UNABLE / STANDBY / ROGER), templates, send. Sound alert on new messages. Needs a controller position to send.
- Each controller signs in to CPDLC with their own Hoppie code (typed in the drawer, kept in server memory only). Stations: CZQO = CZQX, EGGX = EGGX, NAT_FSS = NATX.

## 0.3.1 - 2026-10-21
- Homepage: "Version 2 - Modernised by Gary Thomas" credit, copyright line updated (original (c) Andrew Ogden 2020-2021 kept, Version 2 (c) Gary Thomas 2026), Discord button removed, plugin box links to the Version 2 project, new "Dedicated CPDLC Interaction - coming soon" box.

## 0.3.0 - 2026-10-20
- Controller login (password now, VATSIM login when switched on) and a position screen (CZQO_CTR, EGGX_CTR ...). "Just watch" still works with no login.
- Level, Mach, track and SELCAL can be edited in the flight plan panel; the change is saved on the server and picked up by the plugin, including the open flight plan window.
- "Last change HHMMz by POSITION (name)" in the flight plan panel and a pencil on strips changed in the last 10 minutes.
- Warnings: same track + level entering within 10 minutes, or a level that is not on the track.
- Search box, group by track or entry fix, sort by ETA / callsign / level / last changed.
- Header shows both the display version and the server version.

## 0.2.2 - 2026-10-20
- Every route waypoint is shown as four digits, 52/50, whatever format the flight plan used (52N050W, 5230N05000W, 4545N ...); minutes are dropped (4730N05000W shows as 47/50). Random-route strips are now a single line.

## 0.2.1 - 2026-10-20
- natTrak alerts can be switched on/off separately: Visual (flashing, tab title) and Audio.
- Choice of alert sound: Ping, Chime or Beep (click Tone to cycle; plays a preview).
- Clicking a pending request (box row or its flashing strip) opens natTrak in a small pop-up window on that track's pending list.

## 0.2.0 - 2026-10-20
- Runs against the own vNAAATS server (old api.vnaaats.net backend is gone).
- Fixed "No data on remote" never clearing, and the wrong "updated recently" check.
- Strip text scales with the window width (no more overlapping columns on narrow windows).
- Working HL / LL / ALL TK / RR filter buttons and Default reset.
- Working PST / FaD / WST / TERM checkboxes (interpretation shown as tooltips).
- Oceanic-bound flights from the public VATSIM data feed are shown alongside plugin flights.
- natTrak Information panel connected to the public natTrak API.
- natTrak request box (PENDING / CLEARED), flashing strips and a ping sound for new pending requests.

## 0.1 - original
- vNAAATS-web as published by the vNAAATS team (Apache-2.0).
