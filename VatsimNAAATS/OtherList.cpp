#include "pch.h"
#include "OtherList.h"
#include "Constants.h"
#include "Styles.h"

using namespace Colours;

COtherList::COtherList(POINT topLeftPt) {
	topLeft = topLeftPt;
}

POINT COtherList::GetTopLeft() {
	return topLeft;
}

// TODO: implement expand/retract button
Rect COtherList::DrawList(Graphics* g, CDC* dc, CRadarScreen* screen, vector<string>* otherAircraft) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Make rectangle
	Rect rectangle(topLeft.x, topLeft.y, 100, 500);

	/// Make "Others" text
	// Font
	FontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Don't show size if none
	if (otherAircraft->size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Others");
	}
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Others (" + to_string(otherAircraft->size()) + ")").c_str());
	}

	// Text
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Make text lines
	int offsetY = 14;
	int offsetX = 0;
	int idx = 1;

	for (vector<string>::iterator cs = otherAircraft->begin(); cs != otherAircraft->end(); cs++) {
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


	// Get object area and add object to screen
	CRect area(topLeft.x, topLeft.y, topLeft.x + 110, topLeft.y + (idx * 18));
	screen->AddScreenObject(LIST_OTHERS, "", area, true, "");

	// Return the rectangle
	return rectangle;
}

void COtherList::MoveList(CRect area) { // TODO: check need for isReleased
	topLeft = { area.left, area.top };
}

void COtherList::MoveList(POINT topleft) { // TODO: check need for isReleased
	topLeft = { topleft.x, topleft.y };
}