#include "pch.h"
#include "RoutesHelper.h"
#include "DataHandler.h"

map<string, CTrack> CRoutesHelper::CurrentTracks;

string CRoutesHelper::CurrentTMI = "";

vector<string> CRoutesHelper::ActiveRoutes;

bool CRoutesHelper::GetRoute(CRadarScreen* screen, vector<CRoutePosition>* routeVector, string callsign, CAircraftFlightPlan* copy) {
	// Get the flight plan
	CAircraftFlightPlan* fp = copy != nullptr ? copy : CDataHandler::GetFlightData(callsign);

	// Check validity
	if (!fp->IsValid || fp->Route.size() == 0) {
		return false;
	}

	// Get target
	CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str());

	// Get aircraft direction
	bool direction = CUtils::GetAircraftDirection(target.GetTrackHeading());
	
	// Loop through each route item
	int totalDistance = 0;
	for (int idx = 0; idx < fp->Route.size(); idx++) {
		try {
			// Create position	
			CRoutePosition position;

			// Fix
			position.Fix = fp->Route[idx].Name;
			position.PositionRaw = fp->Route[idx].Position;

			// Get estimate
			if (!direction) { // Westbound
				if (target.GetPosition().GetPosition().m_Longitude > fp->Route[idx].Position.m_Longitude) {
					if (totalDistance == 0) {
						// Calculate distance from aircraft
						totalDistance += target.GetPosition().GetPosition().DistanceTo(fp->Route[idx].Position);
						position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(fp->Route[idx].Position);
					}
					else {
						// Calculate distance point to point
						totalDistance += fp->Route.at(idx - 1).Position.DistanceTo(fp->Route.at(idx).Position);
						position.DistanceFromLastPoint = fp->Route.at(idx - 1).Position.DistanceTo(fp->Route.at(idx).Position);
					}
					position.Estimate = CUtils::ParseZuluTime(false, CUtils::GetTimeDistanceSpeed((int)round(totalDistance), target.GetPosition().GetReportedGS()));
				}
				else {
					position.Estimate = "--";
					position.DistanceFromLastPoint = 0;
				}
			}
			else { // Eastbound
				if (target.GetPosition().GetPosition().m_Longitude > fp->Route[idx].Position.m_Longitude) {
					if (totalDistance == 0) {
						// Calculate distance from aircraft
						totalDistance += target.GetPosition().GetPosition().DistanceTo(fp->Route[idx].Position);
						position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(fp->Route[idx].Position);
					}
					else {
						// Calculate distance point to point
						totalDistance += fp->Route.at(idx - 1).Position.DistanceTo(fp->Route.at(idx).Position);
						position.DistanceFromLastPoint = fp->Route.at(idx - 1).Position.DistanceTo(fp->Route.at(idx).Position);
					}
					position.Estimate = CUtils::ParseZuluTime(false, CUtils::GetTimeDistanceSpeed((int)round(totalDistance), target.GetPosition().GetReportedGS()));
				}
				else {
					position.Estimate = "--";
					position.DistanceFromLastPoint = 0;
				}
			}

			// Altitude
			position.FlightLevel = target.GetPosition().GetFlightLevel() / 100;

			// Add the position
			routeVector->push_back(position);
		}
		catch (std::out_of_range & ex) {
			continue;
		}
	}

	return true;
}

void CRoutesHelper::InitialiseRoute(void* args) {
	// Convert args
	CUtils::CAsyncData* data = (CUtils::CAsyncData*) args;

	// Flight plan
	CAircraftFlightPlan* fp = data->FP != nullptr ? data->FP : CDataHandler::GetFlightData(data->Callsign);

	// Final route vector
	vector<CWaypoint> parsedRoute;

	// Track
	string trackReturned = "";
	
	// First we check if they have a route string
	if (fp != nullptr && !fp->RouteRaw.empty() && fp->RouteRaw.size() > 0) { // Get their route as per the route string
		// We check now if they have a track
		if (fp->Track != "RR") {
			if (fp->Track.size() < 2) {
				for (int i = 0; i < CurrentTracks.find(fp->Track)->second.RouteRaw.size(); i++) {
					CWaypoint point;
					point.Name = CurrentTracks.find(fp->Track)->second.Route[i];
					point.Position = CurrentTracks.find(fp->Track)->second.RouteRaw[i];
					parsedRoute.push_back(point);
				}
			}
			else {
				// Concorde route
				if (trackReturned.size() == 2) {
					if (trackReturned == "SM") {
						parsedRoute = NatSM;
					}
					else if (trackReturned == "SN") {
						parsedRoute = NatSN;
					}
					else if (trackReturned == "SP") {
						parsedRoute = NatSP;
					}
					else if (trackReturned == "SL") {
						parsedRoute = NatSL;
					}
					else {
						parsedRoute = NatSO;
					}
				}
			}
		}
		else { // It's a random routing
			for (int i = 0; i < fp->RouteRaw.size(); i++) {
				// Waypoint obj
				CWaypoint point;

				// We check whether there are any numbers in the item
				bool isAllAlpha = true;
				for (int j = 0; j < fp->RouteRaw[i].size(); j++) {
					if (isdigit(fp->RouteRaw[i].at(j))) {
						isAllAlpha = false;
					}
				}

				// If it's a waypoint we need to search the sector file for the reference
				if (isAllAlpha) {
					// Get sector file
					data->Screen->GetPlugIn()->SelectActiveSectorfile();
					CSectorElement fix; // Fixes element

					// Select waypoint
					try {
						for (fix = data->Screen->GetPlugIn()->SectorFileElementSelectFirst(EuroScopePlugIn::SECTOR_ELEMENT_FIX);
							fix.IsValid();
							fix = data->Screen->GetPlugIn()->SectorFileElementSelectNext(fix, EuroScopePlugIn::SECTOR_ELEMENT_FIX)) {
							string fixName = fix.GetName();
							if (!strcmp(fixName.c_str(), fp->RouteRaw.at(i).c_str())) {
								break;
							}
						}
					}
					catch (std::out_of_range ex) {
						// Leave blank
					}

					// Check here in case not found
					CPosition pos;
					if (fp->RouteRaw[i] == fix.GetName() && fix.GetPosition(&pos, 0)) { // If found then add
						point.Name = fp->RouteRaw[i];
						point.Position = pos;

					}
					else { // If not then set the default (highly unlikely but a fallback just in case)
						pos.m_Latitude = 0.0;
						pos.m_Longitude = 0.0;
						point.Name = fp->RouteRaw[i];
						point.Position = pos;
					}
				}
				else {
					// Make position from the string lat and lon
					CPosition pos;
					pos.m_Latitude = stod(fp->RouteRaw[i].substr(0, 2));
					pos.m_Longitude = -(stod(fp->RouteRaw[i].substr(3, 2)));
					point.Name = fp->RouteRaw[i];
					point.Position = pos;
				}

				// Append the waypoint object
				parsedRoute.push_back(point);
			}
		}
	}
	else { // We get the route as per their VATSIM flight plan if no route string
		// Target, flight plan and route
		CRadarTarget target = data->Screen->GetPlugIn()->RadarTargetSelect(data->Callsign.c_str());
		CFlightPlan fpData = data->Screen->GetPlugIn()->FlightPlanSelect(target.GetCallsign());
		CFlightPlanExtractedRoute route = fpData.GetExtractedRoute();

		// Get NAT track (if they are on it)
		trackReturned = OnNatTrack(data->Screen, data->Callsign.c_str());
		CTrack track;
		if (trackReturned != "") { // If on a track
			// Check if it is concorde first
			if (trackReturned.size() == 1) {	
				bool loopBreak = false;
				for (auto kv : CRoutesHelper::CurrentTracks) {
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
		}

		// If on track
		bool routeFetched = false;
		if (trackReturned != "") {
			// Check concorde
			if (trackReturned.size() == 2) {
				if (trackReturned == "SM") {
					parsedRoute = NatSM;
					routeFetched = true;
				}
				else if (trackReturned == "SN") {
					parsedRoute = NatSN;
					routeFetched = true;
				}
				else if (trackReturned == "SP") {
					parsedRoute = NatSP;
					routeFetched = true;
				}
				else if (trackReturned == "SL") {
					parsedRoute = NatSL;
					routeFetched = true;
				}
				else {
					parsedRoute = NatSO;
					routeFetched = true;
				}
			}
			
			if (!routeFetched) {
				for (int i = 0; i < track.RouteRaw.size(); i++) {
					// Make waypoint
					CWaypoint point;
					point.Name = track.Route[i];
					point.Position = track.RouteRaw[i];

					// Add to route vector
					parsedRoute.push_back(point);
				}
			}			
		} 
		else {
			// Find our entry and exit points
			int entryPoint = -1;
			int exitPoint = -1;
			bool direction = CUtils::GetAircraftDirection(target.GetTrackHeading());
			for (int i = 0; i < route.GetPointsNumber(); i++) {
				if (entryPoint == -1) { // Check entry point
					if (CUtils::IsEntryPoint(string(route.GetPointName(i)), direction ? true : false)) {
						entryPoint = i;
						continue;
					}
				}
				if (exitPoint == -1) { // Exit point
					if (CUtils::IsExitPoint(string(route.GetPointName(i)), direction ? true : false)) {
						exitPoint = i;
						break;
					}
				}
			}

			// Entry and exit points
			int start = entryPoint == -1 ? route.GetPointsCalculatedIndex() : entryPoint;
			int stop = exitPoint == -1 ? route.GetPointsNumber() : exitPoint + 1;

			// First add the aircraft position if entry point is -1
			if (entryPoint == -1) {
				// Waypoint
				CWaypoint point;
				point.Name = "AIRCRAFT";
				point.Position = target.GetPosition().GetPosition();

				// Add to the vector
				parsedRoute.push_back(point);
			}

			// Get entry and exit points
			for (int i = start; i < stop; i++) {
				// First check if position is within reasonable longitudinal and lateral bounds
				if (route.GetPointPosition(i).m_Longitude >= -65 && route.GetPointPosition(i).m_Longitude <= -5
					&& route.GetPointPosition(i).m_Latitude >= 40 && route.GetPointPosition(i).m_Latitude <= 70) {
					// Now make the waypoint
					CWaypoint point;
					point.Name = CUtils::ConvertCoordinateFormat(route.GetPointName(i), 0);
					point.Position = route.GetPointPosition(i);

					// Add waypoint to parsed vector
					parsedRoute.push_back(point);
				}
			}
		}		
	}	

	// Return the vector
	CDataHandler::SetRoute(data->Callsign, &parsedRoute, fp->Track, data->FP != nullptr ? data->FP : nullptr);

	// Cleanup
	delete args;
}

int CRoutesHelper::ParseRoute(string callsign, string rawInput, bool isTrack, CAircraftFlightPlan* copy) {
	// Return vector
	vector<string> route;

	// Track
	string track;

	// Deal with track
	if (isTrack) {
		if (rawInput.size() < 3) {
			if (CRoutesHelper::CurrentTracks.find(rawInput) != CRoutesHelper::CurrentTracks.end()) {
				route = CRoutesHelper::CurrentTracks.at(rawInput).Route;
				track = CRoutesHelper::CurrentTracks.at(rawInput).Identifier;
			}
			else {
				return 1;
			}
		}
		else {
			// Check concorde
			if (rawInput == "SM") {
				for (int i = 0; i < NatSM.size(); i++) {
					route.push_back(NatSM[i].Name);
					track = rawInput;
				}
			}
			else if (rawInput == "SN") {
				for (int i = 0; i < NatSN.size(); i++) {
					track = rawInput;
				}
			}
			else if (rawInput == "SP") {
				for (int i = 0; i < NatSP.size(); i++) {
					track = rawInput;
				}
			}
			else if (rawInput == "SL") {
				for (int i = 0; i < NatSL.size(); i++) {
					track = rawInput;
				}
			}
			else if (rawInput == "SO") {
				for (int i = 0; i < NatSO.size(); i++) {
					track = rawInput;
				}
			}
			else {
				return 1;
			}
		}
	}
	else {
		/// Validation
		// Tokens for split string
		vector<string> tokens;

		// String stream
		stringstream stream(rawInput);

		// Intermediate value
		string intermediate;

		track = "RR";

		// Tokenise the string
		while (getline(stream, intermediate, ' '))
		{
			tokens.push_back(intermediate);
		}

		// Loop the tokens
		for (int i = 0; i < tokens.size(); i++) {
			// Check if digits
			bool isAllAlpha = true;
			for (int j = 0; j < tokens.at(i).size(); j++) {
				if (isdigit(tokens.at(i).at(j))) {
					isAllAlpha = false;
				}
			}

			// If waypoint check the size
			if (isAllAlpha) {
				// Reject if greater or less than 5
				if (tokens.at(i).size() < 5 || tokens.at(i).size() > 5) {
					return 1;
				}
				else {
					// Otherwise make uppercase and push back
					string waypoint;
					for (int j = 0; j < tokens.at(i).size(); j++) {
						waypoint += toupper(tokens.at(i)[j]);
					}
					route.push_back(waypoint);
				}
			}
			else { // It's a coordinate
				if (tokens.at(i).size() < 5 || tokens.at(i).size() > 5) {
					return 1;
				}
				else {
					// Check manually
					if (!isdigit(tokens.at(i)[0]))
						return 1;
					if (!isdigit(tokens.at(i)[1]))
						return 1;
					if (tokens.at(i)[2] != '/')
						return 1;
					if (!isdigit(tokens.at(i)[3]))
						return 1;
					if (!isdigit(tokens.at(i)[4]))
						return 1;

					// We got here so push it
					route.push_back(tokens.at(i));
				}
			}
		}
	}

	// We got here, so set the route and return success code
	if (copy != nullptr) {
		copy->Track = track;
		copy->RouteRaw.clear();
		for (int i = 0; i < route.size(); i++) {
			copy->RouteRaw.push_back(route[i]);
		}
	}
	else {
		CDataHandler::GetFlightData(callsign)->Track = track;
		CDataHandler::GetFlightData(callsign)->RouteRaw.clear();
		for (int i = 0; i < route.size(); i++) {
			CDataHandler::GetFlightData(callsign)->RouteRaw.push_back(route[i]);
		}
	}
	
	return 0;
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
		while (found != string::npos && route.at(found + 5) != 0x20) {
			route = route.substr(found + 5, route.size() - found + 5 - 1);
			found = route.find(string(" NAT"));
		}
		if (route.at(found + 5) == 0x20) { // If it was found (found + 5 was a space character)
			// Get the ID and return
			string trackId;
			trackId.push_back(route.at(found + 4));

			// Check if it exists
			if (CurrentTracks.find(trackId) != CurrentTracks.end())
				return trackId;
			else
				return "";
		}
		else if (route.at(found + 5) == 0x4c ||
			route.at(found + 5) == 0x4d || route.at(found + 5) == 0x4e ||
			route.at(found + 5) == 0x4f || route.at(found + 5) == 0x50) { // A letter matching a Concorde track was found

			// Get the ID and return
			string trackId;
			trackId.push_back(route.at(found + 4) + route.at(found+5));

			// Check if it exists
			if (CurrentTracks.find(trackId) != CurrentTracks.end())
				return trackId;
			else
				return "";
		}
		else {
			return "";
		}
	}
	else { // Not on a NAT
		return "";
	}
}