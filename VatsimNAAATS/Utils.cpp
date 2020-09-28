#define _USE_MATH_DEFINES
#include "pch.h"
#include<cmath>
#include "Utils.h"
#include "Constants.h"
#include "RadarDisplay.h"

// Default values
int CUtils::InboundX = 500;
int CUtils::InboundY = 150;
int CUtils::OthersX = 200;
int CUtils::OthersY = 150;
int CUtils::TrackWindowX = 300;
int CUtils::TrackWindowY = 200;
int CUtils::AltFiltLow = 0;
int CUtils::AltFiltHigh = 700;
bool CUtils::GridEnabled = false;
bool CUtils::TagsEnabled = true;
bool CUtils::QckLookEnabled = false;
bool CUtils::OverlayEnabled = false;
int CUtils::AreaSelection = 802;
int CUtils::SelectedOverlay = 800;
int CUtils::PosType = 802;
int CUtils::SepMinimaVertical = 1000;
int CUtils::SepMinimaLateral = 60;
int CUtils::SepMinimaLongitudinal = 10;

// Save plugin data
void CUtils::SavePluginData(CRadarScreen* screen) {

	// Inbound list
	screen->SaveDataToAsr(SET_INBNDX.c_str(), "X position of Inbound list.", to_string(InboundX).c_str());
	screen->SaveDataToAsr(SET_INBNDY.c_str(), "Y position of Inbound list.", to_string(InboundY).c_str());

	// Others list
	screen->SaveDataToAsr(SET_OTHERSX.c_str(), "X position of Other list.", to_string(OthersX).c_str());
	screen->SaveDataToAsr(SET_OTHERSY.c_str(), "Y position of Other list.", to_string(OthersY).c_str());

	// Altitude filter (TODO)

	// Misc display settings
	screen->SaveDataToAsr(SET_GRID.c_str(), "Grid enabled/disabled.", GridEnabled ? "true" : "false");
	screen->SaveDataToAsr(SET_TAGS.c_str(), "Tags enabled/disabled.", TagsEnabled ? "true" : "false");
	screen->SaveDataToAsr(SET_OVERLAY.c_str(), "Overlay enabled/disabled.", OverlayEnabled ? "true" : "false");
	screen->SaveDataToAsr(SET_QCKLOOK.c_str(), "Quick Look enabled/disabled.", QckLookEnabled ? "true" : "false");

	// Dropdown values
	screen->SaveDataToAsr(SET_AREASEL.c_str(), "Selected area ownership.", to_string(AreaSelection).c_str());
	screen->SaveDataToAsr(SET_OVERLAYSEL.c_str(), "Selected overlay.", to_string(SelectedOverlay).c_str());
	screen->SaveDataToAsr(SET_POSTYPESEL.c_str(), "Selected position type.", to_string(PosType).c_str());
}

// Load plugin data
void CUtils::LoadPluginData(CRadarScreen* screen) {
	// Strings to parse data
	const char* stra;
	const char* strb;

	// Inbound list
	stra = screen->GetDataFromAsr(SET_INBNDX.c_str());
	strb = screen->GetDataFromAsr(SET_INBNDY.c_str());
	if (stra != NULL && strb != NULL) {
		InboundX = stoi(stra);
		InboundY = stoi(strb);
	}

	// Others list
	stra = screen->GetDataFromAsr(SET_OTHERSX.c_str());
	strb = screen->GetDataFromAsr(SET_OTHERSY.c_str());
	if (stra != NULL && strb != NULL) {
		OthersX = stoi(stra);
		OthersY = stoi(strb);
	}

	// Grid enabled
	stra = screen->GetDataFromAsr(SET_GRID.c_str());
	if (stra != NULL) {
		GridEnabled = (stra[0] == 't');
	}
	
	// Tags enabled
	stra = screen->GetDataFromAsr(SET_TAGS.c_str());
	if (stra != NULL) {
		TagsEnabled = (stra[0] == 't');
	}

	// Quick look enabled
	stra = screen->GetDataFromAsr(SET_QCKLOOK.c_str());
	if (stra != NULL) {
		QckLookEnabled = (stra[0] == 't');
	}

	// Overlay enabled
	stra = screen->GetDataFromAsr(SET_OVERLAY.c_str());
	if (stra != NULL) {
		OverlayEnabled = (stra[0] == 't');
	}

	// Area selection
	stra = screen->GetDataFromAsr(SET_AREASEL.c_str());
	if (stra != NULL) {
		AreaSelection = stoi(stra);
	}

	// Overlay selection
	stra = screen->GetDataFromAsr(SET_OVERLAYSEL.c_str());
	if (stra != NULL) {
		SelectedOverlay = stoi(stra);
	}

	// Position type selection
	stra = screen->GetDataFromAsr(SET_POSTYPESEL.c_str());
	if (stra != NULL) {
		PosType = stoi(stra);
	}

	// Show a user message saying that the plugin was loaded successfully
	screen->GetPlugIn()->DisplayUserMessage("Message", "vNAAATS Plugin", string("version " + PLUGIN_VERSION + " initialised.").c_str(), false, false, false, false, false);
}

bool CUtils::GetAircraftDirection(int heading) {
	if ((heading <= 359) && (heading >= 181)) {
		return false; // Westbound
	}
	else if ((heading >= 1) && (heading <= 179)) {

		return true; // Eastbound
	}
}

bool CUtils::IsEntryExitPoint(string pointName, bool side) {
	if (side) { // Gander
		if (find(pointsGander.begin(), pointsGander.end(), pointName) != pointsGander.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
	else { // Shanwick
		if (find(pointsShanwick.begin(), pointsShanwick.end(), pointName) != pointsShanwick.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
}

CPosition CUtils::PositionFromLatLon(double lat, double lon) {
	// Latitude
	int degrees = (int)floor(lat);
	double minutes = (lat - (double)degrees) * 60;
	double seconds = (minutes - floor(minutes)) * 60;
	degrees = abs(degrees); // Get absolute value
	string degreesFormatted;
	string minutesFormatted;
	string secondsFormatted;
	if (degrees < 10) { // Format degrees
		degreesFormatted = "N" + to_string(0) + to_string(0) + to_string(degrees);
	}
	else if (degrees < 100) {
		degreesFormatted = "N" + to_string(0) + to_string(degrees);
	}
	else {
		degreesFormatted = "N" + to_string(degrees);
	}
	if (minutes < 10) { // Format minutes
		minutesFormatted = to_string(0) + to_string((int)minutes);
	}
	else if (minutes == 0) {
		minutesFormatted = to_string(0) + to_string(0);
	}
	else {
		minutesFormatted = to_string((int)minutes);
	}
	if (seconds < 10 && seconds > 0) { // Format seconds
		secondsFormatted = to_string(0) + to_string(seconds);
	}
	else if (minutes == 0) {
		secondsFormatted = to_string(0) + to_string(0);
	}
	else {
		secondsFormatted = to_string(seconds);
	}
	string latitude = degreesFormatted + "." + minutesFormatted + "." + secondsFormatted;
	// Longitude
	degrees = (int)ceil(lon);
	minutes = (lon - (double)degrees) * 60;
	seconds = (minutes - floor(minutes)) * 60;
	degrees = abs(degrees); // Get absolute value
	if (degrees < 10) { // Format degrees
		degreesFormatted = "W" + to_string(0) + to_string(0) + to_string(abs(degrees));
	}
	else if (degrees < 100) {
		degreesFormatted = "W" + to_string(0) + to_string(abs(degrees));
	}
	else {
		degreesFormatted = "W" + to_string(abs(degrees));
	}
	if (minutes < 10) { // Format minutes
		minutesFormatted = to_string(0) + to_string((int)minutes);
	}
	else if (minutes == 0) {
		minutesFormatted = to_string(0) + to_string(0);
	}
	else {
		minutesFormatted = to_string((int)minutes);
	}
	if (seconds < 10 && seconds > 0) { // Format seconds
		secondsFormatted = to_string(0) + to_string(seconds);
	}
	else if (seconds == 0) {
		secondsFormatted = to_string(0) + to_string(0);
	}
	else {
		secondsFormatted = to_string(seconds);
	}
	string longitude = degreesFormatted + "." + minutesFormatted + "." + secondsFormatted;

	// Return
	CPosition pos;
	pos.LoadFromStrings(longitude.c_str(), latitude.c_str());
	return pos;
}

int CUtils::GetMach(int groundSpeed, int speedSound) {
	double result = ((double)groundSpeed / (double)speedSound) * 100.0;
	return (int)result;
}

string CUtils::ParseZuluTime(bool delimit, int deltaTime, CFlightPlan* fp, int fix) {
	time_t now = time(0);
	tm* zuluTime = gmtime(&now);
	int deltaMinutes = 0;
	if (deltaTime != -1) {
		deltaMinutes = deltaTime;
	}
	if (fix != -1) {
		deltaMinutes = fp->GetExtractedRoute().GetPointDistanceInMinutes(fix);
	}
	int hours = zuluTime->tm_hour;
	int minutes = zuluTime->tm_min + deltaMinutes;

	if (minutes >= 60) {
		// Get minutes
		int minRemainder = minutes % 60;

		// Get number of hours
		hours += (minutes - minRemainder) / 60;

		// Reassign number of minutes
		minutes = minRemainder;
	}

	// Check if over 24 hours
	if (hours >= 24) {
		hours = hours - 24;
	}


	// Pad for zeros
	string strHours;
	if (hours < 10) {
		if (hours == 0) {
			strHours = "00";
		}
		else {
			strHours = "0" + to_string(hours);
		}
	}
	else {
		strHours = to_string(hours);
	}

	string strMinutes;
	if (minutes < 10) {
		if (minutes == 0) {
			strMinutes = "00";
		}
		else {
			strMinutes = "0" + to_string(minutes);
		}
	}
	else {
		strMinutes = to_string(minutes);
	}

	if (delimit) {
		return strHours + ":" + strMinutes;
	}
	else {
		return strHours + strMinutes;
	}
}

int CUtils::GetDistanceBetweenPoints(POINT p1, POINT p2) {
	// Get distance
	return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

POINT CUtils::GetMidPoint(POINT p1, POINT p2) {
	// Return midpoint
	return { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2 };
}

int CUtils::GetTimeDistanceSpeed(int distanceNM, int speedGS) {
	// Get time in minutes
	int temp = ((float)distanceNM / (float)speedGS) * 60;
	return ((float)distanceNM / (float)speedGS) * 60;
}

int CUtils::GetDistanceSpeedTime(int speedGS, int timeMin) {
	// Get distance in NM
	float mins = ((float)timeMin / 60.0);
	float gs = (float)speedGS;
	return (float)speedGS * ((float)timeMin / 60.0);
}

double CUtils::ToRadians(double degrees) {
	return (M_PI / 180) * degrees;
}

double CUtils::ToDegrees(double radians) {
	return (180 / M_PI) * radians;
}

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

CPosition CUtils::GetPointDistanceBearing(CPosition position, int distanceNM, int heading) {
	double bearing = CUtils::ToRadians((float)heading);

	// Convert coordinates to radians
	double lat = CUtils::ToRadians(position.m_Latitude);
	double lon = CUtils::ToRadians(position.m_Longitude);

	// Get distance to radius in NM
	double distanceToRadius = (float)distanceNM / (float)RADIUS_EARTH_NM;

	// Calculate lat
	double newLat = asin(sin(lat) * cos(distanceToRadius)
		+ cos(lat) * sin(distanceToRadius) * cos(bearing));

	// Calculate lon
	double newLon = lon + atan2(
		sin(bearing) * sin(distanceToRadius) * cos(lat),
		cos(distanceToRadius) - sin(lat) * sin(newLat));

	/// Convert values to sector file format
	newLat = CUtils::ToDegrees(newLat);
	newLon = CUtils::ToDegrees(newLon);
	
	// Return
	return PositionFromLatLon(newLat, newLon);
}

// Get the intersection between two vectors from points and bearings
CLatLon CUtils::GetIntersectionFromPointBearing(CLatLon position1, CLatLon position2, double bearing1, double bearing2) {
	// Get points
	CNVector pos1 = position1.ToNVector();
	CNVector pos2 = position2.ToNVector();

	// Great circles
	CNVector circle1 = pos1.GreatCircle(bearing1);
	CNVector circle2 = pos2.GreatCircle(bearing2);

	// Two candidate positions to choose
	CNVector int1 = circle1.Cross(circle2);
	CNVector int2 = circle2.Cross(circle1);

	// Pick position
	CNVector intersection;

	// Get positive or negative signs for both possible directions
	int direction1 = sign(circle1.Cross(pos1).Dot(int1));
	int direction2 = sign(circle2.Cross(pos2).Dot(int1));

	// Pick the intersection
	switch (direction1 + direction2) {
		case 2: // Both are positive, so both point to the first intersection
			intersection = int1;
			break;
		case -2: // Both are negative, so both point to the second intersection
			intersection = int2;
			break;
		case 0: // The directions are opposite, so the intersection is the point further away (need to check if this is always true)
			intersection = pos1.Plus(pos2).Dot(int1) > 0 ? int2 : int1;
	}

	// Return the latitude and longitude
	return CNVector(intersection.x, intersection.y, intersection.z).ToLatLon();
}

