#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CTrackInfoWindow
{
	public:
		CTrackInfoWindow(POINT topLeft);
		POINT GetTopLeft();
		void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		void MoveWindow(CRect topleft);
		static CRect DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, bool isPressed);

	private:
		POINT topLeft;
};

