#pragma once
#include "User_Interface.h"
#include "Styling.h"
#include "Elements/UIButton.h"
#include "Elements/UIInput.h"
#include "Elements/UICheckBox.h"
#include "Elements/UIDropDown.h"
#include "Elements/UIScrollBar.h"
#include "Elements/UIContextMenu.h"

using namespace std;
using namespace EuroScopePlugIn;
using namespace Styling;

namespace User_Interface {
	class CUIComponentBase
	{
		public:
			// Construct
			inline CUIComponentBase(int id, int locId) {
				ID = id;
				LocID = locId;
			};
			// Destruct
			virtual ~CUIComponentBase() {};

			// Overrides
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			virtual void RegisterElements() = 0;

			// Fields
			int ID;
			int LocID;

		protected:
			// This is called in RenderContent
			virtual void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			// Elements
			map<int, CUIButton> Buttons_;
			map<int, CUIInput> Inputs_;
			map<int, CUICheckBox> CheckBoxes_;
			map<int, CUIDropDown> DropDowns_;
			map<int, CUIScrollBar> ScrollBars_;
			map<int, CUIContextMenu> ContextMenus_;
	};

	// Logic controller for each component
	class CUIComponentController {
		public:
			virtual void OnButtonDown() = 0;
			virtual void OnButtonUp() = 0;
			virtual void OnDoubleClick() = 0;
	};
}