#include "pch.h"
#include "Styles.h"
#include "AcTargets.h"

using namespace Colours;

void AcTargets::DrawAirplane(Graphics* g, CDC* dc, CRadarScreen* screen, CRadarTarget target, int hdg) {
	// Get the aircraft's position and heading
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target.GetPosition().GetPosition());

	// Save context for later
	int sDC = dc->SaveDC();

	// Define a brush and a container for the target
	SolidBrush brush(Colours::TargetOrange);
	GraphicsContainer gContainer;	

	// Begin drawing
	gContainer = g->BeginContainer();
	
	CFont* lineFont = FontSelector::ATCFont(MEN_FONT_SIZE);
	dc->SelectObject(lineFont);
	dc->SetTextColor(TargetOrange.ToCOLORREF());
	dc->SetTextAlign(TA_CENTER);
	string line(target.GetCallsign());

	dc->TextOutA(acPoint.x, acPoint.y - 230, line.c_str());

	// Rotate the graphics object and set the middle to the aircraft position
	g->RotateTransform(hdg);
	g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);

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

	// Fill the polygon and finish
	g->FillPolygon(&brush, points, 19);
	g->EndContainer(gContainer);

	// Restore context
	dc->RestoreDC(sDC);

	// Deallocate
	DeleteObject(&brush);
	DeleteObject(&points);
	DeleteObject(&gContainer);
	DeleteObject(&acPoint);
	DeleteObject(&lineFont);
}