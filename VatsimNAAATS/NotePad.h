#pragma once
#include "pch.h"
#include "EuroScopePlugIn.h"
#include <string>
#include <map>
#include <gdiplus.h>
#include "BaseWindow.h"
#include "BaseList.h"
#include "Constants.h"
#include "Structures.h"
#include "CommonRenders.h"

class CNotePad : public CBaseWindow, public CBaseList
{
	public:
		// Inherited methods
		CNotePad(POINT topLeft, POINT topLeftList);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		virtual void MakeWindowItems();
		virtual void ButtonDown(int id);
		virtual void ButtonUp(int id);
		virtual void ButtonPress(int id);
		virtual void ButtonUnpress(int id);
		virtual void SetButtonState(int id, CInputState state);
		virtual void RenderList(Graphics* g, CDC* dc, CRadarScreen* screen);

		// Window methods
		void Scroll(CRect area, POINT mousePtr);

	private:
		string notePadContent;
		double trackScrollAreaSize;
		double gripPosDelta = 0;
		double gripSize;
		CRect currentScrollPos = CRect(0, 0, 0, 0);
		int scrollWindowSize;
};

