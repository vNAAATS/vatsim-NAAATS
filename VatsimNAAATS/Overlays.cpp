#include "pch.h"
#include "Overlays.h"

COverlayType COverlays::CurrentType = COverlayType::TCKS_ALL;

map<string, CTrack> COverlays::CurrentTracks;

void COverlays::ShowCurrentOverlay(CDC* dc, Graphics* g, CRadarScreen* screen) {
	// Render the tracks path
	CPathRenderer::RenderPath(dc, g, screen, CPathType::TCKS);
}