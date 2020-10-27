#include "pch.h"
#include "RoutesHelper.h"
#include "DataHandler.h"

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

vector<CWaypoint> CRoutesHelper::InitialiseRoute(CRadarScreen* screen, string callsign) {
	
	
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