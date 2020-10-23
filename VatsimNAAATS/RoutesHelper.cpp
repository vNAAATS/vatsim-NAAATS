#include "pch.h"
#include "RoutesHelper.h"
#include "DataHandler.h"

// TODO: Refactor when implementing flight plan window
vector<CRoutePosition> CRoutesHelper::GetRoute(CRadarScreen* screen, string callsign) {
	// Get the flight plan
	CAircraftFlightPlan* fp = CDataHandler::GetFlightData(callsign);

	// Flag for route not found
	bool noRouteFound = false;

	// Vector to store the final route
	vector<CRoutePosition> returnRoute;

	// Whether the starting point is the aircraft position
	bool startAtAircraft = false;

	// Flags
	CTrack track;
	vector<string> fetchedRoute;
	bool onTrack = false;

	// If they are *not* on a random route
	if (fp->Track != "RR") { 
		
	}
	else { // Random routing, so we set our flag
		fetchedRoute = fp->Route;
	}
}

vector<string> CRoutesHelper::InitialiseRoute(CRadarScreen* screen, string callsign) {
	// Flags
	CTrack track;
	bool onTrack = false;

	// Return vector
	vector<string> returnRoute;

	// Get the track (empty string for no track)
	string trackReturned = OnNatTrack(screen, callsign); 
	if (trackReturned != "") { // If on a track
		bool loopBreak = false;
		for (auto kv : CurrentTracks) {
			if (kv.first == trackReturned) { // Assign track to the returned box
				return kv.second.Route;
			}
		}
		// If for some reason the pilot's track doesn't exist, ignore it
		trackReturned = "";
	}
	if (trackReturned == "") { // Either random routing or no existing track, so we need to get the points manually
		// Route and flight plan
		CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str());
		CFlightPlan fpData = screen->GetPlugIn()->FlightPlanSelect(target.GetCallsign());
		CFlightPlanExtractedRoute route = fpData.GetExtractedRoute();

		// Direction
		bool direction = CUtils::GetAircraftDirection(target.GetTrackHeading());

		// Find entry and exit points
		int entryPoint = -1;
		int exitPoint = -1;
		for (int i = 0; i < route.GetPointsNumber(); i++) {
			if (CUtils::IsEntryPoint(route.GetPointName(i), direction ? true : false)) {
				entryPoint = i;
			}
			else if (CUtils::IsExitPoint(route.GetPointName(i), direction ? true : false)) {
				entryPoint = i;
				break;
			}
		}

		// If the entry point is -1 then we signify the current aircraft position is where to draw from
		if (entryPoint == -1) {
			// Create position	
			string position = "";

			// Add to the vector
			returnRoute.push_back(position);
		}

		// Add the rest of the route
		int counter = 0; // Counter flag
		for (int i = entryPoint == -1 ? route.GetPointsCalculatedIndex() : entryPoint; i < exitPoint == -1 ? route.GetPointsNumber() : exitPoint + 1; i++) {
			// Create position	
			string position;
			position = route.GetPointName(i);

			// Check if position is within the longitudinal bounds
			if (route.GetPointPosition(i).m_Longitude >= -65 && route.GetPointPosition(i).m_Longitude <= -5) {
				// Add the position
				returnRoute.push_back(position);
			}
		}

		// This is their route, we return it
		return returnRoute;
	}
}

string CRoutesHelper::OnNatTrack(CRadarScreen* screen, string callsign) {
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
		else if (route.at(found + 6) == 0x4c ||
			route.at(found + 6) == 0x4d || route.at(found + 6) == 0x4e ||
			route.at(found + 6) == 0x4f || route.at(found + 6) == 0x50) { // A letter matching a Concorde track was found

			// Get the ID and return
			string trackId;
			trackId.push_back(route.at(found + 5));
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