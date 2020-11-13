#include "pch.h"
#include "Overlays.h"

COverlayType COverlays::CurrentType = COverlayType::TCKS_ALL;

void COverlays::ShowCurrentOverlay(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Render the tracks path
	CCommonRenders::RenderTracks(dc, g, screen, &CurrentType);
}

// Show and hide the grid reference and waypoints
void COverlays::ShowHideGridReference(CRadarScreen* screen, bool show) {
	screen->GetPlugIn()->SelectActiveSectorfile();

	CSectorElement gridElement;
	for (gridElement = screen->GetPlugIn()->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_GEO);
		gridElement.IsValid();
		gridElement = screen->GetPlugIn()->SectorFileElementSelectNext(gridElement, EuroScopePlugIn::SECTOR_ELEMENT_GEO))
	{
		string t = gridElement.GetName();
		if (t == "CZQO Landmark Positional Grid Reference" || t == "EGGX Landmark Positional Grid Reference") {
			break;
		}
	}

	// Show the grid
	screen->ShowSectorFileElement(gridElement, gridElement.GetComponentName(0), show);

	CSectorElement gridTextElement;
	for (gridTextElement = screen->GetPlugIn()->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_FREE_TEXT);
		gridTextElement.IsValid();
		gridTextElement = screen->GetPlugIn()->SectorFileElementSelectNext(gridTextElement, EuroScopePlugIn::SECTOR_ELEMENT_FREE_TEXT))
	{
		string t = gridTextElement.GetName();
		if (t.find("CZQO Grid Reference Numbers.") != string::npos || t.find("EGGX Grid Reference Numbers.") != string::npos) {
			break;
		}
	}

	// Show the grid
	screen->ShowSectorFileElement(gridTextElement, gridElement.GetComponentName(0), show);

	/*if (show) { // If to be shown
		// Get sector file and element
		
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());
		
		// Find the grid
		while (gridName != "CZQO Landmark Positional Grid Reference" || gridName != "EGGX Landmark Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		// Find the grid number freetext
		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos || freetextName.find("EGGX Grid Reference Numbers.") == string::npos) {
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

		
	}
	else { // Not showing it
		// Get sector file and element
		screen->GetPlugIn()->SelectActiveSectorfile();
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());

		// Find the grid
		while (gridName != "CZQO Landmark Positional Grid Reference" || gridName != "EGGX Landmark Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		// Find the grid number freetext
		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos || freetextName.find("EGGX Grid Reference Numbers.") == string::npos) {
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
	}*/

	// Refresh the map content
	screen->RefreshMapContent();
	screen->RequestRefresh();
}