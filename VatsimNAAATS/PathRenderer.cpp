#include "pch.h"
#include "PathRenderer.h"

CRadarTarget CPathRenderer::pivTarget;

bool CPathRenderer::RouteDrawASEL = false;

void CPathRenderer::RenderRoutePath(CDC* dc, CRadarScreen* screen, CRadarTarget* asel) {

	// Route and flight plan
	CFlightPlan fpData = screen->GetPlugIn()->FlightPlanSelect(asel->GetCallsign());
	CFlightPlanExtractedRoute route = fpData.GetExtractedRoute();

	// Save context
	int iDC = dc->SaveDC();

	// Pen
	CPen pen(PS_SOLID, 1, TargetOrange.ToCOLORREF());
	dc->SelectObject(pen);

	bool direction = false;
	// If coming from Gander
	if ((asel->GetTrackHeading() >= 1) && (asel->GetTrackHeading() <= 179)) {
		direction = true;
	}

	// Loop through to find the waypoints
	bool beginRouteDraw = false;
	for (int i = 0; i < route.GetPointsNumber(); i++) {
		string name = route.GetPointName(i);
		if (direction) {
			// Find the points
			if (std::find(pointsGander.begin(), pointsGander.end(), name) != pointsGander.end()) {
				if (beginRouteDraw) {
					beginRouteDraw = false;
				}
				else {
					beginRouteDraw = true;
				}
			}
			else {
				if (beginRouteDraw) {
					dc->MoveTo(screen->ConvertCoordFromPositionToPixel(route.GetPointPosition(i - 1)));
					dc->LineTo(screen->ConvertCoordFromPositionToPixel(route.GetPointPosition(i)));
				}
			}
			// Find the points
			if (std::find(pointsShanwick.begin(), pointsShanwick.end(), name) != pointsShanwick.end()) {
				if (beginRouteDraw) {
					beginRouteDraw = false;
				}
				else {
					beginRouteDraw = true;
				}
			}
			else {
				if (beginRouteDraw) {
					dc->MoveTo(screen->ConvertCoordFromPositionToPixel(route.GetPointPosition(i)));
					dc->LineTo(screen->ConvertCoordFromPositionToPixel(route.GetPointPosition(i + 1)));
				}
			}
		}
	}

	// Cleanup
	DeleteObject(pen);

	// Restore context
	dc->RestoreDC(iDC);
}