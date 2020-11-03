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
	CMessage msg;
	msg.Id = 0;
	msg.From = "DLH414";
	msg.To = "CZQX_FSS";
	msg.MessageRaw = "AAL578:CLR_REQ:ARR:KMCO:DCT:ENTRY:TEST:EST:1000Z:TRACK:RR:F410:M81:END_OF_MESSAGE";
	msg.Type = CMessageType::CLEARANCE_REQ;
	ActiveMessages[msg.Id] = msg;

	msg.Id = 1;
	msg.From = "DLH414";
	msg.To = "CZQX_FSS";
	msg.MessageRaw = "AAL578:LOG_ON:CZQX_FSS";
	msg.Type = CMessageType::LOG_ON;
	ActiveMessages[msg.Id] = msg;
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
	dc->TextOutA(titleRect.left + (WINSZ_MSG_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), ("Messages (" + to_string(ActiveMessages.size()) + ")").c_str());

	// Create button bar
	CRect buttonBarRect(windowRect.left, windowRect.bottom - 50, windowRect.left + WINSZ_MSG_WIDTH, windowRect.bottom);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());	

	// Add screen objects
	screen->AddScreenObject(WINDOW, "WIN_MSG", windowRect, true, ""); // So that we can't click anything under the flight plan window
	screen->AddScreenObject(WINDOW, "MSG", titleRect, true, ""); // Movable

	// Close button
	CCommonRenders::RenderButton(dc, screen, { buttonBarRect.left + 10, buttonBarRect.top + 10 }, 55, 30, &windowButtons.at(BTN_CLOSE));

	// Font
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Drawing messages
	int offsetX = 5;
	int offsetY = 2;
	for (int i = 0; i < ActiveMessages.size(); i++) {
		CRect message(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT + offsetY, windowRect.right, 0); // Set to 0 here
		offsetY += dc->GetTextExtent("ABC").cy + 2;

		// Set messages rectangle bottom
		message.bottom = windowRect.top + WINSZ_TITLEBAR_HEIGHT + offsetY + 2;

		if (ActiveMessages[i].Id == SelectedMessage) {
			dc->FillRect(message, &evenDarkerBrush);
		}

		// Text
		dc->TextOutA(message.left + offsetX, message.top + 2, ActiveMessages[i].From.c_str());
		offsetX += 60;
		dc->TextOutA(message.left + offsetX, message.top + 2, ActiveMessages[i].MessageRaw.c_str());

		// Add screen object for message
		screen->AddScreenObject(ACTV_MESSAGE, to_string(ActiveMessages[i].Id).c_str(), message, false, "");

		// Reset X offset
		offsetX = 5;
	}

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
	// Press button
	windowButtons.find(id)->second.State = CInputState::INACTIVE;
	SelectedMessage = -1;
}

void CMessageWindow::ButtonDown(int id) {
	// Press button
	windowButtons.find(id)->second.State = CInputState::ACTIVE;
}

void CMessageWindow::ButtonPress(int id) {
	
}

void CMessageWindow::ButtonDoubleClick(CRadarScreen* screen, int id, CFlightPlanWindow* fltPlnWin) {
	CMessage* msg = &ActiveMessages[id];

	if (msg->Type == CMessageType::LOG_ON) {
		fltPlnWin->Instantiate(screen, msg->From);
		fltPlnWin->IsOpen = true;
		fltPlnWin->IsTransferOpen = true;
		fltPlnWin->IsData = false;
		screen->GetPlugIn()->SetASELAircraft(screen->GetPlugIn()->RadarTargetSelect(msg->From.c_str()));
	}
	else if (msg->Type == CMessageType::CLEARANCE_REQ) {
		fltPlnWin->Instantiate(screen, msg->From);
		// Fix parsing here
		fltPlnWin->IsOpen = true;
		fltPlnWin->IsData = true;
		screen->GetPlugIn()->SetASELAircraft(screen->GetPlugIn()->RadarTargetSelect(msg->From.c_str()));
	}
}

void CMessageWindow::ButtonUnpress(int id) {

}

void CMessageWindow::SetButtonState(int id, CInputState state) {

}