#include "pch.h"
#include "MenuBar.h"
#include "Styles.h"

using namespace Colours;

map<int, string> MenuBar::BuildButtonData() {

}

void MenuBar::DrawMenuBar(Graphics* g, CRadarScreen* screen, POINT topLeft) {

	// Define a brush to draw the bar
	SolidBrush brush(Colours::ScreenBlue);
	
	// Get screen width
	LONG width = screen->GetRadarArea().left + screen->GetRadarArea().right;

	// Create rectangle
	g->FillRectangle(&brush, topLeft.x, topLeft.y, width, 100);
}