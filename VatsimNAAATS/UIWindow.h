#pragma once
#include "pch.h"
#include "UserInterface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace UserInterface {
	class CUIWindow
	{
		public:
			// Construct/destruct
			CUIWindow(POINT topLeft);
			virtual ~CUIWindow();

			// Shared methods
			POINT GetPosition();
			void SetPosition(CRect topleft);
			void GetElementType(int id);
			void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen);

			// Forced overrides
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			virtual void RegisterElements() = 0;

			// Closed status
			bool IsClosed = true;

		protected:
			// Fields
			POINT topLeft;

			// Window elements
			map<int, CUIButton> windowButtons;
			map<int, CUIInput> textInputs;
			map<int, CUICheckbox> checkBoxes;
			map<int, CUIDropDown> dropDowns;
			map<int, CUIScrollBar> scrollBars;
	};
}