#pragma once
#include "..\..\pch.h"
#include "..\User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CUIWindow : public CUIBase
	{
		public:
			CUIWindow(POINT topLeft, SWindowConfig config);

			// Shared methods
			POINT GetPosition();
			void SetPosition(CRect topleft);
			void GetElementType(int id);
			void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen);

			// Forced overrides
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;

			// Base overrides
			virtual void RegisterElements() = 0;
			virtual void Interact(int id) = 0;

			// Fields
			int ID;
			int LocID;
			bool IsClosed = true;
			SWindowConfig Config;

		protected:
			// Fields
			POINT topLeft;
			int width;
			int height;
	};

	// Config to enable customisation of some behaviours/parameters
	struct SWindowConfig { 
		SWindowConfig() {};
		bool IsModal = false;
	};
}