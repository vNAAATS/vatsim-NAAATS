#pragma once
#include "EuroScopePlugIn.h"
#include <vector>
#include <string>
#include <map>
#include "Constants.h"

using namespace std;
using namespace EuroScopePlugIn;

class COverlays
{
	public:
		// Current NAT tracks
		static map<char, Track> CurrentTracks;

		// Display a particular overlay
		static void ShowOverlay(OverlayType type);
};

// Describes a NAT track
struct Track {
	char Identifier;
	int TMI;
	vector<string> Route;
	vector<int> FlightLevels;
	TrackDirection Direction;
};


