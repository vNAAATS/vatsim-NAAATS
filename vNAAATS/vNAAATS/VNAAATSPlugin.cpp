#pragma once
#include "pch.h"
#include <iostream>
#include "VNAAATSPlugin.h"
#include "PluginDisplay.h"

CVNAAATSPlugin::CVNAAATSPlugin() : CPlugIn(COMPATIBILITY_CODE, "vNAAATS", (string(VERSION_NUM) + string(VERSION_MOD)).c_str(), "Andrew Ogden", "(C) Gander Oceanic 2021") {
	// Register the display
	this->Register();
}

CVNAAATSPlugin::~CVNAAATSPlugin() { }

CRadarScreen* CVNAAATSPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated) {
	// Create new display if the display name matches the constant
	if (!strcmp(sDisplayName, DISPLAY_NAME)) {
		return new CPluginDisplay();
	}

	// Show a user message saying that the plugin was loaded successfully
	this->DisplayUserMessage("Message", "vNAAATS", "vNAAATS initialised. Welcome back to the Virtual North Atlantic Advanced Air Traffic System.", false, false, false, false, false);

	// Null return
	return nullptr;
}

void CVNAAATSPlugin::Register() {
	// Register the display type and prevent normal EuroScope traffic from rendering
	RegisterDisplayType(DISPLAY_NAME, false, true, true, true);
}