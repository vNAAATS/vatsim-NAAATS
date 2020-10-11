#include "pch.h"
#include "Overlays.h"

COverlayType COverlays::CurrentType = COverlayType::TCKS_ALL;

map<string, CTrack> COverlays::CurrentTracks;

string COverlays::CurrentTMI = "";

void COverlays::ShowCurrentOverlay(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Render the tracks path
	CPathRenderer::RenderPath(dc, g, screen, CPathType::TCKS);
}

// Show and hide the grid reference and waypoints
void COverlays::ShowHideGridReference(CRadarScreen* screen, bool show) {
	if (show) { // If to be shown
		// Get sector file and element
		screen->GetPlugIn()->SelectScreenSectorfile(screen);
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());
		
		// Find the grid
		while (gridName != "CZQO Positional Grid Reference" && gridName != "EGGX Landmark Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		// Find the grid number freetext
		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos && freetextName.find("EGGX Grid Reference Numbers.") == string::npos) {
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		// Show the freetext
		string componentName;
		while (freetextName.find("CZQO Grid Reference Numbers.") != string::npos || freetextName.find("EGGX Grid Reference Numbers.") != string::npos) {
			componentName = freetext.GetComponentName(0);
			screen->ShowSectorFileElement(freetext, componentName.c_str(), true);
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		// Show the grid
		screen->ShowSectorFileElement(grid, "", true);
	}
	else { // Not showing it
		// Get sector file and element
		screen->GetPlugIn()->SelectScreenSectorfile(screen);
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());

		// Find the grid
		while (gridName != "CZQO Positional Grid Reference" && gridName != "EGGX Landmark Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		// Find the grid number freetext
		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos && freetextName.find("EGGX Grid Reference Numbers.") == string::npos) {
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		// Hide the freetext
		string componentName;
		while (freetextName.find("CZQO Grid Reference Numbers.") != string::npos || freetextName.find("EGGX Grid Reference Numbers.") != string::npos) {
			componentName = freetext.GetComponentName(0);
			screen->ShowSectorFileElement(freetext, componentName.c_str(), false);
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		// Hide the grid
		screen->ShowSectorFileElement(grid, "", false);
	}

	// Refresh the map content
	screen->RefreshMapContent();
}