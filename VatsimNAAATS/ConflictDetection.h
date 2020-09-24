#pragma once
#include "EuroScopePlugIn.h"
#include <vector>
#include "Constants.h"

using namespace std;
using namespace EuroScopePlugIn;

class CConflictDetection
{
	public:
		// Detect the current conflict status between two aircraft
		static CConflictStatus DetectStatusNow(CRadarTarget* targetA, CRadarTarget* targetB); // Compare with single aircraft
		static CConflictStatus DetectStatusNow(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare); // Compare with multiple aircraft
		
		// Return status predictions on the route path for a requested timeframe (one status per minute)
		static vector<CConflictStatus> PredictStatusFuturePath(CRadarTarget* targetA, CRadarTarget* targetB); // Compare with single aircraft
		static vector<CConflictStatus> PredictStatusFuturePath(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare); // Compare with multiple aircraft

		// Return status predictions on a straight line vector for a requested timeframe (one status per minute)
		static vector<CConflictStatus> PredictStatusFutureVector(CRadarTarget* targetA, CRadarTarget* targetB); // Compare with single aircraft
		static vector<CConflictStatus> PredictStatusFutureVector(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare); // Compare with multiple aircraft
};

