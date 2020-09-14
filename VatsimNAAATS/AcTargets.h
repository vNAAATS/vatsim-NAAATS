#pragma once
#include "pch.h"
#include <gdiplus.h>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

/// Drawing aircraft targets
class AcTargets
{
	public:
		// Render the airplane icon
		static void DrawAirplane(Graphics* g, CRadarScreen* screen, CRadarTarget target);
};
