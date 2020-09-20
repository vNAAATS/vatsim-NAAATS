#include "pch.h"
#include "InboundList.h"
#include "Constants.h"
#include "Styles.h"
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace Colours;

CInboundList::CInboundList(POINT topLeftPt) {
	topLeft = topLeftPt;
}

POINT CInboundList::GetTopLeft() {
	return topLeft;
}

Rect CInboundList::DrawList(Graphics* g, CDC* dc, CRadarScreen* screen, vector<pair<CRadarTarget, bool>>* inboundAircraft, vector<pair<string, int>>* epVec)
{
	// Brushes
	SolidBrush transparentBrush(Color(0, 0, 0, 0));
	
	// Save context for later
	int sDC = dc->SaveDC();

	// Make rectangle
	Rect rectangle(topLeft.x, topLeft.y, LIST_INBOUND_WIDTH, 500);

	/// Make "Inbound" text
	// Font
	FontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Don't show size if none
	if (inboundAircraft->size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Inbound");
	} 
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Inbound (" + to_string(inboundAircraft->size()) + ")").c_str());
	}
	

	// Make inbound lines
	int offsetY = 14;
	int offsetX = 18;
	int idx = 0;
	for (vector<pair<CRadarTarget, bool>>::iterator ac = inboundAircraft->begin(); ac != inboundAircraft->end(); ac++) {
		// Get the aircraft
		CFlightPlan fp(screen->GetPlugIn()->FlightPlanSelect(ac->first.GetCallsign()));

		// Text
		FontSelector::SelectATCFont(18, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Direction arrow (Shanwick)
		if (ac->second == false) {
			SolidBrush brush(TextWhite);
			Point points[3] = { Point(rectangle.X + 10, rectangle.Y + (offsetY + 4)),
				Point(rectangle.X + 10, rectangle.Y + (offsetY + 4) + 10),
				Point(rectangle.X, rectangle.Y + (offsetY + 4) + 5) };
			g->FillPolygon(&brush, points, 3);
		}
		// Draw callsign
		string line = string(fp.GetCallsign());
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 140;

		// Draw entry point
		line = epVec->at(idx).first;		
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 70;

		// Draw estimated time
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, ParseZuluTime(fp, epVec->at(idx).second).c_str());
		offsetX += 45;

		// Draw altitude
		//line = to_string(fp.GetControllerAssignedData().GetClearedAltitude());
		line = to_string(fp.GetFinalAltitude() / 100);
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());		
		offsetX += 50;

		// Draw destination
		line = string(fp.GetFlightPlanData().GetDestination());
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 45;

		// Direction arrow (Gander)
		if (ac->second == true) {
			SolidBrush brush(TextWhite);
			Point points[3] = { Point(rectangle.X + offsetX, rectangle.Y + (offsetY + 4)),
				Point(rectangle.X + offsetX, rectangle.Y + (offsetY + 4) + 10),
				Point(rectangle.X + offsetX + 10, rectangle.Y + (offsetY + 4) + 5) };
			g->FillPolygon(&brush, points, 3);
		}

		// Offset the y index and reset the X offset
		offsetY += 18;
		offsetX = 18;

		// Increment
		idx++;
	}

	// Restore device context
	dc->RestoreDC(sDC);

	// Get object area and add object to screen
	CRect area(topLeft.x, topLeft.y, topLeft.x + LIST_INBOUND_WIDTH, topLeft.y + 14);
	screen->AddScreenObject(LIST_INBOUND, "", area, true, "");


	// Deallocate
	DeleteObject(&transparentBrush);

	return rectangle;
}

void CInboundList::MoveList(CRect area, bool isReleased) { // TODO: check need for isReleased
	isMouseReleased = isReleased;
	topLeft = { area.left, area.top };
}

// TODO: move code to a utils class
string CInboundList::ParseZuluTime(CFlightPlan fp, int ep) {
	time_t now = time(0);
	tm* zuluTime = gmtime(&now);
	int deltaMinutes = fp.GetExtractedRoute().GetPointDistanceInMinutes(ep);
	int hours = zuluTime->tm_hour;
	int minutes = zuluTime->tm_min + deltaMinutes;
	
	if (minutes >= 60) {
		// Get minutes
		int minRemainder = minutes % 60;

		// Get number of hours
		hours = (minutes - minRemainder) / 60;

		// Reassign number of minutes
		minutes = minRemainder;
	}

	// Check if over 24 hours
	if (hours >= 24) {
		hours = hours - 24;
	}


	// Pad for zeros
	string strHours;
	if (hours < 10) {
		if (hours == 0) {
			strHours = "00";
		}
		else {
			strHours = "0" + to_string(hours);
		}
	}
	else {
		strHours = to_string(hours);
	}

	string strMinutes;
	if (minutes < 10) {
		if (minutes == 0) {
			strMinutes = "00";
		}
		else {
			strMinutes = "0" + to_string(minutes);
		}
	}
	else {
		strMinutes = to_string(minutes);
	}

	return strHours + strMinutes;
}