#include "pch.h"
#include "InboundList.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"
#include <sstream>
#include <iomanip>

using namespace Colours;

vector<CInboundAircraft> CInboundList::AircraftList;

CInboundList::CInboundList(POINT topLeft) : CBaseList(topLeft) {

}

void CInboundList::RenderList(Graphics* g, CDC* dc, CRadarScreen* screen)
{
	// Save context for later
	int sDC = dc->SaveDC();

	// Make rectangle
	Rect rectangle(topleft.x, topleft.y, LIST_INBOUND_WIDTH, 500);
	
	// Text
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Make inbound lines
	int offsetY = 14;
	int offsetX = 18;
	int idx = 0;

	for (vector<CInboundAircraft>::iterator ac = AircraftList.begin(); ac != AircraftList.end(); ac++) {
		// Direction arrow (Shanwick)
		if (ac->Direction == false) {
			SolidBrush brush(TextWhite);
			Point points[3] = { Point(rectangle.X + 10, rectangle.Y + (offsetY + 4)),
				Point(rectangle.X + 10, rectangle.Y + (offsetY + 4) + 10),
				Point(rectangle.X, rectangle.Y + (offsetY + 4) + 5) };
			g->FillPolygon(&brush, points, 3);
		}
		// Draw callsign
		string line = string(ac->Callsign);
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 140;

		// Draw entry point
		line = ac->Point;		
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 70;

		// Draw estimated time
		line = ac->Estimate;
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 45;

		// Draw altitude
		//line = to_string(fp.GetControllerAssignedData().GetClearedAltitude());
		line = to_string(ac->FinalAltitude / 100);
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());		
		offsetX += 50;

		// Draw destination
		line = string(ac->Destination);
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 45;

		// Direction arrow (Gander)
		if (ac->Direction == true) {
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

	/// Make "Inbound" text
	// Font
	FontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);
	// Draw header but don't show size if none
	if (AircraftList.size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Inbound");
	}
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Inbound (" + to_string(AircraftList.size()) + ")").c_str());
	}

	// Restore device context
	dc->RestoreDC(sDC);

	// Get object area and add object to screen
	CRect area(topleft.x, topleft.y, topleft.x + LIST_INBOUND_WIDTH, topleft.y + 14);
	screen->AddScreenObject(LIST_INBOUND, "", area, true, "");
}
