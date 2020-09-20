#pragma once
#include "pch.h"
#include "RadarDisplay.h"
#include "AcTargets.h"
#include "MenuBar.h"
#include "InboundList.h"
#include "Constants.h"
#include <gdiplus.h>


using namespace Gdiplus;

RadarDisplay::RadarDisplay() 
{
	inboundList = new CInboundList({ 100, 150 });
	menuButtons = MenuBar::BuildButtonData();
	buttonsPressed.insert(make_pair(MENBTN_TAGS, true));
}

RadarDisplay::~RadarDisplay() 
{

}

// On radar screen refresh (occurs about once a second)
void RadarDisplay::OnRefresh(HDC hDC, int Phase)
{
	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	/// TIMERS FOR BUTTONS
	double t = (double)(clock() - buttonClickTimer) / ((double)CLOCKS_PER_SEC);
	if (t >= 0.1) {
		/*if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end() && clickType == true) {
			buttonsPressed.erase(MENBTN_QCKLOOK);
			buttonClickTimer = clock();
		}*/
	}

	// Clean up old list
	inboundAircraft.clear();

	// Get the radar area
	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		// Get first aircraft
		CRadarTarget ac;
		ac = GetPlugIn()->RadarTargetSelectFirst();

		// Get entry time and heading
		int entryMinutes;
		int hdg;

		// List of entry points
		vector<pair<string, int>> epVec;

		// Loop all aircraft
		while (ac.IsValid()) {
			// Flight plan
			CFlightPlan fp = GetPlugIn()->FlightPlanSelect(ac.GetCallsign());

			// Route
			CFlightPlanExtractedRoute rte = fp.GetExtractedRoute();

			// Time and heading
			entryMinutes = fp.GetSectorEntryMinutes();
			hdg = ac.GetPosition().GetReportedHeading();

			// Draw the aircraft if already in airspace
			if (entryMinutes == 0) {
				if (tagStatuses.find(fp.GetCallsign()) == tagStatuses.end()) {
					// Insert point (first time appearance)
					pair<bool, POINT> pt = make_pair(false, POINT{ 0, 0 });
					tagStatuses.insert(make_pair(string(fp.GetCallsign()), pt));

					// Draw target and tag
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg);
					// If tags enabled
					if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
						// Check if detailed
						if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end()) { // All detailed
							auto kv = tagStatuses.find(fp.GetCallsign());
							kv->second.first = true; // Set detailed on
							CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
						}
						else {
							auto kv = tagStatuses.find(fp.GetCallsign());
							kv->second.first = false; // Set detailed off
							CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
						}
					}
				}
				else {
					// Draw target and tag
					CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg);
					// If tags enabled
					if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
						if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end()) { // All detailed
							auto kv = tagStatuses.find(fp.GetCallsign());
							kv->second.first = true; // Set detailed on
							CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
						}
						else {
							auto kv = tagStatuses.find(fp.GetCallsign());
							kv->second.first = false; // Set detailed off
							CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
						}
					}
				}
			}
			else if (entryMinutes > 0) {
				// If inbound 
				if (fp.GetSectorEntryMinutes() > 0 && fp.GetSectorEntryMinutes() <= 90) {
					if (tagStatuses.find(fp.GetCallsign()) == tagStatuses.end()) {
						// Create blank point
						pair<bool, POINT> pt = make_pair(false, POINT{ 0, 0 });
						tagStatuses.insert(make_pair(string(fp.GetCallsign()), pt));

						// Draw target and tag
						CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg);
						// If tags enabled
						if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
							// Check if all detailed
							if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end()) { // All detailed
								auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = true; // Set detailed on
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
								// Unpress detailed if it is there
								if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end() && !aselDetailed) {
									buttonsPressed.erase(MENBTN_DETAILED);
								}
							}
							else {
								auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = false; // Set detailed off
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
							}

							if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end()) { // Check if ASEL detailed
								/*auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = true; // Set detailed on
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);*/
								// Unpress detailed if it is there
								if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end() && aselDetailed) {
									buttonsPressed.erase(MENBTN_QCKLOOK);
								}
							}
							else {
								auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = false; // Set detailed off
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
							}
						}
					}
					else {
						// Draw target and tag
						CAcTargets::DrawAirplane(&g, &dc, this, &ac, hdg);
						// If tags enabled
						if (buttonsPressed.find(MENBTN_TAGS) != buttonsPressed.end()) {
							// Check if all detailed
							if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end()) { // All detailed
								auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = true; // Set detailed on
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
								// Unpress detailed if it is there
								if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end() && !aselDetailed) {
									buttonsPressed.erase(MENBTN_DETAILED);
								}
							}
							else {
								auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = false; // Set detailed off
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);
							}


							// Check if ASEL detailed
							if (buttonsPressed.find(MENBTN_DETAILED) != buttonsPressed.end()) { // All detailed
								/*auto kv = tagStatuses.find(fp.GetCallsign());
								kv->second.first = true; // Set detailed on
								CAcTargets::DrawTag(&dc, this, &ac, &kv->second);*/
								// Unpress detailed if it is there
								if (buttonsPressed.find(MENBTN_QCKLOOK) != buttonsPressed.end() && aselDetailed) {
									buttonsPressed.erase(MENBTN_QCKLOOK);
								}
							}
						}
					}

					if ((hdg <= 359) && (hdg >= 181)) {
						// Shanwick
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							if (std::find(pointsShanwick.begin(), pointsShanwick.end(), rte.GetPointName(i)) != pointsShanwick.end()) {
								inboundAircraft.push_back(make_pair(ac, false));
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								break;
							}
						}		
					}
					else if ((hdg >= 1) && (hdg <= 179)) {
						// Gander
						for (int i = 0; i < rte.GetPointsNumber(); i++) {
							if (std::find(pointsGander.begin(), pointsGander.end(), rte.GetPointName(i)) != pointsGander.end()) {
								inboundAircraft.push_back(make_pair(ac, true));
								epVec.push_back(make_pair(rte.GetPointName(i), i));
								break;
							}
						}
					}
				}
			}
			ac = GetPlugIn()->RadarTargetSelectNext(ac);
		}
		// Draw menu bar
		MenuBar::DrawMenuBar(&dc, &g, this, { RadarArea.left, RadarArea.top }, &menuButtons, &buttonsPressed);
		// Inbound list
		inboundList->DrawList(&g, &dc, this, &inboundAircraft, &epVec);
	}
	
	if (Phase == REFRESH_PHASE_AFTER_LISTS) {
		
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
}

void RadarDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{
	mousePointer = Pt;
	if (ObjectType == LIST_INBOUND) {
		inboundList->MoveList(Area, Released);
	}

	if (ObjectType == SCREEN_TAG) {
		auto kv = tagStatuses.find(sObjectId);
		POINT acPosPix = ConvertCoordFromPositionToPixel(GetPlugIn()->RadarTargetSelect(sObjectId).GetPosition().GetPosition());
		kv->second.second = { Area.left - acPosPix.x, Area.top - acPosPix.y };
	}

	RequestRefresh();
}

void RadarDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area) 
{

}

void RadarDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{
	// If menu button is being unpressed
	if (buttonsPressed.find(ObjectType) != buttonsPressed.end()) {
		buttonsPressed.erase(ObjectType);
	} else if (menuButtons.find(ObjectType) != menuButtons.end()) { // If being pressed
		if (buttonsPressed.find(ObjectType) == buttonsPressed.end()) {
			buttonClickTimer = clock();
			buttonsPressed[ObjectType] = true;
		}
	}

	// If screen object is a tag
	if (ObjectType == SCREEN_TAG) {
		// Set the ASEL
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
	}

	// Qck Look button
	if (ObjectType == MENBTN_QCKLOOK) {
		aselDetailed = false;
	}

	// Detailed button
	if (ObjectType == MENBTN_DETAILED) {
		aselDetailed = true;
	}
	
	RequestRefresh();
}

void RadarDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{ 

}

void RadarDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

}

void RadarDisplay::OnAsrContentToBeSaved(void)
{

}

void RadarDisplay::OnAsrContentLoaded(bool Loaded)
{

}

void RadarDisplay::OnAsrContentToBeClosed(void)
{

}