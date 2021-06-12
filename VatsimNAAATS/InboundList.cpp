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

// TODO: Sort items by time, earliest first
void CInboundList::RenderList(Graphics* g, CDC* dc, CRadarScreen* screen)
{
	// Save context for later
	int sDC = dc->SaveDC();

	// Brush
	SolidBrush brush(TextWhite);

	// Make rectangle
	Rect rectangle(topleft.x, topleft.y, LIST_INBOUND_WIDTH, 500);
	
	// Text
	CFontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Make inbound lines
	int offsetY = 14;
	int offsetX = 18;
	int idx = 0;
	
	// Draw planes if not hidden
	if (HideShowButton) {
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
			line = CUtils::ConvertCoordinateFormat(ac->Point, 0);
			dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
			offsetX += 70;

			// Draw estimated time
			line = ac->Estimate;
			dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
			offsetX += 45;

			// Draw altitude
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
	}

	// Get object area and add object to screen
	CRect area(topleft.x, topleft.y, topleft.x + LIST_INBOUND_WIDTH, topleft.y + 14);
	screen->AddScreenObject(LIST_INBOUND, "", area, true, "");

	/// Make "Inbound" text
	// Font
	CFontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);
	// Draw header but don't show size if none
	if (AircraftList.size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Inbound");
	}
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Inbound (" + to_string(AircraftList.size()) + ")").c_str());
		// Make dropdown button
		size_t textExtentY = dc->GetTextExtent("Inbound").cy;
		if (HideShowButton) { // If show
			Point btn[3] = { Point(rectangle.X + 140, rectangle.Y),
					Point(rectangle.X + 128, rectangle.Y + textExtentY - 2),
					Point(rectangle.X + 152, rectangle.Y + textExtentY - 2) };
			g->FillPolygon(&brush, btn, 3);
		}
		else { // If hide
			Point btn[3] = { Point(rectangle.X + 140, rectangle.Y + textExtentY - 2),
					Point(rectangle.X + 128, rectangle.Y),
					Point(rectangle.X + 152, rectangle.Y) };
			g->FillPolygon(&brush, btn, 3);
		}

		// Add the button
		CRect rect(rectangle.X + 128, rectangle.Y, rectangle.X + 152, rectangle.Y + textExtentY - 2);
		screen->AddScreenObject(LIST_INBOUND, "HIDESHOW", rect, false, "");
	}	

	// Cleanup
	DeleteObject(&brush);

	// Restore device context
	dc->RestoreDC(sDC);
}
