#pragma once
#include "UIElement.h"
#include "UIDropDown.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// Config to enable customisation of some behaviours/parameters (default parameters included)
	struct SContextMenuConfig {
		SContextMenuConfig() {};
	};

	// TODO: Implement
	// Context menu class
	class CUIContextMenu : public CUIElement
	{
		public: // TODO: implement properly when needed
			// Construct
			inline CUIContextMenu(int id, int locId, POINT topLeft, string label, SContextMenuConfig config)
				: CUIElement(id, locId, topLeft, label, EElementState::kInactive) {};

			// Inherited methods
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void ButtonDown(int id);
			virtual void ButtonUp(int id);

			// Config
			SContextMenuConfig Config;
			//map<int, CUIDropDownElement> MenuItems; // TODO: Refine this

		private:
			// Sizing
			int Width_;
			int Height_;

	};
}