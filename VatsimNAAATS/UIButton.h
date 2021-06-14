#pragma once
#include "pch.h"
#include "UserInterface.h"
#include "UIElement.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace UserInterface {
	class CUIButton : public CUIElement
	{
		public:
			// Size is (width, height)
			//CUIButton(int id, int locId, string label, pair<int, int> size, POINT topLeft, SUIElementConfig config,
				//bool cycle = false, vector<pair<int, string>>* cycleElements = nullptr); 

			// have individual worker functions in each window/modal that are called on each button click
			//virtual void ButtonDown(int id) = 0;
			//virtual void ButtonUp(int id) = 0;
			

		protected:

	};
}