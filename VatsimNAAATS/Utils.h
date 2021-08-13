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

class CUtils { // TODO: refactor into namespace
	public:
		// Variables to save
		static int InboundX;
		static int InboundY;
		static int OthersX;
		static int OthersY;
		static int ConflictX; // save
		static int ConflictY; // save
		static int RCLX; // save
		static int RCLY; // save
		static int TrackWindowX;
		static int TrackWindowY;
		static int AltFiltLow;
		static int AltFiltHigh;
		static bool GridEnabled;
		static bool TagsEnabled;
		static bool QckLookEnabled;
		static bool OverlayEnabled;
		static int AreaSelection;
		static int SelectedOverlay;
		static int PosType;
		static int SepMinimaVertical;
		static int SepMinimaLateral;
		static int SepMinimaLongitudinal;

		// Load/save methods
		static void SavePluginData(CRadarScreen* screen);
		static void LoadPluginData(CRadarScreen* screen);
		
		// Text wrapping
		static bool WrapText(CDC* dc, string textToWrap, char wrapChar, int contentWidth, vector<string>* ptrWrappedText);

		// Split string
		static bool StringSplit(string str, char splitBy, vector<string>* ptrTokens);

		// Phraseology parser
		static string ParseToPhraseology(string rawInput, CMessageType type, string callsign);

		// Raw format parser
		static string ParseToRaw(string callsign, CMessageType type, CAircraftFlightPlan* copy = nullptr);

		// Convert coordinates to various type
		static string ConvertCoordinateFormat(string coordinateString, int format);

		// Get the aircraft direction
		static bool GetAircraftDirection(int heading);

		// Check if point is an entry point (direction: true = Gander, false = Shanwick)
		static bool IsEntryPoint(string pointName, bool direction);

		// Check if point is an exit point (direction: true = Gander, false = Shanwick)
		static bool IsExitPoint(string pointName, bool direction);

		// Check if the aircraft is to be displayed on the screen
		static bool IsAircraftRelevant(CRadarScreen* screen, CRadarTarget* target, bool filtersDisabled = false);

		// Check if aircraft is AGCS equipped
		static bool IsAircraftEquipped(string rawRemarks, string rawAcInfo, char equipCode);
		
		// Get CPosition from lat/lon
		static CPosition PositionFromLatLon(double lat, double lon);

		// Calculate Mach
		static int GetMach(int groundSpeed, int speedSound);

		// Get radar target mode
		static CRadarTargetMode GetTargetMode(int radarFlags);

		// Pad zeroes
		static string PadWithZeros(int width, int number);

		// Check all alpha
		static bool IsAllAlpha(string str);

		// Find selcal code from remarks
		static string GetSelcalCode(CFlightPlan* fpData);

		// Parse zulu time
		static string ParseZuluTime(bool delimit, int deltaTime = -1, CFlightPlan* fp = nullptr, int ep = -1);

		// Distance between two points in screen coordinates
		static int GetDistanceBetweenPoints(POINT p1, POINT p2);

		// Midpoint
		static POINT GetMidPoint(POINT p1, POINT p2);

		// Time (minutes) based on distance and speed
		static int GetTimeDistanceSpeed(int distanceNM, int speedGS);

		// Distance (metres) based on speed and time
		static double GetDistanceSpeedTime(int speedGS, int timeSec);

		// Metres to nautical miles
		static double MetresToNauticalMiles(double metres);

		// Degrees to radians
		static double ToRadians(double degrees);

		// Radians to degrees
		static double ToDegrees(double radians);

		// Round a double
		static string RoundDecimalPlaces(double num, int precision);

		// General solution to get the angle between two intersecting paths
		static double GetPathAngle(double hdg1, double hdg2);

		// Get a point based on distance and a heading
		static CPosition GetPointDistanceBearing(CPosition position, int distanceMetres, int heading);

		// Get intersection of two vectors
		static POINT GetIntersectionFromPointBearing(POINT position1, POINT position2, double bearing1, double bearing2);

		// Get parsed lat lon string (i.e. with N/S or E/W)
		static string GetLatLonString(CPosition* pos, bool space = true, int precision = -1, bool showDecimal = true);

		// So we can stop running threads
		static HANDLE GetESProcess();

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

		// DLL path
		static char DllPathFile[_MAX_PATH];
		static string DllPath;
};