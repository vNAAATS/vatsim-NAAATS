#include "pch.h"
#include "PathRenderer.h"

CRadarTarget CPathRenderer::PivTarget;

string CPathRenderer::RouteDrawTarget;

vector<CRoutePosition> CPathRenderer::RouteToDraw;

void CPathRenderer::RenderPath(CDC* dc, Graphics* g, CRadarScreen* screen, CPathType type) {

	// Save context
	int iDC = dc->SaveDC();

	// Pen & brush
	CPen pen(PS_SOLID, 1, TargetOrange.ToCOLORREF());
	dc->SelectObject(pen);
	SolidBrush brush(TargetOrange);

	// Font
	FontSelector::SelectMonoFont(12, dc);
	dc->SetTextColor(TargetOrange.ToCOLORREF());
	dc->SetTextAlign(TA_LEFT);

	// Draw route if route draw selected
	if (type == CPathType::RTE) {
		// Get AC position and move to
		POINT acPosition = screen->ConvertCoordFromPositionToPixel(screen->GetPlugIn()->RadarTargetSelect(RouteDrawTarget.c_str()).GetPosition().GetPosition());
		dc->MoveTo(acPosition);
		if (!RouteToDraw.empty()) {// Failsafe
			for (auto i = RouteToDraw.begin(); i != RouteToDraw.end(); i++) {
				// Get point, text rectangle & define y offset
				string text = i->Fix; // To get TextExtent
				POINT point = screen->ConvertCoordFromPositionToPixel(i->PositionRaw);
				CRect box(point.x - (dc->GetTextExtent(text.c_str()).cx / 2), point.y + 10, point.x + (dc->GetTextExtent(text.c_str()).cx), point.y + 50);
				int offsetY = 0;

				// Draw dot
				Rect pointRect(point.x - 3, point.y - 3, 6, 6);
				g->FillEllipse(&brush, pointRect);

				// Print text for fix
				dc->TextOutA(box.left, box.top, text.c_str());
				offsetY += 14;

				// Print text for estimate
				text = i->Estimate;
				dc->TextOutA(box.left, box.top + offsetY, text.c_str());
				offsetY += 14;

				// Print text for flight level
				text = to_string(i->FlightLevel);
				dc->TextOutA(box.left, box.top + offsetY, text.c_str());

				// Draw line
				dc->LineTo(point);
			}
		}
	}
	// Cleanup
	DeleteObject(pen);
	DeleteObject(&brush);

	// Restore context
	dc->RestoreDC(iDC);
}

void CPathRenderer::GetRoute(CRadarScreen* screen, string callsign) {
	// Set callsign and get radar target
	RouteDrawTarget = callsign;
	CRadarTarget target = screen->GetPlugIn()->RadarTargetSelect(RouteDrawTarget.c_str());

	// Route and flight plan
	// TODO: get NAT track route and interpolate it
	CFlightPlan fpData = screen->GetPlugIn()->FlightPlanSelect(target.GetCallsign());
	CFlightPlanExtractedRoute route = fpData.GetExtractedRoute();

	// Vector to store the final route
	vector<CRoutePosition> returnRoute;

	// Get the index of the nearest point and the exit point
	int nearestPoint = route.GetPointsCalculatedIndex();

	// Get the points in the route
	for (int i = nearestPoint; i < route.GetPointsNumber(); i++) {
		string pointName = route.GetPointName(i); // Debug
		// Aircraft direction
		bool direction = CUtils::GetAircraftDirection(target.GetTrackHeading());

		// Store whether to cancel route draw
		bool breakLoop = false;
		if (CUtils::IsEntryExitPoint(string(route.GetPointName(i)), direction ? false : true)) {
			breakLoop = true;
		}

		// Create position
		CRoutePosition position;
		position.Fix = string(route.GetPointName(i));
		position.PositionRaw = route.GetPointPosition(i);
		position.Estimate = CUtils::ParseZuluTime(false, &fpData, i);
		position.FlightLevel = route.GetPointCalculatedProfileAltitude(i) / 100;

		// Add the position
		returnRoute.push_back(position);

		// Break the loop if end of route
		if (breakLoop) break;
	}

	// Set route draw
	RouteToDraw = returnRoute;
}

int CPathRenderer::ClearCurrentRoute() {
	// Test if route is already empty
	if (!RouteToDraw.empty()) {
		// Clear it and return success code
		RouteToDraw.clear();
		RouteDrawTarget = "";
		return 0;
	}
	else {
		// Vector was already empty so return fail code
		return 1;
	}
}