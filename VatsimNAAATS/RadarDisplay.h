#pragma once
#include <map>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

// Main radar display
class RadarDisplay : public CRadarScreen
{
	public:
		// Constructor & Destructor
		RadarDisplay();
		virtual ~RadarDisplay();

		// Inherited methods
		void OnRefresh(HDC hDC, int Phase);
		void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);
		void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);
		void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
		void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnAsrContentToBeSaved(void);
		void OnAsrContentLoaded(bool Loaded);
		void OnAsrContentToBeClosed(void);

	private:
		map<int, string> MenuButtons;
		map<int, bool> ButtonsPressed;
};

