#pragma once
#include <string>
#include "EuroScopePlugIn.h"
#include <chrono>
#include <cmath>

using namespace std;
using namespace EuroScopePlugIn;

class Utils {
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
		static string ParseZuluTime(bool delimit, CFlightPlan* fp = nullptr, int ep = -1) {
			time_t now = time(0);
			tm* zuluTime = gmtime(&now);
			int deltaMinutes = 0;
			if (ep != -1) {
				deltaMinutes = fp->GetExtractedRoute().GetPointDistanceInMinutes(ep);
			}
			int hours = zuluTime->tm_hour;
			int minutes = zuluTime->tm_min + deltaMinutes;

			if (minutes >= 60) {
				// Get minutes
				int minRemainder = minutes % 60;

				// Get number of hours
				hours = (minutes - minRemainder) / 60;

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
		};

		// Distance between two points in screen coordinates
		static int GetDistanceBetweenPoints(POINT p1, POINT p2) {
			// Get distance
			return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
		}

		// Midpoint
		static POINT GetMidPoint(POINT p1, POINT p2) {
			// Return midpoint
			return { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2 };
		}

		// Time (minutes) based on distance and speed
		static int GetTimeBetweenPoints(int distanceNM,  int speedGS) {
			// Get time in minutes
			double temp = (float)distanceNM / (float)speedGS;
			return ((float)distanceNM / speedGS) * 60;
		}
};