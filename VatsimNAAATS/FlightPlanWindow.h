#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "BaseWindow.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

// Maybe make a base class for all the windows?
class CFlightPlanWindow : public CBaseWindow
{
	public:
		// Inherited methods
		CFlightPlanWindow(POINT topLeft);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		virtual void MakeButtons();

		// Window functions to do

		// Window messages to display
};

