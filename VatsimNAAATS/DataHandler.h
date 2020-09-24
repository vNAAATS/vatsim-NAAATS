#pragma once
#include "vector"
#include "Structures.h"
#include "EuroScopePlugIn.h"

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

