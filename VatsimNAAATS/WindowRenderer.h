#pragma once
#include <string>
#include <map>
#include <gdiplus.h>
#include <EuroScopePlugIn.h>

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;

class CWindowRenderer
{
	public:
		// Window open statuses
		static bool FlightPlanWindowOpen;
		static bool TrackInfoWindowOpen;
		static bool ContractWindowOpen;
		static bool SetupWindowOpen;
		static bool MiscWindowOpen;
		static bool MessageWindowOpen;

		// Window render methods
		static void RenderFlightPlanWindow(CDC* dc, Graphics* g, CRadarScreen* screen, CRadarTarget* target);
		static void RenderTrackInfoWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		static void RenderContractWindow(CDC* dc, Graphics* g, CRadarScreen* screen, CRadarTarget* target);
		static void RenderSetupWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		static void RenderMiscWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		static void RenderMessageWindow(CDC* dc, Graphics* g, CRadarScreen* screen, CRadarTarget* target);
};

