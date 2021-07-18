#pragma once
#include "UIElement.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// Config to enable customisation of some behaviours/parameters (default parameters included)
	struct SDropDownConfig {
		SDropDownConfig();
	};
	// Dropdown element
	//class CUIDropDownElement {

	//};

	// Actual dropdown class
	class CUIDropDown : public CUIElement
	{
		public:
			// Construct
			inline CUIDropDown(int id, int locId, POINT topLeft, string label)
				: CUIElement(id, locId, topLeft, label, EElementState::kInactive) {};

			// Inherited methods
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void ButtonDown(int id);
			virtual void ButtonUp(int id);
		private:
			// Sizing
			string CurrentValue_;
			int CurrentHover_;
			//map<int, CUIDropDownElement> Elements_;
	};
}