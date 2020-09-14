#pragma once
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class RadarDisplay : public CRadarScreen
{
	public:
		RadarDisplay();
		virtual ~RadarDisplay();

		void OnRefresh(HDC hDC, int Phase);
		void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);
		void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);
		void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);
		void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
		void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnAsrContentToBeSaved(void);
		void OnAsrContentLoaded(bool Loaded);
		void OnAsrContentToBeClosed(void);
};

