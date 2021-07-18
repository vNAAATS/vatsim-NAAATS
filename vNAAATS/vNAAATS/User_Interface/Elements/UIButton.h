#pragma once
#include "UIElement.h"

using namespace std;
using namespace Gdiplus;
using namespace Styling;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// CONSTANTS
	const int kDefaultButtonWidth = 68;
	const int kDefaultButtonHeight = 30;

	// Config to enable customisation of some behaviours/parameters (default parameters included)
	struct SButtonConfig {
		SButtonConfig() {};
		int Width = kDefaultButtonWidth;
		int Height = kDefaultButtonHeight;
		bool IsCycled = false;
		vector<string> CycleElements;
		bool IsToggled = true;
		EElementState IntialState = EElementState::kInactive;
		Typefaces::ETypeface Typeface = Typefaces::ETypeface::kRegular16;
	};

	// Button class
	class CUIButton : public CUIElement
	{
		public:
			// Construct
			inline CUIButton(int id, int locId, POINT topLeft, string label, SButtonConfig config) 
				: CUIElement(id, locId, topLeft, label, config.IntialState) {
				// Assign config
				Config = config;
			}

			// Inherited methods
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void ButtonDown(int id);
			virtual void ButtonUp(int id);
			void Cycle();

			// Config
			SButtonConfig Config;
	};
}