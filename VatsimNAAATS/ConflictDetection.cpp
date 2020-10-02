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

/*static pair<bool, int> LateralSeparation(CRadarTarget* targetA, CRadarTarget* targetB) { // Target A in front, target B following

}*/

CSepStatus CConflictDetection::LongitudinalSeparation(CRadarTarget* targetA, CRadarTarget* targetB) { // Target A in front, target B behind (direction irrelevant)
	// Get aircraft headings, altitudes and speeds
	double hdgA = targetA->GetTrackHeading();
	double hdgB = targetB->GetTrackHeading();
	int altA = targetA->GetPosition().GetPressureAltitude();
	int altB = targetB->GetPosition().GetPressureAltitude();
	int gsA = targetA->GetGS();
	int gsB = targetB->GetGS();
	
	// TODO run check to make sure that they are, in fact, on same tracks
	// First check if aircraft are on same tracks
	if (!(hdgB > hdgA + 4) && !(hdgB < hdgA - 4)) {
		// They are on same tracks so check the direction
		CAircraftStatus acA;
		CAircraftStatus acB;
		bool direction = CUtils::GetAircraftDirection(hdgA);
		bool isAcAInFront = false;
		if (direction) { // Switch the direction to get the aircraft in front
			if (targetA->GetPosition().GetPosition().m_Longitude > targetB->GetPosition().GetPosition().m_Longitude) isAcAInFront = true;
		}
		else {
			if (targetA->GetPosition().GetPosition().m_Longitude < targetB->GetPosition().GetPosition().m_Longitude) isAcAInFront = true;
		}

		// Check if on a suitable line of latitude
		double latA = targetA->GetPosition().GetPosition().m_Latitude;
		double latB = targetB->GetPosition().GetPosition().m_Latitude;

		// Assign values
		acA.Callsign = isAcAInFront ? targetA->GetCallsign() : targetB->GetCallsign();
		acB.Callsign = isAcAInFront ? targetB->GetCallsign() : targetA->GetCallsign();
		acA.Altitude = isAcAInFront ? altA : altB;
		acB.Altitude = isAcAInFront ? altB : altA;
		acA.Heading = isAcAInFront ? hdgA : hdgB;
		acB.Heading = isAcAInFront ? hdgB : hdgA;
		acA.GroundSpeed = isAcAInFront ? gsA : gsB;
		acB.GroundSpeed = isAcAInFront ? gsB : gsA;
		acA.Position = isAcAInFront ? targetA->GetPosition().GetPosition() : targetB->GetPosition().GetPosition();
		acB.Position = isAcAInFront ? targetB->GetPosition().GetPosition() : targetA->GetPosition().GetPosition();

		// Create a status object
		CSepStatus status;
		status.AltDifference = acA.Altitude > acB.Altitude ? acA.Altitude - acB.Altitude : acB.Altitude - acA.Altitude;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed);
		status.TrackStatus = CTrackStatus::SAME;
		status.DistanceDecreasing = CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acA.GroundSpeed);
			- CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed) < 0 ? true : false;

		// Return the status
		return status;
	}
	else if (!(abs(hdgA - hdgB) > (180 + 4)) && !(abs(hdgA - hdgB) < (180 - 4))) { // Check if on opposite tracks
		// Opposite direction, we don't care about direction so just assign to status
		CSepStatus status;
		status.AltDifference = altA > altB ? altA - altB : altB - altA;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(targetA->GetPosition().GetPosition().DistanceTo(targetB->GetPosition().GetPosition()), gsA + gsB);
		status.DistanceDecreasing = true; // Fix this to pick up scenarios where aircraft are heading in opposite directions away from each other
		status.TrackStatus = CTrackStatus::OPPOSITE;

		// Return the status
		return status;
	}

	// If not on same track, the paths are intercepting, check if they will ever meet
}

/*static pair<bool, int> VerticalSeparation(CRadarTarget* targetA, CRadarTarget* targetB) { // Target A in front, target B following

}*/