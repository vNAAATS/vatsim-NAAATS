#pragma once
#include "pch.h"
#include "Constants.h"
#include "MenuBar.h"
#include "Styles.h"

using namespace Colours;

/*map<int, string> MenuBar::BuildButtonData() {

}*/

void MenuBar::DrawMenuBar(CDC* dc, CRadarScreen* screen, POINT topLeft) {

	// Brush to draw the bar
	CBrush brush(ScreenBlue.ToCOLORREF());

	// Get screen width
	LONG screenWidth = screen->GetRadarArea().left + screen->GetRadarArea().right;

	// Create the base rectangle and the 3d bevel
	CRect baseMenuRectColour(topLeft.x, topLeft.y, topLeft.x + screenWidth, topLeft.y + 100);
	dc->FillRect(baseMenuRectColour, &brush);
	CRect baseMenuRect(topLeft.x, topLeft.y, topLeft.x + screenWidth, topLeft.y + 100);
	dc->Draw3dRect(baseMenuRect, ScreenBlue.ToCOLORREF(), BevelLight.ToCOLORREF());
	
	// Offset for menu rectangles
	int menuOffsetX;

	// Create rectangle
	CRect rect1(topLeft.x, topLeft.y + 1, topLeft.x + RECT1_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect1, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect1, -1, -1);
	dc->Draw3dRect(rect1, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX = RECT1_WIDTH + 1;

	// Create rectangle
	CRect rect2(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT2_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect2, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect2, -1, -1);
	dc->Draw3dRect(rect2, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT2_WIDTH + 1;

	// Create rectangle
	CRect rect3(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT3_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect3, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect3, -1, -1);
	dc->Draw3dRect(rect3, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT3_WIDTH + 1;

	// Create rectangle
	CRect rect4(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT4_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect4, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect4, -1, -1);
	dc->Draw3dRect(rect4, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT4_WIDTH + 1;

	// Create rectangle
	CRect rect5(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT5_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect5, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect5, -1, -1);
	dc->Draw3dRect(rect5, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT5_WIDTH + 1;

	// Create rectangle
	CRect rect6(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT6_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect6, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect6, -1, -1);
	dc->Draw3dRect(rect6, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT6_WIDTH + 1;

	// Create rectangle
	CRect rect7(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT7_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect7, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect7, -1, -1);
	dc->Draw3dRect(rect7, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT7_WIDTH + 1;

	// Create rectangle
	CRect rect8(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT8_WIDTH, topLeft.y + 98);
	dc->Draw3dRect(rect8, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect8, -1, -1);
	dc->Draw3dRect(rect8, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT8_WIDTH + 1;

	// Create end rectangle by calculating width left on screen
	CRect rect9(menuOffsetX, topLeft.y + 1, menuOffsetX + (screenWidth - menuOffsetX), topLeft.y + 98);
	dc->Draw3dRect(rect9, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect9, -1, -1);
	dc->Draw3dRect(rect9, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
}

/*static CRect DrawMenuBarButton(CDC* dc, POINT topLeft, string text, int width, int height, POINT mousePointer, bool isPressed) 
{

}*/