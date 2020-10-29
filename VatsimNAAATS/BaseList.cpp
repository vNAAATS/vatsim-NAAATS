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
	// Only set Y if it is greater than the top position of the menu bar (to stop clipping over the menu bar)
	topleft = { area.left, area.top > MENBAR_HEIGHT ? area.top : MENBAR_HEIGHT };
}

void CBaseList::MoveList(POINT topLeft) {
	// Only set Y if it is greater than the top position of the menu bar (to stop clipping over the menu bar)
	topleft = { topLeft.x, topLeft.y > MENBAR_HEIGHT ? topLeft.y : MENBAR_HEIGHT };
}