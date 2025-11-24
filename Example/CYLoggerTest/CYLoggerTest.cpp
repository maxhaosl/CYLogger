
// CYLoggerTest.cpp: defines the application's class behavior.
//

#include "pch.h"
#include "framework.h"
#include "CYLoggerTest.h"
#include "CYLoggerTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCYLoggerTestApp

BEGIN_MESSAGE_MAP(CCYLoggerTestApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCYLoggerTestApp construction

CCYLoggerTestApp::CCYLoggerTestApp()
{
	// Support restart manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// place all significant initialization in InitInstance
}


// The one and only CCYLoggerTestApp object

CCYLoggerTestApp theApp;


// CCYLoggerTestApp initialization

BOOL CCYLoggerTestApp::InitInstance()
{
	// If an application manifest specifies ComCtl32.dll version 6 or later to enable
	// visual styles on Windows XP, InitCommonControlsEx() is required; otherwise window
	// creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set it to include all control classes you plan to use in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager in case the dialog contains any shell tree view
	// or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate the “Windows Native” visual manager so MFC controls use themes
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and want to reduce the size of the final
	// executable, remove the following unnecessary initialization routines.
	// Change the registry key for storing settings.
	// TODO: Modify this string appropriately, for example with a company or organization name.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CCYLoggerTestDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  closed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  closed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, application will terminate unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you use MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has closed, return FALSE so the application exits
	//  instead of starting the message pump.
	return FALSE;
}

