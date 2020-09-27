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
	// Get the positions, speeds & headings
	CPosition positionA = targetA->GetPosition().GetPosition();
	CPosition positionB = targetB->GetPosition().GetPosition();
	double headingA = targetA->GetTrackHeading();
	double headingB = targetB->GetTrackHeading();
	int gsA = targetA->GetGS();
	int gsB = targetB->GetGS();

	// Instantiate the status objecet
	CSepInfoLong returnObj;

	// Work out what kind of situation the aircraft are in (NONE, CROSSING, RECIPROCAL, SAME)
	bool isNotIntersecting = false;
	if (headingA - headingB >= -1 && headingA - headingB <= 1) {
		// They are on a same track
		returnObj.Status = CTrackLongStatus::SAME;
	}
	else {
		// They could be on an intersecting track, so test it
	}

	// If the aircraft are on same tracks
	/*if (isNotIntersecting) {
		// Get distance
		double distance = positionA.DistanceTo(positionB);

		// Get their absolute distance in time (absSpeed = distance / aircraft B's ground speed)
		int absTime = CUtils::GetTimeDistanceSpeed(round(distance), gsB);

		// Calculate the relative change in speed between the targets (negative == distance decreasing, positive == distance increasing)
		double relSpeed = (distance / gsA) - (distance / gsB);

		// Make the pair and return
		if (relSpeed < 0) {
			return make_pair(true, absTime);
		}
		else {
			return make_pair(false, absTime);
		}
	}
	else {
		// Get the absolute time between the targets
		
	}*/
}

/*static pair<bool, int> VerticalSeparation(CRadarTarget* targetA, CRadarTarget* targetB) { // Target A in front, target B following

}*/