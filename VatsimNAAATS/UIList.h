#pragma once
#include "pch.h"
#include "User_Interface.h"

using namespace std;
using namespace EuroScopePlugIn;

namespace User_Interface {
	class CUIList : public CUIBase
	{
		public:
			CUIList(POINT topLeft, SListConfig config);

			// Shared methods
			POINT GetPosition();
			void SetPosition(CRect topleft);
			void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen);

			// Forced overrides
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;

			// Base overrides
			virtual void RegisterElements() = 0;
			virtual void Interact(int id) = 0;

			// Fields
			int ID;
			int LocID;
			bool IsCollapsed;
			SListConfig Config;

		protected:
			// Fields
			POINT topLeft;
			map<int, SListColumn> columns;
			vector<vector<string>> rowMatrix;
			int width; // determined from sum of column width
			int height = 15;
	};

	// Config to enable customisation of some behaviours/parameters
	struct SListConfig {
		SListConfig() {};
		bool Collapsable = true;
	};

	// Column definition
	struct SListColumn {
		SListColumn(EListColumnType t, int w) {
			Type = t;
			Width = w;
		}
		EListColumnType Type;
		int Width;
	};	
}