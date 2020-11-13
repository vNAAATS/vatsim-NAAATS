#pragma once
#include "pch.h"
#include <gdiplus.h>
#include "Structures.h"
#include <EuroScopePlugIn.h>
#include <map>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

/// Drawing aircraft targets
class CAcTargets
{
	public:
		// Timers (for flashing white if critical conflict & halo around searched aircraft)
		static clock_t twoSecondTimer;
		static clock_t fiveSecondTimer;

		// Render the airplane icon
		static void DrawAirplane(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, bool tagsOn, map<int, CWinButton>* toggleData, bool halo, bool ptl, CSTCAStatus* status);

		// Render tags
		static POINT DrawTag(CDC* dc, CRadarScreen* screen, CRadarTarget* target, pair<bool, POINT>* tagPosition, bool direction, CSTCAStatus* status);

		// Draw selection halo
		static void RenderSelectionHalo(Graphics* g, CRadarScreen* screen, CRadarTarget* target);

		// Aircraft search
		static string SearchedAircraft;
};
