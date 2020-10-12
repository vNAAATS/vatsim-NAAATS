#include "pch.h"
#include "MenuBar.h"
#include "Constants.h"
#include "MenuBar.h"
#include "Styles.h"
#include "Utils.h"

using namespace Colours;

CMenuBar::CMenuBar() {
	// Button defaults
	buttons[BTN_SETUP] = CWinButton(BTN_SETUP, MENBAR, "Setup", CInputState::INACTIVE, 46);
	buttons[BTN_NOTEPAD] = CWinButton(BTN_NOTEPAD, MENBAR, "NotePad", CInputState::INACTIVE, 78);
	buttons[BTN_ADSC] = CWinButton(BTN_ADSC, MENBAR, "Contracts", CInputState::INACTIVE, 73);
	buttons[BTN_TCKINFO] = CWinButton(BTN_TCKINFO, MENBAR, "Track Info", CInputState::INACTIVE, 78);
	buttons[BTN_MISC] = CWinButton(BTN_MISC, MENBAR, "Misc", CInputState::INACTIVE, 41);
	buttons[BTN_MESSAGE] = CWinButton(BTN_MESSAGE, MENBAR, "Message", CInputState::INACTIVE, 73);
	buttons[BTN_TAGS] = CWinButton(BTN_TAGS, MENBAR, "Tags", CInputState::INACTIVE, 46);
	buttons[BTN_FLIGHTPLAN] = CWinButton(BTN_FLIGHTPLAN, MENBAR, "Flight Plan", CInputState::INACTIVE, 78);
	buttons[BTN_DETAILED] = CWinButton(BTN_DETAILED, MENBAR, "Detailed", CInputState::INACTIVE, 73);
	buttons[BTN_AREASEL] = CWinButton(BTN_AREASEL, MENBAR, "Area Sel", CInputState::INACTIVE, 83);
	buttons[BTN_TCKCTRL] = CWinButton(BTN_TCKCTRL, MENBAR, "Tck Control", CInputState::INACTIVE, 88);
	buttons[BTN_OVERLAYS] = CWinButton(BTN_OVERLAYS, MENBAR, "Overlays", CInputState::INACTIVE, 73);
	buttons[BTN_TYPESEL] = CWinButton(BTN_TYPESEL, MENBAR, "Select", CInputState::INACTIVE, 68);
	buttons[BTN_ALTFILT] = CWinButton(BTN_ALTFILT, MENBAR, "Alt Filter", CInputState::INACTIVE, 86);
	buttons[BTN_HALO] = CWinButton(BTN_HALO, MENBAR, "Halo ", CInputState::INACTIVE, 68);
	buttons[BTN_RBL] = CWinButton(BTN_RBL, MENBAR, "RBL", CInputState::INACTIVE, 48);
	buttons[BTN_RINGS] = CWinButton(BTN_RINGS, MENBAR, "Rings ", CInputState::INACTIVE, 73);
	buttons[BTN_PTL] = CWinButton(BTN_PTL, MENBAR, "PTL ", CInputState::INACTIVE, 68);
	buttons[BTN_PIV] = CWinButton(BTN_PIV, MENBAR, "PIV", CInputState::INACTIVE, 48);
	buttons[BTN_GRID] = CWinButton(BTN_GRID, MENBAR, "Grid", CInputState::INACTIVE, 73);
	buttons[BTN_SEP] = CWinButton(BTN_SEP, MENBAR, "Sep", CInputState::INACTIVE, 43);
	buttons[BTN_QCKLOOK] = CWinButton(BTN_QCKLOOK, MENBAR, "Qck Look", CInputState::INACTIVE, 86);

	/// Dropdown defaults
	map<string, bool> map;
	map.insert(make_pair("CZQX", false));
	map.insert(make_pair("EGGX", false));
	map.insert(make_pair("BDBX", false));
	dropDowns[DRP_AREASEL] = CDropDown(DRP_AREASEL, MENBAR, "CZQX", &map);
	map.clear();
	// TODO: Get tracks
	dropDowns[DRP_TCKCTRL] = CDropDown(DRP_AREASEL, MENBAR, "None", &map);
	map.clear();
	map.insert(make_pair("ALL_TCKS", false));
	map.insert(make_pair("TCKS_EAST", false));
	map.insert(make_pair("TCKS_WEST", false));
	map.insert(make_pair("TCKS_SEL", false));
	map.insert(make_pair("TCKS_ACTV", false));
	dropDowns[DRP_OVERLAYS] = CDropDown(DRP_AREASEL, MENBAR, "ALL_TCKS", &map);
	map.clear();
	map.insert(make_pair("Delivery", false));
	map.insert(make_pair("OCA Enroute", false));
	map.insert(make_pair("Multi-role", false));
	dropDowns[DRP_TYPESEL] = CDropDown(DRP_AREASEL, MENBAR, "Multi-role", &map);
}

CMenuBar::~CMenuBar() {

};

void CMenuBar::RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {

}

void CMenuBar::ButtonDown(int id) {

}

void CMenuBar::ButtonUp(int id) {

}

void CMenuBar::ButtonPress(int id) {

}