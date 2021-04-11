#include "pch.h"
#include "DataHandler.h"
#include "RoutesHelper.h"
#include "Keys.h"
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <WinInet.h>
#pragma comment(lib,"WinInet.Lib")

// Include dependency
using json = nlohmann::json;
using namespace ApiKeys;

const string CDataHandler::TrackURL = "https://tracks.ganderoceanic.com/data";
const string CDataHandler::EventTrackUrl = "https://cdn.ganderoceanic.com/resources/data/eventTracks.json";
map<string, CAircraftFlightPlan> CDataHandler::flights;

const string CDataHandler::GetSingleAircraft = "https://vnaaats-net.ganderoceanic.com/api/FlightDataSingleGet?callsign=";
const string CDataHandler::PostSingleAircraft = "https://vnaaats-net.ganderoceanic.com/api/FlightDataNewPost?code=" + ApiKeys::FUNC_KEY;
const string CDataHandler::FlightDataUpdate = "https://vnaaats-net.ganderoceanic.com/api/FlightDataUpdate?code=" + ApiKeys::FUNC_KEY;

int CDataHandler::PopulateLatestTrackData(CPlugIn* plugin) {
	// Try and get data and pass into string
	string responseString;
	try {
		// Convert URL to LPCSTR type
		LPCSTR lpcURL = TrackURL.c_str();

		// Delete cache data
		DeleteUrlCacheEntry(lpcURL);

		// Download data
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		// If failed
		if (FAILED(hr)) {
			int code = (int)hr;
			// Show user message
			plugin->DisplayUserMessage("vNAAATS", "Error", "Failed to load NAT Track data. Code: " + code, true, true, true, true, true);
			return 1;
		}
		// Put data into buffer
		char tempBuffer[16384];
		DWORD bytesRead = 0;
		hr = pStream->Read(tempBuffer, sizeof(tempBuffer), &bytesRead);
		// Put data into string
		for (int i = 0; i < bytesRead; i++) {
			responseString += tempBuffer[i];
		}
	}
	catch (exception & e) {
		plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to load NAT Track data: " + string(e.what())).c_str(), true, true, true, true, true);
		return 1;
	}
	
	// Parse the json
	try {
		// Clear old tracks
		if (!CRoutesHelper::CurrentTracks.empty()) {
			CRoutesHelper::CurrentTracks.clear();
		}
		
		// Now we parse the json
		auto jsonArray = json::parse(responseString);
		for (int i = 0; i < jsonArray.size(); i++) {
			// Make track
			CTrack track;

			// Identifier
			track.Identifier = jsonArray[i].at("id");

			// TMI
			track.TMI = jsonArray[i].at("tmi");
			CRoutesHelper::CurrentTMI = jsonArray[i].at("tmi");

			// Direction
			if (jsonArray[i].at("direction") == 0) {
				track.Direction = CTrackDirection::UNKNOWN;
			}
			else if (jsonArray[i].at("direction") == 1) {
				track.Direction = CTrackDirection::WEST;
			}
			else {
				track.Direction = CTrackDirection::EAST;
			}

			// Route
			for (int j = 0; j < jsonArray[i].at("route").size(); j++) {
				track.Route.push_back(jsonArray[i].at("route")[j].at("name"));
				track.RouteRaw.push_back(CUtils::PositionFromLatLon(jsonArray[i].at("route")[j].at("latitude"), jsonArray[i].at("route")[j].at("longitude")));
			}

			// Flight levels
			for (int j = 0; j < jsonArray[i].at("flightLevels").size(); j++) {
				track.FlightLevels.push_back((int)jsonArray[i].at("flightLevels")[j]);
			}

			// Push track to tracks array
			CRoutesHelper::CurrentTracks.insert(make_pair(track.Identifier, track));
		}
		// Everything succeeded, show to user
		plugin->DisplayUserMessage("Message", "vNAAATS Plugin", string("Track data loaded successfully. TMI is " + CRoutesHelper::CurrentTMI + ".").c_str(), false, false, false, false, false);
		return 0;
	}
	catch (exception & e) {
		plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to parse NAT track data: " + string(e.what())).c_str(), true, true, true, true, true);
		return 1;
	}
}

CAircraftFlightPlan* CDataHandler::GetFlightData(string callsign) {
	if (flights.find(callsign) != flights.end()) {
		// Ben: this is for you, I have done the 'not found' code at the bottom you need to do the code that gets the existing flight data
		// not sure whether it's as simple as return flights.at(callsign) or something more complex, depends on the API integration
		
		return &flights.find(callsign)->second;
	}
	// Return invalid
	CAircraftFlightPlan fp;
	fp.IsValid = false;
	return &fp;
}

void CDataHandler::GetFlightData(string callsign, CAircraftFlightPlan& fp) {
	if (flights.find(callsign) != flights.end()) {
		fp = flights.find(callsign)->second;
		fp.IsValid = true;
		return;
	}
	fp.IsValid = false;
}

int CDataHandler::UpdateFlightData(CRadarScreen* screen, string callsign, bool updateRoute) {
	// Flight plan
	auto fp = flights.find(callsign);

	// If flight plan not found return non-success code
	if (fp == flights.end()) {
		return 1;
	}

	// Re-initialise the route if requested
	if (updateRoute) {
		CUtils::CAsyncData* data = new CUtils::CAsyncData();
		data->Screen = screen;
		data->Callsign = callsign;
		_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*) data); // Async
	}

	// Success
	return 0;
}

int CDataHandler::CreateFlightData(CRadarScreen* screen, string callsign) {
	// MISSING VALUES:
	// DLStatus
	// SECTOR
	// STATE

	// Euroscope flight plan and my flight plan
	CFlightPlan fpData = screen->GetPlugIn()->FlightPlanSelect(callsign.c_str());
	CAircraftFlightPlan fp;	
	// Instantiate aircraft information
	fp.Callsign = callsign;
	fp.Type = fpData.GetFlightPlanData().GetAircraftFPType();
	fp.Depart = fpData.GetFlightPlanData().GetOrigin();
	fp.Dest = fpData.GetFlightPlanData().GetDestination();
	fp.Etd = CUtils::ParseZuluTime(false, atoi(fpData.GetFlightPlanData().GetEstimatedDepartureTime()));
	fp.ExitTime = fpData.GetSectorExitMinutes();
	fp.DLStatus = ""; // TEMPORARY
	fp.Sector = string(fpData.GetTrackingControllerId()) == "" ? "-1" : fpData.GetTrackingControllerId();
	fp.CurrentMessage = nullptr;
	fp.IsEquipped = CUtils::IsAircraftEquipped(fpData.GetFlightPlanData().GetRemarks(), fpData.GetFlightPlanData().GetAircraftInfo(), fpData.GetFlightPlanData().GetCapibilities());

	// Get SELCAL code
	string remarks = fpData.GetFlightPlanData().GetRemarks();
	size_t found = remarks.find(string("SEL/"));
	// If found
	if (found != string::npos) {
		fp.SELCAL = remarks.substr(found + 4, 4);
	}
	else {
		fp.SELCAL= "";
	}

	// Get communication mode
	string comType;
	comType += toupper(fpData.GetFlightPlanData().GetCommunicationType());
	if (comType == "V") { // Switch each of the values
		comType = "VOX";
	}
	else if (comType == "T") {
		comType = "TXT";
	}
	else if (comType == "R") {
		comType = "RCV";
	}
	else {
		comType = "VOX"; // We assume voice if it defaults
	}
	fp.Communications = comType; // Set the type

	// Set IsCleared
	fp.IsCleared = false;

	// Flight plan is valid
	fp.IsValid = true;

	// Add FP to map
	flights[callsign] = fp;

	// Generate route
	CUtils::CAsyncData* data = new CUtils::CAsyncData();
	data->Screen = screen;
	data->Callsign = callsign;
	_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*) data); // Async

	// Success
	return 0;
}

int CDataHandler::DeleteFlightData(string callsign) {
	if (flights.find(callsign) != flights.end()) {
		// Remove the flight if it exists
		flights.erase(callsign);
		return 0;
	}
	else {
		return 1; // Non-success code occurs when flight doesn't exist to delete
	}
}

int CDataHandler::SetRoute(string callsign, vector<CWaypoint>* route, string track, CAircraftFlightPlan* copiedPlan) {
	if (copiedPlan != nullptr) {
		// Set route if flight exists
		copiedPlan->Route.clear();
		copiedPlan->Route = *route;
		copiedPlan->Route.shrink_to_fit();

		// Set track if not nothing
		if (track != "")
			copiedPlan->Track = track;
		else
			copiedPlan->Track = "RR";

		return 0;
	}
	if (flights.find(callsign) != flights.end()) {
		// Set route if flight exists
		flights.find(callsign)->second.Route.clear();
		flights.find(callsign)->second.Route = *route;
		flights.find(callsign)->second.Route.shrink_to_fit();
		// Set track if not nothing
		if (track != "")
			flights.find(callsign)->second.Track = track;
		else 
			flights.find(callsign)->second.Track = "RR";

		// Success code
		return 0;
	}
	else {
		return 1; // Non-success code occurs when flight doesn't exist
	}
}

void CDataHandler::DownloadNetworkAircraft(void* args) {
	// Convert args
	CUtils::CNetworkAsyncData* data = (CUtils::CNetworkAsyncData*) args;

	// Get callsign & radar screen
	string callsign = data->Callsign;
	CRadarScreen* screen = data->Screen;

	// Create URL
	string reqUrl = GetSingleAircraft + callsign;

	// Try and get data and pass into string
	string responseString;

	try {
		// Convert URL to LPCSTR type
		LPCSTR lpcURL = reqUrl.c_str();

		// Delete cache data
		DeleteUrlCacheEntry(lpcURL);

		// Download data
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		// If failed (probably a 404 not found)
		if (FAILED(hr)) {
			// Cleanup
			delete args;
			return;
		}

		// Put data into buffer
		char tempBuffer[16384];
		DWORD bytesRead = 0;
		hr = pStream->Read(tempBuffer, sizeof(tempBuffer), &bytesRead);
		// Put data into string
		for (int i = 0; i < bytesRead; i++) {
			responseString += tempBuffer[i];
		}
	}
	catch (exception & e) {
		data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to download aircraft data for " + data->Callsign + ". Error: " + string(e.what())).c_str(), true, true, true, true, true);
		// Cleanup
		delete args;
		return;
	}

	// Parse the json
	try {
		// Now we parse the json
		auto jsonArray = json::parse(responseString);
		for (int i = 0; i < jsonArray.size(); i++) {
			// Make a network object
			CNetworkFlightPlan netFP;

			// Fill properties
			netFP.Callsign = jsonArray[i].at("callsign");
			netFP.AssignedLevel = jsonArray[i].at("assignedLevel");
			netFP.AssignedMach = jsonArray[i].at("assignedMach");
			netFP.Track = jsonArray[i].at("track");
			netFP.Route = jsonArray[i].at("route");
			netFP.RouteEtas = jsonArray[i].at("routeEtas");
			netFP.Departure = jsonArray[i].at("departure");
			netFP.Arrival = jsonArray[i].at("arrival");
			netFP.IsEquipped = jsonArray[i].at("isEquipped");
			netFP.TrackedBy = jsonArray[i].at("trackedBy");
			netFP.LastUpdated = jsonArray[i].at("lastUpdated");

			// Get flight plan
			CAircraftFlightPlan* fp = CDataHandler::GetFlightData(callsign);

			// If it is valid
			if (fp->IsValid) {
				// Check if it is cleared
				if (!fp->IsCleared) fp->IsCleared = true;
				// Simply update all the values
				fp->FlightLevel = to_string(netFP.AssignedLevel);
				fp->Mach = to_string(netFP.AssignedMach);
				fp->Track = netFP.Track;
				fp->Depart = netFP.Departure;
				fp->Dest = netFP.Arrival;
				fp->IsEquipped = netFP.IsEquipped;
					
				// Routes
				vector<string> splitString;
				CUtils::StringSplit(netFP.Route, ' ', &splitString);
				bool isUpdated = false;

				// Check first if they are the same length
				if (splitString.size() != fp->RouteRaw.size()) {
					// They are not so update the route
					fp->RouteRaw = splitString;
					isUpdated = true;
				}
				else {
					// Now iterate and find out if there are discrepancies
					for (int i = 0; i < splitString.size(); i++) {
						if (splitString[i] != fp->RouteRaw[i]) {
							// There is a discrepancy
							fp->RouteRaw = splitString;
							isUpdated = true;
							break;
						}
					}
				}

				// If isUpdated is true, then we re-instantiate the route, hopefully without a crash
				if (isUpdated) UpdateFlightData(screen, callsign, true);
			}
		}
	}
	catch (exception & e) {
		data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to parse aircraft data for " + callsign +". Error: " + string(e.what())).c_str(), true, true, true, true, true);
		// Cleanup
		delete args;
		return;
	}

	// Cleanup
	delete args;
}

void CDataHandler::GetAllNetworkAircraft() {

}

void CDataHandler::PostNetworkAircraft(void* args) {
	// Convert args
	CUtils::CNetworkAsyncData* data = (CUtils::CNetworkAsyncData*) args;

	// Construct URL
	string reqUrl = PostSingleAircraft;
	reqUrl += "&callsign=" + data->FP->Callsign;
	reqUrl += "&level=" + to_string(data->FP->AssignedLevel);
	reqUrl += "&mach=" + to_string(data->FP->AssignedMach);
	reqUrl += "&track=" + data->FP->Track;
	reqUrl += "&route=" + data->FP->Route;
	reqUrl += "&routeEtas=" + data->FP->RouteEtas;
	reqUrl += "&departure=" + data->FP->Departure;
	reqUrl += "&arrival=" + data->FP->Arrival;
	reqUrl += "&isEquipped=" + to_string(data->FP->IsEquipped);
	reqUrl += "&trackedBy=" + data->FP->TrackedBy;

	try {
		// Convert URL to LPCSTR type
		LPCSTR lpcURL = reqUrl.c_str();

		// Delete cache data
		DeleteUrlCacheEntry(lpcURL);

		// Download data
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);

		// If failed
		if (FAILED(hr)) {
			// We want to know about it
			data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to post aircraft data for " + data->Callsign).c_str(), true, true, true, true, true);
			// Cleanup
			delete data->FP;
			delete args;
			return;
		}
		else {
			// Success, clean up and move on
			delete data->FP;
			delete args;
			return;
		}
	}
	catch (exception & e) {
		data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to post aircraft data for " + data->Callsign + ". Error: " + string(e.what())).c_str(), true, true, true, true, true);
		// Cleanup
		delete data->FP;
		delete args;
		return;
	}
}

void CDataHandler::UpdateNetworkAircraft(void* args) {

}