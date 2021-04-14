#pragma once
#include "BaseWindow.h"
class CSetupWindow : public CBaseWindow
{
	public:
		// Inherited methods
		CSetupWindow(POINT topLeft);
		virtual void RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen);
		virtual void MakeWindowItems();
		virtual void ButtonDown(int id) = 0;
		virtual void ButtonUp(int id, CRadarScreen* screen = nullptr);
		virtual void ButtonPress(int id);
		virtual void ButtonUnpress(int id) = 0;
		virtual void SetButtonState(int id, CInputState state);

		// Child methods
		void SetTextValue(CRadarScreen* screen, int id, string content);
};

