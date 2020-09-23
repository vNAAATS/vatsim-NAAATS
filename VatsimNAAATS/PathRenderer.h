#pragma once
#include "EuroScopePlugIn.h"
#include "Constants.h"
#include "Styles.h"

using namespace std;
using namespace Colours;
using namespace EuroScopePlugIn;

class CPathRenderer
{
	public:
		// Target to compare ASEL against for PIV
		static CRadarTarget pivTarget;

		// ASEL route draw
		static bool RouteDrawASEL;

		// Render a path
		static void RenderRoutePath(CDC* dc, CRadarScreen* screen, CRadarTarget* asel);
};