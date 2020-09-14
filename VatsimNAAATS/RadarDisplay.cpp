#include "pch.h"
#include "RadarDisplay.h"
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

	CDC dc;
	dc.Attach(hDC);

	CRadarTarget ac;
	ac = GetPlugIn()->RadarTargetSelectFirst();

	while (ac.IsValid()) {
		POINT acPoint = this->ConvertCoordFromPositionToPixel(ac.GetPosition().GetPosition());

		CPen pen;
		pen.CreatePen(0, 1, RGB(255, 128, 0));
		dc.SelectObject(pen);


		dc.MoveTo(acPoint.x, acPoint.y - 8);
		dc.LineTo(acPoint.x - 1, acPoint.y - 7);
		dc.LineTo(acPoint.x - 1, acPoint.y - 2);
		dc.LineTo(acPoint.x - 8, acPoint.y + 3);
		dc.LineTo(acPoint.x - 8, acPoint.y + 4);
		dc.LineTo(acPoint.x - 1, acPoint.y + 2);
		dc.LineTo(acPoint.x - 1, acPoint.y + 7);
		dc.LineTo(acPoint.x - 4, acPoint.y + 9);
		dc.LineTo(acPoint.x - 4, acPoint.y + 10);
		dc.LineTo(acPoint.x, acPoint.y + 9);
		dc.LineTo(acPoint.x + 4, acPoint.y + 10);
		dc.LineTo(acPoint.x + 4, acPoint.y + 9);
		dc.LineTo(acPoint.x + 1, acPoint.y + 7);
		dc.LineTo(acPoint.x + 1, acPoint.y + 2);
		dc.LineTo(acPoint.x + 8, acPoint.y + 4);
		dc.LineTo(acPoint.x + 8, acPoint.y + 3);
		dc.LineTo(acPoint.x + 1, acPoint.y - 2);
		dc.LineTo(acPoint.x + 1, acPoint.y - 7);
		dc.LineTo(acPoint.x, acPoint.y - 8);
		dc.LineTo(acPoint.x, acPoint.y + 9);
		dc.MoveTo(acPoint.x - 8, acPoint.y + 4);
		dc.LineTo(acPoint.x, acPoint.y + 1);
		dc.LineTo(acPoint.x + 8, acPoint.y + 4);
		dc.LineTo(acPoint.x, acPoint.y);
		dc.LineTo(acPoint.x - 8, acPoint.y + 4);
		dc.LineTo(acPoint.x, acPoint.y - 1);
		dc.LineTo(acPoint.x + 8, acPoint.y + 4);
		dc.LineTo(acPoint.x, acPoint.y - 2);
		dc.LineTo(acPoint.x - 8, acPoint.y + 4);
		dc.MoveTo(acPoint.x - 8, acPoint.y + 3);
		dc.LineTo(acPoint.x, acPoint.y + 1);
		dc.LineTo(acPoint.x + 8, acPoint.y + 3);
		dc.LineTo(acPoint.x, acPoint.y);
		dc.LineTo(acPoint.x - 8, acPoint.y + 3);
		dc.LineTo(acPoint.x, acPoint.y - 1);
		dc.LineTo(acPoint.x + 8, acPoint.y + 3);
		dc.MoveTo(acPoint.x + 1, acPoint.y - 7);
		dc.LineTo(acPoint.x - 1, acPoint.y - 7);
		dc.LineTo(acPoint.x + 1, acPoint.y - 6);
		dc.LineTo(acPoint.x - 1, acPoint.y - 6);
		dc.LineTo(acPoint.x + 1, acPoint.y - 5);
		dc.LineTo(acPoint.x - 1, acPoint.y - 5);
		dc.LineTo(acPoint.x + 1, acPoint.y - 4);
		dc.LineTo(acPoint.x - 1, acPoint.y - 4);
		dc.LineTo(acPoint.x + 1, acPoint.y - 3);
		dc.LineTo(acPoint.x - 1, acPoint.y - 3);
		dc.LineTo(acPoint.x + 1, acPoint.y - 2);
		dc.LineTo(acPoint.x - 1, acPoint.y - 2);
		dc.LineTo(acPoint.x + 1, acPoint.y - 1);
		dc.LineTo(acPoint.x - 1, acPoint.y - 1);
		dc.LineTo(acPoint.x + 1, acPoint.y - 2);
		dc.LineTo(acPoint.x - 1, acPoint.y - 2);
		dc.LineTo(acPoint.x + 1, acPoint.y - 3);
		dc.LineTo(acPoint.x - 1, acPoint.y - 3);
		dc.LineTo(acPoint.x + 1, acPoint.y - 4);
		dc.LineTo(acPoint.x - 1, acPoint.y - 4);
		dc.LineTo(acPoint.x + 1, acPoint.y - 5);
		dc.LineTo(acPoint.x - 1, acPoint.y - 5);
		dc.LineTo(acPoint.x + 1, acPoint.y - 6);
		dc.LineTo(acPoint.x - 1, acPoint.y - 6);
		dc.LineTo(acPoint.x + 1, acPoint.y - 7);
		dc.MoveTo(acPoint.x + 1, acPoint.y + 2);
		dc.LineTo(acPoint.x - 1, acPoint.y - 2);
		dc.LineTo(acPoint.x + 1, acPoint.y - 3);
		dc.LineTo(acPoint.x - 1, acPoint.y - 3);
		dc.LineTo(acPoint.x + 1, acPoint.y - 4);
		dc.LineTo(acPoint.x - 1, acPoint.y - 4);
		dc.LineTo(acPoint.x + 1, acPoint.y - 5);
		dc.LineTo(acPoint.x - 1, acPoint.y - 5);
		dc.LineTo(acPoint.x + 1, acPoint.y - 6);
		dc.LineTo(acPoint.x - 1, acPoint.y - 6);
		dc.LineTo(acPoint.x + 1, acPoint.y - 7);
		dc.LineTo(acPoint.x - 1, acPoint.y - 7);
		dc.LineTo(acPoint.x + 1, acPoint.y - 8);
		dc.LineTo(acPoint.x - 1, acPoint.y - 8);
		dc.LineTo(acPoint.x + 1, acPoint.y - 9);
		dc.LineTo(acPoint.x - 1, acPoint.y - 9);
		dc.LineTo(acPoint.x + 1, acPoint.y - 8);
		dc.LineTo(acPoint.x - 1, acPoint.y - 8);
		dc.LineTo(acPoint.x + 1, acPoint.y - 7);
		dc.LineTo(acPoint.x - 1, acPoint.y - 7);
		dc.LineTo(acPoint.x + 1, acPoint.y - 6);
		dc.LineTo(acPoint.x - 1, acPoint.y - 6);
		dc.LineTo(acPoint.x + 1, acPoint.y - 5);
		dc.LineTo(acPoint.x - 1, acPoint.y - 5);
		dc.LineTo(acPoint.x + 1, acPoint.y - 4);
		dc.LineTo(acPoint.x - 1, acPoint.y - 4);
		dc.LineTo(acPoint.x + 1, acPoint.y - 3);
		dc.LineTo(acPoint.x - 1, acPoint.y - 3);
		dc.LineTo(acPoint.x + 1, acPoint.y + 2);
		dc.MoveTo(acPoint.x - 4, acPoint.y + 9);
		dc.LineTo(acPoint.x, acPoint.y + 7);
		dc.LineTo(acPoint.x + 4, acPoint.y + 9);
		dc.LineTo(acPoint.x , acPoint.y + 8);
		dc.LineTo(acPoint.x - 4, acPoint.y + 9);
		dc.LineTo(acPoint.x - 4, acPoint.y + 9);
		dc.LineTo(acPoint.x, acPoint.y + 9);
		dc.LineTo(acPoint.x + 4, acPoint.y + 9);
		dc.MoveTo(acPoint.x - 4, acPoint.y + 10);
		dc.LineTo(acPoint.x, acPoint.y + 7);
		dc.LineTo(acPoint.x + 4, acPoint.y + 10);
		dc.LineTo(acPoint.x, acPoint.y + 8);
		dc.LineTo(acPoint.x - 1, acPoint.y + 10);

		if (&pen != nullptr) {
			pen.DeleteObject();
		}
		ac = GetPlugIn()->RadarTargetSelectNext(ac);
	}

	dc.Detach();
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