#include "pch.h"
#include "ConflictDetection.h"

/*CSepStatus CConflictDetection::DetectStatusNow(CRadarTarget* targetA, CRadarTarget* targetB) {
	
}*/

/*CConflictStatus CConflictDetection::DetectStatusNow(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFuturePath(CRadarTarget* targetA, CRadarTarget* targetB) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFuturePath(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFutureVector(CRadarTarget* targetA, CRadarTarget* targetB) {

}

vector<CConflictStatus> CConflictDetection::PredictStatusFutureVector(CRadarTarget* target, vector<CRadarTarget*>* targetsToCompare) {

}*/

CSepStatus CConflictDetection::GetSeparationStatus(CRadarScreen* screen, CAircraftStatus aircraftA, CAircraftStatus aircraftB, bool currentPointInTime) {
	// Get aircraft headings, altitudes and speeds
	double hdgA = aircraftA.Heading;
	double hdgB = aircraftB.Heading;
	int altA = aircraftA.Altitude;
	int altB = aircraftB.Altitude;
	int gsA = aircraftA.GroundSpeed;
	int gsB = aircraftB.GroundSpeed;

	// Very first thing to do is to check whether we need the separation at a specific point in space (N/A type)
	if (currentPointInTime) {

	}

	// TODO run check to make sure that they are, in fact, on same tracks
	// First check if aircraft are on same tracks
	if (!(hdgB > hdgA + 4) && !(hdgB < hdgA - 4)) {
		// They are on same tracks so check the direction
		CAircraftStatus acA;
		CAircraftStatus acB;
		bool direction = CUtils::GetAircraftDirection(hdgA);
		bool isAcAInFront = false;
		if (direction) { // Switch the direction to get the aircraft in front
			if (aircraftA.Position.m_Longitude > aircraftB.Position.m_Longitude) isAcAInFront = true;
		}
		else {
			if (aircraftA.Position.m_Longitude < aircraftB.Position.m_Longitude) isAcAInFront = true;
		}

		// Check if on a suitable line of latitude
		double latA = aircraftA.Position.m_Latitude;
		double latB = aircraftB.Position.m_Latitude;

		// Assign values
		acA.Callsign = isAcAInFront ? aircraftA.Callsign : aircraftB.Callsign;
		acB.Callsign = isAcAInFront ? aircraftB.Callsign : aircraftA.Callsign;;
		acA.Altitude = isAcAInFront ? aircraftA.Altitude : aircraftB.Altitude;
		acB.Altitude = isAcAInFront ? aircraftB.Altitude : aircraftA.Altitude;
		acA.Heading = isAcAInFront ? aircraftA.Heading : aircraftB.Heading;
		acB.Heading = isAcAInFront ? aircraftB.Heading : aircraftA.Heading;
		acA.GroundSpeed = isAcAInFront ? aircraftA.GroundSpeed : aircraftB.GroundSpeed;
		acB.GroundSpeed = isAcAInFront ? aircraftB.GroundSpeed : aircraftA.GroundSpeed;
		acA.Position = isAcAInFront ? aircraftA.Position : aircraftB.Position;
		acB.Position = isAcAInFront ? aircraftB.Position : aircraftA.Position;

		// Create a status object
		CSepStatus status;
		status.AltDifference = acA.Altitude > acB.Altitude ? acA.Altitude - acB.Altitude : acB.Altitude - acA.Altitude;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed);
		status.TrackStatus = CTrackStatus::SAME;
		status.DistanceDecreasing = CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acA.GroundSpeed);
		-CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed) < 0 ? true : false;

		// Return the status
		return status;
	}
	else if (!(abs(hdgA - hdgB) > (180 + 4)) && !(abs(hdgA - hdgB) < (180 - 4))) { // Check if on opposite tracks
		// Opposite direction, we don't care about direction so just assign to status
		CSepStatus status;
		status.AltDifference = altA > altB ? altA - altB : altB - altA;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(aircraftA.Position.DistanceTo(aircraftB.Position), gsA + gsB);
		status.DistanceDecreasing = true; // Fix this to pick up scenarios where aircraft are heading in opposite directions away from each other
		status.TrackStatus = CTrackStatus::OPPOSITE;

		// Return the status
		return status;
	}
	else { // The paths are intersecting, check whether the intercept is behind either aircraft (i.e. they will never meet)
		// Screen coordinates and direction of aircraft
		POINT p1 = screen->ConvertCoordFromPositionToPixel(aircraftA.Position);
		POINT p2 = screen->ConvertCoordFromPositionToPixel(aircraftB.Position);
		bool dir1 = CUtils::GetAircraftDirection(aircraftA.Heading);
		bool dir2 = CUtils::GetAircraftDirection(aircraftB.Heading);

		// Get intercept
		CPosition intercept = screen->ConvertCoordFromPixelToPosition(CUtils::GetIntersectionFromPointBearing(p1, p2, aircraftA.Heading, aircraftB.Heading));

		// Store result of check
		bool isValidIntercept = true;

		// Now check the coordinates against each aircraft, if point is behind the aircraft then there is no upcoming intersect
		if (dir1) {
			if (aircraftA.Position.m_Longitude > intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
		else {
			if (aircraftA.Position.m_Longitude < intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
		if (dir2) {
			if (aircraftB.Position.m_Longitude > intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
		else {
			if (aircraftB.Position.m_Longitude < intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
	}
	// Return the default status with N/A type if no detected special relationship between aircraft

}