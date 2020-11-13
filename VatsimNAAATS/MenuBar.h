#pragma once
#include "pch.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include <EuroScopePlugIn.h>
#include "Structures.h"

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;
class CMenuBar
{
	public:
		CMenuBar();
		~CMenuBar() {};
		void RenderBar(CDC* dc, Graphics* g, CRadarScreen* screen, string asel);
		bool IsButtonPressed(int id);
		string GetDropDownValue(int id);
		map<int, CWinButton> GetToggleButtons();
		void SetButtonState(int id, CInputState state);
		CInputState GetButtonState(int id);
		void OnOverDropDownItem(int id);
		void SetDropDownValue(int id, int value);
		void MakeDropDownItems(int id);
		void SetTextInput(int id, string value);
		void ButtonDown(int id, int button);
		void ButtonUp(int id, int button);
		void ButtonPress(int id, int button, CRadarScreen* screen);
		void ButtonUnpress(int id, int button, CRadarScreen* screen);
		void GetSelectedTracks(vector<string>& tracksVector);
		int ActiveDropDown;
		int ActiveDropDownHover;

		// Definitions
		static const int BTN_SETUP = 0;
		static const int BTN_NOTEPAD = 1;
		static const int BTN_ADSC = 2;
		static const int BTN_TCKINFO = 3;
		static const int BTN_MISC = 4;
		static const int BTN_MESSAGE = 5;
		static const int BTN_TAGS = 6;
		static const int BTN_FLIGHTPLAN = 7;
		static const int BTN_DETAILED = 8;
		static const int BTN_AREASEL = 9;
		static const int BTN_TCKCTRL = 10;
		static const int BTN_OVERLAYS = 11;
		static const int BTN_TYPESEL = 12;
		static const int BTN_ALTFILT = 13;
		static const int BTN_HALO = 14;
		static const int BTN_RBL = 15;
		static const int BTN_RINGS = 16;
		static const int BTN_PTL = 17;
		static const int BTN_PIV = 18;
		static const int BTN_GRID = 19;
		static const int BTN_SEP = 20;
		static const int BTN_QCKLOOK = 21;
	
		static const int DRP_AREASEL = 100;
		static const int DRP_TCKCTRL = 101;
		static const int DRP_OVERLAYS = 102;
		static const int DRP_TYPESEL = 103;

		static const int TXT_ALTFILT = 30;
		static const int ID_POS = 31;
		static const int TXT_SEARCH = 32;

		const int PANEL_SIZES[8] = { RECT1_WIDTH, RECT2_WIDTH, RECT3_WIDTH, RECT4_WIDTH, RECT5_WIDTH, RECT6_WIDTH, RECT7_WIDTH, RECT8_WIDTH };

		map<int, CDropDown> dropDowns;

	private:
		// Menu bar objects
		map<int, CWinButton> buttons;
		map<int, CTextInput> textInputs;
		map<int, CCheckBox> checkBoxes;
		map<int, int> panels;
};

