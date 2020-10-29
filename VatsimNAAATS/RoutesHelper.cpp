#include "pch.h"
#include "RoutesHelper.h"
#include "DataHandler.h"

map<string, CTrack> CRoutesHelper::CurrentTracks;

string CRoutesHelper::CurrentTMI = "";

vector<CRoutePosition> CRoutesHelper::GetRoute(CRadarScreen* screen, string callsign) {
	// Get the flight plan
	CAircraftFlightPlan* fp = CDataHandler::GetFlightData(callsign);

	// Get target
	CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(callsign.c_str());

	// The fetched route
	vector<CWaypoint>* fetchedRoute = &fp->Route;

	// Vector to store the final route
	vector<CRoutePosition> returnRoute;
	
	// Get aircraft direction
	bool direction = CUtils::GetAircraftDirection(target.GetTrackHeading());
	
	// Loop through each route item
	int totalDistance = 0;
	int i = 0;
	for (auto idx = fetchedRoute->begin(); idx != fetchedRoute->end(); idx++) {
		// Create position	
		CRoutePosition position;

		// Fix
		position.Fix = idx->Name;		
		position.PositionRaw = idx->Position;
		
		// Get estimate
		if (!direction) { // Westbound
			if (target.GetPosition().GetPosition().m_Longitude > idx->Position.m_Longitude) {
				if (totalDistance == 0) {
					// Calculate distance from aircraft
					totalDistance += target.GetPosition().GetPosition().DistanceTo(idx->Position);
					position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(idx->Position);
				}
				else {
					// Calculate distance point to point
					totalDistance += fetchedRoute->at(i - 1).Position.DistanceTo(fetchedRoute->at(i).Position);
					position.DistanceFromLastPoint = fetchedRoute->at(i - 1).Position.DistanceTo(fetchedRoute->at(i).Position);
				}
				position.Estimate = CUtils::ParseZuluTime(false, CUtils::GetTimeDistanceSpeed((int)round(totalDistance), target.GetGS()));
			}
			else {
				position.Estimate = "--";
				position.DistanceFromLastPoint = 0;
			}
		} 
		else { // Eastbound
			if (target.GetPosition().GetPosition().m_Longitude < idx->Position.m_Longitude) {
				if (totalDistance == 0) {
					// Calculate distance from aircraft
					totalDistance += target.GetPosition().GetPosition().DistanceTo(idx->Position);
					position.DistanceFromLastPoint = target.GetPosition().GetPosition().DistanceTo(idx->Position);
				}
				else {
					// Calculate distance point to point
					totalDistance += fetchedRoute->at(i - 1).Position.DistanceTo(fetchedRoute->at(i).Position);
					position.DistanceFromLastPoint = fetchedRoute->at(i - 1).Position.DistanceTo(fetchedRoute->at(i).Position);
				}
				position.Estimate = CUtils::ParseZuluTime(false, CUtils::GetTimeDistanceSpeed((int)round(totalDistance), target.GetGS()));
			}
			else {
				position.Estimate = "--";
				position.DistanceFromLastPoint = 0;
			}
		}

		// Altitude
		position.FlightLevel = stoi(fp->FlightLevel);

		// Add the position
		returnRoute.push_back(position);

		i++;
	}

	return returnRoute;
}

void CRoutesHelper::InitialiseRoute(void* args) {
	// Convert args
	CUtils::CAsyncData* data = (CUtils::CAsyncData*) args;
	// Flight plan
	CAircraftFlightPlan* fp = CDataHandler::GetFlightData(data->Callsign);

	// Final route vector
	vector<CWaypoint> parsedRoute;
	
	// First we check if they have a route string
	if (fp != nullptr && !fp->RouteRaw.empty() && fp->RouteRaw.size() > 0) { // Get their route as per the route string
		for (auto i = fp->RouteRaw.begin(); i != fp->RouteRaw.end();  i++) {
			// Waypoint obj
			CWaypoint point;

			// We check whether there are any numbers in the item
			bool isAllAlpha = true;
			for (int j = 0; j < i->size(); j++) {
				if (isdigit(i->at(j))) {
					isAllAlpha = false;
				}
			}

			// If it's a waypoint we need to search the sector file for the reference
			if (isAllAlpha) {
				// Get sector file
				data->Screen->GetPlugIn()->SelectScreenSectorfile(data->Screen);
				CSectorElement fix(data->Screen->GetPlugIn()->SectorFileElementSelectFirst(5)); // Fixes element

				// Select waypoint
				bool found = false;
				while (string(fix.GetName()) != *i) {
					fix = data->Screen->GetPlugIn()->SectorFileElementSelectNext(fix, 5);
				}

				// Check here in case not found
				CPosition pos;
				if (*i == fix.GetName() && fix.GetPosition(&pos, 0)) { // If found then add
					point.Name = *i;
					point.Position = pos;
					
				}
				else { // If not then set the default (highly unlikely but a fallback just in case)
					pos.m_Latitude = 0.0;
					pos.m_Longitude = 0.0;
					point.Name = *i;
					point.Position = pos;
				}
			}
			else {
				// Make position from the string lat and lon
				CPosition pos;
				pos.m_Latitude = stod(i->substr(0, 2));
				pos.m_Latitude = stod(i->substr(2, 2));
				point.Name = *i;
				point.Position = pos;
			}

			// Append the waypoint object
			parsedRoute.push_back(point);
		}
	}
	else { // We get the route as per their VATSIM flight plan if no route string
		// Target, flight plan and route
		CRadarTarget target = data->Screen->GetPlugIn()->RadarTargetSelect(data->Callsign.c_str());
		CFlightPlan fpData = data->Screen->GetPlugIn()->FlightPlanSelect(target.GetCallsign());
		CFlightPlanExtractedRoute route = fpData.GetExtractedRoute();

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

		// Get the points in the route between entry and exit points
		int start = entryPoint == -1 ? route.GetPointsCalculatedIndex() : entryPoint;
		int stop  = exitPoint == -1 ? route.GetPointsNumber() : exitPoint + 1;
		for (int i = start; i < stop; i++) {
			// First check if position is within reasonable longitudinal bounds
			if (route.GetPointPosition(i).m_Longitude <= -65 && route.GetPointPosition(i).m_Longitude >= -5) {
				// Continue
				continue;
			}

			// Now make the waypoint
			CWaypoint point;
			point.Name = route.GetPointName(i);
			point.Position = route.GetPointPosition(i);

			// Add waypoint to parsed vector
			parsedRoute.push_back(point);
		}
	}	

	// Return the vector
	CDataHandler::SetRoute(data->Callsign, &parsedRoute);

	// Cleanup
	delete args;
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
		else if (route.at(found + 5) == 0x4c ||
			route.at(found + 5) == 0x4d || route.at(found + 5) == 0x4e ||
			route.at(found + 5) == 0x4f || route.at(found + 5) == 0x50) { // A letter matching a Concorde track was found

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