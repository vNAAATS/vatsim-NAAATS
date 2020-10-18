#include "pch.h"
#include "FlightPlanWindow.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"

using namespace Colours;

CFlightPlanWindow::CFlightPlanWindow(POINT topLeft) : CBaseWindow(topLeft) {
	// Make buttons
	MakeWindowItems();

	// Close by default
	IsClosed = true;
};

void CFlightPlanWindow::MakeWindowItems() {
	// Button defaults
	windowButtons[BTN_CLOSE] = CWinButton(BTN_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_COPY] = CWinButton(BTN_COPY, WIN_FLTPLN, "Copy", CInputState::INACTIVE);
	windowButtons[BTN_UNCLEAR] = CWinButton(BTN_UNCLEAR, WIN_FLTPLN, "Unclear", CInputState::DISABLED);
	windowButtons[BTN_COORD] = CWinButton(BTN_COORD, WIN_FLTPLN, "Co-ord", CInputState::DISABLED);
	windowButtons[BTN_MANENTRY] = CWinButton(BTN_MANENTRY, WIN_FLTPLN, "ManEntry", CInputState::INACTIVE);
	windowButtons[BTN_PROBE] = CWinButton(BTN_PROBE, WIN_FLTPLN, "Probe", CInputState::INACTIVE);
	windowButtons[BTN_DELETE] = CWinButton(BTN_DELETE, WIN_FLTPLN, "Delete", CInputState::DISABLED);
	windowButtons[BTN_ADS] = CWinButton(BTN_ADS, WIN_FLTPLN, "ADS", CInputState::DISABLED);
	windowButtons[BTN_READBK] = CWinButton(BTN_READBK, WIN_FLTPLN, "ReadBK", CInputState::DISABLED);
	windowButtons[BTN_MSG] = CWinButton(BTN_MSG, WIN_FLTPLN, "Message", CInputState::INACTIVE);
	windowButtons[BTN_HIST] = CWinButton(BTN_HIST, WIN_FLTPLN, "History", CInputState::INACTIVE);
	windowButtons[BTN_SAVE] = CWinButton(BTN_SAVE, WIN_FLTPLN, "Save", CInputState::INACTIVE);
	windowButtons[BTN_ATCR] = CWinButton(BTN_ATCR, WIN_FLTPLN, "ATC/", CInputState::INACTIVE);
	windowButtons[BTN_ATCR_CPY] = CWinButton(BTN_ATCR_CPY, WIN_FLTPLN, "ATC/", CInputState::INACTIVE);
	windowButtons[BTN_MSG_REMOVE] = CWinButton(BTN_MSG_REMOVE, WIN_FLTPLN, "Remove", CInputState::INACTIVE);
	windowButtons[BTN_MSG_DONE] = CWinButton(BTN_MSG_DONE, WIN_FLTPLN, "Done", CInputState::INACTIVE);
	windowButtons[BTN_MSG_CLOSE] = CWinButton(BTN_MSG_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_MSG_REQUEUE] = CWinButton(BTN_MSG_REQUEUE, WIN_FLTPLN, "Requeue", CInputState::INACTIVE);
	windowButtons[BTN_MSG_FORWARD] = CWinButton(BTN_MSG_FORWARD, WIN_FLTPLN, "Forward", CInputState::INACTIVE);
	windowButtons[BTN_CONF_ACCCL] = CWinButton(BTN_CONF_ACCCL, WIN_FLTPLN, "ACCCL", CInputState::INACTIVE);
	windowButtons[BTN_CONF_MANCL] = CWinButton(BTN_CONF_MANCL, WIN_FLTPLN, "MANCL", CInputState::INACTIVE);
	windowButtons[BTN_CONF_COORD] = CWinButton(BTN_CONF_COORD, WIN_FLTPLN, "CO-ORD", CInputState::INACTIVE);
	windowButtons[BTN_CONF_CLOSE] = CWinButton(BTN_CONF_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_MAN_CANCEL] = CWinButton(BTN_MAN_CANCEL, WIN_FLTPLN, "Cancel", CInputState::INACTIVE);
	windowButtons[BTN_MAN_SUBMIT] = CWinButton(BTN_MAN_SUBMIT, WIN_FLTPLN, "Submit", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_READBK] = CWinButton(BTN_CLRC_READBK, WIN_FLTPLN, "ReadBK", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_VOICE] = CWinButton(BTN_CLRC_VOICE, WIN_FLTPLN, "Voice", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_SEND] = CWinButton(BTN_CLRC_SEND, WIN_FLTPLN, "Send", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_REJECT] = CWinButton(BTN_CLRC_REJECT, WIN_FLTPLN, "Reject", CInputState::INACTIVE);

	// Text defaults
	textInputs[TXT_ACID] = CTextInput(TXT_ACID, WIN_FLTPLN, "ACID", "", 70, CInputState::INACTIVE);
	textInputs[TXT_TYPE] = CTextInput(TXT_TYPE, WIN_FLTPLN, "Type", "", 55, CInputState::INACTIVE);
	textInputs[TXT_DEPART] = CTextInput(TXT_DEPART, WIN_FLTPLN, "Depart", "", 50, CInputState::INACTIVE);
	textInputs[TXT_ETD] = CTextInput(TXT_ETD, WIN_FLTPLN, "Etd", "", 50, CInputState::INACTIVE);
	textInputs[TXT_SELCAL] = CTextInput(TXT_SELCAL, WIN_FLTPLN, "SELCAL", "", 55, CInputState::INACTIVE);
	textInputs[TXT_DATALINK] = CTextInput(TXT_DATALINK, WIN_FLTPLN, "Datalink", "", 60, CInputState::INACTIVE);
	textInputs[TXT_COMMS] = CTextInput(TXT_COMMS, WIN_FLTPLN, "Com", "", 35, CInputState::INACTIVE);
	textInputs[TXT_OWNERSHIP] = CTextInput(TXT_OWNERSHIP, WIN_FLTPLN, "Sector", "", 25, CInputState::INACTIVE);
	textInputs[TXT_SPD] = CTextInput(TXT_SPD, WIN_FLTPLN, "Spd", "", 50, CInputState::ACTIVE);
	textInputs[TXT_LEVEL] = CTextInput(TXT_LEVEL, WIN_FLTPLN, "FL", "", 90, CInputState::ACTIVE);
	textInputs[TXT_DEST] = CTextInput(TXT_DEST, WIN_FLTPLN, "Dest", "", 45, CInputState::ACTIVE);
	textInputs[TXT_TCK] = CTextInput(TXT_TCK, WIN_FLTPLN, "Tck", "", 25, CInputState::DISABLED);
	textInputs[TXT_STATE] = CTextInput(TXT_STATE, WIN_FLTPLN, "State", "", 30, CInputState::DISABLED);
	textInputs[TXT_SPD_CPY] = CTextInput(TXT_SPD_CPY, WIN_FLTPLN, "Spd", "", 50, CInputState::ACTIVE);
	textInputs[TXT_LEVEL_CPY] = CTextInput(TXT_LEVEL_CPY, WIN_FLTPLN, "FL", "", 90, CInputState::ACTIVE);
	textInputs[TXT_DEST_CPY] = CTextInput(TXT_DEST_CPY, WIN_FLTPLN, "Dest", "", 45, CInputState::ACTIVE);
	textInputs[TXT_TCK_CPY] = CTextInput(TXT_TCK_CPY, WIN_FLTPLN, "Tck", "", 25, CInputState::DISABLED);
	textInputs[TXT_STATE_CPY] = CTextInput(TXT_STATE_CPY, WIN_FLTPLN, "State", "", 30, CInputState::DISABLED);
	textInputs[TXT_MAN_FL] = CTextInput(TXT_MAN_FL, WIN_FLTPLN, "FL", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_SPD] = CTextInput(TXT_MAN_SPD, WIN_FLTPLN, "Spd", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_TCK] = CTextInput(TXT_MAN_TCK, WIN_FLTPLN, "Tck", "", 25, CInputState::ACTIVE);
	textInputs[TXT_MAN_DEST] = CTextInput(TXT_MAN_DEST, WIN_FLTPLN, "Dest", "", 45, CInputState::ACTIVE);
	textInputs[TXT_MAN_EP] = CTextInput(TXT_MAN_EP, WIN_FLTPLN, "Entry", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_EPTIME] = CTextInput(TXT_MAN_EPTIME, WIN_FLTPLN, "Time", "", 60, CInputState::ACTIVE);

	/// Dropdown defaults
	map<string, bool> map;
	dropDowns[DRP_ATCR] = CDropDown(DRP_ATCR, WIN_FLTPLN, "", &map, CInputState::INACTIVE, 83);
	map.clear();
	dropDowns[DRP_ATCR_CPY] = CDropDown(DRP_ATCR_CPY, WIN_FLTPLN, "", &map, CInputState::INACTIVE, 83);
	map.clear();

	// Checkbox defaults
	checkBoxes[CHK_CLRC_VOX] = CCheckBox(CHK_CLRC_VOX, WIN_FLTPLN, "VOX", false, CInputState::INACTIVE);
	checkBoxes[CHK_CLRC_ORCA] = CCheckBox(CHK_CLRC_ORCA, WIN_FLTPLN, "ORCA", false, CInputState::INACTIVE);
	checkBoxes[CHK_CLRC_CPDLC] = CCheckBox(CHK_CLRC_CPDLC, WIN_FLTPLN, "CPDLC", false, CInputState::INACTIVE);
	checkBoxes[CHK_CLRC_TXT] = CCheckBox(CHK_CLRC_TXT, WIN_FLTPLN, "TXT", false, CInputState::INACTIVE);

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

	// Get window size
	int size = WINSZ_FLTPLN_HEIGHT_INIT;
	int message = 0;
	if (IsData) {
		size += WINSZ_FLTPLN_HEIGHT_DATA;
	}
	if (IsManualEntryOpen) {
		size += WINSZ_FLTPLN_HEIGHT_MANETRY;
	}
	if (IsCopyMade) {
		size += WINSZ_FLTPLN_HEIGHT_CPY;
	}
	if (IsConflictWindow || IsClearanceOpen) {
		size += WINSZ_FLTPLN_HEIGHT_XTRA;
	}
	if (IsMessageOpen) {
		message = WINSZ_FLTPLN_HEIGHT_MSG;
	}
	// Create base window rectangle
	CRect windowRect(topLeft.x, topLeft.y, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + size);
	dc->FillRect(windowRect, &darkerBrush);

	// Create titlebar
	CRect titleRect(windowRect.left, windowRect.top, windowRect.left + WINSZ_FLTPLN_WIDTH, windowRect.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string(string("Flight Plan") + string(" - ") + string(textInputs[TXT_ACID].Content)).c_str());

	// Add screen objects
	screen->AddScreenObject(WINDOW, "WIN_FLTPLN", windowRect, true, ""); // So that we can't click anything under the flight plan window
	screen->AddScreenObject(WINDOW, "FLTPLN", titleRect, true, ""); // Movable

	// Create button bar
	CRect buttonBarRect(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT - 1, windowRect.left + WINSZ_FLTPLN_WIDTH, windowRect.top + 100);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, ScreenBlue.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, ScreenBlue.ToCOLORREF(), BevelDark.ToCOLORREF());

	/// Draw top menu buttons
	int offsetX = 23;
	int offsetY = 6;
	int counter = 0; // Counter for buttons so we know when to offset the Y axis
	for (auto kv : windowButtons) {
		if (kv.first == BTN_ATCR) break; // Don't draw the restrictions buttons here
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 75, 30, &windowButtons.at(kv.first));
		if (counter != 5) { // Because there are 6 top row buttons
			offsetX += 82;
		}
		else {
			offsetX = 23;
			offsetY += 40;
		}
		// Increment the counter
		counter++;
	}

	// Create info bar
	CRect infoBarRect(windowRect.left, buttonBarRect.bottom + 1, windowRect.left + WINSZ_FLTPLN_WIDTH, buttonBarRect.bottom + 50);
	dc->FillRect(infoBarRect, &darkerBrush);
	dc->Draw3dRect(infoBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(infoBarRect, -1, -1);
	dc->Draw3dRect(infoBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Draw info bar text inputs
	offsetX = 4;
	offsetY = 6;
	for (auto kv : textInputs) {
		if (kv.first == TXT_SPD) break; // Break if end

		// Font
		FontSelector::SelectNormalFont(15, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Draw the text
		dc->TextOutA(infoBarRect.left + offsetX, infoBarRect.top + offsetY, kv.second.Label.c_str());

		// Get text height to set offset and height of input
		int textHeight = dc->GetTextExtent(kv.second.Label.c_str()).cy;
		offsetY += textHeight + 2;
		bool editable = false;
		if (kv.second.State == CInputState::ACTIVE) editable = true;
		CCommonRenders::RenderTextInput(dc, screen, { infoBarRect.left + offsetX, infoBarRect.top + offsetY }, kv.second.Width, textHeight + 5, &textInputs.at(kv.first));
		
		// Offset
		offsetX += kv.second.Width + 5;
		offsetY = 6;
	}

	/// PANELS & WINDOWS
	if (IsMessageOpen) {
		RenderMessageWindow(dc, g, screen, { windowRect.left, windowRect.top });
	}

	// Data panel
	CRect dataPanel;
	if (IsData) {
		dataPanel = RenderDataPanel(dc, g, screen, { windowRect.left, infoBarRect.bottom }, false);
	}

	// Copy panel
	CRect copyPanel;
	if (IsCopyMade) {
		copyPanel = RenderDataPanel(dc, g, screen, { windowRect.left, dataPanel.bottom }, true);
	}

	// Conflict panel
	if (IsConflictWindow && !IsCopyMade) { // No copy
		RenderConflictWindow(dc, g, screen, { windowRect.left, dataPanel.bottom });
	}
	else if (IsConflictWindow && IsCopyMade) { // Copy made
		RenderConflictWindow(dc, g, screen, { windowRect.left, copyPanel.bottom });
	}

	// Clearance panel
	if (IsClearanceOpen && !IsCopyMade) { // No copy
		RenderClearanceWindow(dc, g, screen, { windowRect.left, dataPanel.bottom });
	}
	else if (IsClearanceOpen && IsCopyMade) { // Copy made
		RenderClearanceWindow(dc, g, screen, { windowRect.left, copyPanel.bottom });
	}

	// Manual entry panel
	if (IsManualEntryOpen) {
		RenderManEntryWindow(dc, g, screen, { windowRect.left, infoBarRect.bottom });
	}

	// Coordination window
	if (IsCoordOpen) {
		RenderCoordModal(dc, g, screen, { windowRect.left + 80, windowRect.top + 50 });
	}

	// Create borders
	CRect borderRect(windowRect.left, windowRect.top - message, windowRect.right, windowRect.bottom);
	dc->DrawEdge(borderRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(borderRect, 1, 1);
	dc->Draw3dRect(borderRect, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(borderRect, 1, 1);
	dc->DrawEdge(borderRect, EDGE_RAISED, BF_RECT);

	// Cleanup
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
	

	// Restore device context
	dc->RestoreDC(iDC);
}

CRect CFlightPlanWindow::RenderDataPanel(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft, bool isCopy) {
	// Brushes
	CBrush routeBox(RouteBox.ToCOLORREF());
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());
	CBrush lightBackground(LightBackground.ToCOLORREF());

	// Create data bar
	CRect dataBarRect(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT_DATA + 2);
	dc->Draw3dRect(dataBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(dataBarRect, -1, -1);
	dc->Draw3dRect(dataBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Show callsign in coloured box
	CRect idBox(topLeft.x + 6, dataBarRect.top + 8, topLeft.x + 150, dataBarRect.top + 36);
	dc->FillRect(idBox, &lightBackground);
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(idBox.left + (idBox.Width() / 2), dataBarRect.top + (idBox.Height() / 2) - 2, textInputs[TXT_ACID].Content.c_str());

	// Create the route box
	CRect rteBox(topLeft.x + 5, idBox.bottom + 8, dataBarRect.right - 100, idBox.bottom + 84);
	dc->FillRect(rteBox, &routeBox);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(rteBox, -1, -1);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Draw text in route box

	// Make inputs
	bool txtValid = true;
	int counter = isCopy ? TXT_SPD_CPY : TXT_SPD;
	int offsetX = idBox.Width() + 10;
	int offsetY = 11;
	while (txtValid) {
		// Get index
		if (counter == TXT_SPD_CPY && !isCopy) break; // Break if end
		if (counter == TXT_STATE_CPY && isCopy) txtValid = false;

		CTextInput idx = textInputs.at(counter);

		// Font
		FontSelector::SelectNormalFont(15, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Get text height to set offset and height of input
		int textHeight = dc->GetTextExtent(idx.Label.c_str()).cy;
		int textWidth = dc->GetTextExtent(idx.Label.c_str()).cx;

		// Shove the label down
		if ((counter >= TXT_TCK && !isCopy) || (counter >= TXT_TCK_CPY && isCopy)) {
			offsetY += textHeight;
		}
		// Draw the text
		dc->TextOutA(dataBarRect.left + offsetX, dataBarRect.top + offsetY + 2, idx.Label.c_str());

		// So that track and state get shoved down a row
		if ((counter < TXT_TCK && !isCopy) || (counter < TXT_TCK_CPY && isCopy)) {
			offsetX += textWidth + 5;
		}
		else {
			offsetY += textHeight + 5;
		}

		bool editable = false;
		if (idx.State == CInputState::ACTIVE) editable = true;
		CCommonRenders::RenderTextInput(dc, screen, { dataBarRect.left + offsetX, dataBarRect.top + offsetY }, idx.Width, textHeight + 5, &textInputs.at(counter));

		// Offset
		if (counter == TXT_DEST || counter == TXT_DEST_CPY) { // Need a bit more space
			offsetX += idx.Width + 10;
		}
		else {
			offsetX += idx.Width + 5;
		}

		if (counter == TXT_TCK || counter == TXT_TCK_CPY) {
			offsetY -= textHeight + 20;
		}
		counter++;
	}
	// TODO: Scroll bar here

	// Render drop down and restrictions button 
	int restrictBtnType = isCopy ? BTN_ATCR_CPY : BTN_ATCR;
	int restrictDrpType = isCopy ? DRP_ATCR_CPY : DRP_ATCR;
	CRect button = CCommonRenders::RenderButton(dc, screen, { rteBox.left, rteBox.bottom + 15 }, 45, 20, &windowButtons.at(restrictBtnType), 1);
	CCommonRenders::RenderDropDown(dc, g, screen, { button.right + 5, rteBox.bottom + 15 }, WINSZ_FLTPLN_WIDTH - button.Width() - 20, 20, &dropDowns.at(restrictDrpType));

	// Cleanup
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
	DeleteObject(lightBackground);
	DeleteObject(routeBox);

	return dataBarRect;
}

void CFlightPlanWindow::RenderConflictWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create conflict panel
	CRect conflictPanel(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT_XTRA + 2);
	dc->Draw3dRect(conflictPanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(conflictPanel, -1, -1);
	dc->Draw3dRect(conflictPanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(conflictPanel.left, conflictPanel.top, conflictPanel.left + WINSZ_FLTPLN_WIDTH, conflictPanel.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("Conflict Window - ").c_str()); // TODO: show callsign properly
	
	// Create content panel
	CRect content(conflictPanel.left + 2, titleRect.bottom + 2, conflictPanel.left + ((WINSZ_FLTPLN_WIDTH / 3) * 2.4), titleRect.bottom + WINSZ_FLTPLN_HEIGHT_XTRA - 43);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Draw buttons (3 buttons)
	int offsetY = 3;
	for (int idx = BTN_CONF_ACCCL; idx <= BTN_CONF_CLOSE; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { conflictPanel.right - 82, titleRect.bottom + offsetY }, 75, 30, &windowButtons.at(idx));

		offsetY += 35;
	}

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);

}

void CFlightPlanWindow::RenderMessageWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT bottomLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create message panel
	CRect messagePanel(bottomLeft.x, bottomLeft.y - WINSZ_FLTPLN_HEIGHT_MSG - 2, bottomLeft.x + WINSZ_FLTPLN_WIDTH, bottomLeft.y);
	dc->FillRect(messagePanel, &darkerBrush);
	dc->Draw3dRect(messagePanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(messagePanel, -1, -1);
	dc->Draw3dRect(messagePanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(messagePanel.left, messagePanel.top, messagePanel.left + WINSZ_FLTPLN_WIDTH, messagePanel.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("FROM: ").c_str()); // TODO: show who from properly

	// Create button bar
	CRect buttonBarRect(messagePanel.left - 1, messagePanel.bottom - 40, messagePanel.left + WINSZ_FLTPLN_WIDTH, messagePanel.bottom);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());

	// Draw buttons (5 buttons)
	int offsetX = 5;
	for (int idx = BTN_MSG_REMOVE; idx <= BTN_MSG_FORWARD; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { buttonBarRect.left + offsetX, buttonBarRect.top + 5 }, 75, 30, &windowButtons.at(idx));

		if (idx + 1 == BTN_MSG_FORWARD) {
			offsetX += (buttonBarRect.Width() - offsetX - 75) - 7;
		}
		else {
			offsetX += 82;
		}
	}

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

void CFlightPlanWindow::RenderClearanceWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create clearance panel
	CRect clearancePanel(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT_XTRA + 2);
	dc->Draw3dRect(clearancePanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(clearancePanel, -1, -1);
	dc->Draw3dRect(clearancePanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(clearancePanel.left, clearancePanel.top, clearancePanel.left + WINSZ_FLTPLN_WIDTH, clearancePanel.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string(string("Clearance") + string(" - ")).c_str())); // TODO: show callsign properly

	// Create content panel 1
	CRect contentA(clearancePanel.left + 5, titleRect.bottom + 5, clearancePanel.left + ((WINSZ_FLTPLN_WIDTH / 3) * 2), titleRect.bottom + WINSZ_FLTPLN_HEIGHT_XTRA - 90);
	dc->Draw3dRect(contentA, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(contentA, -1, -1);
	dc->Draw3dRect(contentA, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Create content panel 2
	CRect contentB(clearancePanel.left + 5, contentA.bottom + 10, clearancePanel.left + ((WINSZ_FLTPLN_WIDTH / 3) * 2), contentA.bottom + 62);
	dc->Draw3dRect(contentB, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(contentB, -1, -1);
	dc->Draw3dRect(contentB, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Checkboxes
	int offsetX = 13;
	int offsetY = 10;
	for (int idx = CHK_CLRC_ORCA; idx <= CHK_CLRC_TXT; idx++)
	{
		CRect box = CCommonRenders::RenderCheckBox(dc, g, screen, { contentA.right + offsetX, titleRect.bottom + offsetY }, 15, &checkBoxes.at(idx));
		dc->TextOutA(box.right + 5, box.top - 1, checkBoxes.at(idx).Label.c_str());

		if (idx == CHK_CLRC_ORCA || idx == CHK_CLRC_CPDLC) {
			offsetY += 31;
		}
		else {
			if (idx != CHK_CLRC_TXT)
				offsetY = 10;
			else offsetY += 30;
			offsetX += 80;
		}
	}

	// Buttons (4 buttons)
	offsetX = 13;
	for (int idx = BTN_CLRC_READBK; idx <= BTN_CLRC_REJECT; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { contentA.right + offsetX, titleRect.bottom + offsetY }, 75, 30, &windowButtons.at(idx));

		if (idx == BTN_CLRC_READBK || idx == BTN_CLRC_VOICE) {
			offsetY += 35;
		}
		else {
			offsetY -= 35;
			offsetX += 80;
		}
	}


	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

void CFlightPlanWindow::RenderManEntryWindow(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush routeBox(RouteBox.ToCOLORREF());
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());
	CBrush lightBackground(LightBackground.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create manual entry panel
	CRect manEntryPanel(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT_MANETRY + 2);
	dc->Draw3dRect(manEntryPanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(manEntryPanel, -1, -1);
	dc->Draw3dRect(manEntryPanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(manEntryPanel.left, manEntryPanel.top, manEntryPanel.left + WINSZ_FLTPLN_WIDTH, manEntryPanel.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string(string("Manual Entry") + string(" - ")).c_str())); // TODO: show callsign properly

	// Show callsign in coloured box
	CRect idBox(topLeft.x + 6, titleRect.bottom + 8, topLeft.x + 150, titleRect.bottom + 36);
	dc->FillRect(idBox, &lightBackground);
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(idBox.left + (idBox.Width() / 2), manEntryPanel.top + (idBox.Height() / 2) - 2, textInputs[TXT_ACID].Content.c_str());

	// Create the route box
	CRect rteBox(topLeft.x + 5, idBox.bottom + 8, manEntryPanel.right - 150, idBox.bottom + 94);
	dc->FillRect(rteBox, &routeBox);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(rteBox, -1, -1);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// TODO scroll here

	// Draw text (6 text boxes)
	int offsetX = 5;
	int offsetY = 12;
	for (int idx = TXT_MAN_FL; idx <= TXT_MAN_EPTIME; idx++) {
		// Font
		FontSelector::SelectNormalFont(15, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Get text height to set offset and height of input
		int textHeight = dc->GetTextExtent(textInputs.at(idx).Label.c_str()).cy;
		int textWidth = dc->GetTextExtent(textInputs.at(idx).Label.c_str()).cx;
		dc->TextOutA(idBox.right + offsetX, titleRect.bottom + offsetY + 2, textInputs.at(idx).Label.c_str());
		CCommonRenders::RenderTextInput(dc, screen, { idBox.right + offsetX + textWidth + 5, titleRect.bottom + offsetY }, textInputs.at(idx).Width, textHeight + 5, &textInputs.at(idx));
		
		if (idx < TXT_MAN_DEST) {
			offsetX += textInputs.at(idx).Width + textWidth + 10;
		}
		else {
			if (idx == TXT_MAN_DEST)
				offsetY += textHeight + 20;
			else {
				offsetY += textHeight + 15;
			}
		}
	}

	// Draw buttons (2 buttons)
	offsetX += 3;
	offsetY += 8;
	for (int idx = BTN_MAN_CANCEL; idx <= BTN_MAN_SUBMIT; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { idBox.right + offsetX, titleRect.bottom + offsetY }, 65, 30, &windowButtons.at(idx));

		offsetX += 70;
	}

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

void CFlightPlanWindow::RenderCoordModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft) 
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create coordination window
	CRect coordWindow(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH_MDL, topLeft.y + WINSZ_FLTPLN_HEIGHT_COORD);
	dc->FillRect(coordWindow, &darkerBrush);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(coordWindow, -1, -1);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(coordWindow.left, coordWindow.top, coordWindow.left + WINSZ_FLTPLN_WIDTH_MDL, coordWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("Co-ordination Window - ").c_str())); // TODO: show callsign properly

	// Create borders
	dc->DrawEdge(coordWindow, EDGE_SUNKEN, BF_RECT);
	InflateRect(coordWindow, 1, 1);
	dc->Draw3dRect(coordWindow, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(coordWindow, 1, 1);
	dc->DrawEdge(coordWindow, EDGE_RAISED, BF_RECT);

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

void CFlightPlanWindow::RenderHistoryModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create history window
	CRect histWindow(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH_MDL, topLeft.y + WINSZ_FLTPLN_HEIGHT_HIST);
	dc->FillRect(histWindow, &darkerBrush);
	dc->Draw3dRect(histWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(histWindow, -1, -1);
	dc->Draw3dRect(histWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(histWindow.left, histWindow.top, histWindow.left + WINSZ_FLTPLN_WIDTH_MDL, histWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("History - ").c_str())); // TODO: show callsign properly

	// Create borders
	dc->DrawEdge(histWindow, EDGE_SUNKEN, BF_RECT);
	InflateRect(histWindow, 1, 1);
	dc->Draw3dRect(histWindow, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(histWindow, 1, 1);
	dc->DrawEdge(histWindow, EDGE_RAISED, BF_RECT);

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

void CFlightPlanWindow::RenderATCRestrictModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create restrictions window
	CRect atcrWindow(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH_MDL, topLeft.y + WINSZ_FLTPLN_HEIGHT_ATCR);
	dc->FillRect(atcrWindow, &darkerBrush);
	dc->Draw3dRect(atcrWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(atcrWindow, -1, -1);
	dc->Draw3dRect(atcrWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(atcrWindow.left, atcrWindow.top, atcrWindow.left + WINSZ_FLTPLN_WIDTH_MDL, atcrWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("ATC Restrictions Editor - ").c_str())); // TODO: show callsign properly

	// Create borders
	dc->DrawEdge(atcrWindow, EDGE_SUNKEN, BF_RECT);
	InflateRect(atcrWindow, 1, 1);
	dc->Draw3dRect(atcrWindow, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(atcrWindow, 1, 1);
	dc->DrawEdge(atcrWindow, EDGE_RAISED, BF_RECT);

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

void CFlightPlanWindow::RenderExchangeModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create status exchange and transfer window
	CRect coordWindow(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH_MDL, topLeft.y + WINSZ_FLTPLN_HEIGHT_TSFR);
	dc->FillRect(coordWindow, &darkerBrush);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(coordWindow, -1, -1);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(coordWindow.left, coordWindow.top, coordWindow.left + WINSZ_FLTPLN_WIDTH_MDL, coordWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("Co-ordination - ").c_str())); // TODO: show callsign properly

	// Create borders
	dc->DrawEdge(coordWindow, EDGE_SUNKEN, BF_RECT);
	InflateRect(coordWindow, 1, 1);
	dc->Draw3dRect(coordWindow, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(coordWindow, 1, 1);
	dc->DrawEdge(coordWindow, EDGE_RAISED, BF_RECT);

	// Clean up
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
}

bool CFlightPlanWindow::IsButtonPressed(int id) {
	// If button pressed
	if (id >= 200) { // dropdown
		if (dropDowns.find(id) != dropDowns.end() && dropDowns[id].State == CInputState::ACTIVE) return true;
	}
	else {
		if (windowButtons.find(id) != windowButtons.end() && windowButtons[id].State == CInputState::ACTIVE) return true;
	}

	return false; // Not pressed
}

void CFlightPlanWindow::UpdateData(CRadarScreen* screen, CAcFPStatus status) {
	// Get the data
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(status.Callsign.c_str());
	CFlightPlanData data = screen->GetPlugIn()->FlightPlanSelect(status.Callsign.c_str()).GetFlightPlanData();

	// Get SELCAL code
	string remarks = data.GetRemarks();
	size_t found = remarks.find(string("SEL/"));
	// If found
	if (found != string::npos) {
		textInputs[TXT_SELCAL].Content = remarks.substr(found + 4, 4);
	}
	else {
		textInputs[TXT_SELCAL].Content = "N/A";
	}

	// Get communication mode
	string comType;
	comType += toupper(data.GetCommunicationType());
	if (comType == "V") { // Switch each of the values
		comType = "VOX";
	}
	else if (comType == "T") {
		comType = "TXT";
	}
	else if (comType == "R") {
		comType = "RCV";
	}
	else {
		comType = "VOX"; // We assume voice if it defaults
	}

	// Fill data points
	textInputs[TXT_ACID].Content = fp.GetCallsign();
	textInputs[TXT_TYPE].Content = data.GetAircraftFPType();
	textInputs[TXT_DEPART].Content = data.GetOrigin();
	textInputs[TXT_ETD].Content = CUtils::ParseZuluTime(false, atoi(data.GetEstimatedDepartureTime()));
	textInputs[TXT_DATALINK].Content = "N/A";
	textInputs[TXT_COMMS].Content = comType;
	textInputs[TXT_OWNERSHIP].Content = "34";

	// TEMPORARY FOR BETA TESTING
	int mach = fp.GetControllerAssignedData().GetAssignedMach();
	textInputs[TXT_SPD].Content = string("M") + CUtils::PadWithZeros(3, fp.GetControllerAssignedData().GetAssignedMach());
	textInputs[TXT_LEVEL].Content = to_string(fp.GetControllerAssignedData().GetClearedAltitude());
	textInputs[TXT_DEST].Content = data.GetDestination();
}

void CFlightPlanWindow::OnCloseFlightPlanWindow() {

}

int CFlightPlanWindow::ChangeDataPoint(CRadarScreen* screen, int data, string str) {
	// TODO: Currently temporary format, make more robust when implementing the different window modes
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(textInputs[TXT_ACID].Content.c_str());
	if (data == TXT_SPD) {
		bool isNumber = true;
		for (int i = 0; i < strlen(str.c_str()); i++) { // Check if int
			if (!isdigit(str[i])) isNumber = false;
		}
		if (isNumber && (stoi(str) > 0 && stoi(str) < 250)) {
			bool status = fp.GetControllerAssignedData().SetAssignedMach(stoi(str));
			UpdateData(screen, CAcFPStatus(textInputs[TXT_ACID].Content, CFlightPlanMode::INIT));
			return 0;
		}
		return 1;
	}
	if (data == TXT_LEVEL) {
		bool isNumber = true;
		for (int i = 0; i < strlen(str.c_str()); i++) { // Check if int
			if (!isdigit(str[i])) isNumber = false;
		}
		if (isNumber && (stoi(str) > 0 && stoi(str) < 700)) {
			bool status = fp.GetControllerAssignedData().SetClearedAltitude(stoi(str));
			UpdateData(screen, CAcFPStatus(textInputs[TXT_ACID].Content, CFlightPlanMode::INIT));
			return 0;
		}
		return 1;
	}
	if (data == TXT_DEST) {
		if (strlen(str.c_str()) < 5) {
			bool isAlpha = true;
			for (int i = 0; i < strlen(str.c_str()); i++) { // Check if string
				if (!isalpha(str[i])) isAlpha = false;
			}
			if (isAlpha) {
				bool status = fp.GetFlightPlanData().SetDestination(str.c_str());
				fp.GetFlightPlanData().AmendFlightPlan();
				UpdateData(screen, CAcFPStatus(textInputs[TXT_ACID].Content, CFlightPlanMode::INIT));
				return 0;
			}
		}
		return 1;
	}
}

void CFlightPlanWindow::ButtonUp(int id) {

	if (id == CFlightPlanWindow::BTN_CLOSE) { // Close button
		// If the close button close window
		IsClosed = true;
	}

	// Failsafe
	bool stateSetManually = false; // Flag to show whether we've set the state manually
	if (windowButtons.find(id)->second.State != CInputState::DISABLED) {
		if (id == BTN_COPY) { // Make copy
			IsCopyMade = true;

			// Set the states
			SetButtonState(BTN_DELETE, CInputState::INACTIVE);
			SetButtonState(BTN_COPY, CInputState::DISABLED);

			// Flag
			stateSetManually = true;
		}
		if (id == BTN_DELETE) { // Delete copy
			IsCopyMade = false; // Delete copy (TODO: put more in here to actually delete copies)
			// Set the states
			SetButtonState(BTN_DELETE, CInputState::DISABLED);
			SetButtonState(BTN_COPY, CInputState::INACTIVE);

			stateSetManually = true;
		}
		if (id == BTN_PROBE) {
			IsConflictWindow = !IsConflictWindow ? true : false;
		}
		if (id == BTN_MANENTRY) {
			IsClearanceOpen = !IsClearanceOpen ? true : false;
		}
		if (id == BTN_MSG) {
			IsMessageOpen = !IsMessageOpen ? true : false;
		}
		if (id == BTN_COORD) {
			IsCoordOpen = !IsCoordOpen ? true : false;
		}
	}
	
	// Finally unpress the button if not disabled (and id is actually a button and not set manually)
	if (IsButton(id) && !stateSetManually && windowButtons.find(id)->second.State != CInputState::DISABLED) {
		SetButtonState(id, CInputState::INACTIVE);
	}
}

void CFlightPlanWindow::ButtonDown(int id) {
	// If not disabled, press
	if (IsButton(id) && windowButtons.find(id)->second.State != CInputState::DISABLED) {
		SetButtonState(id, CInputState::ACTIVE);
	}
}

void CFlightPlanWindow::ButtonPress(int id) {
	// Check if dropdown
	if (id >= 800) {
		// Set value
		dropDowns[ActiveDropDown].Value = dropDowns[ActiveDropDown].Items[id].Label;
		// Close drop down
		dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
		ActiveDropDownHover = 0;
		dropDowns[ActiveDropDown].State = CInputState::INACTIVE;
	}

	if (id >= 300) {
		if (checkBoxes.find(id) != checkBoxes.end()) {
			checkBoxes.at(id).IsChecked = checkBoxes.at(id).IsChecked ? false : true;
		}
	}

	// If dropdown then set the state
	if (IsDropDown(id)) {
		SetButtonState(id, CInputState::ACTIVE);
	}
}

void CFlightPlanWindow::ButtonUnpress(int id) {
	// If dropdown then set the state
	if (IsDropDown(id)) {
		SetButtonState(id, CInputState::INACTIVE);
	}
}

void CFlightPlanWindow::SetButtonState(int id, CInputState state) {
	if (id >= 200) { // Dropdown
			if (dropDowns.find(id) != dropDowns.end()) {
			// Remove the old dropdown stuff
			if (ActiveDropDownHover != 0) {
				dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
				ActiveDropDownHover = 0;
			}
			if (ActiveDropDown != 0) {
				dropDowns[ActiveDropDown].State = CInputState::INACTIVE;
				ActiveDropDown = 0;
			}

			dropDowns[id].State = state;
			ActiveDropDown = state == CInputState::ACTIVE ? id : 0;
		}
	}
	else { // normal button
		if (windowButtons.find(id) != windowButtons.end()) {
			windowButtons[id].State = state;
		}
	}
}