#include "pch.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"
#include "RCLList.h"

using namespace Colours;

CRCLList::CRCLList(POINT topLeft) : CBaseList(topLeft) {

}

// TODO: implement expand/retract button
void CRCLList::RenderList(Graphics* g, CDC* dc, CRadarScreen* screen) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Make rectangle
	Rect rectangle(topleft.x, topleft.y, LIST_RCLS_WIDTH, 500);

	/// Make "RCLs" text
	// Font
	FontSelector::SelectMonoFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Don't show size if none
	//if ( ) {
	dc->TextOutA(rectangle.X, rectangle.Y, "RCLs");
	//}
	// {
		//dc->TextOutA(rectangle.X, rectangle.Y, string("Others (" + to_string(AircraftList.size()) + ")").c_str());
	//}

	// Get object area and add object to screen
	CRect area(topleft.x, topleft.y, topleft.x + LIST_RCLS_WIDTH, topleft.y + 14);
	screen->AddScreenObject(LIST_RCLS, "", area, true, "");
}