#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "MenuBar.h"
#include "InboundList.h"
#include "Constants.h"
#include "Utils.h"
#include <gdiplus.h>


using namespace Gdiplus;
using namespace std;
using namespace EuroScopePlugIn;

CRadarDisplay::CRadarDisplay() 
{
	ShowHideGridReference(this, false);
	inboundList = new CInboundList({ 500, 150 });
	otherList = new COtherList({ 200, 150 });
	menuButtons = CMenuBar::BuildButtonData();
	toggleButtons = CMenuBar::BuildToggleButtonData();
	asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
}

CRadarDisplay::~CRadarDisplay() 
{

}

void CRadarDisplay::PopulateProgramData() {
	// Lists
	inboundList->MoveList({ Utils::InboundX, Utils::InboundY }, true);
	otherList->MoveList({ Utils::OthersX, Utils::OthersY });

	// Buttons
	if (Utils::TagsEnabled && buttonsPressed.find(MENBTN_TAGS) == buttonsPressed.end()) {
		buttonsPressed[MENBTN_TAGS] = true;
	}
	if (Utils::GridEnabled && buttonsPressed.find(MENBTN_GRID) == buttonsPressed.end()) {
		buttonsPressed[MENBTN_GRID] = true;
	}

}

// Show and hide the grid reference and waypoints
void CRadarDisplay::ShowHideGridReference(CRadarScreen* screen, bool show) {
	if (show) {
		screen->GetPlugIn()->SelectScreenSectorfile(screen);
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());
		while (gridName != "CZQO Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos) {
			
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
			
		}

		string componentName;
		while (freetextName.find("CZQO Grid Reference Numbers.") != string::npos) {
			componentName = freetext.GetComponentName(0);
			screen->ShowSectorFileElement(freetext, componentName.c_str(), true);
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		screen->ShowSectorFileElement(grid, "", true);
	}
	else {
		screen->GetPlugIn()->SelectScreenSectorfile(screen);
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());
		while (gridName != "CZQO Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos) {
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		string componentName;
		while (freetextName.find("CZQO Grid Reference Numbers.") != string::npos) {
			componentName = freetext.GetComponentName(0);
			screen->ShowSectorFileElement(freetext, componentName.c_str(), false);
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		screen->ShowSectorFileElement(grid, "", false);
	}

	RefreshMapContent();
}

// On radar screen refresh (occurs about once a second)
void CRadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	// Clean up old lists
	inboundAircraft.clear();
	otherAircraft.clear();

	// Get the radar area
	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		// Get first aircraft
		CRadarTarget ac;
		ac = GetPlugIn()->RadarTargetSelectFirst();

		// Get entry time and heading
		int entryMinutes;
		int hdg;

		// List of entry points
		vector<pair<string, int>> epVec;

		// Loop all aircraft
		while (ac.IsValid()) {
			// Flight plan
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(ac.GetCallsign());

			// Route
			CFlightPlanExtractedRoute rte = fp.GetExtractedRoute();

			// Time and heading
			entryMinutes = fp.GetSectorEntryMinutes();
			hdg = ac.GetPosition().GetReportedHeading();
			// Get direction
			bool direction = false;
			// Aircraft to render
			if (entryMinutes >= 0 && entryMinutes <= 90) {
				// If not there
				if (tagStatuses.find(fp.GetCallsign()) == tagStatuses.end()) {
					pair<bool, POINT> pt = make_pair(false, POINT{ 0, 0 });
					tagStatuses.insert(make_pair(string(fp.GetCallsign()), pt));
				}
				
				// Get inbound aircraft and flight direction	
				if ((hdg <= 359) && (hdg >= 181)) {
					if (fp.GetSectorEntryMinutes() > 0 && fp.GetSectorEntryMinutes() <= 90) {
						// Shanwick
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							// Add to inbound aircraft list
							bool breakLoop = false;
							if (std::find(pointsShanwick.begin(), pointsShanwick.end(), rte.GetPointName(i)) != pointsShanwick.end()) {
								inboundAircraft.push_back(make_pair(ac, false));
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								breakLoop = true;
								break;
							}
							if (breakLoop) break;
						}
					}
				}
				else if ((hdg >= 1) && (hdg <= 179)) {
					direction = true;
					if (fp.GetSectorEntryMinutes() > 0 && fp.GetSectorEntryMinutes() <= 90) {
						// Gander
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							// Add to inbound aircraft list
							bool breakLoop = false;
							if (std::find(pointsGander.begin(), pointsGander.end(), rte.GetPointName(i)) != pointsGander.end()) {
								inboundAircraft.push_back(make_pair(ac, true));
								direction = true;
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								breakLoop = true;
								break;
							}
							if (breakLoop) break;
						}
					}
				}

				// Store whether detailed tags are enabled
				bool detailedEnabled = false; 

				// Now we check if all the tags are selected as detailed				
				if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end()) {
					detailedEnabled = true; // Set detailed on

					// Unpress detailed if not already
					if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end() && !aselDetailed) {
						buttonsPressed.erase(MENBTN_DETAILED);
					}
				}

				// Check if only one is set to detailed
				if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end()) {
					if (fp.GetCallsign() == asel) {
						detailedEnabled = true; // Set detailed on
					}

					// Unpress quick look if not already
					if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end() && aselDetailed) {
						buttonsPressed.erase(MENBTN_QCKLOOK);
					}
				}

				bool ptl = false;
				bool halo = false;
				// Set PTL and HALO if they are on
				if (buttonsPressed.find(MENBTN_PTL) != buttonsPressed.end()) {
					ptl = true;
				}
				if (buttonsPressed.find(MENBTN_HALO) != buttonsPressed.end()) {
					halo = true;
				}

				// Draw the tag and target with the information if tags are turned on
				if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
					auto kv = tagStatuses.find(fp.GetCallsign());
					kv->second.first = detailedEnabled; // Set detailed on
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg, true, &toggleButtons, halo, ptl);
					CAcTargets::DrawTag(&dc, this, &ac, &kv->second, direction);
				}
				else {
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg, false, &toggleButtons, halo, ptl);
				}
			}

			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}

		// RBL draw
		if (buttonsPressed.find(MENBTN_RBL) != buttonsPressed.end()) {
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				CAcTargets::RangeBearingLine(&g, &dc, this, aircraftSel1, aircraftSel2);
			}
		}
		else {
			// Reset
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// Draw menu bar and reset dropdown click
		CMenuBar::DrawMenuBar(&dc, &g, this, { RadarArea.left, RadarArea.top }, &menuButtons, &buttonsPressed, &toggleButtons, &dropDownItems, dropDownHover);
		dropDownClicked = -1;

		// Draw Lists
		inboundList->DrawList(&g, &dc, this, &inboundAircraft, &epVec);
		otherList->DrawList(&g, &dc, this, &otherAircraft);
	}
	
	if (Phase == REFRESH_PHASE_AFTER_LISTS) {
		
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
}

void CRadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	mousePointer = Pt;
	if (ObjectType == LIST_INBOUND) {
		inboundList->MoveList(Area, Released);

		// To save
		Utils::InboundX = Area.left;
		Utils::InboundY = Area.top;
	}

	if (ObjectType == LIST_OTHERS) {
		otherList->MoveList(Area);

		Utils::OthersX = Area.left;
		Utils::OthersY = Area.top;
	}

	if (ObjectType == SCREEN_TAG) {
		auto kv = tagStatuses.find(sObjectId);
		POINT acPosPix = ConvertCoordFromPositionToPixel(GetPlugIn()->RadarTargetSelect(sObjectId).GetPosition().GetPosition());
		kv->second.second = { Area.left - acPosPix.x, Area.top - acPosPix.y };
	}

	RequestRefresh();
}

void CRadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) 
{
	// Dropdown
	if (ObjectType == DROPDOWN) {
		dropDownHover = atoi(sObjectId);
	}

	RequestRefresh();
}

void CRadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Left button actions
	if (Button == BUTTON_LEFT) {
		int dropDownToCancel = -1;
		// If screen object is a dropdown
		if (ObjectType == MENDRP_AREASEL) {
			dropDownItems.clear();
			dropDownToCancel = currentDropDownId;
			currentDropDownId = MENDRP_AREASEL;
			dropDownHover = -1;
			dropDownItems[800] = "EGGX";
			dropDownItems[801] = "CZQX";
			dropDownItems[802] = "BDBX";
		}
		else if (ObjectType == MENDRP_TCKCTRL) {
			dropDownItems.clear();
			dropDownToCancel = currentDropDownId;		
			currentDropDownId = MENDRP_TCKCTRL;
			dropDownHover = -1;
		}
		else if (ObjectType == MENDRP_OVERLAYS) {
			dropDownItems.clear();
			dropDownToCancel = currentDropDownId;
			currentDropDownId = MENDRP_OVERLAYS;
			dropDownHover = -1;
		}
		else if (ObjectType == MENDRP_TYPESEL) {
			dropDownItems.clear();
			dropDownToCancel = currentDropDownId;
			currentDropDownId = MENDRP_TYPESEL;
			dropDownHover = -1;
			dropDownItems[800] = "Delivery";
			dropDownItems[801] = "OCA Enroute";
			dropDownItems[802] = "Multi-role";
		}

		// If item is a drop down menu item
		bool brk = false;
		for (auto kv : dropDownItems) {
			if (atoi(sObjectId) == kv.first) {
				CMenuBar::DropDownSelections[currentDropDownId] = kv.second;
				dropDownClicked = atoi(sObjectId);
				dropDownHover = -1;
				buttonsPressed.erase(currentDropDownId);
				currentDropDownId = -1;
				brk = true;
				break;
			}
		}

		// Clear dropdown items if so
		if (brk) dropDownItems.clear();

		// If menu button is being unpressed
		if (buttonsPressed.find(ObjectType) != buttonsPressed.end() && string(sObjectId) == "") {
			buttonsPressed.erase(ObjectType);

			if (ObjectType == currentDropDownId) {
				currentDropDownId = -1;
				dropDownHover = -1;
			}

			// Button settings
			if (ObjectType == MENBTN_TAGS) {
				Utils::TagsEnabled = false;
			}
			else if (ObjectType == MENBTN_GRID) {
				// Grid is off
				Utils::GridEnabled = false;
				ShowHideGridReference(this, Utils::GridEnabled);
			}
		}
		else if (menuButtons.find(ObjectType) != menuButtons.end() && string(sObjectId) == "") { // If being pressed
			if (buttonsPressed.find(ObjectType) == buttonsPressed.end()) {
				buttonsPressed[ObjectType] = true;
				
				// Cancel existing open dropdown
				if (dropDownToCancel != -1) {
					buttonsPressed.erase(dropDownToCancel);
				}

				// Button settings
				if (ObjectType == MENBTN_TAGS) {
					Utils::TagsEnabled = true;
				}
				else if (ObjectType == MENBTN_GRID) {
					// Grid is on
					Utils::GridEnabled = true;
					ShowHideGridReference(this, Utils::GridEnabled);
				}
			}
		} 

		// If screen object is a tag
		if (ObjectType == SCREEN_TAG) {
			// Set the ASEL
			asel = sObjectId;
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);
			GetPlugIn()->SetASELAircraft(fp);

			// RBL (if active)
			if (buttonsPressed.find(MENBTN_RBL) != buttonsPressed.end()) {
				if (aircraftSel1 == "") {
					aircraftSel1 = asel;
				}
				else if (aircraftSel2 == "") {
					aircraftSel2 = asel;
				}
			}
		}

		// Qck Look button
		if (ObjectType == MENBTN_QCKLOOK) {
			aselDetailed = false;
		}

		// Detailed button
		if (ObjectType == MENBTN_DETAILED) {
			aselDetailed = true;
		}
	}
	
	if (Button == BUTTON_RIGHT) {
		if (ObjectType == MENBTN_HALO) {
			// Get the toggle button
			auto cycle = toggleButtons.find(MENBTN_HALO);

			// Increment if less than or equal 3 (20 minute halos max)
			if (cycle->second < 3) {
				cycle->second++;
			}
			else {
				cycle->second = 0;
			}

			// Now assign the values
			auto haloBtn = menuButtons.find(MENBTN_HALO);
			switch (cycle->second) {
			case 0:
				haloBtn->second = "Halo 5";
				break;
			case 1:
				haloBtn->second = "Halo 10";
				break;
			case 2:
				haloBtn->second = "Halo 15";
				break;
			case 3:
				haloBtn->second = "Halo 20";
				break;
			}
		}

		if (ObjectType == MENBTN_PTL) {
			// Get the toggle button
			auto cycle = toggleButtons.find(MENBTN_PTL);

			// Increment if less than or equal 5 (30 minute lines max)
			if (cycle->second < 5) {
				cycle->second++;
			}
			else {
				cycle->second = 0;
			}

			// Now assign the values
			auto ptlBtn = menuButtons.find(MENBTN_PTL);
			switch (cycle->second) {
			case 0:
				ptlBtn->second = "PTL 5";
				break;
			case 1:
				ptlBtn->second = "PTL 10";
				break;
			case 2:
				ptlBtn->second = "PTL 15";
				break;
			case 3:
				ptlBtn->second = "PTL 20";
				break;
			case 4:
				ptlBtn->second = "PTL 25";
				break;
			case 5:
				ptlBtn->second = "PTL 30";
				break;
			}
		}

		if (ObjectType == MENBTN_RINGS) {
			// Get the toggle button
			auto cycle = toggleButtons.find(MENBTN_RINGS);

			// Increment if less than or equal 4 (5 rings max)
			if (cycle->second < 4) {
				cycle->second++;
			}
			else {
				cycle->second = 0;
			}

			// Now assign the values
			auto ringsBtn = menuButtons.find(MENBTN_RINGS);
			switch (cycle->second) {
			case 0:
				ringsBtn->second = "Rings 1";
				break;
			case 1:
				ringsBtn->second = "Rings 2";
				break;
			case 2:
				ringsBtn->second = "Rings 3";
				break;
			case 3:
				ringsBtn->second = "Rings 4";
				break;
			case 4:
				ringsBtn->second = "Rings 5";
				break;
			}
		}
	}
	
	RequestRefresh();
}

void CRadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{

}

void CRadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

}

void CRadarDisplay::OnAsrContentToBeSaved(void)
{
	Utils::SavePluginData(this->GetPlugIn());
}

void CRadarDisplay::OnAsrContentLoaded(bool Loaded)
{
	if (!Loaded)
		return;

	Utils::LoadPluginData(this->GetPlugIn());

	PopulateProgramData();
}