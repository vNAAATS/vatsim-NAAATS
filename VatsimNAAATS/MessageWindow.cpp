#include "pch.h"
#include "MessageWindow.h"
#include "Styles.h"
#include "Constants.h"
#include "RoutesHelper.h"
#include "DataHandler.h"

using namespace Colours;

const int CMessageWindow::BTN_CLOSE = 2;

CMessageWindow::CMessageWindow(POINT topLeft) : CBaseWindow(topLeft) {
	// Make buttons
	MakeWindowItems();

	// Close by default
	IsClosed = true;
};

void CMessageWindow::MakeWindowItems() {
	windowButtons[BTN_CLOSE] = CWinButton(BTN_CLOSE, WIN_MSG, "Close", CInputState::INACTIVE);
}

void CMessageWindow::RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Save device context
	int iDC = dc->SaveDC();

	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());
	CBrush evenDarkerBrush(ButtonPressed.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create base window rectangle
	CRect windowRect(topLeft.x, topLeft.y, topLeft.x + WINSZ_MSG_WIDTH, topLeft.y + WINSZ_MSG_HEIGHT);
	dc->FillRect(windowRect, &darkerBrush);

	// Create titlebar
	CRect titleRect(windowRect.left, windowRect.top, windowRect.left + WINSZ_MSG_WIDTH, windowRect.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_MSG_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), "Messages");

	// Add screen objects
	screen->AddScreenObject(WINDOW, "WIN_MSG", windowRect, true, ""); // So that we can't click anything under the flight plan window
	screen->AddScreenObject(WINDOW, "MSG", titleRect, true, ""); // Movable


	// Create borders
	dc->DrawEdge(windowRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(windowRect, 1, 1);
	dc->Draw3dRect(windowRect, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(windowRect, 1, 1);
	dc->DrawEdge(windowRect, EDGE_RAISED, BF_RECT);

	// Cleanup
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);

	// Restore device context
	dc->RestoreDC(iDC);
}

void CMessageWindow::ButtonUp(int id) {
	if (id == CMessageWindow::BTN_CLOSE) { // Close button
		// If the close button close window
		IsClosed = true;
	}
}

void CMessageWindow::ButtonDown(int id) {

}

void CMessageWindow::ButtonPress(int id) {

}

void CMessageWindow::ButtonUnpress(int id) {

}

void CMessageWindow::SetButtonState(int id, CInputState state) {

}