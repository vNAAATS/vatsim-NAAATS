#pragma once
#include "vector"
#include "Structures.h"
#include "EuroScopePlugIn.h"
#include "Overlays.h"
#include "Utils.h"
#include <json.hpp>

using namespace std;
using namespace EuroScopePlugIn;

class CDataHandler
{
	public:
		// NAT Track URL
		static string TrackURL;

		// Download nat track data
		static int PopulateLatestTrackData(CPlugIn* plugin);
};

