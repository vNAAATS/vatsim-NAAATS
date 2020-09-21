#pragma once
#include <string>
#include "EuroScopePlugIn.h"
#include <chrono>
#include <cmath>

using namespace std;
using namespace EuroScopePlugIn;

class Utils {
	public:
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

		// Use arctan to get the angle
		static double GetHypotenuseAngle(POINT p1, POINT p2) {
			int l;
			int w;
			// Get the side lengths
			l = p1.y - p2.y;
			if (p2.x < p1.x) {
				w = p1.x - p2.x;
			}
			else {
				w = p2.y - p1.y;
			}
			/*
			if (p1.y > p2.y) {
				l = p1.y - p2.y;
				if (p2.x < p1.x) {
					w = p1.x - p2.x;
				}
				else {
					w = p2.y - p1.y;
				}
			}
			else {
				l = p1.y - p1.y;
				if (p1.x < p2.x) {
					w = p2.x - p1.x;
				}
				else {
					w = p1.y - p2.y;
				}
			}*/

			// Theta (in degrees)
			double theta = tan((float)w / (float)l) * (180 / (acos(0.0) * 2));

			// Return the angle (in degrees)
			return atan(theta) * (180 / (acos(0.0) * 2));
		}
};