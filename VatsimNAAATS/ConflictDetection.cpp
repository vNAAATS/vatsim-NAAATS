#include "pch.h"
#include "ConflictDetection.h"

void CConflictDetection::RBLTool(CDC* dc, Graphics* g, CRadarScreen* screen, string target1, string target2) {
	// Save context
	int iDC = dc->SaveDC();

	// Make pens
	CPen whitePen(PS_SOLID, 1, TextWhite.ToCOLORREF());
	CPen yellowPen(PS_SOLID, 1, WarningYellow.ToCOLORREF());
	CPen redPen(PS_SOLID, 1, CriticalRed.ToCOLORREF());

	// Positions
	CRadarTarget ac1 = screen->GetPlugIn()->RadarTargetSelect(target1.c_str());
	CRadarTarget ac2 = screen->GetPlugIn()->RadarTargetSelect(target2.c_str());
	CAircraftStatus status1(ac1.GetCallsign(), ac1.GetPosition().GetPressureAltitude(), ac1.GetGS(), ac1.GetTrackHeading(), ac1.GetPosition().GetPosition());
	CAircraftStatus status2(ac2.GetCallsign(), ac2.GetPosition().GetPressureAltitude(), ac2.GetGS(), ac2.GetTrackHeading(), ac2.GetPosition().GetPosition());

	// Get status
	CSepStatus status = DetectStatus(screen, &status1, &status2);

	// Now get points in screen pixels
	POINT t1Point = screen->ConvertCoordFromPositionToPixel(status1.Position);
	POINT t2Point = screen->ConvertCoordFromPositionToPixel(status2.Position);

	// Select pen
	if (status.ConflictStatus == CConflictStatus::OK) {
		dc->SelectObject(whitePen);
	}
	else if (status.ConflictStatus == CConflictStatus::WARNING) {
		dc->SelectObject(yellowPen);
	}
	else {
		dc->SelectObject(redPen);
	}

	// Draw the line
	dc->MoveTo(t1Point);
	dc->LineTo(t2Point);

	// Now draw the text
	POINT midpoint = CUtils::GetMidPoint(t1Point, t2Point);

	FontSelector::SelectATCFont(15, dc);
	dc->SetTextColor(WarningYellow.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->SetTextCharacterExtra(2);
	dc->TextOutA(midpoint.x, midpoint.y, string(to_string(status.DistanceAsTime) + "/" + to_string(status.AltDifference)).c_str());

	// Restore context
	dc->RestoreDC(iDC);

	// Clean up
	DeleteObject(whitePen);
}

void CConflictDetection::SepTool(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB) {
	// Save context
	int iDC = dc->SaveDC();

	// Make pens
	CPen whitePen(PS_SOLID, 1, TextWhite.ToCOLORREF());
	CPen whiteDottedPen(PS_DOT, 1, TextWhite.ToCOLORREF());
	CPen yellowPen(PS_SOLID, 1, WarningYellow.ToCOLORREF());
	CPen redPen(PS_SOLID, 1, CriticalRed.ToCOLORREF());

	// Radar targets & aircraft objects
	CRadarTarget ac1 = screen->GetPlugIn()->RadarTargetSelect(targetA.c_str());
	CRadarTarget ac2 = screen->GetPlugIn()->RadarTargetSelect(targetB.c_str());
	CAircraftStatus status1(ac1.GetCallsign(), ac1.GetPosition().GetPressureAltitude(), ac1.GetGS(), ac1.GetTrackHeading(), ac1.GetPosition().GetPosition());
	CAircraftStatus status2(ac2.GetCallsign(), ac2.GetPosition().GetPressureAltitude(), ac2.GetGS(), ac2.GetTrackHeading(), ac2.GetPosition().GetPosition());

	// Save the original aircraft positions
	CPosition originalPos1 = status1.Position;
	CPosition originalPos2 = status2.Position;

	// Now we need to run the predictions, getting the statuses and time distance between aircraft every 30 seconds until the distance begins to increase
	bool foundClosestPoint = false;
	vector<CSepStatus> statuses;
	for (int time = 0; time < SEPTOOL_TIME; time += 30) {
		// If this is the first status
		if (statuses.empty()) {
			// Get the original status and add it
			CSepStatus status = DetectStatus(screen, &status1, &status2);
			statuses.push_back(status);
		}
		else { // If not the first status
			// Save the previous positions (for drawing)
			CPosition position1 = status1.Position;
			CPosition position2 = status2.Position;

			// Get the position of the aircraft at this point in time
			int distance1 = CUtils::GetDistanceSpeedTime(status1.GroundSpeed, time);
			int distance2 = CUtils::GetDistanceSpeedTime(status2.GroundSpeed, time);
			status1.Position = CUtils::GetPointDistanceBearing(originalPos1, distance1, status1.Heading);
			status2.Position = CUtils::GetPointDistanceBearing(originalPos2, distance2, status2.Heading);

			// Detect the status and select the pen
			CSepStatus status = DetectStatus(screen, &status1, &status2);

			// Compare the distance in NM to the previous status, if greater, then the previous status was the point of closest approach
			if (status.DistanceAsNM > statuses.back().DistanceAsNM) {
				// Found closest point
				foundClosestPoint = true;
				break;
			}
			else {
				// Add the status
				statuses.push_back(status);
			}
		}
	}

	// Draw line for aircraft A
	dc->MoveTo(screen->ConvertCoordFromPositionToPixel(originalPos1));
	for (vector<CSepStatus>::iterator status = statuses.begin() + 1; status != statuses.end(); status++) {
		// Select pen
		if (status->ConflictStatus == CConflictStatus::OK) {
			dc->SelectObject(whitePen);
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			dc->SelectObject(yellowPen);
		}
		else {
			dc->SelectObject(redPen);
		}
		// Draw line
		dc->LineTo(screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first));
	}

	// Draw line for aircraft B
	dc->MoveTo(screen->ConvertCoordFromPositionToPixel(originalPos2));
	for (vector<CSepStatus>::iterator status = statuses.begin() + 1; status != statuses.end(); status++) {
		// Select pen
		if (status->ConflictStatus == CConflictStatus::OK) {
			dc->SelectObject(whitePen);
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			dc->SelectObject(yellowPen);
		}
		else {
			dc->SelectObject(redPen);
		}
		// Draw line
		dc->LineTo(screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second));
	}

	// Draw line between points and finish and label
	dc->SelectObject(whiteDottedPen);
	dc->MoveTo(screen->ConvertCoordFromPositionToPixel(status1.Position));
	dc->LineTo(screen->ConvertCoordFromPositionToPixel(status2.Position));
	POINT midpoint = CUtils::GetMidPoint(screen->ConvertCoordFromPositionToPixel(status1.Position), screen->ConvertCoordFromPositionToPixel(status2.Position));
	FontSelector::SelectATCFont(15, dc);
	dc->SetTextColor(WarningYellow.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->SetTextCharacterExtra(2);
	dc->TextOutA(midpoint.x, midpoint.y , string(to_string(statuses.back().DistanceAsTime) + "/" + to_string(statuses.back().AltDifference)).c_str());

	// Restore context
	dc->RestoreDC(iDC);

	// Clean up
	DeleteObject(whitePen);
	DeleteObject(whiteDottedPen);
	DeleteObject(yellowPen);
	DeleteObject(redPen);
}

void CConflictDetection::PIVTool(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB) {
	// Make pens
	CPen whitePen(PS_SOLID, 1, TextWhite.ToCOLORREF());
	CPen yellowPen(PS_SOLID, 1, WarningYellow.ToCOLORREF());
	CPen redPen(PS_SOLID, 1, CriticalRed.ToCOLORREF());

	// Radar targets, aircraft objects and routes
	CRadarTarget ac1 = screen->GetPlugIn()->RadarTargetSelect(targetA.c_str());
	CRadarTarget ac2 = screen->GetPlugIn()->RadarTargetSelect(targetB.c_str());
	CAircraftStatus status1(ac1.GetCallsign(), ac1.GetPosition().GetPressureAltitude(), ac1.GetGS(), ac1.GetTrackHeading(), ac1.GetPosition().GetPosition());
	CAircraftStatus status2(ac2.GetCallsign(), ac2.GetPosition().GetPressureAltitude(), ac2.GetGS(), ac2.GetTrackHeading(), ac2.GetPosition().GetPosition());
	pair<bool, vector<CRoutePosition>> route1 = CPathRenderer::GetRoute(screen, status1.Callsign);
	pair<bool, vector<CRoutePosition>> route2 = CPathRenderer::GetRoute(screen, status2.Callsign);

	/// Now we loop for intercepts
	CPosition previousPos1; // Get the 1st route position as the aircraft position if it is
	if (route1.first) {
		previousPos1 = status1.Position;
	}
	// Iterate through first loop
	for (vector<CRoutePosition>::iterator route1Pos = route1.second.begin(); route1Pos != route1.second.end(); route1Pos++) {
		CPosition previousPos2; // Get the 1st route position as the aircraft position if it is
		if (route2.first) {
			previousPos2 = status2.Position;
		}
		// Iterate through second route looking for intersects
		for (vector<CRoutePosition>::iterator route2Pos = route1.second.begin(); route2Pos != route1.second.end(); route2Pos++) {

		}
	}
}

CSepStatus CConflictDetection::DetectStatus(CRadarScreen* screen, CAircraftStatus* aircraftA, CAircraftStatus* aircraftB) {
	// Get aircraft headings, altitudes and speeds
	double hdgA = aircraftA->Heading;
	double hdgB = aircraftB->Heading;
	int altA = aircraftA->Altitude;
	int altB = aircraftB->Altitude;
	int gsA = aircraftA->GroundSpeed;
	int gsB = aircraftB->GroundSpeed;

	// Default status obj
	CSepStatus status;
	status.AltDifference = altA > altB ? altA - altB : altB - altA;
	status.DistanceAsTime = gsA > gsB ? CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), aircraftA->GroundSpeed) : CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), aircraftB->GroundSpeed);
	status.DistanceAsNM = aircraftA->Position.DistanceTo(aircraftB->Position);
	status.TrackStatus = CTrackStatus::NA;
	status.AircraftLocations = make_pair(aircraftA->Position, aircraftB->Position);

	// TODO run check to make sure that they are, in fact, on same tracks
	// First check if aircraft are on same tracks
	if (!(hdgB > hdgA + 4) && !(hdgB < hdgA - 4)) {
		// They are on same tracks so check the direction
		CAircraftStatus acA;
		CAircraftStatus acB;
		bool direction = CUtils::GetAircraftDirection(hdgA);
		bool isAcAInFront = false;
		if (direction) { // Switch the direction to get the aircraft in front
			if (aircraftA->Position.m_Longitude > aircraftB->Position.m_Longitude) isAcAInFront = true;
		}
		else {
			if (aircraftA->Position.m_Longitude < aircraftB->Position.m_Longitude) isAcAInFront = true;
		}

		// Check if on a suitable line of latitude
		double latA = aircraftA->Position.m_Latitude;
		double latB = aircraftB->Position.m_Latitude;

		// Assign values
		acA.Callsign = isAcAInFront ? aircraftA->Callsign : aircraftB->Callsign;
		acB.Callsign = isAcAInFront ? aircraftB->Callsign : aircraftA->Callsign;;
		acA.Altitude = isAcAInFront ? aircraftA->Altitude : aircraftB->Altitude;
		acB.Altitude = isAcAInFront ? aircraftB->Altitude : aircraftA->Altitude;
		acA.Heading = isAcAInFront ? aircraftA->Heading : aircraftB->Heading;
		acB.Heading = isAcAInFront ? aircraftB->Heading : aircraftA->Heading;
		acA.GroundSpeed = isAcAInFront ? aircraftA->GroundSpeed : aircraftB->GroundSpeed;
		acB.GroundSpeed = isAcAInFront ? aircraftB->GroundSpeed : aircraftA->GroundSpeed;
		acA.Position = isAcAInFront ? aircraftA->Position : aircraftB->Position;
		acB.Position = isAcAInFront ? aircraftB->Position : aircraftA->Position;

		// Fill the status object
		status.AltDifference = acA.Altitude > acB.Altitude ? acA.Altitude - acB.Altitude : acB.Altitude - acA.Altitude;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed);	
		status.DistanceAsNM = acA.Position.DistanceTo(acB.Position);
		status.TrackStatus = CTrackStatus::SAME;
		-CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed) < 0 ? true : false;
	}
	else if (!(abs(hdgA - hdgB) > (180 + 4)) && !(abs(hdgA - hdgB) < (180 - 4))) { // Check if on opposite tracks
		// Opposite direction, we don't care about direction so just assign to status
		CSepStatus status;
		status.AltDifference = altA > altB ? altA - altB : altB - altA;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), gsA + gsB);
		status.DistanceAsNM = aircraftA->Position.DistanceTo(aircraftB->Position);
		status.TrackStatus = CTrackStatus::OPPOSITE;
	}
	else { // The paths are intersecting, check whether the intercept is behind either aircraft (i.e. they will never meet)
		// Screen coordinates and direction of aircraft
		POINT p1 = screen->ConvertCoordFromPositionToPixel(aircraftA->Position);
		POINT p2 = screen->ConvertCoordFromPositionToPixel(aircraftB->Position);
		bool dir1 = CUtils::GetAircraftDirection(aircraftA->Heading);
		bool dir2 = CUtils::GetAircraftDirection(aircraftB->Heading);

		// Get intercept
		CPosition intercept = screen->ConvertCoordFromPixelToPosition(CUtils::GetIntersectionFromPointBearing(p1, p2, aircraftA->Heading, aircraftB->Heading));

		// Store result of check
		bool isValidIntercept = true;

		// Now check the coordinates against each aircraft, if point is behind the aircraft then there is no upcoming intersect
		if (dir1) {
			if (aircraftA->Position.m_Longitude > intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
		else {
			if (aircraftA->Position.m_Longitude < intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
		if (dir2) {
			if (aircraftB->Position.m_Longitude > intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}
		else {
			if (aircraftB->Position.m_Longitude < intercept.m_Longitude) {
				isValidIntercept = false;
			}
		}

		// Fill the status if valid intercept
		if (isValidIntercept) {
			status.AltDifference = altA > altB ? altA - altB : altB - altA;
			status.DistanceAsTime = gsA > gsB ? CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), aircraftA->GroundSpeed) : CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), aircraftB->GroundSpeed);
			status.DistanceAsNM = aircraftA->Position.DistanceTo(aircraftB->Position);
			status.TrackStatus = CUtils::GetPathAngle(hdgA, hdgB) < 45 ? CTrackStatus::RECIPROCAL : CTrackStatus::CROSSING;
		}
	}

	/// Conflict status detection
	// Are RVSM
	bool rvsm = aircraftA->Altitude <= 41000 || aircraftB->Altitude <= 41000;

	// Check altitude
	bool verticallySeparated = true;
	if (rvsm) {
		if (status.AltDifference < 1000) {
			verticallySeparated = false;
		}
	}
	else {
		if (status.AltDifference < 2000) {
			verticallySeparated = false;
		}
	}

	// Conflict flag (OK default)
	CConflictStatus conflictStatus = CConflictStatus::OK;

	/// Check separation (eventually base it on equipment code, therefore adding a use for the CTrackStatus flag)
	/// Separation currently fixed to: Lateral = 23NM, Longitudinal = 5 minutes overall inside the OCA
	if (status.DistanceAsTime <= SEPLON_REDUCEDWARN && status.DistanceAsTime > SEPLON_REDUCED) {
		// Distance not met, so check altitude
		if (!verticallySeparated) {
			conflictStatus = CConflictStatus::WARNING;
		}
	}
	else if (status.DistanceAsTime <= SEPLON_REDUCED) {
		// Distance not met, so check altitude
		if (!verticallySeparated) {
			conflictStatus = CConflictStatus::CRITICAL;
		}
	}

	// Assign conflict status
	status.ConflictStatus = conflictStatus;

	// Return the status
	return status;
}