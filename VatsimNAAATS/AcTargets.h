#pragma once
#include "pch.h"
#include <gdiplus.h>
#include "Structures.h"
#include <EuroScopePlugIn.h>
#include <map>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

/// Drawing aircraft targets
class CAcTargets
{
	public:
		// Blink timers (for flashing white if critical conflict & halo around searched aircraft)
		static clock_t twoSecondTimer;
		static clock_t fiveSecondTimer;

		// Initialisation for button states
		static void Initialise();

		// Render the airplane icon
		static void RenderTarget(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, bool tagsOn, map<int, CWinButton>* toggleData, bool halo, bool ptl, CSTCAStatus* status);

		// Render tags
		static POINT RenderTag(CDC* dc, CRadarScreen* screen, CRadarTarget* target, pair<bool, POINT>* tagPosition, bool direction, CSTCAStatus* status);

		// Coordination tag item
		static void RenderCoordTagItem(CDC* dc, CRadarScreen* screen, string callsign, POINT tagPosition);

		// Draw selection halo
		static void RenderSelectionHalo(Graphics* g, CRadarScreen* screen, CRadarTarget* target);

		/// DIFFERENT TARGETS
		static void RenderADSBNoClrc(Graphics* g); // Diamond with a line through middle
		static void RenderRadarCoverage(Graphics* g); // Asterisk
		static void RenderClearanceWarning(Graphics* g); // Triangle
		static void RenderADSBCleared(Graphics* g); // Aeroplane icon

		// Aircraft search
		static string SearchedAircraft;

		// Fields
		static string OpenTrackingDialog;
		static map<string, bool> ButtonStates;
};
