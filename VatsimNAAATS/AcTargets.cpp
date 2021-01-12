#include "pch.h"
#include "AcTargets.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include "MenuBar.h"

using namespace Colours;

clock_t CAcTargets::twoSecondTimer = clock();
clock_t CAcTargets::fiveSecondTimer = clock();
string CAcTargets::SearchedAircraft = "";

void CAcTargets::DrawAirplane(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, bool tagsOn, map<int, CWinButton>* toggleData, bool halo, bool ptl, CSTCAStatus* status) {
	// 2 second timer
	double twoSecT = (double)(clock() - twoSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Get the aircraft's position and heading
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target->GetPosition().GetPosition());

	// Callsign
	string cs = target->GetCallsign();

	// Radar flags
	int radarFlag = target->GetPosition().GetRadarFlags();

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

	// Check jurisdiction
	if (!fp.GetTrackingControllerIsMe() && screen->GetPlugIn()->ControllerMyself().IsController()) {
		// Set middle
		g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);

		// Make diamond
		Point points[4] = {
			Point(-6, 0),
			Point(0, 6),
			Point(6, 0),
			Point(0, -6)
		};

		if (isHandoffToMe)
			g->DrawPolygon(&whitePen, points, 4);
		else
			g->DrawPolygon(&bluePen, points, 4);
		g->EndContainer(gContainer);

		DeleteObject(&points);
	}
	else {
		// Rotate the graphics object and set the middle to the aircraft position
		g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);
		g->RotateTransform(target->GetTrackHeading());

		// This is the icon
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
			// No conflict, draw orange
			if (isHandoffToMe)
				g->FillPolygon(&whiteBrush, points, 19);
			else 
				g->FillPolygon(&orangeBrush, points, 19);
			g->EndContainer(gContainer);
		}

		DeleteObject(&points);
	}
	

	// Check if leader lines are selected
	if (ptl) {
		// Get ptl value
		int val = toggleData->at(CMenuBar::BTN_PTL).Cycle;

		// Leader minutes
		int min = 0;

		// Switch
		switch (val) {
		case 0:
			min = 5;
			break;
		case 1:
			min = 10;
			break;
		case 2:
			min = 15;
			break;
		case 3:
			min = 20;
			break;
		case 4:
			min = 25;
			break;
		case 5:
			min = 30;
			break;
		}

		// Get aircraft point at that time
		int distance = CUtils::GetDistanceSpeedTime(target->GetGS(), min * 60);
		POINT ptlPoint = screen->ConvertCoordFromPositionToPixel(CUtils::GetPointDistanceBearing(target->GetPosition().GetPosition(), distance, target->GetTrackHeading()));

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
		int min = 0;

		// Switch
		switch (val) {
		case 0:
			min = 5;
			break;
		case 1:
			min = 10;
			break;
		case 2:
			min = 15;
			break;
		case 3:
			min = 20;
			break;
		}

		// Get aircraft point at that time
		int distance = CUtils::GetDistanceSpeedTime(target->GetGS(), min * 60);
		POINT haloPoint = screen->ConvertCoordFromPositionToPixel(CUtils::GetPointDistanceBearing(target->GetPosition().GetPosition(), distance, target->GetTrackHeading()));

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
}

POINT CAcTargets::DrawTag(CDC* dc, CRadarScreen* screen, CRadarTarget* target, pair<bool, POINT>* tagPosition, bool direction, CSTCAStatus* status) {
	// 2 second timer
	double twoSecT = (double)(clock() - twoSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Get the aircraft's position and flight plan
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target->GetPosition().GetPosition());
	CFlightPlan acFP = screen->GetPlugIn()->FlightPlanSelect(target->GetCallsign());

	// Check if there is an active handoff to client controller
	bool isHandoffToMe = string(acFP.GetHandoffTargetControllerCallsign()) == string(screen->GetPlugIn()->ControllerMyself().GetCallsign());

	// Save context for later
	int sDC = dc->SaveDC();

	// Aircraft position
	CPosition position = target->GetPosition().GetPosition();

	// Tag rectangle
	CRect tagRect;
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
		if (!acFP.GetTrackingControllerIsMe()) {
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

	// Flight level
	if (status->ConflictStatus == CConflictStatus::CRITICAL) { // Deselect white
		// Critical conflict status, so turn tag red
		textColour = CriticalRed.ToCOLORREF();
	}
	// Check jurisdiction
	if (!acFP.GetTrackingControllerIsMe()) {
		textColour = TargetBlue.ToCOLORREF();
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
	int gs = target->GetGS();
	if (atoi(text.c_str()) > 460) {
		text = "M" + to_string(CUtils::GetMach(gs, 573));
	}
	else {
		text = "M" + to_string(acFP.GetFlightPlanData().PerformanceGetMach(target->GetPosition().GetPressureAltitude(), target->GetVerticalSpeed()));
	}
	dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
	offsetX = 0;
	if (isHandoffToMe) {
		offsetY += 15;
		text = "H/O";
		dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
		offsetY += 15;
	}
	else {
		offsetY += 30;
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

	// Create screen object
	screen->AddScreenObject(SCREEN_TAG, acFP.GetCallsign(), tagRect, true, acFP.GetPilotName());

	// Restore context
	dc->RestoreDC(sDC);

	// Clean up
	DeleteObject(pen);
	DeleteObject(&textColour);

	return { tagRect.left, tagRect.top };
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