#pragma once
#include "EuroScopePlugIn.h"
#include "Constants.h"
#include "Structures.h"
#include "Utils.h"
#include "Styles.h"

using namespace std;
using namespace Colours;
using namespace EuroScopePlugIn;

class CPathRenderer
{
	public:
		// Target to compare ASEL against for PIV
		static CRadarTarget PivTarget;

		// Target for route draw
		static string RouteDrawTarget;

		// Route to draw
		static vector<CRoutePosition> RouteToDraw;

		// Render a path
		static void RenderPath(CDC* dc, Graphics* g, CRadarScreen* screen, CPathType type);

		// Get a route
		static void GetRoute(CRadarScreen* screen, string callsign);

		// Clear route
		static int ClearCurrentRoute();
};