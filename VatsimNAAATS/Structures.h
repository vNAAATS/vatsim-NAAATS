#pragma once
#include <string>
#include <vector>
#include "Constants.h"
#include "Utils.h"

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

// Describes a longitudinal separation situation
struct CSepInfoLong {
	int DistanceAsTime;
	bool DistanceDecreasing;
	CTrackLongStatus Status;
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

	// Convert to vector normal
	CNVector ToNVector() {
		// Lat/lon to radians
		double lat = CUtils::ToRadians(Lat);
		double lon = CUtils::ToRadians(Lon);

		// Trig values
		double sinLat = sin(lat);
		double cosLat = cos(lat);
		double sinLon = sin(lon);
		double cosLon = cos(lon);

		// Return values
		double x = cosLat * cosLon;
		double y = cosLat * sinLon;
		double z = sinLat;

		// Return
		return CNVector(x, y, z);
	}
};

// Describes a 3d vector
struct C3DVector {
	double x;
	double y;
	double z;

	// Constructor
	C3DVector(double vx, double vy, double vz) {
		x = vx;
		y = vy;
		z = vz;
	}

	// Cross product
	C3DVector Cross(C3DVector v) {
		// Assign
		double x = (y * v.z) - (z * v.y);
		double y = (z * v.x) - (x * v.z);
		double z = (x * v.y) - (y * v.x);

		// Return
		return C3DVector(x, y, z);
	}

	// Multiply by scalar
	C3DVector Times(double val) {
		// Return new value
		return C3DVector(x * val, y * val, z * val);
	}

	// Minus by vector
	C3DVector Minus(C3DVector val) {
		// Return new value
		return C3DVector(x - val.x, y - val.y, z - val.z);
	}

	// Length
	double Length() {
		// Get magnitude
		return sqrt((x * x) + (y * y) + (x * z));
	}
};

// Describes an n-vector
struct CNVector {
	double x;
	double y;
	double z;

	// Constructor
	CNVector(double vx, double vy, double vz) {
		x = vx;
		y = vy;
		z = vz;
	}

	// Default constructor
	CNVector() {}

	// Get the vector normal to great circle from heading
	CNVector GreatCircle(double bearing) {
		double theta = CUtils::ToRadians(bearing);
		// 3d vector for this nvector
		C3DVector vec(x, y, z);
		// North pole n vector
		C3DVector vN(0, 0, 1);
		// East
		C3DVector east = vN.Cross(vec);
		// North
		C3DVector north = vec.Cross(east);

		// Calculate return vector
		C3DVector eastVal = east.Times(cos(theta) / east.Length);
		C3DVector northVal = north.Times(sin(theta) / north.Length);
		C3DVector circle = northVal.Minus(eastVal);

		return CNVector(circle.x, circle.y, circle.z);
	}

	// Cross product
	CNVector Cross(CNVector v) {
		// Assign
		double x = (y * v.z) - (z * v.y);
		double y = (z * v.x) - (x * v.z);
		double z = (x * v.y) - (y * v.x);

		// Return
		return CNVector(x, y, z);
	}

	// Dot product
	double Dot(CNVector v) {
		return (x * v.x) + (y * v.y) + (z * v.z);
	}

	// Convert to lat/lon
	CLatLon ToLatLon() {
		double lat = atan2(z, sqrt((x * x) + (y * y)));
		double lon = atan2(y, x);

		return CLatLon(lat, lon);
	}

	// Add the vector
	CNVector Plus(CNVector v) {
		return CNVector(x + v.x, y + v.y, z + v.z);
	}
};

