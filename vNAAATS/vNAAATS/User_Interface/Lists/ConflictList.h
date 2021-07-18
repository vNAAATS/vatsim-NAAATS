#pragma once
#include "UIList.h"
#include "..\User_Interface.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CConflictList : public CUIList
	{
		public:
			// Construct
			inline CConflictList(int id, int locId, POINT topLeft, SListConfig config)
				: CUIList(id, locId, topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();

		private:
			// Friend the controller
			friend class CConflictListController;
	};

	// Logic controller
	class CConflictListController : public CUIComponentController {
		public:
			virtual void OnButtonDown();
			virtual void OnButtonUp();
			virtual void OnDoubleClick();
	};
}