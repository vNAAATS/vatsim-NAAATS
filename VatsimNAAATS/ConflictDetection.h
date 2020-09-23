#pragma once
#include "EuroScopePlugIn.h"
#include <vector>
#include <string>
#include <map>
#include "Constants.h"

using namespace std;
using namespace EuroScopePlugIn;

class CConflictDetection
{
	public:
		// Detect the current conflict status between two aircraft
		static ConflictStatus DetectStatusNow(CRadarTarget* targetA, CRadarTarget* targetB); // Compare with single aircraft
		static ConflictStatus DetectStatusNow(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare); // Compare with multiple aircraft
		
		// Return status predictions for a requested timeframe (one status per minute)
		static vector<ConflictStatus> PredictStatusFuture(CRadarTarget* targetA, CRadarTarget* targetB); // Compare with single aircraft
		static vector<ConflictStatus> PredictStatusFuture(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare); // Compare with multiple aircraft
};

