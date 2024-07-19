////////////////////
// ARO_Ap.cpp - implementation file for the core of the program, creates MainDialog
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"
#include "ARO_App.h"

// Handles F1 Help Menu
BEGIN_MESSAGE_MAP(ARO_AppApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// Singleton design app reference
ARO_AppApp theApp;

// [CONSTRUCTOR(S)]
ARO_AppApp::ARO_AppApp() {}

// [INITIALIZER]
BOOL ARO_AppApp::InitInstance() {
	CWinApp::InitInstance();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard Initialization:
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// such as the name of your company or organization
	SetRegistryKey(_T("ARO Application"));

	// Create the main dialog (wait for responce)
	MainDialog dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	if (nResponse == -1) {
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
		delete pShellManager;

	#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
		ControlBarCleanUp();
	#endif

	// Exit application rather than entering message pump
	return FALSE;
}

