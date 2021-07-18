#include "../../pch.h"
#include "UIButton.h"

using namespace Gdiplus;
using namespace User_Interface;
using namespace Styling;

void CUIButton::RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Create rectangle
	CRect button(TopLeft_.x, TopLeft_.y, TopLeft_.x + Config.Width, TopLeft_.y + Config.Height);

	// Styles
	COLORREF fillColor;
	COLORREF textColor;

	// Set display state
	if (this->State == EElementState::kActive) {
		fillColor = Colours::kUIDark.ToCOLORREF();
	}
	else {
		fillColor = Colours::kUI.ToCOLORREF();
	}

	// Set text color (white or the disabled colour)
	textColor = this->State != EElementState::kDisabled ? Colours::kWhite.ToCOLORREF() : Colours::kUIDisabled.ToCOLORREF();

	// Fill the rectangle
	dc->FillSolidRect(button, fillColor);
	// Button bevel
	dc->Draw3dRect(button, Colours::kUIBevelLight.ToCOLORREF(), Colours::kUIBevelDark.ToCOLORREF());
	InflateRect(button, -1, -1);
	dc->Draw3dRect(button, Colours::kUIBevelLight.ToCOLORREF(), Colours::kUIBevelDark.ToCOLORREF());

	// Align text in middle of the button
	POINT textPosition = { button.Width() / 2, button.Height() / 2 };

	// Write text
	Typefaces::CTextManager::RenderTextLabel(dc, Label_, textPosition, Config.Typeface, EAlignment::kCentre, textColor);

	// Restore context
	dc->RestoreDC(sDC);

	// Add screen object
	screen->AddScreenObject(LocID, to_string(ID).c_str(), button, false, "");

	// Cleanup
	DeleteObject(&fillColor);
	DeleteObject(&textColor);
}

void CUIButton::ButtonDown(int id) {

}

void CUIButton::ButtonUp(int id) {

}

void CUIButton::Cycle() {

}