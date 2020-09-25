#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CTrackInfoWindow
{
	public:
		// Methods
		CTrackInfoWindow(POINT topLeft);
		POINT GetTopLeft();
		void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		void MoveWindow(CRect topleft);
		static CRect DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, bool isPressed, int type, string id = "");

		// Buttons for window
		map<int, pair<string, bool>> WindowButtons;

		// Window functions to do
		bool NATDataRefresh;

		// Window messages to display
		string MsgDataRefresh = "";

	private:
		// Top left point of window
		POINT topLeft;
};

