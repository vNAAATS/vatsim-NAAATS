#pragma once
#include "pch.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;

// The menu bar
class MenuBar
{
	public:
		// Button data
		static map<int, string> BuildButtonData(int section);

		// Render the menu bar
		static void DrawMenuBar(CDC* dc, CRadarScreen* screen, POINT topLeft);

		// Render a button, any button
		static CRect DrawMenuBarButton(CDC* dc, POINT topLeft, string text, int width, int height, POINT mousePointer, bool isPressed);
	
};

