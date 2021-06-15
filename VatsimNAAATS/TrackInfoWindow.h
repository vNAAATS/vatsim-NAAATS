#pragma once
#include "pch.h"
#include "UIWindow.h"
#include "User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CTrackInfoWindow : public CUIWindow
	{
		public:
			// Construct
			inline CTrackInfoWindow(POINT topLeft, SWindowConfig config) : CUIWindow(topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();
			virtual void Interact(int id);

			// STATIC UI ELEMENT IDs HERE
			// 
			// static const int...
	};
}