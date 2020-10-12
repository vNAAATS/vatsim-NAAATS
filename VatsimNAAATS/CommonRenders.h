#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include "Structures.h"
#include <string>
#include <gdiplus.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CCommonRenders // Commonly rendered objects, here for ease of access
{
	public: 
		static void RenderButton(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CWinButton* obj);
		static void RenderTextInput(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CTextInput* obj);
		static void RenderCheckBox(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CCheckBox* obj);
		static void RenderDropDown(CDC* dc, CRadarScreen* screen, POINT topLeft, int width, int height, CDropDown* obj);
};

