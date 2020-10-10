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

void CBaseWindow::DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, CInputState state, int type, string id)
{
	// Save context for later
	int sDC = dc->SaveDC();

	// Create rectangle
	CRect button(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Check if pressed
	if (state == CInputState::ACTIVE) {
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
	if (state == CInputState::DISABLED) { // Disabled text colour
		dc->SetTextColor(Disabled.ToCOLORREF());
	}
	else {
		dc->SetTextColor(TextWhite.ToCOLORREF());
	}
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(button.left + (button.Width() / 2), button.top + vtcAlign, text.c_str());

	// Restore device context
	dc->RestoreDC(sDC);

	// Add object and return the rectangle
	screen->AddScreenObject(type, id.c_str(), button, false, "");
}

void CBaseWindow::DrawTextInput(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, bool canEdit, CWinTextInput* obj, int type, string id) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Brushes
	
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

	// Delete objects

	// If editable, then add object and return the rectangle
	if (canEdit) {
		screen->AddScreenObject(type, id.c_str(), rect, false, "");
	}
}

void CBaseWindow::DrawCheckBox(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CWinCheckBox* obj, int type, string id) {

}

bool CBaseWindow::IsButton(int id) {
	// If exists return true
	if (WindowButtons.find(id) != WindowButtons.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsTextInput(int id) {
	// If exists return true
	if (TextInputs.find(id) != TextInputs.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsCheckBox(int id) {
	// If exists return true
	if (CheckBoxes.find(id) != CheckBoxes.end()) return true;
	return false; // It doesn't exist
}