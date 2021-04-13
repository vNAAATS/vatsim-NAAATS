#include "pch.h"
#include "ConflictDetection.h"

vector<CAircraftStatus> CConflictDetection::PIVLocations1;
vector<CAircraftStatus> CConflictDetection::PIVLocations2;
vector<CRoutePosition> CConflictDetection::PIVRoute1;
vector<CRoutePosition> CConflictDetection::PIVRoute2;
vector<CSepStatus> CConflictDetection::PIVSeparationStatuses;
vector<CSTCAStatus> CConflictDetection::CurrentSTCA;
map<string, int>* CConflictDetection::aircraftOnScreen;

void CConflictDetection::RBLTool(CDC* dc, Graphics* g, CRadarScreen* screen, string target1, string target2) {
	// Save context
	int iDC = dc->SaveDC();

	// Make pens
	Pen orangePen(TargetOrange, 2);
	Pen yellowPen(WarningYellow, 2);
	Pen redPen(CriticalRed, 2);

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
		g->DrawLine(&orangePen, t1Point.x, t1Point.y, t2Point.x, t2Point.y);
	}
	else if (status.ConflictStatus == CConflictStatus::WARNING) {
		g->DrawLine(&yellowPen, t1Point.x, t1Point.y, t2Point.x, t2Point.y);
	}
	else {
		g->DrawLine(&redPen, t1Point.x, t1Point.y, t2Point.x, t2Point.y);
	}

	// Now draw the text
	POINT midpoint = CUtils::GetMidPoint(t1Point, t2Point);

	// Font and colour
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->SetTextCharacterExtra(2);
	dc->TextOutA(midpoint.x, midpoint.y, string(to_string(status.DistanceAsNM) + "(" + to_string(status.DistanceAsTime) + ")/" + to_string(status.AltDifference)).c_str());

	// Restore context
	dc->RestoreDC(iDC);

	// Clean up
	DeleteObject(&orangePen);
	DeleteObject(&yellowPen);
	DeleteObject(&redPen);
}

void CConflictDetection::SepTool(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB) {
	// Save context
	int iDC = dc->SaveDC();

	// Make pens
	Pen orangePen(TargetOrange, 2);
	Pen yellowPen(WarningYellow, 2);
	Pen redPen(CriticalRed, 2);

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
	for (vector<CSepStatus>::iterator status = statuses.begin() + 1; status != statuses.end(); status++) {
		// Select pen
		if (status->ConflictStatus == CConflictStatus::OK) {
			g->DrawLine(&orangePen, screen->ConvertCoordFromPositionToPixel(originalPos1).x, screen->ConvertCoordFromPositionToPixel(originalPos1).y,
				screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first).x, screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first).y);
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			g->DrawLine(&yellowPen, screen->ConvertCoordFromPositionToPixel(originalPos1).x, screen->ConvertCoordFromPositionToPixel(originalPos1).y,
				screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first).x, screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first).y);
		}
		else {
			g->DrawLine(&redPen, screen->ConvertCoordFromPositionToPixel(originalPos1).x, screen->ConvertCoordFromPositionToPixel(originalPos1).y,
				screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first).x, screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.first).y);
		}

		// Set pos
		originalPos1 = status->AircraftLocations.first;
	}

	// Draw line for aircraft B
	for (vector<CSepStatus>::iterator status = statuses.begin() + 1; status != statuses.end(); status++) {
		// Select pen
		if (status->ConflictStatus == CConflictStatus::OK) {
			g->DrawLine(&orangePen, screen->ConvertCoordFromPositionToPixel(originalPos2).x, screen->ConvertCoordFromPositionToPixel(originalPos2).y,
				screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second).x, screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second).y);
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			g->DrawLine(&yellowPen, screen->ConvertCoordFromPositionToPixel(originalPos2).x, screen->ConvertCoordFromPositionToPixel(originalPos2).y,
				screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second).x, screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second).y);
		}
		else {
			g->DrawLine(&redPen, screen->ConvertCoordFromPositionToPixel(originalPos2).x, screen->ConvertCoordFromPositionToPixel(originalPos2).y,
				screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second).x, screen->ConvertCoordFromPositionToPixel(status->AircraftLocations.second).y);
		}
		// Set pos
		originalPos2 = status->AircraftLocations.second;
	}

	// Draw line between points and finish
	orangePen.SetDashStyle(DashStyleDash);
	orangePen.SetDashCap(DashCapRound);
	g->DrawLine(&orangePen, screen->ConvertCoordFromPositionToPixel(status1.Position).x, screen->ConvertCoordFromPositionToPixel(status1.Position).y,
		screen->ConvertCoordFromPositionToPixel(status2.Position).x, screen->ConvertCoordFromPositionToPixel(status2.Position).y);
	POINT midpoint = CUtils::GetMidPoint(screen->ConvertCoordFromPositionToPixel(status1.Position), screen->ConvertCoordFromPositionToPixel(status2.Position));
	
	// Text label plus colour switching
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->SetTextCharacterExtra(2);
	dc->TextOutA(midpoint.x, midpoint.y, string(to_string(statuses.back().DistanceAsNM) + "(" + to_string(statuses.back().DistanceAsTime) + ")/" + to_string(statuses.back().AltDifference)).c_str());

	// Restore context
	dc->RestoreDC(iDC);

	// Clean up
	DeleteObject(&orangePen);
	DeleteObject(&yellowPen);
	DeleteObject(&redPen);
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

	// Set separation statuses
	for (int i = 0; i < length; i++) {
		PIVSeparationStatuses.push_back(DetectStatus(screen, &PIVLocations1.at(i), &PIVLocations2.at(i)));
	}
}

bool CConflictDetection::ProbeTool(CRadarScreen* screen, string callsign, map<string, vector<CSepStatus>>* statuses, CAircraftFlightPlan* copy) {
	// Get the aircraft flight plan
	CAircraftFlightPlan* fp = copy != nullptr ? copy : CDataHandler::GetFlightData(callsign);

	// Clear statuses first up
	statuses->clear();

	// Radar target
	CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str());

	// Statuses for query aircraft
	vector<CAircraftStatus> acStatuses = GetStatusesAlongRoutePoints(screen, callsign, (int)(round((stod(fp->Mach) / 100.0)) * 576.0), stoi(fp->FlightLevel) * 100);

	vector<string> callsigns;

	// Check statuses against on screen aircraft
	for (auto i = aircraftOnScreen->begin(); i != aircraftOnScreen->end(); i++) {
		// Of course, skip the instance of the same aircraft
		if (i->first == callsign) {
			continue;
		}

		// Sep status vector
		vector<CSepStatus> sepStatuses;

		// Tartget and statuses
		target = screen->GetPlugIn()->RadarTargetSelect(i->first.c_str());
		vector<CAircraftStatus> loopAcStatuses = GetStatusesAlongRoutePoints(screen, i->first, target.GetGS(), target.GetPosition().GetPressureAltitude());

		// Get the shortest vector (this is what we set the loop end to so we don't get an overflow)
		int end = loopAcStatuses.size() > acStatuses.size() ? acStatuses.size() : loopAcStatuses.size();

		// Has a warning or critical
		bool addToVector = false;

		// Loop through main aircraft statuses
		for (int i = 0; i < end; i++) {
			// Separation status
			CSepStatus separation = DetectStatus(screen, &acStatuses[i], &loopAcStatuses[i]);

			// If there is a conflict then set the boolean
			if (separation.ConflictStatus == CConflictStatus::CRITICAL || separation.ConflictStatus == CConflictStatus::WARNING)
				addToVector = true;

			// Push back
			sepStatuses.push_back(separation);
		}

		// Push back the vector if a conflict
		if (addToVector) {
			callsigns.push_back(target.GetCallsign());
			statuses->insert({ (string)target.GetCallsign(), sepStatuses });
		}
	}

	return 1;
}

void CConflictDetection::RenderPIV(CDC* dc, Graphics* g, CRadarScreen* screen, string targetA, string targetB) {
	// Save context
	int iDC = dc->SaveDC();

	// Pens & brush
	Pen pen(TargetOrange, 2);
	Pen yellowPen(WarningYellow, 2);
	Pen redPen(CriticalRed, 2);
	SolidBrush brush(TargetOrange);

	// Font
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(TargetOrange.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Anti-aliasing
	g->SetSmoothingMode(SmoothingModeAntiAlias);

	// Hold these for drawing between aircraft and point
	POINT lastPoint1;
	POINT lastPoint2;

	// Begin drawing
	if (!PIVRoute1.empty()) { // Failsafe
		// Radar target
		CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(targetA.c_str());
		POINT aircraftPos = screen->ConvertCoordFromPositionToPixel(target.GetPosition().GetPosition());
		lastPoint1 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVRoute1.begin()->PositionRaw);
		for (auto i = CConflictDetection::PIVRoute1.begin(); i != CConflictDetection::PIVRoute1.end(); i++) {
			POINT point = screen->ConvertCoordFromPositionToPixel(i->PositionRaw);
			// Draw text and dot only if not AIRCRAFT fix
			if (i->Fix != "AIRCRAFT") {
				// Get point, text rectangle & define y offset
				string text = i->Fix; // To get TextExtent				
				CRect box(point.x - (dc->GetTextExtent(text.c_str()).cx / 2), point.y + 10, point.x + (dc->GetTextExtent(text.c_str()).cx), point.y + 50);
				int offsetY = 0;

				// Draw dot
				Rect pointRect(point.x - 3, point.y - 3, 6, 6);
				g->FillEllipse(&brush, pointRect);

				// Print text for fix
				dc->TextOutA(box.left, box.top, text.c_str());
				offsetY += 14;

				// Print text for estimate
				text = i->Estimate;
				dc->TextOutA(box.left, box.top + offsetY, text.c_str());
				offsetY += 14;

				// Print text for flight level
				text = to_string(i->FlightLevel);
				dc->TextOutA(box.left, box.top + offsetY, text.c_str());
			}
			// Draw the line between points if there is no estimate (point is behind the aircraft)
			if (i->Estimate == "--") {
				// Draw line
				g->DrawLine(&pen, lastPoint1.x, lastPoint1.y, point.x, point.y);
				lastPoint1 = point;

				// If next point is either AIRCRAFT, or the estimate is positive draw line between last point and target
				if (i != PIVRoute1.end() - 1) {
					if (std::next(i, 1)->Fix == "AIRCRAFT" || std::next(i, 1)->Estimate != "--") {
						g->DrawLine(&pen, lastPoint1.x, lastPoint1.y, aircraftPos.x, aircraftPos.y);
						lastPoint1 = aircraftPos;
					}
				}
			}
		}
	}
	if (!PIVRoute2.empty()) { // Failsafe
		CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(targetB.c_str());
		POINT aircraftPos = screen->ConvertCoordFromPositionToPixel(target.GetPosition().GetPosition());
		lastPoint2 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVRoute2.begin()->PositionRaw);
		for (auto i = CConflictDetection::PIVRoute2.begin(); i != CConflictDetection::PIVRoute2.end(); i++) {
			POINT point = screen->ConvertCoordFromPositionToPixel(i->PositionRaw);
			// Draw text and dot only if not AIRCRAFT fix
			if (i->Fix != "AIRCRAFT") {
				// Get point, text rectangle & define y offset
				string text = i->Fix; // To get TextExtent
				CRect box(point.x - (dc->GetTextExtent(text.c_str()).cx / 2), point.y + 10, point.x + (dc->GetTextExtent(text.c_str()).cx), point.y + 50);
				int offsetY = 0;

				// Draw dot
				Rect pointRect(point.x - 3, point.y - 3, 6, 6);
				g->FillEllipse(&brush, pointRect);

				// Print text for fix
				dc->TextOutA(box.left, box.top, text.c_str());
				offsetY += 14;

				// Print text for estimate
				text = i->Estimate;
				dc->TextOutA(box.left, box.top + offsetY, text.c_str());
				offsetY += 14;

				// Print text for flight level
				text = to_string(i->FlightLevel);
				dc->TextOutA(box.left, box.top + offsetY, text.c_str());
			}

			// Draw the line between points if there is no estimate (point is behind the aircraft)
			if (i->Estimate == "--") {
				// Draw line
				g->DrawLine(&pen, lastPoint2.x, lastPoint2.y, point.x, point.y);
				lastPoint2 = point;

				// If next point is either AIRCRAFT, or the estimate is positive draw line between last point and target
				if (i != PIVRoute2.end() - 1) {
					if (std::next(i, 1)->Fix == "AIRCRAFT" || std::next(i, 1)->Estimate != "--") {
						g->DrawLine(&pen, lastPoint2.x, lastPoint2.y, aircraftPos.x, aircraftPos.y);
						lastPoint2 = aircraftPos;
					}
				}				
			}
		}
	}

	// Draw conflicts
	int i = 0;
	CConflictStatus previousStatus = CConflictDetection::PIVSeparationStatuses.front().ConflictStatus;
	CRadarTarget tA = screen->GetPlugIn()->RadarTargetSelect(targetA.c_str());
	CRadarTarget tB = screen->GetPlugIn()->RadarTargetSelect(targetB.c_str());
	for (i;  i < CConflictDetection::PIVSeparationStatuses.size(); i++) {
		// Points
		POINT piv1 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations1.at(i).Position);
		POINT piv2 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations2.at(i).Position);
		// Select pen
		if (CConflictDetection::PIVSeparationStatuses.at(i).ConflictStatus == CConflictStatus::OK) {
			g->DrawLine(&pen, lastPoint1.x, lastPoint1.y, piv1.x, piv1.y);
			g->DrawLine(&pen, lastPoint2.x, lastPoint2.y, piv2.x, piv2.y);
		}
		else if (CConflictDetection::PIVSeparationStatuses.at(i).ConflictStatus == CConflictStatus::WARNING) {
			g->DrawLine(&yellowPen, lastPoint1.x, lastPoint1.y, piv1.x, piv1.y);
			g->DrawLine(&yellowPen, lastPoint2.x, lastPoint2.y, piv2.x, piv2.y);
		}
		else {
			g->DrawLine(&redPen, lastPoint1.x, lastPoint1.y, piv1.x, piv1.y);
			g->DrawLine(&redPen, lastPoint2.x, lastPoint2.y, piv2.x, piv2.y);
		}

		// Text
		dc->SetTextColor(WarningYellow.ToCOLORREF());
		dc->SetTextAlign(TA_CENTER);

		// Draw times
		if (previousStatus != CConflictDetection::PIVSeparationStatuses.at(i).ConflictStatus) {
			// Get times (subtract 1 because previous estimate is needed)
			string time1 = CUtils::ParseZuluTime(false, CConflictDetection::PIVLocations1.at(i).Estimate - 1);
			string time2 = CUtils::ParseZuluTime(false, CConflictDetection::PIVLocations2.at(i).Estimate - 1);

			// Print text for both
			dc->TextOutA(lastPoint1.x, lastPoint1.y - 20, time1.c_str());
			dc->TextOutA(lastPoint2.x, lastPoint2.y - 20, time2.c_str());

			// Draw crosses
			yellowPen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
			g->DrawLine(&yellowPen, lastPoint1.x - 5, lastPoint1.y - 5, lastPoint1.x + 5, lastPoint1.y + 5); // AC1
			g->DrawLine(&yellowPen, lastPoint1.x - 5, lastPoint1.y + 5, lastPoint1.x + 5, lastPoint1.y - 5); // AC1
			g->DrawLine(&yellowPen, lastPoint2.x - 5, lastPoint2.y - 5, lastPoint2.x + 5, lastPoint2.y + 5); // AC2
			g->DrawLine(&yellowPen, lastPoint2.x - 5, lastPoint2.y + 5, lastPoint2.x + 5, lastPoint2.y - 5); // AC2

		}

		// Set previous status
		previousStatus = CConflictDetection::PIVSeparationStatuses.at(i).ConflictStatus;

		// Reset points
		lastPoint1 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations1.at(i).Position);
		lastPoint2 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations2.at(i).Position);
	}

	// Now draw the remainder of the line for the longer aircraft
	int length = CConflictDetection::PIVLocations1.size() > CConflictDetection::PIVLocations2.size()
		? (int)CConflictDetection::PIVLocations1.size() : CConflictDetection::PIVLocations2.size();
	// Bool to hold which is longer
	bool isLonger = CConflictDetection::PIVLocations1.size() > CConflictDetection::PIVLocations2.size()
		? true : false;
	POINT pointToDraw;
	if (isLonger) {
		pointToDraw = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations1.at(i - 1).Position);
	}
	else {
		pointToDraw = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations2.at(i - 1).Position);
	}

	// Cap type
	yellowPen.SetLineCap(LineCapSquare, LineCapSquare, DashCapRound);

	// Iterate and draw
	for (i; i < length; i++) {
		// Select orange pen
		if (isLonger) {
			try {
				POINT pt = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations1.at(i).Position);
				g->DrawLine(&pen, pointToDraw.x, pointToDraw.y, pt.x, pt.y);
				dc->MoveTo(pointToDraw);
				pointToDraw = pt;
			}
			catch (exception & ex) {
				continue;
			}
		}
		else {
			try {
				POINT pt = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations2.at(i).Position);
				g->DrawLine(&pen, pointToDraw.x, pointToDraw.y, pt.x, pt.y);
				dc->MoveTo(pointToDraw);
				pointToDraw = pt;
			}
			catch (exception & ex) {
				continue;
			}
		}

	}

	// Cleanup
	DeleteObject(&pen);
	DeleteObject(&yellowPen);
	DeleteObject(&redPen);
	DeleteObject(&brush);
	
	// Restore context
	dc->RestoreDC(iDC);
}

void CConflictDetection::CheckSTCA(CRadarScreen* screen, CRadarTarget* target, map<string, int>* onScreenAircraft) {
	// Set on screen aircraft
	aircraftOnScreen = onScreenAircraft;

	// Status for target
	CAircraftStatus targetAc(target->GetCallsign(), target->GetPosition().GetPressureAltitude(), 
		target->GetGS(), target->GetTrackHeading(), target->GetPosition().GetPosition());

	// Detect the status
	for (auto i = onScreenAircraft->begin(); i != onScreenAircraft->end(); i++) {
		if (i->first == targetAc.Callsign)
			continue;
		CRadarTarget ac = screen->GetPlugIn()->RadarTargetSelect(i->first.c_str());

		// Status for this aircraft
		CAircraftStatus acTest(ac.GetCallsign(), ac.GetPosition().GetPressureAltitude(),
			ac.GetGS(), ac.GetTrackHeading(), ac.GetPosition().GetPosition());

		// Get the separation status
		CSepStatus status = DetectStatus(screen, &targetAc, &acTest);

		if (status.DistanceAsTime < 0)
			continue;

		// Work out whether they already exist in the map
		bool alreadyExist = false;
		auto idx = CurrentSTCA.begin();
		for (idx = CurrentSTCA.begin(); idx != CurrentSTCA.end(); idx++) {
			if ((targetAc.Callsign == idx->CallsignB && acTest.Callsign == idx->CallsignA) || (targetAc.Callsign == idx->CallsignA && acTest.Callsign == idx->CallsignB)) {
				alreadyExist = true;
				break;
			}
		}

		// Check validity of aircraft and delete if 1 aircraft becomes irrelevant
		if (alreadyExist) {
			// Check relevance
			if (!CUtils::IsAircraftRelevant(screen, target)) {
				// Not relevant so remove from map and return
				CurrentSTCA.erase(idx);
				return;
			}
		}

		// Now we switch the status
		switch (status.ConflictStatus) {
			case CConflictStatus::CRITICAL:
				// Check if they already exist
				if (alreadyExist) {
					// Set the flag if the flag is not critical
					if (idx->ConflictStatus != CConflictStatus::CRITICAL) idx->ConflictStatus = CConflictStatus::CRITICAL;
					idx->DistanceAsTime = status.DistanceAsTime;
				}
				else {
					// Add if don't exist
					CurrentSTCA.push_back(CSTCAStatus(targetAc.Callsign, acTest.Callsign, CConflictStatus::CRITICAL, status.DistanceAsTime, status.DistanceAsNM));
				}
				break;
			case CConflictStatus::WARNING:
				// Check if they already exist
				if (alreadyExist) {
					// Set the flag if the flag is not warning
					if (idx->ConflictStatus != CConflictStatus::WARNING) idx->ConflictStatus = CConflictStatus::WARNING;
					idx->DistanceAsTime = status.DistanceAsTime;
				}
				else {
					// Add if don't exist
					CurrentSTCA.push_back(CSTCAStatus(targetAc.Callsign, acTest.Callsign, CConflictStatus::WARNING, status.DistanceAsTime, status.DistanceAsNM));
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

	// Targets
	CRadarTarget targetA = screen->GetPlugIn()->RadarTargetSelect(aircraftA->Callsign.c_str());
	CRadarTarget targetB = screen->GetPlugIn()->RadarTargetSelect(aircraftB->Callsign.c_str());

	// vNAAATS flight plans
	CAircraftFlightPlan fp1;
	CAircraftFlightPlan fp2;
	CDataHandler::GetFlightData(targetA.GetCallsign(), fp1);
	CDataHandler::GetFlightData(targetB.GetCallsign(), fp2);

	// Default status obj
	CSepStatus status;
	status.AltDifference = altA > altB ? altA - altB : altB - altA;
	status.DistanceAsTime = gsA > gsB ? CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), aircraftA->GroundSpeed) : CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), aircraftB->GroundSpeed);
	status.DistanceAsNM = aircraftA->Position.DistanceTo(aircraftB->Position);
	CTrackStatus trackStatus = CTrackStatus::NA;
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
		trackStatus = CTrackStatus::SAME;
		status.IsDistanceClosing = CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acA.GroundSpeed)
			- CUtils::GetTimeDistanceSpeed(acA.Position.DistanceTo(acB.Position), acB.GroundSpeed) < 0 ? true : false;
	}
	else if (!(abs(hdgA - hdgB) > (180 + 4)) && !(abs(hdgA - hdgB) < (180 - 4))) { // Check if on opposite tracks
		// Opposite direction, we don't care about direction so just assign to status
		CSepStatus status;
		status.AltDifference = altA > altB ? altA - altB : altB - altA;
		status.DistanceAsTime = CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), gsA + gsB);
		status.DistanceAsNM = aircraftA->Position.DistanceTo(aircraftB->Position);
		status.IsDistanceClosing = CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), gsA + gsB) < 0 ? true : false;
		trackStatus = CTrackStatus::OPPOSITE;
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
			trackStatus = CUtils::GetPathAngle(hdgA, hdgB) < 45 ? CTrackStatus::RECIPROCAL : CTrackStatus::CROSSING;
			status.IsDistanceClosing = CUtils::GetTimeDistanceSpeed(aircraftA->Position.DistanceTo(aircraftB->Position), gsA > gsB ? gsA : gsB) < 0 ? true : false;
		}
	}

	// Set track status
	status.TrackStatus = trackStatus;

	/// Conflict status detection
	// Are RVSM
	bool rvsm = aircraftA->Altitude <= 41000 || aircraftB->Altitude <= 41000;

	// Check altitude
	bool verticallySeparated = true;
	if (rvsm) {
		if (status.AltDifference < SEPV_LOW) {
			verticallySeparated = false;
		}
	}
	else {
		if (status.AltDifference < SEPV_HIGH) {
			verticallySeparated = false;
		}
	}
	// If either are supersonic & concorde
	if (CUtils::GetMach(aircraftA->GroundSpeed, 573) >= 100 || CUtils::GetMach(aircraftB->GroundSpeed, 573) >= 100) {
		// Vertical sep needs to be greater than 4000 and either type needs to be concorde
		if (status.AltDifference < SEPV_SUPERSONIC 
			&& (screen->GetPlugIn()->FlightPlanSelect(aircraftA->Callsign.c_str()).GetFlightPlanData().GetAircraftFPType() == "CONC"
				|| screen->GetPlugIn()->FlightPlanSelect(aircraftB->Callsign.c_str()).GetFlightPlanData().GetAircraftFPType() == "CONC")) { 
			verticallySeparated = false;
		}
	}

	// Conflict flag (OK default)
	CConflictStatus conflictStatus = CConflictStatus::OK;

	/// Check separation using equipment code reference
	if (fp1.IsEquipped && fp2.IsEquipped) { // if both aircraft are AGCS Equipped
		// Use reduced minima
		if (status.TrackStatus == CTrackStatus::SAME || status.TrackStatus == CTrackStatus::RECIPROCAL || status.TrackStatus == CTrackStatus::OPPOSITE) {
			if (status.DistanceAsNM <= SEPLON_REDUCEDWARN && status.DistanceAsNM >= SEPLON_REDUCED) { // Same tracks
			// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::WARNING;
				}
			}
			else if (status.DistanceAsNM < SEPLON_REDUCED) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::CRITICAL;
				}
			}
		}
		else if (status.TrackStatus == CTrackStatus::CROSSING) {
			if (status.DistanceAsTime <= SEPLON_NONRED_X_WARN && status.DistanceAsTime >= SEPLON_NONRED_X) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::WARNING;
				}
			}
			else if (status.DistanceAsTime < SEPLON_NONRED_X) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::CRITICAL;
				}
			}
		}
		
	}
	else {
		// Use non-reduced minima
		if (status.TrackStatus == CTrackStatus::SAME || status.TrackStatus == CTrackStatus::RECIPROCAL || status.TrackStatus == CTrackStatus::OPPOSITE) { // Same tracks
			if (status.DistanceAsTime <= SEPLON_NONRED_SAME_WARN && status.DistanceAsTime >= SEPLON_NONRED_SAME) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::WARNING;
				}
			}
			else if (status.DistanceAsTime < SEPLON_NONRED_SAME) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::CRITICAL;
				}
			}
		}
		else if (status.TrackStatus == CTrackStatus::CROSSING) { // Crossing tracks
			if (status.DistanceAsTime <= SEPLON_NONRED_X_WARN && status.DistanceAsTime >= SEPLON_NONRED_X) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::WARNING;
				}
			}
			else if (status.DistanceAsTime < SEPLON_NONRED_X) {
				// Distance not met, so check altitude
				if (!verticallySeparated) {
					conflictStatus = CConflictStatus::CRITICAL;
				}
			}
		}
	}
	

	// Last positions
	CPosition targetALast = targetA.GetPreviousPosition(targetA.GetPosition()).GetPosition();
	CPosition targetBLast = targetB.GetPreviousPosition(targetB.GetPosition()).GetPosition();

	// If last position distance closer than the current position distance they are heading away from each other and no conflict
	if (targetALast.DistanceTo(targetBLast) < targetA.GetPosition().GetPosition().DistanceTo(targetB.GetPosition().GetPosition()) 
		&& targetALast.DistanceTo(targetBLast) > 5 && status.TrackStatus != CTrackStatus::OPPOSITE) { // Greater than 5 miles away to remove the conflict
		// No conflict
		conflictStatus = CConflictStatus::OK;
	}

	// Assign conflict status
	status.ConflictStatus = conflictStatus;

	// Return the status
	return status;
}

vector<CAircraftStatus> CConflictDetection::GetStatusesAlongRoute(CRadarScreen* screen, string callsign, int groundSpeed, int altitude, int pivID) {
	// Get the route
	vector<CRoutePosition> route;
	CRoutesHelper::GetRoute(screen, &route, callsign);

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
	int totalTime = 0;
	for (int i = 0; i < route.size(); i++) {
		if (route.at(i).Estimate == "--") { // If fix has been passed then continue
			continue;
		}

		// Get the time
		int time = CUtils::GetTimeDistanceSpeed(route.at(i).DistanceFromLastPoint, groundSpeed);

		// Get the heading
		int heading;
		if (counter == 0) { // If the position is the aircraft's location then get that direction
			heading = acPos.DirectionTo(route.at(i).PositionRaw);
			prevPos = acPos;
			counter++;
		}
		else {
			// Get the direction from point to point
			heading = prevPos.DirectionTo(route.at(i).PositionRaw);
		}

		// Iterate through the times and get the points
		for (int j = 0; j < time; j++) {
			totalTime++;
			CAircraftStatus status;
			status.Callsign = callsign;
			status.Altitude = altitude;
			status.GroundSpeed = groundSpeed;
			status.Heading = heading;
			status.Estimate = totalTime;
			if (statuses.empty()) {
				status.Position = acPos;
			}
			else {
				status.Position = CUtils::GetPointDistanceBearing(prevPos, ((route.at(i).DistanceFromLastPoint * 1852) / time) * j, heading);
			}
			// Add status
			statuses.push_back(status);
		}

		// Reassign previous position variable
		prevPos = route.at(i).PositionRaw;
	}

	// Return
	return statuses;
}

vector<CAircraftStatus> CConflictDetection::GetStatusesAlongRoutePoints(CRadarScreen* screen, string callsign, int groundSpeed, int altitude) {
	// Get the route
	vector<CRoutePosition> route;
	CRoutesHelper::GetRoute(screen, &route, callsign);

	// Status vector
	vector<CAircraftStatus> statuses;

	// Aircraft position
	CPosition acPos = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str()).GetPosition().GetPosition();

	// Iterate
	int counter = 0; // Counter to check if not first pass
	CPosition prevPos; // Previous position
	int totalTime = 0;
	for (int i = 0; i < route.size(); i++) {
		if (route.at(i).Estimate == "--") { // If fix has been passed then continue
			continue;
		}

		// Get the time
		int time = CUtils::GetTimeDistanceSpeed(route.at(i).DistanceFromLastPoint, groundSpeed);
		totalTime += time;

		// Get the heading
		int heading;
		if (counter == 0) { // If the position is the aircraft's location then get that direction
			heading = acPos.DirectionTo(route.at(i).PositionRaw);
			prevPos = acPos;
			counter++;
		}
		else {
			// Get the direction from point to point
			heading = prevPos.DirectionTo(route.at(i).PositionRaw);
		}

		CAircraftStatus status;
		status.Callsign = callsign;
		status.Altitude = altitude;
		status.GroundSpeed = groundSpeed;
		status.Heading = heading;
		status.Estimate = totalTime;
		if (statuses.empty()) {
			status.Position = acPos;
		}
		else {
			status.Position = route.at(i).PositionRaw;
		}
		// Add status
		statuses.push_back(status);

		// Reassign previous position variable
		prevPos = route.at(i).PositionRaw;
	}

	// Return
	return statuses;
}