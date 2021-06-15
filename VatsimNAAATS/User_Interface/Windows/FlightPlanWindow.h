#pragma once
#include "..\..\pch.h"
#include "UIWindow.h"
#include "..\User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CFlightPlanWindow : public CUIWindow
	{
		public:
			// Construct
			inline CFlightPlanWindow(POINT topLeft, SWindowConfig config) : CUIWindow(topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();
			virtual void Interact(int id);

			// ELEMENT MATRIX AND STANDALONE ELEMENT DEFINITIONS BELOW
			// 
			// static const int kMatrix[x][y] = ...; // element matrix
			// static const int kCheckbox = ...; // STANDALONE element
	};
}