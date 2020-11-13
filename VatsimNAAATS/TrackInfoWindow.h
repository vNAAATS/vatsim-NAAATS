#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "BaseWindow.h"
#include "MenuBar.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CTrackInfoWindow : public CBaseWindow
{
	public:
		// Inherited methods
		CTrackInfoWindow(POINT topLeft);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {};
		void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen, CMenuBar* menuBar);
		virtual void MakeWindowItems();
		virtual void ButtonDown(int id);
		virtual void ButtonUp(int id, CRadarScreen* screen = nullptr);
		virtual void ButtonPress(int id);
		virtual void ButtonUnpress(int id);
		virtual void SetButtonState(int id, CInputState state);

		// Window methods
		void Scroll(CRect area, POINT mousePtr);

		// Window functions to do
		bool NATDataRefresh;

		// Window messages to display
		string MsgDataRefresh = "";

		// Panel definitions
		static const int PANEL_BTNS;

		// Button definitions
		static const int BTN_CLOSE;
		static const int BTN_REFRESH;

	private:
		double trackScrollAreaSize;
		double gripPosDelta = 0;
		double gripSize;
		CRect currentScrollPos = CRect(0, 0, 0, 0);
		int scrollWindowSize;
};

