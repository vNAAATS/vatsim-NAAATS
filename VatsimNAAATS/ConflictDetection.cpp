#include "pch.h"
#include "ConflictDetection.h"

vector<CAircraftStatus> CConflictDetection::PIVLocations1;
vector<CAircraftStatus> CConflictDetection::PIVLocations2;
pair<bool, vector<CRoutePosition>> CConflictDetection::PIVRoute1;
pair<bool, vector<CRoutePosition>> CConflictDetection::PIVRoute2;
vector<CSepStatus> CConflictDetection::PIVSeparationStatuses;
vector<CSTCAStatus> CConflictDetection::CurrentSTCA;

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
	CPen orangePen(PS_SOLID, 1, TargetOrange.ToCOLORREF());
	CPen orangeDottedPen(PS_DOT, 1, TargetOrange.ToCOLORREF());
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
			dc->SelectObject(orangePen);
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
			dc->SelectObject(orangePen);
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
	dc->SelectObject(orangeDottedPen);
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
	DeleteObject(orangePen);
	DeleteObject(orangeDottedPen);
	DeleteObject(yellowPen);
	DeleteObject(redPen);
}

void CConflictDetection::PIVTool(CRadarScreen* screen, string targetA, string targetB) {
	// Radar targets, aircraft objects and routes
	CRadarTarget ac1 = screen->GetPlugIn()->RadarTargetSelect(targetA.c_str());
	CRadarTarget ac2 = screen->GetPlugIn()->RadarTargetSelect(targetB.c_str());
	CAircraftStatus status1(ac1.GetCallsign(), ac1.GetPosition().GetPressureAltitude(), ac1.GetGS(), ac1.GetTrackHeading(), ac1.GetPosition().GetPosition());
	CAircraftStatus status2(ac2.GetCallsign(), ac2.GetPosition().GetPressureAltitude(), ac2.GetGS(), ac2.GetTrackHeading(), ac2.GetPosition().GetPosition());

	// Aircraft statuses along route
	PIVLocations1 = GetStatusesAlongRoute(screen, status1.Callsign, status1.GroundSpeed, status1.Altitude, 1);
	PIVLocations2 = GetStatusesAlongRoute(screen, status2.Callsign, status2.GroundSpeed, status2.Altitude, 2);

	// Get the length
	int length = PIVLocations1.size() < PIVLocations2.size() ? (int)PIVLocations1.size() : PIVLocations2.size();

	for (int i = 0; i < length; i++) {
		PIVSeparationStatuses.push_back(DetectStatus(screen, &PIVLocations1.at(i), &PIVLocations2.at(i)));
	}
}

void CConflictDetection::CheckSTCA(CRadarScreen* screen, CRadarTarget* target, map<string, int>* onScreenAircraft) {
	// Status for target
	CAircraftStatus targetAc(target->GetCallsign(), target->GetPosition().GetPressureAltitude(), 
		target->GetGS(), target->GetTrackHeading(), target->GetPosition().GetPosition());

	// Detect the status
	for (auto i = onScreenAircraft->begin(); i != onScreenAircraft->end(); i++) {
		if (i->first == targetAc.Callsign) continue;
		CRadarTarget ac = screen->GetPlugIn()->RadarTargetSelect(i->first.c_str());

		// Status for this aircraft
		CAircraftStatus acTest(ac.GetCallsign(), ac.GetPosition().GetPressureAltitude(),
			ac.GetGS(), ac.GetTrackHeading(), ac.GetPosition().GetPosition());

		// Get the separation status
		CSepStatus status = DetectStatus(screen, &targetAc, &acTest);

		// Work out whether they already exist in the map
		bool alreadyExist = false;
		auto idx = CurrentSTCA.begin();
		for (idx = CurrentSTCA.begin(); idx != CurrentSTCA.end(); idx++) {
			if ((targetAc.Callsign == idx->CallsignB && acTest.Callsign == idx->CallsignA) || (targetAc.Callsign == idx->CallsignA && acTest.Callsign == idx->CallsignB)) {
				alreadyExist = true;
				break;
			}
		}

		// Now we switch the status
		switch (status.ConflictStatus) {
			case CConflictStatus::WARNING:
				// Check if they already exist
				if (alreadyExist) {
					// Set the flag if the flag is not warning
					if (idx->ConflictStatus != CConflictStatus::WARNING) idx->ConflictStatus = CConflictStatus::WARNING;
				}
				else {
					// Add if don't exist
					CurrentSTCA.push_back(CSTCAStatus(targetAc.Callsign, acTest.Callsign, CConflictStatus::WARNING));
				}
				break;
			case CConflictStatus::CRITICAL:
				// Check if they already exist
				if (alreadyExist) {
					// Set the flag if the flag is not critical
					if (idx->ConflictStatus != CConflictStatus::CRITICAL) idx->ConflictStatus = CConflictStatus::CRITICAL;
				}
				else {
					// Add if don't exist
					CurrentSTCA.push_back(CSTCAStatus(targetAc.Callsign, acTest.Callsign, CConflictStatus::CRITICAL));
				}
				break;
			case CConflictStatus::OK:
				// Check if they already exist
				if (alreadyExist) {
					// Erase if they do
					CurrentSTCA.erase(idx);
				}
				break;
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

vector<CAircraftStatus> CConflictDetection::GetStatusesAlongRoute(CRadarScreen* screen, string callsign, int groundSpeed, int altitude, int pivID) {
	// Get the route
	pair<bool, vector<CRoutePosition>> route = CPathRenderer::GetRoute(screen, callsign);

	if (pivID == 1) {
		PIVRoute1 = route;
	}
	else {
		PIVRoute2 = route;
	}

	// Status vector
	vector<CAircraftStatus> statuses;

	// Aircraft position
	CPosition acPos = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str()).GetPosition().GetPosition();

	// Iterate
	int counter = 0; // Counter to check if not first pass
	CPosition prevPos; // Previous position
	for (auto i = route.second.begin(); i != route.second.end(); i++) {
		if (i->Estimate == "--") { // If fix has been passed then continue
			continue;
		}

		// Get the time
		int time = CUtils::GetTimeDistanceSpeed(i->DistanceFromLastPoint, groundSpeed);

		// Get the heading
		int heading;
		if (counter == 0) { // If the position is the aircraft's location then get that direction
			heading = acPos.DirectionTo(i->PositionRaw);
			prevPos = acPos;
			counter++;
		}
		else {
			// Get the direction from point to point
			heading = prevPos.DirectionTo(i->PositionRaw);
		}

		// Iterate through the times and get the points
		for (int j = 0; j < time; j++) {
			CAircraftStatus status;
			status.Callsign = callsign;
			status.Altitude = altitude;
			status.GroundSpeed = groundSpeed;
			status.Heading = heading;
			if (statuses.empty()) {
				status.Position = acPos;
			}
			else {
				status.Position = CUtils::GetPointDistanceBearing(prevPos, ((i->DistanceFromLastPoint * 1852) / time) * j, heading);
			}
			// Add status
			statuses.push_back(status);
		}

		// Reassign previous position variable
		prevPos = i->PositionRaw;
	}

	// Return
	return statuses;
}