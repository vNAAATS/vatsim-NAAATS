#include "pch.h"
#include "DataHandler.h"
#include <iostream>
#include <fstream>

// Include dependency
using json = nlohmann::json;

string CDataHandler::TrackURL = "http://localhost:33224/data";

int CDataHandler::PopulateLatestTrackData(CPlugIn* plugin) {
	// Try and get data and pass into string
	const char* responseCString = "";
	try {
		// Convert URL to LPCSTR type
		LPCSTR lpcURL = TrackURL.c_str();

		// Download data
		CComPtr<IStream> pStream;
		HRESULT hr = URLOpenBlockingStream(NULL, lpcURL, &pStream, 0, NULL);
		// Put data into buffer
		char tempBuffer[2048];
		DWORD bytesRead = 0;
		hr = pStream->Read(tempBuffer, sizeof(tempBuffer), &bytesRead);
		// Put data into string
		for (int i = 0; i < bytesRead; i++) {
			responseCString += tempBuffer[i];
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
		const char* err = e.what();
	}
	
	// Now we parse the json
	auto jsonArray = json::parse(responseCString);
	for (int i = 0; i < jsonArray.size(); i++) {
		// Compile track
		CTrack track;
		track.Identifier = jsonArray[i].at("id");
		track.TMI = jsonArray[i].at("tmi");
		// Direction
		//if (jsonArray[i].at("direction"))
		//track.Direction = 
	}
	

	return 0;
}