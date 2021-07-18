#pragma once
#include "../UIComponentBase.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

namespace User_Interface {
	// CONSTANTS
	const int kListHandleHeight = 15;

	// Config to enable customisation of some behaviours/parameters (default parameters included)
	struct SListConfig {
		SListConfig() {};
		bool Collapsable = true;
		int MaxRowsBeforeScroll = 20;
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
		SListElement(int id, string value, bool clickable, SListColumn* column) {
			Id = id;
			Value = value;
			IsClickable = clickable;
			Column = column;
		};
		int Id;
		string Value;
		bool IsClickable;
		SListColumn* Column;

	};
	// Base list class
	class CUIList : public CUIComponentBase
	{
		public:
			inline CUIList(int id, int locId, POINT topLeft, SListConfig config) : CUIComponentBase(id, locId) {};

			// Shared methods
			POINT GetPosition();
			void SetPosition(POINT topLeft);
			virtual void RenderContent(CDC* dc, Graphics* g, CRadarScreen* screen) = 0;
			virtual void RegisterElements() = 0;

			// Fields
			bool IsCollapsed;
			SListConfig Config;

		protected:
			// Called in RenderContent
			virtual void RenderBase(CDC* dc, Graphics* g, CRadarScreen* screen);
			// Fields
			string Label_;
			POINT TopLeft_;
			vector<SListColumn> Columns_;
			map<int, map<int, SListElement>> RowMatrix_; // key is the row ID, the second map comprises the elements of the row
			int Width_; // determined from sum of column width
			int Height_; // determined from sum of rows * row heights
	};
}