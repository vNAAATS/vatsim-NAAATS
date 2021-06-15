#pragma once
#include "..\..\pch.h"
#include "..\User_Interface.h"

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
			vector<SListColumn> columns;
			map<int, map<int, SListElement>> rowMatrix; // key is the row ID, the second map comprises the elements of the row
			int width; // determined from sum of column width
			int height; // determined from sum of rows * row heights
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

	// Element definition
	struct SListElement {
		SListElement(int id, bool clickable, SListColumn* column) {
			Id = id;
			IsClickable = clickable;
			Column = column;
		};
		int Id;
		bool IsClickable;
		SListColumn* Column;

	};
}