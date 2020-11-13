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

	// Message data async
	struct CGetFlightDataAsync {
		string Callsign;
		CAircraftFlightPlan* Result;
	};
	struct CGetMessagesActioned {
		string Callsign;
		vector<CMessage>* Result;
	};
	struct CGetActiveMessagesAsync {
		string Callsign;
		string Controller;
		unordered_map<int, CMessage> Result;
		unordered_map<int, CMessage> CurrentResults;
	};
	struct CCreateMessageAsync {
		string SentBy;
		string SentTo;
		string ContentsRaw;
		CMessageType Type;
		bool IsActioned;
		bool ToDomestic;
		int* Result;
	};
	struct CDataUpdateAsync {
		string Callsign;
		string Level;
		string Mach;
		string Track;
		string Route;
		bool IsCleared;
		string Destination;
		int* Result;
	};
	struct CMessageActionedAsync {
		int Id;
		bool IsActioned;
		int* Result;
	};

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

	//callback method for curl
	static size_t CDataHandler::WriteApiCallback(void* contents, size_t size, size_t nmemb, void* userp);

	//get flight_data for a/c
	static void ApiGetFlightData(void* args);

	//get messages for controller for an a/c (that aren't actioned)
	static CGetActiveMessagesAsync ApiGetMessagesForController(CGetActiveMessagesAsync data);

	//get messages of an a/c (that ARE actioned)
	static void ApiGetMessages(void* args);
		
	//create flight_data
	static void ApiUpdateFlightData(void* args);

	//create messages
	static void ApiCreateMessage(void* args);

	//set is_actioned on message
	static void ApiMessageActioned(void* args);
	
	

	private:
		// NAT Track URL
		static const string TrackURL;
		static const string EventTrackUrl;
		static map<string, CAircraftFlightPlan> flights;
};
