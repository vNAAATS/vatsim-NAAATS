#include "pch.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"
#include "ConflictList.h"

using namespace Colours;

CConflictList::CConflictList(POINT topLeft) : CBaseList(topLeft) {

}

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
	//if ( ) {
		dc->TextOutA(rectangle.X, rectangle.Y, "Conflict");
	//}
	// {
		//dc->TextOutA(rectangle.X, rectangle.Y, string("Others (" + to_string(AircraftList.size()) + ")").c_str());
	//}

	// Get object area and add object to screen
	CRect area(topleft.x, topleft.y, topleft.x + LIST_CONFLICT_WIDTH, topleft.y + 14);
	screen->AddScreenObject(LIST_CONFLICT, "", area, true, "");
}