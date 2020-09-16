#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "MenuBar.h"
#include "Lists.h"
#include <gdiplus.h>


using namespace Gdiplus;

RadarDisplay::RadarDisplay() 
{

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

		// Loop all aircraft
		while (ac.IsValid()) {
			// Time and heading
			entryMinutes = GetPlugIn()->FlightPlanSelect(ac.GetCallsign()).GetSectorEntryMinutes();
			hdg = ac.GetPosition().GetReportedHeading();

			// Draw the aircraft if already in airspace
			if (entryMinutes == 0) {
				AcTargets::DrawAirplane(&g, &dc, this, ac, hdg);
			}
			else if (entryMinutes > 0) {
				// If inbound
				if (GetPlugIn()->FlightPlanSelect(ac.GetCallsign()).GetSectorEntryMinutes() > 0 && GetPlugIn()->FlightPlanSelect(ac.GetCallsign()).GetSectorEntryMinutes() <= 90) {
					AcTargets::DrawAirplane(&g, &dc, this, ac, hdg);

					if ((hdg <= 359) && (hdg >= 181)) {
						// Shanwick
						inboundAircraft.push_back(make_pair(ac, false));
					}
					else if ((hdg >= 1) && (hdg <= 179)) {
						// Gander
						inboundAircraft.push_back(make_pair(ac, true));
					}
				}
			}
			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}

		// Draw menu bar
		MenuBar::DrawMenuBar(&dc, this, { RadarArea.left, RadarArea.top });
		// Inbound list
		Lists::DrawInboundList(&g, &dc, this, { RadarArea.left, RadarArea.top }, &inboundAircraft);
	}
	
	if (Phase == REFRESH_PHASE_AFTER_LISTS) {
		
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
}

void RadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{

}

void RadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) 
{

}

void RadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

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