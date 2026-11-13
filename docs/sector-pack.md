# Sector pack auto-update (FDD for Windows)

After each AeroNav push, publish the same vNAAATS zip here so the app can install it for controllers (AeroNav downloads need a login, so the app cannot fetch from there):

1. GitHub repo > Releases > open the release tagged `sector-pack` (create it once: tag `sector-pack`, mark it **pre-release** so it is never "latest", because the server reads the latest release for the plugin DLL).
2. Delete the old zip, then attach the new one (the `EXCXO-vNAAATS` zip from AeroNav). Replacing the file is what tells the app there is an update.
3. Next time a controller opens the FDD for Windows, it offers "A newer sector pack". Update now unpacks the zip over their EXOCO folder, only while EuroScope is closed, and never touches `vnaaats.cfg`, and never overwrites a controller's own `.prf` profile, `Screen Layout.txt`, `VCCS.txt` or `VoiceChannels.txt` (those are only installed if missing).

The zip must have the same folder layout the controllers already unzip into EXOCO by hand.

What the app does with the zip (checked against the real EXCXO-vNAAATS zip):
- It unpacks into the folder that holds the `EXCXO` folder. If the controller chose the `EXCXO` folder itself, it steps up one level, so there is no `EXCXO/EXCXO`.
- `VatsimNAAATS.dll` in the zip is skipped: the plugin is updated by its own check, and the pack's copy may be older than the latest release. `vnaaats.cfg` is skipped too.
- An existing `.prf` keeps the controller's own details; only its `Settings sector` line is pointed at the new `.sct`. A `.prf` with a new name (for example "OCA vNAATTS.prf" replacing "OCA vNAATTS Beta.prf") is installed as a new profile, so the controller enters their details again.
- Older builds of the same sector files (same name up to the first `_`) are deleted after a successful update.
