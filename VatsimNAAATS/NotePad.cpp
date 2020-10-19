#include "pch.h"
#include "NotePad.h"
#include "Constants.h"
#include "Styles.h"

using namespace EuroScopePlugIn;
using namespace Gdiplus;
using namespace Colours;

CNotePad::CNotePad(POINT topLeft, POINT topLeftList) : CBaseWindow(topLeft), CBaseList(topLeftList) {
	// Close by default
	IsClosed = true;
}

void CNotePad::RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {
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
	CRect windowRect(topLeft.x, topLeft.y, topLeft.x + WINSZ_NP_WIDTH, topLeft.y + WINSZ_NP_HEIGHT);
	dc->FillRect(windowRect, &darkerBrush);

	// Create titlebar
	CRect titleRect(windowRect.left, windowRect.top, windowRect.left + WINSZ_NP_WIDTH, windowRect.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_NP_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), "Note Pad");

	// Add screen objects
	screen->AddScreenObject(WINDOW, "WIN_NP", windowRect, false, ""); // So that we can't click anything under the flight plan window
	screen->AddScreenObject(WINDOW, "NOTEPAD", titleRect, true, ""); // Movable

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

void CNotePad::RenderList(Graphics* g, CDC* dc, CRadarScreen* screen) {

}

void CNotePad::MakeWindowItems() {

}

void CNotePad::ButtonDown(int id) {

}

void CNotePad::ButtonUp(int id) {

}

void CNotePad::ButtonPress(int id) {

}

void CNotePad::ButtonUnpress(int id) {

}

void CNotePad::SetButtonState(int id, CInputState state) {

}

void CNotePad::Scroll(CRect area, POINT mousePtr) {

}