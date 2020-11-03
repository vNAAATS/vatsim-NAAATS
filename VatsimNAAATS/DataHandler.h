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

	//get flight_data for a/c
	static CAircraftFlightPlan ApiGetFlightData(string callsign);

	//get messages for controller (that aren't actioned)
	static CMessage* ApiGetMessages(string callsign, string controller);

	//get ALL messages from all a/c (regardless of actioned)
	static CMessage* ApiGetAllMessages(string controller);

	//get ALL flight_data where logged_onto is equal to the controller's callsign
	static CMessage* ApiGetAllFlightData(string controller);

	//make messages
	static int ApiPostMessage(string apiKey, string sent_by, string sent_to, string contents_raw, string type, bool is_actioned = FALSE, bool to_domestic = FALSE);

	private:
		// NAT Track URL
		static const string TrackURL;
		static const string EventTrackUrl;
		static map<string, CAircraftFlightPlan> flights;
};
