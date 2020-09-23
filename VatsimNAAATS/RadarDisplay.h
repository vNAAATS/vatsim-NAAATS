#pragma once
#include <map>
#include <string>
#include <vector>
#include "InboundList.h"
#include "OtherList.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

// Main radar display
class CRadarDisplay : public CRadarScreen
{
	public:
		// Constructor & Destructor
		CRadarDisplay();
		virtual ~CRadarDisplay();

		// Custom methods
		void ShowHideGridReference(CRadarScreen* screen, bool show);		
		void PopulateProgramData();

		// Inherited methods
		void OnRefresh(HDC hDC, int Phase);
		void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);
		void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);
		void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
		void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnAsrContentToBeSaved(void);
		void OnAsrContentLoaded(bool Loaded);

		inline void OnAsrContentToBeClosed(void)
		{
			// Manually call save
			OnAsrContentToBeSaved();
			delete this;
		}

	private:
		POINT mousePointer;
		clock_t buttonClickTimer;
		map<int, string> menuButtons;
		map<int, bool> buttonsPressed;
		map<int, int> toggleButtons;
		bool aselDetailed;	
		vector<pair<CRadarTarget, bool>> inboundAircraft;
		vector<CRadarTarget> otherAircraft;
		CInboundList* inboundList;
		COtherList* otherList;
		map<int, string> menuFields;
		string asel;
		map<string, pair<bool, POINT>> tagStatuses;
		string aircraftSel1 = ""; // For use in conflict tools
		string aircraftSel2 = ""; // "
};

