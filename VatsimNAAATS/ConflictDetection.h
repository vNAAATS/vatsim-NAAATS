#pragma once
#include "EuroScopePlugIn.h"
#include <vector>
#include "Constants.h"
#include "Utils.h"
#include "Structures.h"
#include "RoutesHelper.h"
#include "DataHandler.h"
#include "Styles.h"
#include <gdiplus.h>
#include <map>

using namespace std;
using namespace Colours;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CConflictDetection
{
	public:
		// PIV
		static vector<CAircraftStatus> PIVLocations1;
		static vector<CAircraftStatus> PIVLocations2;
		static vector<CRoutePosition> PIVRoute1;
		static vector<CRoutePosition> PIVRoute2;
		static vector<CSepStatus> PIVSeparationStatuses;
		static map<string, int>* aircraftOnScreen;

		// STCA
		static vector<CSTCAStatus> CurrentSTCA;

		// Range Bearing Line tool
		static void RBLTool(CDC* dc, Graphics* g, CRadarScreen* screen, string target1, string target2);

		// Separation vector tool
		static void SepTool(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB);

		// Path Intercept Vector tool
		static void PIVTool(CRadarScreen* screen, string targetA, string targetB);

		// Path Intercept Vector tool
		static void RenderPIV(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB);

		// STCA (run every 10s)
		static void CheckSTCA(CRadarScreen* screen, CRadarTarget* target, map<string, int>* onScreenAircraft);

		// Probe tool
		static bool ProbeTool(CRadarScreen* screen, string callsign, map<string, vector<CSepStatus>>* statuses, CAircraftFlightPlan* copy = nullptr);

	private: 
		// Separation values
		// TODO: enhance and change this for the new separation requirements
		static const int SEPV_LOW = 900; // Subtract a hundred otherwise it's way too sensitive
		static const int SEPV_HIGH = 1900;
		static const int SEPV_SUPERSONIC = 3900;
		static const int SEPLAT_NONREDUCED = 60;
		static const int SEPLAT_REDUCED = 23;
		static const int SEPLON_REDUCED = 5;
		static const int SEPLON_REDUCEDWARN = 8;
		static const int SEPLON_NONRED_SAMEANDX = 15;
		static const int SEPLON_NONRED_MNT = 10;

		// Detect a conflict status between two aircraft (current or future)
		static CSepStatus DetectStatus(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB);

		// Get an aircraft's status along its route
		static vector<CAircraftStatus> GetStatusesAlongRoute(CRadarScreen* screen, string callsign, int groundSpeed, int altitude, int pivID);

		// Get statuses along the route points
		static vector<CAircraftStatus> GetStatusesAlongRoutePoints(CRadarScreen* screen, string callsign, int groundSpeed, int altitude);

		// Mach number technique
		static int MachNumberTechnique(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB, string point);
};

