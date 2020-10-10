#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "BaseWindow.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CFlightPlanWindow : public CBaseWindow
{
	public:
		// Inherited methods
		CFlightPlanWindow(POINT topLeft);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		virtual void MakeWindowItems();

		// Fill data
		void UpdateData(CRadarScreen* screen, CAcFPStatus status);
		void OnCloseFlightPlanWindow();
		int ChangeDataPoint(CRadarScreen* screen, int data, string str); // Master method to deal with all cases of text input

		// Button definitions
		static const int BTN_CLOSE;
		static const int BTN_COPY;
		static const int BTN_TRNSFR;
		static const int BTN_COORD;
		static const int BTN_MANCLEAR;
		static const int BTN_PROBE;
		static const int BTN_DELETE;
		static const int BTN_ADS;
		static const int BTN_READBK;
		static const int BTN_MSG;
		static const int BTN_HIST;
		static const int BTN_SAVE;

		// Text definitions
		static const int TXT_ACID;
		static const int TXT_TYPE;
		static const int TXT_DEPART;
		static const int TXT_ETD;
		static const int TXT_SELCAL;
		static const int TXT_DATALINK;
		static const int TXT_COMMS;
		static const int TXT_OWNERSHIP;
		static const int TXT_SPD;
		static const int TXT_LEVEL;
		static const int TXT_DEST;
		static const int TXT_TCK;
		static const int TXT_STATE;
		static const int TXT_SPD_CPY;
		static const int TXT_LEVEL_CPY;
		static const int TXT_DEST_CPY;
		static const int TXT_TCK_CPY;
		static const int TXT_STATE_CPY;
};

