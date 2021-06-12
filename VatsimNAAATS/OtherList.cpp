#include "pch.h"
#include "OtherList.h"
#include "Constants.h"
#include "Styles.h"

using namespace Colours;

vector<string> COtherList::AircraftList;

COtherList::COtherList(POINT topLeft) : CBaseList(topLeft) {}

// TODO: implement expand/retract button
void COtherList::RenderList(Graphics* g, CDC* dc, CRadarScreen* screen) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Brush
	SolidBrush brush(TextWhite);

	// Make rectangle
	Rect rectangle(topleft.x, topleft.y, LIST_OTHERS_WIDTH, 500);

	/// Make "Others" text
	// Font
	CFontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Get object area and add object to screen
	CRect area(topleft.x, topleft.y, topleft.x + LIST_OTHERS_WIDTH, topleft.y + 14);
	screen->AddScreenObject(LIST_OTHERS, "", area, true, "");

	// Don't show size if none
	if (AircraftList.size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Others");
	}
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Others (" + to_string(AircraftList.size()) + ")").c_str());
		// Make dropdown button
		size_t textExtentY = dc->GetTextExtent("Others").cy;
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
		screen->AddScreenObject(LIST_OTHERS, "HIDESHOW", rect, false, "");
	}

	// Text
	CFontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Make text lines
	int offsetY = 14;
	int offsetX = 0;
	int idx = 1;

	// If list shown
	if (HideShowButton) {
		for (vector<string>::iterator cs = AircraftList.begin(); cs != AircraftList.end(); cs++) {
			// Draw number
			string line = to_string(idx);
			dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
			offsetX += 35;

			// Draw callsign
			line = *cs;
			dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());

			// Offset the y index and reset the X offset
			offsetY += 18;
			offsetX = 0;

			// Increment
			idx++;
		}
	}

	// Cleanup
	DeleteObject(&brush);
}