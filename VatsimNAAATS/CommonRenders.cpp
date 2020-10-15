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
	dc->TextOutA(button.left + (button.Width() / 2), button.top + ((button.bottom - button.top) / 4.5), obj->Label.c_str());

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

// TODO: checkmark items
void CCommonRenders::RenderDropDown(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, int width, int height, CDropDown* obj) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Create dropdown area
	CRect dropDown(topLeft.x, topLeft.y, topLeft.x + width - 15, topLeft.y + height);

	// Fill
	dc->FillSolidRect(dropDown, ScreenBlue.ToCOLORREF());

	// Select font
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Dropdown bevel
	dc->Draw3dRect(dropDown, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(dropDown, -1, -1);
	dc->Draw3dRect(dropDown, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Write selected text
	dc->TextOutA(dropDown.left + 2, dropDown.top + 1, obj->Value.c_str());

	// Create dropdown button
	CRect button(topLeft.x + width - 15, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Check if pressed
	if (obj->State == CInputState::ACTIVE) {
		// Draw text
		dc->FillSolidRect(button, ButtonPressed.ToCOLORREF());
		CRect area(dropDown.left, dropDown.bottom, dropDown.right, dropDown.bottom + (obj->Items.size() * 20) + 2);
		dc->FillSolidRect(area, ScreenBlue.ToCOLORREF());
		dc->Draw3dRect(area, BevelDark.ToCOLORREF(), BevelDark.ToCOLORREF());

		// Draw text
		int offsetY = 2;
		int idx = 0;
		for (auto kv : obj->Items) {
			CRect object(area.left, area.top + offsetY, area.right, area.top + offsetY + 20);
			if (kv.second.IsHovered)
				dc->FillSolidRect(object, ButtonPressed.ToCOLORREF());
			dc->TextOutA(area.left + 2, area.top + offsetY + 2, kv.second.Label.c_str());
			screen->AddScreenObject(kv.second.Type, to_string(kv.second.Id).c_str(), object, false, "");
			offsetY += 20;
			idx;
		}
	}
	else {
		// Button
		dc->FillSolidRect(button, ScreenBlue.ToCOLORREF());
	}

	// Button triangle
	SolidBrush brush(Grey);
	// Coz GDI+ doesn't like GDI
	Rect rectangle(topLeft.x + width - 15, topLeft.y, topLeft.x + width, topLeft.y + height);
	Point points[3] = { Point(rectangle.X + 3, rectangle.Y + 4),
		Point(rectangle.X + 12, rectangle.Y + 4),
		Point(rectangle.X + 7.5, rectangle.Y + 14) };
	g->FillPolygon(&brush, points, 3);
	// Button bevel
	dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(button, -1, -1);
	dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Restore device context
	dc->RestoreDC(sDC);

	// Clean up
	DeleteObject(&brush);

	// Add object and return dropdown
	screen->AddScreenObject(obj->Type, to_string(obj->Id).c_str(), button, false, "");
}