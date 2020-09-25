#pragma once
#include <string>
#include "EuroScopePlugIn.h"
#include <chrono>
#include <cmath>

using namespace std;
using namespace EuroScopePlugIn;

class CUtils {
	public:
		// Variables to save
		static int InboundX;
		static int InboundY;
		static int OthersX;
		static int OthersY;
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


		// Load/save methods
		static void SavePluginData(CPlugIn* plugin);
		static void LoadPluginData(CPlugIn* plugin);

		// Convert other types of coordinates to NN/WW format
		static string ConvertCoordinatesToSlashFormat(string coordinateString);

		// Get the aircraft direction
		static bool GetAircraftDirection(int heading);

		// Check if point is a Gander/Shanwick border point (true = Gander, false = Shanwick)
		static bool IsEntryExitPoint(string pointName, bool direction);

		// Get CPosition from lat/lon
		static CPosition PositionFromLatLon(double lat, double lon);

		// Calculate Mach
		static int GetMach(int groundSpeed, int speedSound);

		// Parse zulu time
		static string ParseZuluTime(bool delimit, CFlightPlan* fp = nullptr, int ep = -1);

		// Distance between two points in screen coordinates
		static int GetDistanceBetweenPoints(POINT p1, POINT p2);

		// Midpoint
		static POINT GetMidPoint(POINT p1, POINT p2);

		// Time (minutes) based on distance and speed
		static int GetTimeDistanceSpeed(int distanceNM, int speedGS);

		// Distance (nm) based on speed and time
		static int GetDistanceSpeedTime(int speedGS, int timeMin);

		// Degrees to radians
		static double ToRadians(double degrees);

		// Radians to degrees
		static double ToDegrees(double radians);

		// Get a point based on distance and a heading
		static CPosition GetPointDistanceBearing(CPosition position, int distanceNM, int heading);
};