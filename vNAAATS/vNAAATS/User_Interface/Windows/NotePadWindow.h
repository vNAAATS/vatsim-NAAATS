#pragma once
#include "UIWindow.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CNotePadWindow : public CUIWindow
	{
		public:
			// Construct
			inline CNotePadWindow(int id, int locId, POINT topLeft, SWindowConfig config)
				: CUIWindow(id, locId, topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();

			// ELEMENT MATRIX AND STANDALONE ELEMENT DEFINITIONS BELOW
			// 
			// static const int kMatrix[x][y] = ...; // element matrix
			// static const int kCheckbox = ...; // STANDALONE element
		private:
			// Friend the controller
			friend class CNotePadWindowController;
	};

	// Logic controller
	class CNotePadWindowController : public CUIComponentController {
		public:
			virtual void OnButtonDown();
			virtual void OnButtonUp();
			virtual void OnDoubleClick();
	};
}