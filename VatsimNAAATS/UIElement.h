#pragma once
#include "pch.h"
#include "UserInterface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace UserInterface {
	class CUIElement
	{
		public:
			// Methods
			CUIElement();
			virtual ~CUIElement();
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen);
			POINT GetTopLeft();
			void SetElementState(CElementState state);

			// Fields
			int ID;
			int LocID;
			CElementState State;

		protected:
			POINT TopLeft_;
	};
}