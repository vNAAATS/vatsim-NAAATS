#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>

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
		virtual void MakeButtons() = 0;
		CRect DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, bool isPressed, int type, string id = "");

		// Buttons for window
		map<int, pair<string, bool>> WindowButtons;

	protected:
		// Top left point of window
		POINT topLeft;
};

