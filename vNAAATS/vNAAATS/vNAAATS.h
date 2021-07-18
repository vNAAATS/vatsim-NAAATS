#pragma once
#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif
#include "resource.h"		// main symbols
#include <EuroScopePlugIn.h>

class CVNAAATSApp : public CWinApp
{
	public:
		CVNAAATSApp();

	// Overrides
	public:
		virtual BOOL InitInstance();

		DECLARE_MESSAGE_MAP()
};
