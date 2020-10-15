#include "pch.h"
#include "BaseList.h"

CBaseList::CBaseList(POINT topLeft) {
	// Set top left
	topLeft = topLeft;
}

POINT CBaseList::GetTopLeft() {
	return topLeft;
}

void CBaseList::MoveList(CRect area) {
	topLeft = { area.left, area.top };
}

void CBaseList::MoveList(POINT topleft) {
	topLeft = { topleft.x, topleft.y };
}