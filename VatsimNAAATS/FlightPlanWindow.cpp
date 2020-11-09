#include "pch.h"
#include "FlightPlanWindow.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"
#include "MessageWindow.h"

using namespace Colours;

CFlightPlanWindow::CFlightPlanWindow(POINT topLeft) : CBaseWindow(topLeft) {
	// Make buttons
	MakeWindowItems();

	// Close by default
	IsClosed = true;

	// Subwindow positions TODO: save
	subWindowPositions[SUBWIN_ATCR] = { 0, 0 };
	subWindowPositions[SUBWIN_COORD] = { 0, 0 };
	subWindowPositions[SUBWIN_HIST] = { 0, 0 };
	subWindowPositions[SUBWIN_XCHANGE] = { 0, 0 };

	// Restriction selections
	restrictionSelections[SEL_ATCR_LCHG] = "LCHG";
	restrictionSelections[SEL_ATCR_MCHG] = "MCHG";
	restrictionSelections[SEL_ATCR_EPC] = "EPC";
	restrictionSelections[SEL_ATCR_RERUTE] = "RERUTE";
	restrictionSelections[SEL_ATCR_RTD] = "RTD";
	restrictionSelections[SEL_ATCR_UNABLE] = "UNABLE";
	restrictionSelections[SEL_ATCR_ATA] = "ATA";
	restrictionSelections[SEL_ATCR_ATB] = "ATB";
	restrictionSelections[SEL_ATCR_XAT] = "XAT";
	restrictionSelections[SEL_ATCR_INT] = "INT";
};

void CFlightPlanWindow::MakeWindowItems() {
	// Button defaults
	windowButtons[BTN_CLOSE] = CWinButton(BTN_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_COPY] = CWinButton(BTN_COPY, WIN_FLTPLN, "Copy", CInputState::DISABLED);
	windowButtons[BTN_UNCLEAR] = CWinButton(BTN_UNCLEAR, WIN_FLTPLN, "Unclear", CInputState::DISABLED);
	windowButtons[BTN_COORD] = CWinButton(BTN_COORD, WIN_FLTPLN, "Co-ord", CInputState::INACTIVE);
	windowButtons[BTN_MANENTRY] = CWinButton(BTN_MANENTRY, WIN_FLTPLN, "ManEntry", CInputState::INACTIVE);
	windowButtons[BTN_PROBE] = CWinButton(BTN_PROBE, WIN_FLTPLN, "Probe", CInputState::DISABLED);
	windowButtons[BTN_DELETE] = CWinButton(BTN_DELETE, WIN_FLTPLN, "Delete", CInputState::DISABLED);
	windowButtons[BTN_ADS] = CWinButton(BTN_ADS, WIN_FLTPLN, "ADS", CInputState::DISABLED);
	windowButtons[BTN_READBK] = CWinButton(BTN_READBK, WIN_FLTPLN, "ReadBK", CInputState::DISABLED);
	windowButtons[BTN_MSG] = CWinButton(BTN_MSG, WIN_FLTPLN, "Message", CInputState::DISABLED);
	windowButtons[BTN_HIST] = CWinButton(BTN_HIST, WIN_FLTPLN, "History", CInputState::INACTIVE);
	windowButtons[BTN_SAVE] = CWinButton(BTN_SAVE, WIN_FLTPLN, "Save", CInputState::DISABLED);
	windowButtons[BTN_ATCR] = CWinButton(BTN_ATCR, WIN_FLTPLN, "ATC/", CInputState::INACTIVE);
	windowButtons[BTN_ATCR_CPY] = CWinButton(BTN_ATCR_CPY, WIN_FLTPLN, "ATC/", CInputState::INACTIVE);
	windowButtons[BTN_MSG_REMOVE] = CWinButton(BTN_MSG_REMOVE, WIN_FLTPLN, "Remove", CInputState::DISABLED);
	windowButtons[BTN_MSG_DONE] = CWinButton(BTN_MSG_DONE, WIN_FLTPLN, "Done", CInputState::DISABLED);
	windowButtons[BTN_MSG_CLOSE] = CWinButton(BTN_MSG_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_MSG_REQUEUE] = CWinButton(BTN_MSG_REQUEUE, WIN_FLTPLN, "Requeue", CInputState::INACTIVE);
	windowButtons[BTN_MSG_FORWARD] = CWinButton(BTN_MSG_FORWARD, WIN_FLTPLN, "Forward", CInputState::DISABLED);
	windowButtons[BTN_CONF_ACCCL] = CWinButton(BTN_CONF_ACCCL, WIN_FLTPLN, "ACCCL", CInputState::INACTIVE);
	windowButtons[BTN_CONF_MANCL] = CWinButton(BTN_CONF_MANCL, WIN_FLTPLN, "MANCL", CInputState::INACTIVE);
	windowButtons[BTN_CONF_COORD] = CWinButton(BTN_CONF_COORD, WIN_FLTPLN, "CO-ORD", CInputState::DISABLED);
	windowButtons[BTN_CONF_CLOSE] = CWinButton(BTN_CONF_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_MAN_CANCEL] = CWinButton(BTN_MAN_CANCEL, WIN_FLTPLN, "Cancel", CInputState::INACTIVE);
	windowButtons[BTN_MAN_SUBMIT] = CWinButton(BTN_MAN_SUBMIT, WIN_FLTPLN, "Submit", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_READBK] = CWinButton(BTN_CLRC_READBK, WIN_FLTPLN, "ReadBK", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_VOICE] = CWinButton(BTN_CLRC_VOICE, WIN_FLTPLN, "Transmit", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_SEND] = CWinButton(BTN_CLRC_SEND, WIN_FLTPLN, "Send", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_REJECT] = CWinButton(BTN_CLRC_REJECT, WIN_FLTPLN, "Reject", CInputState::INACTIVE);
	windowButtons[BTN_ATCR_ADD] = CWinButton(BTN_ATCR_ADD, WIN_FLTPLN, "Add", CInputState::INACTIVE);
	windowButtons[BTN_ATCR_CANCEL] = CWinButton(BTN_ATCR_CANCEL, WIN_FLTPLN, "Cancel", CInputState::INACTIVE);
	windowButtons[BTN_ATCR_OK] = CWinButton(BTN_ATCR_OK, WIN_FLTPLN, "OK", CInputState::INACTIVE);
	windowButtons[BTN_XCHANGE_NOTIFY] = CWinButton(BTN_XCHANGE_NOTIFY, WIN_FLTPLN, "Notify", CInputState::INACTIVE);
	windowButtons[BTN_XCHANGE_CLOSE] = CWinButton(BTN_XCHANGE_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_XCHANGE_ACCEPT] = CWinButton(BTN_XCHANGE_ACCEPT, WIN_FLTPLN, "Accept", CInputState::DISABLED);
	windowButtons[BTN_XCHANGE_TRANSFER] = CWinButton(BTN_XCHANGE_TRANSFER, WIN_FLTPLN, "Transfer", CInputState::DISABLED);
	windowButtons[BTN_XCHANGE_REJECT] = CWinButton(BTN_XCHANGE_REJECT, WIN_FLTPLN, "Reject", CInputState::DISABLED);
	windowButtons[BTN_XCHANGE_TRACK] = CWinButton(BTN_XCHANGE_TRACK, WIN_FLTPLN, "Track", CInputState::INACTIVE);
	windowButtons[BTN_COORD_CLOSE] = CWinButton(BTN_COORD_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);
	windowButtons[BTN_COORD_SENDOK] = CWinButton(BTN_COORD_SENDOK, WIN_FLTPLN, "Send/OK", CInputState::INACTIVE);
	windowButtons[BTN_HIST_CLOSE] = CWinButton(BTN_HIST_CLOSE, WIN_FLTPLN, "Close", CInputState::INACTIVE);

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
	textInputs[TXT_TCK] = CTextInput(TXT_TCK, WIN_FLTPLN, "Tck", "", 25, CInputState::ACTIVE);
	textInputs[TXT_STATE] = CTextInput(TXT_STATE, WIN_FLTPLN, "State", "", 30, CInputState::DISABLED);
	textInputs[TXT_SPD_CPY] = CTextInput(TXT_SPD_CPY, WIN_FLTPLN, "Spd", "", 50, CInputState::ACTIVE);
	textInputs[TXT_LEVEL_CPY] = CTextInput(TXT_LEVEL_CPY, WIN_FLTPLN, "FL", "", 90, CInputState::ACTIVE);
	textInputs[TXT_DEST_CPY] = CTextInput(TXT_DEST_CPY, WIN_FLTPLN, "Dest", "", 45, CInputState::ACTIVE);
	textInputs[TXT_TCK_CPY] = CTextInput(TXT_TCK_CPY, WIN_FLTPLN, "Tck", "", 25, CInputState::DISABLED);
	textInputs[TXT_STATE_CPY] = CTextInput(TXT_STATE_CPY, WIN_FLTPLN, "State", "", 30, CInputState::DISABLED);
	textInputs[TXT_MAN_FL] = CTextInput(TXT_MAN_FL, WIN_FLTPLN, "FL", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_SPD] = CTextInput(TXT_MAN_SPD, WIN_FLTPLN, "Spd", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_TCK] = CTextInput(TXT_MAN_TCK, WIN_FLTPLN, "Tck", "", 25, CInputState::ACTIVE);
	textInputs[TXT_MAN_DEST] = CTextInput(TXT_MAN_DEST, WIN_FLTPLN, "Dest", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_EP] = CTextInput(TXT_MAN_EP, WIN_FLTPLN, "Entry", "", 60, CInputState::ACTIVE);
	textInputs[TXT_MAN_EPTIME] = CTextInput(TXT_MAN_EPTIME, WIN_FLTPLN, "Time", "", 60, CInputState::ACTIVE);
	textInputs[TXT_XCHANGE_CURRENT] = CTextInput(TXT_XCHANGE_CURRENT, WIN_FLTPLN, "Current Authority", "NONE", 160, CInputState::INACTIVE);
	textInputs[TXT_XCHANGE_NEXT] = CTextInput(TXT_XCHANGE_NEXT, WIN_FLTPLN, "Selected Authority", "NONE", 160, CInputState::INACTIVE);
	textInputs[TXT_MAN_RTE] = CTextInput(TXT_MAN_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RTE] = CTextInput(TXT_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);
	textInputs[TXT_CPY_RTE] = CTextInput(TXT_CPY_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);

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
	checkBoxes[CHK_COORD_CZQOV] = CCheckBox(CHK_COORD_CZQOV, WIN_FLTPLN, "Gander", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_EGGXV] = CCheckBox(CHK_COORD_EGGXV, WIN_FLTPLN, "Shanwick", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_BIRDV] = CCheckBox(CHK_COORD_BIRDV, WIN_FLTPLN, "Reykjavik", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_ZNYV] = CCheckBox(CHK_COORD_ZNYV, WIN_FLTPLN, "New York", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_LPPOV] = CCheckBox(CHK_COORD_LPPOV, WIN_FLTPLN, "Santa Maria", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_EISNV] = CCheckBox(CHK_COORD_EISNV, WIN_FLTPLN, "Shannon", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_CZQXV] = CCheckBox(CHK_COORD_CZQXV, WIN_FLTPLN, "Gander", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_CZULV] = CCheckBox(CHK_COORD_CZULV, WIN_FLTPLN, "Montreal", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_EGPXV] = CCheckBox(CHK_COORD_EGPXV, WIN_FLTPLN, "Scottish", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_LFRRV] = CCheckBox(CHK_COORD_LFRRV, WIN_FLTPLN, "Brest", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_PLANV] = CCheckBox(CHK_COORD_PLANV, WIN_FLTPLN, "Planning", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_ENRV] = CCheckBox(CHK_COORD_ENRV, WIN_FLTPLN, "Enroute", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_CZQOT] = CCheckBox(CHK_COORD_CZQOT, WIN_FLTPLN, "Gander", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_EGGXT] = CCheckBox(CHK_COORD_EGGXT, WIN_FLTPLN, "Shanwick", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_BIRDT] = CCheckBox(CHK_COORD_BIRDT, WIN_FLTPLN, "Reykjavik", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_ZNYT] = CCheckBox(CHK_COORD_ZNYT, WIN_FLTPLN, "New York", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_LPPOT] = CCheckBox(CHK_COORD_LPPOT, WIN_FLTPLN, "Santa Maria", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_EISNT] = CCheckBox(CHK_COORD_EISNT, WIN_FLTPLN, "Shannon", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_CZQXT] = CCheckBox(CHK_COORD_CZQXT, WIN_FLTPLN, "Gander", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_CZULT] = CCheckBox(CHK_COORD_CZULT, WIN_FLTPLN, "Montreal", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_EGPXT] = CCheckBox(CHK_COORD_EGPXT, WIN_FLTPLN, "Scottish", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_LFRRT] = CCheckBox(CHK_COORD_LFRRT, WIN_FLTPLN, "Brest", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_PLANT] = CCheckBox(CHK_COORD_PLANT, WIN_FLTPLN, "Planning", false, CInputState::INACTIVE);
	checkBoxes[CHK_COORD_ENRT] = CCheckBox(CHK_COORD_ENRT, WIN_FLTPLN, "Enroute", false, CInputState::INACTIVE);

	// Scroll bars
	scrollBars[SCRL_DATA] = CWinScrollBar(SCRL_DATA, WIN_FLTPLN, 0, 0, true);
	scrollBars[SCRL_CPY] = CWinScrollBar(SCRL_CPY, WIN_FLTPLN, 0, 0, true);
	scrollBars[SCRL_CONF_X] = CWinScrollBar(SCRL_CONF_X, WIN_FLTPLN, 0, 0, true);
	scrollBars[SCRL_CONF_Y] = CWinScrollBar(SCRL_CONF_Y, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_HIST] = CWinScrollBar(SCRL_HIST, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_MSG] = CWinScrollBar(SCRL_MSG, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_CLRC] = CWinScrollBar(SCRL_CLRC, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_CLRC_XTRA] = CWinScrollBar(SCRL_CLRC_XTRA, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_MANENTRY] = CWinScrollBar(SCRL_MANENTRY, WIN_FLTPLN, 0, 0, true);
	scrollBars[SCRL_COORD_STATIONS] = CWinScrollBar(SCRL_COORD_STATIONS, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_COORD_HIST] = CWinScrollBar(SCRL_COORD_HIST, WIN_FLTPLN, 0, 0, false);
	scrollBars[SCRL_XCHANGE] = CWinScrollBar(SCRL_XCHANGE, WIN_FLTPLN, 0, 0, false);
}

void CFlightPlanWindow::MoveSubWindow(int id, POINT topLeft) {
	subWindowPositions[id] = { topLeft.x, topLeft.y };
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
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string(string("Flight Plan") + string(" - ") + primedPlan->Callsign).c_str());

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
		if (windowButtons[BTN_COPY].State == CInputState::DISABLED && !IsCopyMade && primedPlan->IsCleared)
			windowButtons[BTN_COPY].State = CInputState::INACTIVE;
		if (windowButtons[BTN_MANENTRY].State == CInputState::INACTIVE)
			windowButtons[BTN_MANENTRY].State = CInputState::DISABLED;
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

	// Create borders
	CRect borderRect(windowRect.left, windowRect.top - message, windowRect.right, windowRect.bottom);
	dc->DrawEdge(borderRect, EDGE_SUNKEN, BF_RECT);
	InflateRect(borderRect, 1, 1);
	dc->Draw3dRect(borderRect, WindowBorder.ToCOLORREF(), WindowBorder.ToCOLORREF());
	InflateRect(borderRect, 1, 1);
	dc->DrawEdge(borderRect, EDGE_RAISED, BF_RECT);

	// Active coordination modal
	if (IsTransferOpen) {
		RenderExchangeModal(dc, g, screen, { windowRect.left + 80, windowRect.top + 30 });
	}

	// Coordination modal
	if (IsCoordOpen) {
		RenderCoordModal(dc, g, screen, { windowRect.left + 85, windowRect.top + 30 });
	}
	// Coordination modal
	if (IsHistoryOpen) {
		RenderHistoryModal(dc, g, screen, { windowRect.left + 20, windowRect.top + 20 });
	}

	// Restrictions modal
	if (IsATCRestrictionsOpen) {
		RenderATCRestrictModal(dc, g, screen, { windowRect.left + 80, windowRect.top + 30 });
	}

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
	dc->TextOutA(idBox.left + (idBox.Width() / 2), dataBarRect.top + (idBox.Height() / 2) - 2, primedPlan->Callsign.c_str());

	// Create the route box
	CRect rteBox(topLeft.x + 5, idBox.bottom + 8, dataBarRect.right - 100, idBox.bottom + 84);
	dc->FillRect(rteBox, &routeBox);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(rteBox, -1, -1);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Route box to text input
	CTextInput* route = &textInputs.find(TXT_RTE)->second;
	CTextInput* track = &textInputs.find(TXT_TCK)->second;
	// Add object if not disabled
	if (route->State == CInputState::ACTIVE)
		screen->AddScreenObject(route->Type, to_string(route->Id).c_str(), rteBox, false, "");

	// Text align
	FontSelector::SelectATCFont(16, dc);
	dc->SetTextAlign(TA_LEFT);

	// Parse the route text
	int offsetX = 5;
	int offsetY = 2;
	if (route->Error || track->Error) { // If there is an error
		dc->TextOutA(rteBox.left + 4, rteBox.top + 2, "SYNTAX OR TCK ERROR! Check format.");
	}
	else if (route->Content == "") { // If the content is empty
		// Display "Enter route here" message
		dc->TextOutA(rteBox.left + 4, rteBox.top + 2, "Enter route here. Lat/lon format: 50/30.");
	}
	else {
		// Draw the route and estimates
		vector<CRoutePosition> rte;
		CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
		for (int i = 0; i < rte.size(); i++) {
			// If a waypoint
			if (CUtils::IsAllAlpha(rte[i].Fix)) {
				// Write fix down
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Fix.c_str());
				offsetY += 56;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate.c_str());
				offsetY = 2;
				offsetX += dc->GetTextExtent("2323").cx + 15;
			}
			else {
				// Write coordinate down
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Latitude)) + "W").c_str());
				offsetY += 28;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Longitude)) + "N").c_str());
				offsetY += 28;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate.c_str());
				offsetY = 2;
				offsetX += dc->GetTextExtent("2323").cx + 15;
			}
		}
	}

	// Scroll bar values
	if (!isCopy && scrollBars[SCRL_DATA].FrameSize == 0)
		scrollBars[SCRL_DATA] = CWinScrollBar(SCRL_DATA, WIN_FLTPLN, rteBox.Width(), rteBox.Width(), true);

	if (isCopy && scrollBars[SCRL_CPY].FrameSize == 0)
		scrollBars[SCRL_CPY] = CWinScrollBar(SCRL_CPY, WIN_FLTPLN, rteBox.Width(), rteBox.Width(), true);

	// Draw route scroll bar
	CCommonRenders::RenderScrollBar(dc, g, screen, { rteBox.left-1, rteBox.bottom + 3 }, &scrollBars[isCopy ? SCRL_CPY : SCRL_DATA]);

	// Make inputs
	bool txtValid = true;
	int counter = isCopy ? TXT_SPD_CPY : TXT_SPD;
	offsetX = idBox.Width() + 10;
	offsetY = 11;
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
	CRect button = CCommonRenders::RenderButton(dc, screen, { rteBox.left, rteBox.bottom + 18 }, 45, 20, &windowButtons.at(restrictBtnType), 1);
	CCommonRenders::RenderDropDown(dc, g, screen, { button.right + 5, rteBox.bottom + 18 }, WINSZ_FLTPLN_WIDTH - button.Width() - 20, 20, &dropDowns.at(restrictDrpType));

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
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("Conflict Window - " + primedPlan->Callsign).c_str()); // TODO: show callsign properly
	
	// Create content panel
	CRect content(conflictPanel.left + 2, titleRect.bottom + 2, conflictPanel.left + ((WINSZ_FLTPLN_WIDTH / 3) * 2.4), titleRect.bottom + WINSZ_FLTPLN_HEIGHT_XTRA - 43);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Scroll bar values
	if (scrollBars[SCRL_CONF_X].FrameSize == 0)
		scrollBars[SCRL_CONF_X] = CWinScrollBar(SCRL_CONF_X, WIN_FLTPLN, content.Width(), content.Width(), true);

	// Scroll bar values
	if (scrollBars[SCRL_CONF_Y].FrameSize == 0)
		scrollBars[SCRL_CONF_Y] = CWinScrollBar(SCRL_CONF_Y, WIN_FLTPLN, content.Height(), content.Height(), false);

	// Select content font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Iterate through conflicts (NEED BOTH VERTICAL AND HORIZONTAL SCROLL)
	if (currentProbeStatuses.empty()) {
		dc->TextOutA(content.left + 5, content.top + 5, "No predicted conflicts.");
	}
	else {
		for (int i = 0; i < currentProbeStatuses.size(); i++) {

		}
	}
	

	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { content.left - 1, content.bottom + 3 }, &scrollBars[SCRL_CONF_X]);
	CCommonRenders::RenderScrollBar(dc, g, screen, { content.right + 3, content.top - 1 }, &scrollBars[SCRL_CONF_Y]);

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
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("FROM: " + (primedPlan->CurrentMessage != nullptr ? primedPlan->CurrentMessage->From : "")).c_str()); // TODO: show who from properly

	// Create button bar
	CRect buttonBarRect(messagePanel.left - 1, messagePanel.bottom - 40, messagePanel.left + WINSZ_FLTPLN_WIDTH, messagePanel.bottom);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());

	// Create content panel
	CRect content(messagePanel.left + 5, titleRect.bottom + 2, messagePanel.right - 20, buttonBarRect.top - 5);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Draw content
	if (primedPlan->CurrentMessage != nullptr) {
		string message = CUtils::ParseToPhraseology(primedPlan->CurrentMessage->MessageRaw, primedPlan->CurrentMessage->Type);

		// Get the wrapped text
		vector<string> wrappedText;
		int contentSize = content.Width() - 10;
		if (dc->GetTextExtent(message.c_str()).cx > contentSize) {
			CUtils::WrapText(dc, message, ' ', contentSize, &wrappedText);
		}

		// If text is to be wrapped
		if (!wrappedText.empty()) {
			// Iterate to display
			int offsetY = 5;
			for (int i = 0; i < wrappedText.size(); i++) {
				dc->TextOutA(content.left + 5, content.top + offsetY, wrappedText[i].c_str());
				offsetY += dc->GetTextExtent("ABCD").cy + 2;
			}
		}
		else {
			// Just display
			dc->TextOutA(content.left + 5, content.top + 5, message.c_str());
		}
	}

	// Scroll bar values
	if (scrollBars[SCRL_MSG].FrameSize == 0)
		scrollBars[SCRL_MSG] = CWinScrollBar(SCRL_MSG, WIN_FLTPLN, content.Height(), content.Height() + 2, false);

	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { content.right + 3, content.top - 1 }, &scrollBars[SCRL_MSG]);

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
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string(string("Clearance") + string(" - ") + primedPlan->Callsign).c_str())); // TODO: show callsign properly

	// Create content panel 1
	CRect contentA(clearancePanel.left + 5, titleRect.bottom + 5, clearancePanel.left + ((WINSZ_FLTPLN_WIDTH / 3) * 2) - 10, titleRect.bottom + WINSZ_FLTPLN_HEIGHT_XTRA - 90);
	dc->Draw3dRect(contentA, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(contentA, -1, -1);
	dc->Draw3dRect(contentA, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Create content panel 2
	CRect contentB(clearancePanel.left + 5, contentA.bottom + 10, clearancePanel.left + ((WINSZ_FLTPLN_WIDTH / 3) * 2) - 10, contentA.bottom + 62);
	dc->Draw3dRect(contentB, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(contentB, -1, -1);
	dc->Draw3dRect(contentB, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Get the wrapped text
	vector<string> wrappedText;
	int contentSize = contentA.Width() - 10;
	if (dc->GetTextExtent(currentClearanceText.c_str()).cx > contentSize) {
		CUtils::WrapText(dc, currentClearanceText, ' ', contentSize, &wrappedText);
	}

	// If the text is wrapped
	int wrapOffsetY = 5;
	if (!wrappedText.empty()) {
		// Iterate to display
		wrapOffsetY = 5;
		for (int i = 0; i < wrappedText.size(); i++) {
			// Write the message
			dc->TextOutA(contentA.left + 5, contentA.top + wrapOffsetY, wrappedText[i].c_str());
			wrapOffsetY += dc->GetTextExtent("ABCD").cy + 5;
		}
	}
	else {
		// Write without iterating
		dc->TextOutA(contentA.left + 5, contentA.top + 5, currentClearanceText.c_str());
	}

	// Scroll bar values
	if (scrollBars[SCRL_CLRC].FrameSize == 0)
		scrollBars[SCRL_CLRC] = CWinScrollBar(SCRL_CLRC, WIN_FLTPLN, contentA.Height(), contentA.Height(), false);
	if (scrollBars[SCRL_CLRC_XTRA].FrameSize == 0)
		scrollBars[SCRL_CLRC_XTRA] = CWinScrollBar(SCRL_CLRC_XTRA, WIN_FLTPLN, contentB.Height(), contentB.Height(), false);


	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { contentA.right + 3, contentA.top - 1 }, &scrollBars[SCRL_CLRC]);
	CCommonRenders::RenderScrollBar(dc, g, screen, { contentB.right + 3, contentB.top - 1 }, &scrollBars[SCRL_CLRC_XTRA]);

	// Checkboxes
	int offsetX = 25;
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
	offsetX = 25;
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
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string(string("Manual Entry") + string(" - ") + primedPlan->Callsign).c_str())); // TODO: show callsign properly

	// Show callsign in coloured box
	CRect idBox(topLeft.x + 6, titleRect.bottom + 8, topLeft.x + 150, titleRect.bottom + 36);
	dc->FillRect(idBox, &lightBackground);
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(idBox.left + (idBox.Width() / 2), idBox.top + (idBox.Height() / 2) - 10, primedPlan->Callsign.c_str());

	// Create the route box
	CRect rteBox(topLeft.x + 5, idBox.bottom + 8, manEntryPanel.right - 150, idBox.bottom + 94);
	dc->FillRect(rteBox, &routeBox);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(rteBox, -1, -1);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Route box to text input
	CTextInput* route = &textInputs.find(TXT_MAN_RTE)->second;
	CTextInput* track = &textInputs.find(TXT_MAN_TCK)->second;
	if (route->State == CInputState::ACTIVE)
		screen->AddScreenObject(route->Type, to_string(route->Id).c_str(), rteBox, false, "");
	
	// Text align
	FontSelector::SelectATCFont(16, dc);
	dc->SetTextAlign(TA_LEFT);

	// Parse the route text
	int offsetX = 5;
	int offsetY = 2;
	if (route->Error || track->Error) { // If there is an error
		dc->TextOutA(rteBox.left + 4, rteBox.top + 2, "SYNTAX OR TCK ERROR! Check format.");
	}
	else if (route->Content == "") { // If the content is empty
		// Display "Enter route here" message
		dc->TextOutA(rteBox.left + 4, rteBox.top + 2, "Enter route here. Lat/lon format: 50/30.");
	}
	else {
		// Draw the route and estimates
		vector<CRoutePosition> rte;
		CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
		for (int i = 0; i < rte.size(); i++) {
			// If a waypoint
			if (CUtils::IsAllAlpha(rte[i].Fix)) {
				// Write fix down
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Fix.c_str());
				offsetY += 56;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate.c_str());
				offsetY = 2;
				offsetX += dc->GetTextExtent("2323").cx + 15;
			}
			else {
				// Write coordinate down
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Latitude)) + "W").c_str());
				offsetY += 28;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Longitude)) + "N").c_str());
				offsetY += 28;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate.c_str());
				offsetY = 2;
				offsetX += dc->GetTextExtent("2323").cx + 15;
			}
		}
	}
	

	// Scroll bar values
	if (scrollBars[SCRL_MANENTRY].FrameSize == 0)
		scrollBars[SCRL_MANENTRY] = CWinScrollBar(SCRL_MANENTRY, WIN_FLTPLN, rteBox.Width(), rteBox.Width(), true);

	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { rteBox.left - 1, rteBox.bottom + 3 }, &scrollBars[SCRL_MANENTRY]);

	// Draw text (6 text boxes)
	offsetX = 5;
	offsetY = 12;
	for (int idx = TXT_MAN_FL; idx <= TXT_MAN_EPTIME; idx++) {
		// Font
		FontSelector::SelectNormalFont(15, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Because OCD
		if (idx == TXT_MAN_DEST)
			offsetX += 2;
		else if (idx == TXT_MAN_DEST + 1) // Reset
			offsetX -= 2;

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

	// If the position is null then set it
	if (subWindowPositions[SUBWIN_COORD].x == 0 && subWindowPositions[SUBWIN_COORD].y == 0) {
		subWindowPositions[SUBWIN_COORD] = { topLeft.x, topLeft.y };
	}

	// Create coordination window
	CRect coordWindow(subWindowPositions[SUBWIN_COORD].x, subWindowPositions[SUBWIN_COORD].y + 1, subWindowPositions[SUBWIN_COORD].x + WINSZ_FLTPLN_WIDTH_COORD, subWindowPositions[SUBWIN_COORD].y + WINSZ_FLTPLN_HEIGHT_COORD);
	dc->FillRect(coordWindow, &darkerBrush);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(coordWindow, -1, -1);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(coordWindow.left, coordWindow.top, coordWindow.left + WINSZ_FLTPLN_WIDTH_COORD, coordWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_COORD / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("Co-ordination Window - " + primedPlan->Callsign).c_str())); // TODO: show callsign properly
	screen->AddScreenObject(WIN_FLTPLN, to_string(SUBWIN_COORD).c_str(), titleRect, true, "");

	// Create the stations panel
	CRect stations(coordWindow.left + 5, titleRect.bottom + 22, coordWindow.left + (WINSZ_FLTPLN_WIDTH_COORD / 3) * 1.7 - 10, titleRect.bottom + WINSZ_FLTPLN_HEIGHT_COORD - 77);
	dc->Draw3dRect(stations, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(stations, -1, -1);
	dc->Draw3dRect(stations, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Create history panel
	CRect history(stations.right + 18, titleRect.bottom + 22, stations.right + ((WINSZ_FLTPLN_WIDTH_COORD / 3) * 1.3) - 7, titleRect.bottom + WINSZ_FLTPLN_HEIGHT_COORD - 77);
	dc->Draw3dRect(history, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(history, -1, -1);
	dc->Draw3dRect(history, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Select font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Draw headers
	dc->TextOutA(stations.left + 3, stations.top - dc->GetTextExtent("Voice").cy - 2, "VOICE");
	dc->TextOutA(stations.right - dc->GetTextExtent("Manual").cx - 10, stations.top - dc->GetTextExtent("MANUAL").cy - 2, "MANUAL");
	dc->TextOutA(history.left + 3, history.top - dc->GetTextExtent("History").cy - 2, "History");

	// Draw button bar
	CRect buttonBarRect(coordWindow.left, coordWindow.bottom - 50, coordWindow.left + WINSZ_FLTPLN_WIDTH_COORD, coordWindow.bottom);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, BevelLight.ToCOLORREF(), ScreenBlue.ToCOLORREF());

	// Draw buttons (2 buttons)
	int offsetX = buttonBarRect.left + 5;
	for (int idx = BTN_COORD_CLOSE; idx <= BTN_COORD_SENDOK; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { offsetX, buttonBarRect.top + 10 }, 65, 30, &windowButtons.at(idx));

		offsetX = buttonBarRect.right - 73;
	}
	// BUG: last element with header overdrwa the window
	//		TEMP FIX: use counter
	// Draw checkboxes
	int offsetY = 0;
	int contentOffsetY = 0;
	int totalcontent = 0;
	int count = 1;
	for (int i = CHK_COORD_CZQOV; i <= CHK_COORD_ENRV; i++) {
		if (i == CHK_COORD_CZQOV || i == CHK_COORD_EISNV || i == CHK_COORD_PLANV)
		{
			totalcontent += 40; continue;
		}
		totalcontent += 20;
	}
	for (int i = CHK_COORD_CZQOV; i <= CHK_COORD_ENRV; i++) {
		bool headerOffset = false;
		if (!(contentOffsetY < scrollBars[SCRL_COORD_STATIONS].WindowPos)) {
			// Headers
			CRect rect(stations.left, stations.top + offsetY, stations.right - 1, stations.top + offsetY + dc->GetTextExtent("ABCD").cy);
			if (!(contentOffsetY > scrollBars[SCRL_COORD_STATIONS].WindowPos + scrollBars[SCRL_COORD_STATIONS].FrameSize)) {
				if (i == CHK_COORD_CZQOV) {
					dc->FillSolidRect(rect, ButtonPressed.ToCOLORREF());
					dc->TextOutA(stations.left + 35, stations.top + offsetY, "Oceanic");
					offsetY += 20;
					headerOffset = true;
				}
				else if (i == CHK_COORD_EISNV) {
					dc->FillSolidRect(rect, ButtonPressed.ToCOLORREF());
					dc->TextOutA(stations.left + 35, stations.top + offsetY, "Domestic");
					offsetY += 20;
					headerOffset = true;
				}
				else if (i == CHK_COORD_PLANV) {
					if (count == 8) break;
					dc->FillSolidRect(rect, ButtonPressed.ToCOLORREF());
					dc->TextOutA(stations.left + 35, stations.top + offsetY, "Misc");
					offsetY += 20;
					headerOffset = true;
				}
			}
		}

		if (!(contentOffsetY < scrollBars[SCRL_COORD_STATIONS].WindowPos)) {
			CRect box;
			if (!(contentOffsetY > scrollBars[SCRL_COORD_STATIONS].WindowPos + scrollBars[SCRL_COORD_STATIONS].FrameSize)) {
				// Checkboxes
				box = CCommonRenders::RenderCheckBox(dc, g, screen, { stations.left + 5, stations.top + offsetY }, 15, &checkBoxes.at(i));
				CCommonRenders::RenderCheckBox(dc, g, screen, { stations.right - 40, stations.top + offsetY }, 15, &checkBoxes.at(i + 56)); // The manual one
			}
			// Text
			dc->TextOutA(box.right + 15, box.top - 1, checkBoxes.at(i).Label.c_str());
			offsetY += 20;
			count++;
		}
		contentOffsetY += headerOffset ? 40 : 20;
	}

	// Scroll bar values
	if (scrollBars[SCRL_COORD_STATIONS].FrameSize == 0)
	{
		scrollBars[SCRL_COORD_STATIONS] = CWinScrollBar(SCRL_COORD_STATIONS, WIN_FLTPLN, totalcontent, stations.Height(), false);
		scrollBars[SCRL_COORD_STATIONS].GripSize -= 30; // temporary fix for grip bug
	}
	if (scrollBars[SCRL_COORD_HIST].FrameSize == 0)
		scrollBars[SCRL_COORD_HIST] = CWinScrollBar(SCRL_COORD_HIST, WIN_FLTPLN, history.Height(), history.Height(), false);

	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { stations.right + 3, stations.top - 1 }, &scrollBars[SCRL_COORD_STATIONS]);
	CCommonRenders::RenderScrollBar(dc, g, screen, { history.right + 3, history.top - 1 }, &scrollBars[SCRL_COORD_HIST]);

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

	// If the position is null then set it
	if (subWindowPositions[SUBWIN_HIST].x == 0 && subWindowPositions[SUBWIN_HIST].y == 0) {
		subWindowPositions[SUBWIN_HIST] = { topLeft.x, topLeft.y };
	}

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create history window
	CRect histWindow(subWindowPositions[SUBWIN_HIST].x, subWindowPositions[SUBWIN_HIST].y + 1, subWindowPositions[SUBWIN_HIST].x + WINSZ_FLTPLN_WIDTH_HIST, subWindowPositions[SUBWIN_HIST].y + WINSZ_FLTPLN_HEIGHT_HIST);
	dc->FillRect(histWindow, &darkerBrush);
	dc->Draw3dRect(histWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(histWindow, -1, -1);
	dc->Draw3dRect(histWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(histWindow.left, histWindow.top, histWindow.left + WINSZ_FLTPLN_WIDTH_HIST, histWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_HIST / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("Flight History - " + primedPlan->Callsign).c_str())); // TODO: show callsign properly
	screen->AddScreenObject(WIN_FLTPLN, to_string(SUBWIN_HIST).c_str(), titleRect, true, "");

	// Create content panel
	CRect content(histWindow.left + 5, titleRect.bottom + 20, histWindow.right - 20, histWindow.bottom - 40);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Scroll bar values
	if (scrollBars[SCRL_HIST].FrameSize == 0)
		scrollBars[SCRL_HIST] = CWinScrollBar(SCRL_HIST, WIN_FLTPLN, content.Height(), content.Height(), false);

	// Draw scroll bars
	CCommonRenders::RenderScrollBar(dc, g, screen, { content.right + 3, content.top - 1 }, &scrollBars[SCRL_HIST]);

	// Font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Create headers
	int offsetX = content.left + 3;
	dc->TextOutA(offsetX, content.top - dc->GetTextExtent("'SPACER").cy - 2, "Time");
	offsetX += dc->GetTextExtent("Time").cx + 7;
	dc->TextOutA(offsetX, content.top - dc->GetTextExtent("SPACER").cy - 2, "From");
	offsetX += dc->GetTextExtent("Time").cx + 25;
	dc->TextOutA(offsetX, content.top - dc->GetTextExtent("SPACER").cy - 2, "Message");

	// Render close button
	CCommonRenders::RenderButton(dc, screen, { content.left, content.bottom + 5 }, 65, 30, &windowButtons.at(BTN_HIST_CLOSE));

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

	// If the position is null then set it
	if (subWindowPositions[SUBWIN_ATCR].x == 0 && subWindowPositions[SUBWIN_ATCR].y == 0) {
		subWindowPositions[SUBWIN_ATCR] = { topLeft.x, topLeft.y };
	}

	// Create restrictions window
	CRect atcrWindow(subWindowPositions[SUBWIN_ATCR].x, subWindowPositions[SUBWIN_ATCR].y, subWindowPositions[SUBWIN_ATCR].x + WINSZ_FLTPLN_WIDTH_MDL, subWindowPositions[SUBWIN_ATCR].y + WINSZ_FLTPLN_HEIGHT_ATCR);
	dc->FillRect(atcrWindow, &darkerBrush);
	dc->Draw3dRect(atcrWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(atcrWindow, -1, -1);
	dc->Draw3dRect(atcrWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(atcrWindow.left, atcrWindow.top, atcrWindow.left + WINSZ_FLTPLN_WIDTH_MDL, atcrWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("ATC Restrictions Editor - ").c_str())); // TODO: show callsign properly
	screen->AddScreenObject(WIN_FLTPLN, to_string(SUBWIN_ATCR).c_str(), titleRect, true, "");

	// Select font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Create the restrictions panel
	CRect restrictions(atcrWindow.left + 5, titleRect.bottom + 22, atcrWindow.left + (WINSZ_FLTPLN_WIDTH_MDL / 3) * 1, titleRect.bottom + WINSZ_FLTPLN_HEIGHT_ATCR - 63);
	dc->Draw3dRect(restrictions, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(restrictions, -1, -1);
	dc->Draw3dRect(restrictions, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Add restrictions
	int offsetY = restrictions.top + 2;
	for (int i = SEL_ATCR_LCHG; i <= SEL_ATCR_INT; i++) {
		// Register click
		CRect textObj(restrictions.left, offsetY, restrictions.right - 1, offsetY + dc->GetTextExtent(restrictionSelections[i].c_str()).cy);
		if (i == selectedRestriction)
			dc->FillSolidRect(textObj, ButtonPressed.ToCOLORREF());

		// Text out
		dc->TextOutA(restrictions.left + 2, offsetY, restrictionSelections[i].c_str());
		
		// Screen object
		screen->AddScreenObject(WIN_FLTPLN, to_string(i).c_str(), textObj, false, "");

		// Offset
		offsetY += dc->GetTextExtent(restrictionSelections[i].c_str()).cy + 2;
	}


	// Create content panel
	CRect content(restrictions.right + 7, titleRect.bottom + 22, restrictions.right + ((WINSZ_FLTPLN_WIDTH_MDL / 3) * 1.945), titleRect.bottom + WINSZ_FLTPLN_HEIGHT_ATCR - 63);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Draw headers
	dc->TextOutA(restrictions.left + 3, restrictions.top - dc->GetTextExtent("Restrictions").cy - 2, "Restrictions");
	dc->TextOutA(content.left + 3, content.top - dc->GetTextExtent("ATC/").cy - 2, "ATC/");

	// Draw buttons (3 buttons)
	int offsetX = restrictions.left;
	for (int idx = BTN_ATCR_ADD; idx <= BTN_ATCR_OK; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { offsetX, restrictions.bottom + 5 }, 75, 30, &windowButtons.at(idx));

		if (idx == BTN_ATCR_ADD) {
			offsetX = content.right - 155;
		}
		else {
			offsetX += 80;
		}
	}


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

	// If the position is null then set it
	if (subWindowPositions[SUBWIN_XCHANGE].x == 0 && subWindowPositions[SUBWIN_XCHANGE].y == 0) {
		subWindowPositions[SUBWIN_XCHANGE] = { topLeft.x, topLeft.y };
	}
	
	// If selected controller enable the button
	if (selectedAuthority != "" && windowButtons[BTN_XCHANGE_TRANSFER].State == CInputState::DISABLED)
		SetButtonState(BTN_XCHANGE_TRANSFER, CInputState::INACTIVE);

	// Create coordination window
	CRect coordWindow(subWindowPositions[SUBWIN_XCHANGE].x, subWindowPositions[SUBWIN_XCHANGE].y + 1, subWindowPositions[SUBWIN_XCHANGE].x + WINSZ_FLTPLN_WIDTH_MDL, subWindowPositions[SUBWIN_XCHANGE].y + WINSZ_FLTPLN_HEIGHT_COORD);
	dc->FillRect(coordWindow, &darkerBrush);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(coordWindow, -1, -1);
	dc->Draw3dRect(coordWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(coordWindow.left, coordWindow.top, coordWindow.left + WINSZ_FLTPLN_WIDTH_MDL, coordWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string("Active Co-ordination - " + primedPlan->Callsign).c_str()); // TODO: show callsign properly
	screen->AddScreenObject(WIN_FLTPLN, to_string(SUBWIN_XCHANGE).c_str(), titleRect, true, "");

	// Select font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Create content panel
	CRect content(coordWindow.left + (coordWindow.Width() / 3) * 1.48, titleRect.bottom + 22, coordWindow.right - 20, titleRect.bottom + WINSZ_FLTPLN_HEIGHT_COORD - 30);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	dc->TextOutA(content.left + 3, content.top - dc->GetTextExtent("Active Authorities").cy - 2, "Active Authorities");

	// Scroll bar values
	if (scrollBars[SCRL_XCHANGE].FrameSize == 0)
		scrollBars[SCRL_XCHANGE] = CWinScrollBar(SCRL_XCHANGE, WIN_FLTPLN, content.Height(), content.Height(), false);

	// Draw scroll bar
	CCommonRenders::RenderScrollBar(dc, g, screen, { content.right + 3, content.top - 1 }, &scrollBars[SCRL_XCHANGE]);

	// Select font
	FontSelector::SelectNormalFont(15, dc);

	// List content
	int offsetX = 2;
	int offsetY = 2;
	for (auto kv : onlineControllers) {
		CRect rect(content.left, content.top + offsetY, content.right, content.top + offsetY + dc->GetTextExtent("ABCD").cy);
		if (kv.first == selectedAuthority)
			dc->FillSolidRect(rect, ButtonPressed.ToCOLORREF());
		dc->TextOutA(content.left + offsetX, content.top + offsetY, kv.second.GetPositionId());
		offsetX += 45;
		dc->TextOutA(content.left + offsetX, content.top + offsetY, kv.second.GetCallsign());
		offsetX = 0;
		offsetY += dc->GetTextExtent("ABCD").cy;
		screen->AddScreenObject(WIN_FLTPLN_TSFR, kv.second.GetCallsign(), rect, true, "");
	}

	// Get authorities
	string currentController = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
	textInputs[TXT_XCHANGE_CURRENT].Content = currentController != "" ? currentController : "UNTRACKED";
	textInputs[TXT_XCHANGE_NEXT].Content = selectedAuthority != "" ? selectedAuthority : "NONE";

	// Set buttons
	if (primedPlan->CurrentMessage != nullptr && primedPlan->CurrentMessage->Type == CMessageType::LOG_ON) {
		if (windowButtons[BTN_XCHANGE_ACCEPT].State == CInputState::DISABLED && windowButtons[BTN_XCHANGE_REJECT].State == CInputState::DISABLED) {
			windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::INACTIVE;
			windowButtons[BTN_XCHANGE_REJECT].State = CInputState::INACTIVE;
		}
	}

	// Draw buttons (6 buttons)
	offsetX = coordWindow.left + 5;
	offsetY = coordWindow.bottom - 107;
	for (int idx = BTN_XCHANGE_NOTIFY; idx <= BTN_XCHANGE_TRACK; idx++) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { offsetX, offsetY }, 85, 30, &windowButtons.at(idx));

		// Offsets
		if (idx == BTN_XCHANGE_CLOSE || idx == BTN_XCHANGE_TRANSFER) {
			offsetY += 35;
			offsetX = coordWindow.left + 5;
		}
		else {
			offsetX += 90;
		}
	}

	// Draw text (2 boxes)
	offsetX = coordWindow.left + 5;
	offsetY = content.top;
	for (int idx = TXT_XCHANGE_CURRENT; idx <= TXT_XCHANGE_NEXT; idx++) {
		// Font
		FontSelector::SelectNormalFont(15, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Get text height to set offset and height of input
		int textHeight = dc->GetTextExtent(textInputs.at(idx).Label.c_str()).cy;
		dc->TextOutA(offsetX, offsetY, textInputs.at(idx).Label.c_str());
		CCommonRenders::RenderTextInput(dc, screen, { offsetX, offsetY + textHeight + 5 }, textInputs.at(idx).Width, textHeight + 5, &textInputs.at(idx));
		
		offsetY += textHeight * 2 + 20;
	
	}

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

void CFlightPlanWindow::Instantiate(CRadarScreen* screen,string callsign, CMessage* msg) {
	// Make *all* window items again to reset
	MakeWindowItems();
	// Reset open windows
	IsData = false;
	IsCopyMade = false;
	IsConflictWindow = false;
	IsClearanceOpen = false;
	IsManualEntryOpen = false;
	IsMessageOpen = false;
	IsMessageOpen = false;
	IsCoordOpen = false;
	IsHistoryOpen = false;
	IsATCRestrictionsOpen = false;
	IsTransferOpen = false;

	// Get the data
	CAircraftFlightPlan* fp = CDataHandler::GetFlightData(callsign);
	if (!fp->IsValid) return;
	primedPlan = fp;

	// Add message
	if (msg != nullptr || primedPlan->CurrentMessage != nullptr) {
		primedPlan->CurrentMessage = msg;
		IsMessageOpen = true;
		windowButtons[BTN_MSG].State = CInputState::INACTIVE;
	}
	else {
		IsMessageOpen = false;
		windowButtons[BTN_MSG].State = CInputState::DISABLED;
	}

	// Window states
	if (fp->IsCleared) {
		// Need primed panel
		IsData = true;

		// Set route
		string route;
		for (int i = 0; i < fp->RouteRaw.size(); i++) {
			route += fp->RouteRaw[i] + " ";
		}
		route = route.substr(0, route.size() - 2); // Get rid of extra space

		// Set input states to disabled
		SetButtonState(BTN_ATCR, CInputState::DISABLED);
		dropDowns[DRP_ATCR].State = CInputState::DISABLED;
		textInputs[TXT_SPD].State = CInputState::DISABLED;
		textInputs[TXT_LEVEL].State = CInputState::DISABLED;
		textInputs[TXT_DEST].State = CInputState::DISABLED;
		textInputs[TXT_TCK].State = CInputState::DISABLED;
		textInputs[TXT_RTE].State = CInputState::DISABLED;

		// Set input contents
		textInputs[TXT_SPD].Content = string("M") + CUtils::PadWithZeros(3, stoi(primedPlan->Mach));
		textInputs[TXT_LEVEL].Content = primedPlan->FlightLevel;
		textInputs[TXT_DEST].Content = primedPlan->Dest;
		if (primedPlan->Track == "RR") {
			textInputs[TXT_TCK].Content = "RR";
		}
		else {
			textInputs[TXT_TCK].Content = primedPlan->Track;
		}
		textInputs[TXT_RTE].Content = route;
	}
	else {
		IsData = false;
		SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::DISABLED);
	}

	// Fill data points
	textInputs[TXT_ACID].Content = primedPlan->Callsign;
	textInputs[TXT_TYPE].Content = primedPlan->Type;
	textInputs[TXT_DEPART].Content = primedPlan->Depart;
	textInputs[TXT_ETD].Content = primedPlan->Etd;
	textInputs[TXT_DATALINK].Content = primedPlan->DLStatus;
	textInputs[TXT_COMMS].Content = primedPlan->Communications;
	textInputs[TXT_OWNERSHIP].Content = primedPlan->Sector;
	textInputs[TXT_SELCAL].Content = primedPlan->SELCAL;
}

void CFlightPlanWindow::SetTextValue(CRadarScreen* screen, int id, string content) {
	// Mach numbers
	if (id == TXT_SPD || id == TXT_SPD_CPY || id == TXT_MAN_SPD) {
		// Validation (range & length)
		// Check if it is a string
		bool isNumber = true;
		for (int i = 0; i < content.size(); i++) { // Check if string
			if (!isdigit(content[i])) return;
		}
		// It's a number, check the length
		if (stoi(content) > 200 || stoi(content) < 1) return;

		// We reached here so set the mach
		content = string("M") + CUtils::PadWithZeros(3, stoi(content));
	}
	// Tracks
	if (id == TXT_TCK || id == TXT_TCK_CPY || id == TXT_MAN_TCK) {
		// Validation (range & length)
		// Check if it is a string
		bool isNumber = false;
		for (int i = 0; i < content.size(); i++) { // Check if string
			if (!isalpha(content[i])) return;
			else toupper(content[i]);
		}

		// It's a string, check the length
		if (content.size() > 2 || content.size() < 1) return;

		// Parse the track
		int status = CRoutesHelper::ParseRoute(primedPlan->Callsign, content, true);
		if (status == 1) { // Validation failed
			textInputs.find(id)->second.Error = true;
		}
		else {
			if (status == 0) { // Validation success
				// Generate the route
				CUtils::CAsyncData* data = new CUtils::CAsyncData();
				data->Screen = screen;
				data->Callsign = primedPlan->Callsign;
				_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*)data); // Async

				// Set the error to false
				textInputs.find(id)->second.Error = false;

				// So we can put the content for the route boxes, make a string
				string routeString;
				for (int i = 0; i < primedPlan->RouteRaw.size(); i++) {
					routeString += primedPlan->RouteRaw[i] + " ";
				}
				// Set the contents
				if (id == TXT_TCK) {
					textInputs.find(TXT_RTE)->second.Content = routeString;
					textInputs.find(TXT_RTE)->second.Error = false;
				} else if (id == TXT_TCK_CPY) {
					textInputs.find(TXT_CPY_RTE)->second.Content = routeString;
					textInputs.find(TXT_CPY_RTE)->second.Error = false;
				}
				else {
					textInputs.find(TXT_MAN_RTE)->second.Content = routeString;
					textInputs.find(TXT_MAN_RTE)->second.Error = false;
				}
			}
		}
	}
	// Routes
	if (id == TXT_RTE  || id == TXT_MAN_RTE || id == TXT_CPY_RTE) {
		// Parse the route
		int status = CRoutesHelper::ParseRoute(primedPlan->Callsign, content);
		if (status == 1) { // Validation failed
			// Errored
			textInputs.find(id)->second.Error = true;
		}
		else {
			if (status == 0) { // Validation success
				// Generate the route
				CUtils::CAsyncData* data = new CUtils::CAsyncData();
				data->Screen = screen;
				data->Callsign = primedPlan->Callsign;
				_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*) data); // Async

				// Set the error to false
				textInputs.find(id)->second.Error = false;

				// Set the contents
				if (id == TXT_RTE) {
					textInputs.find(TXT_TCK)->second.Content = primedPlan->Track;
					textInputs.find(TXT_TCK)->second.Error = false;
				}
				else if (id == TXT_MAN_RTE) {
					textInputs.find(TXT_TCK_CPY)->second.Content = primedPlan->Track;
					textInputs.find(TXT_TCK_CPY)->second.Error = false;
				}
				else {
					textInputs.find(TXT_MAN_TCK)->second.Content = primedPlan->Track;
					textInputs.find(TXT_MAN_TCK)->second.Error = false;
				}
			}
		}
	}

	// Set the text
	if (textInputs.find(id) != textInputs.end()) {
		textInputs.find(id)->second.Content = content;
	}
}

CInputState CFlightPlanWindow::GetInputState(int id) {
	if (IsTextInput(id)) {
		return textInputs[id].State;
	}
	else if (IsButton(id)) {
		return windowButtons[id].State;
	}
}

void CFlightPlanWindow::ButtonUp(int id, CRadarScreen* screen) {

	if (id == CFlightPlanWindow::BTN_CLOSE) { // Close button
		// If the close button close window
		IsClosed = true;
		IsOpen = false;
	}

	// Failsafe
	bool stateSetManually = false; // Flag to show whether we've set the state manually
	if (IsButton(id) && windowButtons.find(id)->second.State != CInputState::DISABLED) {
		if (id == BTN_COPY) { // Make copy
			IsCopyMade = true;

			// Set the states
			SetButtonState(BTN_DELETE, CInputState::INACTIVE);
			SetButtonState(BTN_COPY, CInputState::DISABLED);
			SetButtonState(BTN_PROBE, CInputState::INACTIVE);
			// Set copy
			textInputs[TXT_STATE_CPY].Content = "UA";
			primedPlan->State = "CPY";

			// Flag
			stateSetManually = true;
		}
		if (id == BTN_DELETE) { // Delete copy
			IsCopyMade = false; // Delete copy (TODO: put more in here to actually delete copies)
			IsConflictWindow = false;
			// Set the states
			SetButtonState(BTN_DELETE, CInputState::DISABLED);
			SetButtonState(BTN_COPY, CInputState::INACTIVE);
			SetButtonState(BTN_PROBE, CInputState::DISABLED);

			stateSetManually = true;
		}
		if (id == BTN_PROBE) {
			IsConflictWindow = true;
			SetButtonState(BTN_MSG_REQUEUE, CInputState::DISABLED);
			SetButtonState(BTN_PROBE, CInputState::DISABLED);
			CConflictDetection::ProbeTool(screen, primedPlan->Callsign, &currentProbeStatuses);
			
			if (currentProbeStatuses.empty()) {
				SetButtonState(BTN_CONF_ACCCL, CInputState::INACTIVE);
				SetButtonState(BTN_CONF_MANCL, CInputState::DISABLED);
			}
			else {
				SetButtonState(BTN_CONF_MANCL, CInputState::INACTIVE);
				SetButtonState(BTN_CONF_ACCCL, CInputState::DISABLED);
			}
		}
		if (id == BTN_CONF_CLOSE) {
			IsConflictWindow = false;
			SetButtonState(BTN_PROBE, CInputState::INACTIVE);
		}
		if (id == BTN_MANENTRY || id == BTN_MAN_CANCEL) {
			IsManualEntryOpen = !IsManualEntryOpen ? true : false;
		}
		if (id == BTN_CONF_ACCCL || id == BTN_CONF_MANCL) {
			IsConflictWindow = false;
			IsCoordOpen = false;
			SetButtonState(BTN_PROBE, CInputState::DISABLED);
			SetButtonState(BTN_UNCLEAR, CInputState::INACTIVE);
			SetButtonState(BTN_DELETE, CInputState::DISABLED);

			// Switch the clearance type
			CMessageType type;
			if (primedPlan->CurrentMessage->Type == CMessageType::CLEARANCE_REQ) {
				type = CMessageType::CLEARANCE_ISSUE;
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = true;
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
				checkBoxes.at(CHK_CLRC_ORCA).State = CInputState::INACTIVE;
				checkBoxes.at(CHK_CLRC_CPDLC).State = CInputState::DISABLED;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;
			}
			else if (primedPlan->CurrentMessage->Type == CMessageType::REVISION_REQ){
				type = CMessageType::REVISION_ISSUE;
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
				checkBoxes.at(CHK_CLRC_CPDLC).State = CInputState::INACTIVE;
				checkBoxes.at(CHK_CLRC_ORCA).State = CInputState::DISABLED;
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = true;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;
			}
			IsClearanceOpen = true;
			currentClearanceText = CUtils::ParseToPhraseology(CUtils::ParseToRaw(primedPlan->Callsign, type), type);
		}
		if (id == BTN_CLRC_REJECT) {
			SetButtonState(BTN_PROBE, CInputState::INACTIVE);
			IsClearanceOpen = false;
		}
		if (id == BTN_CLRC_SEND) {
			SetButtonState(BTN_UNCLEAR, CInputState::DISABLED);
			SetButtonState(BTN_READBK, CInputState::INACTIVE);
			IsClearanceOpen = false;
			if (primedPlan->CurrentMessage->Type == CMessageType::CLEARANCE_REQ) {
				primedPlan->IsCleared = true;
				primedPlan->State = "PC";
				SetButtonState(BTN_ATCR, CInputState::DISABLED);
				dropDowns[DRP_ATCR].State = CInputState::DISABLED;
				textInputs[TXT_SPD].State = CInputState::DISABLED;
				textInputs[TXT_LEVEL].State = CInputState::DISABLED;
				textInputs[TXT_DEST].State = CInputState::DISABLED;
				textInputs[TXT_TCK].State = CInputState::DISABLED;
				textInputs[TXT_RTE].State = CInputState::DISABLED;
				// Send clearance to natTrak here
			}
		}
		if (id == BTN_READBK) {
			if (windowButtons[BTN_READBK].State == CInputState::ACTIVE) {
				IsClearanceOpen = false;
				IsCopyMade = false;
				primedPlan->State = "";
				textInputs[TXT_STATE].Content = "";
				SetButtonState(BTN_MSG_DONE, CInputState::INACTIVE);
				SetButtonState(BTN_READBK, CInputState::DISABLED);
				SetButtonState(BTN_DELETE, CInputState::DISABLED);
				SetButtonState(BTN_COPY, CInputState::INACTIVE);
				// Update flight data here on natTrak
			}
		}
		if (id == BTN_UNCLEAR) {
			if (windowButtons[BTN_UNCLEAR].State == CInputState::ACTIVE) {
				IsClearanceOpen = false;
				SetButtonState(BTN_UNCLEAR, CInputState::DISABLED);
				SetButtonState(BTN_DELETE, CInputState::INACTIVE);
				SetButtonState(BTN_PROBE, CInputState::INACTIVE);
			}
		}
		if (id == BTN_MSG || id  == BTN_MSG_CLOSE) {
			IsMessageOpen = !IsMessageOpen ? true : false;
		}
		if (id == BTN_COORD || id == BTN_XCHANGE_CLOSE) {
			selectedAuthority = "";
			SetButtonState(BTN_XCHANGE_TRANSFER, CInputState::DISABLED);
			windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::DISABLED;
			windowButtons[BTN_XCHANGE_REJECT].State = CInputState::DISABLED;
			IsTransferOpen = !IsTransferOpen ? true : false;
		}
		if (id == BTN_COORD_SENDOK) {
			IsCoordOpen = false;
		}
		if (id == BTN_ATCR || id == BTN_ATCR_CANCEL) {
			IsATCRestrictionsOpen = !IsATCRestrictionsOpen ? true : false;
		}
		if (id == BTN_CONF_COORD || id == BTN_COORD_CLOSE) {
			IsCoordOpen = !IsCoordOpen ? true : false;
		}
		if (id == BTN_HIST || id == BTN_HIST_CLOSE) {
			IsHistoryOpen = !IsHistoryOpen ? true : false;
		}

		// MESSAGE WINDOW
		if (id == BTN_MSG_REQUEUE) {
			// If the message is a logon message or transfer message
			if (primedPlan->CurrentMessage->Type == CMessageType::LOG_ON || primedPlan->CurrentMessage->Type == CMessageType::TRANSFER) {
				// Reset the accept/reject buttons
				SetButtonState(BTN_XCHANGE_ACCEPT, CInputState::DISABLED);
				SetButtonState(BTN_XCHANGE_REJECT, CInputState::DISABLED);
			}
			else if (primedPlan->CurrentMessage->Type == CMessageType::CLEARANCE_REQ) {
				IsData = false;
				primedPlan->Track = "";
				primedPlan->RouteRaw.clear();
				primedPlan->Route.clear();
				primedPlan->Restrictions.clear();
				primedPlan->Mach = "";
				primedPlan->FlightLevel = "";
				primedPlan->Track = "";
				primedPlan->Dest = "";
				SetButtonState(BTN_COPY, CInputState::DISABLED);
				SetButtonState(BTN_MANENTRY, CInputState::INACTIVE);
				SetButtonState(BTN_PROBE, CInputState::DISABLED);
			}
			// Erase the message from the aircraft and properly requeue it
			CMessageWindow::OngoingMessages.erase(primedPlan->CurrentMessage->Id);
			primedPlan->CurrentMessage = nullptr;
			IsMessageOpen = false;
			SetButtonState(BTN_MSG, CInputState::DISABLED);
			Instantiate(screen, primedPlan->Callsign);
		}
		if (id == BTN_MSG_DONE) {
			IsMessageOpen = false;
			// Mark message as done here
			CMessageWindow::OngoingMessages.erase(primedPlan->CurrentMessage->Id);
			CMessageWindow::ActiveMessages.erase(primedPlan->CurrentMessage->Id);
			primedPlan->CurrentMessage = nullptr;
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

	// Check if checkbox
	if (id >= 300) {
		if (checkBoxes.find(id) != checkBoxes.end()) {
			checkBoxes.at(id).IsChecked = checkBoxes.at(id).IsChecked ? false : true;

			if (id == CHK_CLRC_CPDLC) {
				SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_SEND, CInputState::INACTIVE);
				SetButtonState(BTN_CLRC_REJECT, CInputState::INACTIVE);
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;

			} else if (id == CHK_CLRC_ORCA) {
				SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_SEND, CInputState::INACTIVE);
				SetButtonState(BTN_CLRC_REJECT, CInputState::INACTIVE);
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;

			} else if (id == CHK_CLRC_TXT) {
				SetButtonState(BTN_CLRC_VOICE, CInputState::INACTIVE);
				SetButtonState(BTN_CLRC_READBK, CInputState::INACTIVE);
				SetButtonState(BTN_CLRC_SEND, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_REJECT, CInputState::DISABLED);
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;

			} else if (id == CHK_CLRC_VOX) {
				SetButtonState(BTN_CLRC_VOICE, CInputState::INACTIVE);
				SetButtonState(BTN_CLRC_READBK, CInputState::INACTIVE);
				SetButtonState(BTN_CLRC_SEND, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_REJECT, CInputState::DISABLED);
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
			}
		}
	}

	// Check if selection
	if (id >= 400) {
		if (restrictionSelections.find(id) != restrictionSelections.end()) {
			selectedRestriction = id;
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