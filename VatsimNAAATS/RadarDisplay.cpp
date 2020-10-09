#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "MenuBar.h"
#include "InboundList.h"
#include "Constants.h"
#include "PathRenderer.h"
#include "DataHandler.h"
#include "Utils.h"
#include "ConflictDetection.h"
#include <thread> 
#include <gdiplus.h>


using namespace Gdiplus;
using namespace std;
using namespace EuroScopePlugIn;

CRadarDisplay::CRadarDisplay() 
{
	ShowHideGridReference(this, false);
	inboundList = new CInboundList({ CUtils::InboundX, CUtils::InboundY });
	otherList = new COtherList({ CUtils::OthersX, CUtils::OthersY });
	trackWindow = new CTrackInfoWindow({ CUtils::TrackWindowX, CUtils::TrackWindowY });
	fltPlnWindow = new CFlightPlanWindow({ 1000, 200 });
	menuButtons = CMenuBar::BuildButtonData();
	toggleButtons = CMenuBar::BuildToggleButtonData();
	asel = GetPlugIn()->FlightPlanSelectASEL().GetCallsign();
	fiveSecondTimer = clock();
}

CRadarDisplay::~CRadarDisplay() 
{
	delete inboundList;
	delete otherList;
}

void CRadarDisplay::PopulateProgramData() {
	// Lists
	inboundList->MoveList({ CUtils::InboundX, CUtils::InboundY }, true);
	otherList->MoveList({ CUtils::OthersX, CUtils::OthersY });

	// Dropdown values
	CMenuBar::dropDownSelections.find(MENDRP_AREASEL)->second = CMenuBar::ParseDropDownId(CUtils::AreaSelection, MENDRP_AREASEL);
	CMenuBar::dropDownSelections.find(MENDRP_TCKCTRL)->second = string("None");
	CMenuBar::dropDownSelections.find(MENDRP_OVERLAYS)->second = CMenuBar::ParseDropDownId(CUtils::SelectedOverlay, MENDRP_OVERLAYS);
	CMenuBar::dropDownSelections.find(MENDRP_TYPESEL)->second = CMenuBar::ParseDropDownId(CUtils::PosType, MENDRP_TYPESEL);

	// Buttons
	if (CUtils::TagsEnabled && buttonsPressed.find(MENBTN_TAGS) == buttonsPressed.end()) {
		buttonsPressed[MENBTN_TAGS] = true;
	}
	if (CUtils::GridEnabled && buttonsPressed.find(MENBTN_GRID) == buttonsPressed.end()) {
		buttonsPressed[MENBTN_GRID] = true;
	}
	if (CUtils::OverlayEnabled && buttonsPressed.find(MENBTN_OVERLAYS) == buttonsPressed.end()) {
		buttonsPressed[MENBTN_OVERLAYS] = true;
	}
	if (CUtils::QckLookEnabled && buttonsPressed.find(MENBTN_QCKLOOK) == buttonsPressed.end()) {
		buttonsPressed[MENBTN_QCKLOOK] = true;
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
	// download the tracks
	CDataHandler::PopulateLatestTrackData(GetPlugIn());
}

// Show and hide the grid reference and waypoints
void CRadarDisplay::ShowHideGridReference(CRadarScreen* screen, bool show) {
	if (show) {
		screen->GetPlugIn()->SelectScreenSectorfile(screen);
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());
		while (gridName != "CZQO Positional Grid Reference" && gridName != "EGGX Landmark Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos && freetextName.find("EGGX Grid Reference Numbers.") == string::npos) {
			
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
			
		}

		string componentName;
		while (freetextName.find("CZQO Grid Reference Numbers.") != string::npos || freetextName.find("EGGX Grid Reference Numbers.") != string::npos) {
			componentName = freetext.GetComponentName(0);
			screen->ShowSectorFileElement(freetext, componentName.c_str(), true);
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		screen->ShowSectorFileElement(grid, "", true);
	}
	else {
		screen->GetPlugIn()->SelectScreenSectorfile(screen);
		CSectorElement grid(screen->GetPlugIn()->SectorFileElementSelectFirst(13));
		string gridName = string(grid.GetName());
		while (gridName != "CZQO Positional Grid Reference" && gridName != "EGGX Landmark Positional Grid Reference") {
			grid = screen->GetPlugIn()->SectorFileElementSelectNext(grid, 13);
			gridName = string(grid.GetName());
		}

		CSectorElement freetext(screen->GetPlugIn()->SectorFileElementSelectFirst(14));
		string freetextName = string(freetext.GetName());
		while (freetextName.find("CZQO Grid Reference Numbers.") == string::npos && freetextName.find("EGGX Grid Reference Numbers.") == string::npos) {
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		string componentName;
		while (freetextName.find("CZQO Grid Reference Numbers.") != string::npos || freetextName.find("EGGX Grid Reference Numbers.") != string::npos) {
			componentName = freetext.GetComponentName(0);
			screen->ShowSectorFileElement(freetext, componentName.c_str(), false);
			freetext = screen->GetPlugIn()->SectorFileElementSelectNext(freetext, 14);
			freetextName = string(freetext.GetName());
		}

		screen->ShowSectorFileElement(grid, "", false);
	}

	RefreshMapContent();
}

// On radar screen refresh (occurs about once a second)
void CRadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	// 1 second timer
	double fiveSecT = (double)(clock() - fiveSecondTimer) / ((double)CLOCKS_PER_SEC);

	// Clear lists if not empty and time is greater than 1 second
	if (fiveSecT >= 5 && !inboundAircraft.empty()) {
		inboundAircraft.clear();
	}
	if (fiveSecT >= 5 && !otherAircraft.empty()) {
		otherAircraft.clear();
	}

	// Redo the PIV calculations every 5 seconds
	if (fiveSecT >= 5 && buttonsPressed.find(MENBTN_PIV) != buttonsPressed.end()) {
		CConflictDetection::PIVLocations1.clear();
		CConflictDetection::PIVLocations2.clear();
		CConflictDetection::PIVSeparationStatuses.clear();
		CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
	}
	
	// Get the radar area
	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		// Draw overlays if enabled
		if (buttonsPressed.find(MENBTN_OVERLAYS) != buttonsPressed.end()) {
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

				if (fiveSecT >= 5) {
					// If going westbound
					if (!direction && entryMinutes > 0) {
						if (CMenuBar::dropDownSelections[MENDRP_AREASEL] == "CZQX") {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// Find out if 30 west is in their flight plan
								if (rte.GetPointPosition(i).m_Longitude == -30.0) {
									// Test flight time
									if (rte.GetPointDistanceInMinutes(i) > 0 && rte.GetPointDistanceInMinutes(i) < 60) {
										// Add if within
										inboundAircraft.push_back(CListAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(), 
												rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), false));
										break;
									}
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherAircraft.push_back(ac.GetCallsign());
							}
						}
						else {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// They are coming from land so check entry points
								if (CUtils::IsEntryExitPoint(rte.GetPointName(i), direction)) {
									// Add if within
									inboundAircraft.push_back(CListAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
										rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), false));
									break;
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherAircraft.push_back(ac.GetCallsign());
							}
						}
					}
					else if (direction && entryMinutes > 0) {
						if (CMenuBar::dropDownSelections[MENDRP_AREASEL] == "EGGX") {
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								// Find out if 30 west is in their flight plan
								if (rte.GetPointPosition(i).m_Longitude == -30.0) {
									// Test flight time
									if (rte.GetPointDistanceInMinutes(i) > 0 && rte.GetPointDistanceInMinutes(i) < 60) {
										// Add if within
										inboundAircraft.push_back(CListAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
											rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), true));
										break;
									}
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherAircraft.push_back(ac.GetCallsign());
							}
						}
						else {
							// They are coming from land so check entry points
							int i;
							for (i = 0; i < rte.GetPointsNumber(); i++) {
								if (CUtils::IsEntryExitPoint(rte.GetPointName(i), direction)) {
									// Add if within
									inboundAircraft.push_back(CListAircraft(ac.GetCallsign(), fp.GetFinalAltitude(), fp.GetClearedAltitude(),
										rte.GetPointName(i), CUtils::ParseZuluTime(false, -1, &fp, i), fp.GetFlightPlanData().GetDestination(), true));
									break;
								}
							}
							if (i == rte.GetPointsNumber()) {
								// Add to 'others' list
								otherAircraft.push_back(ac.GetCallsign());
							}
						}
					}
				}

				// Store whether detailed tags are enabled
				bool detailedEnabled = false;

				// Now we check if all the tags are selected as detailed				
				if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end()) {
					detailedEnabled = true; // Set detailed on

					// Unpress detailed if not already
					if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end() && !aselDetailed) {
						buttonsPressed.erase(MENBTN_DETAILED);
					}
				}

				// Check if only one is set to detailed
				if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end()) {
					if (fp.GetCallsign() == asel) {
						detailedEnabled = true; // Set detailed on
					}

					// Unpress quick look if not already
					if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end() && aselDetailed) {
						buttonsPressed.erase(MENBTN_QCKLOOK);
					}
				}

				bool ptl = false;
				bool halo = false;
				// Set PTL and HALO if they are on
				if (buttonsPressed.find(MENBTN_PTL) != buttonsPressed.end()) {
					ptl = true;
				}
				if (buttonsPressed.find(MENBTN_HALO) != buttonsPressed.end()) {
					halo = true;
				}

				// RBL draw
				if (buttonsPressed.find(MENBTN_RBL) != buttonsPressed.end()) {
					if (aircraftSel1 != "" && aircraftSel2 != "") {
						CConflictDetection::RBLTool(&dc, &g, this, aircraftSel1, aircraftSel2);
					}
				}

				// SEP draw
				if (buttonsPressed.find(MENBTN_SEP) != buttonsPressed.end()) {
					// If both aircraft selected then draw
					if (aircraftSel1 != "" && aircraftSel2 != "") {
						CConflictDetection::SepTool(&dc, &g, this, aircraftSel1, aircraftSel2);
					}
				}

				// PIV draw
				if (buttonsPressed.find(MENBTN_PIV) != buttonsPressed.end()) {
					// If both aircraft selected then draw
					if (aircraftSel1 != "" && aircraftSel2 != "") {
						// Render
						CPathRenderer::RenderPath(&dc, &g, this, CPathType::PIV);
					}
				}

				// Draw the tag and target with the information if tags are turned on
				if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
					auto kv = tagStatuses.find(fp.GetCallsign());
					kv->second.first = detailedEnabled; // Set detailed on
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, true, &toggleButtons, halo, ptl);
					CAcTargets::DrawTag(&dc, this, &ac, &kv->second, direction);
				}
				else {
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, false, &toggleButtons, halo, ptl);
				}
			}

			// Select the next target
			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}


		// Clear ASELs if none of the range/separation tools are pressed
		if (buttonsPressed.find(MENBTN_PIV) == buttonsPressed.end()
			&& buttonsPressed.find(MENBTN_RBL) == buttonsPressed.end()
			&& buttonsPressed.find(MENBTN_SEP) == buttonsPressed.end()) {
			// Reset ASELs if none enabled
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// Draw menu bar and reset dropdown click
		CMenuBar::DrawMenuBar(&dc, &g, this, { RadarArea.left, RadarArea.top }, &menuButtons, &buttonsPressed, &toggleButtons);
		CMenuBar::dropDownClicked = -1;

		// Check the lists are not empty first, then draw
		inboundList->DrawList(&g, &dc, this, &inboundAircraft);
		otherList->DrawList(&g, &dc, this, &otherAircraft);

		// Draw track info window if button pressed
		if (buttonsPressed.find(MENBTN_TCKINFO) != buttonsPressed.end()) {
			trackWindow->RenderWindow(&dc, &g, this);
		}

		// Draw flight plan window window if button pressed
		if (buttonsPressed.find(MENBTN_FLIGHTPLAN) != buttonsPressed.end()) {
			fltPlnWindow->RenderWindow(&dc, &g, this);
		}
		// Finally, reset the clock if time has been exceeded
		if (fiveSecT >= 5) {
			fiveSecondTimer = clock();
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
		inboundList->MoveList(Area, Released);

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
	if (ObjectType == DROPDOWN) {
		CMenuBar::dropDownHover = atoi(sObjectId);
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Left button actions
	if (Button == BUTTON_LEFT) {
		int dropDownToCancel = -1;
		// If screen object is a dropdown
		if (ObjectType == MENDRP_AREASEL) {
			CMenuBar::dropDownItems.clear();
			dropDownToCancel = CMenuBar::currentDropDownId;
			CMenuBar::currentDropDownId = MENDRP_AREASEL;
			CMenuBar::dropDownHover = -1;
			CMenuBar::dropDownItems[DRP_AREA_EGGX] = "EGGX";
			CMenuBar::dropDownItems[DRP_AREA_CZQX] = "CZQX";
			CMenuBar::dropDownItems[DRP_AREA_BDBX] = "BDBX";
		}
		else if (ObjectType == MENDRP_TCKCTRL) {
			CMenuBar::dropDownItems.clear();
			dropDownToCancel = CMenuBar::currentDropDownId;
			CMenuBar::currentDropDownId = MENDRP_TCKCTRL;
			int idx = 0;
			for (auto kv : COverlays::CurrentTracks) {
				idx++;
				CMenuBar::dropDownItems[800 + idx] = string(string("TCK ") + kv.first);				
			}
			CMenuBar::dropDownHover = -1;
		}
		else if (ObjectType == MENDRP_OVERLAYS) {
			CMenuBar::dropDownItems.clear();
			dropDownToCancel = CMenuBar::currentDropDownId;
			CMenuBar::currentDropDownId = MENDRP_OVERLAYS;
			CMenuBar::dropDownHover = -1;
			CMenuBar::dropDownItems[DRP_OVL_ALL] = "ALL_TCKS";
			CMenuBar::dropDownItems[DRP_OVL_EAST] = "TCKS_EAST";
			CMenuBar::dropDownItems[DRP_OVL_WEST] = "TCKS_WEST";
			//CMenuBar::dropDownItems[DRP_OVL_SEL] = "OCA_TCKSSEL";
		}
		else if (ObjectType == MENDRP_TYPESEL) {
			CMenuBar::dropDownItems.clear();
			dropDownToCancel = CMenuBar::currentDropDownId;
			CMenuBar::currentDropDownId = MENDRP_TYPESEL;
			CMenuBar::dropDownHover = -1;
			CMenuBar::dropDownItems[DRP_TYPE_DEL] = "Delivery";
			CMenuBar::dropDownItems[DRP_TYPE_ENR] = "OCA Enroute";
			CMenuBar::dropDownItems[DRP_TYPE_MULTI] = "Multi-role";
		}

		// If item is a drop down menu item
		bool brk = false;
		for (auto kv : CMenuBar::dropDownItems) {
			if (atoi(sObjectId) == kv.first) {
				CMenuBar::dropDownSelections[CMenuBar::currentDropDownId] = kv.second;
				if (CMenuBar::currentDropDownId == MENDRP_AREASEL) {
					CUtils::AreaSelection = atoi(sObjectId);
				}
				else if (CMenuBar::currentDropDownId == MENDRP_OVERLAYS) {
					CUtils::SelectedOverlay = atoi(sObjectId);
				}
				else if (CMenuBar::currentDropDownId == MENDRP_TYPESEL) {
					CUtils::PosType = atoi(sObjectId);
				}
				// If an overlay
				if (CMenuBar::currentDropDownId == MENDRP_OVERLAYS) {
					if (atoi(sObjectId) == DRP_OVL_ALL) {
						COverlays::CurrentType = COverlayType::TCKS_ALL;
					}
					else if (atoi(sObjectId) == DRP_OVL_EAST) {
						COverlays::CurrentType = COverlayType::TCKS_EAST;
					}
					else if (atoi(sObjectId) == DRP_OVL_WEST) {
						COverlays::CurrentType = COverlayType::TCKS_WEST;
					}
					else if (atoi(sObjectId) == DRP_OVL_SEL) {
						// TODO: implement when ownership is done
					}
				}
				CMenuBar::dropDownClicked = atoi(sObjectId);
				CMenuBar::dropDownHover = -1;
				buttonsPressed.erase(CMenuBar::currentDropDownId);
				CMenuBar::currentDropDownId = -1;
				brk = true;
				break;
			}
		}

		// Clear dropdown items if so
		if (brk) CMenuBar::dropDownItems.clear();

		// If menu button is being unpressed
		if (buttonsPressed.find(ObjectType) != buttonsPressed.end() && string(sObjectId) == "") {
			buttonsPressed.erase(ObjectType);

			if (ObjectType == CMenuBar::currentDropDownId) {
				CMenuBar::currentDropDownId = -1;
				CMenuBar::dropDownHover = -1;
			}

			// Button settings
			if (ObjectType == MENBTN_TAGS) {
				CUtils::TagsEnabled = false;
			}
			else if (ObjectType == MENBTN_GRID) {
				// Grid is off
				CUtils::GridEnabled = false;
				ShowHideGridReference(this, CUtils::GridEnabled);
			}
			else if (ObjectType == MENBTN_OVERLAYS) {
				// Overlays are disabled
				CUtils::OverlayEnabled = false;
			}
			else if (ObjectType == MENBTN_QCKLOOK) {
				// Quick look is off
				CUtils::QckLookEnabled = false;
			}
		}
		else if (menuButtons.find(ObjectType) != menuButtons.end() && string(sObjectId) == "") { // If being pressed
			if (buttonsPressed.find(ObjectType) == buttonsPressed.end()) {
				buttonsPressed[ObjectType] = true;
				
				// Cancel existing open dropdown
				if (dropDownToCancel != -1) {
					buttonsPressed.erase(dropDownToCancel);
				}

				// Button settings
				if (ObjectType == MENBTN_TAGS) {
					CUtils::TagsEnabled = true;
				}
				else if (ObjectType == MENBTN_GRID) {
					// Grid is on
					CUtils::GridEnabled = true;
					ShowHideGridReference(this, CUtils::GridEnabled);
				}
				else if (ObjectType == MENBTN_OVERLAYS) {
					// Overlays are enabled
					CUtils::OverlayEnabled = true;
				}
				else if (ObjectType == MENBTN_QCKLOOK) {
					// Quick look is on
					CUtils::QckLookEnabled = true;
				}
			}
		} 

		// If screen object is a tag
		if (ObjectType == SCREEN_TAG) {
			// Set the ASEL
			asel = sObjectId;
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(sObjectId);
			GetPlugIn()->SetASELAircraft(fp);

			// Probing tools
			if (buttonsPressed.find(MENBTN_RBL) != buttonsPressed.end() 
				|| buttonsPressed.find(MENBTN_SEP) != buttonsPressed.end()
				|| buttonsPressed.find(MENBTN_PIV) != buttonsPressed.end()) {
				if (aircraftSel1 == "") {
					aircraftSel1 = asel;
				}
				else if (aircraftSel2 == "") {
					aircraftSel2 = asel;
				}
			}

			if (fp.GetCallsign() == aircraftSel2 && buttonsPressed.find(MENBTN_PIV) != buttonsPressed.end()) {
				CConflictDetection::PIVTool(this, aircraftSel1, aircraftSel2);
			}
		}

		// Qck Look button
		if (ObjectType == MENBTN_QCKLOOK) {
			aselDetailed = false;
		}

		// Detailed button
		if (ObjectType == MENBTN_DETAILED) {
			aselDetailed = true;
		}

		// If the button is the PIV button
		if (ObjectType == MENBTN_PIV) {
			// Erase RBL (if active)
			if (buttonsPressed.find(MENBTN_RBL) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_RBL);
			}
			// Erase SEP (if active)
			if (buttonsPressed.find(MENBTN_SEP) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_SEP);
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
		if (ObjectType == MENBTN_RBL) {
			// Erase PIV (if active)
			if (buttonsPressed.find(MENBTN_PIV) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_RBL);
			}
			// Erase SEP (if active)
			if (buttonsPressed.find(MENBTN_SEP) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_SEP);
			}
			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
		}

		// If the button is the SEP button
		if (ObjectType == MENBTN_SEP) {
			// Erase RBL (if active)
			if (buttonsPressed.find(MENBTN_RBL) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_RBL);
			}
			// Erase PIV (if active)
			if (buttonsPressed.find(MENBTN_PIV) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_SEP);
			}
			// Reset ASELs
			aircraftSel1 = "";
			aircraftSel2 = "";
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

		if (ObjectType == MENBTN_HALO) {
			// Get the toggle button
			auto cycle = toggleButtons.find(MENBTN_HALO);

			// Increment if less than or equal 3 (20 minute halos max)
			if (cycle->second < 3) {
				cycle->second++;
			}
			else {
				cycle->second = 0;
			}

			// Now assign the values
			auto haloBtn = menuButtons.find(MENBTN_HALO);
			switch (cycle->second) {
			case 0:
				haloBtn->second = "Halo 5";
				break;
			case 1:
				haloBtn->second = "Halo 10";
				break;
			case 2:
				haloBtn->second = "Halo 15";
				break;
			case 3:
				haloBtn->second = "Halo 20";
				break;
			}
		}

		if (ObjectType == MENBTN_PTL) {
			// Get the toggle button
			auto cycle = toggleButtons.find(MENBTN_PTL);

			// Increment if less than or equal 5 (30 minute lines max)
			if (cycle->second < 5) {
				cycle->second++;
			}
			else {
				cycle->second = 0;
			}

			// Now assign the values
			auto ptlBtn = menuButtons.find(MENBTN_PTL);
			switch (cycle->second) {
			case 0:
				ptlBtn->second = "PTL 5";
				break;
			case 1:
				ptlBtn->second = "PTL 10";
				break;
			case 2:
				ptlBtn->second = "PTL 15";
				break;
			case 3:
				ptlBtn->second = "PTL 20";
				break;
			case 4:
				ptlBtn->second = "PTL 25";
				break;
			case 5:
				ptlBtn->second = "PTL 30";
				break;
			}
		}

		if (ObjectType == MENBTN_RINGS) {
			// Get the toggle button
			auto cycle = toggleButtons.find(MENBTN_RINGS);

			// Increment if less than or equal 4 (5 rings max)
			if (cycle->second < 4) {
				cycle->second++;
			}
			else {
				cycle->second = 0;
			}

			// Now assign the values
			auto ringsBtn = menuButtons.find(MENBTN_RINGS);
			switch (cycle->second) {
			case 0:
				ringsBtn->second = "Rings 1";
				break;
			case 1:
				ringsBtn->second = "Rings 2";
				break;
			case 2:
				ringsBtn->second = "Rings 3";
				break;
			case 3:
				ringsBtn->second = "Rings 4";
				break;
			case 4:
				ringsBtn->second = "Rings 5";
				break;
			}
		}
	}
	
	RequestRefresh();
}

void CRadarDisplay::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Shared close button for windows
	if (ObjectType == WINBTN_CLOSE) {
		if (string(sObjectId) == "TCKINFO") {
			// Press button on screen
			trackWindow->WindowButtons.find(WINBTN_CLOSE)->second.second = true;
		}
	}

	// Track info window refresh button
	if (string(sObjectId) == "TCKINFO") {
		if (ObjectType == WINBTN_TCKINFO_REFRESH) {
			// Press button on screen
			trackWindow->WindowButtons.find(WINBTN_TCKINFO_REFRESH)->second.second = true;
		}
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// Shared close button for windows
	if (ObjectType == WINBTN_CLOSE) {
		if (string(sObjectId) == "TCKINFO") {
			// Unpress button and reset window state
			trackWindow->WindowButtons.find(WINBTN_CLOSE)->second.second = false;
			trackWindow->MsgDataRefresh = "";
			// Unpress track info window button
			if (buttonsPressed.find(MENBTN_TCKINFO) != buttonsPressed.end()) {
				buttonsPressed.erase(MENBTN_TCKINFO);
			}
		}
	}

	// Track info window refresh button
	if (string(sObjectId) == "TCKINFO") {
		if (ObjectType == WINBTN_TCKINFO_REFRESH) {
			// Unpress button on screen
			trackWindow->NATDataRefresh = true;
			trackWindow->WindowButtons.find(WINBTN_TCKINFO_REFRESH)->second.second = false;
		}
	}

	// Refresh
	RequestRefresh();
}

void CRadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{

}

void CRadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

}

void CRadarDisplay::OnAsrContentToBeSaved(void)
{
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