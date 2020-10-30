#pragma once
#include "EuroScopePlugIn.h"
#include "Constants.h"
#include "Structures.h"
#include "Utils.h"

using namespace std;
using namespace EuroScopePlugIn;

class CRoutesHelper
{
	public:
		// Current NAT tracks
		static map<string, CTrack> CurrentTracks;

		// Current TMI
		static string CurrentTMI;

		// Active aircraft routes to draw
		static vector<string> ActiveRoutes;

		// Get a route
		static vector<CRoutePosition> GetRoute(CRadarScreen* screen, string callsign);

		// Initialise route
		static void InitialiseRoute(void* args);

		// Is on a NAT track
		static string OnNatTrack(CRadarScreen* screen, string callsign);
};

