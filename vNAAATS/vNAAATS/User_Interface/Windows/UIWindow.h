#pragma once
#include "../UIComponentBase.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// CONSTANTS
	const int kWindowTitlebarHeight = 20;

	// Config to enable customisation of some behaviours/parameters
	struct SWindowConfig {
		SWindowConfig() {};
		bool IsModal = false;
	};
	// Base window class
	class CUIWindow : public CUIComponentBase
	{
		public:
			// Construct
			inline CUIWindow(int id, int locId, POINT topLeft, SWindowConfig config) : CUIComponentBase(id, locId) {};

			// Shared methods
			POINT GetPosition();
			void SetPosition(POINT topLeft);
			EElementType GetElementType(int id);
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			virtual void RegisterElements() = 0;

			// Fields
			bool IsClosed = true;
			SWindowConfig Config;

		protected:
			// Called in RenderContent
			virtual void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen);
			// Fields
			string Label_;
			POINT TopLeft_;
			int Width_;
			int Height_;
	};
}