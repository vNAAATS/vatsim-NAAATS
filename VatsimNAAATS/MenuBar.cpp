#include "pch.h"
#include "MenuBar.h"
#include "Constants.h"
#include "MenuBar.h"
#include "Styles.h"
#include "Utils.h"
#include "CommonRenders.h"

using namespace Colours;

CMenuBar::CMenuBar() {
	// Button defaults
	buttons[BTN_SETUP] = CWinButton(BTN_SETUP, MENBAR, "Setup", CInputState::DISABLED, 46);
	buttons[BTN_NOTEPAD] = CWinButton(BTN_NOTEPAD, MENBAR, "NotePad", CInputState::DISABLED, 78);
	buttons[BTN_ADSC] = CWinButton(BTN_ADSC, MENBAR, "Contracts", CInputState::DISABLED, 73);
	buttons[BTN_TCKINFO] = CWinButton(BTN_TCKINFO, MENBAR, "Track Info", CInputState::INACTIVE, 78);
	buttons[BTN_MISC] = CWinButton(BTN_MISC, MENBAR, "Misc", CInputState::DISABLED, 41);
	buttons[BTN_MESSAGE] = CWinButton(BTN_MESSAGE, MENBAR, "Message", CInputState::INACTIVE, 73);
	buttons[BTN_TAGS] = CWinButton(BTN_TAGS, MENBAR, "Tags", CInputState::INACTIVE, 46);
	buttons[BTN_FLIGHTPLAN] = CWinButton(BTN_FLIGHTPLAN, MENBAR, "Flight Plan", CInputState::INACTIVE, 78);
	buttons[BTN_DETAILED] = CWinButton(BTN_DETAILED, MENBAR, "Detailed", CInputState::INACTIVE, 73);
	buttons[BTN_AREASEL] = CWinButton(BTN_AREASEL, MENBAR, "Area Sel", CInputState::INACTIVE, 83);
	buttons[BTN_TCKCTRL] = CWinButton(BTN_TCKCTRL, MENBAR, "Tck Control", CInputState::INACTIVE, 88);
	buttons[BTN_OVERLAYS] = CWinButton(BTN_OVERLAYS, MENBAR, "Overlays", CInputState::INACTIVE, 73);
	buttons[BTN_TYPESEL] = CWinButton(BTN_TYPESEL, MENBAR, "Select", CInputState::INACTIVE, 68);
	buttons[BTN_ALTFILT] = CWinButton(BTN_ALTFILT, MENBAR, "Alt Filter", CInputState::INACTIVE, 86);
	buttons[BTN_HALO] = CWinButton(BTN_HALO, MENBAR, "Halo 5", CInputState::INACTIVE, 68, 0);
	buttons[BTN_RBL] = CWinButton(BTN_RBL, MENBAR, "RBL", CInputState::INACTIVE, 48);
	buttons[BTN_RINGS] = CWinButton(BTN_RINGS, MENBAR, "Rings 1", CInputState::INACTIVE, 73, 0);
	buttons[BTN_PTL] = CWinButton(BTN_PTL, MENBAR, "PTL 5", CInputState::INACTIVE, 68, 0);
	buttons[BTN_PIV] = CWinButton(BTN_PIV, MENBAR, "PIV", CInputState::INACTIVE, 48);
	buttons[BTN_GRID] = CWinButton(BTN_GRID, MENBAR, "Grid", CInputState::INACTIVE, 73);
	buttons[BTN_SEP] = CWinButton(BTN_SEP, MENBAR, "Sep", CInputState::INACTIVE, 43);
	buttons[BTN_QCKLOOK] = CWinButton(BTN_QCKLOOK, MENBAR, "Qck Look", CInputState::INACTIVE, 86);

	/// Dropdown defaults
	map<string, bool> map;
	map.insert(make_pair("CZQX", false));
	map.insert(make_pair("EGGX", false));
	map.insert(make_pair("BDBX", false));
	dropDowns[DRP_AREASEL] = CDropDown(DRP_AREASEL, MENBAR, "CZQX", &map, CInputState::INACTIVE);
	map.clear();
	// TODO: Get tracks
	dropDowns[DRP_TCKCTRL] = CDropDown(DRP_TCKCTRL, MENBAR, "None", &map, CInputState::INACTIVE);
	map.clear();
	map.insert(make_pair("ALL_TCKS", false));
	map.insert(make_pair("TCKS_EAST", false));
	map.insert(make_pair("TCKS_WEST", false));
	map.insert(make_pair("TCKS_SEL", false));
	map.insert(make_pair("TCKS_ACTV", false));
	dropDowns[DRP_OVERLAYS] = CDropDown(DRP_OVERLAYS, MENBAR, "ALL_TCKS", &map, CInputState::INACTIVE);
	map.clear();
	map.insert(make_pair("Delivery", false));
	map.insert(make_pair("OCA Enroute", false));
	map.insert(make_pair("Multi-role", false));
	dropDowns[DRP_TYPESEL] = CDropDown(DRP_TYPESEL, MENBAR, "Multi-role", &map, CInputState::INACTIVE);
}

void CMenuBar::RenderBar(CDC* dc, Graphics* g, CRadarScreen* screen, string asel) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Brush to draw the bar
	CBrush brush(ScreenBlue.ToCOLORREF());

	// Get screen width
	RECT radarArea = screen->GetRadarArea();
	LONG screenWidth = radarArea.left + radarArea.right;

	// Create the base rectangle and the 3d bevel
	CRect baseMenuRectColour(radarArea.left, radarArea.top, radarArea.left + screenWidth, MENBAR_HEIGHT);
	dc->FillRect(baseMenuRectColour, &brush);
	CRect baseMenuRect(radarArea.left, radarArea.top, radarArea.left + screenWidth, MENBAR_HEIGHT);
	dc->Draw3dRect(baseMenuRect, ScreenBlue.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Draw the panels
	int menuOffsetX = 0;
	for (int i = 0; i <= sizeof(PANEL_SIZES) / sizeof(PANEL_SIZES[0]); i++) {
		CRect rect1(menuOffsetX, radarArea.top + 1, menuOffsetX + PANEL_SIZES[i], MENBAR_HEIGHT - 2);
		dc->Draw3dRect(rect1, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		InflateRect(rect1, -1, -1);
		dc->Draw3dRect(rect1, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		menuOffsetX += PANEL_SIZES[i] + 1;
	}
	
	/// ITEM RENDERING
	// Font selection
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Calculate date
	time_t now = time(0);
	tm* date = gmtime(&now);
	string strDate;
	strDate += to_string(date->tm_mon + 1);
	strDate += "-";
	strDate += to_string(date->tm_mday);
	strDate += "-";
	strDate += to_string(1900 + date->tm_year);
	
	// Render time
	dc->TextOutA(radarArea.left + 64, radarArea.top + 12.5, strDate.c_str());

	// Buttons
	int idx = 0;
	int offsetX = 128;
	int offsetY = 30;
	string text = "";
	for (auto kv : buttons) {
		// Offsets
		bool offsetIsItemSize = false;
		switch (kv.first) {
			case BTN_MISC:
				offsetX = 10;
				offsetY += MENBAR_BTN_HEIGHT + 1;
				offsetIsItemSize = true;
				break;
			case BTN_AREASEL:
				offsetX = RECT1_WIDTH + 10;
				offsetY = 30;
				offsetIsItemSize = true;
				break;
			case BTN_TCKCTRL:
				offsetIsItemSize = false;
				break;
			case BTN_OVERLAYS:
				offsetX += 164;
				break;
			case BTN_TYPESEL:
				offsetX = offsetX = RECT1_WIDTH + RECT2_WIDTH + 85;
				break;
			case BTN_ALTFILT:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + 10;
				break;
			case BTN_HALO:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + 11;
				offsetY = 30;
				offsetIsItemSize = true;
				break;
			case BTN_PTL:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + 11;
				offsetY += MENBAR_BTN_HEIGHT + 1;
				offsetIsItemSize = true;
				break;
			case BTN_QCKLOOK:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + RECT5_WIDTH + 11;
				break;
			default:
				offsetIsItemSize = true;
				break;
		}

		// Buttons
		CCommonRenders::RenderButton(dc, screen, { offsetX, offsetY }, kv.second.Width, 30, &kv.second);

		// Misc items

		// If offsetting between buttons
		if (offsetIsItemSize) {
			offsetX += kv.second.Width + 1;
		}
		
		// Increment
		idx++;
	}
}

bool CMenuBar::IsButtonPressed(int id) {
	// If button pressed
	if (buttons.find(id) != buttons.end() && buttons[id].State == CInputState::ACTIVE) return true;

	return false; // Not pressed
}

string CMenuBar::GetDropDownValue(int id) {
	// Get the value
	return dropDowns[id].Value;
}

map<int, CWinButton> CMenuBar::GetToggleButtons() {
	map<int, CWinButton> map;
	map[BTN_PTL] = buttons[BTN_PTL];
	map[BTN_RINGS] = buttons[BTN_RINGS];
	map[BTN_HALO] = buttons[BTN_HALO];

	return map;
}

void CMenuBar::SetButtonState(int id, CInputState state) {
	// Set the state to the requested one (with failsafe check)
	if (buttons.find(id)->second.State != CInputState::DISABLED) {
		if (buttons.find(id) != buttons.end()) {
			buttons[id].State = state;
		}
	}
}

void CMenuBar::OnOverDropDownItem(int id) {
	// Reset the hover state of the old one and set the state of new one
	dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
	dropDowns[ActiveDropDown].Items[id].IsHovered = true;
	ActiveDropDownHover = id;
}

void CMenuBar::ButtonDown(int id, int button) {

}

void CMenuBar::ButtonUp(int id, int button) {

}

void CMenuBar::ButtonPress(int id, int button) {
	if (button == EuroScopePlugIn::BUTTON_LEFT) {
		// Finally, press the button
		SetButtonState(id, CInputState::ACTIVE);
	}
	else if (button == EuroScopePlugIn::BUTTON_RIGHT) {
		if (id == BTN_HALO) {
			// Get the toggle button
			auto haloBtn = buttons.find(BTN_HALO);

			// Increment if less than or equal 3 (20 minute halos max)
			if (haloBtn->second.Cycle < 3) {
				haloBtn->second.Cycle++;
			}
			else {
				haloBtn->second.Cycle = 0;
			}

			switch (haloBtn->second.Cycle) {
			case 0:
				haloBtn->second.Label = "Halo 5";
				break;
			case 1:
				haloBtn->second.Label = "Halo 10";
				break;
			case 2:
				haloBtn->second.Label = "Halo 15";
				break;
			case 3:
				haloBtn->second.Label = "Halo 20";
				break;
			}
		}

		if (id == BTN_PTL) {
			// Get the toggle button
			auto ptlBtn = buttons.find(BTN_PTL);

			// Increment if less than or equal 5 (30 minute lines max)
			if (ptlBtn->second.Cycle < 5) {
				ptlBtn->second.Cycle++;
			}
			else {
				ptlBtn->second.Cycle = 0;
			}
			switch (ptlBtn->second.Cycle) {
			case 0:
				ptlBtn->second.Label = "PTL 5";
				break;
			case 1:
				ptlBtn->second.Label = "PTL 10";
				break;
			case 2:
				ptlBtn->second.Label = "PTL 15";
				break;
			case 3:
				ptlBtn->second.Label = "PTL 20";
				break;
			case 4:
				ptlBtn->second.Label = "PTL 25";
				break;
			case 5:
				ptlBtn->second.Label = "PTL 30";
				break;
			}
		}

		if (id == BTN_RINGS) {
			// Get the toggle button
			auto ringsBtn = buttons.find(BTN_RINGS);

			// Increment if less than or equal 4 (5 rings max)
			if (ringsBtn->second.Cycle < 4) {
				ringsBtn->second.Cycle++;
			}
			else {
				ringsBtn->second.Cycle = 0;
			}

			switch (ringsBtn->second.Cycle) {
			case 0:
				ringsBtn->second.Label = "Rings 1";
				break;
			case 1:
				ringsBtn->second.Label = "Rings 2";
				break;
			case 2:
				ringsBtn->second.Label = "Rings 3";
				break;
			case 3:
				ringsBtn->second.Label = "Rings 4";
				break;
			case 4:
				ringsBtn->second.Label = "Rings 5";
				break;
			}
		}
	}
}

void CMenuBar::ButtonUnpress(int id, int button) {
	// Finally, unpress the button
	SetButtonState(id, CInputState::INACTIVE);
}