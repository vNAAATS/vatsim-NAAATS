#pragma once
#include "../User_Interface.h"
#include "../Styling.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CUIElement
	{
		public:
			// Constructor
			inline CUIElement(int id, int locId, POINT topLeft, string label, EElementState state) {
				// Assign
				ID = id;
				LocID = locId;
				TopLeft_ = topLeft;
				Label_ = label;
				State = state;
			};
			inline virtual ~CUIElement() {};
			inline POINT GetPosition() {
				return TopLeft_;
			};
			inline void SetPosition(POINT topLeft) {
				TopLeft_ = topLeft;
			};
			inline void SetElementState(EElementState state) {
				State = state;
			};
			inline EElementState GetElementState() {
				return State;
			};

			// Forced child overrides
			virtual void RenderElement(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			virtual void ButtonDown(int id) = 0;
			virtual void ButtonUp(int id) = 0;

			// Fields
			int ID;
			int LocID;
			EElementState State;

		protected:
			// Fields
			string Label_;
			POINT TopLeft_;
	};
}