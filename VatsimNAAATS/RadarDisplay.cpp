#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "MenuBar.h"
#include "InboundList.h"
#include "Constants.h"
#include <gdiplus.h>


using namespace Gdiplus;

RadarDisplay::RadarDisplay() 
{
	inboundList = new CInboundList({ 100, 150 });
	menuButtons = MenuBar::BuildButtonData();
	buttonsPressed.insert(make_pair(MENBTN_TAGS, true));
	asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
}

RadarDisplay::~RadarDisplay() 
{

}

// On radar screen refresh (occurs about once a second)
void RadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	/// TIMERS FOR BUTTONS
	double t = (double)(clock() - buttonClickTimer) / ((double)CLOCKS_PER_SEC);
	if (t >= 0.1) {
		/*if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end() && clickType == true) {
			buttonsPressed.erase(MENBTN_QCKLOOK);
			buttonClickTimer = clock();
		}*/
	}

	// Clean up old list
	inboundAircraft.clear();

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
							if (std::find(pointsShanwick.begin(), pointsShanwick.end(), rte.GetPointName(i)) != pointsShanwick.end()) {
								inboundAircraft.push_back(make_pair(ac, false));
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								break;
							}
						}
					}
				}
				else if ((hdg >= 1) && (hdg <= 179)) {
					direction = true;
					if (fp.GetSectorEntryMinutes() > 0 && fp.GetSectorEntryMinutes() <= 90) {
						// Gander
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							// Add to inbound aircraft list
							if (std::find(pointsGander.begin(), pointsGander.end(), rte.GetPointName(i)) != pointsGander.end()) {
								inboundAircraft.push_back(make_pair(ac, true));
								direction = true;
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								break;
							}
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

				// Draw the tag and target with the information if tags are turned on
				if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
					auto kv = tagStatuses.find(fp.GetCallsign());
					kv->second.first = detailedEnabled; // Set detailed on
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg, true);
					CAcTargets::DrawTag(&dc, this, &ac, &kv->second, direction);
				}
				else {
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg, false);
				}
			}

			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}
		// Draw menu bar
		MenuBar::DrawMenuBar(&dc, &g, this, { RadarArea.left, RadarArea.top }, &menuButtons, &buttonsPressed);
		// Inbound list
		inboundList->DrawList(&g, &dc, this, &inboundAircraft, &epVec);
	}
	
	if (Phase == REFRESH_PHASE_AFTER_LISTS) {
		
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
}

void RadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	mousePointer = Pt;
	if (ObjectType == LIST_INBOUND) {
		inboundList->MoveList(Area, Released);
	}

	if (ObjectType == SCREEN_TAG) {
		auto kv = tagStatuses.find(sObjectId);
		POINT acPosPix = ConvertCoordFromPositionToPixel(GetPlugIn()->RadarTargetSelect(sObjectId).GetPosition().GetPosition());
		kv->second.second = { Area.left - acPosPix.x, Area.top - acPosPix.y };
	}

	RequestRefresh();
}

void RadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) 
{

}

void RadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// If menu button is being unpressed
	if (buttonsPressed.find(ObjectType) != buttonsPressed.end()) {
		buttonsPressed.erase(ObjectType);
	} else if (menuButtons.find(ObjectType) != menuButtons.end()) { // If being pressed
		if (buttonsPressed.find(ObjectType) == buttonsPressed.end()) {
			buttonClickTimer = clock();
			buttonsPressed[ObjectType] = true;
		}
	}

	// If screen object is a tag
	if (ObjectType == SCREEN_TAG) {
		// Set the ASEL
		asel = sObjectId;
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
	}

	// Qck Look button
	if (ObjectType == MENBTN_QCKLOOK) {
		aselDetailed = false;
	}

	// Detailed button
	if (ObjectType == MENBTN_DETAILED) {
		aselDetailed = true;
	}
	
	RequestRefresh();
}

void RadarDisplay::OnAsrContentToBeSaved(void)
{
	/// Save all necessary data to ASR

	// Inbound list
	SaveDataToAsr(ASR_INBND_X.c_str(), "Position of the vNAAATS inbound list (x coordinate)", to_string(inboundList->GetTopLeft().x).c_str());
	SaveDataToAsr(ASR_INBND_Y.c_str(), "Position of the vNAAATS inbound list (y coordinate)", to_string(inboundList->GetTopLeft().y).c_str());
}

void RadarDisplay::OnAsrContentLoaded(bool Loaded)
{
	if (!Loaded)
		return;

	// Get inbound list data
	const char* inbX = GetDataFromAsr(ASR_INBND_X.c_str());
	const char* inbY = GetDataFromAsr(ASR_INBND_Y.c_str());
	if (inbX != NULL && inbY != NULL) {
		inboundList->MoveList(CRect(atoi(inbX), atoi(inbY), 0, 0), true);
	}	
}

void RadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{ 

}

void RadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

}

void RadarDisplay::OnAsrContentToBeClosed(void)
{
	delete this;
}