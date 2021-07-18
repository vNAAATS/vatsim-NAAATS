#pragma once
#include "UIElement.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// Config to enable customisation of some behaviours/parameters (default parameters included)
	struct SCheckBoxConfig {
		SCheckBoxConfig() {};
		EAlignment LabelAlign = EAlignment::kRight;
		EElementState IntialState = EElementState::kInactive;
		bool InitialCheckStatus = false;
	};

	// Checkbox class
	class CUICheckBox : public CUIElement
	{
		public:
			// Construct
			inline CUICheckBox(int id, int locId, POINT topLeft, string label, SCheckBoxConfig config)
				: CUIElement(id, locId, topLeft, label, Config.IntialState) {};

			// Inherited methods
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void ButtonDown(int id);
			virtual void ButtonUp(int id);

			// Config & fields
			SCheckBoxConfig Config;
			bool IsChecked; // We have this since an element can be checked and also be active/inactive/disabled
	};
}