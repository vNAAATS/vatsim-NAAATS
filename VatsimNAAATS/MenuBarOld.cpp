#pragma once
#include "pch.h"
#include "Constants.h"
#include "MenuBar.h"
#include "Styles.h"
#include "Utils.h"

using namespace Colours;

map<int, string> CMenuBar::dropDownSelections = {
	{ MENDRP_AREASEL, "" }, 
	{ MENDRP_TCKCTRL, "" },
	{ MENDRP_OVERLAYS, "" },
	{ MENDRP_TYPESEL, "" }
};

map<int, string> CMenuBar::selectedTracks;

map<int, string> CMenuBar::dropDownItems;
int CMenuBar::currentDropDownId = -1;
int CMenuBar::dropDownHover = -1;
int CMenuBar::dropDownClicked = -1;

map<int, string> CMenuBar::BuildButtonData() {
	map<int, string> data;

	data[MENBTN_SETUP] = "Setup";
	data[MENBTN_NOTEPAD] = "NotePad";
	data[MENBTN_ADSC] = "Contracts";
	data[MENBTN_TCKINFO] = "Track Info";
	data[MENBTN_MISC] = "Misc";
	data[MENBTN_MESSAGE] = "Message";
	data[MENBTN_TAGS] = "Tags";
	data[MENBTN_FLIGHTPLAN] = "Flight Plan";
	data[MENBTN_DETAILED] = "Detailed";
	data[MENBTN_AREASEL] = "Area Sel";
	data[MENBTN_TCKCTRL] = "Tck Control";
	data[MENBTN_OVERLAYS] = "Overlays";
	data[MENBTN_TYPESEL] = "Select";
	data[MENBTN_ALTFILT] = "Alt Filter";
	data[MENBTN_HALO] = "Halo 5";
	data[MENBTN_RBL] = "RBL";
	data[MENBTN_RINGS] = "Rings 1";
	data[MENBTN_MTT] = "MTT";
	data[MENBTN_PTL] = "PTL 5";
	data[MENBTN_PIV] = "PIV";
	data[MENBTN_GRID] = "Grid";
	data[MENBTN_SEP] = "Sep";
	data[MENBTN_POS] = "CX-BBX";
	data[MENBTN_QCKLOOK] = "Qck Look";
	data[MENDRP_AREASEL] = "";
	data[MENDRP_TCKCTRL] = "";
	data[MENDRP_OVERLAYS] = "";
	data[MENDRP_TYPESEL] = "";

	return data;
}

map<int, int> CMenuBar::BuildToggleButtonData() {
	map<int, int> data;

	data[MENBTN_HALO] = 0;
	data[MENBTN_RINGS] = 0;
	data[MENBTN_PTL] = 0;

	return data;
}

string CMenuBar::ParseDropDownId(int id, int type) {
	if (type == MENDRP_AREASEL) {
		if (id == DRP_AREA_EGGX) {
			return string("EGGX");
		}
		else if (id == DRP_AREA_CZQX) {
			return string("CZQX");
		}
		else {
			return string("BDBX");
		}
	}
	else if (type == MENDRP_OVERLAYS) {
		if (id == DRP_OVL_ALL) {
			return string("ALL_TCKS");
		}
		else if (id == DRP_OVL_EAST) {
			return string("TCKS_EAST");
		}
		else if (id == DRP_OVL_WEST) {
			return string("TCKS_WEST");
		}
		else {
			return string("TCKS_SEL");
		}
	}
	else if (type == MENDRP_TYPESEL) {
		if (id == DRP_TYPE_DEL) {
			return string("Delivery");
		}
		else if (id == DRP_TYPE_ENR) {
			return string("OCA Enroute");
		}
		else {
			return string("Multi-Role");
		}
	}
}

void CMenuBar::DrawMenuBar(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, map<int, string>* btnData, map<int, bool>* pressedData, map<int, int>* toggleData) {

	// Brush to draw the bar
	CBrush brush(ScreenBlue.ToCOLORREF());

	// Get screen width
	LONG screenWidth = screen->GetRadarArea().left + screen->GetRadarArea().right;

	// Create the base rectangle and the 3d bevel
	CRect baseMenuRectColour(topLeft.x, topLeft.y, topLeft.x + screenWidth, MENBAR_HEIGHT);
	dc->FillRect(baseMenuRectColour, &brush);
	CRect baseMenuRect(topLeft.x, topLeft.y, topLeft.x + screenWidth, MENBAR_HEIGHT);
	dc->Draw3dRect(baseMenuRect, ScreenBlue.ToCOLORREF(), BevelLight.ToCOLORREF());
	
	// Offset for menu rectangles
	int menuOffsetX = topLeft.x;

	// Create the first section
	CRect rect1(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT1_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect1, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect1, -1, -1);
	dc->Draw3dRect(rect1, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Calculate date
	time_t now = time(0);
	tm* date = gmtime(&now);
	string strDate;
	strDate += to_string(date->tm_mon + 1);
	strDate += "-";
	strDate += to_string(date->tm_mday);
	strDate += "-";
	strDate += to_string(1900 + date->tm_year);
	int sDC = dc->SaveDC();

	// Font stuff
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(topLeft.x + 64, topLeft.y + 12.5, strDate.c_str());

	// Offset for the next rectangle
	menuOffsetX = RECT1_WIDTH + 1;

	// Create rectangle
	CRect rect2(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT2_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect2, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect2, -1, -1);
	dc->Draw3dRect(rect2, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT2_WIDTH + 1;

	// Create rectangle
	CRect rect3(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT3_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect3, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect3, -1, -1);
	dc->Draw3dRect(rect3, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT3_WIDTH + 1;

	// Create rectangle
	CRect rect4(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT4_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect4, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect4, -1, -1);
	dc->Draw3dRect(rect4, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT4_WIDTH + 1;

	// Create rectangle
	CRect rect5(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT5_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect5, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect5, -1, -1);
	dc->Draw3dRect(rect5, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT5_WIDTH + 1;

	// Create rectangle
	CRect rect6(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT6_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect6, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect6, -1, -1);
	dc->Draw3dRect(rect6, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT6_WIDTH + 1;

	// Create rectangle
	CRect rect7(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT7_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect7, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect7, -1, -1);
	dc->Draw3dRect(rect7, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT7_WIDTH + 1;

	// Create rectangle
	CRect rect8(menuOffsetX, topLeft.y + 1, menuOffsetX + RECT8_WIDTH, MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect8, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect8, -1, -1);
	dc->Draw3dRect(rect8, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	menuOffsetX += RECT8_WIDTH + 1;

	// Create end rectangle by calculating width left on screen
	CRect rect9(menuOffsetX, topLeft.y + 1, menuOffsetX + (screenWidth - menuOffsetX), MENBAR_HEIGHT - 2);
	dc->Draw3dRect(rect9, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(rect9, -1, -1);
	dc->Draw3dRect(rect9, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Render zulu time
	FontSelector::SelectNormalFont(30, dc);
	dc->TextOutA(screen->GetRadarArea().right / 2, MENBAR_HEIGHT + 5, CUtils::ParseZuluTime(true).c_str());

	// Create buttons
	// TODO: comment
	int offsetX = 128;
	int offsetY = 30;
	int idx = 0;
	for (auto kv : *btnData) {
		int btnWidth = 0;
		switch (kv.first) {
		case MENBTN_SETUP:
		case MENBTN_TAGS:
			btnWidth = 38 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_NOTEPAD:
		case MENBTN_FLIGHTPLAN:
			btnWidth = 70 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_ADSC:
		case MENBTN_DETAILED:
			btnWidth = 65 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_TCKINFO:
			btnWidth = 70 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_MISC:
			btnWidth = 35 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_MESSAGE:
			btnWidth = 65 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_AREASEL:
		case MENDRP_AREASEL:
			btnWidth = 75 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_TCKCTRL:
		case MENDRP_TCKCTRL:
			btnWidth = 80 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_OVERLAYS:
		case MENDRP_OVERLAYS:
			btnWidth = 65 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_TYPESEL:
		case MENDRP_TYPESEL:
			btnWidth = 60 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_ALTFILT:
		case MENTXT_ALTFILT:
			btnWidth = 78 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_HALO:
		case MENBTN_PTL:
			btnWidth = 60 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_RBL:
		case MENBTN_PIV:
			btnWidth = 40 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_RINGS:
		case MENBTN_GRID:
			btnWidth = 65 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_MTT:
		case MENBTN_SEP:
			btnWidth = 35 + (BTN_PAD_SIDE * 2);
			break;
		case MENBTN_POS:
		case MENBTN_QCKLOOK:
			btnWidth = 78 + (BTN_PAD_SIDE * 2);
			break;
		}

		if (idx == 4)
		{
			offsetX = 10;
			offsetY += MENBAR_BTN_HEIGHT + 1;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			offsetX += btnWidth + 1;
			
		}
		else if (idx < 9) {
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			offsetX += btnWidth + 1;
		}
		else if (idx == 9) {
			offsetX += 7;
			FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
			dc->SetTextColor(TextWhite.ToCOLORREF());
			string asel = string(screen->GetPlugIn()->FlightPlanSelectASEL().GetCallsign());			
			dc->TextOutA(offsetX, offsetY + BTN_PAD_TOP + 1.5, string("ASEL: " + (asel != "" ? asel : "NONE")).c_str()); // TODO: interpolate ASEL
			offsetY = 30;
			offsetX = RECT1_WIDTH + 10;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			offsetX += btnWidth + 1;
		}
		else if (idx == 10) {
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			offsetX += (btnWidth * 1.41);
		}
		else if (idx == 11) {
			FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
			dc->SetTextColor(TextWhite.ToCOLORREF());
			dc->TextOutA(offsetX, offsetY + BTN_PAD_TOP, "Map");
			offsetX += 40;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
		}
		else if (idx == 12) {
			FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
			dc->SetTextColor(TextWhite.ToCOLORREF());
			offsetX = RECT1_WIDTH + RECT2_WIDTH + 10;
			dc->TextOutA(offsetX, offsetY + BTN_PAD_TOP, "Pos Type");
			offsetX += 75;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
		}
		else if (idx == 13) {
			offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + 10;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			// Format the text
			offsetY += MENBAR_BTN_HEIGHT + 1;
			string lowAlt = CUtils::PadWithZeros(3, CUtils::AltFiltLow);
			string highAlt = CUtils::PadWithZeros(3, CUtils::AltFiltHigh);
			DrawMenuBarButton(dc, screen, { offsetX, offsetY }, make_pair(428, string(lowAlt + "-" + highAlt)), 
				btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + 11;
			offsetY = 30;
		}
		else if (idx < 18) {
			if (kv.first != MENBTN_MTT) {
				if (pressedData->find(kv.first) != pressedData->end()) {
					DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
				}
				else {
					DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
				}
			}
			offsetX += btnWidth + 1;
		}
		else if (idx == 18)
		{
			offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + 11;
			offsetY += MENBAR_BTN_HEIGHT + 1;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			offsetX += btnWidth + 1;
		}
		else if (idx < 22) {
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
			offsetX += btnWidth + 1;
		}
		else if (idx == 22) {
			offsetY = 30;
			offsetX = RECT1_WIDTH + RECT2_WIDTH + RECT3_WIDTH + RECT4_WIDTH + RECT5_WIDTH + 11;
			DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, true);
		}
		else if (idx == 23) {
			offsetY += MENBAR_BTN_HEIGHT + 1;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, true, false);
			}
			else {
				DrawMenuBarButton(dc, screen, { offsetX, offsetY }, kv, btnWidth, MENBAR_BTN_HEIGHT, BTN_PAD_TOP, { 0, 0 }, true, false, false);
			}
		}
		else if (idx == 24) {
			offsetX = RECT1_WIDTH + 10;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, btnWidth - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, true, 400 + idx);
			}
			else {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, btnWidth - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, false, 400 + idx);
			}
			offsetX += btnWidth + 1;
		}
		else if (idx == 25) {
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, btnWidth - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, true, 400 + idx);
			}
			else {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, btnWidth - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, false, 400 + idx);
			}
			offsetX += (btnWidth * 1.41);
		}
		else if (idx == 26) {
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, (btnWidth + 40) - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, true, 400 + idx);
			}
			else {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, (btnWidth + 40) - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, false, 400 + idx);
			}
		}
		else if (idx == 27) {
			offsetX = RECT1_WIDTH + RECT2_WIDTH + 10;
			if (pressedData->find(kv.first) != pressedData->end()) {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, (btnWidth + 75) - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, true, 400 + idx);
			}
			else {
				DrawDropDown(dc, g, screen, { offsetX, offsetY + 8 }, kv, (btnWidth + 75) - 15, MENBAR_BTN_HEIGHT / 1.5, BTN_PAD_TOP, { 0, 0 }, false, 400 + idx);
			}
		}

		dc->RestoreDC(sDC);
		idx++;
	}

	// Delete object
	DeleteObject(&brush);
}

CRect CMenuBar::DrawMenuBarButton(CDC* dc, CRadarScreen* screen, POINT topLeft, pair<int, string> kv, int width, int height, int vtcAlign, POINT mousePointer, bool isCentred, bool isPressed, bool isPosActive)
{
	// Save context for later
	int sDC = dc->SaveDC();

	// Brushes
	CBrush btnNormal(ScreenBlue.ToCOLORREF());
	
	CBrush btnPressed(ButtonPressed.ToCOLORREF());

	// Create rectangle
	CRect button(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Check if pressed
	if (isPressed && kv.first != 428) {
		if (isPosActive) {
			dc->FillSolidRect(button, LightGreen.ToCOLORREF());
			// Button bevel
			dc->Draw3dRect(button, GreenBevelLight.ToCOLORREF(), GreenBevelDark.ToCOLORREF());
			InflateRect(button, -1, -1);
			dc->Draw3dRect(button, GreenBevelLight.ToCOLORREF(), GreenBevelDark.ToCOLORREF());
		}
		else {
			dc->FillSolidRect(button, ButtonPressed.ToCOLORREF());
			// Button bevel
			dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
			InflateRect(button, -1, -1);
			dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		}
	}
	else {
		if (kv.first == 428) {
			dc->FillSolidRect(button, ButtonPressed.ToCOLORREF());
			// Button bevel
			dc->Draw3dRect(button, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
			InflateRect(button, -1, -1);
			dc->Draw3dRect(button, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
			/// Add screen objects for text
			// Low
			CSize rectSize = dc->GetTextExtent("000");
			CRect lowRect(button.left + 14, button.top + vtcAlign, button.left + 16 + rectSize.cx, button.top + vtcAlign + rectSize.cy);
			screen->AddScreenObject(TXT_ENTRY, "ALTFILT_LOW", lowRect, false, "");
			// High
			CRect highRect(button.right - 16 - rectSize.cx, button.top + vtcAlign, button.right - 14, button.top + vtcAlign + rectSize.cy);
			screen->AddScreenObject(TXT_ENTRY, "ALTFILT_HIGH", highRect, false, "");
		}
		else if (isPosActive) {
			dc->FillSolidRect(button, LightGreen.ToCOLORREF());
			// Button bevel
			dc->Draw3dRect(button, GreenBevelLight.ToCOLORREF(), GreenBevelDark.ToCOLORREF());
			InflateRect(button, -1, -1);
			dc->Draw3dRect(button, GreenBevelLight.ToCOLORREF(), GreenBevelDark.ToCOLORREF());
		}
		else {
			dc->FillSolidRect(button, ScreenBlue.ToCOLORREF());
			// Button bevel
			dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
			InflateRect(button, -1, -1);
			dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		}
	}

	// Draw text
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	
	// Check centred
	if (isCentred) {
		dc->SetTextAlign(TA_CENTER);
		dc->TextOutA(button.left + (button.Width() / 2), button.top + vtcAlign, kv.second.c_str());
	}
	else {
		dc->TextOutA(topLeft.x + BTN_PAD_SIDE, topLeft.y + BTN_PAD_TOP, kv.second.c_str());
	}
	
	// Restore device context
	dc->RestoreDC(sDC);

	// Delete objects
	DeleteObject(&btnNormal);
	DeleteObject(&btnPressed);

	// Add object and return the rectangle
	if (kv.first != 428) { // If not the altitude filter
		screen->AddScreenObject(kv.first, "", button, false, "");
	}
	return button;
}

CRect CMenuBar::DrawDropDown(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, pair<int, string> kv, int width, int height, int vtcAlign, POINT mousePointer, bool isOpen, int dpId) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Create dropdown area
	CRect dropDown(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Fill
	dc->FillSolidRect(dropDown, ScreenBlue.ToCOLORREF());

	// Select font
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());

	// Dropdown bevel
	dc->Draw3dRect(dropDown, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(dropDown, -1, -1);
	dc->Draw3dRect(dropDown, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Write selected text
	dc->TextOutA(dropDown.left + 2, dropDown.top + 1, dropDownSelections.at(dpId).c_str());

	// Create dropdown button
	CRect button(topLeft.x + width, topLeft.y, topLeft.x + width + 15, topLeft.y + height);

	// Check if pressed
	if (isOpen) {
		// Draw text
		dc->FillSolidRect(button, ButtonPressed.ToCOLORREF());
		CRect area(dropDown.left, dropDown.bottom, dropDown.right, dropDown.bottom + (dropDownItems.size() * 20) + 2);
		dc->FillSolidRect(area, ScreenBlue.ToCOLORREF());
		dc->Draw3dRect(area, BevelDark.ToCOLORREF(), BevelDark.ToCOLORREF());

		// Draw text
		int offsetY = 2;
		for (auto kv : dropDownItems) {
			CRect object(area.left, area.top + offsetY, area.right, area.top + offsetY + 20);
			if (dropDownHover == kv.first)
				dc->FillSolidRect(object, ButtonPressed.ToCOLORREF());
			dc->TextOutA(area.left + 2, area.top + offsetY + 2, kv.second.c_str());
			screen->AddScreenObject(DROPDOWN, to_string(kv.first).c_str(), object, false, "");	
			offsetY += 20;
		}
	}
	else {
		// Button
		dc->FillSolidRect(button, ScreenBlue.ToCOLORREF());
	}

	// Button triangle
	SolidBrush brush(Grey);
	// Coz GDI+ doesn't like GDI
	Rect rectangle(topLeft.x + width, topLeft.y, topLeft.x + width + 15, topLeft.y + height);
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
	screen->AddScreenObject(kv.first, "", button, false, "");
	return dropDown;
}