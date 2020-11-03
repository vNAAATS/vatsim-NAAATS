#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "Constants.h"
#include "Structures.h"
#include "CommonRenders.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CBaseWindow
{
	public:
		// Methods
		CBaseWindow(POINT topLeft);
		virtual ~CBaseWindow() {};
		POINT GetTopLeft();
		void MoveWindow(CRect topleft);
		void Scroll(int id, POINT newPtr, POINT oldPtr);
		string GetTextValue(int id);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
		virtual void MakeWindowItems() = 0;
		virtual void ButtonDown(int id) = 0;
		virtual void ButtonUp(int id) = 0;
		virtual void ButtonPress(int id) = 0;
		virtual void ButtonUnpress(int id) = 0;
		virtual void SetButtonState(int id, CInputState state) = 0;
		int ActiveDropDown;
		int ActiveDropDownHover;

		// Check if an item is a particular type
		virtual bool IsButton(int id);
		virtual bool IsDropDown(int id);
		virtual bool IsTextInput(int id);
		virtual bool IsCheckBox(int id);

		// Closed flag
		bool IsClosed;

	protected:
		// Top left point of window
		POINT topLeft;

		// Window items
		map<int, CWinButton> windowButtons;
		map<int, CTextInput> textInputs;
		map<int, CCheckBox> checkBoxes;
		map<int, CDropDown> dropDowns;
		map<int, CWinScrollBar> scrollBars;
};

