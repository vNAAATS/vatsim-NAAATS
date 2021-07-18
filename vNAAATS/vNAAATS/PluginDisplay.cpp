#include "pch.h"
#include "PluginDisplay.h"
#include <gdiplus.h>
#include <ctype.h>
#include <iostream>

using namespace std;
using namespace Gdiplus;
using namespace User_Interface;

CPluginDisplay::CPluginDisplay()
{
	// Initialise fonts
	Typefaces::CTextManager::InitialiseFonts();

	// Screen objects
	menuBar = new CMenuBar(User_Interface::kMenuBarId, User_Interface::kScreen);
};

CPluginDisplay::~CPluginDisplay()
{

}

void CPluginDisplay::OnRefresh(HDC hDC, int Phase)
{
	// Create device context
	CDC dc;
	dc.Attach(hDC);

	// Graphics object
	Graphics g(hDC);

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
		// UI component rendering
		RenderWorker(&dc, &g, this);
	}

	// De-allocation
	dc.Detach();
	g.ReleaseHDC(hDC);
	dc.DeleteDC();
};

void CPluginDisplay::RenderWorker(CDC* dc, Graphics* g, CRadarScreen* screen)
{
	// MenuBar
	menuBar->RenderContent(dc, g, screen);
};

void CPluginDisplay::OnMoveScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, bool Released)
{

};

void CPluginDisplay::OnOverScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area)
{

};

void CPluginDisplay::OnClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

};

void CPluginDisplay::OnButtonDownScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

};

void CPluginDisplay::OnButtonUpScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

};

void CPluginDisplay::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{

};

void CPluginDisplay::OnDoubleClickScreenObject(int ObjectType, const char* sObjectId, POINT Pt, RECT Area, int Button)
{

};

void CPluginDisplay::OnAsrContentToBeSaved(void)
{

};

void CPluginDisplay::OnAsrContentLoaded(bool Loaded)
{

};