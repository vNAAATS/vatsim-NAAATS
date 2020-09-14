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

	Graphics g(hDC);

	CRadarTarget ac;
	ac = GetPlugIn()->RadarTargetSelectFirst();

	while (ac.IsValid()) {
		POINT acPoint = this->ConvertCoordFromPositionToPixel(ac.GetPosition().GetPosition());

		SolidBrush brush(Color(255, 125, 0));
		GraphicsContainer gContainer;
		double hdg = (double)ac.GetPosition().GetReportedHeading();

		gContainer = g.BeginContainer();
		g.RotateTransform(hdg);
		g.TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);
		Point points[19] = {
			Point(0,-8),
			Point(-1,-7),
			Point(-1,-2),
			Point(-8,3),
			Point(-8,4),
			Point(-1,2),
			Point(-1,7),
			Point(-4,9),
			Point(-4,10),
			Point(0,9),
			Point(4,10),
			Point(4,9),
			Point(1,7),
			Point(1,2),
			Point(8,4),
			Point(8,3),
			Point(1,-2),
			Point(1,-7),
			Point(0,-8)
		};
		g.FillPolygon(&brush, points, 19);
		g.EndContainer(gContainer);

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