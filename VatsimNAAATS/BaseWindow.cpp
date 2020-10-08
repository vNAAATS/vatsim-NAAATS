#include "pch.h"
#include "BaseWindow.h"
#include "Styles.h"

using namespace Colours;

CBaseWindow::CBaseWindow(POINT topLeftPt) {
	// Set top left
	topLeft = topLeftPt;
}

POINT CBaseWindow::GetTopLeft() {
	return topLeft;
}

void CBaseWindow::MoveWindow(CRect area) {
	topLeft = { area.left, area.top };
}

CRect CBaseWindow::DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, bool isPressed, int type, string id)
{
	// Save context for later
	int sDC = dc->SaveDC();

	// Brushes
	CBrush btnNormal(ScreenBlue.ToCOLORREF());

	CBrush btnPressed(ButtonPressed.ToCOLORREF());

	// Create rectangle
	CRect button(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Check if pressed
	if (isPressed) {
		dc->FillSolidRect(button, ButtonPressed.ToCOLORREF());
		// Button bevel
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		InflateRect(button, -1, -1);
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	}
	else {
		dc->FillSolidRect(button, ScreenBlue.ToCOLORREF());
		// Button bevel
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		InflateRect(button, -1, -1);
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	}

	// Draw text
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(button.left + (button.Width() / 2), button.top + vtcAlign, text.c_str());

	// Restore device context
	dc->RestoreDC(sDC);

	// Delete objects
	DeleteObject(&btnNormal);
	DeleteObject(&btnPressed);

	// Add object and return the rectangle
	screen->AddScreenObject(type, id.c_str(), button, false, "");
	return button;
}