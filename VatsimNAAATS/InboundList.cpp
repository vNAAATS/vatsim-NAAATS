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

		// TODO fix
		// Draw estimated time
		time_t now = time(0);
		tm* zuluTime = gmtime(&now);
		zuluTime->tm_min += fp.GetExtractedRoute().GetPointDistanceInMinutes(epVec->at(idx).second);
		int h = zuluTime->tm_hour;
		int h_add = (zuluTime->tm_min % 60) / 60;
		int h_add_floored = floor(h_add);
		h += h_add_floored;
		int m = zuluTime->tm_min - (h_add - h_add_floored) * 60;
		int zuluTimeInt = h * 100 + m;
		// Leading zeroes
		std::ostringstream stream;
		stream << std::setw(4) << std::setfill('0') << zuluTimeInt;
		line = stream.str();
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
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

void CInboundList::MoveList(CRect area, bool isReleased) {
	isMouseReleased = isReleased;
	topLeft = { area.left, area.top };
}