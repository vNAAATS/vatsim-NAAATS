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

class CInboundList
{
	public:
		CInboundList(POINT topLeft);
		POINT GetTopLeft();
		Rect DrawList(Graphics* g, CDC* dc, CRadarScreen* screen, vector<pair<CRadarTarget, bool>>* inboundAircraft, vector<pair<string, int>>* epVec);
		void MoveList(CRect area, bool isReleased);
		void MoveList(POINT topleft, bool isReleased);

	private:
		bool isMouseReleased = true;
		POINT topLeft;
};

