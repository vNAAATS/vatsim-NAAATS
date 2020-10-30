#pragma once
#include "vector"
#include "Structures.h"
#include "EuroScopePlugIn.h"
#include "Overlays.h"
#include "Utils.h"
#include <json.hpp>

using namespace std;
using namespace EuroScopePlugIn;

class CDataHandler
{
	public:
		// Download nat track data
		static int PopulateLatestTrackData(CPlugIn* plugin);

		// Get flight data
		static CAircraftFlightPlan* GetFlightData(string callsign);

		// Update a flight data object
		static int UpdateFlightData(CRadarScreen* screen, string callsign, bool updateRoute);

		// Create a new flight data object
		static int CreateFlightData(CRadarScreen* screen, string callsign);

		// Deletes a flight data object out of the flights map
		static int DeleteFlightData(string callsign);

		// Set route
		static int SetRoute(string callsign, vector<CWaypoint>* route, string track);

	private:
		// NAT Track URL
		static const string TrackURL;
		static const string CTPTrackUrl;
		static map<string, CAircraftFlightPlan> flights;
};

