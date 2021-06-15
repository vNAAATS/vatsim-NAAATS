#pragma once
#include "pch.h"
#include "UIList.h"
#include "User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CInboundList : public CUIList
	{
		public:
			// Construct
			inline CInboundList(POINT topLeft, SListConfig config) : CUIList(topLeft, config) {};

			// Inherited methods
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen);
			virtual void RegisterElements();
			virtual void Interact(int id);
	};
}