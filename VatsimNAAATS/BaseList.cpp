#include "pch.h"
#include "BaseList.h"

CBaseList::CBaseList(POINT topLeft) {
	// Set top left
	topleft = topLeft;
}

POINT CBaseList::GetTopLeft() {
	return topleft;
}

void CBaseList::MoveList(CRect area) {
	topleft = { area.left, area.top };
}

void CBaseList::MoveList(POINT topLeft) {
	topleft = { topLeft.x, topLeft.y };
}