#include "pch.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"
#include "ConflictList.h"

using namespace Colours;

CConflictList::CConflictList(POINT topLeft) : CBaseList(topLeft) {}

// TODO: implement expand/retract button
void CConflictList::RenderList(Graphics* g, CDC* dc, CRadarScreen* screen) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Make rectangle
	Rect rectangle(topleft.x, topleft.y, LIST_CONFLICT_WIDTH, 500);

	/// Make "Conflict" text
	// Font
	FontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Don't show size if none
	if (CConflictDetection::CurrentSTCA.size() == 0) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Conflict");
	}
	else {
		dc->TextOutA(rectangle.X, rectangle.Y, string("Conflict (" + to_string(CConflictDetection::CurrentSTCA.size()) + ")").c_str());
	}

	// Text
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextAlign(TA_LEFT);

	// Make text lines
	int offsetY = 14;
	int offsetX = 0;
	for (auto item = CConflictDetection::CurrentSTCA.begin(); item != CConflictDetection::CurrentSTCA.end(); item++) {
		// Pick text colour
		COLORREF textColour;
		if (item->ConflictStatus == CConflictStatus::CRITICAL) {
			// Select red because critical conflict
			textColour = CriticalRedLight.ToCOLORREF();
		}
		else {
			// Select yellow because warning
			textColour = WarningYellow.ToCOLORREF();
		}

		// Text colour
		dc->SetTextColor(textColour);

		// Draw callsign A
		string line = item->CallsignA;
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 80;

		// Draw callsign B
		line = item->CallsignB;
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());
		offsetX += 90;

		// Draw status
		if (item->ConflictStatus == CConflictStatus::WARNING) { // Warning
			line = "W" + to_string(item->DistanceAsTime);
		}
		else { // Critical
			line = "C" + to_string(item->DistanceAsTime);
		}
		dc->TextOutA(rectangle.X + offsetX, rectangle.Y + offsetY, line.c_str());

		// Offset the y index and reset the X offset
		offsetY += 18;
		offsetX = 0;
	}

	// Get object area and add object to screen
	CRect area(topleft.x, topleft.y, topleft.x + LIST_CONFLICT_WIDTH, topleft.y + 14);
	screen->AddScreenObject(LIST_CONFLICT, "", area, true, "");
}