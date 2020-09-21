#pragma once
#include "pch.h"
#include <gdiplus.h>
#include <EuroScopePlugIn.h>
#include <map>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

/// Drawing aircraft targets
class CAcTargets
{
	public:
		// Render the airplane icon
		static void DrawAirplane(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, int hdg, bool tagsOn, map<int,int>* toggleData, bool halo, bool ptl);

		// Render tags
		static POINT DrawTag(CDC* dc, CRadarScreen* screen, CRadarTarget* target, pair<bool, POINT>* tagPosition, bool direction);

		// Range bearing line tool
		static void RangeBearingLine(CDC* dc, CRadarScreen* screen, string target1, string target2);
};
