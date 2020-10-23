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

	// Draw lines
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Get a rectangle for the content
	int contentSize = CRoutesHelper::CurrentTracks.size() * 45; // We minus 25 because 25 extra is always added on at the end of the loop
	CRect scrollContent(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT, windowRect.right, windowRect.top + WINSZ_TITLEBAR_HEIGHT + contentSize);
	/// Scroll bar mechanics
	scrollWindowSize = WINSZ_MSG_HEIGHT - 3 - (titleRect.Height() + 1); // Size of the window (which is also the size of the track for the scroll grip)
	double contentRatio = (double)scrollWindowSize / (double)contentSize; // Ratio of content to window
	gripSize = scrollWindowSize * contentRatio; // Based on the ratio, we get the size of the grip
	int minGripSize = 20; // Minimum size in case the window is big
	// Check that the grip size is not greater than the window size (content is smaller)
	if (gripSize > scrollWindowSize) gripSize = scrollWindowSize; // Set the grip size to the window size if it is
	if (gripSize < minGripSize) gripSize = minGripSize; // Make sure the minimum grip size is there
	// Grip position mechanics
	int winScrollAreaSize = contentSize - scrollWindowSize; // Total scrollable area
	double windowPosRatio = (double)scrollWindowSize / (double)winScrollAreaSize; // Ratio of window to scrollable area
	trackScrollAreaSize = scrollWindowSize - gripSize; // Same as window, do to keep grip flying off end of track
	double gripPosOnTrack = trackScrollAreaSize * windowPosRatio + gripPosDelta - ((gripSize != scrollWindowSize) ? gripSize : 0); // This is the location of the grip on the track

	// Now we draw the scroll track
	CRect scrollBarTrack(windowRect.right - 13, windowRect.top + titleRect.Height(), windowRect.right, windowRect.bottom - 2);
	dc->FillRect(scrollBarTrack, &evenDarkerBrush);
	dc->Draw3dRect(scrollBarTrack, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(scrollBarTrack, -1, -1);
	dc->Draw3dRect(scrollBarTrack, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// And then the actual scroll grip
	CRect scrollGrip(scrollBarTrack.left, scrollBarTrack.top + gripPosOnTrack, scrollBarTrack.right, scrollBarTrack.top + gripPosOnTrack + gripSize);
	dc->FillRect(scrollGrip, &darkerBrush);
	dc->Draw3dRect(scrollGrip, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(scrollGrip, -1, -1);
	dc->Draw3dRect(scrollGrip, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Add the screen object
	screen->AddScreenObject(WIN_SCROLLBAR, "MSGWIN", scrollGrip, true, "");

	// Now we need to get the clipped scroll area
	double gripPositionRatio = gripPosOnTrack != 0.0 ? (gripPosOnTrack + gripSize) / trackScrollAreaSize : 0;
	double windowPosition = gripPosOnTrack != 0.0 ? gripPositionRatio * winScrollAreaSize - scrollWindowSize : 0;
	CRect clipContent(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT + windowPosition, windowRect.right, windowRect.top + WINSZ_TITLEBAR_HEIGHT + scrollWindowSize + windowPosition);
	// Set offsets for line drawing and spacer size
	int offsetX = 25;
	int offsetY = 25;
	int contentOffsetY = 25;
	string spacer = "SPACE"; // To use GetTextExtent() for a consistent sized spacer
	// Draw lines
	/*for (auto kv : COverlays::CurrentTracks) {
		int content = (int)scrollContent.top + contentOffsetY;
		if (windowRect.top + contentOffsetY >= clipContent.top && windowRect.top + contentOffsetY <= clipContent.bottom) {
			dc->TextOutA(windowRect.left + offsetX, windowRect.top + offsetY, "TCK");
			offsetX += dc->GetTextExtent("TCK").cx + 37;
			// Output route
			for (int i = 0; i < kv.second.Route.size(); i++) {
				dc->TextOutA(windowRect.left + offsetX, windowRect.top + offsetY, kv.second.Route[i].c_str());
				offsetX += (int)dc->GetTextExtent(spacer.c_str()).cx;
			}
			// Reset offsets
			offsetX = 24;
			offsetY += 20;
		}
		contentOffsetY += 20;
		if (windowRect.top + contentOffsetY >= clipContent.top && windowRect.top + contentOffsetY <= clipContent.bottom) {
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
		contentOffsetY += 25;
	}*/

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

void CMessageWindow::Scroll(CRect area, POINT mousePtr) {
	// Current bar position
	if (currentScrollPos.left == 0) { // If initial then just set to the area
		currentScrollPos = area;
	}
	else {
		// Get the delta and check whether it is in the bounds, if it is then change position
		int delta = gripPosDelta + area.top - currentScrollPos.top;
		if (delta > scrollWindowSize - gripSize) {
			gripPosDelta = trackScrollAreaSize;
		}
		else if (delta < 0) {
			gripPosDelta = 0;
		}
		else {
			gripPosDelta += area.top - currentScrollPos.top;
		}
		currentScrollPos = area;
	}
}