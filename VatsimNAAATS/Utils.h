#pragma once
#include <string>
#include "EuroScopePlugIn.h"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <psapi.h>
#include "Structures.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace CUtils {
		// Variables to save
		int InboundX = 1400;
		int InboundY = 170;
		int OthersX = 1200;
		int OthersY = 150;
		int ConflictX = 60; // save
		int ConflictY = 120; // save
		int RCLX = 600; // save
		int RCLY = 150; // save
		int TrackWindowX = 300;
		int TrackWindowY = 200;
		int AltFiltLow = 0;
		int AltFiltHigh = 700;
		bool GridEnabled = false;
		bool TagsEnabled = true;
		bool QckLookEnabled = false;
		bool OverlayEnabled = false;
		int AreaSelection = 802;
		int SelectedOverlay = 800;
		int PosType = 802;
		char DllPathFile[_MAX_PATH];
		string DllPath;
		int SepMinimaVertical;
		int SepMinimaLateral;
		int SepMinimaLongitudinal;

		// Load/save methods
		void SavePluginData(CRadarScreen* screen);
		void LoadPluginData(CRadarScreen* screen);
		
		// Text wrapping
		bool WrapText(CDC* dc, string textToWrap, char wrapChar, int contentWidth, vector<string>* ptrWrappedText);

		// Split string
		bool StringSplit(string str, char splitBy, vector<string>* ptrTokens);

		// Phraseology parser
		string ParseToPhraseology(string rawInput, CMessageType type, string callsign);

		// Raw format parser
		string ParseToRaw(string callsign, CMessageType type, CAircraftFlightPlan* copy = nullptr);

		// Convert coordinates to various type
		string ConvertCoordinateFormat(string coordinateString, int format);

		// Get the aircraft direction
		bool GetAircraftDirection(int heading);

		// Check if point is an entry point (direction: true = Gander, false = Shanwick)
		bool IsEntryPoint(string pointName, bool direction);

		// Check if point is an exit point (direction: true = Gander, false = Shanwick)
		bool IsExitPoint(string pointName, bool direction);

		// Check if the aircraft is to be displayed on the screen
		bool IsAircraftRelevant(CRadarScreen* screen, CRadarTarget* target, bool filtersDisabled = false);

		// Check if aircraft is AGCS equipped
		bool IsAircraftEquipped(string rawRemarks, string rawAcInfo, char equipCode);
		
		// Get CPosition from lat/lon
		CPosition PositionFromLatLon(double lat, double lon);

		// Calculate Mach
		int GetMach(int groundSpeed, int speedSound);

		// Get radar target mode
		CRadarTargetMode GetTargetMode(int radarFlags);

		// Pad zeroes
		string PadWithZeros(int width, int number);

		// Check all alpha
		bool IsAllAlpha(string str);

		// Find selcal code from remarks
		string GetSelcalCode(CFlightPlan* fpData);

		// Parse zulu time
		string ParseZuluTime(bool delimit, int deltaTime = -1, CFlightPlan* fp = nullptr, int ep = -1);

		// Distance between two points in screen coordinates
		int GetDistanceBetweenPoints(POINT p1, POINT p2);

		// Midpoint
		POINT GetMidPoint(POINT p1, POINT p2);

		// Time (minutes) based on distance and speed
		int GetTimeDistanceSpeed(int distanceNM, int speedGS);

		// Distance (metres) based on speed and time
		double GetDistanceSpeedTime(int speedGS, int timeSec);

		// Metres to nautical miles
		double MetresToNauticalMiles(double metres);

		// Degrees to radians
		double ToRadians(double degrees);

		// Radians to degrees
		double ToDegrees(double radians);

		// Round a double
		string RoundDecimalPlaces(double num, int precision);

		// General solution to get the angle between two intersecting paths
		double GetPathAngle(double hdg1, double hdg2);

		// Get a point based on distance and a heading
		CPosition GetPointDistanceBearing(CPosition position, int distanceMetres, int heading);

		// Get intersection of two vectors
		POINT GetIntersectionFromPointBearing(POINT position1, POINT position2, double bearing1, double bearing2);

		// Get parsed lat lon string (i.e. with N/S or E/W)
		string GetLatLonString(CPosition* pos, bool space = true, int precision = -1, bool showDecimal = true);

		// So we can stop running threads
		HANDLE GetESProcess();

		// We need this struct for flight plan threading
		struct CAsyncData {
			CRadarScreen* Screen;
			string Callsign;
			CAircraftFlightPlan* FP = nullptr;
		};

		// This is for asyncing vNAAATS API data
		struct CNetworkAsyncData {
			CRadarScreen* Screen;
			string Callsign;
			CPlugIn* plugin = nullptr;
			CNetworkFlightPlan* FP = nullptr;
		};
};