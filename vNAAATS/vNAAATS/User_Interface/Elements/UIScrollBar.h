#pragma once
#include "UIElement.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CUIScrollBar : public CUIElement
	{
		public:
			// Construct
			inline CUIScrollBar(int id, int locId, POINT topLeft, string label)
				: CUIElement(id, locId, topLeft, label, EElementState::kInactive) {};

			// Inherited methods
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void ButtonDown(int id);
			virtual void ButtonUp(int id);

		private:

	};
}