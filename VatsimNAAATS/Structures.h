#pragma once
#include <cmath>
#include <string>
#include <vector>
#include "Constants.h"

// Describes a NAT track
struct CTrack {
	string Identifier;
	string TMI;
	vector<string> Route;
	vector<CPosition> RouteRaw;
	vector<int> FlightLevels;
	CTrackDirection Direction;
};

// Describes a point along an aircraft's route
struct CRoutePosition {
	string Fix;
	CPosition PositionRaw;
	string Estimate;
	int FlightLevel;
};

// Aircraft displayed on list (inbound at the moment)
struct CListAircraft {
	CListAircraft(string cs, int fA, int cL, string pt, string est, string dest, bool direction) {
		Callsign = cs;
		FinalAltitude = fA;
		ClearedLevel = cL;
		Point = pt;
		Estimate = est;
		Destination = dest;
		Direction = direction;
	}
	string Callsign;
	int FinalAltitude;
	int ClearedLevel;
	string Point;
	string Estimate;
	string Destination;
	bool Direction;
};

// General aircraft status
struct CAircraftStatus {
	CAircraftStatus() {}; // Default constructor
	CAircraftStatus(string cs, int alt, int gs, int hdg, CPosition pos) {
		Callsign = cs;
		Altitude = alt;
		GroundSpeed = gs;
		Heading = hdg;
		Position = pos;
	}
	string Callsign;
	int Altitude;
	int GroundSpeed;
	int Heading;
	CPosition Position;
};

// Describes a separation status
struct CSepStatus {
	int DistanceAsTime;
	int AltDifference;
	bool DistanceDecreasing;
	CTrackStatus TrackStatus;
};

// Describes a lat/lon
struct CLatLon {
	double Lat;
	double Lon;

	// Constructor
	CLatLon(double lat, double lon) {
		Lat = lat;
		Lon = lon;
	}

	// Convert degrees to radians (can't use Utils.h because circular dependency)
	double ToRadians(double degrees) {
		return (M_PI / 180) * degrees;
	}
};

