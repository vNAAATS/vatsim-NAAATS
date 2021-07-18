#pragma once
#include "UIList.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class COtherList : public CUIList
	{
		public:
			// Construct
			inline COtherList(int id, int locId, POINT topLeft, SListConfig config) 
				: CUIList(id, locId, topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();

		private:
			// Friend the controller
			friend class COtherListController;
	};

	// Logic controller
	class COtherListController : public CUIComponentController {
		public:
			virtual void OnButtonDown();
			virtual void OnButtonUp();
			virtual void OnDoubleClick();
	};
}