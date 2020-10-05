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
		// Target for route draw
		static string RouteDrawTarget;

		// Route to draw
		static pair<bool, vector<CRoutePosition>> RouteToDraw;

		// Render a path
		static void RenderPath(CDC* dc, Graphics* g, CRadarScreen* screen, CPathType type, vector<CSepStatus>* pivA = nullptr, vector<CSepStatus>* pivB = nullptr);

		// Get a route
		static pair<bool, vector<CRoutePosition>> GetRoute(CRadarScreen* screen, string callsign);

		// Is on a NAT track
		static string OnNatTrack(CRadarScreen* screen, string callsign);

		// Clear route
		static int ClearCurrentRoute();
};