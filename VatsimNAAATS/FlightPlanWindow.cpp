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
	windowButtons[BTN_MANENTRY] = CWinButton(BTN_MANENTRY, WIN_FLTPLN, "ManEntry", CInputState::DISABLED);
	windowButtons[BTN_PROBE] = CWinButton(BTN_PROBE, WIN_FLTPLN, "Probe", CInputState::DISABLED);
	windowButtons[BTN_DELETE] = CWinButton(BTN_DELETE, WIN_FLTPLN, "Delete", CInputState::DISABLED);
	windowButtons[BTN_ADS] = CWinButton(BTN_ADS, WIN_FLTPLN, "ADS", CInputState::DISABLED);
	windowButtons[BTN_READBK] = CWinButton(BTN_READBK, WIN_FLTPLN, "ReadBK", CInputState::DISABLED);
	windowButtons[BTN_MSG] = CWinButton(BTN_MSG, WIN_FLTPLN, "Message", CInputState::DISABLED);
	windowButtons[BTN_HIST] = CWinButton(BTN_HIST, WIN_FLTPLN, "History", CInputState::INACTIVE);
	windowButtons[BTN_SAVE] = CWinButton(BTN_SAVE, WIN_FLTPLN, "Save", CInputState::DISABLED);
	windowButtons[BTN_ATCR] = CWinButton(BTN_ATCR, WIN_FLTPLN, "ATC/", CInputState::DISABLED);
	windowButtons[BTN_ATCR_CPY] = CWinButton(BTN_ATCR_CPY, WIN_FLTPLN, "ATC/", CInputState::DISABLED);
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
	windowButtons[BTN_MAN_SUBMIT] = CWinButton(BTN_MAN_SUBMIT, WIN_FLTPLN, "Submit", CInputState::DISABLED);
	windowButtons[BTN_CLRC_READBK] = CWinButton(BTN_CLRC_READBK, WIN_FLTPLN, "ReadBK", CInputState::INACTIVE);
	windowButtons[BTN_CLRC_VOICE] = CWinButton(BTN_CLRC_VOICE, WIN_FLTPLN, "Man TX", CInputState::INACTIVE);
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
	windowButtons[BTN_RESTRI_EDIT_CLOSE] = CWinButton(BTN_RESTRI_EDIT_CLOSE, WIN_FLTPLN, "Cancel", CInputState::INACTIVE);
	windowButtons[BTN_RESTRI_EDIT_OK] = CWinButton(BTN_RESTRI_EDIT_OK, WIN_FLTPLN, "OK", CInputState::INACTIVE);

	// Text defaults
	textInputs[TXT_ACID] = CTextInput(TXT_ACID, WIN_FLTPLN, "ACID", "", 70, CInputState::INACTIVE);
	textInputs[TXT_TYPE] = CTextInput(TXT_TYPE, WIN_FLTPLN, "Type", "", 55, CInputState::INACTIVE);
	textInputs[TXT_DEPART] = CTextInput(TXT_DEPART, WIN_FLTPLN, "Depart", "", 50, CInputState::INACTIVE);
	textInputs[TXT_ETD] = CTextInput(TXT_ETD, WIN_FLTPLN, "Etd", "", 50, CInputState::INACTIVE);
	textInputs[TXT_SELCAL] = CTextInput(TXT_SELCAL, WIN_FLTPLN, "SELCAL", "", 55, CInputState::INACTIVE);
	textInputs[TXT_DATALINK] = CTextInput(TXT_DATALINK, WIN_FLTPLN, "Datalink", "", 60, CInputState::INACTIVE);
	textInputs[TXT_COMMS] = CTextInput(TXT_COMMS, WIN_FLTPLN, "Com", "", 35, CInputState::INACTIVE);
	textInputs[TXT_OWNERSHIP] = CTextInput(TXT_OWNERSHIP, WIN_FLTPLN, "Sector", "", 30, CInputState::INACTIVE);
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
	textInputs[TXT_MAN_EP] = CTextInput(TXT_MAN_EP, WIN_FLTPLN, "Entry", "DISABLED", 60, CInputState::DISABLED);
	textInputs[TXT_MAN_EPTIME] = CTextInput(TXT_MAN_EPTIME, WIN_FLTPLN, "Time", "DISABLED", 60, CInputState::DISABLED);
	textInputs[TXT_XCHANGE_CURRENT] = CTextInput(TXT_XCHANGE_CURRENT, WIN_FLTPLN, "Current Authority", "NONE", 160, CInputState::INACTIVE);
	textInputs[TXT_XCHANGE_NEXT] = CTextInput(TXT_XCHANGE_NEXT, WIN_FLTPLN, "Selected Authority", "NONE", 160, CInputState::INACTIVE);
	textInputs[TXT_MAN_RTE] = CTextInput(TXT_MAN_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RTE] = CTextInput(TXT_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);
	textInputs[TXT_CPY_RTE] = CTextInput(TXT_CPY_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);
	textInputs[TXT_CPY_RTE] = CTextInput(TXT_CPY_RTE, WIN_FLTPLN, "", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_LCHG_LATLON] = CTextInput(TXT_RESTRI_LCHG_LATLON, WIN_FLTPLN, "Lat/Lon", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_LCHG_TIME] = CTextInput(TXT_RESTRI_LCHG_TIME, WIN_FLTPLN, "Time", "", 0, CInputState::DISABLED);
	textInputs[TXT_RESTRI_MCHG_LATLON] = CTextInput(TXT_RESTRI_MCHG_LATLON, WIN_FLTPLN, "Lat/Lon", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_MCHG_TIME] = CTextInput(TXT_RESTRI_MCHG_TIME, WIN_FLTPLN, "Time", "", 0, CInputState::DISABLED);
	textInputs[TXT_RESTRI_ATA_LATLON] = CTextInput(TXT_RESTRI_ATA_LATLON, WIN_FLTPLN, "Lat/Lon", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_ATA_TIME] = CTextInput(TXT_RESTRI_ATA_TIME, WIN_FLTPLN, "Time", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_ATB_LATLON] = CTextInput(TXT_RESTRI_ATB_LATLON, WIN_FLTPLN, "Lat/Lon", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_ATB_TIME] = CTextInput(TXT_RESTRI_ATB_TIME, WIN_FLTPLN, "Time", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_XAT_LATLON] = CTextInput(TXT_RESTRI_XAT_LATLON, WIN_FLTPLN, "Lat/Lon", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_XAT_TIME] = CTextInput(TXT_RESTRI_XAT_TIME, WIN_FLTPLN, "Time", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_INT_CALLSIGN] = CTextInput(TXT_RESTRI_INT_CALLSIGN, WIN_FLTPLN, "Callsign", "", 0, CInputState::ACTIVE);
	textInputs[TXT_RESTRI_INT_INTERVAL] = CTextInput(TXT_RESTRI_INT_INTERVAL, WIN_FLTPLN, "Interval", "", 0, CInputState::ACTIVE);
	textInputs[TXT_EQUIPPED] = CTextInput(TXT_EQUIPPED, WIN_FLTPLN, "", "", 400, CInputState::INACTIVE);
	textInputs[TXT_CPY_EQUIPPED] = CTextInput(TXT_CPY_EQUIPPED, WIN_FLTPLN, "", "", 400, CInputState::INACTIVE);

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
	checkBoxes[CHK_RESTRI_LCHG] = CCheckBox(CHK_RESTRI_LCHG, WIN_FLTPLN, "Time?", false, CInputState::DISABLED);
	checkBoxes[CHK_RESTRI_MCHG] = CCheckBox(CHK_RESTRI_MCHG, WIN_FLTPLN, "Time?", false, CInputState::DISABLED);
	checkBoxes[CHK_RESTRI_UNABLE_SPD] = CCheckBox(CHK_RESTRI_UNABLE_SPD, WIN_FLTPLN, "Speed", false, CInputState::INACTIVE);
	checkBoxes[CHK_RESTRI_UNABLE_LVL] = CCheckBox(CHK_RESTRI_UNABLE_LVL, WIN_FLTPLN, "Level", false, CInputState::INACTIVE);
	checkBoxes[CHK_RESTRI_UNABLE_RTE] = CCheckBox(CHK_RESTRI_UNABLE_RTE, WIN_FLTPLN, "Route", false, CInputState::INACTIVE);

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

	// If not open, set the state to open
	IsOpen = true;
	IsClosed = false;

	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Get window size
	int size = IsData ? WINSZ_FLTPLN_HEIGHT_INIT - 25 : WINSZ_FLTPLN_HEIGHT_INIT;
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
	CRect infoBarRect(windowRect.left, buttonBarRect.bottom + 1, windowRect.left + WINSZ_FLTPLN_WIDTH, buttonBarRect.bottom + (IsData ? 50 : 75));
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

	if (!IsCopyMade && primedPlan->CurrentMessage == nullptr && !IsClearanceOpen && !IsConflictWindow && !IsManualEntryOpen) {
		windowButtons[BTN_XCHANGE_TRACK].State = CInputState::INACTIVE;
	}
	else {
		windowButtons[BTN_XCHANGE_TRACK].State = CInputState::DISABLED;
	}

	windowButtons[BTN_COPY].State = CInputState::DISABLED;

	// Draw AGCS equipped directly below aircraft details if no data panel
	if (!IsData) {
		if (primedPlan->IsEquipped)
			textInputs.at(TXT_EQUIPPED).Content = "AGCS EQUIPPED";
		else
			textInputs.at(TXT_EQUIPPED).Content = "NON COMPLIANT";
		CCommonRenders::RenderTextInput(dc, screen, { infoBarRect.left + 5, infoBarRect.bottom - 25 }, WINSZ_FLTPLN_WIDTH - 13, 20, &textInputs.at(TXT_EQUIPPED));
	}

	// Data panel
	bool controllerIsMe = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerIsMe();
	CRect dataPanel;
	if (IsData) {
		dataPanel = RenderDataPanel(dc, g, screen, { windowRect.left, infoBarRect.bottom }, false);
		if (windowButtons[BTN_COPY].State == CInputState::DISABLED && !IsCopyMade && primedPlan->IsCleared && primedPlan->State == "") {
			if (primedPlan->CurrentMessage != nullptr) {
				if (primedPlan->CurrentMessage->Type != CMessageType::CLEARANCE_REQ) {
					windowButtons[BTN_COPY].State = CInputState::INACTIVE;
				}
			}
			else {
				windowButtons[BTN_COPY].State = CInputState::INACTIVE;
			}
		}
	}

	/*if (!IsData)
		windowButtons[BTN_MANENTRY].State = CInputState::INACTIVE;
	else
		windowButtons[BTN_MANENTRY].State = CInputState::DISABLED;*/

	// So that you can't click read back if all the data is not there for manual entry
	bool isDataFilled = true;
	if (!primedPlan->IsCleared) {
		if (textInputs[TXT_DEST].Content == "" || textInputs[TXT_TCK].Content == "" || textInputs[TXT_RTE].Content == ""
			|| textInputs[TXT_LEVEL].Content == "" || textInputs[TXT_SPD].Content == "") {
			isDataFilled = false;
		}
	}

	if (((!primedPlan->IsCleared && IsData && isDataFilled) || IsCopyMade)) {
		windowButtons[BTN_READBK].State = CInputState::INACTIVE;
	}
	else {
		windowButtons[BTN_READBK].State = CInputState::DISABLED;
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

	// Restrictions type sub-modal
	if (RestrictionSubModalType > 0) {
		RenderATCRestrictSubModal(dc, g, screen, { windowRect.left + 80, windowRect.top + 30 });
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
	CBrush evenLighterBackground(NoReadBk.ToCOLORREF());

	// Create data bar
	CRect dataBarRect(topLeft.x, topLeft.y + 1, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT_DATA + 2);
	dc->Draw3dRect(dataBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(dataBarRect, -1, -1);
	dc->Draw3dRect(dataBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Show callsign in coloured box
	CRect idBox(topLeft.x + 6, dataBarRect.top + 8, topLeft.x + 150, dataBarRect.top + 36);
	dc->FillRect(idBox, !primedPlan->IsCleared || isCopy ? &evenLighterBackground : &lightBackground);
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
	CTextInput* route = &textInputs.find(isCopy ? TXT_CPY_RTE : TXT_RTE)->second;
	CTextInput* track = &textInputs.find(isCopy ? TXT_TCK_CPY : TXT_TCK)->second;
	// Add object if not disabled
	if (route->State == CInputState::ACTIVE)
		screen->AddScreenObject(route->Type, to_string(route->Id).c_str(), rteBox, false, "");

	// Text align
	FontSelector::SelectATCFont(16, dc);
	dc->SetTextAlign(TA_LEFT);

	// Parse the route text
	int offsetX = 5;
	int offsetY = 2;
	int contentoffsetX = 5;
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
		if (isCopy) {
			CRoutesHelper::GetRoute(screen, &rte, copiedPlan.Callsign, &copiedPlan);
		}
		else {
			CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
		}
		
		// Reverse it if it's westbound, they do it IRL
		if (!CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth()))
		{
			std::reverse(rte.begin(), rte.end());
		}

		for (int i = 0; i < rte.size(); i++) {
			// If a waypoint
			if (!(contentoffsetX < scrollBars[SCRL_DATA].WindowPos))
			{
				if (!(contentoffsetX > scrollBars[SCRL_DATA].WindowPos + scrollBars[SCRL_DATA].FrameSize - 15))
				{
					if (CUtils::IsAllAlpha(rte[i].Fix)) {
						// Write fix down
						dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Fix.c_str());
						offsetY += 56;
						dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate == "--" ? "\xa0\xa0" : rte[i].Estimate.c_str());
						offsetY = 2;
						offsetX += dc->GetTextExtent(rte[i].Fix.c_str()).cx + 15;
					}
					else {
						// Write coordinate down
						dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Longitude)) + "W").c_str());
						offsetY += 28;
						dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Latitude)) + "N").c_str());
						offsetY += 28;						
						dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate == "--" ? "\xa0\xa0" : rte[i].Estimate.c_str());
						offsetY = 2;
						offsetX += dc->GetTextExtent("2323").cx + 15;
					}
				}
			}
			contentoffsetX += dc->GetTextExtent(rte[i].Fix.c_str()).cx + 15;
		}
	}

	// Scroll bar values

	if (scrollBars[SCRL_DATA].FrameSize == 0 || (scrollBars[SCRL_DATA].ContentSize != contentoffsetX && scrollBars[SCRL_DATA].PositionDelta == 0 && scrollBars[SCRL_DATA].ContentRatio != 1))
	{
		int framebox = rteBox.Width();
		scrollBars[SCRL_DATA] = CWinScrollBar(SCRL_DATA, WIN_FLTPLN, contentoffsetX, framebox, true);
		if (contentoffsetX != framebox)
		{
			//scrollBars[SCRL_DATA].GripSize -= 30; // temporary fix for grip bug
			scrollBars[SCRL_DATA].TotalScrollableArea = framebox - scrollBars[SCRL_DATA].GripSize;
		}
	}

	if (scrollBars[SCRL_CPY].FrameSize == 0 || (scrollBars[SCRL_CPY].ContentSize != contentoffsetX && scrollBars[SCRL_CPY].PositionDelta == 0 && scrollBars[SCRL_CPY].ContentRatio != 1))
	{
		int framebox = rteBox.Width();
		scrollBars[SCRL_CPY] = CWinScrollBar(SCRL_CPY, WIN_FLTPLN, contentoffsetX, framebox, true);
		if (contentoffsetX != framebox)
		{
			//scrollBars[SCRL_CPY].GripSize -= 30; // temporary fix for grip bug
			scrollBars[SCRL_CPY].TotalScrollableArea = framebox - scrollBars[SCRL_CPY].GripSize;
		}
	}

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

	// Render the AGCS Equipped text box
	int boxType = isCopy ? TXT_CPY_EQUIPPED : TXT_EQUIPPED;
	if (primedPlan->IsEquipped)
		textInputs.at(boxType).Content = "AGCS EQUIPPED";
	else
		textInputs.at(boxType).Content = "NON COMPLIANT";
	CCommonRenders::RenderTextInput(dc, screen, { dataBarRect.left + 5, rteBox.bottom + 43 }, WINSZ_FLTPLN_WIDTH - 13, 20, &textInputs.at(boxType));

	// Cleanup
	DeleteObject(darkerBrush);
	DeleteObject(lighterBrush);
	DeleteObject(lightBackground);
	DeleteObject(evenLighterBackground);
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
		int offsetX = content.left + 5;
		int offsetY = content.top + 5;
		// Draw main aircraft data
		dc->TextOutA(offsetX, offsetY, primedPlan->Callsign.c_str());
		offsetX += 65;
		dc->TextOutA(offsetX, offsetY, primedPlan->FlightLevel.c_str());
		offsetX += 30;
		dc->TextOutA(offsetX, offsetY, ("M" + CUtils::PadWithZeros(3, stoi(primedPlan->Mach))).c_str());
		offsetX += 45;
		// Draw main route
		for (int i = 0; i < primedPlan->RouteRaw.size(); i++) {			
			dc->TextOutA(offsetX, offsetY, primedPlan->RouteRaw[i].c_str());
			offsetX += 45;
		}
		
		offsetY += dc->GetTextExtent("ABCD").cy + 2;

		offsetX = content.left + 5;

		// Draw every other aircraft
		for (auto kv : currentProbeStatuses) {
			CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(kv.first.c_str());
			int mach = target.GetCorrelatedFlightPlan().GetFlightPlanData().PerformanceGetMach(target.GetPosition().GetPressureAltitude(), target.GetVerticalSpeed());
			// Draw aircraft data
			dc->TextOutA(offsetX, offsetY, kv.first.c_str());
			offsetX += 65;
			dc->TextOutA(offsetX, offsetY, to_string((int)(round(target.GetPosition().GetFlightLevel()) / 100.0)).c_str());
			offsetX += 30;
			dc->TextOutA(offsetX, offsetY, ("M" + CUtils::PadWithZeros(3, mach)).c_str());
			offsetX += 45;
			// Draw statuses
			for (int i = 0; i < kv.second.size(); i++) {
				char conflict = kv.second[i].ConflictStatus == CConflictStatus::CRITICAL ? 'C' : 'W';
				dc->TextOutA(offsetX, offsetY, (conflict + to_string(kv.second[i].DistanceAsTime)).c_str());
				offsetX += 45;
			}
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

	int offsetY = 0;
	int contentoffsetY = 0;
	// Draw content
	if (primedPlan->CurrentMessage != nullptr) {
		string message = CUtils::ParseToPhraseology(primedPlan->CurrentMessage->MessageRaw, primedPlan->CurrentMessage->Type, primedPlan->Callsign);

		// Get the wrapped text
		vector<string> wrappedText;
		int contentSize = content.Width() - 10;
		if (dc->GetTextExtent(message.c_str()).cx > contentSize) {
			CUtils::WrapText(dc, message, ' ', contentSize, &wrappedText);
		}

		// If text is to be wrapped
		if (!wrappedText.empty()) {
			// Iterate to display
			offsetY = 5;
			contentoffsetY = 5;
			for (int i = 0; i < wrappedText.size(); i++) {
				if (!(contentoffsetY < scrollBars[SCRL_MSG].WindowPos))
				{
					if (!(contentoffsetY > scrollBars[SCRL_MSG].WindowPos + scrollBars[SCRL_MSG].FrameSize-25))
					{
						dc->TextOutA(content.left + 5, content.top + offsetY, wrappedText[i].c_str());
						offsetY += dc->GetTextExtent("ABCD").cy + 2;
					}
				}
				contentoffsetY += dc->GetTextExtent("ABCD").cy + 2;
			}
		}
		else {
			// Just display
			dc->TextOutA(content.left + 5, content.top + 5, message.c_str());
			contentoffsetY = content.Height()+2;
		}
	}

	// Scroll bar values
	if(scrollBars[SCRL_MSG].FrameSize == 0 || (scrollBars[SCRL_MSG].ContentSize != contentoffsetY && scrollBars[SCRL_MSG].PositionDelta == 0 && scrollBars[SCRL_MSG].ContentRatio != 1))
	{
		int framebox = content.Height()+2;
		scrollBars[SCRL_MSG] = CWinScrollBar(SCRL_MSG, WIN_FLTPLN, contentoffsetY, framebox, false);
		if (contentoffsetY != framebox)
		{
			scrollBars[SCRL_MSG].GripSize -= 30; // temporary fix for grip bug
			scrollBars[SCRL_MSG].TotalScrollableArea = framebox - scrollBars[SCRL_MSG].GripSize;
		}
	}

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
	int contentoffsetY = 5;
	if (!wrappedText.empty()) {
		// Iterate to display
		wrapOffsetY = 5;
		for (int i = 0; i < wrappedText.size(); i++) {
			if (!(contentoffsetY < scrollBars[SCRL_CLRC].WindowPos))
			{
				if (!(contentoffsetY > scrollBars[SCRL_CLRC].WindowPos + scrollBars[SCRL_CLRC].FrameSize - 10))
				{
					// Write the message
					dc->TextOutA(contentA.left + 5, contentA.top + wrapOffsetY, wrappedText[i].c_str());
					wrapOffsetY += dc->GetTextExtent("ABCD").cy + 5;
				}
			}
			contentoffsetY += dc->GetTextExtent("ABCD").cy + 5;
		}
	}
	else {
		wrapOffsetY = contentoffsetY = 0;
		// Write without iterating
		dc->TextOutA(contentA.left + 5, contentA.top + 5, currentClearanceText.c_str());
	}

	if (contentoffsetY < contentA.Height())
		contentoffsetY = contentA.Height();

	// Scroll bar values
	if (scrollBars[SCRL_CLRC].FrameSize == 0 || (scrollBars[SCRL_CLRC].ContentSize != contentoffsetY && scrollBars[SCRL_CLRC].PositionDelta == 0 && scrollBars[SCRL_CLRC].ContentRatio != 1))
	{
		int framebox = contentA.Height();
		scrollBars[SCRL_CLRC] = CWinScrollBar(SCRL_CLRC, WIN_FLTPLN, contentoffsetY, framebox, false);
		if (contentoffsetY != framebox)
		{
			scrollBars[SCRL_CLRC].GripSize -= 20; // temporary fix for grip bug
			scrollBars[SCRL_CLRC].TotalScrollableArea = framebox - scrollBars[SCRL_CLRC].GripSize;
		}
	}
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

// Todo: modify this to make more suitable for voice clearances, and for position report entry and fix bugs
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

	if (textInputs[TXT_MAN_FL].Content == "" || textInputs[TXT_MAN_SPD].Content == "") {
		if (windowButtons[BTN_MAN_SUBMIT].State == CInputState::INACTIVE)
			windowButtons[BTN_MAN_SUBMIT].State = CInputState::DISABLED;
	}
	else {
		windowButtons[BTN_MAN_SUBMIT].State = CInputState::INACTIVE;
	}

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

		// Reverse it if it's westbound, they do it IRL
		if (CUtils::GetAircraftDirection(!screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth()))
		{
			std::reverse(rte.begin(), rte.end());
		}

		for (int i = 0; i < rte.size(); i++) {
			// If a waypoint
			if (CUtils::IsAllAlpha(rte[i].Fix)) {
				// Write fix down
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Fix.c_str());
				offsetY += 56;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate == "--" ? "\xa0\xa0" : rte[i].Estimate.c_str());
				offsetY = 2;
				offsetX += dc->GetTextExtent(rte[i].Fix.c_str()).cx + 15;
			}
			else {
				// Write coordinate down
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Longitude)) + "W").c_str());
				offsetY += 28;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, (to_string((int)abs(rte[i].PositionRaw.m_Latitude)) + "N").c_str());
				offsetY += 28;
				dc->TextOutA(rteBox.left + offsetX, rteBox.top + offsetY, rte[i].Estimate == "--" ? "\xa0\xa0" : rte[i].Estimate.c_str());
				offsetY = 2;
				offsetX += dc->GetTextExtent(rte[i].Fix.c_str()).cx + 15;
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

	// Validation
	if (textInputs.at(TXT_MAN_FL).Content == "" ||
		textInputs.at(TXT_MAN_SPD).Content == "" ||
		textInputs.at(TXT_MAN_TCK).Content == "" ||
		textInputs.at(TXT_MAN_DEST).Content == "" ||
		textInputs.at(TXT_MAN_RTE).Content == "")
		windowButtons[BTN_MAN_SUBMIT].State = CInputState::DISABLED;
	else 
		windowButtons[BTN_MAN_SUBMIT].State = CInputState::INACTIVE;

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

	// Plan
	CAircraftFlightPlan* plan = IsCopyMade ? &copiedPlan : primedPlan;

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
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("ATC Restrictions Editor - " + primedPlan->Callsign).c_str())); // TODO: show callsign properly
	screen->AddScreenObject(WIN_FLTPLN, "RESTRICTIONS", atcrWindow, false, ""); // So it can't be moved
	screen->AddScreenObject(WIN_FLTPLN, to_string(SUBWIN_ATCR).c_str(), titleRect, true, "");


	// Select font
	FontSelector::SelectNormalFont(15, dc);
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

		// Colour
		dc->SetTextColor(Disabled.ToCOLORREF());

		// Screen object
		if (i != SEL_ATCR_RTD && i != SEL_ATCR_EPC) {
			if (IsCopyMade && (i != SEL_ATCR_RERUTE && i != SEL_ATCR_INT)) {
				screen->AddScreenObject(WIN_FLTPLN, to_string(i).c_str(), textObj, false, "");
				dc->SetTextColor(TextWhite.ToCOLORREF());
			}
			else if (!primedPlan->IsCleared && (i != SEL_ATCR_LCHG && i != SEL_ATCR_MCHG && i != SEL_ATCR_UNABLE && i != SEL_ATCR_ATA && i != SEL_ATCR_ATB && i != SEL_ATCR_XAT)) {
				screen->AddScreenObject(WIN_FLTPLN, to_string(i).c_str(), textObj, false, "");
				dc->SetTextColor(TextWhite.ToCOLORREF());
			}
		}

		// Text out
		dc->TextOutA(restrictions.left + 2, offsetY, restrictionSelections[i].c_str());

		// Offset
		offsetY += dc->GetTextExtent(restrictionSelections[i].c_str()).cy + 2;
	}


	// Create content panel
	CRect content(restrictions.right + 7, titleRect.bottom + 22, restrictions.right + ((WINSZ_FLTPLN_WIDTH_MDL / 3) * 1.945), titleRect.bottom + WINSZ_FLTPLN_HEIGHT_ATCR - 63);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(content, -1, -1);
	dc->Draw3dRect(content, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Select font
	FontSelector::SelectNormalFont(14, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Fill content panel
	offsetY = content.top + 2;
	if (!plan->Restrictions.empty()) {
		for (int i = 600; i <= 600 + plan->Restrictions.size() - 1; i++) {
			size_t extent = dc->GetTextExtent("ABCD").cy;
			// Register click
			CRect textObj(content.left, offsetY, content.right - 1, offsetY + extent);
			if (i == selectedActiveRestriction)
				dc->FillSolidRect(textObj, ButtonPressed.ToCOLORREF());

			// Text out
			dc->TextOutA(content.left + 2, offsetY, plan->Restrictions[i - 600].Human.c_str());

			screen->AddScreenObject(WIN_FLTPLN, to_string(i).c_str(), textObj, false, "");

			// Offset
			offsetY += extent + 2;
		}
	}

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

	if (primedPlan->Callsign.size() == 0)
		return;

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
	if (selectedAuthority != "" && windowButtons[BTN_XCHANGE_TRANSFER].State == CInputState::DISABLED && screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerIsMe())
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

	// Select font
	FontSelector::SelectNormalFont(15, dc);

	// List content
	int offsetX = 2;
	int offsetY = 2;
	int contentoffsetY = offsetY;
	for (auto kv : onlineControllers) {
		CRect rect(content.left, content.top + offsetY, content.right, content.top + offsetY + dc->GetTextExtent("ABCD").cy);
		if (!(contentoffsetY < scrollBars[SCRL_XCHANGE].WindowPos))
		{
			if (!(contentoffsetY > scrollBars[SCRL_XCHANGE].WindowPos + scrollBars[SCRL_XCHANGE].FrameSize - 15))
			{
				if (kv.first == selectedAuthority)
					dc->FillSolidRect(rect, ButtonPressed.ToCOLORREF());
				dc->TextOutA(content.left + offsetX, content.top + offsetY, kv.second.GetPositionId());
				offsetX += 45;
				dc->TextOutA(content.left + offsetX, content.top + offsetY, kv.second.GetCallsign());
				offsetX = 0;
				offsetY += dc->GetTextExtent("ABCD").cy;
				screen->AddScreenObject(WIN_FLTPLN_TSFR, kv.second.GetCallsign(), rect, true, "");
			}
		}
		contentoffsetY += dc->GetTextExtent("ABCD").cy;
	}

	// Scroll bar values
	if (scrollBars[SCRL_XCHANGE].FrameSize == 0 || (scrollBars[SCRL_XCHANGE].ContentSize != contentoffsetY && scrollBars[SCRL_XCHANGE].PositionDelta == 0 && scrollBars[SCRL_XCHANGE].ContentRatio != 1))
	{
		int framebox = content.Height();
		scrollBars[SCRL_XCHANGE] = CWinScrollBar(SCRL_XCHANGE, WIN_FLTPLN, contentoffsetY, framebox, false);
		if (contentoffsetY != framebox)
		{
			scrollBars[SCRL_XCHANGE].GripSize -= 30; // temporary fix for grip bug
			scrollBars[SCRL_XCHANGE].TotalScrollableArea = framebox - scrollBars[SCRL_XCHANGE].GripSize;
		}
		if (scrollBars[SCRL_XCHANGE].ContentRatio > 1)
		{
			scrollBars[SCRL_XCHANGE] = CWinScrollBar(SCRL_XCHANGE, WIN_FLTPLN, framebox, framebox, false);
		}
	}

	// Draw scroll bar
	CCommonRenders::RenderScrollBar(dc, g, screen, { content.right + 3, content.top - 1 }, &scrollBars[SCRL_XCHANGE]);

	// Get authorities
	string currentController = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
	textInputs[TXT_XCHANGE_CURRENT].Content = currentController != "" ? currentController : "UNTRACKED";
	textInputs[TXT_XCHANGE_NEXT].Content = selectedAuthority != "" ? selectedAuthority : "NONE";

	// Set buttons
	if (primedPlan->CurrentMessage != nullptr) {
		if ((primedPlan->CurrentMessage->Type == CMessageType::LOG_ON || primedPlan->CurrentMessage->Type == CMessageType::TRANSFER)) {
			if (windowButtons[BTN_XCHANGE_ACCEPT].State == CInputState::DISABLED && windowButtons[BTN_XCHANGE_REJECT].State == CInputState::DISABLED) {
				windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::INACTIVE;
				windowButtons[BTN_XCHANGE_REJECT].State = CInputState::INACTIVE;
			}
		}
	}


	string controller = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign();
	string controllerMe = screen->GetPlugIn()->ControllerMyself().GetCallsign();
	if ((string(screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign()) != "" && !screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerIsMe())) {
		windowButtons[BTN_XCHANGE_TRANSFER].State = CInputState::DISABLED;
		windowButtons[BTN_XCHANGE_TRACK].State = CInputState::DISABLED;
		if (screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerIsMe() && string(screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign()) == "")
			windowButtons[BTN_XCHANGE_TRACK].Label = "Release";
		else
			windowButtons[BTN_XCHANGE_TRACK].Label = "Track";
	}
	else {
		windowButtons[BTN_XCHANGE_TRACK].State = CInputState::INACTIVE;
	}

	if (controller == controllerMe) {
		windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::INACTIVE;
		windowButtons[BTN_XCHANGE_REJECT].State = CInputState::INACTIVE;
	}

	if (controller == controllerMe) {
		windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::INACTIVE;
		windowButtons[BTN_XCHANGE_REJECT].State = CInputState::INACTIVE;
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

void CFlightPlanWindow::RenderATCRestrictSubModal(CDC* dc, Graphics* g, CRadarScreen* screen, POINT topLeft)
{
	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create restrictions sub window
	CRect atcrWindow(topLeft.x, topLeft.y, topLeft.x + WINSZ_FLTPLN_WIDTH_MDL, topLeft.y + WINSZ_FLTPLN_HEIGHT_ATCR/2);
	dc->FillRect(atcrWindow, &darkerBrush);
	dc->Draw3dRect(atcrWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(atcrWindow, -1, -1);
	dc->Draw3dRect(atcrWindow, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Create titlebar
	CRect titleRect(atcrWindow.left, atcrWindow.top, atcrWindow.left + WINSZ_FLTPLN_WIDTH_MDL, atcrWindow.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH_MDL / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), (string("ATC/ " + restrictionSelections[RestrictionSubModalType] + " - " + primedPlan->Callsign).c_str()));
	screen->AddScreenObject(WIN_FLTPLN, to_string(SUBWIN_ATCR).c_str(), titleRect, true, "");

	// Select font
	FontSelector::SelectNormalFont(15, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	int offsetX = atcrWindow.left + 5;
	int offsetYTopRow = titleRect.bottom + 5;
	int offsetYBottomRow = offsetYTopRow + 20;

	dc->TextOutA(offsetX, offsetYBottomRow + 2, restrictionSelections[RestrictionSubModalType].c_str());

	offsetX += dc->GetTextExtent(restrictionSelections[RestrictionSubModalType].c_str()).cx + 5;

	if (RestrictionSubModalType == SEL_ATCR_LCHG || RestrictionSubModalType == SEL_ATCR_MCHG) {
		int elements[3] = { TXT_RESTRI_LCHG_LATLON, CHK_RESTRI_LCHG, TXT_RESTRI_LCHG_TIME };

		if (RestrictionSubModalType == SEL_ATCR_MCHG) {
			elements[0] = TXT_RESTRI_MCHG_LATLON;
			elements[1] = CHK_RESTRI_MCHG;
			elements[2] = TXT_RESTRI_MCHG_TIME;
		}

		for (int k = 0; k < 3; k++) {
			if (k == 0 || k == 2) {
				int InputWidth = k == 0 ? 70 : 40;
				CCommonRenders::RenderTextInput(dc, screen, { offsetX, offsetYBottomRow }, InputWidth, 20, &textInputs[elements[k]]);

				dc->TextOutA(offsetX, offsetYTopRow, textInputs[elements[k]].Label.c_str());
				offsetX += InputWidth + 5 + 20;
			}

			if (k == 1) {
				CRect r = CCommonRenders::RenderCheckBox(dc, g, screen, { offsetX + 10, offsetYBottomRow }, 20, &checkBoxes[elements[k]]);

				dc->TextOutA(offsetX, offsetYTopRow, checkBoxes[elements[k]].Label.c_str());
				offsetX += r.Width() + 10 + 20;
			}

		}
	}
	if (RestrictionSubModalType == SEL_ATCR_UNABLE) {
		for (int k = CHK_RESTRI_UNABLE_SPD; k <= CHK_RESTRI_UNABLE_RTE; k++) {
			CRect r = CCommonRenders::RenderCheckBox(dc, g, screen, { offsetX + 10, offsetYBottomRow }, 20, &checkBoxes[k]);

			dc->TextOutA(offsetX, offsetYTopRow, checkBoxes[k].Label.c_str());
			offsetX += r.Width() + 10 + 20;
		}
	}
	if (RestrictionSubModalType == SEL_ATCR_INT) {
		int InputWidth = 70;
		CCommonRenders::RenderTextInput(dc, screen, { offsetX, offsetYBottomRow }, InputWidth, 20, &textInputs[TXT_RESTRI_INT_CALLSIGN]);

		dc->TextOutA(offsetX, offsetYTopRow, textInputs[TXT_RESTRI_INT_CALLSIGN].Label.c_str());
		offsetX += InputWidth + 10;

		dc->TextOutA(offsetX, offsetYBottomRow+3, "+");

		offsetX += 15;

		InputWidth = 40;
		CCommonRenders::RenderTextInput(dc, screen, { offsetX, offsetYBottomRow }, InputWidth, 20, &textInputs[TXT_RESTRI_INT_INTERVAL]);

		dc->TextOutA(offsetX, offsetYTopRow, textInputs[TXT_RESTRI_INT_INTERVAL].Label.c_str());
	}
	if (RestrictionSubModalType == SEL_ATCR_ATA || RestrictionSubModalType == SEL_ATCR_ATB || RestrictionSubModalType == SEL_ATCR_XAT) {
		int firstElement = TXT_RESTRI_ATA_LATLON;
		int lastElement = TXT_RESTRI_ATA_TIME;

		if (RestrictionSubModalType == SEL_ATCR_ATB) {
			firstElement = TXT_RESTRI_ATB_LATLON;
			lastElement = TXT_RESTRI_ATB_TIME;
		}

		if (RestrictionSubModalType == SEL_ATCR_XAT) {
			firstElement = TXT_RESTRI_XAT_LATLON;
			lastElement = TXT_RESTRI_XAT_TIME;
		}

		for (int k = firstElement; k <= lastElement; k++) {
			int InputWidth = k == firstElement ? 70 : 40;
			CCommonRenders::RenderTextInput(dc, screen, {offsetX, offsetYBottomRow }, InputWidth, 20, &textInputs[k]);

			dc->TextOutA(offsetX, offsetYTopRow, textInputs[k].Label.c_str());
			offsetX += InputWidth + 5 + 20;
		}
	}

	// Draw buttons
	offsetX = atcrWindow.left + 5;
	for (int idx = BTN_RESTRI_EDIT_OK; idx >= BTN_RESTRI_EDIT_CLOSE; idx--) {
		// Draw the button
		CCommonRenders::RenderButton(dc, screen, { offsetX, atcrWindow.bottom - 35 }, 75, 30, &windowButtons.at(idx));
		offsetX += 80;
	}

	// Buttons Panel
	CRect ButtonsPanel(atcrWindow.left, atcrWindow.bottom - 45, atcrWindow.right, atcrWindow.bottom);
	dc->Draw3dRect(ButtonsPanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(ButtonsPanel, -1, -1);
	dc->Draw3dRect(ButtonsPanel, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

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
	if (!fp->IsValid) {
		primedPlan = new CAircraftFlightPlan();
		return;
	}
	else {
		primedPlan = fp;
	}

	// Add message
	if (msg != nullptr) {
		primedPlan->CurrentMessage = msg;
		IsMessageOpen = true;
		windowButtons[BTN_MSG].State = CInputState::INACTIVE;
	}
	else if (primedPlan->CurrentMessage != nullptr) {
		IsMessageOpen = true;
		windowButtons[BTN_MSG].State = CInputState::INACTIVE;
	}
	else {
		IsMessageOpen = false;
		windowButtons[BTN_MSG].State = CInputState::DISABLED;
	}

	if (!fp->IsCleared) {
		string route;
		if (!primedPlan->Route.empty()) {
			// Set route
			for (int i = 0; i < fp->Route.size(); i++) {
				route += primedPlan->Route[i].Name + " ";
			}
			route.pop_back(); // Get rid of extra space
		}
		// Set route
		if (primedPlan->Track == "RR")
			SetTextValue(screen, TXT_MAN_RTE, route);
		else 
			SetTextValue(screen, TXT_MAN_TCK, primedPlan->Track);

		// Set destination
		if (textInputs.at(TXT_MAN_DEST).Content == "") {
			textInputs.at(TXT_MAN_DEST).Content = primedPlan->Dest;
		}
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
		route.pop_back(); // Get rid of extra space

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
		textInputs[TXT_RTE].State == CInputState::DISABLED;

		if (fp->State == "CPY") {
			primedPlan->State = "CPY";

			// Set copied plan
			copiedPlan = *primedPlan;
			copiedPlan.State = "UA";

			// Values
			SetTextValue(screen, CFlightPlanWindow::TXT_SPD_CPY, copiedPlan.Mach);
			SetTextValue(screen, CFlightPlanWindow::TXT_LEVEL_CPY, copiedPlan.FlightLevel);
			SetTextValue(screen, CFlightPlanWindow::TXT_DEST_CPY, copiedPlan.Dest);
			SetTextValue(screen, CFlightPlanWindow::TXT_STATE_CPY, copiedPlan.State);
			if (copiedPlan.Track != "RR") {
				SetTextValue(screen, CFlightPlanWindow::TXT_TCK_CPY, copiedPlan.Track);
			}
			else {
				SetTextValue(screen, CFlightPlanWindow::TXT_CPY_RTE, route);
			}

			SetButtonState(CFlightPlanWindow::BTN_DELETE, CInputState::INACTIVE);
			SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::DISABLED);
			IsCopyMade = true;
			textInputs[TXT_TCK_CPY].State = CInputState::ACTIVE;
			SetButtonState(CFlightPlanWindow::BTN_PROBE, CInputState::INACTIVE);
		}
	}
	else {
		if (primedPlan->CurrentMessage != nullptr && primedPlan->CurrentMessage->Type == CMessageType::CLEARANCE_REQ && msg == nullptr) {
			// Set route
			string route;
			for (int i = 0; i < fp->RouteRaw.size(); i++) {
				route += fp->RouteRaw[i] + " ";
			}
			route = route.substr(0, route.size() - 1); // Get rid of extra space
			IsData = true;
			SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::DISABLED);
			// Instantiate flight plan variables
			SetTextValue(screen, CFlightPlanWindow::TXT_SPD, primedPlan->Mach);
			SetTextValue(screen, CFlightPlanWindow::TXT_LEVEL, primedPlan->FlightLevel);
			SetTextValue(screen, CFlightPlanWindow::TXT_DEST, primedPlan->Dest);
			SetTextValue(screen, CFlightPlanWindow::TXT_STATE, "UA");
			SetButtonState(CFlightPlanWindow::BTN_PROBE, CInputState::INACTIVE);
			if (primedPlan->Track != "RR") {
				SetTextValue(screen, CFlightPlanWindow::TXT_TCK, primedPlan->Track);
			}
			else {
				SetTextValue(screen, CFlightPlanWindow::TXT_RTE, route);
			}
		}
		else {
			if (!primedPlan->Route.empty()) {
				// Set route
				string route;
				for (int i = 0; i < fp->Route.size(); i++) {
					route += primedPlan->Route[i].Name + " ";
				}
				route.pop_back(); // Get rid of extra space
				IsData = true;
				SetTextValue(screen, CFlightPlanWindow::TXT_SPD, primedPlan->Mach);
				SetTextValue(screen, CFlightPlanWindow::TXT_LEVEL, primedPlan->FlightLevel);
				SetTextValue(screen, CFlightPlanWindow::TXT_DEST, primedPlan->Dest);
				SetTextValue(screen, CFlightPlanWindow::TXT_STATE, "UA");
				SetButtonState(CFlightPlanWindow::BTN_PROBE, CInputState::INACTIVE);
				if (primedPlan->Track != "RR") {
					SetTextValue(screen, CFlightPlanWindow::TXT_TCK, primedPlan->Track);
				}
				else {
					SetTextValue(screen, CFlightPlanWindow::TXT_RTE, route);
				}
			}
			else
				IsData = false;
			SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::DISABLED);
		}
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
	textInputs[TXT_DATALINK].Content = primedPlan->DLStatus == "" ? "OFFLINE" : "ONLINE";


	// If tracked by other controller
	string currentController = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
	if (!screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerIsMe()) {
		SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::DISABLED);
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_NOTIFY, CInputState::DISABLED);
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_TRACK, CInputState::DISABLED);
		windowButtons[BTN_XCHANGE_TRACK].Label = "Track";
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_TRANSFER, CInputState::DISABLED);
	}
	else {
		SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::INACTIVE);
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_NOTIFY, CInputState::INACTIVE);
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_TRACK, CInputState::INACTIVE);
		windowButtons[BTN_XCHANGE_TRACK].Label = "Release";
	}

	if (currentController == "" || (currentController != "" && string(screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign()) != "")) {
		SetButtonState(CFlightPlanWindow::BTN_COPY, CInputState::INACTIVE);
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_NOTIFY, CInputState::DISABLED);
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_TRACK, CInputState::INACTIVE);
		windowButtons[BTN_XCHANGE_TRACK].Label = "Track";
		SetButtonState(CFlightPlanWindow::BTN_XCHANGE_TRANSFER, CInputState::DISABLED);
	}
}

void CFlightPlanWindow::ParseRestriction(string content, CRestrictionType type) {
	CFlightRestriction restriction;
	// Parse restriction
	if (type == CRestrictionType::LCHG) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);

		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "LCHG AT ";
		if (splitString.empty())
			restriction.Human += content;
		else
			restriction.Human += splitString[0] + " " + splitString[1] + "Z";
	}
	else if (type == CRestrictionType::MCHG) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);

		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "MCHG AT ";
		if (splitString.empty())
			restriction.Human += content;
		else
			restriction.Human += splitString[0] + " " + splitString[1] + "Z";
	}
	else if (type == CRestrictionType::RERUTE) {
		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "RERUTE VIA CURRENT ENTERED RTE";
	}
	else if (type == CRestrictionType::UNABLE) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);
		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "UNABLE ";
		if (splitString.empty()) {
			if (content == "MCHG") {
				restriction.Human += "SPD REQ";
			}
			else if (content == "LCHG") {
				restriction.Human += "LVL REQ";
			}
			else {
				restriction.Human += "RERUTE REQ";
			}
		}
		else {
			int counter = 0;
			for (int i = 0; i < splitString.size(); i++) {
				if (splitString[i] == "MCHG") {
					restriction.Human += "SPD REQ";
					counter++;
				}
				else if (splitString[i] == "LCHG") {
					if (counter > 1)
						restriction.Human += ", LVL REQ";
					else
						restriction.Human += "LVL REQ";
					counter++;
				}
				else if (splitString[i] == "RERUTE") {
					if (counter > 1)
						restriction.Human += ", RERUTE REQ";
					else
						restriction.Human += "RERUTE REQ";
				}
			}
		}
	}
	else if (type == CRestrictionType::ATA) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);

		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "AT " + splitString[0] + " AFTR " + splitString[1] + "Z";
	}
	else if (type == CRestrictionType::ATB) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);
		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "AT " + splitString[0] + " BEFR " + splitString[1] + "Z";
	}
	else if (type == CRestrictionType::XAT) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);
		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "CROSS " + splitString[0] + " AT " + splitString[1] + "Z";
	}
	else if (type == CRestrictionType::INT) {
		// Split
		vector<string> splitString;
		if (content.find(':') != string::npos)
			CUtils::StringSplit(content, ':', &splitString);
		// Assign
		restriction.Content = content;
		restriction.Type = type;
		restriction.Human += "INT " + splitString[0] + " + " + splitString[1];
	}
	else if (type == CRestrictionType::EPC) { // Don't implement yet

	}
	else if (type == CRestrictionType::RTD) { // Don't implement yet

	}

	// Now add the restriction
	if (IsCopyMade)
		copiedPlan.Restrictions.push_back(restriction);
	else
		primedPlan->Restrictions.push_back(restriction);
}

void CFlightPlanWindow::SetTextValue(CRadarScreen* screen, int id, string content) {
	// Mach numbers
	if (id == TXT_SPD || id == TXT_SPD_CPY || id == TXT_MAN_SPD) {
		if (content == "") return;
		// Validation (range & length)
		// Check if it is a string
		bool isNumber = true;
		for (int i = 0; i < content.size(); i++) { // Check if string
			if (!isdigit(content[i])) return;
		}
		// It's a number, check the length
		if (stoi(content) > 200 || stoi(content) < 1) return;

		if (id == TXT_SPD || id == TXT_MAN_SPD) {
			primedPlan->Mach = content;
		}
		else if (id == TXT_SPD_CPY) {
			copiedPlan.Mach = content;
		}
		// We reached here so set the mach
		content = string("M") + CUtils::PadWithZeros(3, stoi(content));
	}

	// Mach numbers
	if (id == TXT_LEVEL || id == TXT_LEVEL_CPY || id == TXT_MAN_FL) {
		if (content == "") return;
		// Validation (range & length)
		// Check if it is a string
		bool isNumber = true;
		for (int i = 0; i < content.size(); i++) { // Check if string
			if (!isdigit(content[i])) return;
		}
		// It's a number, check the length
		if (stoi(content) > 999 || stoi(content) < 1) return;

		// Assign
		if (id == TXT_LEVEL || id == TXT_MAN_FL) {
			primedPlan->FlightLevel = content;
		}
		else if (id == TXT_LEVEL_CPY) {
			copiedPlan.FlightLevel = content;
		}
	}
	// Tracks
	if (id == TXT_TCK || id == TXT_TCK_CPY || id == TXT_MAN_TCK) {
		if (content == "") return;
		// Validation (range & length)
		// Check if it is a string
		bool isNumber = false;
		for (int i = 0; i < content.size(); i++) { // Check if string
			if (!isalpha(content[i])) return;
			else content[i] = toupper(content[i]);
		}

		// If the input was "RR" then return 
		if (content == "RR") return;

		// It's a string, check the length
		if (content.size() > 3 || content.size() < 1) return;

		int status;
		if (id == TXT_TCK_CPY) {
			status = CRoutesHelper::ParseRoute(copiedPlan.Callsign, content, true, &copiedPlan);
		}
		else {
			status = CRoutesHelper::ParseRoute(primedPlan->Callsign, content, true);
		}
		if (status == 1) { // Validation failed
			textInputs.find(id)->second.Error = true;
		}
		else {
			if (status == 0) { // Validation success
				// Generate the route
				CUtils::CAsyncData* data = new CUtils::CAsyncData();
				data->Screen = screen;
				data->Callsign = primedPlan->Callsign;
				data->FP = id == TXT_TCK_CPY ? &copiedPlan : nullptr;
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
		if (content == "") return;
		// Parse the route
		int status;
		if (id == TXT_CPY_RTE) {
			status = CRoutesHelper::ParseRoute(copiedPlan.Callsign, content, false, &copiedPlan);
		}
		else {
			status = CRoutesHelper::ParseRoute(primedPlan->Callsign, content);
		}

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
				data->FP = id == TXT_CPY_RTE ? &copiedPlan : nullptr;
				_beginthread(CRoutesHelper::InitialiseRoute, 0, (void*) data); // Async

				// Set the error to false
				textInputs.find(id)->second.Error = false;

				// Set the contents
				if (id == TXT_RTE) {
					textInputs.find(TXT_TCK)->second.Content = primedPlan->Track;
					textInputs.find(TXT_TCK)->second.Error = false;
				}
				else if (id == TXT_CPY_RTE) {
					textInputs.find(TXT_TCK_CPY)->second.Content = copiedPlan.Track;
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
		for (int i = 0; i < content.size(); i++) { // Check if string
			if (isalpha(content[i])) 
				content[i] = toupper(content[i]);
		}
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
			SetButtonState(BTN_XCHANGE_TRACK, CInputState::DISABLED);

			// Temporary until messages implemented
			SetButtonState(BTN_READBK, CInputState::INACTIVE);

			// Set copy
			textInputs[TXT_STATE_CPY].Content = "UA";
			primedPlan->State = "CPY";

			// Set copied plan
			copiedPlan = *primedPlan;
			copiedPlan.State = "UA";

			// Set route
			string route;
			for (int i = 0; i < primedPlan->RouteRaw.size(); i++) {
				route += primedPlan->RouteRaw[i] + " ";
			}

			textInputs[TXT_SPD_CPY].Content = string("M") + CUtils::PadWithZeros(3, stoi(copiedPlan.Mach));
			textInputs[TXT_LEVEL_CPY].Content = copiedPlan.FlightLevel;
			textInputs[TXT_DEST_CPY].Content = copiedPlan.Dest;
			textInputs[TXT_TCK_CPY].State = CInputState::ACTIVE;
			if (primedPlan->Track != "RR") {
				SetTextValue(screen, CFlightPlanWindow::TXT_TCK_CPY, primedPlan->Track);
			}
			else {
				SetTextValue(screen, CFlightPlanWindow::TXT_CPY_RTE, route);
			}

			// Flag
			stateSetManually = true;
		}
		if (id == BTN_DELETE) {
			IsCopyMade = false; // Delete copy
			IsConflictWindow = false;
			IsClearanceOpen = false;
			primedPlan->State = "";
			// Set the states
			SetButtonState(BTN_DELETE, CInputState::DISABLED);
			SetButtonState(BTN_COPY, CInputState::INACTIVE);
			SetButtonState(BTN_PROBE, CInputState::DISABLED);

			// Set input contents to zero
			textInputs[TXT_SPD_CPY].Content = "";
			textInputs[TXT_LEVEL_CPY].Content = "";
			textInputs[TXT_DEST_CPY].Content = "";
			textInputs[TXT_TCK_CPY].Content = "";
			textInputs[TXT_CPY_RTE].Content = "";
			copiedPlan.IsValid = false;
			stateSetManually = true;
		}
		if (id == BTN_PROBE) {
			IsConflictWindow = true;
			SetButtonState(BTN_MSG_REQUEUE, CInputState::DISABLED);
			SetButtonState(BTN_PROBE, CInputState::DISABLED);
			CConflictDetection::ProbeTool(screen, primedPlan->Callsign, &currentProbeStatuses, IsCopyMade ? &copiedPlan : nullptr);

			if (currentProbeStatuses.empty()) {
				SetButtonState(BTN_CONF_ACCCL, CInputState::DISABLED);
				SetButtonState(BTN_CONF_MANCL, CInputState::DISABLED);
			}
			else {
				SetButtonState(BTN_CONF_MANCL, CInputState::DISABLED);
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
			SetButtonState(BTN_ATCR, CInputState::DISABLED);
			dropDowns[DRP_ATCR].State = CInputState::DISABLED;
			textInputs[TXT_SPD].State = CInputState::DISABLED;
			textInputs[TXT_LEVEL].State = CInputState::DISABLED;
			textInputs[TXT_DEST].State = CInputState::DISABLED;
			textInputs[TXT_TCK].State = CInputState::DISABLED;
			textInputs[TXT_RTE].State = CInputState::DISABLED;

			// Switch the clearance type
			CMessageType type;
			if (primedPlan->CurrentMessage != nullptr && primedPlan->CurrentMessage->Type == CMessageType::CLEARANCE_REQ) {
				type = CMessageType::CLEARANCE_ISSUE;
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = true;
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
				checkBoxes.at(CHK_CLRC_ORCA).State = CInputState::INACTIVE;
				checkBoxes.at(CHK_CLRC_CPDLC).State = CInputState::DISABLED;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;
				SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
			}
			else if (IsCopyMade || (primedPlan->CurrentMessage != nullptr && primedPlan->CurrentMessage->Type == CMessageType::REVISION_REQ)) {
				type = CMessageType::REVISION_ISSUE;
				checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
				checkBoxes.at(CHK_CLRC_CPDLC).State = CInputState::INACTIVE;
				checkBoxes.at(CHK_CLRC_ORCA).State = CInputState::DISABLED;
				checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = true;
				checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
				checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;
				SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
				SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
			}
			IsClearanceOpen = true;
			currentClearanceText = CUtils::ParseToPhraseology(CUtils::ParseToRaw(primedPlan->Callsign, type, IsCopyMade ? &copiedPlan : nullptr), type, primedPlan->Callsign);
		}
		if (id == BTN_CLRC_REJECT) {
			SetButtonState(BTN_PROBE, CInputState::INACTIVE);
			if (IsCopyMade) {
				SetButtonState(BTN_DELETE, CInputState::INACTIVE);
				SetButtonState(BTN_ATCR_CPY, CInputState::INACTIVE);
				dropDowns[DRP_ATCR_CPY].State = CInputState::INACTIVE;
				textInputs[TXT_SPD_CPY].State = CInputState::INACTIVE;
				textInputs[TXT_LEVEL_CPY].State = CInputState::INACTIVE;
				textInputs[TXT_DEST_CPY].State = CInputState::INACTIVE;
				textInputs[TXT_TCK_CPY].State = CInputState::INACTIVE;
				textInputs[TXT_CPY_RTE].State = CInputState::INACTIVE;
			}
			else {
				SetButtonState(BTN_ATCR, CInputState::INACTIVE);
				dropDowns[DRP_ATCR].State = CInputState::INACTIVE;
				textInputs[TXT_SPD].State = CInputState::INACTIVE;
				textInputs[TXT_LEVEL].State = CInputState::INACTIVE;
				textInputs[TXT_DEST].State = CInputState::INACTIVE;
				textInputs[TXT_TCK].State = CInputState::INACTIVE;
				textInputs[TXT_RTE].State = CInputState::INACTIVE;
			}

			IsClearanceOpen = false;
		}
		if (id == BTN_CLRC_SEND) {
			SetButtonState(BTN_UNCLEAR, CInputState::DISABLED);
			SetButtonState(BTN_READBK, CInputState::INACTIVE);
			IsClearanceOpen = false;
			if (primedPlan->CurrentMessage != nullptr && primedPlan->CurrentMessage->Type == CMessageType::CLEARANCE_REQ) {

				primedPlan->IsCleared = true;
				primedPlan->State = "PC";
				SetButtonState(BTN_ATCR, CInputState::DISABLED);
				dropDowns[DRP_ATCR].State = CInputState::DISABLED;
				textInputs[TXT_SPD].State = CInputState::DISABLED;
				textInputs[TXT_LEVEL].State = CInputState::DISABLED;
				textInputs[TXT_DEST].State = CInputState::DISABLED;
				textInputs[TXT_TCK].State = CInputState::DISABLED;
				textInputs[TXT_RTE].State = CInputState::DISABLED;
				textInputs[TXT_STATE].Content = "PC";
			}
			if (IsCopyMade) {
				copiedPlan.IsValid = false;
				IsCopyMade = false;
				copiedPlan.Restrictions.clear();
				primedPlan->FlightLevel = copiedPlan.FlightLevel;
				primedPlan->Mach = copiedPlan.Mach;
				primedPlan->Track = copiedPlan.Track;
				primedPlan->RouteRaw = copiedPlan.RouteRaw;
			}
			else {
				primedPlan->Restrictions.clear();
			}
		}
		if (id == BTN_CLRC_VOICE) {
			checkBoxes.at(CHK_CLRC_ORCA).State = CInputState::DISABLED;
			checkBoxes.at(CHK_CLRC_CPDLC).State = CInputState::DISABLED;
			checkBoxes.at(CHK_CLRC_VOX).State = CInputState::DISABLED;
			checkBoxes.at(CHK_CLRC_TXT).State = CInputState::DISABLED;
			SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
			SetButtonState(BTN_CLRC_READBK, CInputState::INACTIVE);
			SetButtonState(BTN_ATCR, CInputState::DISABLED);
			dropDowns[DRP_ATCR].State = CInputState::DISABLED;
			textInputs[TXT_SPD].State = CInputState::DISABLED;
			textInputs[TXT_LEVEL].State = CInputState::DISABLED;
			textInputs[TXT_DEST].State = CInputState::DISABLED;
			textInputs[TXT_TCK].State = CInputState::DISABLED;
			textInputs[TXT_RTE].State = CInputState::DISABLED;
		}
		if (id == BTN_READBK || id == BTN_CLRC_READBK) {
			if (windowButtons[BTN_READBK].State != CInputState::DISABLED) {
				/// Temporary until messages implemented
				SetButtonState(BTN_PROBE, CInputState::DISABLED);
				IsConflictWindow = false;
				if (IsCopyMade) { // Temporary
					// Copy the values to the primed plan & set the txt boxes
					primedPlan->Mach = copiedPlan.Mach;
					primedPlan->FlightLevel = copiedPlan.FlightLevel;
					primedPlan->Dest = copiedPlan.Dest;
					primedPlan->Track = copiedPlan.Track;
					primedPlan->RouteRaw = copiedPlan.RouteRaw;

					// Set route
					string route;
					for (int i = 0; i < primedPlan->RouteRaw.size(); i++) {
						route += primedPlan->RouteRaw[i] + " ";
					}

					// Set input contents
					textInputs[TXT_SPD].Content = string("M") + CUtils::PadWithZeros(3, stoi(primedPlan->Mach));
					textInputs[TXT_LEVEL].Content = primedPlan->FlightLevel;
					textInputs[TXT_DEST].Content = primedPlan->Dest;
					if (primedPlan->Track == "RR")
						SetTextValue(screen, CFlightPlanWindow::TXT_RTE, route);
					else
						SetTextValue(screen, CFlightPlanWindow::TXT_TCK, primedPlan->Track);

					textInputs[TXT_RTE].State == CInputState::DISABLED;
				}

				IsClearanceOpen = false;
				IsCopyMade = false;
				primedPlan->State = "";
				textInputs[TXT_STATE].Content = "";
				copiedPlan.IsValid = false;
				SetButtonState(BTN_MSG_DONE, CInputState::INACTIVE);
				SetButtonState(BTN_READBK, CInputState::DISABLED);
				SetButtonState(BTN_DELETE, CInputState::DISABLED);
				dropDowns[DRP_ATCR].State = CInputState::DISABLED;
				SetButtonState(BTN_COPY, CInputState::INACTIVE);				

				// Until messages are implemented, we clear them here in the system and disable all text inputs
				if (!primedPlan->IsCleared) {
					primedPlan->IsCleared = true;
					textInputs[TXT_SPD].State = CInputState::DISABLED;
					textInputs[TXT_LEVEL].State = CInputState::DISABLED;
					textInputs[TXT_DEST].State = CInputState::DISABLED;
					textInputs[TXT_TCK].State = CInputState::DISABLED;
					textInputs[TXT_RTE].State = CInputState::DISABLED;

					// Create network object
					CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
					netFP->Callsign = primedPlan->Callsign;
					netFP->Type = primedPlan->Type;
					netFP->AssignedLevel = stoi(primedPlan->FlightLevel);
					netFP->AssignedMach = stoi(primedPlan->Mach);
					netFP->Track = primedPlan->Track;
					netFP->Departure = primedPlan->Depart;
					netFP->Arrival = primedPlan->Dest;
					netFP->Direction = CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
					netFP->Selcal = primedPlan->SELCAL == "N/A" ? "" : primedPlan->SELCAL;
					netFP->DatalinkConnected = false; // we can't tell from here so default
					netFP->IsEquipped = primedPlan->IsEquipped;
					netFP->State = primedPlan->State;
					netFP->Etd = primedPlan->Etd;
					if (primedPlan->IsRelevant || (screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetSectorExitMinutes() != -1)) {
						netFP->Relevant = false;
					}
					else {
						netFP->Relevant = true;
					}
					netFP->TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetRadarFlags());
					netFP->TrackedBy = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
					netFP->TrackedById = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerId();
					netFP->Route = ""; // Initialise
					netFP->RouteEtas = ""; // Initialise
					
					// Get routes and estimates
					vector<CRoutePosition> rte;
					CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
					for (int i = 0; i < rte.size(); i++) {
						if (i != rte.size() - 1) {
							netFP->Route += rte[i].Fix + " ";
							netFP->RouteEtas += rte[i].Estimate + " ";
						}
						else {
							netFP->Route += rte[i].Fix;
							netFP->RouteEtas += rte[i].Estimate;
						}							
					}

					// Post data to the database
					CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
					data->Screen = screen;
					data->Callsign = primedPlan->Callsign;
					data->FP = netFP;
					_beginthread(CDataHandler::PostNetworkAircraft, 0, (void*)data); // Async
				}
				else { // Delete this duplicate code nonsense
					// Create network object
					CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
					netFP->Callsign = copiedPlan.Callsign;
					netFP->Type = copiedPlan.Type;
					netFP->AssignedLevel = stoi(copiedPlan.FlightLevel);
					netFP->AssignedMach = stoi(copiedPlan.Mach);
					netFP->Track = copiedPlan.Track;
					netFP->Departure = copiedPlan.Depart;
					netFP->Arrival = copiedPlan.Dest;
					netFP->Direction = CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(copiedPlan.Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
					netFP->Selcal = copiedPlan.SELCAL == "N/A" ? "" : copiedPlan.SELCAL;
					netFP->DatalinkConnected = false; // we can't tell from here so default
					netFP->IsEquipped = copiedPlan.IsEquipped;
					netFP->State = copiedPlan.State;
					netFP->Etd = copiedPlan.Etd;
					if (primedPlan->IsRelevant || (screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetSectorExitMinutes() != -1)) {
						netFP->Relevant = true;
					}
					else {
						netFP->Relevant = false;
					}
					netFP->TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(copiedPlan.Callsign.c_str()).GetPosition().GetRadarFlags());
					netFP->TrackedBy = screen->GetPlugIn()->FlightPlanSelect(copiedPlan.Callsign.c_str()).GetTrackingControllerCallsign();
					netFP->TrackedById = screen->GetPlugIn()->FlightPlanSelect(copiedPlan.Callsign.c_str()).GetTrackingControllerId();
					netFP->Route = ""; // Initialise
					netFP->RouteEtas = ""; // Initialise

					// Get routes and estimates
					vector<CRoutePosition> rte;
					copiedPlan.IsValid = true;
					CRoutesHelper::GetRoute(screen, &rte, copiedPlan.Callsign, &copiedPlan);
					for (int i = 0; i < rte.size(); i++) {
						if (i != rte.size() - 1) {
							netFP->Route += rte[i].Fix + " ";
							netFP->RouteEtas += rte[i].Estimate + " ";
						}
						else {
							netFP->Route += rte[i].Fix;
							netFP->RouteEtas += rte[i].Estimate;
						}
					}

					// Post data to the database
					DWORD activeCode;
					HANDLE hnd = CUtils::GetESProcess();
					GetExitCodeProcess(hnd, &activeCode);
					// Check if the app is still active
					if (activeCode == STILL_ACTIVE) {
						CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
						data->Screen = screen;
						data->Callsign = primedPlan->Callsign;
						data->FP = netFP;
						_beginthread(CDataHandler::UpdateNetworkAircraft, 0, (void*)data); // Async
					}
				}
			}
		}
		if (id == BTN_XCHANGE_TRACK) {
			if (screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerIsMe()) {
				CLogger::Log(CLogType::NORM, "Attempting to drop aircraft " + primedPlan->Callsign + ".", "CRadarDisplay::OnRefresh");
				screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).EndTracking();
				windowButtons[BTN_XCHANGE_TRACK].Label = "Track";
				SetButtonState(BTN_XCHANGE_TRANSFER, CInputState::DISABLED);
				textInputs[TXT_XCHANGE_CURRENT].Content = "UNTRACKED";
				if (!primedPlan->IsCleared) {
					IsData = false;
					SetButtonState(BTN_MANENTRY, CInputState::INACTIVE);
				} else 
				{
					if (primedPlan->IsValid) {
						CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
						netFP->Callsign = primedPlan->Callsign;
						netFP->Type = primedPlan->Type;
						netFP->AssignedLevel = stoi(primedPlan->FlightLevel);
						netFP->AssignedMach = stoi(primedPlan->Mach);
						netFP->Track = primedPlan->Track;
						netFP->Departure = primedPlan->Depart;
						netFP->Arrival = primedPlan->Dest;
						netFP->Direction = CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
						netFP->Selcal = primedPlan->SELCAL == "N/A" ? "" : primedPlan->SELCAL;
						netFP->DatalinkConnected = primedPlan->DLStatus == "true" ? true : false;
						netFP->IsEquipped = primedPlan->IsEquipped;
						netFP->State = primedPlan->State;
						netFP->Etd = primedPlan->Etd;
						if (primedPlan->IsRelevant || (screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetSectorExitMinutes() != -1)) {
							netFP->Relevant = true;
						}
						else {
							netFP->Relevant = false;
						}
						netFP->TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetRadarFlags());
						netFP->TrackedBy = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
						netFP->TrackedById = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerId();
						netFP->Route = ""; // Initialise
						netFP->RouteEtas = ""; // Initialise

						// Get routes and estimates
						vector<CRoutePosition> rte;
						CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
						for (int i = 0; i < rte.size(); i++) {
							if (i != rte.size() - 1) {
								netFP->Route += rte[i].Fix + " ";
								netFP->RouteEtas += rte[i].Estimate + " ";
							}
							else {
								netFP->Route += rte[i].Fix;
								netFP->RouteEtas += rte[i].Estimate;
							}
						}

						// Post data to the database
						DWORD activeCode;
						HANDLE hnd = CUtils::GetESProcess();
						GetExitCodeProcess(hnd, &activeCode);
						// Check if the app is still active
						if (activeCode == STILL_ACTIVE) {
							CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
							data->Screen = screen;
							data->Callsign = primedPlan->Callsign;
							data->FP = netFP;
							_beginthread(CDataHandler::UpdateNetworkAircraft, 0, (void*)data); // Async
						}
					}
				}
				SetButtonState(BTN_MANENTRY, CInputState::INACTIVE);
			}
			else {
				CLogger::Log(CLogType::NORM, "Attempting to track aircraft " + primedPlan->Callsign + ".", "CRadarDisplay::OnRefresh");
				screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).StartTracking();
				windowButtons[BTN_XCHANGE_TRACK].Label = "Release";
				textInputs[TXT_XCHANGE_CURRENT].Content = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
				if (primedPlan->IsValid&& primedPlan->IsCleared) {
					CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
					netFP->Callsign = primedPlan->Callsign;
					netFP->Type = primedPlan->Type;
					netFP->AssignedLevel = stoi(primedPlan->FlightLevel);
					netFP->AssignedMach = stoi(primedPlan->Mach);
					netFP->Track = primedPlan->Track;
					netFP->Departure = primedPlan->Depart;
					netFP->Arrival = primedPlan->Dest;
					netFP->Direction = CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
					netFP->Selcal = primedPlan->SELCAL == "N/A" ? "" : primedPlan->SELCAL;
					netFP->DatalinkConnected = primedPlan->DLStatus == "true" ? true : false;
					netFP->IsEquipped = primedPlan->IsEquipped;
					netFP->State = primedPlan->State;
					netFP->Etd = primedPlan->Etd;
					if (primedPlan->IsRelevant || (screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetSectorExitMinutes() != -1)) {
						netFP->Relevant = true;
					}
					else {
						netFP->Relevant = false;
					}
					netFP->TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetRadarFlags());
					netFP->TrackedBy = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
					netFP->TrackedById = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerId();
					netFP->Route = ""; // Initialise
					netFP->RouteEtas = ""; // Initialise

					// Get routes and estimates
					vector<CRoutePosition> rte;
					CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
					for (int i = 0; i < rte.size(); i++) {
						if (i != rte.size() - 1) {
							netFP->Route += rte[i].Fix + " ";
							netFP->RouteEtas += rte[i].Estimate + " ";
						}
						else {
							netFP->Route += rte[i].Fix;
							netFP->RouteEtas += rte[i].Estimate;
						}
					}

					// Post data to the database
					CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
					data->Screen = screen;
					data->Callsign = primedPlan->Callsign;
					data->FP = netFP;
					_beginthread(CDataHandler::UpdateNetworkAircraft, 0, (void*)data); // Async
				}
			}
		}
		if (id == BTN_XCHANGE_TRANSFER) {
			CLogger::Log(CLogType::NORM, "Initiating handoff of aircraft " + primedPlan->Callsign + " to station " + selectedAuthority + ".", "CRadarDisplay::OnRefresh");
			screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).InitiateHandoff(selectedAuthority.c_str());
			SetButtonState(BTN_XCHANGE_TRANSFER, CInputState::DISABLED);
			windowButtons[BTN_XCHANGE_TRACK].Label = "Track";
			selectedAuthority = "";
			if (primedPlan->IsValid && primedPlan->IsCleared) {
				CNetworkFlightPlan* netFP = new CNetworkFlightPlan();
				netFP->Callsign = primedPlan->Callsign;
				netFP->Type = primedPlan->Type;
				netFP->AssignedLevel = stoi(primedPlan->FlightLevel);
				netFP->AssignedMach = stoi(primedPlan->Mach);
				netFP->Track = primedPlan->Track;
				netFP->Departure = primedPlan->Depart;
				netFP->Arrival = primedPlan->Dest;
				netFP->Direction = CUtils::GetAircraftDirection(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetReportedHeadingTrueNorth());
				netFP->Selcal = primedPlan->SELCAL == "N/A" ? "" : primedPlan->SELCAL;
				netFP->DatalinkConnected = primedPlan->DLStatus == "true" ? true : false;
				netFP->IsEquipped = primedPlan->IsEquipped;
				netFP->State = primedPlan->State;
				netFP->Etd = primedPlan->Etd;
				if (primedPlan->ExitTime != -1) {
					netFP->Relevant = true;
				}
				else {
					netFP->Relevant = false;
				}
				netFP->TargetMode = CUtils::GetTargetMode(screen->GetPlugIn()->RadarTargetSelect(primedPlan->Callsign.c_str()).GetPosition().GetRadarFlags());
				netFP->TrackedBy = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerCallsign();
				netFP->TrackedById = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetTrackingControllerId();
				netFP->Route = ""; // Initialise
				netFP->RouteEtas = ""; // Initialise

				// Get routes and estimates
				vector<CRoutePosition> rte;
				CRoutesHelper::GetRoute(screen, &rte, primedPlan->Callsign);
				for (int i = 0; i < rte.size(); i++) {
					if (i != rte.size() - 1) {
						netFP->Route += rte[i].Fix + " ";
						netFP->RouteEtas += rte[i].Estimate + " ";
					}
					else {
						netFP->Route += rte[i].Fix;
						netFP->RouteEtas += rte[i].Estimate;
					}
				}

				// Post data to the database
				CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
				data->Screen = screen;
				data->Callsign = primedPlan->Callsign;
				data->FP = netFP;
				_beginthread(CDataHandler::UpdateNetworkAircraft, 0, (void*)data); // Async
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
		if (id == BTN_XCHANGE_ACCEPT) {
			string controller = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign();
			SetButtonState(BTN_MANENTRY, CInputState::INACTIVE);
			string controllerMe = screen->GetPlugIn()->ControllerMyself().GetCallsign();
			if (controller == controllerMe) {
				screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).AcceptHandoff();
				windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::DISABLED;
				windowButtons[BTN_XCHANGE_REJECT].State = CInputState::DISABLED;
				windowButtons[BTN_XCHANGE_TRACK].Label = "Release";
			}
			if (primedPlan->CurrentMessage != nullptr && (primedPlan->CurrentMessage->Type == CMessageType::LOG_ON || primedPlan->CurrentMessage->Type == CMessageType::TRANSFER)) {
				IsMessageOpen = false;
				// Mark message as done here
				primedPlan->DLStatus = "ONLINE";
				textInputs[TXT_DATALINK].Content = "ONLINE";
				windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::DISABLED;
				windowButtons[BTN_XCHANGE_REJECT].State = CInputState::DISABLED;
				CMessageWindow::OngoingMessages.erase(primedPlan->CurrentMessage->Id);
				CMessageWindow::ActiveMessages.erase(primedPlan->CurrentMessage->Id);
				primedPlan->CurrentMessage = nullptr;
			}
		}
		if (id == BTN_XCHANGE_REJECT) {
			string controller = screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign();
			string controllerMe = screen->GetPlugIn()->ControllerMyself().GetCallsign();
			if (controller == controllerMe) {
				screen->GetPlugIn()->FlightPlanSelect(primedPlan->Callsign.c_str()).RefuseHandoff();
				windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::DISABLED;
				windowButtons[BTN_XCHANGE_REJECT].State = CInputState::DISABLED;
			}
			if (primedPlan->CurrentMessage != nullptr && (primedPlan->CurrentMessage->Type == CMessageType::LOG_ON || primedPlan->CurrentMessage->Type == CMessageType::TRANSFER)) {
				IsMessageOpen = false;
				// Mark message as done here
				windowButtons[BTN_XCHANGE_ACCEPT].State = CInputState::DISABLED;
				windowButtons[BTN_XCHANGE_REJECT].State = CInputState::DISABLED;
				CMessageWindow::OngoingMessages.erase(primedPlan->CurrentMessage->Id);
				CMessageWindow::ActiveMessages.erase(primedPlan->CurrentMessage->Id);
				primedPlan->CurrentMessage = nullptr;
			}
		}
		if (id == BTN_COORD_SENDOK) {
			IsCoordOpen = false;
		}
		if (id == BTN_ATCR || id == BTN_ATCR_CPY || id == BTN_ATCR_OK) {
			IsATCRestrictionsOpen = !IsATCRestrictionsOpen ? true : false;
		}
		if (id == BTN_ATCR_CANCEL) {
			if (IsCopyMade)
				copiedPlan.Restrictions.clear();
			else
				primedPlan->Restrictions.clear();
			IsATCRestrictionsOpen = !IsATCRestrictionsOpen ? true : false;
		}
		if (id == BTN_CONF_COORD || id == BTN_COORD_CLOSE) {
			IsCoordOpen = !IsCoordOpen ? true : false;
		}
		if (id == BTN_HIST || id == BTN_HIST_CLOSE) {
			IsHistoryOpen = !IsHistoryOpen ? true : false;
		}
		if (id == BTN_MAN_SUBMIT) {
			// Set states
			IsManualEntryOpen = false;
			IsData = true;
			SetButtonState(BTN_MANENTRY, CInputState::DISABLED);
			SetButtonState(BTN_READBK, CInputState::INACTIVE);
			SetButtonState(BTN_PROBE, CInputState::INACTIVE);
			
			// Assign
			textInputs[TXT_DEST].Content = textInputs[TXT_MAN_DEST].Content;
			textInputs[TXT_LEVEL].Content = textInputs[TXT_MAN_FL].Content;
			textInputs[TXT_RTE].Content = textInputs[TXT_MAN_RTE].Content;
			textInputs[TXT_SPD].Content = textInputs[TXT_MAN_SPD].Content;
			textInputs[TXT_TCK].Content = textInputs[TXT_MAN_TCK].Content;			
			primedPlan->State = "";
			textInputs[TXT_STATE].Content = primedPlan->State;
			primedPlan->Dest = textInputs[TXT_DEST].Content;
			primedPlan->Track = textInputs[TXT_TCK].Content;

			// Clear
			textInputs[TXT_MAN_DEST].Content = "";
			textInputs[TXT_MAN_EP].Content = "";
			textInputs[TXT_MAN_EPTIME].Content = "";
			textInputs[TXT_MAN_FL].Content = "";
			textInputs[TXT_MAN_RTE].Content = "";
			textInputs[TXT_MAN_SPD].Content = "";
			textInputs[TXT_MAN_TCK].Content = "";
		}
		if (id == BTN_MAN_CANCEL) {
			textInputs[TXT_MAN_DEST].Content = "";
			textInputs[TXT_MAN_EP].Content = "";
			textInputs[TXT_MAN_EPTIME].Content = "";
			textInputs[TXT_MAN_FL].Content = "";
			textInputs[TXT_MAN_RTE].Content = "";
			textInputs[TXT_MAN_SPD].Content = "";
			textInputs[TXT_MAN_TCK].Content = "";
			IsManualEntryOpen = false;
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
				IsData = true;
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
		if (id == BTN_RESTRI_EDIT_CLOSE) {
			RestrictionSubModalType = -1;
			// TODO: Maybe clean up the values here on cancel
		}

		if (id == BTN_ATCR_ADD) {
			this->ButtonDoubleClick(selectedRestriction);
		}

		if (id == BTN_RESTRI_EDIT_OK) {
			if (RestrictionSubModalType == SEL_ATCR_LCHG) {
				string content;
				if (textInputs[TXT_RESTRI_LCHG_TIME].Content != "" && checkBoxes[CHK_RESTRI_LCHG].IsChecked) {
					content += textInputs[TXT_RESTRI_LCHG_LATLON].Content + ":" + textInputs[TXT_RESTRI_LCHG_TIME].Content;
				}
				else {
					content += textInputs[TXT_RESTRI_LCHG_LATLON].Content;
				}
				ParseRestriction(content, CRestrictionType::LCHG);
			}
			else if (RestrictionSubModalType == SEL_ATCR_MCHG) {
				string content;
				if (textInputs[TXT_RESTRI_MCHG_TIME].Content != "") {
					content += textInputs[TXT_RESTRI_MCHG_LATLON].Content + ":" + textInputs[TXT_RESTRI_MCHG_TIME].Content;
				}
				else {
					content += textInputs[TXT_RESTRI_MCHG_LATLON].Content;
				}
				ParseRestriction(content, CRestrictionType::MCHG);
			}
			else if (RestrictionSubModalType == SEL_ATCR_UNABLE) {
				string content;
				int counter = 0;
				if (checkBoxes[CHK_RESTRI_UNABLE_LVL].IsChecked) {
					counter++;
					content += "LCHG";
				}
				if (checkBoxes[CHK_RESTRI_UNABLE_SPD].IsChecked) {
					counter++;
					if (counter > 1)
						content += ":MCHG";
					else
						content += "MCHG";

				}
				if (checkBoxes[CHK_RESTRI_UNABLE_RTE].IsChecked) {
					counter++;
					if (counter > 1)
						content += ":RERUTE";
					else
						content += "RERUTE";
				}

				ParseRestriction(content, CRestrictionType::UNABLE);
			}
			else if (RestrictionSubModalType == SEL_ATCR_ATA) {
				string content;
				if (textInputs[TXT_RESTRI_ATA_TIME].Content != "") {
					content += textInputs[TXT_RESTRI_ATA_LATLON].Content + ":" + textInputs[TXT_RESTRI_ATA_TIME].Content;
				}
				ParseRestriction(content, CRestrictionType::ATA);
			}
			else if (RestrictionSubModalType == SEL_ATCR_ATB) {
				string content;
				if (textInputs[TXT_RESTRI_ATB_TIME].Content != "") {
					content += textInputs[TXT_RESTRI_ATB_LATLON].Content + ":" + textInputs[TXT_RESTRI_ATB_TIME].Content;
				}
				ParseRestriction(content, CRestrictionType::ATB);
			}
			else if (RestrictionSubModalType == SEL_ATCR_XAT) {
				string content;
				if (textInputs[TXT_RESTRI_XAT_TIME].Content != "") {
					content += textInputs[TXT_RESTRI_XAT_LATLON].Content + ":" + textInputs[TXT_RESTRI_XAT_TIME].Content;
				}
				ParseRestriction(content, CRestrictionType::XAT);
			}
			else if (RestrictionSubModalType == SEL_ATCR_INT) {
				string content;
				content += textInputs[TXT_RESTRI_INT_CALLSIGN].Content + ":" + textInputs[TXT_RESTRI_INT_INTERVAL].Content;
				ParseRestriction(content, CRestrictionType::INT);
			}

			// One done handling, close the modal
			RestrictionSubModalType = -1;
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
		if (dropDowns[id].State != CInputState::DISABLED) {
			// Set value
			dropDowns[ActiveDropDown].Value = dropDowns[ActiveDropDown].Items[id].Label;
			// Close drop down
			dropDowns[ActiveDropDown].Items[ActiveDropDownHover].IsHovered = false;
			ActiveDropDownHover = 0;
			dropDowns[ActiveDropDown].State = CInputState::INACTIVE;
		}
	}

	// Check if checkbox
	if (id >= 300) {
		if (checkBoxes.find(id) != checkBoxes.end()) {
			if (checkBoxes.at(id).State != CInputState::DISABLED) {
				checkBoxes.at(id).IsChecked = !checkBoxes.at(id).IsChecked;

				if (id == CHK_CLRC_CPDLC) {
					SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_SEND, CInputState::INACTIVE);
					SetButtonState(BTN_CLRC_REJECT, CInputState::INACTIVE);
					checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
					checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
					checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;

				}
				else if (id == CHK_CLRC_ORCA) {
					SetButtonState(BTN_CLRC_VOICE, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_SEND, CInputState::INACTIVE);
					SetButtonState(BTN_CLRC_REJECT, CInputState::INACTIVE);
					checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
					checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
					checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;

				}
				else if (id == CHK_CLRC_TXT) {
					SetButtonState(BTN_CLRC_VOICE, CInputState::INACTIVE);
					SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_SEND, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_REJECT, CInputState::INACTIVE);
					checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
					checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
					checkBoxes.at(CHK_CLRC_VOX).IsChecked = false;

				}
				else if (id == CHK_CLRC_VOX) {
					SetButtonState(BTN_CLRC_VOICE, CInputState::INACTIVE);
					SetButtonState(BTN_CLRC_READBK, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_SEND, CInputState::DISABLED);
					SetButtonState(BTN_CLRC_REJECT, CInputState::INACTIVE);
					checkBoxes.at(CHK_CLRC_ORCA).IsChecked = false;
					checkBoxes.at(CHK_CLRC_TXT).IsChecked = false;
					checkBoxes.at(CHK_CLRC_CPDLC).IsChecked = false;
				}
				else if (id == CHK_RESTRI_LCHG) {
					if (checkBoxes.at(CHK_RESTRI_LCHG).IsChecked) {
						textInputs[TXT_RESTRI_LCHG_LATLON].State = CInputState::DISABLED;
						textInputs[TXT_RESTRI_LCHG_TIME].State = CInputState::ACTIVE;
					}
					else {
						textInputs[TXT_RESTRI_LCHG_LATLON].State = CInputState::ACTIVE;
						textInputs[TXT_RESTRI_LCHG_TIME].State = CInputState::DISABLED;
					}
				}
				else if (id == CHK_RESTRI_MCHG) {
					if (checkBoxes.at(CHK_RESTRI_MCHG).IsChecked) {
						textInputs[TXT_RESTRI_MCHG_LATLON].State = CInputState::DISABLED;
						textInputs[TXT_RESTRI_MCHG_TIME].State = CInputState::ACTIVE;
					}
					else {
						textInputs[TXT_RESTRI_MCHG_LATLON].State = CInputState::ACTIVE;
						textInputs[TXT_RESTRI_MCHG_TIME].State = CInputState::DISABLED;
					}
				}
			}
		}
	}

	// Check if selection
	if (id >= 400) {
		if (IsCopyMade && (id == SEL_ATCR_RERUTE || id == SEL_ATCR_INT))
			return;
		if (!primedPlan->IsCleared && (id == SEL_ATCR_LCHG || id == SEL_ATCR_MCHG || id == SEL_ATCR_UNABLE || id == SEL_ATCR_ATA || id == SEL_ATCR_ATB || id == SEL_ATCR_XAT))
			return;
		if (restrictionSelections.find(id) != restrictionSelections.end()) {
			selectedRestriction = id;
		}
	}
	if (id >= 500) {
		selectedActiveRestriction = id;
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

void CFlightPlanWindow::ButtonDoubleClick(int id)
{
	if (id >= SEL_ATCR_LCHG && id <= SEL_ATCR_INT) {
		if (id == SEL_ATCR_EPC || id == SEL_ATCR_RTD)
			return;

		if (id == SEL_ATCR_RERUTE) {
			//TODO: Handle Reroute message
			ParseRestriction("RERUTE", CRestrictionType::RERUTE);
			return;
		}

		// Plan
		CAircraftFlightPlan* plan = IsCopyMade ? &copiedPlan : primedPlan;

		// So that we can't double restrictions
		for (int i = 0; i < plan->Restrictions.size(); i++) {
			if (plan->Restrictions[i].Type == CRestrictionType::LCHG) {
				if (id == SEL_ATCR_LCHG)
					return;
			} else if (plan->Restrictions[i].Type == CRestrictionType::MCHG) {
				if (id == SEL_ATCR_MCHG)
					return;
			}
			else if (plan->Restrictions[i].Type == CRestrictionType::UNABLE) {
				if (id == SEL_ATCR_UNABLE)
					return;
			}
			else if (plan->Restrictions[i].Type == CRestrictionType::RERUTE) {
				if (id == SEL_ATCR_RERUTE)
					return;
			}
			else if (plan->Restrictions[i].Type == CRestrictionType::ATA) {
				if (id == SEL_ATCR_ATA)
					return;
			}
			else if (plan->Restrictions[i].Type == CRestrictionType::ATB) {
				if (id == SEL_ATCR_ATB)
					return;
			}
			else if (plan->Restrictions[i].Type == CRestrictionType::XAT) {
				if (id == SEL_ATCR_XAT)
					return;
			}
			else if (plan->Restrictions[i].Type == CRestrictionType::INT) {
				if (id == SEL_ATCR_INT)
					return;
			}
		}

		RestrictionSubModalType = id;
	}

	if (id >= 600) {
		int remove = id - 600;
		if (IsCopyMade)
			copiedPlan.Restrictions.erase(copiedPlan.Restrictions.begin() + remove);
		else
			primedPlan->Restrictions.erase(primedPlan->Restrictions.begin() + remove);
		selectedActiveRestriction = -1;
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
