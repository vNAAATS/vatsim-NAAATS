#include "pch.h"
#include "Lists.h"
#include "Constants.h"
#include "Styles.h"

using namespace Colours;

Rect Lists::DrawInboundList(Graphics* g, CDC* dc, CRadarScreen* screen, POINT topLeft, vector<pair<CRadarTarget, bool>>* inboundAircraft)
{
	// Brushes
	SolidBrush transparentBrush(Color(0, 0, 0, 0));
	
	// Save context for later
	int sDC = dc->SaveDC();

	// Make rectangle
	Rect rectangle(100, 150, LIST_INBOUND_WIDTH, 500);
	g->FillRectangle(&transparentBrush, rectangle);

	/// Make "Inbound" text
	// Font
	FontSelector::SelectMonoFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);
	dc->TextOutA(rectangle.X, rectangle.Y, string("Inbound (" + to_string(inboundAircraft->size()) + ")").c_str());

	// Make inbound lines
	int offsetY = 14;
	for (vector<pair<CRadarTarget, bool>>::iterator ac = inboundAircraft->begin(); ac != inboundAircraft->end(); ac++) {
		// Get the aircraft
		CFlightPlan fp(screen->GetPlugIn()->FlightPlanSelect(ac->first.GetCallsign()));

		FontSelector::SelectATCFont(MEN_FONT_SIZE, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);
		string line;
		if (ac->second == true) {
			line = string(fp.GetCallsign()) + "  >";
		}
		else {
			line = "<  " + string(fp.GetCallsign());
		}

		dc->TextOutA(rectangle.X, rectangle.Y + offsetY, line.c_str());

		offsetY += 14;
	}

	// Restore device context
	dc->RestoreDC(sDC);

	// Deallocate
	DeleteObject(&transparentBrush);

	return rectangle;
}