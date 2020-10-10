#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "Constants.h"
#include "Structures.h"

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
		void DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, CInputState state, int type, string id);
		void DrawTextInput(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, bool canEdit, CWinTextInput* obj, int type, string id);
		void DrawCheckBox(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CWinCheckBox* obj, int type, string id);

		// Window items
		map<int, pair<string, CInputState>> WindowButtons;
		map<int, CWinTextInput> TextInputs;
		map<int, CWinCheckBox> CheckBoxes;

		// Check if an item is a particular type
		bool IsButton(int id);
		bool IsTextInput(int id);
		bool IsCheckBox(int id);

	protected:
		// Top left point of window
		POINT topLeft;
};

