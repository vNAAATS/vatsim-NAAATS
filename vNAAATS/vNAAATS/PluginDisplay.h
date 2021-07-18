#pragma once
#include <EuroScopePlugIn.h>
#include <map>
#include <string>
#include <vector>
#include <future>
#include "User_Interface/UIIncludes.h"

using namespace std;
using namespace Gdiplus;
using namespace User_Interface;
using namespace EuroScopePlugIn;

class CPluginDisplay : public CRadarScreen 
{
	public:
		// Constructor & Destructor
		CPluginDisplay();
		virtual ~CPluginDisplay();

		// Class members
		void RenderWorker(CDC* dc, Graphics* g, CRadarScreen* screen);

		// Inherited methods
		void OnRefresh(HDC hDC, int Phase);
		void OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released);
		void OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area);
		void OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
		void OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button);
		void OnAsrContentToBeSaved(void);
		void OnAsrContentLoaded(bool Loaded);

		// Close
		inline void OnAsrContentToBeClosed(void)
		{
			// Manually call save
			OnAsrContentToBeSaved();
			delete this;
		}

	private:
		CMenuBar* menuBar = nullptr;
};

