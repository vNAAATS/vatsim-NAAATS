#pragma once
#include <vector>
#include <string>
#include <map>
#include "Constants.h"
#include "Structures.h"
#include "CommonRenders.h"
#include "MenuBar.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class COverlays
{
	public:
		// Current overlay type
		static COverlayType CurrentType;

		// Display the currently selected overlay
		static void ShowCurrentOverlay(CDC* dc, Graphics* g, CRadarScreen* screen, CMenuBar* menubar);

		// Grid reference
		static void ShowHideGridReference(CRadarScreen* screen, bool show);
};


