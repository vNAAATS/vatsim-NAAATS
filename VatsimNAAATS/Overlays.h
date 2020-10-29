#pragma once
#include <vector>
#include <string>
#include <map>
#include "Constants.h"
#include "Structures.h"
#include "CommonRenders.h"

using namespace std;

class COverlays
{
	public:
		// Current overlay type
		static COverlayType CurrentType;

		// Display the currently selected overlay
		static void ShowCurrentOverlay(CDC* dc, Graphics* g, CRadarScreen* screen);

		// Grid reference
		static void ShowHideGridReference(CRadarScreen* screen, bool show);

		// Clear route
		static int ClearCurrentRoute();
};


