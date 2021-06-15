#pragma once
#include "pch.h"
#include <iostream>
#include "NAAATS.h"
#include "Constants.h"
#include "RadarDisplay.h"

CNAAATSPlugin::CNAAATSPlugin() : CPlugIn(COMPATIBILITY_CODE, PLUGIN_NAME.c_str(), PLUGIN_VERSION.c_str(), PLUGIN_AUTHOR.c_str(), PLUGIN_COPYRIGHT.c_str())
{
	// Register the display
	this->Register();
}

CNAAATSPlugin::~CNAAATSPlugin() {

}

CRadarScreen* CNAAATSPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated) 
{
	// Create new display if the display name matches the constant
	if (!strcmp(sDisplayName, DISPLAY_NAME)) {
		return new CRadarDisplay();
	}

	return nullptr;
}

void CNAAATSPlugin::Register() {
	// Register the display type and prevent normal EuroScope traffic from rendering
	RegisterDisplayType(DISPLAY_NAME, false, true, true, true);
}