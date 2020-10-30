#include "pch.h"
#include "DataHandler.h"
#include "RoutesHelper.h"
#include <iostream>
#include <fstream>
#include <WinInet.h>
#pragma comment(lib,"WinInet.Lib" )

// Include dependency
using json = nlohmann::json;

const string CDataHandler::TrackURL = "https://tracks.ganderoceanic.com/data";
const string CDataHandler::CTPTrackUrl = "https://tracks.ganderoceanic.com/ctp";
map<string, CAircraftFlightPlan> CDataHandler::flights;

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
		// Put data into buffer
		char tempBuffer[2048];
		DWORD bytesRead = 0;
		hr = pStream->Read(tempBuffer, sizeof(tempBuffer), &bytesRead);
		// Put data into string
		for (int i = 0; i < bytesRead; i++) {
			responseString += tempBuffer[i];
		}
		// If failed
		if (FAILED(hr)) {
			// Show user message
			plugin->DisplayUserMessage("vNAAATS", "Error", "Failed to load NAT Track data.", true, true, true, true, true);
			return 1;
		}
	}
	catch (exception & e) {
		plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to load NAT Track data: " + string(e.what())).c_str(), true, true, true, true, true);
		return 1;
	}
	
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

int CDataHandler::UpdateFlightData(CRadarScreen* screen, string callsign, bool updateRoute) {
	// Flight plan
	auto fp = flights.find(callsign);

	// If flight plan not found return non-success code
	if (fp == flights.end()) {
		return 1;
	}

	// Ben: I have done the update route part for the initialisation if passed in, but you need to do the 
	// update querying to update the flight data, maybe make an IsFlightDataUpdated() method returning the
	// updated data from natTrak if it has been updated and returning an FP object with IsValid == false if not updated

	// Re-initialise the route if requested
	if (updateRoute) {
		CUtils::CAsyncData data;
		data.Screen = screen;
		data.Callsign = callsign;
		_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*) &data); // Async
	}

	// Success
	return 0;
}

int CDataHandler::CreateFlightData(CRadarScreen* screen, string callsign) {
	// FP object
	CAircraftFlightPlan fp;	
	fp.Callsign = callsign;
	fp.IsValid = true;
	fp.IsCleared = false; // Ben: this was temporary for my needs, make it so it updates dynamically to true if natTrak clearance

	// Ben: So here I have done what I needed for the routes, I need you to initialise everything else
	// either from natTrak if the data is there, or from Euroscope values if no natTrak data existing for the pilot

	// Add FP to map
	flights[callsign] = fp;

	// Generate route (Ben: this must be the last thing done in this method)
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

int CDataHandler::SetRoute(string callsign, vector<CWaypoint>* route, string track) {
	if (flights.find(callsign) != flights.end()) {
		// Set route if flight exists
		flights.find(callsign)->second.Route = *route;

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