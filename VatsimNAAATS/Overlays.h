#pragma once
#include <vector>
#include <string>
#include <map>
#include "Constants.h"
#include "Structures.h"
#include "PathRenderer.h"

using namespace std;

class COverlays
{
	public:
		// Current overlay type
		static COverlayType CurrentType;

		// Current NAT tracks
		static map<string, CTrack> CurrentTracks;

		// Current TMI
		static string CurrentTMI;

		// Display the currently selected overlay
		static void ShowCurrentOverlay(CDC* dc, Graphics* g, CRadarScreen* screen);
};


