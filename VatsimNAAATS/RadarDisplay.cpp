#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "InboundList.h"
#include "Constants.h"
#include "DataHandler.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include "DataHandler.h"
#include <thread>
#include <gdiplus.h>
#include <ctype.h>
#include <iostream>

using namespace Gdiplus;
using namespace std;
using namespace EuroScopePlugIn;

CRadarDisplay::CRadarDisplay()
{
	//COverlays::ShowHideGridReference(this, false);
	inboundList = new CInboundList({ CUtils::InboundX, CUtils::InboundY });
	otherList = new COtherList({ CUtils::OthersX, CUtils::OthersY });
	//rclList = new CRCLList({ CUtils::RCLX, CUtils::RCLY }); // TODO: settings save
	conflictList = new CConflictList({ CUtils::ConflictX, CUtils::ConflictY }); // TODO: settings save
	trackWindow = new CTrackInfoWindow({ CUtils::TrackWindowX, CUtils::TrackWindowY });
	fltPlnWindow = new CFlightPlanWindow({ 1000, 300 }); // TODO: settings save
	msgWindow = new CMessageWindow({ 500, 500 }); // TODO: settings save
	npWindow = new CNotePad({ 300, 300 }, { 800, 200 }); // TODO: save settings
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
	menuBar->SetDropDownValue(CMenuBar::DRP_AREASEL, CUtils::AreaSelection);
	menuBar->SetDropDownValue(CMenuBar::DRP_OVERLAYS, CUtils::SelectedOverlay);
	menuBar->SetDropDownValue(CMenuBar::DRP_TYPESEL, CUtils::PosType);

	// Buttons
	if (CUtils::TagsEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_TAGS)) {
		menuBar->SetButtonState(CMenuBar::BTN_TAGS, CInputState::ACTIVE);
	}
	if (CUtils::GridEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_GRID)) {
		menuBar->SetButtonState(CMenuBar::BTN_GRID, CInputState::ACTIVE);
	}
	if (CUtils::OverlayEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS)) {
		menuBar->SetButtonState(CMenuBar::BTN_OVERLAYS, CInputState::ACTIVE);
	}
	if (CUtils::QckLookEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK)) {
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

	// Set tracks in menu bar
	menuBar->MakeDropDownItems(menuBar->DRP_TCKCTRL);

	// Get DLL path
	GetModuleFileNameA(HINSTANCE(&__ImageBase), CUtils::DllPathFile, sizeof(CUtils::DllPathFile));
	CUtils::DllPath = CUtils::DllPathFile;
	CUtils::DllPath.resize(CUtils::DllPath.size() - strlen("VatsimNAAATS.dll"));

	// Initialise fonts
	FontSelector::InitialiseFonts();

	// Start cursor update loop
	appCursor->screen = this;
	_beginthread(CursorStateUpdater, 0, (void*) appCursor);
}

// On radar screen refresh (modified to occur 4 times a second)
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

	// Online controllers
	if (fiveSecT >= 5) {
		// Clear online controllers first
		if (!fltPlnWindow->onlineControllers.empty())
			fltPlnWindow->onlineControllers.clear();
		// Get controllers
		CController controller;
		for (controller = GetPlugIn()->ControllerSelectFirst(); controller.IsValid(); controller = GetPlugIn()->ControllerSelectNext(controller)) {
			if (string(controller.GetCallsign()).find("CTR") != string::npos || string(controller.GetCallsign()).find("FSS") != string::npos)
			fltPlnWindow->onlineControllers[controller.GetCallsign()] = controller;
		}
	}

	// Set the ASEL if it is different to the current one in program
	string cs = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
	if (GetPlugIn()->FlightPlanSelectASEL().GetCallsign() != asel) {
		asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
	}

	// Set the flight plan button state
	if (aircraftOnScreen.empty() || asel == "" || !CDataHandler::GetFlightData(asel)->IsValid || !CDataHandler::GetFlightData(asel)->IsFirstUpdate || !GetPlugIn()->ControllerMyself().IsController()) {
		if (menuBar->GetButtonState(CMenuBar::BTN_FLIGHTPLAN) != CInputState::DISABLED)
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::DISABLED);
	}
	else {
		if (menuBar->GetButtonState(CMenuBar::BTN_FLIGHTPLAN) == CInputState::DISABLED)
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::INACTIVE);
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
						menuBar->SetButtonState(CMenuBar::BTN_RBL, CInputState::INACTIVE);
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
				// Erase ASEL
				if (idx->first == asel) {
					asel = "";
				}

				// Erase STCA if exists
				auto jdx = CConflictDetection::CurrentSTCA.begin();
				// Loop this way to avoid a vector overflow
				while (jdx != CConflictDetection::CurrentSTCA.end()) {
					if (idx->first == jdx->CallsignA || idx->first == jdx->CallsignB) {
						jdx = CConflictDetection::CurrentSTCA.erase(jdx);
					}
					else {
						jdx++;
					}
				}

				// Erase route if exists
				auto kdx = CRoutesHelper::ActiveRoutes.begin();
				// Loop this way to avoid a vector overflow
				while (kdx != CRoutesHelper::ActiveRoutes.end()) {
					if (idx->first == *kdx || idx->first == *kdx) {
						kdx = CRoutesHelper::ActiveRoutes.erase(kdx);
					}
					else {
						kdx++;
					}
				}

				if (CDataHandler::GetFlightData(idx->first)->IsValid) {
					CDataHandler::DeleteFlightData(idx->first);
				}

				// Erase flight plan window
				menuBar->SetButtonState(menuBar->BTN_FLIGHTPLAN, CInputState::DISABLED);
				fltPlnWindow->IsOpen = false;

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
		/// Write current lat lon on screen
		int sDC = dc.SaveDC();
		// Select font
		FontSelector::SelectMonoFont(12, &dc);
		dc.SetTextColor(TextWhite.ToCOLORREF());
		dc.SetTextAlign(TA_LEFT);
		// Get radar area and lat lon
		CRect radarBounds = this->GetRadarArea();
		dc.TextOutA(radarBounds.right - 185, radarBounds.bottom - dc.GetTextExtent("ABC").cy - 2, 
			CUtils::GetLatLonString(&appCursor->latLonPosition).c_str());
		dc.RestoreDC(sDC);

		// Draw overlays if enabled
		if (menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS)) {
			COverlays::ShowCurrentOverlay(&dc, &g, this, menuBar);
		}

		// Get first aircraft
		CRadarTarget ac;
		ac = GetPlugIn()->RadarTargetSelectFirst();

		// Get entry time and direction
		int entryMinutes;
		bool direction;

		// Draw routes
		if (CRoutesHelper::ActiveRoutes.size() != CRoutesHelper::ActiveRoutes.empty() && CRoutesHelper::ActiveRoutes.size() != 0) {
			CCommonRenders::RenderRoutes(&dc, &g, this);
		}

		// Loop all aircraft
		while (ac.IsValid()) {
			// The system plan
			CAircraftFlightPlan aircraftFlightPlan;
			CDataHandler::GetFlightData(cs.c_str(), aircraftFlightPlan);

			// Check if they are a selected aircraft
			if (ac.GetCallsign() == CAcTargets::SearchedAircraft) {
				if (((double)(clock() - CAcTargets::fiveSecondTimer) / ((double)CLOCKS_PER_SEC)) >= 5) {
					CAcTargets::SearchedAircraft = "";
					CAcTargets::fiveSecondTimer = clock();
				}
				else {
					CAcTargets::RenderSelectionHalo(&g, this, &ac);
				}
			}
			// If PSSR button not pressed
			if (!menuBar->IsButtonPressed(CMenuBar::BTN_PSSR)) {
				// Check their PSSR state to hide all non ADS-B aircraft
				if (CUtils::GetTargetMode(ac.GetPosition().GetRadarFlags()) != CRadarTargetMode::ADS_B) {
					// Select the next target
					ac = GetPlugIn()->RadarTargetSelectNext(ac);
					if (aircraftOnScreen.find(ac.GetCallsign()) != aircraftOnScreen.end()) aircraftOnScreen.erase(ac.GetCallsign());
					continue;
				}
			}

			// Check their altitude, if they are outside the filter, skip them
			if (altFiltEnabled && !menuBar->IsButtonPressed(CMenuBar::BTN_ALL)) {
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

			// Check track filtering
			if (menuBar->GetButtonState(menuBar->BTN_TCKCTRL) == CInputState::ACTIVE && !menuBar->IsButtonPressed(CMenuBar::BTN_ALL)) {
				// Primed plan
				string cs = (string)fp.GetCallsign();
				vector<string> tracks;
				auto idx = find_if(CConflictDetection::CurrentSTCA.begin(), CConflictDetection::CurrentSTCA.end(), [&cs](const CSTCAStatus& obj) {return obj.CallsignA == cs || obj.CallsignB == cs; });
				if (idx == CConflictDetection::CurrentSTCA.end())
					menuBar->GetSelectedTracks(tracks);
				bool skipAircraft = tracks.empty() ? false : true;
				for (int i = 0; i < tracks.size(); i++) {
					if (aircraftFlightPlan.IsValid) {
						bool showAc = aircraftFlightPlan.Track == tracks[i];
						if (showAc) {
							skipAircraft = false;
							break;
						}						
					}
					else if (CRoutesHelper::OnNatTrack(this, string(fp.GetCallsign())) == tracks[i]) {
						skipAircraft = false;
						break;
					}
					else {
						continue;
					}
				}

				if (skipAircraft) {
					// Select the next target
					ac = GetPlugIn()->RadarTargetSelectNext(ac);
					continue;
				}
			}
			
			// Parse inbound & other
			bool filtersDisabled = menuBar->IsButtonPressed(CMenuBar::BTN_ALL);
			if (CUtils::IsAircraftRelevant(this, &ac, filtersDisabled)) {
				
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
								if (CUtils::IsEntryPoint(rte.GetPointName(i), direction) || CUtils::IsExitPoint(rte.GetPointName(i), direction)) {
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
								if (CUtils::IsEntryPoint(rte.GetPointName(i), direction) || CUtils::IsExitPoint(rte.GetPointName(i), direction)) {
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
				if (menuBar->IsButtonPressed(CMenuBar::BTN_EXT)) {
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

					// Unpress extended if not already
					if (menuBar->IsButtonPressed(CMenuBar::BTN_EXT) && aselDetailed) {
						menuBar->SetButtonState(CMenuBar::BTN_EXT, CInputState::INACTIVE);
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
				CSTCAStatus stcaStatus(ac.GetCallsign(), "", CConflictStatus::OK, -1, -1); // Create default
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
					CAcTargets::RenderTarget(&g, &dc, this, &ac, true, &menuBar->GetToggleButtons(), halo, ptl, &stcaStatus);
					POINT tagPosition = CAcTargets::RenderTag(&dc, this, &ac, &kv->second, direction, &stcaStatus);

					// If tracking dialog open
					if (CAcTargets::OpenTrackingDialog != "" && CAcTargets::OpenTrackingDialog == ac.GetCallsign()) {
						CAcTargets::RenderCoordTagItem(&dc, this, ac.GetCallsign(), tagPosition);
					}

				}
				else {
					if (aircraftOnScreen.find(ac.GetCallsign()) == aircraftOnScreen.end()) aircraftOnScreen.insert(make_pair(ac.GetCallsign(), 0));
					CAcTargets::RenderTarget(&g, &dc, this, &ac, false, &menuBar->GetToggleButtons(), halo, ptl, &stcaStatus);
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

		// Draw lists
		inboundList->RenderList(&g, &dc, this);
		otherList->RenderList(&g, &dc, this);
		//rclList->RenderList(&g, &dc, this);
		conflictList->RenderList(&g, &dc, this);

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
				CConflictDetection::RenderPIV(&dc, &g, this, aircraftSel1, aircraftSel2);
			}
		}

		// QDM draw
		if (menuBar->IsButtonPressed(CMenuBar::BTN_QDM)) {
			// Check if first position is set
			if (RulerPoint1.m_Latitude != 0.0 && RulerPoint1.m_Longitude != 0.0) {
				// Render
				CCommonRenders::RenderQDM(&dc, &g, this, &RulerPoint1, &RulerPoint2, appCursor->position, &appCursor->latLonPosition);
			}
		}

		// Draw track info window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_TCKINFO)) {
			trackWindow->RenderWindow(&dc, &g, this, menuBar);
		}

		// Draw message window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_MESSAGE)) {
			msgWindow->RenderWindow(&dc, &g, this);
		}

		// Draw flight plan window if button pressed
		if (fltPlnWindow->IsOpen && !menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::ACTIVE);
		}
		if (menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->RenderWindow(&dc, &g, this);
		}

		// Draw flight plan window if button pressed
		if (menuBar->IsButtonPressed(CMenuBar::BTN_NOTEPAD)) {
			npWindow->RenderWindow(&dc, &g, this);
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

// Data updates
void CRadarDisplay::OnRadarTargetPositionUpdate(CRadarTarget RadarTarget) {
	// Check if they are relevant on the screen
	bool filtersDisabled = menuBar->IsButtonPressed(CMenuBar::BTN_ALL);
	if (CUtils::IsAircraftRelevant(this, &RadarTarget, filtersDisabled)) {
		// They are relevant so get the flight plan
		CAircraftFlightPlan* fp = CDataHandler::GetFlightData(RadarTarget.GetCallsign());

		// If not valid then it doesn't exist and we need to make it
		if (!fp->IsValid) {
			// Create it
			CDataHandler::CreateFlightData(this, RadarTarget.GetCallsign());
		}
		else {
			// Update exit time
			int exitMinutes = GetPlugIn()->FlightPlanSelect(RadarTarget.GetCallsign()).GetSectorExitMinutes();
			if (exitMinutes != -1) {
				fp->ExitTime = exitMinutes;
			}

			// Download the vNAAATS network data on the plane
			CUtils::CNetworkAsyncData* data = new CUtils::CNetworkAsyncData();
			data->Screen = this;
			data->Callsign = fp->Callsign;
			if (!fp->IsFirstUpdate) fp->IsFirstUpdate = true;
			_beginthread(CDataHandler::DownloadNetworkAircraft, 0, (void*)data); // Async
		}
	}
	else { // Not relevant
		// Check if they have a flight plan data object
		if (CDataHandler::GetFlightData(RadarTarget.GetCallsign())->IsValid) {
			// Delete the flight data object
			CDataHandler::DeleteFlightData(RadarTarget.GetCallsign());
		}
	}
}

void CRadarDisplay::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	// Erase any route drawing
	if (CRoutesHelper::ActiveRoutes.size() != 0 || CRoutesHelper::ActiveRoutes.size() != CRoutesHelper::ActiveRoutes.empty()) {
		int found = -1; // Found flag so we can remove if needed
		for (int i = 0; i < CRoutesHelper::ActiveRoutes.size(); i++) {
			// If the route is currently on the screen
			if (CRoutesHelper::ActiveRoutes[i] == FlightPlan.GetCallsign()) {
				// Set to remove
				found = i;
				break;
			}
		}

		// Erase if the item was found, otherwise add
		if (found != -1) {
			CRoutesHelper::ActiveRoutes.erase(CRoutesHelper::ActiveRoutes.begin() + found);
		}
	}
}

void CRadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
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

	// Move conflict list
	if (ObjectType == LIST_CONFLICT) {
		conflictList->MoveList(Area);

		// To save
		CUtils::ConflictX = Area.left;
		CUtils::ConflictY = Area.top;
	}

	// Move RCLs list
	if (ObjectType == LIST_RCLS) {
		//rclList->MoveList(Area);

		CUtils::RCLX = Area.left;
		CUtils::RCLY = Area.top;
	}

	// Move tag
	if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_TAG_CS) {
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

		if (string(sObjectId) == "NOTEPAD")
			npWindow->MoveWindow(Area);

		CUtils::TrackWindowX = Area.left;
		CUtils::TrackWindowY = Area.top;
	}

	// Move subwindows for flight plan
	if (ObjectType == WIN_FLTPLN) {
		if (atoi(sObjectId) >= 400 && atoi(sObjectId) <= 420) {
			fltPlnWindow->MoveSubWindow(atoi(sObjectId), { Area.left, Area.top });
		}
		if (atoi(sObjectId) >= 500) {
			fltPlnWindow->Scroll(atoi(sObjectId), Pt, mousePointer);
		}
	}

	// Scrolling
	if (ObjectType == WIN_SCROLLBAR) {
		if (string(sObjectId) == "TCKINFO") trackWindow->Scroll(Area, mousePointer);
		if (string(sObjectId) == "520") msgWindow->Scroll(atoi(sObjectId), Pt, mousePointer);
	}

	// Mouse pointer
	mousePointer = Pt;

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area)
{
	mousePointer = Pt;
	// Dropdown
	if (ObjectType == MENBAR) {
		if (atoi(sObjectId) >= 800) {
			menuBar->OnOverDropDownItem(atoi(sObjectId));
		}
	}
	// If it is a message
	else if (ObjectType == ACTV_MESSAGE) {
		msgWindow->SelectedMessage = atoi(sObjectId);
	}
	// If it is a button on the tracking dialog
	if (ObjectType == SCREEN_TAG_CS_BTN) {
		CAcTargets::ButtonStates[sObjectId] = true;
	}
	// TODO: button state reset
	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// If menu button
	if (ObjectType == MENBAR) {
		if (Button == BUTTON_RIGHT) { // Toggle buttons
			menuBar->ButtonPress(atoi(sObjectId), Button, this);
		}
		else {
			if (!menuBar->IsButtonPressed(atoi(sObjectId))) {
				// Increase refresh resolution for QDM
				if (atoi(sObjectId) == CMenuBar::BTN_QDM) {
					RefreshResolution = 0.04;
				}
				menuBar->ButtonPress(atoi(sObjectId), Button, this);
			}
			else {
				// Disable all QDM if it is the QDM button
				if (atoi(sObjectId) == CMenuBar::BTN_QDM) {
					RulerPoint1.m_Latitude = 0.0;
					RulerPoint1.m_Longitude = 0.0;
					RulerPoint2.m_Latitude = 0.0;
					RulerPoint2.m_Longitude = 0.0;

					// Reset refresh resolution
					RefreshResolution = 0.2;
				}
				menuBar->ButtonUnpress(atoi(sObjectId), Button, this);
			}
		}
	} else if (ObjectType == WIN_FLTPLN) {
		if (!fltPlnWindow->IsButtonPressed(atoi(sObjectId))) {
			fltPlnWindow->ButtonPress(atoi(sObjectId));
		}
		else {
			fltPlnWindow->ButtonUnpress(atoi(sObjectId));
		}
	}

	// Left button actions
	if (Button == BUTTON_LEFT) {
		// If screen object is a tag
		if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_TAG_CS) {
			// Set the ASEL
			asel = sObjectId;
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);
			GetPlugIn()->SetASELAircraft(fp);

			// Re-instantiate flight plan window if the aircraft is not the currently ASELed one
			if (asel != sObjectId && fltPlnWindow->IsOpen) {
				fltPlnWindow->Instantiate(this, asel);
			}

			// Probing tools
			if (menuBar->IsButtonPressed(CMenuBar::BTN_PIV)
				|| menuBar->IsButtonPressed(CMenuBar::BTN_RBL)
				|| menuBar->IsButtonPressed(CMenuBar::BTN_SEP)) {
				// Make sure flight plans are valid
				if (aircraftSel1 == "" && CDataHandler::GetFlightData(asel)->IsValid) {
					aircraftSel1 = asel;
				}
				else if (aircraftSel2 == "" && aircraftSel1 != asel && CDataHandler::GetFlightData(asel)->IsValid) {
					aircraftSel2 = asel;
				}
			}

			if (fp.GetCallsign() == aircraftSel2 && menuBar->IsButtonPressed(CMenuBar::BTN_PIV)) {
				CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
			}
		}

		// Flight plan button
		if (atoi(sObjectId) == CMenuBar::BTN_FLIGHTPLAN && menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->Instantiate(this, asel);
		}

		if (!menuBar->IsButtonPressed(CMenuBar::BTN_FLIGHTPLAN)) {
			fltPlnWindow->IsOpen = false;
		}

		// Qck Look button
		if (atoi(sObjectId) == CMenuBar::BTN_EXT) {
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
		if (ObjectType == ALTFILT_TEXT) {
			// If the low altitude filter
			if (string(sObjectId) == "ALTFILT_LOW") {
				GetPlugIn()->OpenPopupEdit(Area, FUNC_ALTFILT_LOW, "");
			}
			// If the high altitude filter
			if (string(sObjectId) == "ALTFILT_HIGH") {
				GetPlugIn()->OpenPopupEdit(Area, FUNC_ALTFILT_HIGH, "");
			}
		}

		if (ObjectType == MENBAR) {
			if (string(sObjectId) == to_string(menuBar->TXT_SEARCH))
				GetPlugIn()->OpenPopupEdit(Area, atoi(sObjectId), "");
		}

		// If a flight plan window text entry
		if (ObjectType == WIN_FLTPLN) {
			if (fltPlnWindow->IsTextInput(atoi(sObjectId)) && (fltPlnWindow->GetInputState(atoi(sObjectId)) != CInputState::DISABLED || fltPlnWindow->GetInputState(atoi(sObjectId)) != CInputState::INACTIVE)) {
				GetPlugIn()->OpenPopupEdit(Area, atoi(sObjectId), fltPlnWindow->GetTextValue(atoi(sObjectId)).c_str());
			}
		}

		// If it is a hide show button for a list
		if (ObjectType == LIST_INBOUND) {
			if (string(sObjectId) == "HIDESHOW") {
				inboundList->HideShowButton = inboundList->HideShowButton == true ? false : true;
			}
		}
		if (ObjectType == LIST_OTHERS) {
			if (string(sObjectId) == "HIDESHOW") {
				otherList->HideShowButton = otherList->HideShowButton == true ? false : true;
			}
		}

		// If it is a message
		if (ObjectType == ACTV_MESSAGE) {
			msgWindow->SelectedMessage = atoi(sObjectId);
		}

		// If a coordination
		if (ObjectType == WIN_FLTPLN_TSFR) {
			if (GetPlugIn()->FlightPlanSelect(fltPlnWindow->primedPlan->Callsign.c_str()).GetTrackingControllerIsMe() &&
				string(GetPlugIn()->FlightPlanSelect(fltPlnWindow->primedPlan->Callsign.c_str()).GetHandoffTargetControllerCallsign()) == "") {
				fltPlnWindow->selectedAuthority = sObjectId;
			}
		}
	}

	if (Button == BUTTON_RIGHT) {
		if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_TAG_CS) {
			/// Set route drawing
			// Make sure flight plan exists otherwise it will crash, and also that they aren't PIV aircraft
			if (CDataHandler::GetFlightData(string(sObjectId))->IsValid && string(sObjectId) != aircraftSel1 && string(sObjectId) != aircraftSel2) {
				int found = -1; // Found flag so we can remove if needed
				for (int i = 0; i < CRoutesHelper::ActiveRoutes.size(); i++) {
					// If the route is currently on the screen
					if (CRoutesHelper::ActiveRoutes[i] == sObjectId) {
						// Set to remove
						found = i;
						break;
					}
				}

				// Erase if the item was found, otherwise add
				if (found != -1) {
					CRoutesHelper::ActiveRoutes.erase(CRoutesHelper::ActiveRoutes.begin() + found);
				}
				else {
					CRoutesHelper::ActiveRoutes.push_back(string(sObjectId));
				}
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

	// Note pad window
	if (ObjectType == WIN_NOTEPAD) {
		npWindow->ButtonDown(atoi(sObjectId));
	}

	// Message window
	if (ObjectType == WIN_MSG) {
		msgWindow->ButtonDown(atoi(sObjectId));
	}

	// Menu bar
	if (ObjectType == MENBAR) {
		menuBar->ButtonDown(atoi(sObjectId));
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Track info window
	if (ObjectType == WIN_TCKINFO) {
		if (atoi(sObjectId) == CTrackInfoWindow::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_TCKINFO, CInputState::INACTIVE);
		}
		trackWindow->ButtonUp(atoi(sObjectId));
	}

	// Flight plan window
	if (ObjectType == WIN_FLTPLN) {
		if (atoi(sObjectId) == CFlightPlanWindow::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_FLIGHTPLAN, CInputState::INACTIVE);
		}
		if (atoi(sObjectId) < 200) {
			fltPlnWindow->ButtonUp(atoi(sObjectId), this);
		}
	}

	// Note pad window
	if (ObjectType == WIN_NOTEPAD) {
		if (atoi(sObjectId) == CNotePad::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_NOTEPAD, CInputState::INACTIVE);
		}
		npWindow->ButtonUp(atoi(sObjectId));
	}

	// Message window
	if (ObjectType == WIN_MSG) {
		if (atoi(sObjectId) == CMessageWindow::BTN_CLOSE) {
			// Close window if the close button
			menuBar->SetButtonState(CMenuBar::BTN_MESSAGE, CInputState::INACTIVE);
		}
		msgWindow->ButtonUp(atoi(sObjectId));
	}

	// Menu bar
	if (ObjectType == MENBAR) {
		// Clear active routes
		if (atoi(sObjectId) == CMenuBar::BTN_RTEDEL) {
			CRoutesHelper::ActiveRoutes.clear();
		}
		menuBar->ButtonDown(atoi(sObjectId));
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
	if (FunctionId == CMenuBar::TXT_SEARCH) {
		string itemString = string(sItemString);
		string value;
		for (int i = 0; i < itemString.size(); i++) {
			char c = itemString[i];
			if (isalpha(c))
				c = toupper(c);
			value += c;
		}
		if (GetPlugIn()->RadarTargetSelect(value.c_str()).IsValid()) {
			CAcTargets::SearchedAircraft = value;
			CAcTargets::fiveSecondTimer = clock();
			GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(value.c_str()));
		}
	}

	// If it is a flight plan window text input
	if (fltPlnWindow->IsTextInput(FunctionId)) {
		fltPlnWindow->SetTextValue(this, FunctionId, string(sItemString));
	}
}

void CRadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// If it is a message
	if (ObjectType == ACTV_MESSAGE) {
		msgWindow->ButtonDoubleClick(this, atoi(sObjectId), fltPlnWindow);
	}
	// If it concerns the flight plan window
	if (ObjectType == WIN_FLTPLN) {
		fltPlnWindow->ButtonDoubleClick(atoi(sObjectId));
	}
	// If it is an aircraft callsign
	if (ObjectType == SCREEN_TAG_CS) {
		//CAcTargets::OpenTrackingDialog = sObjectId;
	}
}

void CRadarDisplay::OnAsrContentToBeSaved(void)
{
	// Buttons
	CUtils::TagsEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_TAGS) ? true : false;
	CUtils::GridEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_GRID) ? true : false;
	CUtils::OverlayEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_OVERLAYS) ? true : false;
	CUtils::QckLookEnabled = menuBar->IsButtonPressed(CMenuBar::BTN_QCKLOOK) ? true : false; // TODO: Change this to Ext
	
	// Save
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

void CRadarDisplay::CursorStateUpdater(void* args) {
	// Pointer to cursor
	CAppCursor* cursor = (CAppCursor*)args;

	// Timer
	clock_t hundredmsTimer = clock();
	clock_t refreshTimer = clock();

	// Get the process information - infinite loop in separate thread = bad unless you manually break the loop on application close
	HANDLE hnd = CUtils::GetESProcess();
	DWORD activeCode;
	GetExitCodeProcess(hnd, &activeCode);
	while (1) {
		// Check if the app is still active
		if (cursor->isESClosed || activeCode != STILL_ACTIVE)
			break;// Break if not

		// Ok so the app is still active let's get the cursor data
		if (((double)(clock() - hundredmsTimer) / ((double)CLOCKS_PER_SEC)) >= 0.04) { // Greater than or equal to 50ms
			// Get cursor position (only if previous cursor position is inside the radar area)
			bool isCursorInsideRadarArea = false;
			CRect radarArea = cursor->screen->GetRadarArea();
			// Get the position
			GetCursorPos(&cursor->position);				

			// Keep the coordinates down to one screen's worth
			if (cursor->position.x > 3840) // Window is on 3rd screen
				cursor->position.x -= 3840;
			else if (cursor->position.x > 1920) // Window is on 2nd screen
				cursor->position.x -= 1920;

			// Check if cursor inside radar area
			if (cursor->position.x > radarArea.left &&
				cursor->position.x < radarArea.right &&
				cursor->position.y > radarArea.top + MENBAR_HEIGHT && // We want *our* radar screen so we add the vNAAATS menu bar height
				cursor->position.y < radarArea.bottom) {
				isCursorInsideRadarArea = true;
				// Get lat lon
				cursor->latLonPosition = cursor->screen->ConvertCoordFromPixelToPosition(cursor->position);
			}

			// Get button presses
			bool leftBtnPressed = (GetAsyncKeyState(VK_LBUTTON) & (1 << 15)) != 0;
			bool rightBtnPressed = (GetAsyncKeyState(VK_RBUTTON) & (1 << 15)) != 0;

			// Check button presses
			if ((!leftBtnPressed && !rightBtnPressed) || !isCursorInsideRadarArea) {
				cursor->button = 0;
			}

			/// Events!
			// On left click
			if (leftBtnPressed && cursor->button == 0) {
				// QDM button
				if (cursor->screen->menuBar->IsButtonPressed(CMenuBar::BTN_QDM) && isCursorInsideRadarArea) {
					bool pointSet = false;
					// Activate QDM, first check if first ruler point already filled
					if (cursor->screen->RulerPoint1.m_Latitude == 0.0 && cursor->screen->RulerPoint1.m_Longitude == 0.0) {
						// It isn't filled so set it
						cursor->screen->RulerPoint1.m_Latitude = cursor->latLonPosition.m_Latitude;
						cursor->screen->RulerPoint1.m_Longitude = cursor->latLonPosition.m_Longitude;

						// So that we don't accidently set the 2nd point at the same time
						pointSet = true;
					}
					// Check the 2nd point
					if (!pointSet && (cursor->screen->RulerPoint2.m_Latitude == 0.0 && cursor->screen->RulerPoint2.m_Longitude == 0.0)) {
						// It isn't filled so set it
						cursor->screen->RulerPoint2.m_Latitude = cursor->latLonPosition.m_Latitude;
						cursor->screen->RulerPoint2.m_Longitude = cursor->latLonPosition.m_Longitude;

						// So we dont cancel the QDM automatically
						pointSet = true;
					}

					// Both points are down so we need to reset
					if (!pointSet && (cursor->screen->RulerPoint2.m_Latitude != 0.0 && cursor->screen->RulerPoint2.m_Longitude != 0.0)) {
						cursor->screen->RulerPoint1.m_Latitude = 0.0;
						cursor->screen->RulerPoint1.m_Longitude = 0.0;
						cursor->screen->RulerPoint2.m_Latitude = 0.0;
						cursor->screen->RulerPoint2.m_Longitude = 0.0;
					}
				}

				// Set the button so the event doesn't fire again
				cursor->button = 2;
			}
			// On right click
			if (rightBtnPressed && cursor->button == 0) {

				// Set the button so the event doesn't fire again
				cursor->button = 1;
			}

			// Reset clock
			hundredmsTimer = clock();
		}

		// Detect double click (probably don't need this and it doesn't work anyway)
		/*if (cursor->singleClickTimer != 0) { // First check if the timer is active
			if (((double)(clock() - cursor->singleClickTimer) / ((double)CLOCKS_PER_SEC)) <= 0.4) { // Check if timer still less than 0.4 seconds
				// Now check the button state
				if (cursor->button == 2 && !cursor->isDoubleClick) {
					cursor->screen->GetPlugIn()->DisplayUserMessage("vNAAATS", "Info", string("Double click").c_str(), true, true, true, true, true);
					cursor->isDoubleClick = true;
				}				
			}
			else {
				// Set timer to 0;
				cursor->singleClickTimer = 0;
				cursor->isDoubleClick = false;
			}			
		}*/
		
		// Call refresh sequence more often
		if (((double)(clock() - refreshTimer) / ((double)CLOCKS_PER_SEC)) >= cursor->screen->RefreshResolution) {
			// Refresh the radar screen
			cursor->screen->RequestRefresh();

			// Reset clock
			refreshTimer = clock();
		}
		
 	}

	// Clean up and return
	delete args;
	return;
}