#include "pch.h"
#include "AcTargets.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include "MenuBar.h"
#include "Logger.h"

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

void CAcTargets::RenderTarget(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, bool tagsOn, map<int, CWinButton>* toggleData, bool halo, bool ptl, CSTCAStatus* status) {
	// 2 second timer
	double twoSecT = (double)(clock() - twoSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Performance timer
	double functionClock = clock();

	// Get the aircraft's position and heading
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target->GetPosition().GetPosition());

	// Callsign
	string cs = target->GetCallsign();

	// Flight plan
	CFlightPlan fp = screen->GetPlugIn()->FlightPlanSelect(cs.c_str());
	CAircraftFlightPlan* acFP = CDataHandler::GetFlightData(cs);

	// Radar flags	
	acFP->TargetMode = CUtils::GetTargetMode(target->GetPosition().GetRadarFlags());
	CRadarTargetMode targetMode = acFP->TargetMode;

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
	Pen bluePen(&blueBrush, 1.5);
	Pen orangePen(&orangeBrush, 1.5);
	Pen whitePen(&whiteBrush, 1.5);
	Pen redPen(&redBrush, 1.5);
	Pen yellowPen(&yellowBrush, 1.5);
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

	// TODO: Abstract this from this method
	// We change the target based on state
	// Set middle
	g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);
	// Change targets depending on mode
	if (targetMode == CRadarTargetMode::ADS_B) {	
		if (acFP->IsCleared) {
			// Rotate the graphics object and set the middle to the aircraft position
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
				}
				else {
					g->FillPolygon(&redBrush, points, 19);
				}
			}
			else if (status->ConflictStatus == CConflictStatus::WARNING) {
				// Warning status, turn target yellow
				g->FillPolygon(&yellowBrush, points, 19);
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
			}

			// Cleanup
			g->EndContainer(gContainer);
			DeleteObject(&points);
		}
		else {
			// Make diamond with line
			Point diamond[4] = {
				Point(-6, 0),
				Point(0, 6),
				Point(6, 0),
				Point(0, -6)
			};

			// Draw the polygon with the appropriate colour and finish
			if (status->ConflictStatus == CConflictStatus::CRITICAL) {
				// Critical conflict status, so flash white and red every second
				if (twoSecT >= 1.1) {
					g->DrawPolygon(&whitePen, diamond, 4);
					g->DrawLine(&whitePen, Point(0, 6), Point(0, -6));
				}
				else {
					g->DrawPolygon(&redPen, diamond, 4);
					g->DrawLine(&redPen, Point(0, 6), Point(0, -6));
				}
			}
			else if (status->ConflictStatus == CConflictStatus::WARNING) {
				// Warning status, turn target yellow
				g->DrawPolygon(&yellowPen, diamond, 4);
				g->DrawLine(&yellowPen, Point(0, 6), Point(0, -6));
			}
			else {
				// No conflict, draw orange if tracked and blue if not
				if (isHandoffToMe) {
					g->DrawPolygon(&whitePen, diamond, 4);
					g->DrawLine(&whitePen, Point(0, 6), Point(0, -6));
				}
				else {
					if (fp.GetTrackingControllerIsMe()) {
						g->DrawPolygon(&orangePen, diamond, 4);
						g->DrawLine(&orangePen, Point(0, 6), Point(0, -6));
					}
					else {
						g->DrawPolygon(&bluePen, diamond, 4);
						g->DrawLine(&bluePen, Point(0, 6), Point(0, -6));
					}
				}
			}

			// Cleanup
			g->EndContainer(gContainer);
			DeleteObject(&diamond);
		}
	}
	else {
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

		// Draw the polygon with the appropriate colour and finish
		if (status->ConflictStatus == CConflictStatus::CRITICAL) {
			// Critical conflict status, so flash white and red every second
			if (twoSecT >= 1.1) {
				g->DrawPolygon(&whitePen, points, 12);
			}
			else {
				g->DrawPolygon(&redPen, points, 12);
			}
		}
		else if (status->ConflictStatus == CConflictStatus::WARNING) {
			// Warning status, turn target yellow
			g->DrawPolygon(&yellowPen, points, 12);
		}
		else {
			// No conflict, draw orange if tracked and blue if not
			if (isHandoffToMe) {
				g->DrawPolygon(&whitePen, points, 12);
				g->DrawLine(&whitePen, Point(0, 6), Point(0, -6));
			}
			else {
				if (fp.GetTrackingControllerIsMe()) {
					g->DrawPolygon(&orangePen, points, 12);
					g->DrawLine(&orangePen, Point(0, 6), Point(0, -6));
				}
				else {
					g->DrawPolygon(&bluePen, points, 12);
					g->DrawLine(&bluePen, Point(0, 6), Point(0, -6));
				}
			}
		}

		// Cleanup
		g->EndContainer(gContainer);
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
		case 6:
			min = 60;
			break;
		}

		// Get aircraft point at that time
		int distance = CUtils::GetDistanceSpeedTime(target->GetPosition().GetReportedGS(), min * 60);
		POINT ptlPoint = screen->ConvertCoordFromPositionToPixel(CUtils::GetPointDistanceBearing(target->GetPosition().GetPosition(), distance, target->GetPosition().GetReportedHeadingTrueNorth()));

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

		// Leader miles (actually metres but whatever, the function converts)
		int dist = 0;

		// Switch
		switch (val) {
		case 0:
			dist = 9260; // 5 nautical miles in metres
			break;
		case 1:
			dist = 18520; // 10nm
			break;
		case 2:
			dist = 27780; // 15nm
			break;
		case 3:
			dist = 37040; // 20nm
			break;
		case 4:
			dist = 46300; // 25nm
			break;
		case 5:
			dist = 55560; // 30nm
			break;
		case 6:
			dist = 111120; // 60nm
			break;
		case 7:
			dist = 55560; // 100nm
			break;
		}

		// Get aircraft point at that time
		//int distance = CUtils::GetDistanceSpeedTime(target->GetGS(), min * 60);
		POINT haloPoint = screen->ConvertCoordFromPositionToPixel(CUtils::GetPointDistanceBearing(target->GetPosition().GetPosition(), dist, target->GetPosition().GetReportedHeadingTrueNorth()));

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
	DeleteObject(&redPen);
	DeleteObject(&yellowPen);

	// Compute render time and store it
	clock_t t = clock();
	double lastRenderTimeMs = (t - functionClock) / (CLOCKS_PER_SEC / 1000);

	// Log if render time was greater than 4ms
	if (lastRenderTimeMs >= 4) {
		CLogger::Log(CLogType::WARN, "TARGET render time was >= 4ms. Time elapsed: " + to_string((int)round(lastRenderTimeMs)) + "ms.", target->GetCallsign());
	}
}

POINT CAcTargets::RenderTag(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget* target, pair<bool, POINT>* tagPosition, bool direction, CSTCAStatus* status, string asel) {	
	// 2 second timer
	double twoSecT = (double)(clock() - twoSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Performance timer
	double functionClock = clock();

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
					tagRect = CRect(acPoint.x - 112, acPoint.y + 10, acPoint.x - 25, acPoint.y + 42);
				}
				else {
					tagRect = CRect(acPoint.x + 40, acPoint.y - 25, acPoint.x + 127, acPoint.y + 8);
				}
			}
			else {
				// Not detailed
				if (direction) {
					tagRect = CRect(acPoint.x - 112, acPoint.y + 10, acPoint.x - 25, acPoint.y + 42);
				}
				else {
					tagRect = CRect(acPoint.x + 40, acPoint.y - 25, acPoint.x + 127, acPoint.y + 8);
				}
			}
		}
		else {
			if (tagPosition->first == true) {
				// Detailed
				tagRect = CRect(acPoint.x + tagOffsetX, acPoint.y + tagOffsetY, (acPoint.x + tagOffsetX) + 88, (acPoint.y + tagOffsetY) + 33);
			}
			else {
				// Not Detailed
				tagRect = CRect(acPoint.x + tagOffsetX, acPoint.y + tagOffsetY, (acPoint.x + tagOffsetX) + 88, (acPoint.y + tagOffsetY) + 33);
			}
		}

		// Pick text colour
		COLORREF textColour = TargetBlue.ToCOLORREF();; // Default colour
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
		int offsetX = 2;
		int offsetY = 0;

		// Callsign handoff initiated
		if (string(acFP.GetHandoffTargetControllerCallsign()).length() != 0) {
			if (acFP.GetTrackingControllerIsMe())				
				dc->SetTextColor(TextWhite.ToCOLORREF());
		}

		// Draw purple callsign if not cleared yet and 15 minutes out or already in airspace
		if (!fp.IsCleared && acFP.GetSectorEntryMinutes() < 15 && acFP.GetSectorEntryMinutes() > -1) {
			dc->SetTextColor(NoClearance.ToCOLORREF());
		}

		// Callsign
		text = acFP.GetCallsign();
		dc->TextOutA(tagRect.left + 1, tagRect.top, text.c_str());
		offsetY += 15;

		// Orange if observer
		if (!screen->GetPlugIn()->ControllerMyself().IsController()) {
			if (position.m_Longitude > -70 && position.m_Longitude < -5)
				textColour = TargetOrange.ToCOLORREF();
			if (position.m_Latitude < 80 && position.m_Longitude < -35)
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

		FontSelector::SelectMonoFont(12, dc);
		dc->SetTextColor(textColour);
		text = to_string(target->GetPosition().GetFlightLevel() / 100);
		dc->TextOutA(tagRect.left, tagRect.top + offsetY, text.c_str());
		offsetX += 50;

		// Mach
		int gs = target->GetPosition().GetReportedGS();
		text = "N" + to_string(gs);

		dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
		offsetX = 2;

		// Handoff initiated
		if (isHandoffToMe) {
			offsetY += 15;
			text = "H/O";
			dc->TextOutA(tagRect.right - dc->GetTextExtent("H/O").cx - 12, tagRect.top + offsetY, text.c_str());
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
			offsetY += 13;
		}

		if (tagPosition->first == true) {
			// Aircraft type
			text = acFP.GetFlightPlanData().GetAircraftFPType();
			dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());
			offsetX += 50;

			// Destination
			text = acFP.GetFlightPlanData().GetDestination();
			dc->TextOutA(tagRect.left + offsetX, tagRect.top + offsetY, text.c_str());

			tagRect.bottom += 25;
		}

		// Draw box if they are the asel
		if (acFP.GetCallsign() == asel) {
			// Draw purple box if not cleared yet and 15 minutes out or already in airspace
			if (!fp.IsCleared && (acFP.GetSectorEntryMinutes() < 15 && acFP.GetSectorEntryMinutes() > -1)) {
				dc->Draw3dRect(tagRect, NoClearance.ToCOLORREF(), NoClearance.ToCOLORREF());
			} else {
				dc->Draw3dRect(tagRect, textColour, textColour);
			}	
		}
				
		/// Tag line
		CSize txtExtent = dc->GetTextExtent(acFP.GetCallsign()); // Get callsign length

		// Pen
		CPen pen(PS_SOLID, 1, textColour);
		SolidBrush white(TextWhite);
		dc->SelectObject(pen);
		int tagMiddle = tagRect.left + ((tagRect.right - tagRect.left) / 2);

		// Dog leg
		if ((tagRect.left + txtExtent.cx + 5) < acPoint.x) {
			dc->MoveTo({ tagRect.left + txtExtent.cx + 2, tagRect.top + 8 });

			// Position
			if ((tagRect.right - (tagRect.right - acPoint.x)) > (tagRect.left + txtExtent.cx + 5)) {
				if ((tagRect.right - (tagRect.right - acPoint.x)) < tagRect.right + 5) {
					dc->LineTo({ tagRect.right - (tagRect.right - acPoint.x), tagRect.top + 8 });
					// Draw cleared dot TODO enhance this
					/*Rect temp(tagRect.right - (tagRect.right - acPoint.x), tagRect.top + 8, 
						tagRect.right - (tagRect.right - acPoint.x) + 1, tagRect.top + 9);
					if (!fp.IsCleared && !(acFP.GetSectorExitMinutes() > 15 || acFP.GetSectorExitMinutes() == -1))
						g->FillEllipse(&white, temp);*/
				}
				else {
					// Draw cleared dot TODO enhance this
					/*Rect temp(tagRect.right + 5, tagRect.top + 8,
						tagRect.right + 6, tagRect.top + 9);
					if (!fp.IsCleared && !(acFP.GetSectorExitMinutes() > 15 || acFP.GetSectorExitMinutes() == -1))
						g->FillEllipse(&white, temp);*/
					dc->LineTo({ tagRect.right + 5, tagRect.top + 8 });
				}
				dc->LineTo({ acPoint.x, acPoint.y });
			}
		}
		else { // Line to target
			dc->MoveTo({ tagRect.left - 2, tagRect.top + 8 });

			// Position
			if (tagRect.left - (tagRect.left - acPoint.x) - (txtExtent.cx + 2) > tagRect.left - 10) {
				dc->LineTo({ tagRect.left - (tagRect.left - acPoint.x) - (txtExtent.cx + 2), tagRect.top + 8 });
				// Draw cleared dot TODO enhance this
				/*Rect temp(tagRect.left - (tagRect.left - acPoint.x) - (txtExtent.cx + 2), tagRect.top + 8, 
					tagRect.left - (tagRect.left - acPoint.x) - (txtExtent.cx + 2) + 1, tagRect.top + 9);
				if (!fp.IsCleared && !(acFP.GetSectorExitMinutes() > 15 || acFP.GetSectorExitMinutes() == -1))
					g->FillEllipse(&white, temp);*/
			}
			else {
				// Draw cleared dot TODO enhance this
				/*Rect temp(tagRect.left - 10, tagRect.top + 8,
					tagRect.left - 9, tagRect.top + 9);
				if (!fp.IsCleared && !(acFP.GetSectorExitMinutes() > 15 || acFP.GetSectorExitMinutes() == -1))
					g->FillEllipse(&white, temp);*/
				dc->LineTo({ tagRect.left - 10, tagRect.top + 8 });
			}
			dc->LineTo(acPoint);
		}

		// Create screen object for tag
		screen->AddScreenObject(SCREEN_TAG, acFP.GetCallsign(), tagRect, true, (string(acFP.GetPilotName()) + " - " + string(acFP.GetFlightPlanData().GetRoute())).c_str());

		// Now create screen object for callsign
		CRect callsignRect(tagRect.left, tagRect.top, tagRect.left + dc->GetTextExtent(acFP.GetCallsign()).cx, tagRect.top + dc->GetTextExtent(acFP.GetCallsign()).cy);
		screen->AddScreenObject(SCREEN_TAG_CS, acFP.GetCallsign(), callsignRect, true, (string(acFP.GetPilotName()) + " - " + string(acFP.GetFlightPlanData().GetRoute())).c_str());

		// Restore context
		dc->RestoreDC(sDC);

		// Clean up
		DeleteObject(pen);
		DeleteObject(&white);
		DeleteObject(&textColour);

		// Compute render time and store it
		clock_t t = clock();
		double lastRenderTimeMs = (t - functionClock) / (CLOCKS_PER_SEC / 1000);

		// Log if render time was greater than 4ms
		if (lastRenderTimeMs >= 4) {
			CLogger::Log(CLogType::WARN, "TAG render time was >= 4ms. Elapsed: " + to_string((int)round(lastRenderTimeMs)) + "ms.", target->GetCallsign());
		}

		return { tagRect.left, tagRect.top };		
	}
	catch (exception &e) {
		// Log inside EuroScope
		screen->GetPlugIn()->DisplayUserMessage("vNAAATS", "Error", string(string(e.what())).c_str(), true, true, true, true, true);

		// Clogger
		CLogger::Log(CLogType::ERR, string(string(e.what())), "CAcTargets::RenderTag()");

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
