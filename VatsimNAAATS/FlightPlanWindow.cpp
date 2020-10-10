#include "pch.h"
#include "FlightPlanWindow.h"
#include "Constants.h"
#include "Styles.h"
#include "Utils.h"

using namespace Colours;

const int CFlightPlanWindow::BTN_CLOSE = 0;
const int CFlightPlanWindow::BTN_COPY = 1;
const int CFlightPlanWindow::BTN_TRNSFR = 2;
const int CFlightPlanWindow::BTN_COORD = 3;
const int CFlightPlanWindow::BTN_MANCLEAR = 4;
const int CFlightPlanWindow::BTN_PROBE = 5;
const int CFlightPlanWindow::BTN_DELETE = 6;
const int CFlightPlanWindow::BTN_ADS = 7;
const int CFlightPlanWindow::BTN_READBK = 8;
const int CFlightPlanWindow::BTN_MSG = 9;
const int CFlightPlanWindow::BTN_HIST = 10;
const int CFlightPlanWindow::BTN_SAVE = 11;
const int CFlightPlanWindow::TXT_ACID = 100;
const int CFlightPlanWindow::TXT_TYPE = 101;
const int CFlightPlanWindow::TXT_DEPART = 102;
const int CFlightPlanWindow::TXT_ETD = 103;
const int CFlightPlanWindow::TXT_SELCAL = 104;
const int CFlightPlanWindow::TXT_DATALINK = 105;
const int CFlightPlanWindow::TXT_COMMS = 106;
const int CFlightPlanWindow::TXT_OWNERSHIP = 107;
const int CFlightPlanWindow::TXT_SPD = 108;
const int CFlightPlanWindow::TXT_LEVEL = 109;
const int CFlightPlanWindow::TXT_DEST = 110;
const int CFlightPlanWindow::TXT_TCK = 111;
const int CFlightPlanWindow::TXT_STATE = 112;
const int CFlightPlanWindow::TXT_SPD_CPY = 113;
const int CFlightPlanWindow::TXT_LEVEL_CPY = 114;
const int CFlightPlanWindow::TXT_DEST_CPY = 115;
const int CFlightPlanWindow::TXT_TCK_CPY = 116;
const int CFlightPlanWindow::TXT_STATE_CPY = 117;

CFlightPlanWindow::CFlightPlanWindow(POINT topLeft) : CBaseWindow(topLeft) {
	// Make buttons
	MakeWindowItems();
};

void CFlightPlanWindow::MakeWindowItems() {
	// Button defaults
	WindowButtons[BTN_CLOSE] = make_pair("Close", CInputState::INACTIVE);
	WindowButtons[BTN_COPY] = make_pair("Copy", CInputState::DISABLED);
	WindowButtons[BTN_TRNSFR] = make_pair("Transfer", CInputState::DISABLED);
	WindowButtons[BTN_COORD] = make_pair("Co-ord", CInputState::DISABLED);
	WindowButtons[BTN_MANCLEAR] = make_pair("Man Clrc", CInputState::INACTIVE);
	WindowButtons[BTN_PROBE] = make_pair("Probe", CInputState::DISABLED);
	WindowButtons[BTN_DELETE] = make_pair("Delete", CInputState::DISABLED);
	WindowButtons[BTN_ADS] = make_pair("ADS", CInputState::DISABLED);
	WindowButtons[BTN_READBK] = make_pair("ReadBK", CInputState::DISABLED);
	WindowButtons[BTN_MSG] = make_pair("Message", CInputState::INACTIVE);
	WindowButtons[BTN_HIST] = make_pair("History", CInputState::INACTIVE);
	WindowButtons[BTN_SAVE] = make_pair("Save", CInputState::DISABLED);

	// Text defaults
	TextInputs[TXT_ACID] = CWinTextInput("ACID", "", 70, CInputState::INACTIVE);
	TextInputs[TXT_TYPE] = CWinTextInput("Type", "", 55, CInputState::INACTIVE);
	TextInputs[TXT_DEPART] = CWinTextInput("Depart", "", 50, CInputState::INACTIVE);
	TextInputs[TXT_ETD] = CWinTextInput("Etd", "", 50, CInputState::INACTIVE);
	TextInputs[TXT_SELCAL] = CWinTextInput("SELCAL", "", 55, CInputState::INACTIVE);
	TextInputs[TXT_DATALINK] = CWinTextInput("Datalink", "", 60, CInputState::INACTIVE);
	TextInputs[TXT_COMMS] = CWinTextInput("Com", "", 35, CInputState::INACTIVE);
	TextInputs[TXT_OWNERSHIP] = CWinTextInput("Sector", "", 25, CInputState::INACTIVE);
	TextInputs[TXT_SPD] = CWinTextInput("Spd", "", 50, CInputState::ACTIVE);
	TextInputs[TXT_LEVEL] = CWinTextInput("FL", "", 90, CInputState::ACTIVE);
	TextInputs[TXT_DEST] = CWinTextInput("Dest", "", 45, CInputState::ACTIVE);
	TextInputs[TXT_TCK] = CWinTextInput("Tck", "", 25, CInputState::DISABLED);
	TextInputs[TXT_STATE] = CWinTextInput("State", "", 35, CInputState::DISABLED);
}

void CFlightPlanWindow::RenderWindow(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Save device context
	int iDC = dc->SaveDC();

	// Create brushes
	CBrush darkerBrush(ScreenBlue.ToCOLORREF());
	CBrush lighterBrush(WindowBorder.ToCOLORREF());
	CBrush lightBackground(LightBackground.ToCOLORREF());
	CBrush routeBox(RouteBox.ToCOLORREF());

	// Select title font
	FontSelector::SelectNormalFont(16, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);

	// Create base window rectangle
	CRect windowRect(topLeft.x, topLeft.y, topLeft.x + WINSZ_FLTPLN_WIDTH, topLeft.y + WINSZ_FLTPLN_HEIGHT_DATA);
	dc->FillRect(windowRect, &darkerBrush);

	// Create titlebar
	CRect titleRect(windowRect.left, windowRect.top, windowRect.left + WINSZ_FLTPLN_WIDTH, windowRect.top + WINSZ_TITLEBAR_HEIGHT);
	dc->FillRect(titleRect, &lighterBrush);
	dc->DrawEdge(titleRect, EDGE_RAISED, BF_BOTTOM);
	dc->TextOutA(titleRect.left + (WINSZ_FLTPLN_WIDTH / 2), titleRect.top + (WINSZ_TITLEBAR_HEIGHT / 7), string(string("Flight Plan") + string(" - ") + string(TextInputs[TXT_ACID].Content)).c_str());

	// Add screen object
	screen->AddScreenObject(WINDOW, "WIN_FLTPLN", windowRect, true, ""); // So that we can't click anything under the flight plan window
	screen->AddScreenObject(WINDOW, "FLTPLN", titleRect, true, ""); // Movable

	// Create button bar
	CRect buttonBarRect(windowRect.left, windowRect.top + WINSZ_TITLEBAR_HEIGHT - 1, windowRect.left + WINSZ_FLTPLN_WIDTH, windowRect.top + 100);
	dc->FillRect(buttonBarRect, &darkerBrush);
	dc->Draw3dRect(buttonBarRect, ScreenBlue.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(buttonBarRect, -1, -1);
	dc->Draw3dRect(buttonBarRect, ScreenBlue.ToCOLORREF(), BevelDark.ToCOLORREF());

	/// Draw top menu buttons
	int offsetX = 25;
	int offsetY = 6;
	int counter = 0; // Counter for buttons so we know when to offset the Y axis
	for (auto kv : WindowButtons) {
		// Draw the button
		DrawButton(dc, screen, WindowButtons.at(kv.first).first, { buttonBarRect.left + offsetX, buttonBarRect.top + offsetY }, 70, 30, 6, WindowButtons.at(kv.first).second, WIN_FLTPLN, to_string(kv.first).c_str());
		if (counter != 5) { // Because there are 6 top row buttons
			offsetX += 82;
		}
		else {
			offsetX = 25;
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
	for (auto kv : TextInputs) {
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
		DrawTextInput(dc, screen, { infoBarRect.left + offsetX, infoBarRect.top + offsetY }, kv.second.Width, textHeight + 5, editable, &kv.second, WIN_FLTPLN, to_string(kv.first).c_str());

		// Offset
		offsetX += kv.second.Width + 5;
		offsetY = 6;
	}

	// TODO: switch to only show if in progress clearance/clearance issued
	// Create data bar
	CRect dataBarRect(windowRect.left, infoBarRect.bottom + 1, windowRect.left + WINSZ_FLTPLN_WIDTH, infoBarRect.bottom + 160);
	dc->FillRect(dataBarRect, &darkerBrush);
	dc->Draw3dRect(dataBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());
	InflateRect(dataBarRect, -1, -1);
	dc->Draw3dRect(dataBarRect, BevelLight.ToCOLORREF(), BevelDark.ToCOLORREF());

	// Show callsign in coloured box
	CRect idBox(windowRect.left + 6, dataBarRect.top + 8, windowRect.left + 150, dataBarRect.top + 36);
	dc->FillRect(idBox, &lightBackground);
	FontSelector::SelectATCFont(18, dc);
	dc->SetTextColor(Black.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	dc->TextOutA(idBox.left + (idBox.Width() / 2), dataBarRect.top + (idBox.Height() / 2) - 2, TextInputs[TXT_ACID].Content.c_str());

	// Create the route box
	CRect rteBox(windowRect.left + 5, idBox.bottom + 8, windowRect.right + - 100, idBox.bottom + 84);
	dc->FillRect(rteBox, &routeBox);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(rteBox, -1, -1);
	dc->Draw3dRect(rteBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Draw text in route box


	// Make inputs
	bool txtValid = true;
	counter = TXT_SPD;
	offsetX = idBox.Width() + 10;
	offsetY = 11;
	while (txtValid) {
		// Get index
		if (counter == TXT_SPD_CPY) break; // Break if end
		CWinTextInput idx = TextInputs.at(counter);

		// Font
		FontSelector::SelectNormalFont(15, dc);
		dc->SetTextColor(TextWhite.ToCOLORREF());
		dc->SetTextAlign(TA_LEFT);

		// Get text height to set offset and height of input
		int textHeight = dc->GetTextExtent(idx.Label.c_str()).cy;
		int textWidth = dc->GetTextExtent(idx.Label.c_str()).cx;

		// Shove the label down
		if (counter >= TXT_TCK) { 
			offsetY += textHeight;
		}
		// Draw the text
		dc->TextOutA(dataBarRect.left + offsetX, dataBarRect.top + offsetY + 2, idx.Label.c_str());

		// So that track and state get shoved down a row
		if (counter < TXT_TCK ) { 
			offsetX += textWidth + 5;
		}
		else {
			offsetY += textHeight + 5;
		}
		bool editable = false;
		if (idx.State == CInputState::ACTIVE) editable = true;
		DrawTextInput(dc, screen, { dataBarRect.left + offsetX, dataBarRect.top + offsetY }, idx.Width, textHeight + 5, editable, &idx, WIN_FLTPLN, to_string(counter).c_str());

		// Offset
		if (counter == TXT_DEST) { // Need a bit more space
			offsetX += idx.Width + 10;
		}
		else {
			offsetX += idx.Width + 5;
		}
		if (counter == TXT_TCK) {
			offsetY -= textHeight + 20;
		}
		counter++;
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
	DeleteObject(lightBackground);
	DeleteObject(routeBox);

	// Restore device context
	dc->RestoreDC(iDC);
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
		TextInputs[TXT_SELCAL].Content = remarks.substr(found + 4, 4);
	}
	else {
		TextInputs[TXT_SELCAL].Content = "N/A";
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
	TextInputs[TXT_ACID].Content = fp.GetCallsign();
	TextInputs[TXT_TYPE].Content = data.GetAircraftFPType();
	TextInputs[TXT_DEPART].Content = data.GetOrigin();
	TextInputs[TXT_ETD].Content = CUtils::ParseZuluTime(false, atoi(data.GetEstimatedDepartureTime()));
	TextInputs[TXT_DATALINK].Content = "N/A";
	TextInputs[TXT_COMMS].Content = comType;
	TextInputs[TXT_OWNERSHIP].Content = "34";

	// TEMPORARY FOR BETA TESTING
	int mach = fp.GetControllerAssignedData().GetAssignedMach();
	TextInputs[TXT_SPD].Content = string("M") + CUtils::PadWithZeros(3, fp.GetControllerAssignedData().GetAssignedMach());
	TextInputs[TXT_LEVEL].Content =  to_string(fp.GetControllerAssignedData().GetClearedAltitude());
	TextInputs[TXT_DEST].Content = data.GetDestination();
}

void CFlightPlanWindow::OnCloseFlightPlanWindow() {

}

int CFlightPlanWindow::ChangeDataPoint(CRadarScreen* screen, int data, string str) {
	// TODO: Currently temporary format, make more robust when implementing the different window modes
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(TextInputs[TXT_ACID].Content.c_str());
	if (data == TXT_SPD) {
		bool isNumber = true;
		for (int i = 0; i < strlen(str.c_str()); i++) { // Check if int
			if (!isdigit(str[i])) isNumber = false;
		}
		if (isNumber && (stoi(str) > 0 && stoi(str) < 250)) {
			bool status = fp.GetControllerAssignedData().SetAssignedMach(stoi(str));
			UpdateData(screen, CAcFPStatus(TextInputs[TXT_ACID].Content, CFlightPlanMode::INIT));
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
			UpdateData(screen, CAcFPStatus(TextInputs[TXT_ACID].Content, CFlightPlanMode::INIT));
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
				UpdateData(screen, CAcFPStatus(TextInputs[TXT_ACID].Content, CFlightPlanMode::INIT));
				return 0;
			}
		}
		return 1;
	}
}