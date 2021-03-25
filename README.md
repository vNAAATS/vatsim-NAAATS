# VATSIM North Atlantic Advanced Air Traffic System
![vNAAATS Image](https://i.imgur.com/452ZeM2.png)

vNAAATS is a plugin designed to revolutionise day-to-day oceanic operations on the VATSIM network. It simulates the GAATS+ HMI (Gander Automated Air Traffic System +) used by Oceanic controllers at Gander and Prestwick (Shanwick). The plugin supports track drawing, full clearance workflows, and a full conflict toolset for Oceanic controllers. In addition, it supports a CPDLC-esque exchange of data via datalink through natTrak.

### Main Features
- Custom drawn oceanic tags with detailed mode and status indicators
- Smart traffic filtering by area and altitude; oceanic controllers only see the aircraft relevant to them
- NAT Track overlays and overview window
- Smart route drawing function, interpolating NAT tracks as the aircraft's route if they are flying one
- Lists: Inbound (showing direction of flight), Other, Conflicts
- Sep tool showing potential conflicts for two aircraft presuming continuance on current heading
- PIV (Path Intercept Vector) tool showing potential conflicts along two aircraft routes
- RBL (Range Bearing Line) showing distance between two aircraft and indicating any conflicts, distance shown in minutes and miles
- Halo and PTL (Projected Track Line) tools with ability to choose distance
- Custom line ruler mechanic similar to default ES ruler
- Grid display
- Flight plan window with ability to share data between plugins
- Probe tool to detect conflicts down a track before issuing an oceanic clearance
- Controller notepad for general information
- Recognition of aircraft equipment code to determine compatibility with ADS-B procedures and indicator to display whether aircraft is compatible
- Saving of data
- ... and more!

![logo](https://resources.ganderoceanic.com/media/img/brand/bnr/small_bnr.png)

Deleaker helps me find critical memory leaks in this C++ project. Go check it out here! http://deleaker.com/
