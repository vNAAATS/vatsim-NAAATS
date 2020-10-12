#pragma once
#include "pch.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;
class CMenuBar
{
	CMenuBar();
	~CMenuBar() {};
	static void MakeDropDowns();
	void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
	void RenderAltFilter(CDC* dc, Graphics* g, CRadarScreen* screen);
	void RenderPositionID(CDC* dc, Graphics* g, CRadarScreen* screen);
	void ButtonDown(int id);
	void ButtonUp(int id);
	void ButtonPress(int id);

	// Definitions
	const int BTN_SETUP = 0;
	const int BTN_NOTEPAD = 1;
	const int BTN_ADSC = 2;
	const int BTN_TCKINFO = 3;
	const int BTN_MISC = 4;
	const int BTN_MESSAGE = 5;
	const int BTN_TAGS = 6;
	const int BTN_FLIGHTPLAN = 7;
	const int BTN_DETAILED = 8;
	const int BTN_AREASEL = 9;
	const int BTN_TCKCTRL = 10;
	const int BTN_OVERLAYS = 11;
	const int BTN_TYPESEL = 12;
	const int BTN_ALTFILT = 13;
	const int BTN_HALO = 14;
	const int BTN_RBL = 15;
	const int BTN_RINGS = 16;
	const int BTN_PTL = 17;
	const int BTN_PIV = 18;
	const int BTN_GRID = 19;
	const int BTN_SEP = 20;
	const int BTN_QCKLOOK = 21;
	
	const int DRP_AREASEL = 100;
	const int DRP_TCKCTRL = 101;
	const int DRP_OVERLAYS = 102;
	const int DRP_TYPESEL = 103;

	const int TXT_ALTFILT = 30;
	const int ID_POS = 31;

	private:
		// Menu bar objects
		map<int, CWinButton> buttons;
		map<int, CTextInput> textInputs;
		map<int, CCheckBox> checkBoxes;
		map<int, CDropDown> dropDowns;
		map<int, int> panels;
};

