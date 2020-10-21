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
		CAircraftFlightPlan GetFlightData(string callsign);

		// Set flight data
		int SetFlightData(CAircraftFlightPlan);

	private:
		// NAT Track URL
		static const string TrackURL;
		static const string CTPTrackUrl;
		map<string, CAircraftFlightPlan> flights;
};

