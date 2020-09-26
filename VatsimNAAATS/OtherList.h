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

class COtherList
{
	public:
		COtherList(POINT topLeft);
		POINT GetTopLeft();
		Rect DrawList(Graphics* g, CDC* dc, CRadarScreen* screen, vector <string> * otherAircraft);
		void MoveList(CRect area);
		void MoveList(POINT topleft);

	private:
		POINT topLeft;
};
