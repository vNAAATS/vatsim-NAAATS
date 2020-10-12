#include "pch.h"
#include "CommonRenders.h"
#include "Styles.h"

using namespace Colours;

void CCommonRenders::RenderButton(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CWinButton* obj) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Create rectangle
	CRect button(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Check if pressed
	if (obj->State == CInputState::ACTIVE) {
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
	FontSelector::SelectNormalFont(16, dc);
	if (obj->State == CInputState::DISABLED) { // Disabled text colour
		dc->SetTextColor(Disabled.ToCOLORREF());
	}
	else {
		dc->SetTextColor(TextWhite.ToCOLORREF());
	}
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(button.left + (button.Width() / 2), button.top + (button.Height() / 2), obj->Label.c_str());

	// Restore device context
	dc->RestoreDC(sDC);

	// Add object and return the rectangle
	screen->AddScreenObject(obj->Type, to_string(obj->Id).c_str(), button, false, "");
}

void CCommonRenders::RenderTextInput(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CTextInput* obj) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Select font
	FontSelector::SelectATCFont(14, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Create rectangle
	CRect rect(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Draw rectangle
	dc->FillSolidRect(rect, ScreenBlue.ToCOLORREF());
	// Button bevel
	dc->Draw3dRect(rect, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(rect, -1, -1);
	dc->Draw3dRect(rect, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Select text colour
	if (obj->State == CInputState::DISABLED) {
		dc->SetTextColor(Disabled.ToCOLORREF());
	}
	else {
		dc->SetTextColor(TextWhite.ToCOLORREF());
	}

	// Draw text
	dc->TextOutA(rect.left + 3, rect.top + 2, obj->Content.c_str());

	// Restore device context
	dc->RestoreDC(sDC);

	// If editable, then add object and return the rectangle
	if (obj->State == CInputState::ACTIVE) {
		screen->AddScreenObject(obj->Type, to_string(obj->Id).c_str(), rect, false, "");
	}
}
void CCommonRenders::RenderCheckBox(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CCheckBox* obj) {

}

void CCommonRenders::RenderDropDown(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CDropDown* obj) {

}