# vNAAATS: VATSIM North Atlantic Advanced Air Traffic System
![vNAAATS Image](https://i.imgur.com/452ZeM2.png)

vNAAATS is the oceanic control toolset for the VATSIM North Atlantic: a EuroScope plugin plus a live web **Flight Data Display (FDD)**, built for Gander Oceanic (CZQO) and Shanwick (EGGX) controllers. It simulates the GAATS+ HMI used by oceanic controllers, with track drawing, clearance (RCL) workflows, CPDLC and a full conflict toolset.

This is version 2, revived from Andrew Ogden's original vNAAATS with his permission. It is an independent community project, not run or endorsed by VATSIM, for flight simulation only.

## What is in it
- **Flight Data Display** at https://vnaaats.com/fdd: flights grouped by NAT track, edit level, Mach and track, SELCAL and notes, natTrak clearance requests, Auto RCL, CPDLC, and a handoff helper for busy periods. Works on tablets and installs as an app. See the [icon key](https://vnaaats.com/icons) ([PDF](https://vnaaats.com/vNAAATS-icon-key.pdf)) for what every symbol and colour means.
- **FDD for Windows:** an app around the display that also looks after the controller's computer. When it starts it checks the plugin, airline list, sector pack and API key, installs updates (EuroScope closed), and only then asks "Ready to open EuroScope?". It never overwrites the controller's own profile, key or layout.
- **EuroScope plugin:** tracking, handoffs and transfers from the web display, re-tracking after a crash, conflict probe, natTrak and CPDLC integration.
- **Concorde:** the four historic tracks SM, SN, SO and SP, block level, voice position reports and overdue alerts.
- **Server:** the backend for the plugin and display (flight data, natTrak data, CPDLC through Hoppie's network, sign-in and keys).
- **Sign-in and data protection:** the display is covered until you sign in with a position; there is no view-only mode. Controllers get a personal API key at https://vnaaats.com/access.

### Plugin tools (carried over from the original)
- Custom drawn oceanic tags with mode and status indicators; smart traffic filtering by area and altitude
- NAT track overlays and overview window; route drawing that interpolates NAT tracks
- Lists: Inbound (showing direction of flight), Other, Conflicts
- Sep, PIV (Path Intercept Vector), RBL (Range Bearing Line), Halo and PTL tools, line ruler, grid display
- Flight plan window, probe tool to detect conflicts down a track before an oceanic clearance, controller notepad
- Recognition of aircraft equipment to show ADS-B procedure compatibility
- ... and more

## Get started
- Get your key: https://vnaaats.com/access
- Install guide: https://vnaaats.com/guide (first time) and https://vnaaats.com/download
- How-to videos and training: https://vnaaats.com/training and https://vnaaats.com/videos
- Manual: https://vnaaats.com/manual

**Updating the plugin by hand:** close EuroScope and replace **only** `VatsimNAAATS.dll` (keep your `vnaaats.cfg`). The FDD for Windows app does this for you.

## Releases
- `VatsimNAAATS.dll` is the plugin; `VatsimNAAATS-plugin.zip` holds the plugin, readme, example config and radar screen file.
- `vnaaats-server.tar.gz` is the server and built display (for whoever runs the server).
- The FDD for Windows installer is published as the `fdd-windows` release.
- The sector pack for the app's auto-update lives on the `sector-pack` pre-release (see [docs/sector-pack.md](docs/sector-pack.md)).

## For maintainers
| Folder | What it is | Check it |
| --- | --- | --- |
| `VatsimNAAATS/` | EuroScope plugin (C++, built by GitHub Actions) | Build plugin workflow |
| `web/` | Website and Flight Data Display (SvelteKit) | `npm run build`, `npx svelte-check` |
| `server/` | Backend (Python 3.8+, no packages) | `python3 -m unittest discover -s tests` |
| `launcher/` | FDD for Windows app (Rust, Tauri) | `cargo test -p vnaaats-core` |

Pushing `master` to the `server-and-squawk` branch builds the plugin, the display, the server package and the Windows app. To update the server, download `vnaaats-server.tar.gz` from the `server-latest` release and run `bash deploy/setup.sh vnaaats.com` on the server (see [server/README.md](server/README.md)). `web/CHANGELOG.md` records display changes; `RELEASE-NOTES.md` is the release summary.
