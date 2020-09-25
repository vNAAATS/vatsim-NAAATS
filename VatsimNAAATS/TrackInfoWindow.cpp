#include "pch.h"
#include "TrackInfoWindow.h"
#include "Styles.h"
#include "Constants.h"
#include "Overlays.h"
#include "DataHandler.h"

using namespace Colours;

CTrackInfoWindow::CTrackInfoWindow(POINT topLeftPt) {
	topLeft = topLeftPt;

	WindowButtons[WINBTN_TCKINFO_REFRESH] = make_pair("Refresh NAT Data", false);
	WindowButtons[WINBTN_CLOSE] = make_pair("Close", false);
}

POINT CTrackInfoWindow::GetTopLeft() {
	return topLeft;
}

void CTrackInfoWindow::RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {
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
	CRect windowRect(topLeft.x, topLeft.y, topLeft.x + WINSZ_TCKINFO_WIDTH, topLeft.y + WINSZ_TCKINFO_HEIGHT);
	dc->FillRect(windowRect, &darkerBrush);
	
	// Create titlebar
	CRect titleRect(windowRect.left, windowRect.top, windowRect.left + WINSZ_TCKINFO_WIDTH, windowRect.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_TCKINFO_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("Track Info - TMI: " + COverlays::CurrentTMI).c_str());

	// Create button bar
	CRect buttonBarRect(windowRect.left, windowRect.bottom - 50, windowRect.left + WINSZ_TCKINFO_WIDTH, windowRect.bottom);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->DrawEdge(buttonBarRect, EDGE_RAISED, BF_TOP);
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());

	/// Draw buttons
	// Refresh button
	DrawButton(dc, screen, "Refresh NAT Data", { buttonBarRect.left + 10, buttonBarRect.top + 10 }, 140, 30, 6, WindowButtons.at(WINBTN_TCKINFO_REFRESH).second, WINBTN_TCKINFO_REFRESH, "TCKINFO");
	// Close button
	DrawButton(dc, screen, "Close", { (buttonBarRect.right - 60) - 10, buttonBarRect.top + 10 }, 55, 30, 6, WindowButtons.at(WINBTN_CLOSE).second, WINBTN_CLOSE, "TCKINFO");

	// Draw lines
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Refresh NAT data if clicked
	if (NATDataRefresh) {
		int status = CDataHandler::PopulateLatestTrackData(screen->GetPlugIn());
		// Show data 
		if (status == 0) {
			MsgDataRefresh = "Refresh successful.";
		}
		else {
			MsgDataRefresh = "Failed to refresh.";
		}
		NATDataRefresh = false;
	}

	// Show refresh message 
	dc->TextOutA(((windowRect.right + windowRect.left) / 2) + 10, buttonBarRect.top + 16, MsgDataRefresh.c_str());

	// Set offsets for line drawing
	int offsetX = 25;
	int offsetY = 25;
	// TODO: implement scroll
	// Draw lines
	for (auto kv : COverlays::CurrentTracks) {
		dc->TextOutA(windowRect.left + offsetX, windowRect.top + offsetY, "TCK");
		offsetX += dc->GetTextExtent("TCK").cx + 35;
		// Output route
		for (int i = 0; i < kv.second.Route.size(); i++) {
			dc->TextOutA(windowRect.left + offsetX, windowRect.top + offsetY, kv.second.Route[i].c_str());
			offsetX += (int)dc->GetTextExtent(kv.second.Route[i].c_str()).cx + 5;
		}
		// Reset offsets
		offsetY += 20;
		offsetX = 24;
		// Output track ID
		dc->TextOutA(windowRect.left + offsetX, windowRect.top + offsetY, kv.first.c_str());
		offsetX += dc->GetTextExtent(kv.first.c_str()).cx + 43;
		// Output flight levels
		for (int i = 0; i < kv.second.FlightLevels.size(); i++) {
			dc->TextOutA(windowRect.left + offsetX, windowRect.top + offsetY, to_string(kv.second.FlightLevels[i] / 100).c_str());
			offsetX += dc->GetTextExtent(to_string(kv.second.FlightLevels[i] / 100).c_str()).cx + 5;
		}

		// Reset x offset and increment y offset
		offsetX = 25;
		offsetY += 25;
	}

	// Create borders
	dc->DrawEdge(windowRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(windowRect, 1, 1);
	dc->Draw3dRect(windowRect, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(windowRect, 1, 1);
	dc->DrawEdge(windowRect, EDGE_RAISED, BF_RECT);

	// Add screen object
	screen->AddScreenObject(WINDOW, "TCKINFO", titleRect, true, "");

	// Cleanup
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);

	// Restore device context
	dc->RestoreDC(iDC);
}


void CTrackInfoWindow::MoveWindow(CRect area) {
	topLeft = { area.left, area.top };
}

CRect CTrackInfoWindow::DrawButton(CDC* dc, CRadarScreen* screen, string text, POINT topLeft, int width, int height, int vtcAlign, bool isPressed, int type, string id)
{
	// Save context for later
	int sDC = dc->SaveDC();

	// Brushes
	CBrush btnNormal(ScreenBlue.ToCOLORREF());

	CBrush btnPressed(ButtonPressed.ToCOLORREF());

	// Create rectangle
	CRect button(topLeft.x, topLeft.y, topLeft.x + width, topLeft.y + height);

	// Check if pressed
	if (isPressed) {
		dc->FillSolidRect(button, ButtonPressed.ToCOLORREF());
		// Button bevel
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		InflateRect(button, -1, -1);
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	}
	else {
		dc->FillSolidRect(button, ScreenBlue.ToCOLORREF());
		// Button bevel
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
		InflateRect(button, -1, -1);
		dc->Draw3dRect(button, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	}

	// Draw text
	FontSelector::SelectNormalFont(MEN_FONT_SIZE, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(button.left + (button.Width() / 2), button.top + vtcAlign, text.c_str());

	// Restore device context
	dc->RestoreDC(sDC);

	// Delete objects
	DeleteObject(&btnNormal);
	DeleteObject(&btnPressed);

	// Add object and return the rectangle
	screen->AddScreenObject(type, id.c_str(), button, false, "");
	return button;
}