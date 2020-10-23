#pragma once
#include "EuroScopePlugIn.h"
#include "Constants.h"
#include "Structures.h"
#include "Utils.h"
#include "Styles.h"

using namespace std;
using namespace Colours;
using namespace EuroScopePlugIn;

class CPathRenderer
{
	public:
		// Render a path
		static void RenderPath(CDC* dc, Graphics* g, CRadarScreen* screen, CPathType type, vector<CSepStatus>* pivA = nullptr, vector<CSepStatus>* pivB = nullptr);
};