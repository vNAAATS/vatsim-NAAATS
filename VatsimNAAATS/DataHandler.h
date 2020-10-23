#pragma once
#include "vector"
#include "Structures.h"
#include "EuroScopePlugIn.h"
#include "Overlays.h"
#include "Utils.h"
#include "Structures.h"
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

		// Set flight data
		static int SetFlightData(CAircraftFlightPlan plan);

		// Make a new block
		static int MakeNewFlightData(string callsign);

	private:
		// NAT Track URL
		static const string TrackURL;
		static const string CTPTrackUrl;
		static map<string, CAircraftFlightPlan> flights;
};

