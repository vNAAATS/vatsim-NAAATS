#include "pch.h"
#include "BaseWindow.h"
#include "Styles.h"

using namespace Colours;

CBaseWindow::CBaseWindow(POINT topLeft) {
	// Set top left
	topLeft = topLeft;
}

POINT CBaseWindow::GetTopLeft() {
	return topLeft;
}

void CBaseWindow::MoveWindow(CRect area) {
	topLeft = { area.left, area.top };
}

bool CBaseWindow::IsButton(int id) {
	// If exists return true
	if (WindowButtons.find(id) != WindowButtons.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsTextInput(int id) {
	// If exists return true
	if (TextInputs.find(id) != TextInputs.end()) return true;
	return false; // It doesn't exist
}

bool CBaseWindow::IsCheckBox(int id) {
	// If exists return true
	if (CheckBoxes.find(id) != CheckBoxes.end()) return true;
	return false; // It doesn't exist
}