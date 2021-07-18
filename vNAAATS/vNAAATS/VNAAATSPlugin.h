#pragma once
#include <string>
#include <EuroScopePlugIn.h>

using namespace EuroScopePlugIn;

#define DISPLAY_NAME "vNAAATS SITM"
#define VERSION_NUM "v2.0"
#define VERSION_MOD "p"

class CVNAAATSPlugin : public CPlugIn {
	public:
		// Constructor & destructor
		CVNAAATSPlugin();
		virtual ~CVNAAATSPlugin();

		// OnRadarScreenCreated event
		CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

	private:
		// Register the radar display
		void Register();
};

