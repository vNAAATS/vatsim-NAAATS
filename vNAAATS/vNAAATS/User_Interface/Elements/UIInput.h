#pragma once
#include "UIElement.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// CONSTANTS
	const int kDefaultInputWidth = 100;
	const int kDefaultInputHeight = 20;

	// Config to enable customisation of some behaviours/parameters (default parameters included)
	struct SInputConfig {
		SInputConfig() {};
		SInputConfig(int width) {
			Width = width;
		};
		SInputConfig(int width, int height) {
			Width = width;
			Height = height;
		}
		int Width = kDefaultInputWidth;
		int Height = kDefaultInputHeight;
		EAlignment LabelAlign = EAlignment::kLeftUp;
		string InitialValue = "";
		string HintText = "";
		EElementState IntialState = EElementState::kInactive;
	};

	// Text input class
	class CUIInput : public CUIElement
	{
		public:
			// Construct
			inline CUIInput(int id, int locId, POINT topLeft, string label, SInputConfig config)
				: CUIElement(id, locId, topLeft, label, config.IntialState) {};

			// Inherited methods
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void ButtonDown(int id);
			virtual void ButtonUp(int id);
			void SetTextValue(string value);
			string GetTextValue();

			// Config
			SInputConfig Config;

		private:
			// Sizing
			string Value_;
	};
}