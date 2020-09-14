#pragma once
#include "pch.h"
#include <gdiplus.h>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class AcTargets
{
	public:
		static void DrawAirplane(Graphics* g, CRadarScreen* screen, CRadarTarget target);
};
