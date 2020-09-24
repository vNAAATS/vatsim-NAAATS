#pragma once
#include <vector>
#include <string>
#include <map>
#include "Constants.h"
#include "Structures.h"

using namespace std;

class COverlays
{
	public:
		// Current NAT tracks
		static map<char, CTrack> CurrentTracks;

		// Display a particular overlay
		static void ShowOverlay(COverlayType type);
};


