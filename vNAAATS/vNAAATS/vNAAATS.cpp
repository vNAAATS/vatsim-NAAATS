#include "pch.h"
#include "framework.h"
#include "vNAAATS.h"
#include "VNAAATSPlugin.h"
#include <gdiplus.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Gdiplus;

// Token for gdiplus
ULONG_PTR m_gdiplusToken = 0;

BEGIN_MESSAGE_MAP(CVNAAATSApp, CWinApp)
END_MESSAGE_MAP()

CVNAAATSApp::CVNAAATSApp() { }

CVNAAATSApp theApp;
EuroScopePlugIn::CPlugIn* pNAAATS = nullptr;

BOOL CVNAAATSApp::InitInstance() {
	CWinApp::InitInstance();

	return TRUE;
}

// Plugin Initialisation
void __declspec (dllexport)
EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance) {
	// Initialise module
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

	// Instantiate the plugin
	*ppPlugInInstance = (CPlugIn*)new CVNAAATSPlugin();
}

// Plugin exit
void __declspec (dllexport)
EuroScopePlugInExit(void) {
	// Shutdown module and GDI+
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		GdiplusShutdown(m_gdiplusToken);

	// And poof, gone
	delete pNAAATS;
}
