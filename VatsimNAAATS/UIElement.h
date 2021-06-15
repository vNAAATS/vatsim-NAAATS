#pragma once
#include "pch.h"
#include "User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CUIElement
	{
		public:
			// Construct/destruct
			CUIElement();
			virtual ~CUIElement();
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			void SetElementState(EElementState state);

			// Fields
			int ID;
			int LocID;
			EElementState State;
	};
}