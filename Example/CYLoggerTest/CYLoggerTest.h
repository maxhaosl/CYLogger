
// CYLoggerTest.h: main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before this file for PCH support"
#endif

#include "resource.h"		// main symbols


// CCYLoggerTestApp:
// See CYLoggerTest.cpp for implementation of this class
//

class CCYLoggerTestApp : public CWinApp
{
public:
	CCYLoggerTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCYLoggerTestApp theApp;
