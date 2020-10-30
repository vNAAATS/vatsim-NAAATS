#define _USE_MATH_DEFINES
#include "pch.h"
#include<cmath>
#include "Utils.h"
#include "Constants.h"
#include "RadarDisplay.h"

// Default values
int CUtils::InboundX = 1400;
int CUtils::InboundY = 170;
int CUtils::OthersX = 1200;
int CUtils::OthersY = 150;
int CUtils::ConflictX = 60;
int CUtils::ConflictY = 120;
int CUtils::RCLX = 600;
int CUtils::RCLY = 150;
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

// Save plugin data
void CUtils::SavePluginData(CRadarScreen* screen) {

	// Inbound list
	screen->SaveDataToAsr(SET_INBNDX.c_str(), "X position of Inbound list.", to_string(InboundX).c_str());
	screen->SaveDataToAsr(SET_INBNDY.c_str(), "Y position of Inbound list.", to_string(InboundY).c_str());

	// Others list
	screen->SaveDataToAsr(SET_OTHERSX.c_str(), "X position of Other list.", to_string(OthersX).c_str());
	screen->SaveDataToAsr(SET_OTHERSY.c_str(), "Y position of Other list.", to_string(OthersY).c_str());

	// Altitude filter (TODO)
	screen->SaveDataToAsr(SET_ALTFILT_LOW.c_str(), "Lower level for altitude filter.", to_string(AltFiltLow).c_str());
	screen->SaveDataToAsr(SET_ALTFILT_HIGH.c_str(), "Upper level for altitude filter.", to_string(AltFiltHigh).c_str());

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

	// Others list
	stra = screen->GetDataFromAsr(SET_ALTFILT_LOW.c_str());
	strb = screen->GetDataFromAsr(SET_ALTFILT_HIGH.c_str());
	if (stra != NULL && strb != NULL) {
		AltFiltLow = stoi(stra);
		AltFiltHigh = stoi(strb);
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

// Returns the requested format, or returns the same string if the format was unchanged
string CUtils::ConvertCoordinateFormat(string coordinateString, int format) { // format = 0 (slash format), 1 (xxxxN), 2 (xxNxxxW)
	// Return var
	string returnFormat;
	try {
		// First we make sure there are numbers
		int isAllAlpha = true;
		for (int j = 0; j < coordinateString.size(); j++) {
			if (isdigit(coordinateString.at(j))) {
				isAllAlpha = false;
			}
		}
		// Check the current format of the input string
		int currentFormat = -1;
		if (coordinateString.find('/') != string::npos) {
			currentFormat = 0;
		}
		else if (coordinateString.find('W') == string::npos && coordinateString.find('/') == string::npos && coordinateString.size() == 5) {
			currentFormat = 1;
		}
		else if (coordinateString.find('W') != string::npos) {
			currentFormat = 2;
		}

		// Check the current format, if -1 or matches, just return the input string
		if (currentFormat == -1 || currentFormat == format || isAllAlpha || coordinateString.size() > 7) {
			return coordinateString;
		}

		// Otherwise, change the format
		if (format == 0) {
			if (currentFormat == 1) {
				returnFormat = coordinateString.substr(0, 2) + "/" + coordinateString.substr(2, 2);

			}
			else if (currentFormat == 2) {
				returnFormat = coordinateString.substr(0, 2) + "/" + coordinateString.substr(4, 2);
			}
		}
		else if (format == 1) {
			if (currentFormat == 0) {
				returnFormat = coordinateString.substr(0, 2) + coordinateString.substr(3, 2) + "N";
			}
			else if (currentFormat == 2) {
				returnFormat = coordinateString.substr(0, 2) + coordinateString.substr(4, 2) + "N";
			}
		}
		else {
			if (currentFormat == 0) {
				returnFormat = coordinateString.substr(0, 2) + "N0" + coordinateString.substr(3, 2) + "W";
			}
			else if (currentFormat == 1) {
				returnFormat = coordinateString.substr(0, 2) + "N0" + coordinateString.substr(2, 2) + "W";
			}
		}
	}
	catch (exception & ex) {
		// Return the old one if an exception occurs
		return coordinateString;
	}
	
	// Return the string
	return returnFormat;
}


bool CUtils::GetAircraftDirection(int heading) {
	if ((heading <= 359) && (heading >= 180)) {
		return false; // Westbound
	}
	else if ((heading >= 0) && (heading <= 179)) {

		return true; // Eastbound
	}
}

bool CUtils::IsEntryPoint(string pointName, bool side) {
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

bool CUtils::IsExitPoint(string pointName, bool side) {
	if (side) { // Gander
		if (find(pointsShanwick.begin(), pointsShanwick.end(), pointName) != pointsShanwick.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
	else { // Shanwick
		if (find(pointsGander.begin(), pointsGander.end(), pointName) != pointsGander.end()) {
			return true; // Match
		}
		else {
			return false; // No match
		}
	}
}

bool CUtils::IsAircraftRelevant(CRadarScreen* screen, CRadarTarget* target) {
	// Flag
	bool valid = true;

	// Flight plan
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(target->GetCallsign());

	// Time and direction
	int entryMinutes = fp.GetSectorEntryMinutes();

	// If not ever going to enter, or greater than 60 min out
	if (entryMinutes < 0 || entryMinutes > 60) {
		valid = false;
	}

	// However we should keep them on the screen if they aren't long out of the airspace
	//if ()

	return valid;
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

string CUtils::PadWithZeros(int width, int number) {
	std::stringstream ss;
	ss << setfill('0') << setw(width) << number;
	return ss.str();
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

double CUtils::GetDistanceSpeedTime(int speedGS, int timeSec) {
	// Get distance in metres
	return ((float)speedGS * 0.514444) * ((float)timeSec);
}

double CUtils::ToRadians(double degrees) {
	return degrees / 180.0 * M_PI;
}

double CUtils::ToDegrees(double radians) {
	return radians / M_PI * 180.0;
}

double CUtils::MetresToNauticalMiles(double metres) {
	return double(metres * 0.00053996);
}
 
template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

// This algorithm is a general solution to get the angle of intersection to any given path
double CUtils::GetPathAngle(double hdg1, double hdg2) {
	// Get theta
	double theta = abs(hdg2 - hdg1);

	// If the angle is obtuse, the aircraft are approaching, so minus 180
	if (theta > 90) return theta - 180;

	// Otherwise just return theta
	return theta;
}

CPosition CUtils::GetPointDistanceBearing(CPosition position, int distanceMetres, int heading) {

	// Get distance in nautical miles
	double distance = MetresToNauticalMiles(distanceMetres) / 60 * M_PI / 180;

	// Convert track heading and coordinates to radians
	double track = ToRadians(heading);
	double lat = ToRadians(position.m_Latitude);
	double lon = ToRadians(position.m_Longitude);

	// Calculate lat
	double newLat = asin(sin(lat) * cos(distance) + cos(lat) * sin(distance) * cos(track));

	// Calculate lon
	double newLon = cos(newLat) == 0 ? lon : fmod(lon + asin(sin(track) * sin(distance) / cos(newLat)) + M_PI, 2 * M_PI) - M_PI;
	
	// Return
	CPosition newPos;
	newPos.m_Latitude = CUtils::ToDegrees(newLat);
	newPos.m_Longitude = CUtils::ToDegrees(newLon);
	return newPos;
}

// Get the intersection between two vectors from two screen coordinates and bearings
// This algorithm utilises Euclidean geometry, taking advantage of the available conversions between lat/lon and screen coordinates to get accurate points. 
// If this creates inaccuracies, I will re-do the algorithm in non-Euclidian terms.
POINT CUtils::GetIntersectionFromPointBearing(POINT position1, POINT position2, double bearing1, double bearing2) {
	/* We solve the linear system using Cramer's rule: 
	 * ax1 - y1 = b 
	 * cx2 - y2 = d
	 * to find our intercept. (Origin is 0,0 to lat/lon)
	 * 
	 * Cramer's rule in terms of this solution: 
	 * Ax = b => xi = det(Ai)/det(A) | i = 1, 2
	 * 
	 * We use trigonometry to convert the bearings into positive and negative slopes:
	 * slope = 1 / tan(bearing) where bearing is in radians
	 */
	
	// Get the slopes
	double slope1 = 1.0 / tan(ToRadians(bearing1));
	double slope2 = 1.0 / tan(ToRadians(bearing2));

	// Get our b and d values
	double b = (slope1 * position1.x) + position1.y;
	double d = (slope2 * position2.x) + position2.y;

	// Calculate determinents using the coefficient matrix:
	// [slope1][1]
	// [slope2][1]
	// and the answer matrix:
	// [b]
	// [d]
	double determinent = slope1 - slope2;
	double detX = b - d;
	double detY = (slope1 * d) - (slope2 * b);

	// From the determinents, get our new X and Y values
	int newX = round(detX / determinent);
	int newY = round(detY / determinent);

	// Return the screen coordinates
	return POINT({ newX, newY });
}

