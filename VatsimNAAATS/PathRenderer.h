#pragma once
#include "EuroScopePlugIn.h"
#include "Constants.h"

using namespace std;
using namespace EuroScopePlugIn;

class CPathRenderer
{
	public:
		// Target to compare ASEL against for PIV
		static CRadarTarget pivTarget;

		// Render a path
		static void RenderPath(PathType type, CRadarTarget* asel = nullptr);
};