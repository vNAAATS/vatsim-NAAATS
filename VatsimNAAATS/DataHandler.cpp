#include "pch.h"
#include "DataHandler.h"
#include "RoutesHelper.h"
#include <iostream>
#include <fstream>
#include <json.hpp>
//#include "ApiSettings.h"
#include <WinInet.h>

#include "ApiSettings.h"
#include "curl/curl.h"
#pragma comment(lib,"WinInet.Lib")

// Include dependency
using json = nlohmann::json;

const string CDataHandler::TrackURL = "https://tracks.ganderoceanic.com/data";
const string CDataHandler::EventTrackUrl = "https://tracks.ganderoceanic.com/event";
map<string, CAircraftFlightPlan> CDataHandler::flights;

int CDataHandler::PopulateLatestTrackData(CPlugIn* plugin) {
	// Try and get data and pass into string
	string responseString;
	try {
		// Convert URL to LPCSTR type
		LPCSTR lpcURL = EventTrackUrl.c_str();

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
		char tempBuffer[2048];
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
		// Ben: this is for you, I have done the 'not found' code at the bottom you need to do the code that gets the existing flight data
		// not sure whether it's as simple as return flights.at(callsign) or something more complex, depends on the API integration

		fp = flights.find(callsign)->second;
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
	// MISSING VALUES:
	// DLStatus
	// FLIGHT LEVEL
	// MACH
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
	fp.ExitTime = screen->GetPlugIn()->FlightPlanSelect(callsign.c_str()).GetSectorExitMinutes();
	fp.DLStatus = ""; // TEMPORARY
	fp.Sector = string(fpData.GetTrackingControllerId()) == "" ? "-1" : fpData.GetTrackingControllerId();
	fp.CurrentMessage = nullptr;

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

	// TODO: PULL NATTRAK DATA AND FILL RESTRICTIONS, CURRENTMESSAGE AND FLIGHTHISTORY, ALSO ROUTERAW IF IT IS THERE, SAME WITH ISCLEARED

	// Set IsCleared (TEMPORARY)
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

size_t CDataHandler::WriteApiCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

/*void CDataHandler::ApiGetFlightData(void* args)
{
	// Convert args
	CGetFlightDataAsync* data = (CGetFlightDataAsync*)args;
	string callsign = data->Callsign;
	CAircraftFlightPlan* res = data->Result;

	// Cleanup
	delete args;

	string result = "";
	CURL* curl;
	CURLcode result_code;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if (curl)
	{
		stringstream url;
		url << ApiSettings::apiUrl << "/flight_data/get.php?apiKey=" << ApiSettings::apiKey << "&callsign=" << callsign;
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDataHandler::WriteApiCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

		result_code = curl_easy_perform(curl);
		
		curl_easy_cleanup(curl);

		if (result.length() <= 0)
		{
			CAircraftFlightPlan fp;
			fp.IsValid = false;
			res = &fp;
			return;
		} else if (result == "-1")
		{
			CAircraftFlightPlan fp;
			fp.IsValid = false;
			res = &fp;
			return;
		} else
		{
			json jsonArray;
			try
			{
				jsonArray = json::parse(result);
			} catch(nlohmann::detail::parse_error& ex)
			{
				CAircraftFlightPlan fp;
				fp.IsValid = false;
				res = &fp;
				return;
			}
			
			if(jsonArray.size() > 1) //if >1 row was returned (shouldn't be any dupes!)
			{
				CAircraftFlightPlan fp;
				fp.IsValid = false;
				res = &fp;
				return;
			}

			auto row = jsonArray[0];

			string route,
				   callsign,
				   destination,
			       logged_onto,
				   assigned_level,
			       assigned_mach,
			       track;
			
			//null checking
			if(!row.at("route").is_null())
			{
				route = (string)row.at("route");
			}

			if (!row.at("callsign").is_null())
			{
				callsign = (string)row.at("callsign");
			}
			
			if (!row.at("destination").is_null())
			{
				destination = (string)row.at("destination");
			}
			
			if (!row.at("logged_onto").is_null())
			{
				logged_onto = (string)row.at("logged_onto");
			} 
			
			if (!row.at("assigned_level").is_null())
			{
				assigned_level = (string)row.at("assigned_level");
			}
			
			if (!row.at("assigned_mach").is_null())
			{
				assigned_mach = (string)row.at("assigned_mach");
			}
			
			if (!row.at("track").is_null())
			{
				track = (string)row.at("track");
			}

			//end null checking
			
			CAircraftFlightPlan* flight_plan = new CAircraftFlightPlan(
				route,
				callsign,
				"",
				"",
				destination,
				"",
				"",
				"",
				"",
				logged_onto,
				assigned_level,
				assigned_mach,
				track,
				"",
				0,
				false,
				true 
			);

			bool is_track = false;
			if((string)row.at("track") != "RR")
			{
				is_track = true;
			}

			CRoutesHelper::ParseRoute((string)row.at("callsign"), (string)row.at("route"), is_track);
			
			res = flight_plan;
		}
	}
}

CDataHandler::CGetActiveMessagesAsync CDataHandler::ApiGetMessagesForController(CDataHandler::CGetActiveMessagesAsync data)
{
	// Convert args
	string callsign = data.Callsign;
	string controller = data.Controller;

	string result = "";
	CURL* curl;
	CURLcode result_code;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	
	if(curl)
	{
		stringstream url;
		url << ApiSettings::apiUrl << "/messages/get.php?apiKey=" << ApiSettings::apiKey << "&callsign=" << callsign << "&action=from";
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDataHandler::WriteApiCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

		result_code = curl_easy_perform(curl);

		if (result_code != CURLE_OK) {
			curl_easy_cleanup(curl);
			return data;
		}

		curl_easy_cleanup(curl);


		if (result == "-1")
		{
			return data;
		}
	} else {
	}
	
	json j;
	try
	{
		j = json::parse(result);

	}
	catch (nlohmann::detail::parse_error& ex)
	{
		return data;
	}

	if(j.is_null())
	{
		return data;
	}

	for (auto obj : j) {
		if (obj["sent_to"].get<std::string>() == controller && static_cast<int>(obj["is_actioned"]) == 0) {
			string type_string = obj["type"];

			CMessageType type;

			if (type_string == "LOG_ON")
			{
				type = CMessageType::LOG_ON;
			}
			else if (type_string == "LOG_ON_CONFIRM")
			{
				type = CMessageType::LOG_ON_CONFIRM;
			}
			else if (type_string == "LOG_ON_REJECT")
			{
				type = CMessageType::LOG_ON_REJECT;
			}
			else if (type_string == "TRANSFER")
			{
				type = CMessageType::TRANSFER;
			}
			else if (type_string == "TRANSFER_ACCEPT")
			{
				type = CMessageType::TRANSFER_ACCEPT;
			}
			else if (type_string == "TRANSFER_REJECT")
			{
				type = CMessageType::TRANSFER_REJECT;
			}
			else if (type_string == "CLEARANCE_REQ")
			{
				type = CMessageType::CLEARANCE_REQ;
			}
			else if (type_string == "CLEARANCE_ISSUE")
			{
				type = CMessageType::CLEARANCE_ISSUE;
			}
			else if (type_string == "CLEARANCE_REJECT")
			{
				type = CMessageType::CLEARANCE_REJECT;
			}
			else if (type_string == "REVISION_REQ")
			{
				type = CMessageType::REVISION_REQ;
			}
			else if (type_string == "REVISION_ISSUE")
			{
				type = CMessageType::REVISION_ISSUE;
			}
			else if (type_string == "REVISION_REJECT")
			{
				type = CMessageType::REVISION_REJECT;
			}
			else if (type_string == "WILCO")
			{
				type = CMessageType::WILCO;
			}
			else if (type_string == "ROGER")
			{
				type = CMessageType::ROGER;
			}
			else if (type_string == "UNABLE")
			{
				type = CMessageType::UNABLE;
			}
			else
			{
				continue;
			}
			pair<int, CMessage> msg(obj["id"], { obj["id"],
								obj["sent_to"].get<std::string>(),
								obj["sent_by"].get<std::string>(),
								obj["contents_raw"].get<std::string>(),
								obj["created_at"].get<std::string>(),
								type });

			if (data.CurrentResults.find(obj["id"]) == data.CurrentResults.end()) {
				data.Result.insert(msg);
			}
				

		}
	}

	return data;
}

void CDataHandler::ApiGetMessages(void* args)
{
	// Convert args
	CGetMessagesActioned* data = (CGetMessagesActioned*)args;
	string callsign = data->Callsign;
	vector<CMessage>* res = data->Result;

	string result = "";
	CURL* curl;
	CURLcode result_code;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();

	if (curl)
	{
		stringstream url;
		url << ApiSettings::apiUrl << "/messages/get.php?apiKey=" << ApiSettings::apiKey << "&callsign=" << callsign << "&action=from";
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDataHandler::WriteApiCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

		result_code = curl_easy_perform(curl);

		if (result_code != CURLE_OK) {
			curl_easy_cleanup(curl);
			return;
		}

		curl_easy_cleanup(curl);

		if (result == "-1")
		{
			return;
		}
	}

	json j;
	try
	{
		j = json::parse(result);

	}
	catch (nlohmann::detail::parse_error& ex)
	{
		return;
	}

	if (j.is_null())
	{
		return;
	}

	vector<CMessage> messages;
	for (auto obj : j) {
		if (static_cast<int>(obj["is_actioned"]) == 1) {			
			string type_string = obj["type"];

			CMessageType type;

			if (type_string == "LOG_ON")
			{
				type = CMessageType::LOG_ON;
			}
			else if (type_string == "LOG_ON_CONFIRM")
			{
				type = CMessageType::LOG_ON_CONFIRM;
			}
			else if (type_string == "LOG_ON_REJECT")
			{
				type = CMessageType::LOG_ON_REJECT;
			}
			else if (type_string == "TRANSFER")
			{
				type = CMessageType::TRANSFER;
			}
			else if (type_string == "TRANSFER_ACCEPT")
			{
				type = CMessageType::TRANSFER_ACCEPT;
			}
			else if (type_string == "TRANSFER_REJECT")
			{
				type = CMessageType::TRANSFER_REJECT;
			}
			else if (type_string == "CLEARANCE_REQ")
			{
				type = CMessageType::CLEARANCE_REQ;
			}
			else if (type_string == "CLEARANCE_ISSUE")
			{
				type = CMessageType::CLEARANCE_ISSUE;
			}
			else if (type_string == "CLEARANCE_REJECT")
			{
				type = CMessageType::CLEARANCE_REJECT;
			}
			else if (type_string == "REVISION_REQ")
			{
				type = CMessageType::REVISION_REQ;
			}
			else if (type_string == "REVISION_ISSUE")
			{
				type = CMessageType::REVISION_ISSUE;
			}
			else if (type_string == "REVISION_REJECT")
			{
				type = CMessageType::REVISION_REJECT;
			}
			else if (type_string == "WILCO")
			{
				type = CMessageType::WILCO;
			}
			else if (type_string == "ROGER")
			{
				type = CMessageType::ROGER;
			}
			else if (type_string == "UNABLE")
			{
				type = CMessageType::UNABLE;
			}
			else
			{
				continue;
			}
			messages.push_back({ obj["id"],
								obj["sent_to"].get<std::string>(),
								obj["sent_by"].get<std::string>(),
								obj["contents_raw"].get<std::string>(),
								obj["created_at"].get<std::string>(),
								type });

		}
	}

	res = &messages;
}

void CDataHandler::ApiUpdateFlightData(void* args)
{
	// Convert args
	CDataUpdateAsync* data = (CDataUpdateAsync*)args;
	string callsign = data->Callsign;
	string level = data->Level;
	string mach = data->Mach;
	string track = data->Track;
	string route = data->Route;
	bool is_cleared = data->IsCleared;
	string destination = data->Destination;
	int* res = data->Result;

	// Cleanup
	delete args;

	string result = "";
	CURL* curl;
	CURLcode result_code;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	
	if(curl)
	{
		stringstream url,
					 post_data;
		struct curl_slist* headers = nullptr;
		
		url << ApiSettings::apiUrl << "/flight_data/update.php";
		const char* escaped_route = curl_easy_escape(curl, route.c_str(), route.length());
		post_data << "apiKey=" << ApiSettings::apiKey << "&callsign=" << callsign << "&assigned_level=" << level << "&assigned_mach=" << mach << "&track=" << track << "&route=" << escaped_route << "&is_cleared=" << is_cleared << "&destination=" << destination;
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
		
		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDataHandler::WriteApiCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		string cpost_s = post_data.str();
		const char* cpost_data = cpost_s.c_str();
		long cpost_data_len = cpost_s.length();
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cpost_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, cpost_data_len);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		result_code = curl_easy_perform(curl);

		if (result_code != CURLE_OK) {
			curl_easy_cleanup(curl);
			res = (int*) 1;
		}

		curl_easy_cleanup(curl);

		if (result == "-1")
		{
			res = (int*) 1;
		} else if(result == "0")
		{
			res = (int*) 0;
		} else
		{
			res = (int*) 1;
		}
	}
}

void CDataHandler::ApiCreateMessage(void* args)
{
	// Convert args
	CCreateMessageAsync* data = (CCreateMessageAsync*)args;
	string sent_by = data->SentBy;
	string sent_to = data->SentTo;
	string contents_raw = data->ContentsRaw;
	CMessageType type = data->Type;
	bool is_actioned = data->IsActioned;
	bool to_domestic = data->ToDomestic;
	int* res = data->Result;

	// Cleanup
	delete args;

	string result = "";
	CURL* curl;
	CURLcode result_code;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();

	if (curl)
	{
		stringstream url,
			post_data;
		struct curl_slist* headers = nullptr;
		string s_type;
		string s_is_actioned;
		string s_to_domestic;

		if (type == CMessageType::LOG_ON)
		{
			s_type = "LOG_ON";
		}
		else if (type == CMessageType::LOG_ON_CONFIRM)
		{
			s_type = "LOG_ON_CONFIRM";
		}
		else if (type == CMessageType::LOG_ON_REJECT)
		{
			s_type = "LOG_ON_REJECT";
		}
		else if (type == CMessageType::TRANSFER)
		{
			s_type = "TRANSFER";
		}
		else if (type == CMessageType::TRANSFER_ACCEPT)
		{
			s_type = "TRANSFER_ACCEPT";
		}
		else if (type == CMessageType::TRANSFER_REJECT)
		{
			s_type = "TRANSFER_REJECT";
		}
		else if (type == CMessageType::CLEARANCE_REQ)
		{
			s_type = "CLEARANCE_REQ";
		}
		else if (type == CMessageType::CLEARANCE_ISSUE)
		{
			s_type = "CLEARANCE_ISSUE";
		}
		else if (type == CMessageType::CLEARANCE_REJECT)
		{
			s_type = "CLEARANCE_REJECT";
		}
		else if (type == CMessageType::REVISION_REQ)
		{
			s_type = "REVISION_REQ";
		}
		else if (type == CMessageType::REVISION_ISSUE)
		{
			s_type = "REVISION_ISSUE";
		}
		else if (type == CMessageType::REVISION_REJECT)
		{
			s_type = "REVISION_REJECT";
		}
		else if (type == CMessageType::WILCO)
		{
			s_type = "WILCO";
		}
		else if (type == CMessageType::ROGER)
		{
			s_type = "ROGER";
		}
		else if (type == CMessageType::UNABLE)
		{
			s_type = "UNABLE";
		}
		else
		{
			res = (int*) 1;
		}

		if(is_actioned == TRUE)
		{
			s_is_actioned = "TRUE";
		}
		else
		{
			s_is_actioned = "FALSE";
		}

		if (to_domestic == TRUE)
		{
			s_to_domestic = "TRUE";
		}
		else
		{
			s_to_domestic = "FALSE";
		}
		
		url << ApiSettings::apiUrl << "/messages/create.php";
		post_data << "apiKey=" << ApiSettings::apiKey << "&sent_by=" << sent_by << "&sent_to=" << sent_to << "&contents_raw=" << contents_raw << "&type=" << s_type << "&is_actioned=" << s_is_actioned << "&to_domestic=" << s_to_domestic;
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

		string cpost_s = post_data.str();
		const char* cpost_data = cpost_s.c_str();
		long cpost_data_len = cpost_s.length();

		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDataHandler::WriteApiCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cpost_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, cpost_data_len);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		result_code = curl_easy_perform(curl);

		if (result_code != CURLE_OK) {
			curl_easy_cleanup(curl);
			res = (int*) 1;
		}

		curl_easy_cleanup(curl);

		if (result == "-1")
		{
			res = (int*) 1;
		}
		else if (result == "0")
		{
			res = (int*) 0;
		}
		else
		{
			res = (int*) 1;
		}
	}
}

void CDataHandler::ApiMessageActioned(void* args)
{
	// Convert args
	CMessageActionedAsync* data = (CMessageActionedAsync*)args;
	int id = data->Id;
	bool is_actioned = data->IsActioned;
	int* res = data->Result;

	// Cleanup
	delete args;

	string result = "";
	CURL* curl;
	CURLcode result_code;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();

	if (curl)
	{
		stringstream url,
			post_data;
		struct curl_slist* headers = nullptr;
		string action;
		
		if(is_actioned == true)
		{
			action = "set_is_actioned_true";
		}
		else if(is_actioned == false)
		{
			action = "set_is_actioned_false";
		}
		else
		{
			res = (int*) 1;
		}
		
		url << ApiSettings::apiUrl << "/messages/update.php";
		post_data << "apiKey=" << ApiSettings::apiKey << "&id=" << id << "&action=" << action;
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

		string cpost_s = post_data.str();
		const char* cpost_data = cpost_s.c_str();
		long cpost_data_len = cpost_s.length();

		curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CDataHandler::WriteApiCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cpost_data);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, cpost_data_len);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		result_code = curl_easy_perform(curl);

		if (result_code != CURLE_OK) {
			curl_easy_cleanup(curl);
			res = (int*) 1;
		}

		curl_easy_cleanup(curl);

		if (result == "-1")
		{
			res = (int*) 1;
		}
		else if (result == "0")
		{
			res = (int*) 0;
		}
		else
		{
			res = (int*) 1;
		}
	}
}*/
