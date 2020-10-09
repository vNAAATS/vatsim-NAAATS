#include "pch.h"
#include "FlightPlanWindow.h"
#include "Constants.h"
#include "Styles.h"

using namespace Colours;

CFlightPlanWindow::CFlightPlanWindow(POINT topLeft) : CBaseWindow(topLeft) {
	// Make buttons
	MakeButtons();
};


void CFlightPlanWindow::MakeButtons() {
	WindowButtons[WINBTN_CLOSE] = make_pair("Close", false);
	WindowButtons[WINBTN_FLTPLN_COPY] = make_pair("Copy", false);
	WindowButtons[WINBTN_FLTPLN_COORD] = make_pair("Co-ord", false);
	WindowButtons[WINBTN_FLTPLN_VOXCLEAR] = make_pair("Vox Clrc", false);
	WindowButtons[WINBTN_FLTPLN_PROBE] = make_pair("Probe", false);
	WindowButtons[WINBTN_FLTPLN_DELETE] = make_pair("Delete", false);
	WindowButtons[WINBTN_FLTPLN_ADS] = make_pair("ADS", false);
	WindowButtons[WINBTN_FLTPLN_MSG] = make_pair("Message", false);
	WindowButtons[WINBTN_FLTPLN_HIST] = make_pair("History", false);
	WindowButtons[WINBTN_FLTPLN_SAVE] = make_pair("Save", false);
}

void CFlightPlanWindow::RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Save device context
	int iDC = dc->SaveDC();

	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create base window rectangle
	CRect windowRect(topLeft.x, topLeft.y, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT);
	dc->FillRect(windowRect, &darkerBrush);

	// Create titlebar
	CRect titleRect(windowRect.left, windowRect.top, windowRect.left + WINSZ_FLTPLN_WIDTH, windowRect.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("Flight Plan - BAW123").c_str());

	// Create button bar
	CRect buttonBarRect(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT - 1, windowRect.left + WINSZ_FLTPLN_WIDTH, windowRect.top + 101);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, ScreenBlue.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, ScreenBlue.ToCOLORREF(), BevelDark.ToCOLORREF());

	/// Draw top menu buttons
	int offsetX = 30;
	int offsetY = 6;
	// Close button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_CLOSE).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_CLOSE).second, WINBTN_CLOSE, "FLTPLN");
	offsetX += 100;
	// Copy button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_COPY).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_COPY).second, WINBTN_FLTPLN_COPY, "FLTPLN");
	offsetX += 100;
	// Coord button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_COORD).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_COORD).second, WINBTN_FLTPLN_COORD, "FLTPLN");
	offsetX += 100;
	// Vox clearance button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_VOXCLEAR).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_VOXCLEAR).second, WINBTN_FLTPLN_VOXCLEAR, "FLTPLN");
	offsetX += 100;
	// Probe button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_PROBE).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_PROBE).second, WINBTN_FLTPLN_PROBE, "FLTPLN");
	offsetX = 30;
	offsetY += 40;
	// Close button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_DELETE).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_DELETE).second, WINBTN_FLTPLN_DELETE, "FLTPLN");
	offsetX += 100;
	// Copy button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_ADS).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_ADS).second, WINBTN_FLTPLN_ADS, "FLTPLN");
	offsetX += 100;
	// Coord button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_MSG).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_MSG).second, WINBTN_FLTPLN_MSG, "FLTPLN");
	offsetX += 100;
	// Vox clearance button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_HIST).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_HIST).second, WINBTN_FLTPLN_HIST, "FLTPLN");
	offsetX += 100;
	// Probe button
	DrawButton(dc, screen, WindowButtons.at(WINBTN_FLTPLN_SAVE).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 85, 30, 6, WindowButtons.at(WINBTN_FLTPLN_SAVE).second, WINBTN_FLTPLN_SAVE, "FLTPLN");


	// Create borders
	dc->DrawEdge(windowRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(windowRect, 1, 1);
	dc->Draw3dRect(windowRect, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(windowRect, 1, 1);
	dc->DrawEdge(windowRect, EDGE_RAISED, BF_RECT);

	// Add screen object
	screen->AddScreenObject(WINDOW, "FLTPLN", titleRect, true, "");

	// Cleanup
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);

	// Restore device context
	dc->RestoreDC(iDC);
}