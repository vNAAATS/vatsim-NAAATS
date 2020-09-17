#pragma once
#include "pch.h"
#include <string>
#include <map>
#include <vector>
#include <gdiplus.h>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class Lists
{
	public:
		// Inbound list
		static Rect DrawInboundList(Graphics* g, CDC* dc, CRadarScreen* screen, POINT topLeft, vector<pair<CRadarTarget, bool>>* inboundAircraft, vector<pair<string, int>>* epVec);
};

