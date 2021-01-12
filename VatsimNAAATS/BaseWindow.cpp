#include "pch.h"
#include "BaseWindow.h"
#include "Styles.h"

using namespace Colours;

CBaseWindow::CBaseWindow(POINT topleft) {
	// Set top left
	topLeft = topleft;
}

POINT CBaseWindow::GetTopLeft() {
	return topLeft;
}

void CBaseWindow::MoveWindow(CRect area) {
	topLeft = { area.left, area.top };
}

// TODO: Rewrite scrolling mechanic... again
void CBaseWindow::Scroll(int id, POINT newPtr, POINT oldPtr) {
	// Get mouse delta
	double posDelta = 0.0;
	if (scrollBars[id].IsHorizontal) {
		posDelta = newPtr.x - oldPtr.x;
	}
	else {
		posDelta = newPtr.y - oldPtr.y;
	}

	// Get new grip position
	scrollBars[id].PositionDelta += posDelta;

	// Limit grip position
	if (scrollBars[id].PositionDelta < 0) {
		scrollBars[id].PositionDelta = 0;
	}
	if (scrollBars[id].PositionDelta > (scrollBars[id].FrameSize - 33 - scrollBars[id].GripSize)) {
		scrollBars[id].PositionDelta = scrollBars[id].FrameSize - 33 - scrollBars[id].GripSize;
	}

	// Get window position
	scrollBars[id].WindowPos = (scrollBars[id].PositionDelta / (scrollBars[id].FrameSize - 12 - scrollBars[id].GripSize)) * scrollBars[id].TotalScrollableArea;
}

string CBaseWindow::GetTextValue(int id) {
	if (textInputs.find(id) != textInputs.end()) {
		return textInputs.find(id)->second.Content;
	}
	else {
		return "";
	}
}

bool CBaseWindow::IsButton(int id) {
	// If exists return true
	if (windowButtons.find(id) != windowButtons.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsDropDown(int id) {
	// If exists return true
	if (dropDowns.find(id) != dropDowns.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsTextInput(int id) {
	// If exists return true
	if (textInputs.find(id) != textInputs.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsCheckBox(int id) {
	// If exists return true
	if (checkBoxes.find(id) != checkBoxes.end()) return true;
	return false; // It doesn't exist
}