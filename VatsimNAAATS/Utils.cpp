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
	if (stra != NULL && stra == "true") {
		GridEnabled = true;
	}
	else {
		GridEnabled = false;
	}

	// Tags
	stra = plugin->GetDataFromSettings(SET_TAGS.c_str());
	if (stra != NULL && stra == "true") {
		TagsEnabled = true;
	}
	else {
		TagsEnabled = false;
	}

	// Show a user message saying that the plugin was loaded successfully
	plugin->DisplayUserMessage("Message", "vNAAATS Plugin", string("version " + PLUGIN_VERSION + " loaded successfully.").c_str(), false, false, false, false, false);
}