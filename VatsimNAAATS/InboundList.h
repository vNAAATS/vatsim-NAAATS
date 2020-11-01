#pragma once
#include "pch.h"
#include <string>
#include <map>
#include <vector>
#include <gdiplus.h>
#include <EuroScopePlugIn.h>
#include "Structures.h"
#include "BaseList.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CInboundList : public CBaseList
{
	public:
		CInboundList(POINT topLeft);
		virtual void RenderList(Graphics* g, CDC* dc, CRadarScreen* screen);

		// Aircraft list
		static vector<CInboundAircraft> AircraftList;

		// Hides and shows
		bool HideShowButton = true;
};

