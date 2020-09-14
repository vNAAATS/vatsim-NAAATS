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
		static map<int, string> BuildButtonData();

		// Render the menu bar
		static void DrawMenuBar(Graphics* g, CRadarScreen* screen, POINT topLeft);
	
};

