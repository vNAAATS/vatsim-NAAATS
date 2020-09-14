#include "pch.h"
#include "AcTargets.h"

void AcTargets::DrawAirplane(Graphics* g, CRadarScreen* screen, CRadarTarget target) {
	POINT acPoint = screen->ConvertCoordFromPositionToPixel(target.GetPosition().GetPosition());

	SolidBrush brush(Color(255, 125, 0));
	GraphicsContainer gContainer;
	double hdg = (double)target.GetPosition().GetReportedHeading();

	gContainer = g->BeginContainer();
	g->RotateTransform(hdg);
	g->TranslateTransform(acPoint.x, acPoint.y, MatrixOrderAppend);
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
	g->FillPolygon(&brush, points, 19);
	g->EndContainer(gContainer);
}