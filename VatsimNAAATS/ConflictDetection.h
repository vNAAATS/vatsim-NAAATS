#pragma once
#include "EuroScopePlugIn.h"
#include <vector>
#include "Constants.h"
#include "Utils.h"
#include "Structures.h"

using namespace std;
using namespace EuroScopePlugIn;

class CConflictDetection
{
	public:
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
		static CConflictStatus DetectStatus(CRadarScreen* screen, CAircraftStatus* acA, CAircraftStatus* acB);

		// Get separation status
		static CSepStatus GetSeparationStatus(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB);
		static int MachNumberTechnique(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB, string point);
};

