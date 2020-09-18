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

			// Draw the aircraft if already in airspace
			if (entryMinutes == 0) {
				AcTargets::DrawAirplane(&g, &dc, this, ac, hdg);
			}
			else if (entryMinutes > 0) {
				// If inbound
				if (fp.GetSectorEntryMinutes() > 0 && fp.GetSectorEntryMinutes() <= 90) {
					AcTargets::DrawAirplane(&g, &dc, this, ac, hdg);

					if ((hdg <= 359) && (hdg >= 181)) {
						// Shanwick
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							if (std::find(pointsShanwick.begin(), pointsShanwick.end(), rte.GetPointName(i)) != pointsShanwick.end()) {
								inboundAircraft.push_back(make_pair(ac, false));
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								break;
							}
						}		
					}
					else if ((hdg >= 1) && (hdg <= 179)) {
						// Gander
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							if (std::find(pointsGander.begin(), pointsGander.end(), rte.GetPointName(i)) != pointsGander.end()) {
								inboundAircraft.push_back(make_pair(ac, true));
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								break;
							}
						}
					}
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
			buttonsPressed[ObjectType] = true;
		}
	}

	
	
}

void RadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{ 

}

void RadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

}

void RadarDisplay::OnAsrContentToBeSaved(void)
{

}

void RadarDisplay::OnAsrContentLoaded(bool Loaded)
{

}

void RadarDisplay::OnAsrContentToBeClosed(void)
{

}