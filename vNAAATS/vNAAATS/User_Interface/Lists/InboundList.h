#pragma once
#include "UIList.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CInboundList : public CUIList
	{
		public:
			// Construct
			inline CInboundList(int id, int locId, POINT topLeft, SListConfig config)
				: CUIList(id, locId, topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();
		
		private:
			// Friend the controller
			friend class CInboundListController;
	};

	// Logic controller
	class CInboundListController : public CUIComponentController {
		public:
			virtual void OnButtonDown();
			virtual void OnButtonUp();
			virtual void OnDoubleClick();
	};
}