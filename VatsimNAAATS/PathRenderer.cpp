#include "pch.h"
#include "PathRenderer.h"
#include "Overlays.h"
#include "ConflictDetection.h"

string CPathRenderer::RouteDrawTarget;

pair<bool, vector<CRoutePosition>> CPathRenderer::RouteToDraw;

void CPathRenderer::RenderPath(CDC* dc, Graphics* g, CRadarScreen* screen, CPathType type, vector<CSepStatus>* pivA, vector<CSepStatus>* pivB) {

	// Save context
	int iDC = dc->SaveDC();	

	// Font
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(TargetOrange.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Draw route path if route type
	if (type == CPathType::RTE) {
		// Pen & brush
		CPen pen(PS_SOLID, 1, TargetOrange.ToCOLORREF());
		dc->SelectObject(pen);
		SolidBrush brush(TargetOrange);

		// Get AC position and move to
		dc->MoveTo(screen->ConvertCoordFromPositionToPixel(RouteToDraw.first != true ? RouteToDraw.second[0].PositionRaw : (screen->GetPlugIn()->RadarTargetSelect(RouteDrawTarget.c_str()).GetPosition().GetPosition())));
		if (!RouteToDraw.second.empty()) {// Failsafe
			for (auto i = RouteToDraw.second.begin(); i != RouteToDraw.second.end(); i++) {
				// Get point, text rectangle & define y offset
				string text = i->Fix; // To get TextExtent
				POINT point = screen->ConvertCoordFromPositionToPixel(i->PositionRaw);
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

				// Draw line
				dc->LineTo(point);
			}
		}

		// Cleanup
		DeleteObject(pen);
		DeleteObject(&brush);
	}
	else if (type == CPathType::PIV) {
		// Pens & brush
		CPen pen(PS_SOLID, 1, TargetOrange.ToCOLORREF());
		CPen yellowPen(PS_SOLID, 1, WarningYellow.ToCOLORREF());
		CPen redPen(PS_SOLID, 1, CriticalRed.ToCOLORREF());
		dc->SelectObject(pen);
		SolidBrush brush(TargetOrange);

		// Set extra character spacing (workaround for a bug I need to fix)
		dc->SetTextCharacterExtra(2);

		// Get first AC position and draw
		dc->MoveTo(screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVRoute1.first != true ? CConflictDetection::PIVRoute1.second[0].PositionRaw 
			: (screen->GetPlugIn()->RadarTargetSelect(CConflictDetection::PIVLocations1.at(0).Callsign.c_str()).GetPosition().GetPosition())));
		if (!CConflictDetection::PIVRoute1.second.empty()) {// Failsafe
			for (auto i = CConflictDetection::PIVRoute1.second.begin(); i != CConflictDetection::PIVRoute1.second.end(); i++) {
				// Get point, text rectangle & define y offset
				string text = i->Fix; // To get TextExtent
				POINT point = screen->ConvertCoordFromPositionToPixel(i->PositionRaw);
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
		}

		// Get second AC position and draw
		dc->MoveTo(screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVRoute2.first != true ? CConflictDetection::PIVRoute2.second[0].PositionRaw
			: (screen->GetPlugIn()->RadarTargetSelect(CConflictDetection::PIVLocations2.at(0).Callsign.c_str()).GetPosition().GetPosition())));
		if (!CConflictDetection::PIVRoute2.second.empty()) {// Failsafe
			for (auto i = CConflictDetection::PIVRoute2.second.begin(); i != CConflictDetection::PIVRoute2.second.end(); i++) {
				// Get point, text rectangle & define y offset
				string text = i->Fix; // To get TextExtent
				POINT point = screen->ConvertCoordFromPositionToPixel(i->PositionRaw);
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
		}

		// Draw conflicts
		POINT ac1 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVSeparationStatuses.begin()->AircraftLocations.first);
		POINT ac2 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVSeparationStatuses.begin()->AircraftLocations.second);
		int idx = 0; // Array index
		for (auto i = CConflictDetection::PIVSeparationStatuses.begin(); i != CConflictDetection::PIVSeparationStatuses.end(); i++) {
			// Select pen
			if (i->ConflictStatus == CConflictStatus::OK) {
				dc->SelectObject(pen);
			}
			else if (i->ConflictStatus == CConflictStatus::WARNING) {
				dc->SelectObject(yellowPen);
			}
			else {
				dc->SelectObject(redPen);
			}
			// Draw lines
			dc->MoveTo(ac1);
			dc->LineTo(screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations1.at(idx).Position));
			dc->MoveTo(ac2);
			dc->LineTo(screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations2.at(idx).Position));

			// Reset points
			ac1 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations1.at(idx).Position);
			ac2 = screen->ConvertCoordFromPositionToPixel(CConflictDetection::PIVLocations2.at(idx).Position);

			// Increment
			idx++;
		}

		// Cleanup
		DeleteObject(pen);
		DeleteObject(yellowPen);
		DeleteObject(redPen);
		DeleteObject(&brush);
	}
	else if (type == CPathType::TCKS) { // Draw tracks if the type is tracks
		// Pen
		CPen pen(PS_SOLID, 1, TextWhite.ToCOLORREF());
		dc->SelectObject(pen);

		// Font
		FontSelector::SelectMonoFont(14, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_CENTER);

		// Loop tracks
		for (auto kv : COverlays::CurrentTracks) {
			// Show eastbound/eastbound only if that type is selected
			if (COverlays::CurrentType == COverlayType::TCKS_EAST && kv.second.Direction != CTrackDirection::EAST) {
				continue;
			}
			else if (COverlays::CurrentType == COverlayType::TCKS_WEST && kv.second.Direction != CTrackDirection::WEST) {
				continue;
			}

			// Move to start and draw 
			POINT firstPointCoord = screen->ConvertCoordFromPositionToPixel(kv.second.RouteRaw[0]);
			dc->MoveTo(firstPointCoord);
			string id = kv.first;
			if (kv.second.Direction == CTrackDirection::EAST) {
				dc->TextOutA(firstPointCoord.x - 12, firstPointCoord.y - 5, id.c_str());
			}
			else {
				dc->TextOutA(firstPointCoord.x + 12, firstPointCoord.y - 5, id.c_str());
			}

			// Draw lines
			for (int i = 0; i < kv.second.RouteRaw.size(); i++) {
				dc->LineTo(screen->ConvertCoordFromPositionToPixel(kv.second.RouteRaw[i]));
			}
		}

		// Cleanup
		DeleteObject(pen);
	}

	// Restore context
	dc->RestoreDC(iDC);
}

// TODO: Refactor when implementing flight plan window
pair<bool, vector<CRoutePosition>> CPathRenderer::GetRoute(CRadarScreen* screen, string callsign) {
	// Get target
	CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str());

	// Route and flight plan
	CFlightPlan fpData = screen->GetPlugIn()->FlightPlanSelect(target.GetCallsign());
	CFlightPlanExtractedRoute route = fpData.GetExtractedRoute();

	// Vector to store the final route
	vector<CRoutePosition> returnRoute;

	// Whether the starting point is the aircraft position
	bool startAtAircraft = false;

	// Get NAT track (if they are on it)
	string trackReturned = OnNatTrack(screen, callsign);
	CTrack track;
	if (trackReturned != "") { // If on a track
		bool loopBreak = false;
		for (auto kv : COverlays::CurrentTracks) {
			if (kv.first == trackReturned) { // Assign track to the returned box
				track = kv.second;
				loopBreak = true;
				break;
			}
		}
		if (!loopBreak) { // If for some reason the pilot's track doesn't exist, ignore it
			trackReturned = "";
		}
	}

	// Now get the appropriate route
	if (trackReturned != "") {
		int totalDistance = 0;
		for (int i = 0; i < track.RouteRaw.size(); i++) {
			// Create position
			CRoutePosition position;

			// Fix
			position.Fix = track.Route[i];

			// Position
			position.PositionRaw = track.RouteRaw[i];

			// Get estimate
			if (track.Direction == CTrackDirection::WEST) { // If heading westbound, check if the aircraft is past the point
				if (target.GetPosition().GetPosition().m_Longitude > track.RouteRaw[i].m_Longitude) {
					if (totalDistance == 0) {
						// Calculate distance from aircraft
						totalDistance += target.GetPosition().GetPosition().DistanceTo(track.RouteRaw[i]);
						position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(track.RouteRaw[i]);
					}
					else {
						// Calculate distance point to point
						totalDistance += track.RouteRaw[i - 1].DistanceTo(track.RouteRaw[i]);
						position.DistanceFromLastPoint = track.RouteRaw[i - 1].DistanceTo(track.RouteRaw[i]);
					}
					position.Estimate = CUtils::ParseZuluTime(false, CUtils::GetTimeDistanceSpeed((int)round(totalDistance), target.GetGS()));
				}
				else {
					position.Estimate = "--";
					position.DistanceFromLastPoint = 0;
				}
			} else if (track.Direction == CTrackDirection::EAST) { // If heading westbound, check if the aircraft is past the point
				if (target.GetPosition().GetPosition().m_Longitude < track.RouteRaw[i].m_Longitude) {
					if (totalDistance == 0) {
						// Calculate distance from aircraft
						totalDistance += target.GetPosition().GetPosition().DistanceTo(track.RouteRaw[i]);
						position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(track.RouteRaw[i]);
					}
					else {
						// Calculate distance point to point
						totalDistance += track.RouteRaw[i - 1].DistanceTo(track.RouteRaw[i]);
						position.DistanceFromLastPoint = track.RouteRaw[i - 1].DistanceTo(track.RouteRaw[i]);
					}
					position.Estimate = CUtils::ParseZuluTime(false, CUtils::GetTimeDistanceSpeed((int)round(totalDistance), target.GetGS()));
				}
				else {
					position.Estimate = "--";
					position.DistanceFromLastPoint = 0;
				}
			}

			// Altitude
			position.FlightLevel = fpData.GetClearedAltitude() != 0 ? fpData.GetClearedAltitude() / 100 : fpData.GetFinalAltitude() / 100;

			// Add the position
			returnRoute.push_back(position);
		}
	}
	else {
		// Instantiate draw start variable (-1 for aircraft and index for actual point) and get aircraft direction
		int point = -1;
		bool direction = CUtils::GetAircraftDirection(target.GetTrackHeading());
		// Get entry point
		for (int i = 0; i < route.GetPointsNumber(); i++) {
			if (point == -1) {
				if (CUtils::IsEntryExitPoint(string(route.GetPointName(i)), direction ? true : false)) {
					point = i;
					break;
				}
			}
		}

		// Start at aircraft if position is -1
		if (point == -1) startAtAircraft = true;

		// Get the points in the route
		int counter = 0; // Counter flag
		for (int i = point == -1 ? route.GetPointsCalculatedIndex() : point; i < route.GetPointsNumber(); i++) {
			
			// Store whether to cancel route draw
			bool breakLoop = false;
			if (CUtils::IsEntryExitPoint(string(route.GetPointName(i)), direction ? false : true)) {
				breakLoop = true;
			}

			// Create position	
			CRoutePosition position;
			position.Fix = route.GetPointName(i);
			position.PositionRaw = route.GetPointPosition(i);
			position.Estimate = fpData.GetExtractedRoute().GetPointDistanceInMinutes(i) != -1 ? CUtils::ParseZuluTime(false, -1, &fpData, i) : "--";
			position.FlightLevel = route.GetPointCalculatedProfileAltitude(i) / 100;

			// Get the distance from the last point
			if (counter > 0) {
				position.DistanceFromLastPoint = route.GetPointPosition(i - 1).DistanceTo(route.GetPointPosition(i));
			}
			else { // Last point is the aircraft
				position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(route.GetPointPosition(i));
			}

			// Check if it is a passed point
			if (position.Estimate == "--") {
				position.DistanceFromLastPoint = 0;
			}
			else if (counter > 0 && returnRoute.back().Estimate == "--") {
				position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(route.GetPointPosition(i));
			}

			// Check if position is within the longitudinal bounds
			if (position.PositionRaw.m_Longitude >= -65 && position.PositionRaw.m_Longitude <= -5) {
				// Add the position
				returnRoute.push_back(position);
			}

			// Increment counter
			counter++;

			// Break the loop if end of route
			if (breakLoop) break;
		}
	}
	
	// Set route draw
	return make_pair(startAtAircraft, returnRoute);
}

string CPathRenderer::OnNatTrack(CRadarScreen* screen, string callsign) {
	// Get flight plan
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(callsign.c_str());

	// Get route and begin search
	string route = fp.GetFlightPlanData().GetRoute();
	size_t found = route.find(string(" NAT"));
	// If found
	if (found != string::npos) {
		// Make sure that it's not a waypoint starting with NAT (check character count)
		if (route.at(found + 5) == 0x20) { // If it was found (found + 5 was a space character)
			// Get the ID and return
			string trackId;
			trackId.push_back(route.at(found + 4));
			return trackId;
		}
		else {
			return "";
		}
	}
	else { // Not on a NAT
		return "";
	}
}

int CPathRenderer::ClearCurrentRoute() {
	// Test if route is already empty
	if (!RouteToDraw.second.empty()) {
		// Clear it and return success code
		RouteToDraw.first = false;
		RouteToDraw.second.clear();
		RouteDrawTarget = "";
		return 0;
	}
	else {
		// Vector was already empty so return fail code
		return 1;
	}
}