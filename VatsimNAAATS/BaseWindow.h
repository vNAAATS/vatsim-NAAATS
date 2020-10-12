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
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
		virtual void MakeWindowItems() = 0;
		virtual void ButtonDown(int id) = 0;
		virtual void ButtonUp(int id) = 0;
		virtual void ButtonPress(int id) = 0;

		// Check if an item is a particular type
		virtual bool IsButton(int id);
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
};

