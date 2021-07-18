#include "../../pch.h"
#include "MenuBar.h"
#include <gdiplus.h>

using namespace Gdiplus;
using namespace User_Interface;
using namespace Styling;

void CMenuBar::RegisterElements() {

}

void CMenuBar::RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Radar area and screen width
	RECT radarArea = screen->GetRadarArea();
	LONG screenWidth = radarArea.right;

	// Brush to draw the bar
	CBrush brush(Colours::kUI.ToCOLORREF());

	// Create the base rectangle and the 3d bevel
	CRect baseMenuRectColour(radarArea.left, radarArea.top, radarArea.left + screenWidth, Height_);
	dc->FillRect(baseMenuRectColour, &brush);

	// Draw the panels
	int offsetX = 0;
	for (int i = 0; i <= sizeof(kPanels_) / sizeof(kPanels_[0]); i++) {
		// We set the x coord if it isn't already set for everything after the first panel
		if (kPanels_[i].second.x == 0 || kPanels_[i].second.y == 0) {
			kPanels_[i].second.x = offsetX;
			kPanels_[i].second.y = radarArea.top;
		}
		// Let's draw it now
		CRect rect1(offsetX, radarArea.top + 1, offsetX + kPanels_[i].first, Height_ - 2);
		dc->Draw3dRect(rect1, Colours::kUIBevelDark.ToCOLORREF(), Colours::kUIBevelLight.ToCOLORREF());
		InflateRect(rect1, -1, -1);
		dc->Draw3dRect(rect1, Colours::kUIBevelDark.ToCOLORREF(), Colours::kUIBevelLight.ToCOLORREF());
		offsetX += kPanels_[i].first + 1;
	}

	// Clean up
	DeleteObject(&brush);

	// Restore context
	dc->RestoreDC(sDC);
}

void CMenuBar::RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Start by rendering the base
	RenderBase(dc, g, screen);

	// Let's store the coordinates for the panel currently being processed here
	POINT panelTopLeft;

	/// Let's now render panel by panel
	/// Panel 1
	panelTopLeft = kPanels_[0].second;
	// Calculate date
	time_t now = time(0);
	tm* date = gmtime(&now);
	string strDate;
	strDate += to_string(date->tm_mon + 1);
	strDate += "-";
	strDate += to_string(date->tm_mday);
	strDate += "-";
	strDate += to_string(1900 + date->tm_year);
	
	// Text label
	Typefaces::CTextManager::RenderTextLabel(dc, strDate, { panelTopLeft.x + 64, panelTopLeft.y + 12 }, Typefaces::ETypeface::kRegular16, EAlignment::kCentre);

	// Okie render buttons


	// Restore context
	dc->RestoreDC(sDC);
}

void CMenuBar::Interact(int id, EElementType type) {

}