#pragma once
#include "..\pch.h"
#include "User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CUIBase
	{
		public:
			// Construct/destruct
			CUIBase();
			virtual ~CUIBase();

			// Overrides
			virtual void RegisterElements() = 0;
			virtual void Interact(int id) = 0; // Worker function to process interaction with a UI element

		protected:
			// Elements
			map<int, CUIButton> buttons;
			map<int, CUIInput> inputs;
			map<int, CUICheckbox> checkBoxes;
			map<int, CUIDropDown> dropDowns;
			map<int, CUIScrollBar> scrollBars;
			map<int, CUIContextMenu> contextMenus;
	};
}