#include "pch.h"
#include "ConflictDetection.h"

/*CConflictStatus CConflictDetection::DetectStatusNow(CRadarTarget* targetA, CRadarTarget* targetB) {
	
}

/CConflictStatus CConflictDetection::DetectStatusNow(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFuturePath(CRadarTarget* targetA, CRadarTarget* targetB) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFuturePath(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFutureVector(CRadarTarget* targetA, CRadarTarget* targetB) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFutureVector(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare) {

}*/

static pair<bool, int> LateralSeparation(CRadarTarget* targetA, CRadarTarget* targetB) { // Target A in front, target B following

}

static CSepInfoLong LongitudinalSeparation(CRadarTarget* targetA, CRadarTarget* targetB, bool useVector) { // Target A in front, target B following

}

/*static pair<bool, int> VerticalSeparation(CRadarTarget* targetA, CRadarTarget* targetB) { // Target A in front, target B following

}*/