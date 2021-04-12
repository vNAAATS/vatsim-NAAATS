#include "pch.h"
#include "AcTargets.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include "MenuBar.h"

using namespace Colours;

clock_t CAcTargets::twoSecondTimer = clock();
clock_t CAcTargets::fiveSecondTimer = clock();
string CAcTargets::SearchedAircraft = "";
string CAcTargets::OpenTrackingDialog = "";
map<string, bool> CAcTargets::ButtonStates;

void CAcTargets::Initialise() {
	// Initialise button states
	ButtonStates["Co-ord"] = false;
	ButtonStates["Track"] = false;
	ButtonStates["Release"] = false;
	ButtonStates["Accept"] = false;
	ButtonStates["Refuse"] = false;
}

void CAcTargets::DrawAirplane(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, bool tagsOn, map<int, CWinButton>* toggleData, bool halo, bool ptl, CSTCAStatus* status) {
	// 2 second timer
	double twoSecT = (double)(clock() - twoSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Get the aircraft's position and heading
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target->GetPosition().GetPosition());

	// Callsign
	string cs = target->GetCallsign();

	// Radar flags
	CRadarTargetMode targetMode = CUtils::GetTargetMode(target->GetPosition().GetRadarFlags());

	// Flight plan
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(target->GetCallsign());

	// Check if there is an active handoff to client 
	bool isHandoffToMe = string(fp.GetHandoffTargetControllerCallsign()) == string(screen->GetPlugIn()->ControllerMyself().GetCallsign());

	// Aircraft position
	CPosition position = target->GetPosition().GetPosition();

	// Save context for later
	int sDC = dc->SaveDC();

	// Define brushes and a container for the target
	SolidBrush orangeBrush(TargetOrange);
	SolidBrush blueBrush(TargetBlue);
	SolidBrush yellowBrush(WarningYellow);
	SolidBrush redBrush(CriticalRed);
	SolidBrush whiteBrush(TextWhite);
	Pen bluePen(&blueBrush, 1);
	Pen orangePen(&orangeBrush, 1);
	Pen whitePen(&whiteBrush, 1);
	GraphicsContainer gContainer;

	// Begin drawing
	gContainer = g->BeginContainer();

	// Set text colour
	COLORREF textColour = TargetBlue.ToCOLORREF(); // Default colour
	if (status->ConflictStatus == CConflictStatus::CRITICAL) {
		// Critical conflict status, so flash callsign white and red
		if (twoSecT >= 1.1) {
			textColour = TextWhite.ToCOLORREF();
		}
		else {
			textColour = CriticalRed.ToCOLORREF();
		}
	}
	else if (status->ConflictStatus == CConflictStatus::WARNING) {
		// Warning status, turn tag yellow
		textColour = WarningYellow.ToCOLORREF();
	}
	else {
		// Check jurisdiction
		if (!fp.GetTrackingControllerIsMe()) {
			// Callsign blue, no critical conflict
			textColour = TargetBlue.ToCOLORREF();

			// Orange if observer
			if (!screen->GetPlugIn()->ControllerMyself().IsController()) {
				if (position.m_Longitude > -70 && position.m_Longitude < -5)
					textColour = TargetOrange.ToCOLORREF();
				if (position.m_Latitude < 80 && position.m_Longitude < -35)
					textColour = TargetOrange.ToCOLORREF();
			}
		}
		else {
			// No conflict or only warning, tag orange
			textColour = TargetOrange.ToCOLORREF();
		}
	}

	// Draw the tracking controller
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(textColour);
	dc->SetTextAlign(TA_CENTER);
	string line;
	if (tagsOn) {
		line = string(fp.GetTrackingControllerId()) != "" ? string(fp.GetTrackingControllerId()) : "";
		dc->TextOutA(acPoint.x, acPoint.y - 20, line.c_str());
	}

	// Anti aliasing
	g->SetSmoothingMode(SmoothingModeAntiAlias);

	// We change the target based on state
	if (targetMode != CRadarTargetMode::ADS_B && screen->GetPlugIn()->ControllerMyself().IsController()) {
		// Set middle
		g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);
		// Change targets depending on mode
		if (targetMode == CRadarTargetMode::PRIMARY) {	
			// Make asterisk
			Point points[12] = {
				Point(-5, 0),
				Point(5, 0),
				Point(0, 0),
				Point(0, 5),
				Point(0, -5),
				Point(0, 0),
				Point(-5, -5),
				Point(5, 5),
				Point(0,0),
				Point(-5, 5),
				Point(5, -5),
				Point(0, 0)
			};

			if (isHandoffToMe)
				g->DrawPolygon(&whitePen, points, 12);
			else
				g->DrawPolygon(&bluePen, points, 12);
			g->EndContainer(gContainer);

			DeleteObject(&points);
		}
		else if (targetMode == CRadarTargetMode::SECONDARY_C) {
			// Make star
			Point points1[4] = { // Star point 1
				Point(-5, 0),
				Point(-8, -4),
				Point(-2, -4),
				Point(-5, 0)
			};
			Point points2[4] = { // Star point 2
				Point(-2, -4),
				Point(0, -9),
				Point(2, -4),
				Point(-2, -4)
			};
			Point points3[4] = { // Star point 3
				Point(2, -4),
				Point(8, -4),
				Point(4, 0),
				Point(2, -4)
			};
			Point points4[4] = { // Star point 4
				Point(4, 0),
				Point(6, 7),
				Point(0, 4),
				Point(4, 0)
			};
			Point points5[4] = { // Star point 5
				Point(0, 4),
				Point(-6, 7),
				Point(-4, 0),
				Point(0, 4)
			};

			// Make diamond
			Point diamond[4] = {
				Point(-4, 0),
				Point(0, 4),
				Point(4, 0),
				Point(0, -4)
			};

			if (isHandoffToMe) {
				// Star
				g->DrawPolygon(&whitePen, points1, 4);
				g->FillPolygon(&whiteBrush, points1, 4);
				g->DrawPolygon(&whitePen, points2, 4);
				g->FillPolygon(&whiteBrush, points2, 4);
				g->DrawPolygon(&whitePen, points3, 4);
				g->FillPolygon(&whiteBrush, points3, 4);
				g->DrawPolygon(&whitePen, points4, 4);
				g->FillPolygon(&whiteBrush, points4, 4);
				g->DrawPolygon(&whitePen, points5, 4);
				g->FillPolygon(&whiteBrush, points5, 4);

				// Diamond
				g->DrawPolygon(&whitePen, diamond, 4);
			}
			else {
				
				if (fp.GetTrackingControllerIsMe()) {
					// Star
					g->DrawPolygon(&orangePen, points1, 4);
					g->FillPolygon(&orangeBrush, points1, 4);
					g->DrawPolygon(&orangePen, points2, 4);
					g->FillPolygon(&orangeBrush, points2, 4);
					g->DrawPolygon(&orangePen, points3, 4);
					g->FillPolygon(&orangeBrush, points3, 4);
					g->DrawPolygon(&orangePen, points4, 4);
					g->FillPolygon(&orangeBrush, points4, 4);
					g->DrawPolygon(&orangePen, points5, 4);
					g->FillPolygon(&orangeBrush, points5, 4);

					// Diamond
					g->DrawPolygon(&orangePen, diamond, 4);
				}
				else {
					// Star
					g->DrawPolygon(&bluePen, points1, 4);
					g->FillPolygon(&blueBrush, points1, 4);
					g->DrawPolygon(&bluePen, points2, 4);
					g->FillPolygon(&blueBrush, points2, 4);
					g->DrawPolygon(&bluePen, points3, 4);
					g->FillPolygon(&blueBrush, points3, 4);
					g->DrawPolygon(&bluePen, points4, 4);
					g->FillPolygon(&blueBrush, points4, 4);
					g->DrawPolygon(&bluePen, points5, 4);
					g->FillPolygon(&blueBrush, points5, 4);

					// Diamond
					g->DrawPolygon(&bluePen, diamond, 4);
				}
			}
	
			g->EndContainer(gContainer);

			DeleteObject(&points1);
			DeleteObject(&points2);
			DeleteObject(&points3);
			DeleteObject(&points4);
			DeleteObject(&points5);
			DeleteObject(&diamond);
		}
		else {
			// Make diamond with line
			Point points[5] = {
				Point(-5, 0),
				Point(0, 5),
				Point(5, 0),
				Point(0, -5),
				Point(0, 5)
			};

			if (isHandoffToMe) {
				g->DrawPolygon(&whitePen, points, 5);
			}
			else {
				if (fp.GetTrackingControllerIsMe())
					g->DrawPolygon(&orangePen, points, 5);
				else
					g->DrawPolygon(&bluePen, points, 5);
			}

			g->EndContainer(gContainer);

			DeleteObject(&points);
		}
	}
	else {
		// Rotate the graphics object and set the middle to the aircraft position
		g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);
		g->RotateTransform(target->GetPosition().GetReportedHeadingTrueNorth());

		// This is the aircraft icon
		Point points[19] = {
			Point(0,-8),
			Point(-1,-7),
			Point(-1,-2),
			Point(-8,3),
			Point(-8,4),
			Point(-1,2),
			Point(-1,7),
			Point(-4,9),
			Point(-4,10),
			Point(0,9),
			Point(4,10),
			Point(4,9),
			Point(1,7),
			Point(1,2),
			Point(8,4),
			Point(8,3),
			Point(1,-2),
			Point(1,-7),
			Point(0,-8)
		};

		// Fill the polygon with the appropriate colour and finish
		if (status->ConflictStatus == CConflictStatus::CRITICAL) {
			// Critical conflict status, so flash white and red every second
			if (twoSecT >= 1.1) {
				g->FillPolygon(&whiteBrush, points, 19);
				g->EndContainer(gContainer);
			}
			else {
				g->FillPolygon(&redBrush, points, 19);
				g->EndContainer(gContainer);
			}
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			// Warning status, turn target yellow
			g->FillPolygon(&yellowBrush, points, 19);
			g->EndContainer(gContainer);
		}
		else {
			// No conflict, draw orange if tracked and blue if not
			if (isHandoffToMe) {
				g->FillPolygon(&whiteBrush, points, 19);
			}				
			else {
				if (fp.GetTrackingControllerIsMe())
					g->FillPolygon(&orangeBrush, points, 19);
				else
					g->FillPolygon(&blueBrush, points, 19);
			}
				
			g->EndContainer(gContainer);
		}

		DeleteObject(&points);
	}
	

	// Check if leader lines are selected
	if (ptl) {
		// Get ptl value
		int val = toggleData->at(CMenuBar::BTN_PTL).Cycle;

		// Leader minutes
		int dist = 0;

		// Switch
		switch (val) {
		case 0:
			dist = 5;
			break;
		case 1:
			dist = 10;
			break;
		case 2:
			dist = 15;
			break;
		case 3:
			dist = 20;
			break;
		case 4:
			dist = 25;
			break;
		case 5:
			dist = 30;
			break;
		}

		// Get aircraft point at that time
		//int distance = CUtils::GetDistanceSpeedTime(target->GetGS(), min * 60);
		POINT ptlPoint = screen->ConvertCoordFromPositionToPixel(CUtils::GetPointDistanceBearing(target->GetPosition().GetPosition(), dist, target->GetTrackHeading()));

		// Draw leader
		Pen pen(Colours::TargetOrange, 1);
		Pen pen2(Colours::TargetBlue, 1);
		if (fp.GetTrackingControllerIsMe())
			g->DrawLine(&pen, acPoint.x, acPoint.y, ptlPoint.x, ptlPoint.y);
		else
			g->DrawLine(&pen2, acPoint.x, acPoint.y, ptlPoint.x, ptlPoint.y);

		// Cleanup
		DeleteObject(&pen);
		DeleteObject(&pen2);
	}

	// Draw halos
	if (halo) {
		// Get ptl value
		int val = toggleData->at(CMenuBar::BTN_HALO).Cycle;

		// Leader minutes
		int dist = 0;

		// Switch
		switch (val) {
		case 0:
			dist = 5;
			break;
		case 1:
			dist = 10;
			break;
		case 2:
			dist = 15;
			break;
		case 3:
			dist = 20;
			break;
		}

		// Get aircraft point at that time
		//int distance = CUtils::GetDistanceSpeedTime(target->GetGS(), min * 60);
		POINT haloPoint = screen->ConvertCoordFromPositionToPixel(CUtils::GetPointDistanceBearing(target->GetPosition().GetPosition(), dist, target->GetTrackHeading()));

		// Get distance
		int radius = CUtils::GetDistanceBetweenPoints(acPoint, haloPoint);

		// Draw halo
		Rect temp(acPoint.x - radius, acPoint.y - radius, radius * 2, radius * 2);
		// Draw leader
		Pen pen(Colours::TargetOrange, 1);
		Pen pen2(Colours::TargetBlue, 1);
		if (fp.GetTrackingControllerIsMe())
			g->DrawEllipse(&pen, temp);
		else
			g->DrawEllipse(&pen2, temp);
		

		// Cleanup
		DeleteObject(&pen);
		DeleteObject(&pen2);
	}

	// Restore context
	dc->RestoreDC(sDC);

	// Deallocate
	DeleteObject(&orangeBrush);
	DeleteObject(&yellowBrush);
	DeleteObject(&redBrush);
	DeleteObject(&whiteBrush);
	DeleteObject(&blueBrush);
	DeleteObject(&gContainer);
	DeleteObject(&acPoint);
	DeleteObject(&bluePen);
	DeleteObject(&whitePen);
	DeleteObject(&orangePen);
}

POINT CAcTargets::DrawTag(CDC* dc, CRadarScreen* screen, CRadarTarget* target, pair<bool, POINT>* tagPosition, bool direction, CSTCAStatus* status) {	
	// 2 second timer
	double twoSecT = (double)(clock() - twoSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Get the aircraft's position and flight plan
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target->GetPosition().GetPosition());
	CFlightPlan acFP = screen->GetPlugIn()->FlightPlanSelect(target->GetCallsign());

	CAircraftFlightPlan fp;
	CDataHandler::GetFlightData(acFP.GetCallsign(), fp);

	// Check if there is an active handoff to client controller
	bool isHandoffToMe = string(acFP.GetHandoffTargetControllerCallsign()) == string(screen->GetPlugIn()->ControllerMyself().GetCallsign());

	// Save context for later
	int sDC = dc->SaveDC();

	// Aircraft position
	CPosition position = target->GetPosition().GetPosition();

	// Tag rectangle
	CRect tagRect;
	try { // Temporary
		int tagOffsetX = tagPosition->second.x;
		int tagOffsetY = tagPosition->second.y;
		if (tagOffsetX == 0 && tagOffsetY == 0) { // default point, we need to set it
			if (tagPosition->first == true) {
				// Detailed

				if (direction) {
					tagRect = CRect(acPoint.x - 110, acPoint.y + 10, acPoint.x - 25, acPoint.y + 66);
				}
				else {
					tagRect = CRect(acPoint.x + 40, acPoint.y - 25, acPoint.x + 125, acPoint.y + 10);
				}
			}
			else {
				// Not detailed
				if (direction) {
					tagRect = CRect(acPoint.x - 110, acPoint.y + 10, acPoint.x - 25, acPoint.y + 40);
				}
				else {
					tagRect = CRect(acPoint.x + 40, acPoint.y - 25, acPoint.x + 125, acPoint.y + 5);
				}
			}
		}
		else {
			if (tagPosition->first == true) {
				// Detailed
				tagRect = CRect(acPoint.x + tagOffsetX, acPoint.y + tagOffsetY, (acPoint.x + tagOffsetX) + 85, (acPoint.y + tagOffsetY) + 56);
			}
			else {
				// Not Detailed
				tagRect = CRect(acPoint.x + tagOffsetX, acPoint.y + tagOffsetY, (acPoint.x + tagOffsetX) + 85, (acPoint.y + tagOffsetY) + 30);
			}
		}

		// Pick text colour
		COLORREF textColour = TargetBlue.ToCOLORREF(); // Default colour
		if (status->ConflictStatus == CConflictStatus::CRITICAL) {
			// Critical conflict status, so flash callsign white and red
			if (twoSecT >= 1.1) {
				textColour = TextWhite.ToCOLORREF();
			}
			else {
				textColour = CriticalRed.ToCOLORREF();
			}
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			// Check jurisdiction
			if (acFP.GetTrackingControllerIsMe()) {
				textColour = WarningYellow.ToCOLORREF();
			}
		}
		else {
			// Check jurisdiction
			if (!acFP.GetTrackingControllerIsMe()) {
				// Callsign blue, no critical conflict
				textColour = TargetBlue.ToCOLORREF();

				// Orange if observer
				if (!screen->GetPlugIn()->ControllerMyself().IsController()) {
					if (position.m_Longitude > -70 && position.m_Longitude < -5)
						textColour = TargetOrange.ToCOLORREF();
					if (position.m_Latitude < 80 && position.m_Longitude < -35)
						textColour = TargetOrange.ToCOLORREF();
				}
			}
			else {
				// No conflict or only warning, tag orange
				textColour = TargetOrange.ToCOLORREF();
			}
		}

		// Pick font for callsign
		FontSelector::SelectATCFont(16, dc);
		dc->SetTextColor(textColour);
		dc->SetTextAlign(TA_LEFT);
		string text;

		// Offsets
		int offsetX = 0;
		int offsetY = 0;

		// Callsign
		text = acFP.GetCallsign();
		dc->TextOutA(tagRect.left + 1, tagRect.top, text.c_str());
		offsetY += 15;

		// Check jurisdiction
		if (!acFP.GetTrackingControllerIsMe()) {
			textColour = TargetBlue.ToCOLORREF();
		}
		else {
			textColour = TargetOrange.ToCOLORREF();
		}

		// Conflict
		if (status->ConflictStatus == CConflictStatus::CRITICAL) { // Deselect white
			// Critical conflict status, so turn tag red
			textColour = CriticalRed.ToCOLORREF();
		}

		// Handoff
		if (isHandoffToMe) {
			textColour = TextWhite.ToCOLORREF();
		}

		// Orange if observer
		if (!screen->GetPlugIn()->ControllerMyself().IsController()) {
			if (position.m_Longitude > -70 && position.m_Longitude < -5)
				textColour = TargetOrange.ToCOLORREF();
			if (position.m_Latitude < 80 && position.m_Longitude < -35)
				textColour = TargetOrange.ToCOLORREF();
		}
		FontSelector::SelectMonoFont(12, dc);
		dc->SetTextColor(textColour);
		text = to_string(target->GetPosition().GetFlightLevel() / 100);
		dc->TextOutA(tagRect.left, tagRect.top + offsetY, text.c_str());
		offsetX += 50;

		// Mach
		int gs = target->GetPosition().GetReportedGS();
		if (atoi(text.c_str()) > 460) {
			text = "M" + to_string(CUtils::GetMach(gs, 573));
		}
		else {
			text = "M" + to_string(acFP.GetFlightPlanData().PerformanceGetMach(target->GetPosition().GetPressureAltitude(), target->GetVerticalSpeed()));
		}
		dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
		offsetX = 0;

		// Handoff initiated
		if (isHandoffToMe) {
			offsetY += 15;
			text = "H/O";
			dc->TextOutA(tagRect.right - dc->GetTextExtent("H/O").cx - 10, tagRect.top + offsetY, text.c_str());
			offsetY += 15;
		}
		else {
			offsetY += 30;
		}

		// Are they equipped?
		if (!fp.IsEquipped) {
			// Offset
			offsetY -= 15;
			text = "*";
			dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
			offsetY += 15;
		}

		if (tagPosition->first == true) {
			// Aircraft type
			text = acFP.GetFlightPlanData().GetAircraftFPType();
			dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
			offsetX += 50;

			// Destination
			text = acFP.GetFlightPlanData().GetDestination();
			dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
		}

		/// Tag line
		CSize txtExtent = dc->GetTextExtent(acFP.GetCallsign()); // Get callsign length

		// Pen
		CPen pen(PS_SOLID, 1, textColour);
		dc->SelectObject(pen);
		int tagMiddle = tagRect.left + ((tagRect.right - tagRect.left) / 2);

		// Dog leg
		if ((tagRect.left + txtExtent.cx + 5) < acPoint.x) {
			dc->MoveTo({ tagRect.left + txtExtent.cx + 2, tagRect.top + 8 });
			if ((tagRect.right - (tagRect.right - acPoint.x)) > (tagRect.left + txtExtent.cx + 5)) {
				if ((tagRect.right - (tagRect.right - acPoint.x)) < tagRect.right + 5) {
					dc->LineTo({ tagRect.right - (tagRect.right - acPoint.x), tagRect.top + 8 });
				}
				else {
					dc->LineTo({ tagRect.right + 5, tagRect.top + 8 });
				}
				dc->LineTo({ acPoint.x, acPoint.y });
			}
		}
		else { // Line to target
			dc->MoveTo({ tagRect.left - 2, tagRect.top + 8 });


			if (tagRect.left - (tagRect.left - acPoint.x) - (txtExtent.cx + 2) > tagRect.left - 10) {
				dc->LineTo({ tagRect.left - (tagRect.left - acPoint.x) - (txtExtent.cx + 2), tagRect.top + 8 });
			}
			else {
				dc->LineTo({ tagRect.left - 10, tagRect.top + 8 });
			}
			dc->LineTo(acPoint);
		}

		// Create screen object for tag
		screen->AddScreenObject(SCREEN_TAG, acFP.GetCallsign(), tagRect, true, (string(acFP.GetPilotName()) + " - " + string(acFP.GetFlightPlanData().GetRoute())).c_str());

		// Now create screen object for callsign
		CRect callsignRect(tagRect.left, tagRect.top, tagRect.left + dc->GetTextExtent(acFP.GetCallsign()).cx, tagRect.top + dc->GetTextExtent(acFP.GetCallsign()).cy);
		screen->AddScreenObject(SCREEN_TAG_CS, acFP.GetCallsign(), callsignRect, true, "Callsign clicked");

		// Restore context
		dc->RestoreDC(sDC);

		// Clean up
		DeleteObject(pen);
		DeleteObject(&textColour);

		return { tagRect.left, tagRect.top };
	}
	catch (exception & e) {
		screen->GetPlugIn()->DisplayUserMessage("vNAAATS", "Error", string(string(e.what())).c_str(), true, true, true, true, true);
		return { tagRect.left, tagRect.top };
	}
}

void CAcTargets::RenderCoordTagItem(CDC* dc, CRadarScreen* screen, string callsign, POINT tagPosition) {
	// Save context for later
	int sDC = dc->SaveDC();

	// Get flight plan
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(callsign.c_str());

	// Box to draw
	CRect coordBox;

	// State
	int dialogState; // 0 = not tracked, 1 = tracked by me, 2 = tracked by someone else, 3 = handoff initiated

	// Get tracking status
	if (fp.GetTrackingControllerId() == "") { // If not being tracked
		dialogState = 0;
		coordBox = CRect(tagPosition.x, tagPosition.y, tagPosition.x + 75, tagPosition.y + 40);
	}
	else if (fp.GetTrackingControllerIsMe()) { // If being tracked by me
		dialogState = 1;
		coordBox = CRect(tagPosition.x, tagPosition.y, tagPosition.x + 75, tagPosition.y + 40);
	}
	else if (fp.GetTrackingControllerId() != "") { // If being tracked by someone else
		dialogState = 2;
		coordBox = CRect(tagPosition.x, tagPosition.y, tagPosition.x + 75, tagPosition.y + 23);
	}
	else if (fp.GetHandoffTargetControllerCallsign() == screen->GetPlugIn()->ControllerMyself().GetCallsign()) {
		dialogState = 3;
		coordBox = CRect(tagPosition.x, tagPosition.y, tagPosition.x + 75, tagPosition.y + 58);
	}
	
	// Fill rectangle
	dc->FillSolidRect(coordBox, ScreenBlue.ToCOLORREF());
	dc->Draw3dRect(coordBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());
	InflateRect(coordBox, -1, -1);
	dc->Draw3dRect(coordBox, BevelDark.ToCOLORREF(), BevelLight.ToCOLORREF());

	// Draw buttons
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(TextWhite.ToCOLORREF());
	
	// Check the state & draw buttons
	CRect textRect;
	switch (dialogState) {
		case 0:
			textRect = CRect(coordBox.left, coordBox.top + 2, coordBox.right, coordBox.top + dc->GetTextExtent("Track").cy + 2);
			if (ButtonStates.at("Track")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 5, "Track");
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Track", textRect, true, "");

			textRect = CRect(coordBox.left, coordBox.top + 21, coordBox.right, coordBox.top + 21 + dc->GetTextExtent("Co-ord").cy + 2);
			if (ButtonStates.at("Co-ord")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 23, "Co-ord");
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Co-ord", textRect, true, "");
			break;
		case 1:
			textRect = CRect(coordBox.left, coordBox.top + 2, coordBox.right, coordBox.top + dc->GetTextExtent("Release").cy + 2);
			if (ButtonStates.at("Release")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 5, "Release");
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Release", textRect, true, "");			

			textRect = CRect(coordBox.left, coordBox.top + 21, coordBox.right, coordBox.top + 21 + dc->GetTextExtent("Co-ord").cy + 2);
			if (ButtonStates.at("Co-ord")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 23, "Co-ord");
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Co-ord", textRect, true, "");
			break;
		case 2:
			textRect = CRect(coordBox.left, coordBox.top + 2, coordBox.right, coordBox.top + dc->GetTextExtent("Co-ord").cy + 2);
			if (ButtonStates.at("Co-ord")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 5, "Co-ord");			
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Co-ord", textRect, true, "");
			break;
		case 3:
			textRect = CRect(coordBox.left, coordBox.top + 2, coordBox.right, coordBox.top + dc->GetTextExtent("Accept").cy + 2);
			if (ButtonStates.at("Accept")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 5, "Accept");
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Accept", textRect, true, "");

			textRect = CRect(coordBox.left, coordBox.top + 21, coordBox.right, coordBox.top + 21 + dc->GetTextExtent("Refuse").cy + 2);
			if (ButtonStates.at("Refuse")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 23, "Refuse");
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Refuse", textRect, true, "");

			textRect = CRect(coordBox.left, coordBox.top + 39, coordBox.right, coordBox.top + 39 + dc->GetTextExtent("Co-ord").cy + 2);
			if (ButtonStates.at("Co-ord")) {
				dc->FillSolidRect(coordBox, ButtonPressed.ToCOLORREF());
			}
			dc->TextOutA(coordBox.left + 4, coordBox.top + 41, "Co-ord");			
			screen->AddScreenObject(SCREEN_TAG_CS_BTN, "Co-ord", textRect, true, "");
	}

	// Restore context
	dc->RestoreDC(sDC);
}

void CAcTargets::RenderSelectionHalo(Graphics* g, CRadarScreen* screen, CRadarTarget* target) {
	// Brush
	SolidBrush white(TextWhite);

	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target->GetPosition().GetPosition());

	// Anti aliasing
	g->SetSmoothingMode(SmoothingModeAntiAlias);

	// Draw halo
	Rect temp(acPoint.x - 50, acPoint.y - 50, 50 * 2, 50 * 2);
	Pen pen(&white, 1);
	g->DrawEllipse(&pen, temp);

	// Cleanup
	DeleteObject(&pen);
}