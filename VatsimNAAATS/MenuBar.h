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
class CMenuBar
{
	public:
		// TODO: Refactor all dropdown stuff into this file
		// Dropdowns
		static map<int, string> dropDownSelections;
		static map<int, string> dropDownItems;
		static int currentDropDownId;
		static int dropDownHover;
		static int dropDownClicked;

		// Current drop down id
		static int currentDropDown;

		// Button data
		static map<int, string> BuildButtonData();

		// Toggle button data
		static map<int, int> BuildToggleButtonData();

		// Render the menu bar
		static void DrawMenuBar(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, map<int, string>* btnData, map<int, bool>* pressedData, map<int, int>* toggleData);

		// Render a button, any button
		static CRect DrawMenuBarButton(CDC* dc, CRadarScreen* screen, POINT topLeft, pair<int, string> kv, int width, int height, int vtcAlign, POINT mousePointer, bool isCentred, bool isPressed, bool isPosActive);
		
		// Render a drop down
		static CRect DrawDropDown(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, pair<int, string> kv, int width, int height, int vtcAlign, POINT mousePointer, bool isOpen, int dpId);
};

