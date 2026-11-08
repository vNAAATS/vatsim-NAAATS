#include "pch.h"
#include "DataHandler.h"
#include "RoutesHelper.h"
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <WinInet.h>
#pragma comment(lib,"WinInet.Lib")

// Include dependency
using json = nlohmann::json;

// Server settings. Defaults are replaced by vnaaats.cfg (next to the DLL) in LoadSettings().
static const string DEFAULT_SERVER = "https://vnaaats.com";
string CDataHandler::ControllerCallsign = "";
string CDataHandler::DownloadUrl = DEFAULT_SERVER + "/download";
string CDataHandler::ScratchUrl = "";
string CDataHandler::AselUrl = "";
string CDataHandler::RouteUrl = "";
bool CDataHandler::Retrack = true;
string CDataHandler::TrackedUrl = "";
string CDataHandler::RetrackUrl = "";
volatile long CDataHandler::RetrackFetched = 0;
static vector<string> retrackList;
static std::mutex retrackLock;
string CDataHandler::FddUrl = DEFAULT_SERVER + "/fdd";
string CDataHandler::TrackURL = DEFAULT_SERVER + "/GetAllNatTracks";
map<string, CAircraftFlightPlan> CDataHandler::flights;

string CDataHandler::VersionUrl = DEFAULT_SERVER + "/pluginversion";
const string CDataHandler::PluginVersion = "https://raw.githubusercontent.com/garythms/vatsim-naaats-v2/server-and-squawk/pluginversion.txt";
string CDataHandler::TrackSource = DEFAULT_SERVER + "/GetTrackSource";
string CDataHandler::GetSingleAircraft = DEFAULT_SERVER + "/GetSingleFlightData?callsign=";
string CDataHandler::PostSingleAircraft = DEFAULT_SERVER + "/PostFlightData?code=";
string CDataHandler::FlightDataUpdate = DEFAULT_SERVER + "/UpdateFlightData?code=";
string CDataHandler::CpdlcHandoffUrl = DEFAULT_SERVER + "/CpdlcHandoff?code=";
string CDataHandler::CommandsUrl = DEFAULT_SERVER + "/PluginCommands?code=";
string CDataHandler::CommandDoneUrl = DEFAULT_SERVER + "/PluginCommandDone?code=";
std::mutex CDataHandler::remoteLock;
std::mutex CDataHandler::deleteLock;
std::vector<string> CDataHandler::deleteQueue;
std::mutex CDataHandler::pendingLock;
std::map<string, CDataHandler::CPendingNet> CDataHandler::pending;
vector<CDataHandler::CRemoteCommand> CDataHandler::remoteQueue;
std::atomic<bool> CDataHandler::RemotePollBusy(false);
clock_t CDataHandler::RemoteLastPoll = 0;
bool CDataHandler::SquawkOnClearance = false;   // removed: the server now tells pilots to squawk 2000 over CPDLC; the cfg key is ignored
bool CDataHandler::AutoType = false;
string CDataHandler::SettingsUrl = "";
long long CDataHandler::SettingsStamp = 0;
volatile long CDataHandler::SettingsBusy = 0;
volatile long CDataHandler::SettingsMessagePending = 0;
bool CDataHandler::AutoClick = true;
int CDataHandler::ContactButton = 0;   // 0 left, 1 middle, 2 right
int CDataHandler::RouteSeconds = 10;
bool CDataHandler::BackgroundType = false;
bool CDataHandler::Highlight = true;

string CDataHandler::UrlEncode(const string& value) {
	static const char* hex = "0123456789ABCDEF";
	string out;
	for (unsigned char ch : value) {
		if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~' || ch == '/' || ch == ',') {
			out += (char)ch;
		}
		else {
			out += '%'; out += hex[ch >> 4]; out += hex[ch & 15];
		}
	}
	return out;
}

// vnaaats.cfg lines:  server=http://host   key=SECRET   squawk=1
void CDataHandler::LoadSettings() {
	string server = DEFAULT_SERVER, key;
	ifstream in(CUtils::DllPath + "\\vnaaats.cfg");
	string line;
	while (getline(in, line)) {
		size_t eq = line.find('=');
		if (eq == string::npos || line.empty() || line[0] == '#') continue;
		string k = line.substr(0, eq), v = line.substr(eq + 1);
		while (!v.empty() && (v.back() == '\r' || v.back() == ' ')) v.pop_back();
		while (!v.empty() && v.front() == ' ') v.erase(v.begin());
		if (k == "server") server = v;
		else if (k == "key") key = v;
		else if (k == "autotype") AutoType = (v == "1");
		else if (k == "retrack") Retrack = (v != "0");
		else if (k == "autoclick") AutoClick = (v != "0");
		else if (k == "contactbutton") ContactButton = (v == "middle" ? 1 : (v == "right" ? 2 : 0));
		else if (k == "backgroundtype") BackgroundType = (v == "1");
		else if (k == "routeseconds") { try { RouteSeconds = max(0, min(600, stoi(v))); } catch (...) { RouteSeconds = 10; } }
		else if (k == "settingsstamp") { try { SettingsStamp = stoll(v); } catch (...) { SettingsStamp = 0; } }
		else if (k == "highlight") Highlight = (v != "0");
		else if (k == "remotelog") CLogger::RemoteMode = (v == "0" ? 0 : (v == "2" ? 2 : 1));
	}
	while (!server.empty() && server.back() == '/') server.pop_back();
	FddUrl = server + "/fdd";
	VersionUrl = server + "/pluginversion";
	DownloadUrl = server + "/download";
	TrackURL = server + "/GetAllNatTracks";
	TrackSource = server + "/GetTrackSource";
	GetSingleAircraft = server + "/GetSingleFlightData?callsign=";
	PostSingleAircraft = server + "/PostFlightData?code=" + UrlEncode(key);
	FlightDataUpdate = server + "/UpdateFlightData?code=" + UrlEncode(key);
	CpdlcHandoffUrl = server + "/CpdlcHandoff?code=" + UrlEncode(key);
	CommandsUrl = server + "/PluginCommands?code=" + UrlEncode(key);
	SettingsUrl = server + "/PluginSettings?code=" + UrlEncode(key);
	ScratchUrl = server + "/PluginScratch?code=" + UrlEncode(key);
	AselUrl = server + "/PluginAsel?code=" + UrlEncode(key);
	RouteUrl = server + "/PluginRoute?code=" + UrlEncode(key);
	TrackedUrl = server + "/PluginTracked?code=" + UrlEncode(key);
	RetrackUrl = server + "/PluginRetrack?code=" + UrlEncode(key);
	CommandDoneUrl = server + "/PluginCommandDone?code=" + UrlEncode(key);
	CLogger::RemoteUrl = key.empty() ? "" : server + "/PluginLog?code=" + UrlEncode(key);
	if (key.empty()) CLogger::Log(CLogType::WARN, "No API key in vnaaats.cfg - flight data cannot be saved to the server.", "CDataHandler::LoadSettings");
}

int CDataHandler::CheckPluginVersion(CPlugIn* plugin)
{
	// Try and get data and pass into string
	// Our own server first (the PC already trusts it for everything else), GitHub only if that fails. A failed check is only a
	// warning: it must not pop up in EuroScope or raise an error report, because nothing is wrong with the plugin itself.
	string responseString = "";
	string lastProblem = "";
	const string urls[2] = { VersionUrl, PluginVersion };
	for (int u = 0; u < 2 && responseString.empty(); u++) {
		try {
			LPCSTR lpcURL = urls[u].c_str();
			DeleteUrlCacheEntry(lpcURL);
			CComPtr<IStream> pStream;
			HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
			if (FAILED(hr)) {
				lastProblem = string(u == 0 ? "our server" : "GitHub") + " code " + to_string((int)hr);
				continue;
			}
			char tempBuffer[16384];
			DWORD bytesRead = 0;
			pStream->Read(tempBuffer, sizeof(tempBuffer), &bytesRead);
			string got = "";
			for (DWORD i = 0; i < bytesRead; i++) got += tempBuffer[i];
			while (!got.empty() && isspace((unsigned char)got.back())) got.pop_back();
			while (!got.empty() && isspace((unsigned char)got.front())) got.erase(got.begin());
			// a version looks like 2.7.2; anything else (an error page, say) is ignored
			bool ok = !got.empty() && got.size() <= 16;
			for (char ch : got) if (!isdigit((unsigned char)ch) && ch != '.') ok = false;
			if (ok) responseString = got; else lastProblem = string(u == 0 ? "our server" : "GitHub") + " sent something unexpected";
		}
		catch (exception & e) {
			lastProblem = string(u == 0 ? "our server" : "GitHub") + ": " + e.what();
		}
	}
	if (responseString.empty()) {
		CLogger::Log(CLogType::WARN, "Could not check for a new plugin version (" + lastProblem + ")", "CDataHandler::CheckPluginVersion");
		return -1;
	}

	// Check version
	if (responseString != PLUGIN_VERSION) {
		// Display dialog if update available
		int msgBox = MessageBox(NULL, (LPCSTR)("A new version of vNAAATS (" + responseString + ") is available. Your version: " + PLUGIN_VERSION +
			"\n\nClick OK to open the download page.\nClose EuroScope, then replace ONLY VatsimNAAATS.dll with the new one.\nKeep your existing vnaaats.cfg - do not overwrite it.").c_str(),
			(LPCSTR)"vNAAATS Version Notification", MB_ICONINFORMATION | MB_OK);

		if (msgBox == IDOK) {
			// Open the download page on our own server
			ShellExecute(NULL, "open", DownloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		return msgBox;
	}

	return -1;
}

int CDataHandler::PopulateLatestTrackData(CPlugIn* plugin) {
	// Try and get data and pass into string
	string responseString;
	try {
		// Convert URL to LPCSTR type
		string eventTracks = TrackURL + "?event=true";
		LPCSTR lpcURL = GetTrackSource(plugin) == 0 ? TrackURL.c_str() : eventTracks.c_str();

		// Delete cache data
		DeleteUrlCacheEntry(lpcURL);

		// Download data
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		// If failed
		if (FAILED(hr)) {
			int code = (int)hr;
			// Show user message
			plugin->DisplayUserMessage("vNAAATS", "Error", (string("Track data download failed. Code: ") + to_string(code)).c_str(), true, true, true, true, true);
			// Clogger
			CLogger::Log(CLogType::ERR, string("Could not connect to tracks API. Code: ") + to_string(code), "CDataHandler::PopulateLatestTrackData");
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
		// Log to ES
		plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to load NAT Track data: " + string(e.what())).c_str(), true, true, true, true, true);
		// Clogger
		CLogger::Log(CLogType::EXC, "Failed to load NAT Track data: " + string(string(e.what())), "CDataHandler::PopulateLatestTrackData");
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

				// Get lat and lon vars
				double lat = jsonArray[i].at("route")[j].at("latitude");
				double lon = jsonArray[i].at("route")[j].at("longitude");

				// Finally, append
				track.RouteRaw.push_back(CUtils::PositionFromLatLon(lat, lon));				
			}

			// Flight levels
			for (int j = 0; j < jsonArray[i].at("flightLevels").size(); j++) {
				track.FlightLevels.push_back((int)jsonArray[i].at("flightLevels")[j]);
			}

			// Validity
			track.validFrom = string(jsonArray[i].at("validFrom"));
			track.validTo = string(jsonArray[i].at("validTo"));

			// Push track to tracks array
			CRoutesHelper::CurrentTracks.insert(make_pair(track.Identifier, track));
		}
		// Everything succeeded, show to user
		plugin->DisplayUserMessage("Message", "vNAAATS Plugin", string("Track data loaded successfully. TMI is " + CRoutesHelper::CurrentTMI + ".").c_str(), false, false, false, false, false);
		// Clogger
		CLogger::Log(CLogType::NORM, string("Track data loaded successfully. TMI is " + CRoutesHelper::CurrentTMI + "."), "CDataHandler::PopulateLatestTrackData");
		return 0;
	}
	catch (exception & e) {
		// User message
		plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to parse NAT track JSON return: " + string(e.what())).c_str(), true, true, true, true, true);
		// Clogger
		CLogger::Log(CLogType::EXC, "Failed to parse NAT track JSON return: " + string(e.what()), "CDataHandler::PopulateLatestTrackData");
		return 1;
	}
}

int CDataHandler::GetTrackSource(CPlugIn* plugin) {
	// Try and get data and pass into string
	string responseString;
	try {
		// Convert URL to LPCSTR type
		LPCSTR lpcURL = TrackSource.c_str();

		// Delete cache data
		DeleteUrlCacheEntry(lpcURL);

		// Download data
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		// If failed
		if (FAILED(hr)) {
			int code = (int)hr;
			// Show user message
			plugin->DisplayUserMessage("vNAAATS", "Error", (string("Track source retrieval failed. Code: ") + to_string(code)).c_str(), true, true, true, true, true);
			// Clogger
			CLogger::Log(CLogType::ERR, string("Could not connect to the vNAAATS network. Code: ") + to_string(code), "CDataHandler::GetTrackSource");
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

		return stoi(responseString);
	}
	catch (exception & e) {
		// Log to ES
		plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to load track source data: " + string(e.what())).c_str(), true, true, true, true, true);
		// Clogger
		CLogger::Log(CLogType::EXC, "Failed to load track source: " + string(string(e.what())), "CDataHandler::GetTrackSource");
		return 1;
	}
}

CAircraftFlightPlan* CDataHandler::GetFlightData(string callsign) {
	if (flights.find(callsign) != flights.end()) {
		
		return &flights.find(callsign)->second;
	}
	// Return invalid. Must outlive this call: it used to return the address of a local variable, which crashed callers that read it.
	static CAircraftFlightPlan invalidFp;
	invalidFp.IsValid = false;
	return &invalidFp;
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

	try {
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
		fp.DLStatus = "false"; // TEMPORARY
		fp.Sector = string(fpData.GetTrackingControllerId()) == "" ? "-1" : fpData.GetTrackingControllerId();
		fp.CurrentMessage = nullptr;
		fp.IsRelevant = fp.ExitTime != -1 ? true : false;
		fp.IsEquipped = CUtils::IsAircraftEquipped(fpData.GetFlightPlanData().GetRemarks(), fpData.GetFlightPlanData().GetAircraftInfo(), fpData.GetFlightPlanData().GetCapibilities());
		fp.TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(callsign.c_str()).GetPosition().GetRadarFlags());

		// Scrape the selcal from the remarks and set the flight plan property if it exists
		string selcal = CUtils::GetSelcalCode(&fpData);
		fp.SELCAL = selcal != "" ? selcal : "N/A";

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
		_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*)data); // Async

		// Log string
		string fDLog = "Flight data object " + data->Callsign + " generated successfully.";
		// Verbose details
		fDLog.append("\nType: " + fp.Type);
		fDLog.append("\nDoF: " + CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth()) ? "East" : "West");
		fDLog.append("\Equipped?: " + fp.IsEquipped ? "True" : "False");
		fDLog.append("\TimeToExit: " + fp.ExitTime);
		CLogger::Log(CLogType::NORM, "Flight data object for " + data->Callsign + " generated successfully.", "CDataHandler::CreateFlightData");

		// Success
		return 0;
	}
	catch (exception & ex) {
		// Clogger
		CLogger::Log(CLogType::EXC, "Flight data generation for " + callsign + " failed: " + string(ex.what()), "CDataHandler::CreateFlightData");

		// Failed
		return 1;
	}
}

int CDataHandler::DeleteFlightData(string callsign) {
	if (flights.find(callsign) != flights.end()) {
		// Remove the flight if it exists
		flights.erase(callsign);
		// Clogger
		CLogger::Log(CLogType::NORM, "Flight data object for " + callsign + " destroyed successfully.", "CDataHandler::DeleteFlightData");
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
		// If failed (probably a 404 not found): nobody has handled this aircraft yet. Ask the server for its filed route
		// (the VATSIM flight plan, only when it looks right) so it shows without a controller having to enter it.
		if (FAILED(hr)) {
			try {
				static std::mutex routeTriedLock;
				static std::map<string, clock_t> routeTried;
				bool ask = false;
				{
					std::lock_guard<std::mutex> lk(routeTriedLock);
					auto it = routeTried.find(callsign);
					if (it == routeTried.end() || (double)(clock() - it->second) / (double)CLOCKS_PER_SEC >= 120.0) {
						routeTried[callsign] = clock();
						ask = !RouteUrl.empty();
					}
				}
				if (ask) {
					string routeReq = RouteUrl + "&callsign=" + UrlEncode(callsign);
					DeleteUrlCacheEntry(routeReq.c_str());
					CComPtr<IStream> rStream;
					if (SUCCEEDED(URLOpenBlockingStream(NULL, routeReq.c_str(), &rStream, 0, NULL))) {
						char rBuf[4096];
						DWORD rRead = 0;
						string rResp;
						if (SUCCEEDED(rStream->Read(rBuf, sizeof(rBuf), &rRead))) rResp.assign(rBuf, rRead);
						auto rj = json::parse(rResp);
						if (rj.value("ok", false) && !rj.value("route", "").empty()) {
							std::lock_guard<std::mutex> lk(pendingLock);
							CPendingNet pn;
							pn.routeOnly = true;
							pn.routeText = rj.value("route", "");
							pn.routeTrack = rj.value("track", "");
							pending[callsign] = pn;
						}
					}
				}
			}
			catch (...) {}
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
		CLogger::Log(CLogType::EXC, "Could not download network data for aircraft " + callsign + ": " + string(e.what()) + "\nRequest URL: \n" + reqUrl, "CDataHandler::DownloadNetworkAircraft");
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
			netFP.Type = jsonArray[i].at("type");
			netFP.AssignedLevel = jsonArray[i].at("assignedLevel");
			netFP.AssignedMach = jsonArray[i].at("assignedMach");
			netFP.Track = jsonArray[i].at("track");
			netFP.Route = jsonArray[i].at("route");
			netFP.RouteEtas = jsonArray[i].at("routeEtas");
			netFP.Departure = jsonArray[i].at("departure");
			netFP.Arrival = jsonArray[i].at("arrival");
			netFP.Direction = jsonArray[i].at("direction");
			netFP.Etd = jsonArray[i].at("etd");
			netFP.Selcal = jsonArray[i].at("selcal");
			netFP.DatalinkConnected = jsonArray[i].at("datalinkConnected");
			netFP.IsEquipped = jsonArray[i].at("isEquipped");
			netFP.State = jsonArray[i].at("state");
			netFP.Relevant = jsonArray[i].at("relevant");
			netFP.TrackedBy = jsonArray[i].at("trackedBy");
			netFP.TrackedById = jsonArray[i].at("trackedById");
			netFP.TargetMode = jsonArray[i].at("targetMode");
			netFP.LastUpdated = jsonArray[i].at("lastUpdated");
			netFP.Cpdlc = jsonArray[i].value("cpdlc", false); // older servers do not send it
			netFP.CpdlcRelease = jsonArray[i].value("cpdlcRelease", false);
			netFP.NtSent = jsonArray[i].value("ntSent", false);
			netFP.ContactMeAt = jsonArray[i].value("cmAt", 0.0); // older servers do not send it

			// Hand the data to the draw thread. Writing into the live flight plan from this background thread raced with drawing
			// (an open flight plan window reading the route while the server's route replaced it) and could crash EuroScope.
			{
				std::lock_guard<std::mutex> lk(pendingLock);
				CPendingNet pn; pn.net = netFP; pn.scratch = jsonArray[i].value("scratch", "");
				pending[callsign] = pn;
			}
		}
	}
	catch (exception & e) {
		data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to parse aircraft data for " + callsign +". Error: " + string(e.what())).c_str(), true, true, true, true, true);
		CLogger::Log(CLogType::EXC, "Could not parse downloaded JSON for network aircraft " + callsign + ": " + string(e.what()) + "\nRequest URL: \n" + reqUrl, "CDataHandler::DownloadNetworkAircraft");
		// Cleanup
		delete args;
		return;
	}

	// Cleanup
	delete args;
}

// Runs on the EuroScope (draw) thread: applies what DownloadNetworkAircraft fetched, so the flight plans are only ever changed
// by the thread that reads them.
void CDataHandler::QueueDelete(const string& callsign) {
	std::lock_guard<std::mutex> lk(deleteLock);
	for (auto& s : deleteQueue) if (s == callsign) return;
	deleteQueue.push_back(callsign);
}

std::vector<string> CDataHandler::TakeQueuedDeletes() {
	std::lock_guard<std::mutex> lk(deleteLock);
	std::vector<string> out;
	out.swap(deleteQueue);
	return out;
}

void CDataHandler::ApplyPendingNetwork(CRadarScreen* screen) {
	std::map<string, CDataHandler::CPendingNet> local;
	{
		std::lock_guard<std::mutex> lk(pendingLock);
		if (pending.empty()) return;
		local.swap(pending);
	}
	for (auto& kv : local) {
		string callsign = kv.first;
		CNetworkFlightPlan& netFP = kv.second.net;
		CPendingNet& pn = kv.second;
		try {
			// Get flight plan
			CAircraftFlightPlan* fp = CDataHandler::GetFlightData(callsign);

			// Only the filed route (no saved record on the server): fill it in, change nothing else
			if (pn.routeOnly) {
				if (fp->IsValid && !fp->IsCleared) {
					vector<string> rr;
					CUtils::StringSplit(pn.routeText, ' ', &rr);
					if (!rr.empty() && rr != fp->RouteRaw) {
						fp->RouteRaw = rr;
						if ((fp->Track == "RR" || fp->Track.empty()) && !pn.routeTrack.empty()) fp->Track = pn.routeTrack;
						UpdateFlightData(screen, callsign, true);
						CLogger::Log(CLogType::NORM, "Filed route applied to " + callsign + ": " + pn.routeText, "CDataHandler::ApplyPendingNetwork");
					}
				}
				continue;
			}

			// If it is valid
			if (fp->IsValid) {
				// Check if it is cleared
				if (!fp->IsCleared) fp->IsCleared = true;
				// Did level, speed, track or SELCAL change (e.g. edited on the web FDD)? Then the open flight plan window must refresh
				bool valuesChanged = fp->FlightLevel != to_string(netFP.AssignedLevel) || fp->Mach != to_string(netFP.AssignedMach) || fp->Track != netFP.Track;
				if (netFP.Selcal != "" && netFP.Selcal != "N/A" && netFP.Selcal != fp->SELCAL) {
					fp->SELCAL = netFP.Selcal;
					fp->SelcalManual = true; // keep it, do not overwrite from the remarks
					valuesChanged = true;
				}
				if (valuesChanged) { fp->WindowRefresh = true; CLogger::Log(CLogType::NORM, "Web/server change applied to " + callsign + " (level, speed, track or SELCAL).", "CDataHandler::ApplyPendingNetwork"); }
				fp->CpdlcConnected = netFP.Cpdlc;
				fp->NtSent = netFP.NtSent;
				fp->ContactMeAt = netFP.ContactMeAt;
				{
					string netScratch = pn.scratch;
					if (!fp->NetScratchKnown || fp->NetScratch != netScratch) fp->NetScratch = netScratch;
					fp->NetScratchKnown = true;
				}
				// After UNICOM the server asks for the tag to be released (done on the draw thread, once per request)
				if (netFP.CpdlcRelease && !fp->ReleaseHandled) {
					fp->ReleaseRequested = true;
					fp->ReleaseHandled = true;
				}
				if (!netFP.CpdlcRelease) fp->ReleaseHandled = false;
				// Simply update all the values
				fp->FlightLevel = to_string(netFP.AssignedLevel);
				fp->Mach = to_string(netFP.AssignedMach);
				fp->Track = netFP.Track;
				// Keep what EuroScope gave us if the server has nothing for these
				if (!netFP.Departure.empty() && netFP.Departure != "NOFP") fp->Depart = netFP.Departure;
				if (!netFP.Arrival.empty() && netFP.Arrival != "NOFP") fp->Dest = netFP.Arrival;
				if (!netFP.Etd.empty()) fp->Etd = netFP.Etd;
				fp->State = netFP.State;
				// Work equipment out again from the EuroScope flight plan each time: the value saved on the server can be a stale "not
				// equipped" from when the plan was first loaded (remarks not there yet). Fall back to the server value if EuroScope has no remarks.
				{
					CFlightPlan esFp = screen->GetPlugIn()->FlightPlanSelect(callsign.c_str());
					string esRemarks = esFp.IsValid() ? string(esFp.GetFlightPlanData().GetRemarks()) : "";
					if (!esRemarks.empty()) fp->IsEquipped = CUtils::IsAircraftEquipped(esRemarks, esFp.GetFlightPlanData().GetAircraftInfo(), esFp.GetFlightPlanData().GetCapibilities());
					else fp->IsEquipped = netFP.IsEquipped;
				}
				fp->IsRelevant = netFP.Relevant;
				fp->DLStatus = to_string(netFP.DatalinkConnected);
				fp->TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(callsign.c_str()).GetPosition().GetRadarFlags());

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
		catch (exception & e) {
			CLogger::Log(CLogType::EXC, "Could not apply network data for " + callsign + ": " + string(e.what()), "CDataHandler::ApplyPendingNetwork");
		}
	}
}

void CDataHandler::GetAllNetworkAircraft() {

}

void CDataHandler::PostNetworkAircraft(void* args) {
	// Convert args
	CUtils::CNetworkAsyncData* data = (CUtils::CNetworkAsyncData*) args;

	// Prefix
	string reqUrl = PostSingleAircraft;

	try {
		// Switch target mode
		int mode = 0;
		switch (data->FP->TargetMode) {
			case CRadarTargetMode::PRIMARY:
				mode = 0;
				break;
			case CRadarTargetMode::SECONDARY_S:
				mode = 1;
				break;
			case CRadarTargetMode::SECONDARY_C:
				mode = 2;
				break;
			case CRadarTargetMode::ADS_B:
				mode = 3;
				break;
			default:
				mode = 3;
				break;
		}

		// Construct URL
		reqUrl += "&callsign=" + UrlEncode(data->FP->Callsign);
		reqUrl += "&type=" + UrlEncode(data->FP->Type);
		reqUrl += "&level=" + to_string(data->FP->AssignedLevel);
		reqUrl += "&mach=" + to_string(data->FP->AssignedMach);
		reqUrl += "&track=" + UrlEncode(data->FP->Track);
		reqUrl += "&route=" + UrlEncode(data->FP->Route);
		reqUrl += "&routeEtas=" + UrlEncode(data->FP->RouteEtas);
		reqUrl += "&departure=" + UrlEncode(data->FP->Departure);
		reqUrl += "&arrival=" + UrlEncode(data->FP->Arrival);
		reqUrl += "&direction=" + to_string(data->FP->Direction);
		reqUrl += "&etd=" + UrlEncode(data->FP->Etd);
		reqUrl += "&selcal=" + UrlEncode(data->FP->Selcal);
		reqUrl += "&datalinkConnected=" + to_string(data->FP->DatalinkConnected);
		reqUrl += "&isEquipped=" + to_string(data->FP->IsEquipped);
		reqUrl += "&state=" + UrlEncode(data->FP->State);
		reqUrl += "&relevant=" + to_string(data->FP->Relevant);
		reqUrl += "&targetMode=" + to_string(mode);
		reqUrl += "&trackedBy=" + UrlEncode(data->FP->TrackedBy);
		reqUrl += "&trackedById=" + UrlEncode(data->FP->TrackedById);
		reqUrl += "&by=" + UrlEncode(ControllerCallsign); // who made the change (shown on the FDD)
	
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
			data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to post aircraft data for " + data->Callsign + ". The server returned an error.").c_str(), true, true, true, true, true);
			CLogger::Log(CLogType::ERR, "Could not post aircraft " + data->FP->Callsign + " to the network. A connection to the server could not be established.", "CDataHandler::PostNetworkAircraft");
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
		CLogger::Log(CLogType::EXC, "Could not post aircraft " + data->FP->Callsign + " to the network: " + string(e.what()) + "\nRequest URL: \n" + reqUrl, "CDataHandler::PostNetworkAircraft");
		// Cleanup
		delete data->FP;
		delete args;
		return;
	}
}

void CDataHandler::PollRemoteCommands(void* args) {
	try {
		string reqUrl = CommandsUrl + "&me=" + UrlEncode(ControllerCallsign);
		LPCSTR lpcURL = reqUrl.c_str();
		DeleteUrlCacheEntry(lpcURL);
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		if (SUCCEEDED(hr)) {
			string body;
			char buf[16384];
			DWORD got = 0;
			while (SUCCEEDED(pStream->Read(buf, sizeof(buf), &got)) && got > 0) body.append(buf, got);
			auto arr = json::parse(body);
			if (arr.is_array()) {
				std::lock_guard<std::mutex> lock(remoteLock);
				for (auto& item : arr) {
					CRemoteCommand rc;
					rc.Id = item.value("id", 0);
					rc.Type = item.value("type", "");
					rc.Callsign = item.value("callsign", "");
					if (item.contains("args") && item["args"].is_object()) {
						auto& a = item["args"];
						if (a.contains("level") && a["level"].get<int>() > 0) rc.Level = to_string(a["level"].get<int>());
						if (a.contains("mach") && a["mach"].get<int>() > 0) rc.Mach = to_string(a["mach"].get<int>());
						rc.Selcal = a.value("selcal", "");
						rc.Target = a.value("target", "");
						rc.Freq = a.value("freq", "");
						rc.Text = a.value("text", "");
					if (a.contains("plan") && a["plan"].is_object()) rc.Plan = a["plan"].dump();   // flight plan edit: JSON text for the draw thread
					}
					if (rc.Id > 0) remoteQueue.push_back(rc);
				}
			}
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not check for FDD commands: " + string(e.what()), "CDataHandler::PollRemoteCommands");
	}
	RemotePollBusy = false;
}

vector<CDataHandler::CRemoteCommand> CDataHandler::TakeRemoteCommands() {
	std::lock_guard<std::mutex> lock(remoteLock);
	vector<CRemoteCommand> out;
	out.swap(remoteQueue);
	return out;
}

struct CRemoteResult { int Id; bool Ok; string Msg; };

void CDataHandler::ReportRemoteResult(int id, bool ok, const string& msg) {
	if (id <= 0) { // a command the plugin gave itself (automatic re-track): there is no server queue entry, just log how it went
		CLogger::Log(ok ? CLogType::NORM : CLogType::WARN, "Automatic re-track: " + msg, "CDataHandler::ReportRemoteResult");
		return;
	}
	_beginthread(CDataHandler::ReportRemoteCommand, 0, (void*)new CRemoteResult{ id, ok, msg });
}

static std::mutex memoLock;
static std::map<string, CDataHandler::CTrackMemo> trackMemo;

void CDataHandler::MemoTracked(const string& callsign, const string& level, const string& mach, const string& selcal) {
	std::lock_guard<std::mutex> lk(memoLock);
	CTrackMemo& m = trackMemo[callsign];
	m.Level = level; m.Mach = mach; m.Selcal = selcal;
	m.DisconnectedAt = 0;
}

void CDataHandler::MemoDisconnected(const string& callsign) {
	std::lock_guard<std::mutex> lk(memoLock);
	auto it = trackMemo.find(callsign);
	if (it != trackMemo.end() && it->second.DisconnectedAt == 0) it->second.DisconnectedAt = time(NULL);
}

void CDataHandler::MemoForget(const string& callsign) {
	std::lock_guard<std::mutex> lk(memoLock);
	trackMemo.erase(callsign);
}

vector<std::pair<string, CDataHandler::CTrackMemo>> CDataHandler::MemoSnapshot() {
	std::lock_guard<std::mutex> lk(memoLock);
	return vector<std::pair<string, CTrackMemo>>(trackMemo.begin(), trackMemo.end());
}

void CDataHandler::QueueLocalCommand(const CRemoteCommand& cmd) {
	std::lock_guard<std::mutex> lock(remoteLock);
	remoteQueue.push_back(cmd);
}

void CDataHandler::ReportRemoteCommand(void* args) {
	CRemoteResult* r = (CRemoteResult*)args;
	try {
		string reqUrl = CommandDoneUrl + "&id=" + to_string(r->Id) + "&ok=" + (r->Ok ? "1" : "0") + "&msg=" + UrlEncode(r->Msg);
		LPCSTR lpcURL = reqUrl.c_str();
		DeleteUrlCacheEntry(lpcURL);
		CComPtr<IStream> pStream;
		URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not report an FDD command result: " + string(e.what()), "CDataHandler::ReportRemoteCommand");
	}
	delete r;
}

void CDataHandler::PostCpdlcHandoff(void* args) {
	CUtils::CNetworkAsyncData* data = (CUtils::CNetworkAsyncData*) args;
	try {
		string reqUrl = CpdlcHandoffUrl;
		reqUrl += "&callsign=" + UrlEncode(data->Callsign);
		reqUrl += "&target=" + UrlEncode(data->Target);
		reqUrl += "&freq=" + UrlEncode(data->Freq);
		reqUrl += "&by=" + UrlEncode(ControllerCallsign);

		LPCSTR lpcURL = reqUrl.c_str();
		DeleteUrlCacheEntry(lpcURL);
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		if (FAILED(hr)) {
			CLogger::Log(CLogType::ERR, "Could not send the CPDLC handoff message for " + data->Callsign + ".", "CDataHandler::PostCpdlcHandoff");
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "CPDLC handoff failed for " + data->Callsign + ": " + string(e.what()), "CDataHandler::PostCpdlcHandoff");
	}
	delete data;
}

void CDataHandler::UpdateNetworkAircraft(void* args) {
	// Convert args
	CUtils::CNetworkAsyncData* data = (CUtils::CNetworkAsyncData*) args;

	// Prefix
	string reqUrl = FlightDataUpdate;

	try {
		// Switch target mode
		int mode = 0;
		switch (data->FP->TargetMode) {
		case CRadarTargetMode::PRIMARY:
			mode = 0;
			break;
		case CRadarTargetMode::SECONDARY_S:
			mode = 1;
			break;
		case CRadarTargetMode::SECONDARY_C:
			mode = 2;
			break;
		case CRadarTargetMode::ADS_B:
			mode = 3;
			break;
		default:
			mode = 3;
			break;
		}

		// Construct URL		
		reqUrl += "&callsign=" + UrlEncode(data->FP->Callsign);
		reqUrl += "&type=" + UrlEncode(data->FP->Type);
		reqUrl += "&level=" + to_string(data->FP->AssignedLevel);
		reqUrl += "&mach=" + to_string(data->FP->AssignedMach);
		reqUrl += "&track=" + UrlEncode(data->FP->Track);
		reqUrl += "&route=" + UrlEncode(data->FP->Route);
		reqUrl += "&routeEtas=" + UrlEncode(data->FP->RouteEtas);
		reqUrl += "&departure=" + UrlEncode(data->FP->Departure);
		reqUrl += "&arrival=" + UrlEncode(data->FP->Arrival);
		reqUrl += "&direction=" + to_string(data->FP->Direction);
		reqUrl += "&etd=" + UrlEncode(data->FP->Etd);
		reqUrl += "&selcal=" + UrlEncode(data->FP->Selcal);
		reqUrl += "&datalinkConnected=" + to_string(data->FP->DatalinkConnected);
		reqUrl += "&isEquipped=" + to_string(data->FP->IsEquipped);
		reqUrl += "&state=" + UrlEncode(data->FP->State);
		reqUrl += "&relevant=" + to_string(data->FP->Relevant);
		reqUrl += "&targetMode=" + to_string(mode);
		reqUrl += "&trackedBy=" + UrlEncode(data->FP->TrackedBy);
		reqUrl += "&trackedById=" + UrlEncode(data->FP->TrackedById);
		reqUrl += "&by=" + UrlEncode(ControllerCallsign); // who made the change (shown on the FDD)
	
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
			data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to update aircraft data for " + data->Callsign + ". The server returned an error.").c_str(), true, true, true, true, true);
			CLogger::Log(CLogType::ERR, "Could not post update for aircraft " + data->FP->Callsign + ".  A connection to the server could not be established.", "CDataHandler::UpdateNetworkAircraft");
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
		data->plugin->DisplayUserMessage("vNAAATS", "Error", string("Failed to update aircraft data for " + data->Callsign + ". Error: " + string(e.what())).c_str(), true, true, true, true, true);
		CLogger::Log(CLogType::EXC, "Could not post aircraft " + data->FP->Callsign + " to the network: " + string(e.what()) + "\nRequest URL: \n" + reqUrl, "CDataHandler::UpdateNetworkAircraft");
		// Cleanup
		delete data->FP;
		delete args;
		return;
	}
}

// Scratchpad note. EuroScope's scratchpad is shared with other controllers by the network, so a note typed here goes with the
// aircraft to the next controller. We keep it in step with the server (so the FDD can show and edit it):
//  - changed in EuroScope  -> send it to the server
//  - changed on the FDD     -> write it into EuroScope (only if EuroScope allows it)
// Runs on the draw thread.
struct CScratchPush { string Callsign, Text; };

void CDataHandler::ReportTracked(void* args) {
	CTrackedPush* p = (CTrackedPush*)args;
	try {
		if (!TrackedUrl.empty() && !p->Me.empty()) {
			string reqUrl = TrackedUrl + "&me=" + UrlEncode(p->Me) + "&list=" + UrlEncode(p->List);
			DeleteUrlCacheEntry(reqUrl.c_str());
			CComPtr<IStream> pStream;
			URLOpenBlockingStream(NULL, reqUrl.c_str(), &pStream, 0, NULL);
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not report tracked aircraft: " + string(e.what()), "CDataHandler::ReportTracked");
	}
	delete p;
}

void CDataHandler::FetchRetrack(void* args) {
	try {
		if (!RetrackUrl.empty() && !ControllerCallsign.empty()) {
			string reqUrl = RetrackUrl + "&me=" + UrlEncode(ControllerCallsign);
			DeleteUrlCacheEntry(reqUrl.c_str());
			CComPtr<IStream> pStream;
			if (SUCCEEDED(URLOpenBlockingStream(NULL, reqUrl.c_str(), &pStream, 0, NULL))) {
				string body;
				char buf[4096];
				DWORD got = 0;
				while (SUCCEEDED(pStream->Read(buf, sizeof(buf), &got)) && got > 0) body.append(buf, got);
				auto j = json::parse(body);
				std::lock_guard<std::mutex> lock(retrackLock);
				retrackList.clear();
				if (j.contains("callsigns") && j["callsigns"].is_array())
					for (auto& c : j["callsigns"]) if (c.is_string()) retrackList.push_back(c.get<string>());
			}
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not fetch the aircraft to re-track: " + string(e.what()), "CDataHandler::FetchRetrack");
	}
	InterlockedExchange(&RetrackFetched, 1);
}

void CDataHandler::FetchSettings(void* args) {
	try {
		if (!SettingsUrl.empty()) {
			string reqUrl = SettingsUrl;
			DeleteUrlCacheEntry(reqUrl.c_str());
			CComPtr<IStream> pStream;
			if (SUCCEEDED(URLOpenBlockingStream(NULL, reqUrl.c_str(), &pStream, 0, NULL))) {
				string body;
				char buf[4096];
				DWORD got = 0;
				while (SUCCEEDED(pStream->Read(buf, sizeof(buf), &got)) && got > 0) body.append(buf, got);
				auto j = json::parse(body);
				long long stamp = j.contains("stamp") && j["stamp"].is_number() ? j["stamp"].get<long long>() : 0;
				if (stamp > SettingsStamp && j.contains("settings") && j["settings"].is_object() && !j["settings"].empty()) {
					// Only these options can be changed from the FDD (never the key or the server address)
					static const char* allowed[] = { "autotype", "autoclick", "highlight", "retrack", "squawk", "remotelog" };
					map<string, string> want;
					for (const char* name : allowed) {
						if (j["settings"].contains(name)) {
							auto& v = j["settings"][name];
							string val = v.is_string() ? v.get<string>() : (v.is_number() ? to_string(v.get<int>()) : "");
							if (val == "0" || val == "1" || val == "2") want[name] = val;
						}
					}
					string path = CUtils::DllPath + "\\vnaaats.cfg";
					vector<string> lines;
					{
						ifstream in(path);
						string l;
						while (getline(in, l)) { if (!l.empty() && l.back() == '\r') l.pop_back(); lines.push_back(l); }
					}
					map<string, string> have;
					for (auto& line : lines) {
						size_t eq = line.find('=');
						if (eq == string::npos || line.empty() || line[0] == '#') continue;
						string key = line.substr(0, eq);
						while (!key.empty() && key.back() == ' ') key.pop_back();
						auto it = want.find(key);
						if (it != want.end()) { have[key] = line.substr(eq + 1); line = key + "=" + it->second; }
					}
					bool changed = false;
					for (auto& kv : want) {
						auto h = have.find(kv.first);
						string cur = h == have.end() ? string("") : h->second;
						while (!cur.empty() && (cur.back() == ' ' || cur.back() == '\r')) cur.pop_back();
						if (h == have.end()) { lines.push_back(kv.first + "=" + kv.second); changed = true; }
						else if (cur != kv.second) changed = true;
					}
					// remember which version of the FDD settings is in the file
					bool stampDone = false;
					for (auto& line : lines) if (line.rfind("settingsstamp=", 0) == 0) { line = "settingsstamp=" + to_string(stamp); stampDone = true; }
					if (!stampDone) lines.push_back("settingsstamp=" + to_string(stamp));
					ofstream out(path, ios::trunc);
					for (auto& line : lines) out << line << "\r\n";
					out.close();
					SettingsStamp = stamp;
					if (changed) {
						CLogger::Log(CLogType::NORM, "Settings chosen on the FDD were saved to vnaaats.cfg. EuroScope must be restarted for them to take effect.", "CDataHandler::FetchSettings");
						InterlockedExchange(&SettingsMessagePending, 1);
					}
				}
			}
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not fetch the settings: " + string(e.what()), "CDataHandler::FetchSettings");
	}
	InterlockedExchange(&SettingsBusy, 0);
}

vector<string> CDataHandler::TakeRetrack() {
	std::lock_guard<std::mutex> lock(retrackLock);
	vector<string> out;
	out.swap(retrackList);
	return out;
}

void CDataHandler::ReportAsel(void* args) {
	CAselPush* p = (CAselPush*)args;
	try {
		if (!AselUrl.empty() && !p->Me.empty()) {
			string reqUrl = AselUrl + "&me=" + UrlEncode(p->Me) + "&callsign=" + UrlEncode(p->Callsign);
			DeleteUrlCacheEntry(reqUrl.c_str());
			CComPtr<IStream> pStream;
			URLOpenBlockingStream(NULL, reqUrl.c_str(), &pStream, 0, NULL);
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not report the selected aircraft: " + string(e.what()), "CDataHandler::ReportAsel");
	}
	delete p;
}

void CDataHandler::PushScratch(void* args) {
	CScratchPush* p = (CScratchPush*)args;
	try {
		if (!ScratchUrl.empty()) {
			string reqUrl = ScratchUrl + "&callsign=" + UrlEncode(p->Callsign) + "&text=" + UrlEncode(p->Text);
			DeleteUrlCacheEntry(reqUrl.c_str());
			CComPtr<IStream> pStream;
			URLOpenBlockingStream(NULL, reqUrl.c_str(), &pStream, 0, NULL);
		}
	}
	catch (exception & e) {
		CLogger::Log(CLogType::EXC, "Could not send the scratchpad note: " + string(e.what()), "CDataHandler::PushScratch");
	}
	delete p;
}

string CDataHandler::SyncScratch(CFlightPlan& esFp) {
	if (!esFp.IsValid()) return "";
	string cs = esFp.GetCallsign();
	CAircraftFlightPlan* fp = GetFlightData(cs);
	const char* esRaw = esFp.GetControllerAssignedData().GetScratchPadString();
	string es = esRaw ? esRaw : "";
	if (flights.find(cs) == flights.end() || !fp->IsValid || !fp->NetScratchKnown) return es;   // not on the server yet: just show what EuroScope has   // not on the server yet: just show what EuroScope has
	string net = fp->NetScratch;

	auto push = [&](const string& text) { _beginthread(PushScratch, 0, (void*)new CScratchPush{ cs, text }); };
	auto write = [&](const string& text) {
		esFp.GetControllerAssignedData().SetScratchPadString(text.c_str());
		const char* backRaw = esFp.GetControllerAssignedData().GetScratchPadString();
		string back = backRaw ? backRaw : "";
		fp->ScratchSynced = (back == text) ? text : back;   // if EuroScope refused, stay with what it still has
		if (back != text) fp->ScratchRetryAt = clock() + 5 * CLOCKS_PER_SEC;
	};

	if (!fp->ScratchInit) {
		fp->ScratchInit = true;
		fp->LastNetScratch = net;
		if (!es.empty() && es != net) { fp->ScratchSynced = es; push(es); }
		else if (es.empty() && !net.empty()) write(net);
		else fp->ScratchSynced = es;
	}
	else if (es != fp->ScratchSynced && clock() >= fp->ScratchRetryAt) {      // changed in EuroScope
		fp->ScratchSynced = es;
		push(es);
	}
	else if (net != fp->LastNetScratch) {                                       // changed on the FDD
		fp->LastNetScratch = net;
		if (net != es) write(net);
		else fp->ScratchSynced = es;
	}
	else if (net != fp->ScratchSynced && es == fp->ScratchSynced && clock() >= fp->ScratchRetryAt && fp->ScratchRetryAt != 0) {
		write(net);                                                             // EuroScope refused earlier: try again
	}
	fp->Scratch = fp->ScratchSynced;
	return fp->Scratch;
}
