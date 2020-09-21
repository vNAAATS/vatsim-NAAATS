#pragma once
#include <map>
#include <string>
#include <vector>
#include "InboundList.h"
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
		POINT mousePointer;
		clock_t buttonClickTimer;
		map<int, string> menuButtons;
		map<int, bool> buttonsPressed;
		map<int, int> toggleButtons;
		bool aselDetailed;	
		vector<pair<CRadarTarget, bool>> inboundAircraft;
		CInboundList* inboundList;
		map<int, string> dropDownsCurrent;
		map<int, string> menuFields;
		string asel;
		map<string, pair<bool, POINT>> tagStatuses;

		// Selected active tracks for ownership (vector)
		// Selected active control postion for ownership
		// ASEL
};

