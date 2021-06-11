#include "pch.h"
#include "framework.h"
#include "VatsimNAAATS.h"
#include "EuroScopePlugIn.h"
#include "NAAATS.h"
#include <gdiplus.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

// Token for gdiplus
ULONG_PTR m_gdiplusToken = 0;

BEGIN_MESSAGE_MAP(CVatsimNAAATSApp, CWinApp)
END_MESSAGE_MAP()

CVatsimNAAATSApp::CVatsimNAAATSApp() {}

CVatsimNAAATSApp theApp;
EuroScopePlugIn::CPlugIn* pNAAATS = nullptr;

BOOL CVatsimNAAATSApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

// Plugin Initialisation
void __declspec (dllexport)
EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);
	* ppPlugInInstance = (CPlugIn*)new CNAAATSPlugin();

	// Instantiate logger as the very first thing we do
	CLogger::InstantiateLogFile();
}

// Plugin exit
void __declspec (dllexport)
EuroScopePlugInExit(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	GdiplusShutdown(m_gdiplusToken);
	delete pNAAATS;
}