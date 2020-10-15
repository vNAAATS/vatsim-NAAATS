#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "InboundList.h"
#include "Constants.h"
#include "PathRenderer.h"
#include "DataHandler.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include <thread> 
#include <gdiplus.h>
#include <ctype.h>


using namespace Gdiplus;
using namespace std;
using namespace EuroScopePlugIn;

CRadarDisplay::CRadarDisplay() 
{
	COverlays::ShowHideGridReference(this, false);
	inboundList = new CInboundList({ CUtils::InboundX, CUtils::InboundY });
	otherList = new COtherList({ CUtils::OthersX, CUtils::OthersY });
	trackWindow = new CTrackInfoWindow({ CUtils::TrackWindowX, CUtils::TrackWindowY });
	fltPlnWindow = new CFlightPlanWindow({ 1000, 200 }); // TODO: settings save
	msgWindow = new CMessageWindow({ 500, 500 }); // TODO: settings save
	menuBar = new CMenuBar();
	asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
	fiveSecondTimer = clock();
	tenSecondTimer = clock();
}

CRadarDisplay::~CRadarDisplay() 
{
	delete inboundList;
	delete otherList;
	delete trackWindow;
	delete fltPlnWindow;
	delete msgWindow;
}

void CRadarDisplay::PopulateProgramData() {
	// Lists
	inboundList->MoveList({ CUtils::InboundX, CUtils::InboundY });
	otherList->MoveList({ CUtils::OthersX, CUtils::OthersY });

	// Dropdown values
	/*CMenuBar::dropDownSelections.find(MENDRP_AREASEL)->second = CMenuBar::ParseDropDownId(CUtils::AreaSelection, MENDRP_AREASEL);
	CMenuBar::dropDownSelections.find(MENDRP_TCKCTRL)->second = string("None");
	CMenuBar::dropDownSelections.find(MENDRP_OVERLAYS)->second = CMenuBar::ParseDropDownId(CUtils::SelectedOverlay, MENDRP_OVERLAYS);
	CMenuBar::dropDownSelections.find(MENDRP_TYPESEL)->second = CMenuBar::ParseDropDownId(CUtils::PosType, MENDRP_TYPESEL);*/

	// Buttons
	if (CUtils::TagsEnabled && menuBar->IsButtonPressed(CMenuBar::BTN_TAGS)) {
		menuBar->SetButtonState(CMenuBar::BTN_TAGS, CInputState::ACTIVE);
	}
	if (CUtils::GridEnabled && menuBar->IsButtonPressed(CMenuBar::BTN_GRID)) {
		menuBar->SetButtonState(CMenuBar::BTN_GRID, CInputState::ACTIVE);
	}
	if (CUtils::OverlayEnabled && menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS)) {
		menuBar->SetButtonState(CMenuBar::BTN_OVERLAYS, CInputState::ACTIVE);
	}
	if (CUtils::QckLookEnabled && menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK)) {
		menuBar->SetButtonState(CMenuBar::BTN_QCKLOOK, CInputState::ACTIVE);
	}

	// Parse track ID
	if (CUtils::SelectedOverlay == DRP_OVL_ALL) {
		COverlays::CurrentType = COverlayType::TCKS_ALL;
	}
	else if (CUtils::SelectedOverlay == DRP_OVL_EAST) {
		COverlays::CurrentType = COverlayType::TCKS_EAST;
	}
	else if (CUtils::SelectedOverlay == DRP_OVL_WEST) {
		COverlays::CurrentType = COverlayType::TCKS_WEST;
	}
	else {
		COverlays::CurrentType = COverlayType::TCKS_SEL;
	}

	// Download the tracks
	CDataHandler::PopulateLatestTrackData(GetPlugIn());

	// Initialise fonts
	FontSelector::InitialiseFonts();
}

// On radar screen refresh (occurs about once a second)
void CRadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	// 5 second timer
	double fiveSecT = (double)(clock() - fiveSecondTimer) / ((double)CLOCKS_PER_SEC);
	// 10 second timer
	double tenSecT = (double)(clock() - tenSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Clear lists if not empty and time is greater than 1 second
	if (fiveSecT >= 5 && !inboundList->AircraftList.empty()) {
		inboundList->AircraftList.clear();
	}
	if (fiveSecT >= 5 && !otherList->AircraftList.empty()) {
		otherList->AircraftList.clear();
	}

	// Reset currently on screen list
	if (tenSecT >= 10 && !aircraftOnScreen.empty()) {
		// Loop on screen aircraft
		auto idx = aircraftOnScreen.begin();
		while(idx != aircraftOnScreen.end()) {
			// Check if valid
			CRadarTarget target = GetPlugIn()->RadarTargetSelect(idx->first.c_str());
			if (!target.IsValid()) { // If not valid
				// Erase aircraft selections if they are an asel
				if (idx->first == aircraftSel1 || idx->first == aircraftSel2) { // We need to annul the selection and disable tool if activated
					aircraftSel1 = "";
					// Reset RBL (if active)
					if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
						menuBar->SetButtonState(CMenuBar::BTN_QCKLOOK, CInputState::INACTIVE);
					}
					// Reset SEP (if active)
					if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
						menuBar->SetButtonState(CMenuBar::BTN_SEP, CInputState::INACTIVE);
					}
					// Reset PIV (if active)
					if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
						menuBar->SetButtonState(CMenuBar::BTN_PIV, CInputState::INACTIVE);
					}
				}
				if (idx->first == asel) {
					asel = "";
				}
				// Finally erase the on screen reference
				idx = aircraftOnScreen.erase(idx);
			}
			else {
				++idx; // We increment here to avoid a vector overflow
			}
		}
	}

	// Redo the PIV calculations every 5 seconds
	if (fiveSecT >= 5 && menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
		CConflictDetection::PIVLocations1.clear();
		CConflictDetection::PIVLocations2.clear();
		CConflictDetection::PIVSeparationStatuses.clear();
		CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
	}
	
	// Check if the altitude filter is on
	bool altFiltEnabled = false;
	if (menuBar->IsButtonPressed(CMenuBar::BTN_ALTFILT)) altFiltEnabled = true;

	// Get the radar area
	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		// Draw overlays if enabled
		if (menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS)) {
			COverlays::ShowCurrentOverlay(&dc, &g, this);
		}

		// Get first aircraft
		CRadarTarget ac;
		ac = GetPlugIn()->RadarTargetSelectFirst();

		// Get entry time and direction
		int entryMinutes;
		bool direction;

		// Loop all aircraft
		while (ac.IsValid()) {
			// Very first thing we do is check their altitude, if they are outside the filter, skip them
			if (altFiltEnabled) {
				if (ac.GetPosition().GetPressureAltitude() / 100 < CUtils::AltFiltLow || ac.GetPosition().GetPressureAltitude() / 100 > CUtils::AltFiltHigh) {
					// Select the next target
					ac = GetPlugIn()->RadarTargetSelectNext(ac);
					if (aircraftOnScreen.find(ac.GetCallsign()) != aircraftOnScreen.end()) aircraftOnScreen.erase(ac.GetCallsign());
					continue;
				}
			}

			// Flight plan
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(ac.GetCallsign());

			// Route
			CFlightPlanExtractedRoute rte = fp.GetExtractedRoute();

			// Time and direction
			entryMinutes = fp.GetSectorEntryMinutes();
			direction = CUtils::GetAircraftDirection(ac.GetPosition().GetReportedHeading());

			// Parse inbound & other			
			if (entryMinutes >= 0 && entryMinutes < 60) {
				// If not there then add the status
				if (tagStatuses.find(fp.GetCallsign()) == tagStatuses.end()) {
					pair<bool, POINT> pt = make_pair(false, POINT{ 0, 0 });
					tagStatuses.insert(make_pair(string(fp.GetCallsign()), pt));
				}

				// STCA
				if (tenSecT >= 10) {
					CConflictDetection::CheckSTCA(this, &ac, &aircraftOnScreen);
				}

				if (fiveSecT >= 5) {
					// If going westbound
					if (!direction && entryMinutes > 0) {
						if (menuBar->GetDropDownValue(CMenuBar::DRP_AREASEL) == "CZQX") {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// Find out if 30 west is in their flight plan
								if (rte.GetPointPosition(i).m_Longitude == -30.0) {
									// Test flight time
									if (rte.GetPointDistanceInMinutes(i) > 0 && rte.GetPointDistanceInMinutes(i) < 60) {
										// Add if within
										inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
												rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), false));
										break;
									}
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
						else {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// They are coming from land so check entry points
								if (CUtils::IsEntryExitPoint(rte.GetPointName(i), direction)) {
									// Add if within
									inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
										rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), false));
									break;
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
					}
					else if (direction && entryMinutes > 0) {
						if (menuBar->GetDropDownValue(CMenuBar::DRP_AREASEL) == "EGGX") {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// Find out if 30 west is in their flight plan
								if (rte.GetPointPosition(i).m_Longitude == -30.0) {
									// Test flight time
									if (rte.GetPointDistanceInMinutes(i) > 0 && rte.GetPointDistanceInMinutes(i) < 60) {
										// Add if within
										inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
											rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), true));
										break;
									}
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
						else {
							// They are coming from land so check entry points
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								if (CUtils::IsEntryExitPoint(rte.GetPointName(i), direction)) {
									// Add if within
									inboundList->AircraftList.push_back(CInboundAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
										rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), true));
									break;
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherList->AircraftList.push_back(ac.GetCallsign());
							}
						}
					}
				}

				// Store whether detailed tags are enabled
				bool detailedEnabled = false;

				// Now we check if all the tags are selected as detailed				
				if (menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK)) {
					detailedEnabled = true; // Set detailed on

					// Unpress detailed if not already
					if (menuBar->IsButtonPressed(CMenuBar::BTN_DETAILED) && !aselDetailed) {
						menuBar->SetButtonState(CMenuBar::BTN_DETAILED, CInputState::INACTIVE);
					}
				}

				// Check if only one is set to detailed
				if (menuBar->IsButtonPressed(CMenuBar::BTN_DETAILED)) {
					if (fp.GetCallsign() == asel) {
						detailedEnabled = true; // Set detailed on
					}

					// Unpress quick look if not already
					if (menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK) && aselDetailed) {
						menuBar->SetButtonState(CMenuBar::BTN_QCKLOOK, CInputState::INACTIVE);
					}
				}

				bool ptl = false;
				bool halo = false;
				// Set PTL and HALO if they are on
				if (menuBar->IsButtonPressed(CMenuBar::BTN_PTL)) {
					ptl = true;
				}
				if (menuBar->IsButtonPressed(CMenuBar::BTN_HALO)) {
					halo = true;
				}

				// Get STCA so it can be drawn
				CSTCAStatus stcaStatus(ac.GetCallsign(), "", CConflictStatus::OK); // Create default
				auto idx = CConflictDetection::CurrentSTCA.begin();
				for (idx = CConflictDetection::CurrentSTCA.begin(); idx != CConflictDetection::CurrentSTCA.end(); idx++) {
					if (ac.GetCallsign() == idx->CallsignA || ac.GetCallsign() == idx->CallsignB) {
						stcaStatus = *idx;
						break; // Break for optimisation
					}
				}

				// Draw the tag and target with the information if tags are turned on and within altitude filter
				if (menuBar->IsButtonPressed(CMenuBar::BTN_TAGS)) {
					if (aircraftOnScreen.find(ac.GetCallsign()) == aircraftOnScreen.end()) aircraftOnScreen.insert(make_pair(ac.GetCallsign(), 0));
					auto kv = tagStatuses.find(fp.GetCallsign());
					kv->second.first = detailedEnabled; // Set detailed on
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, true, &menuBar->GetToggleButtons(), halo, ptl, &stcaStatus);
					CAcTargets::DrawTag(&dc, this, &ac, &kv->second, direction, &stcaStatus);
					
				}
				else {
					if (aircraftOnScreen.find(ac.GetCallsign()) == aircraftOnScreen.end()) aircraftOnScreen.insert(make_pair(ac.GetCallsign(), 0));
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, false, &menuBar->GetToggleButtons(), halo, ptl, &stcaStatus);
				}
			}
			else { // If not there, and the aircraft was on the screen, then delete
				if (aircraftOnScreen.find(ac.GetCallsign()) != aircraftOnScreen.end()) aircraftOnScreen.erase(ac.GetCallsign());
			}

			// Select the next target
			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}


		// Clear ASELs if none of the range/separation tools are pressed
		if (!menuBar->IsButtonPressed(CMenuBar::BTN_PIV)
			&& !menuBar->IsButtonPressed(CMenuBar::BTN_RBL)
			&& !menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
			// Reset ASELs if none enabled
			aircraftSel1 = "";
			aircraftSel2 = "";
		}


		/// RENDERING
		// Draw menu bar
		menuBar->RenderBar(&dc, &g, this, asel);

		// Check the lists are not empty first, then draw
		inboundList->RenderList(&g, &dc, this);
		otherList->RenderList(&g, &dc, this);

		// SEP draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
			// If both aircraft selected then draw
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				CConflictDetection::SepTool(&dc, &g, this, aircraftSel1, aircraftSel2);
			}
		}

		// RBL draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				CConflictDetection::RBLTool(&dc, &g, this, aircraftSel1, aircraftSel2);
			}
		}

		// PIV draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
			// If both aircraft selected then draw
			if (aircraftSel1 != "" && aircraftSel2 != "") {
				// Render
				CPathRenderer::RenderPath(&dc, &g, this, CPathType::PIV);
			}
		}

		// Draw track info window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_TCKINFO)) {
			trackWindow->RenderWindow(&dc, &g, this);
		}

		// Draw message window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_MESSAGE)) {
			msgWindow->RenderWindow(&dc, &g, this);
		}

		// Draw flight plan window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->RenderWindow(&dc, &g, this);
		}

		// Finally, reset the clocks if time has been exceeded
		if (fiveSecT >= 5) {
			fiveSecondTimer = clock();
		}
		if (tenSecT >= 10) {
			tenSecondTimer = clock();
		}
		if (double twoSecT = (double)(clock() - CAcTargets::twoSecondTimer) / ((double)CLOCKS_PER_SEC) >= 2.2) { // Ac target and tag colours
			CAcTargets::twoSecondTimer = clock();
		}
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
	dc.DeleteDC();
}

void CRadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	// Mouse pointer
	mousePointer = Pt;
	// Move inbound list
	if (ObjectType == LIST_INBOUND) {
		inboundList->MoveList(Area);

		// To save
		CUtils::InboundX = Area.left;
		CUtils::InboundY = Area.top;
	}

	// Move other list
	if (ObjectType == LIST_OTHERS) {
		otherList->MoveList(Area);

		CUtils::OthersX = Area.left;
		CUtils::OthersY = Area.top;
	}

	// Move tag
	if (ObjectType == SCREEN_TAG) {
		auto kv = tagStatuses.find(sObjectId);
		POINT acPosPix = ConvertCoordFromPositionToPixel(GetPlugIn()->RadarTargetSelect(sObjectId).GetPosition().GetPosition());
		kv->second.second = { Area.left - acPosPix.x, Area.top - acPosPix.y };
	}

	// Move window
	if (ObjectType == WINDOW) {
		if (string(sObjectId) == "TCKINFO")
		trackWindow->MoveWindow(Area);

		if (string(sObjectId) == "FLTPLN")
			fltPlnWindow->MoveWindow(Area);

		if (string(sObjectId) == "MSG")
			msgWindow->MoveWindow(Area);

		CUtils::TrackWindowX = Area.left;
		CUtils::TrackWindowY = Area.top;
	}

	// Scrolling
	if (ObjectType == WIN_SCROLLBAR) {
		if (string(sObjectId) == "TCKINFO") trackWindow->Scroll(Area, mousePointer);
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) 
{
	// Dropdown
	if (ObjectType == MENBAR) {
		if (atoi(sObjectId) >= 800) {
			menuBar->OnOverDropDownItem(atoi(sObjectId));
		}
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// If menu button
	if (ObjectType == MENBAR) {
		if (!menuBar->IsButtonPressed(atoi(sObjectId))) {
			menuBar->ButtonPress(atoi(sObjectId), Button);
		}
		else {
			menuBar->ButtonUnpress(atoi(sObjectId), Button);
		}
	}

	// Left button actions
	if (Button == BUTTON_LEFT) {
		// If screen object is a tag
		if (ObjectType == SCREEN_TAG) {
			// Set the ASEL
			asel = sObjectId;
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);
			GetPlugIn()->SetASELAircraft(fp);

			if (menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
				fltPlnWindow->UpdateData(this, CAcFPStatus(asel, CFlightPlanMode::INIT));
			}			

			// Probing tools
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)
				|| menuBar->IsButtonPressed(CMenuBar::BTN_RBL)
				|| menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				if (aircraftSel1 == "") {
					aircraftSel1 = asel;
				}
				else if (aircraftSel2 == "") {
					aircraftSel2 = asel;
				}
			}

			if (fp.GetCallsign() == aircraftSel2 && menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
			}
		}

		// Qck Look button
		if (atoi(sObjectId) == CMenuBar::BTN_QCKLOOK) {
			aselDetailed = false;
		}

		// Detailed button
		if (atoi(sObjectId) == CMenuBar::BTN_DETAILED) {
			aselDetailed = true;
		}

		// If the button is the PIV button
		if (atoi(sObjectId) == CMenuBar::BTN_PIV) {
			// Erase RBL (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
				menuBar->SetButtonState(CMenuBar::BTN_RBL, CInputState::INACTIVE);
			}

			// Erase SEP (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				menuBar->SetButtonState(CMenuBar::BTN_SEP, CInputState::INACTIVE);
			}

			// If PIV already active then clear everything
			if (!CConflictDetection::PIVSeparationStatuses.empty()) {
				CConflictDetection::PIVLocations1.clear();
				CConflictDetection::PIVLocations2.clear();
				CConflictDetection::PIVSeparationStatuses.clear();
			}
			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If the button is the RBL button
		if (atoi(sObjectId) == CMenuBar::BTN_RBL) {
			// Erase PIV (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				menuBar->SetButtonState(CMenuBar::BTN_PIV, CInputState::INACTIVE);
			}

			// Erase SEP (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				menuBar->SetButtonState(CMenuBar::BTN_SEP, CInputState::INACTIVE);
			}
			
			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If the button is the SEP button
		if (atoi(sObjectId) == CMenuBar::BTN_SEP) {
			// Erase RBL (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_RBL)) {
				menuBar->SetButtonState(CMenuBar::BTN_RBL, CInputState::INACTIVE);
			}

			// Erase PIV (if active)
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				menuBar->SetButtonState(CMenuBar::BTN_PIV, CInputState::INACTIVE);
			}

			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If a menu text entry
		/*if (ObjectType == TXT_ENTRY) {
			// If the low altitude filter
			if (string(sObjectId) == "ALTFILT_LOW") {
				GetPlugIn()->OpenPopupEdit(Area, FUNC_ALTFILT_LOW, "");
			}
			// If the high altitude filter
			if (string(sObjectId) == "ALTFILT_HIGH") {
				GetPlugIn()->OpenPopupEdit(Area, FUNC_ALTFILT_HIGH, "");
			}
		}*/

		// If a flight plan window text entry
		if (ObjectType == WIN_FLTPLN) {
			if (fltPlnWindow->IsTextInput(atoi(sObjectId))) {
				GetPlugIn()->OpenPopupEdit(Area, atoi(sObjectId), "");
			}
		}
	}
	
	if (Button == BUTTON_RIGHT) {
		if (ObjectType == SCREEN_TAG) {
			// Set the ASEL
			asel = sObjectId;
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);
			GetPlugIn()->SetASELAircraft(fp);

			// Set route drawing
			if (CPathRenderer::RouteDrawTarget == "" || fp.GetCallsign() != CPathRenderer::RouteDrawTarget) {
				CPathRenderer::RouteToDraw = CPathRenderer::GetRoute(this, fp.GetCallsign());
				CPathRenderer::RouteDrawTarget = fp.GetCallsign();
			}
			else {
				CPathRenderer::ClearCurrentRoute();
			}
		}
	}
	
	RequestRefresh();
}

void CRadarDisplay::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Track info window
	if (ObjectType == WIN_TCKINFO) {
		trackWindow->ButtonDown(atoi(sObjectId));
	}

	// Flight plan window
	if (ObjectType == WIN_FLTPLN) {
		fltPlnWindow->ButtonDown(atoi(sObjectId));
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Track info window
	if (ObjectType == WIN_TCKINFO) {
		trackWindow->ButtonUp(atoi(sObjectId));
	}

	// Flight plan window
	if (ObjectType == WIN_FLTPLN) {
		fltPlnWindow->ButtonUp(atoi(sObjectId));
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	// Set low alt filter
	if (FunctionId == FUNC_ALTFILT_LOW) {
		// Validation (range & type)
		// Check if it is a string
		bool isNumber = true;
		for (int i = 0; i < strlen(sItemString); i++) { // Check if string
			if (!isdigit(sItemString[i])) isNumber = false;
		}
		if (isNumber && (atoi(sItemString) < 1000 && atoi(sItemString) > 0)) {
			CUtils::AltFiltLow = atoi(sItemString); // Return if in range
		}
		
	}
	// Set high alt filter
	if (FunctionId == FUNC_ALTFILT_HIGH) {
		// Validation (range & type)
		bool isNumber = true;
		for (int i = 0; i < strlen(sItemString); i++) { // Check if string
			if (!isdigit(sItemString[i])) isNumber = false;
		}
		if (isNumber && (atoi(sItemString) < 999 && atoi(sItemString) > 0)) {
			CUtils::AltFiltHigh = atoi(sItemString); // Return if in range
		}
	}

	// If it is a flight plan window text input
	if (fltPlnWindow->IsTextInput(FunctionId) && string(sItemString) != "") {
		fltPlnWindow->ChangeDataPoint(this, FunctionId, string(sItemString));
	}
}

void CRadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

}

void CRadarDisplay::OnAsrContentToBeSaved(void)
{
	// Buttons
	CUtils::TagsEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_TAGS) ? true : false;
	CUtils::GridEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_GRID) ? true : false;
	CUtils::OverlayEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS) ? true : false;
	CUtils::QckLookEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK) ? true : false;

	CUtils::SavePluginData(this);
}

void CRadarDisplay::OnAsrContentLoaded(bool Loaded)
{
	if (!Loaded)
		return;

	// Load the plugin data
	CUtils::LoadPluginData(this);

	// Populate it
	PopulateProgramData();
}