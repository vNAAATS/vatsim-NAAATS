// VatsimNAAATS.h : main header file for the VatsimNAAATS DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVatsimNAAATSApp
// See VatsimNAAATS.cpp for the implementation of this class
//

class CVatsimNAAATSApp : public CWinApp
{
public:
	CVatsimNAAATSApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
