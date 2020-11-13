#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "BaseWindow.h"
#include "FlightPlanWindow.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CMessageWindow : public CBaseWindow
{
	public:
		// Inherited methods
		CMessageWindow(POINT topLeft);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		virtual void MakeWindowItems();
		virtual void ButtonDown(int id);
		virtual void ButtonUp(int id, CRadarScreen* screen = nullptr);
		virtual void ButtonPress(int id);
		void ButtonDoubleClick(CRadarScreen* screen, int id, CFlightPlanWindow* fltPlnWin);
		virtual void ButtonUnpress(int id);
		virtual void SetButtonState(int id, CInputState state);

		// Button definitions
		static const int BTN_CLOSE;

		// Currently active messages
		static unordered_map<int, CMessage> ActiveMessages;
		static unordered_map<int, CMessage*> OngoingMessages;
		int SelectedMessage = -1;
		int MessageCount = 0;

		static const int SCRL_MSGWNDW = 520;
};

