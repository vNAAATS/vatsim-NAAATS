#include "pch.h"
#include "MenuBar.h"
#include "Constants.h"
#include "MenuBar.h"
#include "Styles.h"
#include "Utils.h"
#include "CommonRenders.h"
#include "Overlays.h"

using namespace Colours;

CMenuBar::CMenuBar() {
	// Button defaults
	buttons[BTN_SETUP] = CWinButton(BTN_SETUP, MENBAR, "Setup", CInputState::DISABLED, 46);
	buttons[BTN_NOTEPAD] = CWinButton(BTN_NOTEPAD, MENBAR, "NotePad", CInputState::DISABLED, 78);
	buttons[BTN_ADSC] = CWinButton(BTN_ADSC, MENBAR, "Contracts", CInputState::DISABLED, 73);
	buttons[BTN_TCKINFO] = CWinButton(BTN_TCKINFO, MENBAR, "Track Info", CInputState::INACTIVE, 78);
	buttons[BTN_MISC] = CWinButton(BTN_MISC, MENBAR, "Misc", CInputState::DISABLED, 41);
	buttons[BTN_MESSAGE] = CWinButton(BTN_MESSAGE, MENBAR, "Message", CInputState::DISABLED, 73);
	buttons[BTN_TAGS] = CWinButton(BTN_TAGS, MENBAR, "Tags", CInputState::ACTIVE, 46);
	buttons[BTN_FLIGHTPLAN] = CWinButton(BTN_FLIGHTPLAN, MENBAR, "Flight Plan", CInputState::DISABLED, 78);
	buttons[BTN_DETAILED] = CWinButton(BTN_DETAILED, MENBAR, "Detailed", CInputState::INACTIVE, 73);
	buttons[BTN_AREASEL] = CWinButton(BTN_AREASEL, MENBAR, "Area Sel", CInputState::DISABLED, 83);
	buttons[BTN_TCKCTRL] = CWinButton(BTN_TCKCTRL, MENBAR, "Tck Control", CInputState::INACTIVE, 88);
	buttons[BTN_OVERLAYS] = CWinButton(BTN_OVERLAYS, MENBAR, "Overlays", CInputState::INACTIVE, 73);
	buttons[BTN_TYPESEL] = CWinButton(BTN_TYPESEL, MENBAR, "Select", CInputState::DISABLED, 68);
	buttons[BTN_ALTFILT] = CWinButton(BTN_ALTFILT, MENBAR, "Alt Filter", CInputState::INACTIVE, 86);
	buttons[BTN_HALO] = CWinButton(BTN_HALO, MENBAR, "Halo 5", CInputState::INACTIVE, 68, 0);
	buttons[BTN_RBL] = CWinButton(BTN_RBL, MENBAR, "RBL", CInputState::INACTIVE, 48);
	buttons[BTN_RINGS] = CWinButton(BTN_RINGS, MENBAR, "Rings 1", CInputState::DISABLED, 73, 0);
	buttons[BTN_QDM] = CWinButton(BTN_QDM, MENBAR, "QDM", CInputState::INACTIVE, 43);
	buttons[BTN_PTL] = CWinButton(BTN_PTL, MENBAR, "PTL 5", CInputState::INACTIVE, 68, 0);
	buttons[BTN_PIV] = CWinButton(BTN_PIV, MENBAR, "PIV", CInputState::INACTIVE, 48);
	buttons[BTN_GRID] = CWinButton(BTN_GRID, MENBAR, "Grid", CInputState::DISABLED, 73);
	buttons[BTN_SEP] = CWinButton(BTN_SEP, MENBAR, "Sep", CInputState::INACTIVE, 43);
	buttons[BTN_QCKLOOK] = CWinButton(BTN_QCKLOOK, MENBAR, "Qck Look", CInputState::DISABLED, 86);
	buttons[BTN_PSSR] = CWinButton(BTN_PSSR, MENBAR, "PSR_SYMBOL", CInputState::ACTIVE, 40);
	buttons[BTN_EXT] = CWinButton(BTN_EXT, MENBAR, "Ext", CInputState::INACTIVE, 40);
	buttons[BTN_AUTOTAG] = CWinButton(BTN_AUTOTAG, MENBAR, "Auto Tag", CInputState::DISABLED, 75);
	buttons[BTN_ALL] = CWinButton(BTN_ALL, MENBAR, "ALL", CInputState::INACTIVE, 40);
	buttons[BTN_RTEDEL] = CWinButton(BTN_RTEDEL, MENBAR, "Rte Del", CInputState::INACTIVE, 75);

	// Text inputs
	textInputs[TXT_SEARCH] = CTextInput(TXT_SEARCH, MENBAR, "Search A/C: ", "", 100, CInputState::ACTIVE);

	/// Dropdown defaults
	map<string, bool> map;
	map.insert(make_pair("CZQX", false));
	map.insert(make_pair("EGGX", false));
	map.insert(make_pair("BDBX", false));
	dropDowns[DRP_AREASEL] = CDropDown(DRP_AREASEL, MENBAR, "CZQX", &map, CInputState::INACTIVE, 83);
	map.clear();
	dropDowns[DRP_TCKCTRL] = CDropDown(DRP_TCKCTRL, MENBAR, "None", &map, CInputState::INACTIVE, 88);
	map.clear();
	map.insert(make_pair("ALL_TCKS", false));
	map.insert(make_pair("TCKS_EAST", false));
	map.insert(make_pair("TCKS_WEST", false));
	map.insert(make_pair("TCKS_SEL", false));
	map.insert(make_pair("TCKS_ACTV", false));
	dropDowns[DRP_OVERLAYS] = CDropDown(DRP_OVERLAYS, MENBAR, "ALL_TCKS", &map, CInputState::INACTIVE, 113);
	map.clear();
	map.insert(make_pair("Delivery", false));
	map.insert(make_pair("OCA Enroute", false));
	map.insert(make_pair("Multi-role", false));
	dropDowns[DRP_TYPESEL] = CDropDown(DRP_TYPESEL, MENBAR, "Multi-role", &map, CInputState::INACTIVE, 143);

	// Clogger
	CLogger::Log(CLogType::NORM, "Finished instantiation.", "MENUBAR");
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
	// Render zulu time
	CFontSelector::SelectNormalFont(30, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->TextOutA(screen->GetRadarArea().right / 2, MENBAR_HEIGHT + 5, CUtils::ParseZuluTime(true).c_str());

	// Font selection
	CFontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
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
	
	// Render date
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
			case BTN_PSSR:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + RECT5_WIDTH + RECT6_WIDTH + RECT7_WIDTH + 35;
				offsetY = 30;
				offsetIsItemSize = true;
				break;
			case BTN_ALL:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + RECT5_WIDTH + RECT6_WIDTH + RECT7_WIDTH + 35;
				offsetY += MENBAR_BTN_HEIGHT + 1;
				break;
			case BTN_RTEDEL:
				offsetX += 82;
				offsetIsItemSize = true;
				break;
			default:
				offsetIsItemSize = true;
				break;
		}

		// Button rendering
		if (kv.first != BTN_AREASEL)
			CCommonRenders::RenderButton(dc, screen, { offsetX, offsetY }, kv.second.Width, 30, &kv.second);

		// Text alignment
		dc->SetTextAlign(TA_LEFT);

		// Text rendering
		if (kv.first == BTN_DETAILED) {
			text = "ASEL: " + (asel == "" ? "None" : asel);
			dc->TextOutA(offsetX + kv.second.Width + 4, offsetY + 7, text.c_str());
		}
		else if (kv.first == BTN_OVERLAYS) {
			text = "Map";
			dc->TextOutA(offsetX - (kv.second.Width / 2) - 2, offsetY + 7, text.c_str());
		}
		else if (kv.first == BTN_TYPESEL) {
			text = "Pos Type";
			dc->TextOutA(offsetX - kv.second.Width - 2, offsetY + 7, text.c_str());
		}
		else if (kv.first == BTN_AREASEL) {
			dc->SetTextAlign(TA_CENTER);
			text = "Area Sel";
			dc->TextOutA(offsetX + (kv.second.Width / 2), offsetY + 7, text.c_str());
		}

		// If offsetting between buttons
		if (offsetIsItemSize) {
			offsetX += kv.second.Width + 1;
		}
		
		// Increment
		idx++;
	}

	// Misc items
	offsetX = RECT1_WIDTH + 10;
	offsetY += 8;
	for (auto kv : dropDowns) {
		// Offsets
		bool offsetIsItemSize = true;
		switch (kv.first) {
			case DRP_OVERLAYS: 
				offsetX += 36;
				offsetIsItemSize = false;
				break;
			case DRP_TYPESEL:
				offsetX = RECT1_WIDTH + RECT2_WIDTH + 10;
				offsetIsItemSize = false;
				break;
		}

		// Render
		CCommonRenders::RenderDropDown(dc, g, screen, { offsetX, offsetY }, kv.second.Width, 20, &kv.second);

		// If offsetting between buttons
		if (offsetIsItemSize) {
			offsetX += kv.second.Width + 1;
		}
	}

	// Altitude filter (we only need this once so we draw it directly here)
	CRect altFilt(RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + 10, 61, RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + 10 + 86, 91);
	dc->FillSolidRect(altFilt, ButtonPressed.ToCOLORREF());
	// Button bevel
	dc->Draw3dRect(altFilt, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(altFilt, -1, -1);
	dc->Draw3dRect(altFilt, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	/// Add screen objects for text
	// Low
	CSize rectSize = dc->GetTextExtent("000");
	CRect lowRect(altFilt.left + 14, altFilt.top + 6, altFilt.left + 16 + rectSize.cx, altFilt.top + 6 + rectSize.cy);
	screen->AddScreenObject(ALTFILT_TEXT, "ALTFILT_LOW", lowRect, false, "");
	// High
	CRect highRect(altFilt.right - 16 - rectSize.cx, altFilt.top + 6, altFilt.right - 14, altFilt.top + 6 + rectSize.cy);
	screen->AddScreenObject(ALTFILT_TEXT, "ALTFILT_HIGH", highRect, false, "");
	
	// Text out
	string lowAlt = CUtils::PadWithZeros(3, CUtils::AltFiltLow);
	string highAlt = CUtils::PadWithZeros(3, CUtils::AltFiltHigh);
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(altFilt.left + (altFilt.Width() / 2), altFilt.top + 7, (lowAlt + "-" + highAlt).c_str());

	// Render the selection input
	dc->SetTextAlign(TA_LEFT);
	offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + RECT5_WIDTH + RECT6_WIDTH + 13;
	dc->TextOutA(offsetX, 30, textInputs[TXT_SEARCH].Label.c_str());
	CCommonRenders::RenderTextInput(dc, screen, { offsetX, dc->GetTextExtent("ABCD").cy + 35 }, textInputs[TXT_SEARCH].Width, 20, & textInputs[TXT_SEARCH]);

	// Clean up
	DeleteObject(&brush);

	// Restore
	dc->RestoreDC(sDC);
}

bool CMenuBar::IsButtonPressed(int id) {
	// If button pressed
	if (id >= 100) { // dropdown
		if (dropDowns.find(id) != dropDowns.end() && dropDowns[id].State == CInputState::ACTIVE) return true;
	}
	else {
		if (buttons.find(id) != buttons.end() && buttons[id].State == CInputState::ACTIVE) return true;
	}

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

void CMenuBar::MakeDropDownItems(int id) {
	if (id == DRP_TCKCTRL) {
		map<string, bool> map;
		for (auto kv : CRoutesHelper::CurrentTracks) {
			map.insert(make_pair(kv.first, true));
		}
		dropDowns[DRP_TCKCTRL].Items.clear();
		dropDowns[DRP_TCKCTRL].MakeItems(&map);
	}
}

void CMenuBar::SetButtonState(int id, CInputState state) {
	// Set the state to the requested one (with failsafe check)
	if (id >= 100) { // dropdown
		if (dropDowns.find(id)->second.State != CInputState::DISABLED) {
			if (dropDowns.find(id) != dropDowns.end()) {
				// Remove the old dropdown stuff
				if (ActiveDropDownHover != 0) {
					dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
					ActiveDropDownHover = 0;
				}
				if (ActiveDropDown != 0) {
					dropDowns[ActiveDropDown].State = CInputState::INACTIVE;
					ActiveDropDown = 0;
				}
				
				dropDowns[id].State = state;
				ActiveDropDown = state == CInputState::ACTIVE ? id : 0;
			}
		}
	}
	else { // normal button
		if (buttons.find(id) != buttons.end()) {
			buttons[id].State = state;
		}
	}
}

void CMenuBar::SetTextInput(int id, string value) {
	if (textInputs.find(id) != textInputs.end()) {
		textInputs[id].Content = value;
	}
}

CInputState CMenuBar::GetButtonState(int id) {
	if (id >= 100) { // dropdown
		if (dropDowns.find(id) != dropDowns.end()) {
			return dropDowns.find(id)->second.State;
		}
	}
	else {
		if (buttons.find(id) != buttons.end()) {
			return buttons.find(id)->second.State;
		}
	}
}

void CMenuBar::OnOverDropDownItem(int id) {
	// Reset the hover state of the old one and set the state of new one
	if (ActiveDropDownHover != 0) {
		dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
		dropDowns[ActiveDropDown].Items[id].IsHovered = true;
	}
	ActiveDropDownHover = id;
}

void CMenuBar::SetDropDownValue(int id, int value) {
	// Set the value
	dropDowns[id].Value = dropDowns[id].Items[value].Label;
}

void CMenuBar::ButtonDown(int id) {
	if (id == BTN_RTEDEL)
		SetButtonState(id, CInputState::ACTIVE);
}

void CMenuBar::ButtonUp(int id) {
	if (id == BTN_RTEDEL)
		SetButtonState(id, CInputState::INACTIVE);
}

void CMenuBar::ButtonPress(int id, int button, CRadarScreen* screen = nullptr) {
	if (button == EuroScopePlugIn::BUTTON_LEFT) {
		// Check if dropdown
		if (id >= 800) {
			if (!dropDowns[ActiveDropDown].Items[id].IsCheckItem) {
				// Set value
				dropDowns[ActiveDropDown].Value = dropDowns[ActiveDropDown].Items[id].Label;
				// Close drop down
				dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
				ActiveDropDownHover = 0;
				dropDowns[ActiveDropDown].State = CInputState::INACTIVE;
			} else {
				dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
				ActiveDropDownHover = 0;
				dropDowns[ActiveDropDown].Items[id].State = dropDowns[ActiveDropDown].Items[id].State == CInputState::INACTIVE ? CInputState::ACTIVE : CInputState::INACTIVE;
			}

			// Save values
			if (ActiveDropDown == DRP_AREASEL) {
				CUtils::AreaSelection = id;
			} else if (ActiveDropDown == DRP_OVERLAYS) {
				CUtils::SelectedOverlay = id;

				switch (id) {
					case 800: // ALL_TCKS
						COverlays::CurrentType = COverlayType::TCKS_ALL;
						break;
					case 801: // TCKS_ACTV
						COverlays::CurrentType = COverlayType::TCKS_ACTV;
						break;
					case 802: // TCKS_EAST
						COverlays::CurrentType = COverlayType::TCKS_EAST;
						break;
					case 803: // TCKS_SEL
						COverlays::CurrentType = COverlayType::TCKS_SEL;
						break;
					case 804: // TCKS_WEST
						COverlays::CurrentType = COverlayType::TCKS_WEST;
						break;
				}
			}
			else if (ActiveDropDown == DRP_TYPESEL) {
				CUtils::PosType = id;
			}

			// Reset drop down
			if (!dropDowns[ActiveDropDown].Items[id].IsCheckItem)
				ActiveDropDown = 0;
		}
		else {
			// Press the button
			if (GetButtonState(id) != CInputState::DISABLED && id != BTN_RTEDEL)
				SetButtonState(id, CInputState::ACTIVE);

			// Grid
			if (id == BTN_GRID) {
				COverlays::ShowHideGridReference(screen, true);
			}
		}
	}
	else if (button == EuroScopePlugIn::BUTTON_RIGHT) {
		if (id == BTN_HALO) {
			// Get the toggle button
			auto haloBtn = buttons.find(BTN_HALO);

			// Increment if less than or equal 7 (100 mile halos max)
			if (haloBtn->second.Cycle < 7) {
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
			case 4:
				haloBtn->second.Label = "Halo 25";
				break;
			case 5:
				haloBtn->second.Label = "Halo 30";
				break;
			case 6:
				haloBtn->second.Label = "Halo 60";
				break;
			case 7:
				haloBtn->second.Label = "Halo 100";
				break;
			}
		}

		if (id == BTN_PTL) {
			// Get the toggle button
			auto ptlBtn = buttons.find(BTN_PTL);

			// Increment if less than or equal 6 (60 minute lines max)
			if (ptlBtn->second.Cycle < 6) {
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
			case 6:
				ptlBtn->second.Label = "PTL 60";
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

void CMenuBar::ButtonUnpress(int id, int button, CRadarScreen* screen) {
	// Finally, unpress the button
	SetButtonState(id, CInputState::INACTIVE);

	// Grid
	if (id == BTN_GRID) {
		COverlays::ShowHideGridReference(screen, false);
	}
}

void CMenuBar::GetSelectedTracks(vector<string>& tracksVector) {
	for (auto idx : dropDowns[DRP_TCKCTRL].Items) {
		if (idx.second.State == CInputState::ACTIVE) {
			tracksVector.push_back(idx.second.Label);
		}
	}
}