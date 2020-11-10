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

	//callback method for curl
	static size_t CDataHandler::WriteApiCallback(void* contents, size_t size, size_t nmemb, void* userp);

	//get flight_data for a/c
	static CAircraftFlightPlan* ApiGetFlightData(string callsign);

	//get messages for controller for an a/c (that aren't actioned)
	static vector<CMessage> ApiGetMessagesForController(string callsign, string controller);

	//get messages of an a/c (that ARE actioned)
	static vector<CMessage> ApiGetMessages(string callsign);
	
	//get ALL flight_data where logged_onto is equal to the controller's callsign
	static CMessage* ApiGetAllFlightData(string controller);

	//create flight_data
	static int ApiCreateFlightData(string callsign, string req_logged_onto, string destination);

	private:
		// NAT Track URL
		static const string TrackURL;
		static const string EventTrackUrl;
		static map<string, CAircraftFlightPlan> flights;
};
