#pragma once
#include <string>
#include <EuroScopePlugIn.h>

using namespace EuroScopePlugIn;

class CNAAATSPlugin : public CPlugIn
{
	public:
		CNAAATSPlugin();
		virtual ~CNAAATSPlugin();

		CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

	private:
		void Register();
};

