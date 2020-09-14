#pragma once
#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif
#include "resource.h"
#include <EuroScopePlugIn.h>


class CVatsimNAAATSApp : public CWinApp
{
public:
	CVatsimNAAATSApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

