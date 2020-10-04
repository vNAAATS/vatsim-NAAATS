#pragma once
#include "EuroScopePlugIn.h"
#include <vector>
#include "Constants.h"
#include "Utils.h"
#include "Structures.h"
#include "Styles.h"
#include <gdiplus.h>

using namespace std;
using namespace Colours;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CConflictDetection
{
	public:
		// Separation vector tool
		static void SepTool(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB);

		// PIVTool

		// Probe
		
		// STCA (run every 30s)

	private: 
		// Separation values
		static const int SEPV_LOW = 1000;
		static const int SEPV_HIGH = 2000;
		static const int SEPV_SUPERSONIC = 4000;
		static const int SEPLAT_NONREDUCED = 60;
		static const int SEPLAT_REDUCED = 23;
		static const int SEPLON_REDUCED = 5;
		static const int SEPLON_REDUCEDWARN = 8;
		static const int SEPLON_NONRED_SAMEANDX = 15;
		static const int SEPLON_NONRED_MNT = 10;
		// Detect a conflict status between two aircraft (current or future)
		static CSepStatus DetectStatus(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB);
		// Mach number technique
		static int MachNumberTechnique(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB, string point);
};

