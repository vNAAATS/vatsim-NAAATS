#include "pch.h"
#include "Styles.h"
#include "AcTargets.h"

void AcTargets::DrawAirplane(Graphics* g, CRadarScreen* screen, CRadarTarget target) {
	// Get the aircraft's position and heading
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target.GetPosition().GetPosition());
	double hdg = (double)target.GetPosition().GetReportedHeading();

	// Define a brush and a container for the target
	SolidBrush brush(Colours::TargetOrange);
	GraphicsContainer gContainer;	

	// Begin drawing
	gContainer = g->BeginContainer();
	
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
}