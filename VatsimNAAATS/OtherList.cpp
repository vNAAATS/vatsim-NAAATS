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
Rect COtherList::DrawList(Graphics* g, CDC* dc, CRadarScreen* screen, vector<CRadarTarget>* otherAircraft) {
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
	if (otherAircraft->size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Others");
	}
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Others (" + to_string(otherAircraft->size()) + ")").c_str());
	}


	// Make inbound lines
	int offsetY = 14;
	int offsetX = 18;
	int idx = 0;


	// Get object area and add object to screen
	CRect area(topLeft.x, topLeft.y, topLeft.x + LIST_OTHERS, topLeft.y + 14);
	screen->AddScreenObject(LIST_OTHERS, "", area, true, "");

	return rectangle;
}

void COtherList::MoveList(CRect area) { // TODO: check need for isReleased
	topLeft = { area.left, area.top };
}