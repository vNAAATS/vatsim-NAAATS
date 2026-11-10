# vNAAATS server
Backend for the vNAAATS EuroScope plugin and the flight data display (`../web`). Python 3.8+, no packages needed.
Endpoints and settings: see the header of `vnaaats_server.py`.
Tests: `python3 -m unittest discover -s tests -v`
Install on Ubuntu: `bash deploy/setup.sh [domain]` (put the built `web` folder next to `deploy/` to serve the display).
Missing fix coordinates: see `/unresolved`, add them on the admin page `/admin/addfix` (uses the API key) or in `extra_fixes.json`.
The API key is generated on the server (`/etc/vnaaats.env`) and is never stored in this repository.


## Network traffic (other controllers' / all oceanic aircraft)

The server also reads the public VATSIM data feed (every `VNAAATS_FEED_SECONDS`, default 60) and keeps every pilot whose
filed route has North Atlantic coordinate waypoints (e.g. `52/50`, `5730/30`, `52N050W`). They are served at
`/GetNetworkTraffic` and the flight data display merges them in (flights posted through the plugin win over the same callsign).
This is filed flight plan data only - it contains no controller clearances, levels assigned, or estimates (those only exist when a
controller posts through the plugin). Set `VNAAATS_VATSIM_FEED=off` to turn it off.

## natTrak clearance data

`/GetNatTrakData` serves the public natTrak API (`https://nattrak.vatsim.net/api/plugins`, refreshed every `VNAAATS_FEED_SECONDS`):
for each callsign the status (CLEARED / PENDING), track, entry fix, level, Mach, estimate, clearance time and extra info.
The flight data display shows it in the "natTrak Information" panel. For flights found in the VATSIM feed, a CLEARED natTrak
clearance also fills in the level, Mach and track on the strip. Set `VNAAATS_NATTRAK_PLUGIN_URL=off` to disable.
(The natTrak controller pages `/controllers/clx/...` need a VATSIM login and are not used.)

## Web editing, login and positions (server 0.4.0)

The flight data display can now change level, Mach, track and SELCAL. Only signed-in controllers with a position (not OBSERVER) can.
Reading (viewing the FDD) stays open. The plugin keeps using the API key for its own saves.

- **Password login** (works straight away): `VNAAATS_FDD_PASSWORD` in `/etc/vnaaats.env`. `deploy/setup.sh` creates one the first time and prints it. Five wrong tries from one address lock that address out for 5 minutes.
- **Which callsigns count:** the rules go by the start of the callsign, so extra and relief callsigns get the same access as the main position, however many people are already signed in: `NAT_I_FSS` (instructor) and the relief `NAT__FSS` behave as `NAT_FSS`; `CZQO_A_CTR` to `CZQO_L_CTR` and `_DEL`, and relief `CZQO__CTR`, as Gander; `EGGX_A_CTR` to `EGGX_L_CTR` and `_DEL`, and relief `EGGX__CTR`, as Shanwick. (CPDLC is the exception: one logon per station.)
- **Positions** on the opening screen: `VNAAATS_POSITIONS=CZQO_CTR,CZQO_DEL,EGGX_CTR,...` (OBSERVER = view only). Anyone can also type another position.
- **VATSIM login** (needs HTTPS and a VATSIM Connect client; see below): `VNAAATS_CONNECT_CLIENT_ID`, `VNAAATS_CONNECT_CLIENT_SECRET`, `VNAAATS_PUBLIC_URL=https://your.domain`. Optional `VNAAATS_ALLOWED_CIDS=1234567,7654321` (only these CIDs) or `VNAAATS_MIN_RATING` (default 2 = S1 and above). For the VATSIM sandbox set `VNAAATS_CONNECT_BASE=https://auth-dev.vatsim.net`.
  Register the app at https://auth.vatsim.net (production needs an approved organisation); the redirect URL to register is `https://your.domain/auth/callback`.
- Each flight remembers `updatedBy` / `updatedAt` / the last 10 changes (`history`). A web edit is held until the plugin has fetched it, so a plugin that has not yet polled cannot overwrite it with an old value.

## CPDLC (server 0.6.0)

The server acts as a CPDLC ground station through Hoppie's ACARS. Nothing to set on the server: each controller types **their own Hoppie logon code** in the FDD's CPDLC drawer after choosing their position.
The code is held in server memory only while that controller is signed in (sign out, change position, session expiry or a server restart forgets it). It is never written to disk or logged.

Stations (position -> station callsign pilots log on to): `CZQO_*` -> **CZQX**, `EGGX_*` -> **EGGX**, `NAT_*` (NAT_FSS) -> **NATX**. `*_DEL` (delivery) positions and OBSERVER do not use CPDLC. Override with `VNAAATS_CPDLC_STATIONS=CZQO_CTR=CZQX,EGGX_CTR=EGGX,NAT_FSS=NATX` (then only those positions have a station).
Other settings: `VNAAATS_CPDLC=off` (disable), `VNAAATS_CPDLC_AUTOACCEPT` (default 1: logon requests accepted automatically), `VNAAATS_HOPPIE_URL`.

How it works: while a controller is signed in, the server polls Hoppie for that station (about every minute, faster after traffic), accepts `REQUEST LOGON`, stores messages, and pings connected aircraft every 5 minutes (dropping them after two misses). Connected aircraft get `"cpdlc": true` in the flight data, so the plugin shows `[BAW123]` and the FDD shows brackets plus the drawer. Only one controller can hold a station at a time (Hoppie allows one logon per callsign). Pilots connected to a station must log on again after it has been stopped.

Automatic behaviour (while a station is signed in):
- **Logon rule:** a pilot within 1000 nm of any active NAT track waypoint is accepted automatically. Further away they get "UNABLE. CONTACT CONTROLLER ON VOICE 20 MILES BEFORE OCEANIC ENTRY POINT" and are not connected. If the pilot is not on the VATSIM data feed yet, or there are no tracks, they are accepted.
- **Oceanic clearance requests** (a downlink mentioning an oceanic clearance) are answered with "UNABLE TO PROVIDE OCEANIC CLEARANCES VIA CPDLC. PLEASE VISIT NATTRAK.VATSIM.NET AND SUBMIT YOUR REQUEST THERE" and do not alert the controller.
- **Handoff** (started in the EuroScope flight plan window): the plugin calls `/CpdlcHandoff`; if the aircraft is connected to a running station the pilot is sent "CONTACT <full station name> ON <frequency>, BYE." (frequency from EuroScope, name from `topsky_cpdlc.txt`). When the pilot answers WILCO or ROGER the aircraft is logged off CPDLC. UNABLE or STANDBY leave it connected.
- **UNICOM** (FDD drawer button): sends the UNICOM message; on WILCO/ROGER the pilot is logged off and the flight record carries `cpdlcRelease: true` for 3 minutes so the plugin releases the tag.
- `topsky_cpdlc.txt` holds the station names and the "Standard messages" list (edit it and restart to change them).

Cautions:
- The logoff uplink text (`LOGOFF`, in `cpdlc.py`) is a best guess: not confirmed against the live Hoppie network. The aircraft is dropped from our connected list either way.
- Passing CPDLC on to the next controller's station is not automatic: the pilot logs on to the next station themselves.
- Polling removes messages from Hoppie's queue, so nothing else (TopSky, another client) may be logged on as the same station callsign.
- LOGOFF handling and exact reply behaviour have not yet been checked against the live Hoppie system.

## Concorde position reports (server 0.8.0)

Concorde (aircraft type starting `CONC`) reports by voice, so controllers type its times in on the FDD. `POST /EditPosReport {callsign, fix, est, act}` (signed in, not OBSERVER) stores an estimate and/or actual time (four digits; both empty clears it) per route fix, with who entered it. `posReports` is added to a CONC flight's data; the FDD shows the typed times in place of the calculated ones and flashes when a fix is more than 3 minutes overdue. Reports are kept for 24 hours. The plugin does not show these times yet.
