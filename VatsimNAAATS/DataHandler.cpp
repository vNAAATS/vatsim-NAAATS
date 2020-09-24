#include "pch.h"
#include "DataHandler.h"

string CDataHandler::TrackURL = "http://localhost:33224/data";

int CDataHandler::PopulateLatestTrackData(CPlugIn* plugin) {
	string responseString;
	// Try open file
	try {
		// Destination
		TCHAR dllpath[2048];
		GetCurrentDirectory(2048, dllpath);
		string dest = string(dllpath) + "\\plugindata\\tracks.json";

		// Convert URL & destination to LPCSTR type
		LPCSTR lpcURL = TrackURL.c_str();
		LPCSTR lpcDest = dest.c_str();

		// Download file
		HRESULT hr = URLDownloadToFile(NULL, lpcURL, lpcDest, 0, NULL);
	}
	catch (exception & e) {
		const char* err = e.what();
	}
	
	return 0;
}