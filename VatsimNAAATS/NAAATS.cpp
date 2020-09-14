#include "pch.h"
#include "NAAATS.h"
#include "Constants.h"
#include "RadarDisplay.h"

CNAAATSPlugin::CNAAATSPlugin() : CPlugIn(COMPATIBILITY_CODE, PLUGIN_NAME.c_str(), PLUGIN_VERSION.c_str(), PLUGIN_AUTHOR.c_str(), PLUGIN_COPYRIGHT.c_str())
{
	this->Register();
	DisplayUserMessage("Message", "vNAAATS Plugin", string("version " + PLUGIN_VERSION + " loaded successfully.").c_str(), false, false, false, false, false);
}

CNAAATSPlugin::~CNAAATSPlugin()
{

}

CRadarScreen* CNAAATSPlugin::OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated) 
{
	if (!strcmp(sDisplayName, DISPLAY_NAME)) {
		return new RadarDisplay();
	}

	return nullptr;
}

void CNAAATSPlugin::Register() {
	RegisterDisplayType(DISPLAY_NAME, false, true, true, true);
}