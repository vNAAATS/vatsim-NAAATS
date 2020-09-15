#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "MenuBar.h"
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
	// Graphics object
	Graphics g(hDC);

	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Get the radar area
	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		// Get first aircraft
		CRadarTarget ac;
		ac = GetPlugIn()->RadarTargetSelectFirst();

		// Loop all aircraft
		while (ac.IsValid()) {
			// Draw the aircraft ONLY if inbound or already in airspace
			if (GetPlugIn()->FlightPlanSelect(ac.GetCallsign()).GetSectorEntryMinutes() != -1 || GetPlugIn()->FlightPlanSelect(ac.GetCallsign()).GetSectorExitMinutes() != -1) {
				AcTargets::DrawAirplane(&g, this, ac);
			}

			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}
	}
	
	if (Phase == REFRESH_PHASE_AFTER_TAGS) {
		MenuBar::DrawMenuBar(&dc, this, { RadarArea.left, RadarArea.top });
	}

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