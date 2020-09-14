#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include <gdiplus.h>


using namespace Gdiplus;

RadarDisplay::RadarDisplay() 
{

}

RadarDisplay::~RadarDisplay() 
{

}

void RadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	if (Phase != REFRESH_PHASE_BEFORE_TAGS) return;

	Graphics g(hDC);

	CRadarTarget ac;
	ac = GetPlugIn()->RadarTargetSelectFirst();

	while (ac.IsValid()) {
		AcTargets::DrawAirplane(&g, this, ac);

		ac = GetPlugIn()->RadarTargetSelectNext(ac);
	}
}

void RadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{

}

void RadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) 
{

}

void RadarDisplay::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
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