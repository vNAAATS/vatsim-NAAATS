#include "pch.h"
#include "Utils.h"
#include "Constants.h"
#include "RadarDisplay.h"

// Default values
int Utils::InboundX = 500;
int Utils::InboundY = 150;
int Utils::OthersX = 200;
int Utils::OthersY = 150;
int Utils::AltFiltLow = 0;
int Utils::AltFiltHigh = 700;
bool Utils::GridEnabled = false;
bool Utils::TagsEnabled = true;

// Save plugin data
void Utils::SavePluginData(CPlugIn* plugin) {

	// Inbound list
	plugin->SaveDataToSettings(SET_INBNDX.c_str(), "X position of Inbound list.", to_string(InboundX).c_str());
	plugin->SaveDataToSettings(SET_INBNDY.c_str(), "Y position of Inbound list.", to_string(InboundY).c_str());

	// Others list
	plugin->SaveDataToSettings(SET_OTHERSX.c_str(), "X position of Other list.", to_string(OthersX).c_str());
	plugin->SaveDataToSettings(SET_OTHERSY.c_str(), "Y position of Other list.", to_string(OthersY).c_str());

	// Altitude filter (TODO)

	// Misc display settings
	plugin->SaveDataToSettings(SET_GRID.c_str(), "Grid turned on/off.", GridEnabled ? "true" : "false");
	plugin->SaveDataToSettings(SET_TAGS.c_str(), "Tags turned on/off.", TagsEnabled ? "true" : "false");
}

// Load plugin data
void Utils::LoadPluginData(CPlugIn* plugin) {
	// Strings to parse data
	const char* stra;
	const char* strb;

	// Inbound list
	stra = plugin->GetDataFromSettings(SET_INBNDX.c_str());
	strb = plugin->GetDataFromSettings(SET_INBNDY.c_str());
	if (stra != NULL && strb != NULL) {
		InboundX = stoi(stra);
		InboundY = stoi(strb);
	}

	// Others list
	stra = plugin->GetDataFromSettings(SET_OTHERSX.c_str());
	strb = plugin->GetDataFromSettings(SET_OTHERSY.c_str());
	if (stra != NULL && strb != NULL) {
		OthersX = stoi(stra);
		OthersY = stoi(strb);
	}

	// Grid
	stra = plugin->GetDataFromSettings(SET_GRID.c_str());
	if (stra != NULL) {
		GridEnabled = (stra[0] == 't');
	}
	

	// Tags
	stra = plugin->GetDataFromSettings(SET_TAGS.c_str());
	if (stra != NULL) {
		TagsEnabled = (stra[0] == 't');
	}

	// Show a user message saying that the plugin was loaded successfully
	plugin->DisplayUserMessage("Message", "vNAAATS Plugin", string("version " + PLUGIN_VERSION + " loaded successfully.").c_str(), false, false, false, false, false);
}

string Utils::ParseZuluTime(bool delimit, CFlightPlan* fp = nullptr, int ep = -1) {
	time_t now = time(0);
	tm* zuluTime = gmtime(&now);
	int deltaMinutes = 0;
	if (ep != -1) {
		deltaMinutes = fp->GetExtractedRoute().GetPointDistanceInMinutes(ep);
	}
	int hours = zuluTime->tm_hour;
	int minutes = zuluTime->tm_min + deltaMinutes;

	if (minutes >= 60) {
		// Get minutes
		int minRemainder = minutes % 60;

		// Get number of hours
		hours += (minutes - minRemainder) / 60;

		// Reassign number of minutes
		minutes = minRemainder;
	}

	// Check if over 24 hours
	if (hours >= 24) {
		hours = hours - 24;
	}


	// Pad for zeros
	string strHours;
	if (hours < 10) {
		if (hours == 0) {
			strHours = "00";
		}
		else {
			strHours = "0" + to_string(hours);
		}
	}
	else {
		strHours = to_string(hours);
	}

	string strMinutes;
	if (minutes < 10) {
		if (minutes == 0) {
			strMinutes = "00";
		}
		else {
			strMinutes = "0" + to_string(minutes);
		}
	}
	else {
		strMinutes = to_string(minutes);
	}

	if (delimit) {
		return strHours + ":" + strMinutes;
	}
	else {
		return strHours + strMinutes;
	}
}

int Utils::GetDistanceBetweenPoints(POINT p1, POINT p2) {
	// Get distance
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

POINT Utils::GetMidPoint(POINT p1, POINT p2) {
	// Return midpoint
	return { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2 };
}

int Utils::GetTimeBetweenPoints(int distanceNM, int speedGS) {
	// Get time in minutes
	double temp = (float)distanceNM / (float)speedGS;
	return ((float)distanceNM / speedGS) * 60;
}