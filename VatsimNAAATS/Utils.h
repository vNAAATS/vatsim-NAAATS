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
		static int AltFiltLow;
		static int AltFiltHigh;
		static bool GridEnabled;
		static bool TagsEnabled;

		static void SavePluginData(CPlugIn* plugin);
		static void LoadPluginData(CPlugIn* plugin);

		// TODO: Cleanup into cpp file

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
		static pair<double, double> GetPointDistanceBearing(pair<double, double> position, int distanceNM, int heading);
};