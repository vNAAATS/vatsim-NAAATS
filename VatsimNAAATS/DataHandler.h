#pragma once
#include "vector"
#include "Structures.h"
#include "EuroScopePlugIn.h"
#include "Overlays.h"
#include "Utils.h"
#include "Logger.h"
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
	static void GetFlightData(string callsign, CAircraftFlightPlan& fp);

	// Update a flight data object
	static int UpdateFlightData(CRadarScreen* screen, string callsign, bool updateRoute);

	// Create a new flight data object
	static int CreateFlightData(CRadarScreen* screen, string callsign);

	// Deletes a flight data object out of the flights map
	static int DeleteFlightData(string callsign);

	// Set route
	static int SetRoute(string callsign, vector<CWaypoint>* route, string track, CAircraftFlightPlan* copiedPlan = nullptr);

	/// vNAAATS network methods
	// Download aircraft data (single)
	static void DownloadNetworkAircraft(void* args);

	// Download all aircraft data
	static void GetAllNetworkAircraft();

	// Post new aircraft data
	static void PostNetworkAircraft(void* args);

	// Update aircraft data
	static void UpdateNetworkAircraft(void* args);
	
	private:
		// Methods
		static int GetTrackSource(CPlugIn* plugin); // Event tracks or not

		// NAT Track URL
		static const string TrackURL;
		static const string EventTrackUrl;
		static map<string, CAircraftFlightPlan> flights;

		// vNAAATS API Links
		static const string TrackSource;
		static const string GetSingleAircraft;
		static const string FlightDataUpdate;
		static const string PostSingleAircraft;
};
