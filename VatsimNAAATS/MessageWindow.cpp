#include "pch.h"
#include "MessageWindow.h"
#include "Styles.h"
#include "Constants.h"
#include "RoutesHelper.h"
#include "DataHandler.h"

using namespace Colours;

const int CMessageWindow::BTN_CLOSE = 2;
unordered_map<int, CMessage*> CMessageWindow::OngoingMessages;
unordered_map<int, CMessage> CMessageWindow::ActiveMessages;

CMessageWindow::CMessageWindow(POINT topLeft) : CBaseWindow(topLeft) {
	// Make buttons
	MakeWindowItems();

	// Close by default
	IsClosed = true;
};

void CMessageWindow::MakeWindowItems() {
	windowButtons[BTN_CLOSE] = CWinButton(BTN_CLOSE, WIN_MSG, "Close", CInputState::INACTIVE);
	scrollBars[SCRL_MSGWNDW] = CWinScrollBar(SCRL_MSGWNDW, WIN_SCROLLBAR, 0, 0, false);
	CMessage msg;
	msg.Id = 0;
	msg.From = "DLH414";
	msg.To = "CZQX_FSS";
	msg.MessageRaw = "DLH414:CLEARANCE_REQUEST:KMCO:NULL:NEEKO:1000:A:410:81:FREETEXT";
	msg.Type = CMessageType::CLEARANCE_REQ;
	ActiveMessages[msg.Id] = msg;

	msg.Id = 1;
	msg.From = "DLH414";
	msg.To = "CZQX_FSS";
	msg.MessageRaw = "DLH414:LOG_ON:CZQX_FSS";
	msg.Type = CMessageType::LOG_ON;
	ActiveMessages[msg.Id] = msg;

	msg.Id = 2;
	msg.From = "DLH414";
	msg.To = "CZQX_FSS";
	msg.MessageRaw = "DLH414:REVISION_REQ:MCHG:85";
	msg.Type = CMessageType::REVISION_REQ;
	ActiveMessages[msg.Id] = msg;

	/*CMessage msg;

	for (int x = 0; x <= 5; x++)
	{
		msg.Id = x;
		msg.From = "VIR7" + std::to_string(x);
		msg.To = "CZQX_FSS";
		msg.MessageRaw = msg.From + std::string(":CLEARANCE_REQUEST:KMCO:NULL:NEEKO:1000:A:410:81:FREETEXT");
		msg.Type = CMessageType::CLEARANCE_REQ;
		ActiveMessages[msg.Id] = msg;


	}
	for (int x = 6; x <= 10; x++)
	{
		msg.Id = x;
		msg.From = "DLH41" + std::to_string(x);
		msg.To = "CZQX_FSS";
		msg.MessageRaw = msg.From + std::string(":LOG_ON:CZQX_FSS");
		msg.Type = CMessageType::LOG_ON;
		ActiveMessages[msg.Id] = msg;
	}*/
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
	int offsetY = 0;
	int contentOffsetY = 0;
	int totalcontent = 0;
	/*for (int i = 0; i < ActiveMessages.size(); i++) {
		// Skip displaying if the message is active
		//if (OngoingMessages.find(ActiveMessages[i].Id) != OngoingMessages.end()) {
		//	continue;
		//}
		totalcontent += dc->GetTextExtent("ABC").cy + 5;

		// Parse the message
		string parsed = CUtils::ParseToPhraseology(ActiveMessages[i].MessageRaw, ActiveMessages[i].Type);

		// Get the wrapped text
		vector<string> wrappedText;
		int contentSize = windowRect.Width() - (5 + 70) - 15;
		if (dc->GetTextExtent(parsed.c_str()).cx > contentSize) {
			CUtils::WrapText(dc, parsed, ' ', contentSize, &wrappedText);
		}

		totalcontent += (!wrappedText.empty() ? dc->GetTextExtent("ABCD").cy * wrappedText.size() + 15 : 5) + 5;

		int wrapOffsetY = 0;
		if (!wrappedText.empty()) {
			for (int i = 0; i < wrappedText.size(); i++) {
				wrapOffsetY += dc->GetTextExtent("ABCD").cy + 5;
			}
		}

		// Set messages rectangle bottom
		totalcontent += wrapOffsetY != 0 ? wrapOffsetY - (dc->GetTextExtent("ABC").cy + 2) : 0;
	}*/
	for (int i = 0; i < ActiveMessages.size(); i++) {
		totalcontent += 50;
	}
	for (int i = 0; i < ActiveMessages.size(); i++) {
		// Skip displaying if the message is active
		if (OngoingMessages.find(ActiveMessages[i].Id) != OngoingMessages.end()) {
			continue;
		}
		if (!ActiveMessages[i].MessageRaw.length()) continue;
		int deltaoffsetY = offsetY;
		if (!(contentOffsetY < scrollBars[SCRL_MSGWNDW].WindowPos))
		{
			if (!(contentOffsetY > scrollBars[SCRL_MSGWNDW].WindowPos + scrollBars[SCRL_MSGWNDW].FrameSize - 25))
			{
				CRect message(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT + offsetY, windowRect.right, 0); // Set to 0 here
				offsetY += dc->GetTextExtent("ABC").cy + 5;

				// Parse the message
				string parsed = CUtils::ParseToPhraseology(ActiveMessages[i].MessageRaw, ActiveMessages[i].Type, ActiveMessages[i].From);

				// Get the wrapped text
				vector<string> wrappedText;
				int contentSize = windowRect.Width() - (offsetX + 70) - 15;
				if (dc->GetTextExtent(parsed.c_str()).cx > contentSize) {
					CUtils::WrapText(dc, parsed, ' ', contentSize, &wrappedText);
				}

				// Set messages rectangle bottom
				message.bottom = windowRect.top + WINSZ_TITLEBAR_HEIGHT + (!wrappedText.empty() ? dc->GetTextExtent("ABCD").cy * wrappedText.size() + 15 + deltaoffsetY : offsetY) + 5;

				if (ActiveMessages[i].Id == SelectedMessage) {
					dc->FillRect(message, &evenDarkerBrush);
				}

				// Text 'from'
				dc->TextOutA(message.left + offsetX, message.top + 5, ActiveMessages[i].From.c_str());
				offsetX += 70;

				// If the text is wrapped
				int wrapOffsetY = 0;
				if (!wrappedText.empty()) {
					// Iterate to display
					wrapOffsetY = 5;
					for (int i = 0; i < wrappedText.size(); i++) {
						// Write the message
						dc->TextOutA(message.left + offsetX, message.top + wrapOffsetY, wrappedText[i].c_str());
						wrapOffsetY += dc->GetTextExtent("ABCD").cy + 5;
					}
				}
				else {
					// Write without iterating
					dc->TextOutA(message.left + offsetX, message.top + 5, parsed.c_str());
				}

				// Add screen object for message
				screen->AddScreenObject(ACTV_MESSAGE, to_string(ActiveMessages[i].Id).c_str(), message, false, "");

				// Reset X offset
				offsetX = 5;

				// Add wrapped text size to y offset
				offsetY += wrapOffsetY != 0 ? wrapOffsetY - (dc->GetTextExtent("ABC").cy + 2) : 0;
			}
		}
		//contentOffsetY += 50;
		//contentOffsetY += (offsetY - deltaoffsetY);
		contentOffsetY += (offsetY - deltaoffsetY) == 0 ? 50 : (offsetY - deltaoffsetY);
	}

	// Scroll bar values
	if (scrollBars[SCRL_MSGWNDW].FrameSize == 0 || (scrollBars[SCRL_MSGWNDW].ContentSize != contentOffsetY && scrollBars[SCRL_MSGWNDW].PositionDelta == 0 && scrollBars[SCRL_MSGWNDW].ContentRatio != 1))
	{
		int framebox = windowRect.Height() - 65;
		if (contentOffsetY == 0)
			contentOffsetY = framebox; // ratio 1:1
		scrollBars[SCRL_MSGWNDW] = CWinScrollBar(SCRL_MSGWNDW, WIN_SCROLLBAR, contentOffsetY, framebox, false);
		if (contentOffsetY != framebox)
		{
			scrollBars[SCRL_MSGWNDW].GripSize -= 30; // temporary fix for grip bug
			scrollBars[SCRL_MSGWNDW].TotalScrollableArea = framebox - scrollBars[SCRL_MSGWNDW].GripSize;
		}
	}

	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { windowRect.right - 12, windowRect.top + 20 }, &scrollBars[SCRL_MSGWNDW]);

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

void CMessageWindow::ButtonUp(int id, CRadarScreen* screen) {
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
	// Message pointer
	CMessage* msg = &ActiveMessages[id];

	// Get rid of selected message so if requeued the click highlight doesn't reappear
	SelectedMessage = -1;

	if (!screen->GetPlugIn()->FlightPlanSelect(msg->From.c_str()).IsValid()) {
		ActiveMessages.erase(id);
		return;
	}
	if (msg->Type == CMessageType::LOG_ON || msg->Type == CMessageType::TRANSFER) {
		fltPlnWin->Instantiate(screen, msg->From, msg);
		fltPlnWin->IsOpen = true;
		fltPlnWin->IsTransferOpen = true;
		screen->GetPlugIn()->SetASELAircraft(screen->GetPlugIn()->FlightPlanSelect(msg->From.c_str()));
		OngoingMessages[msg->Id] = msg;
	}
	else if (msg->Type == CMessageType::TRANSFER_ACCEPT ||
		msg->Type == CMessageType::TRANSFER_ACCEPT ||
		msg->Type == CMessageType::WILCO ||
		msg->Type == CMessageType::UNABLE ||
		msg->Type == CMessageType::ROGER) {
		ActiveMessages.erase(id);
	}
	else if (msg->Type == CMessageType::CLEARANCE_REQ) {
		/// Parse the message
		// Split string
		vector<string> tokens;
		CUtils::StringSplit(msg->MessageRaw, ':', &tokens);

		// Values
		string arrival;
		string track;
		string route = "";
		string ep;
		string epEst;
		string level;
		string speed;
		// Loop tokens
		arrival = tokens[2];
		route = tokens[3] != "NULL" && tokens[6] == "RR" ? tokens[3] : "";
		track = tokens[6] != "RR" ? tokens[6] : "RR";
		ep = tokens[4];
		epEst = tokens[5];
		level = tokens[7];
		speed = tokens[8];
		// Instantiate flight plan window
		fltPlnWin->Instantiate(screen, msg->From, msg);
		fltPlnWin->IsOpen = true;
		fltPlnWin->IsData = true;
		fltPlnWin->SetButtonState(CFlightPlanWindow::BTN_PROBE, CInputState::INACTIVE);
		fltPlnWin->primedPlan->Mach = speed;
		fltPlnWin->primedPlan->Track = track;
		fltPlnWin->primedPlan->FlightLevel = level;
		fltPlnWin->primedPlan->Dest = arrival;
		fltPlnWin->primedPlan->State = "UA";

		// Instantiate flight plan variables
		fltPlnWin->SetTextValue(screen, CFlightPlanWindow::TXT_SPD, speed);
		fltPlnWin->SetTextValue(screen, CFlightPlanWindow::TXT_LEVEL, level);
		fltPlnWin->SetTextValue(screen, CFlightPlanWindow::TXT_DEST, arrival);
		fltPlnWin->SetTextValue(screen, CFlightPlanWindow::TXT_STATE, "UA");
		if (track != "RR") {
			fltPlnWin->SetTextValue(screen, CFlightPlanWindow::TXT_TCK, track);
		}
		else {
			fltPlnWin->SetTextValue(screen, CFlightPlanWindow::TXT_RTE, route);
		}
		screen->GetPlugIn()->SetASELAircraft(screen->GetPlugIn()->FlightPlanSelect(msg->From.c_str()));
		OngoingMessages[msg->Id] = msg;
	}
	else if (msg->Type == CMessageType::REVISION_REQ) {
		// Instantiate flight plan window
		fltPlnWin->Instantiate(screen, msg->From, msg);
	}
}

void CMessageWindow::ButtonUnpress(int id) {

}

void CMessageWindow::SetButtonState(int id, CInputState state) {

}