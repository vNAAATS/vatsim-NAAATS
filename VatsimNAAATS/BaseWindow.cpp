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

bool CBaseWindow::IsButton(int id) {
	// If exists return true
	if (windowButtons.find(id) != windowButtons.end()) return true;
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