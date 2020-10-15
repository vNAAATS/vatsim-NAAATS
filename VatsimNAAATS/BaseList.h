#pragma once
#include "pch.h"
#include <string>
#include <map>
#include <vector>
#include <gdiplus.h>
#include "Structures.h"
#include <EuroScopePlugIn.h>

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class CBaseList
{
	public: 
		CBaseList(POINT topLeft);
		virtual ~CBaseList() {};
		POINT GetTopLeft();
		void MoveList(CRect area); // By area
		void MoveList(POINT topLeft); // By top left point
		virtual void RenderList(Graphics* g, CDC* dc, CRadarScreen* screen) = 0;

	protected:
		// Top left point of list
		POINT topLeft;
};

