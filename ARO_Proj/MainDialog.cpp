// [DESCRIPTION]
// MainDialog.cpp: implementation file for the main dialog of the program
// Authors: Benjamin Richardson, Rebecca Tucker, Kostiantyn Makrasnov, Andrew O'Kins, and More @ ISP ASL

// [DEFINITIONS/ABRIVIATIONS]
// MFC - Microsoft Foundation Class library - Used to design UI in C++
// SLM - Spatial Light Modulator - the device that contains parameters which we want to optimize (takes in image an reflects light based on that image)
// LC  - Liquid Crystal - the type of diplay the SLM has

// [INCUDE FILES]
#include "stdafx.h"				// Required in source
#include "resource.h"
#include "MainDialog.h"			// Header file for dialog functions
#include <iostream>				// for cout.clear()

// - Aglogrithm Related
#include "uGA_Optimization.h"
#include "SGA_Optimization.h"
#include "BruteForce_Optimization.h"

//	- Helper
#include "Utility.h"			// Collection of static helper functions
#include "SLMController.h"		// Wrapper for SLM control
#include "CameraController.h"	// Camera interface wrapper

#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

// [CONSTRUCTOR/COMPONENT EVENTS]
// Constructor for dialog
MainDialog::MainDialog(CWnd* pParent) : CDialog(IDD_AROMAIN_DIALOG, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// Reference UI components
void MainDialog::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UGA_BUTTON, m_uGAButton);
	DDX_Control(pDX, IDC_SGA_BUTTON, m_SGAButton);
	DDX_Control(pDX, IDC_OPT_BUTTON, m_OptButton);
	DDX_Control(pDX, IDC_START_STOP_BUTTON, m_StartStopButton);
	DDX_Control(pDX, IDC_MULTITHREAD_ENABLE, m_MultiThreadEnable);
	DDX_Control(pDX, IDC_TAB1, m_TabControl);
}

//Set functions that respond to user action
BEGIN_MESSAGE_MAP(MainDialog, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_UGA_BUTTON, &MainDialog::OnBnClickedUgaButton)
	ON_BN_CLICKED(IDC_SGA_BUTTON, &MainDialog::OnBnClickedSgaButton)
	ON_BN_CLICKED(IDC_OPT_BUTTON, &MainDialog::OnBnClickedOptButton)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &MainDialog::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_START_STOP_BUTTON, &MainDialog::OnBnClickedStartStopButton)
	ON_BN_CLICKED(IDC_LOAD_SETTINGS, &MainDialog::OnBnClickedLoadSettings)
	ON_BN_CLICKED(IDC_SAVE_SETTINGS, &MainDialog::OnBnClickedSaveSettings)
	ON_BN_CLICKED(IDC_MULTITHREAD_ENABLE, &MainDialog::OnBnClickedMultiThreadEnable)
	ON_BN_CLICKED(IDC_ABOUT_BUTTON, &MainDialog::OnBnClickedAboutButton)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////
//
//   OnInitDialog()
//
//   Inputs: none
//
//   Returns: true or false depending on if an error was encountered
//
//   Purpose: This is called when the dialog is first constructed. This is a good place to 
//			  initialize variables for the program. First build the board class and set the
//			  run parameters. Then pre-load images to the hardware.
//
///////////////////////////////////////////////////////
BOOL MainDialog::OnInitDialog() {
	//[CONSOLE OUTPUT]
	if (!AllocConsole()) {
		AfxMessageBox(L"Console will not be shown!");
	}
	freopen_s(&fp, "CONOUT$", "w", stdout);
	std::cout.clear();

	//[UI SETUP]
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);

	//Setup the tab control that hosts all of the app's settings
	//1) https://stackoverflow.com/questions/1044315/setwindowpos-function-not-moving-window
	//2) https://www.youtube.com/watch?v=WHPNzx4E5rM
	// - set tab headings
	LPCTSTR headings[] = { L"GA Settings", L"PSA Settings", L"SLM Settings", L"Camera Settings", L"AOI Settings", L"Output Settings" };
	for (int i = 0; i < 6; i++) {
		m_TabControl.InsertItem(i, headings[i]);
	}
	// - set all tab components (Create Dialogs from Templates)
	CRect rect;
	m_TabControl.GetClientRect(&rect); //Gets dimensions of the tab control to fit in dialogs inside

	// Creating windows to be for the various settings under the tab options
	m_ga_ControlDlg.Create(IDD_GA_CONTROL, &m_TabControl);
	m_ga_ControlDlg.SetWindowPos(NULL, rect.top + 5, rect.left + 30, rect.Width() - 10, rect.Height() - 35, SWP_HIDEWINDOW | SWP_NOZORDER);

	m_ia_ControlDlg.Create(IDD_IA_CONTROL, &m_TabControl);
	m_ia_ControlDlg.SetWindowPos(NULL, rect.top + 5, rect.left + 30, rect.Width() - 10, rect.Height() - 35, SWP_HIDEWINDOW | SWP_NOZORDER);

	m_slmControlDlg.Create(IDD_SLM_CONTROL, &m_TabControl);
	m_slmControlDlg.SetWindowPos(NULL, rect.top + 5, rect.left + 30, rect.Width() - 10, rect.Height() - 35, SWP_HIDEWINDOW | SWP_NOZORDER);

	m_cameraControlDlg.Create(IDD_CAMERA_CONTROL, &m_TabControl);
	m_cameraControlDlg.SetWindowPos(NULL, rect.top + 5, rect.left + 30, rect.Width() - 10, rect.Height() - 35, SWP_HIDEWINDOW | SWP_NOZORDER);

	m_aoiControlDlg.Create(IDD_AOI_CONTROL, &m_TabControl);
	m_aoiControlDlg.SetWindowPos(NULL, rect.top + 5, rect.left + 30, rect.Width() - 10, rect.Height() - 35, SWP_HIDEWINDOW | SWP_NOZORDER);

	m_outputControlDlg.Create(IDD_OUTPUT_CONTROL, &m_TabControl);
	m_outputControlDlg.SetWindowPos(NULL, rect.top + 5, rect.left + 30, rect.Width() - 10, rect.Height() - 35, SWP_HIDEWINDOW | SWP_NOZORDER);

	// Setting initial shown tab being 
	this->m_pwndShow = &m_ga_ControlDlg;
	this->m_pwndShow->ShowWindow(SW_SHOW);

	// Setup the tool tip controller for main dialog
	this->m_mainToolTips = new CToolTipCtrl();
	this->m_mainToolTips->Create(this);

	this->m_mainToolTips->AddTool(GetDlgItem(IDC_SAVE_SETTINGS), L"Save current configurations to a file");
	this->m_mainToolTips->AddTool(GetDlgItem(IDC_LOAD_SETTINGS), L"Load a pre-made configuration from a file");
	this->m_mainToolTips->AddTool(GetDlgItem(IDC_MULTITHREAD_ENABLE), L"Enable the usage of multithreading to perform the GAs faster");
	this->m_mainToolTips->AddTool(GetDlgItem(IDC_SGA_BUTTON), L"Use the Simple Genetic Algorithm");
	this->m_mainToolTips->AddTool(GetDlgItem(IDC_UGA_BUTTON), L"Use the Micro Genetic Algorithm");
	this->m_mainToolTips->AddTool(GetDlgItem(IDC_OPT_BUTTON), L"Use the Brute Force Algorithm (multithreading is not utilized!)");
	this->m_mainToolTips->AddTool(GetDlgItem(IDC_START_STOP_BUTTON), L"Control start/abort of the algorithm");

	this->m_mainToolTips->Activate(true);

	//[SET UI DEFAULTS]
	// - get reference to slm controller
	this->slmCtrl = m_slmControlDlg.getSLMCtrl();
	this->slmCtrl->SetMainDlg(this);
	Utility::printLine("INFO: There are " + std::to_string(this->slmCtrl->numBoards) + " boards");

	// - set all default settings
	this->setDefaultUI();

	// Give a warning message if no boards have been detected
	if (this->slmCtrl != nullptr && this->slmCtrl->numBoards < 1) {
		MessageBox((LPCWSTR)L"No SLM has been detected to be connected!",
			(LPCWSTR)L"No SLM detected!",
			MB_ICONWARNING | MB_OK);
	}

	this->camCtrl = new CameraController(this);
	if (this->camCtrl != nullptr) {
		m_aoiControlDlg.SetCameraController(this->camCtrl);
	}
	else {
		Utility::printLine("WARNING: Camera Control NULL");
	}

	if (this->camCtrl != nullptr && !this->camCtrl->hasCameras()) {
		MessageBox(
			(LPCWSTR)L"No cameras have been detected to be connected!",
			(LPCWSTR)L"No cameras detected!",
			MB_ICONWARNING | MB_OK);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// Set the UI to default values
void MainDialog::setDefaultUI() {
	this->opt_selection_ = OptType::NONE;
	// Multithreading enabled by default
	this->m_MultiThreadEnable.SetCheck(BST_CHECKED);
	
	this->m_cameraControlDlg.setDefaultUI();

	this->m_ga_ControlDlg.setDefaultUI();

	this->m_ia_ControlDlg.setDefaultUI();

	this->m_aoiControlDlg.setDefaultUI();

	//Use slm control reference to set additional settings
	this->m_slmControlDlg.populateSLMlist(); // Simple method to setup the list of selections
	this->m_slmControlDlg.OnCbnSelchangeSlmSelect();

	// Output controls
	this->m_outputControlDlg.setDefaultUI();
}

/////////////////////////////////////////////////////////////////////
//
//   OnSysCommand() - MFC function
//
//   Inputs: none
//
//   Returns: none
//
//   Purpose: MFC function
//
//   Modifications: 
//
/////////////////////////////////////////////////////////////////////
void MainDialog::OnSysCommand(UINT nID, LPARAM lParam) {
	CDialog::OnSysCommand(nID, lParam);
}

////////////////////////////////////////////////////////////////////////////
//
//   OnPaint() - MFC function
//
//   Inputs: none
//
//   Returns: none
//
//   Purpose: If you add a minimize button to your dialog, you will need the code below
//			  to draw the icon.  For MFC applications using the document/view model,
//			  this is automatically done for you by the framework.
//
//   Modifications:
//
/////////////////////////////////////////////////////////////////////////
void MainDialog::OnPaint() {
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialog::OnPaint();
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//   OnQueryDragIcon() - MFC function
//   
//   Inputs: none
//
//   Returns: handle to the cursor
//
//   Purpose: The system calls this function to obtain the cursor to display while the user drags
//			  the minimized window.
//
//   Modifications:
//
/////////////////////////////////////////////////////////////////////////////
HCURSOR MainDialog::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}

/////////////////////////////////////////////////
//
//   OnClose()
//
//   Inputs: none
//
//   Returns: none
//
//   Purpose: This function is used to clean up at the close of the program. 
//			  It is called when the user clicks the little X in the upper corner.
//
//   Modifications:
//				Will halt closing if the optimization is still running, giving 
//				MessageBox to user as well as console to inform them
//
///////////////////////////////////////////////////
void MainDialog::OnClose() {
	// If optimization is running, give warning and prevent closing of application
	if (this->running_optimization_) {
		Utility::printLine("WARNING: Optimization still running!");
		MessageBox(
			(LPCWSTR)L"Still running optimization!",
			(LPCWSTR)L"The optimization is still running! Must be stopped first.",
			MB_ICONWARNING | MB_OK);
	}
	else {
		Utility::printLine("INFO: System beginning to close closing!");

		delete this->m_mainToolTips;
		delete this->camCtrl;
		// SLM controller is destructed by the SLM Dialog Controller
		//Finish console output
		fclose(fp);
		if (!FreeConsole()) {
			AfxMessageBox(L"Could not free the console!");
		}
		Utility::printLine("INFO: Console realeased (why are you seeing this?)");

		int result = int();
		this->EndDialog(result);
		CDialog::OnClose();
	}
}

/* OnOK: perfomrs enter key functions - used to prevent accidental exit of program and equipment damage */
void MainDialog::OnOK() {}

BOOL MainDialog::PreTranslateMessage(MSG* pMsg) {
	if (this->m_mainToolTips != NULL) {
		this->m_mainToolTips->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

// OnBnClickedUgaButton: Select the uGA Algorithm Button
void MainDialog::OnBnClickedUgaButton() {
	this->opt_selection_ = OptType::uGA;
	Utility::printLine("INFO: uGA optimization selected");

	// Disabling uGA (now that it's selected) and enabling other options and start button
	this->m_uGAButton.EnableWindow(false);
	this->m_SGAButton.EnableWindow(true);
	this->m_OptButton.EnableWindow(true);
	this->m_StartStopButton.EnableWindow(true);
}

//OnBnClickedSgaButton: Select the SGA Algorithm Button
void MainDialog::OnBnClickedSgaButton() {
	Utility::printLine("INFO: SGA optimization selected");
	this->opt_selection_ = OptType::SGA;

	// Disabling SGA (now that it's selected) and enabling other options and start button
	this->m_SGAButton.EnableWindow(false);
	this->m_uGAButton.EnableWindow(true);
	this->m_OptButton.EnableWindow(true);
	this->m_StartStopButton.EnableWindow(true);
}

//OnBnClickedOptButton: Select the OPT5 Algorithm Button
void MainDialog::OnBnClickedOptButton() {
	Utility::printLine("INFO: IA optimization selected");
	this->opt_selection_ = OptType::IA;

	// Disabling BF (now that it's selected) and enabling other options and start button
	this->m_OptButton.EnableWindow(false);
	this->m_SGAButton.EnableWindow(true);
	this->m_uGAButton.EnableWindow(true);
	this->m_StartStopButton.EnableWindow(true);
}

//OnTcnSelchangeTab1: changes the shown dialog when a new tab is selected
void MainDialog::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult) {
	//Hide the tab shown currently
	if (m_pwndShow != NULL)	{
		m_pwndShow->ShowWindow(SW_HIDE);
		m_pwndShow = NULL;
	}
	int tabIndex = m_TabControl.GetCurSel();
	switch (tabIndex) {
	case 0:
		m_pwndShow = &m_ga_ControlDlg;
		break;
	case 1:
		m_pwndShow = &m_ia_ControlDlg;
		break;
	case 2:
		m_pwndShow = &m_slmControlDlg;
		break;
	case 3:
		m_pwndShow = &m_cameraControlDlg;
		break;
	case 4:
		m_pwndShow = &m_aoiControlDlg;
		break;
	case 5:
		m_pwndShow = &m_outputControlDlg;
		break;
	default:
		Utility::printLine("WARNING: Requested to show a tab that shouldn't exist!");
	}

	this->m_pwndShow->ShowWindow(SW_SHOW);
	*pResult = 0;
}

// [UI UPDATE]
void MainDialog::disableMainUI(bool isMainEnabled) {
	//All controls have the same state
	// - enabled: when nothing is running
	// - disabled when the algorithm is running

	// Reference for directly enabling/disabling items: https://stackoverflow.com/questions/30350537/how-to-check-and-uncheck-and-enable-and-disable-a-check-box-control-in-mfc
	GetDlgItem(IDC_UGA_BUTTON)->EnableWindow(isMainEnabled);
	GetDlgItem(IDC_SGA_BUTTON)->EnableWindow(isMainEnabled);
	GetDlgItem(IDC_OPT_BUTTON)->EnableWindow(isMainEnabled);
	GetDlgItem(IDC_MULTITHREAD_ENABLE)->EnableWindow(isMainEnabled);
	GetDlgItem(IDC_SAVE_SETTINGS)->EnableWindow(isMainEnabled);
	GetDlgItem(IDC_LOAD_SETTINGS)->EnableWindow(isMainEnabled);
	GetDlgItem(IDC_TAB1)->EnableWindow(isMainEnabled);
}

// Start the selected optimization if haven't started, or attempt to stop if already running by setting flag
void MainDialog::OnBnClickedStartStopButton() {
	if (this->running_optimization_ == true) {
		Utility::printLine("INFO: Optimization currently running, attempting to stop safely.");
		this->stopFlag = true;
	}
	else {
		Utility::printLine("INFO: No optimization currently running, attempting to start depending on selection.");
		this->stopFlag = false;

		// Give an error message if no boards were detected to optimize
		if (this->slmCtrl->boards.size() < 1) {
			MessageBox(
				(LPCWSTR)L"No SLM has been detected to be optimize! Cancelling start of optimization.",
				(LPCWSTR)L"No SLM detected!",
				MB_ICONERROR | MB_OK);
			return;
		}
		else if (!this->slmCtrl->optimizeAny()) {
			MessageBox(
				(LPCWSTR)L"No SLM have been selected to be optimized! Cancelling start of optimization since there is nothing to optimize.",
				(LPCWSTR)L"No SLMs to optimize!",
				MB_ICONERROR | MB_OK);
			return;
		}

		// Give an error message if no camera
		if (!this->camCtrl->hasCameras()) {
			MessageBox(
				(LPCWSTR)L"No camera has been detected to possibly use! Cancelling action.",
				(LPCWSTR)L"No camera detected!",
				MB_ICONERROR | MB_OK);
			return;
		}
		this->runOptThread = AfxBeginThread(optThreadMethod, LPVOID(this), THREAD_PRIORITY_ABOVE_NORMAL);
		this->runOptThread->m_bAutoDelete = true; // Explicit setting for auto delete
	}
}

// Worker thread process for running optimization while MainDialog continues listening for other input
// Input: instance - pointer to MainDialog instance that called this method (will be cast to MainDialgo*)
// Output: optimization according to dlg.opt_selection_ is performed
UINT __cdecl optThreadMethod(LPVOID instance) {
	MainDialog * dlg = (MainDialog*)instance;
	dlg->disableMainUI(false); // Disable main UI (except for Start/Stop Button)

	// Setting that we are now runnign an optimization
	dlg->running_optimization_ = true;	// Change label of this button to START now that the optimization is over
	dlg->m_StartStopButton.SetWindowTextW(L"STOP");

	// Perform the optimzation operation depending on selection
	if (dlg->opt_selection_ == dlg->OptType::IA) {
		BruteForce_Optimization opt(dlg, dlg->camCtrl, dlg->slmCtrl);
		dlg->opt_success = opt.runOptimization();
	}
	else if (dlg->opt_selection_ == dlg->OptType::SGA) {
		SGA_Optimization opt(dlg, dlg->camCtrl, dlg->slmCtrl);
		dlg->opt_success = opt.runOptimization();
	}
	else if (dlg->opt_selection_ == dlg->OptType::uGA) {
		uGA_Optimization opt(dlg, dlg->camCtrl, dlg->slmCtrl);
		dlg->opt_success = opt.runOptimization();
	}
	else {
		Utility::printLine("ERROR: No optimization method selected!");
		dlg->opt_success = false;
	}
	// Output if error/failure in Optimization
	if (!dlg->opt_success) {
		Utility::printLine("ERROR: Optimization failed!");
		MessageBox(NULL, (LPCWSTR)L"An error had occurred while running the optimization.", (LPCWSTR)L"Error!", MB_ICONERROR | MB_OK);
	}

	// Change label of this button to START now that the optimization is over
	dlg->m_StartStopButton.SetWindowTextW(L"Start Optimization");
	// Update UI
	dlg->disableMainUI(true);

	Utility::printLine("INFO: End of worker optimization thread!");
	// Setting that we are no longer running an optimization
	dlg->running_optimization_ = false;
	return 0;
}

void MainDialog::OnBnClickedMultiThreadEnable() {
	bool thread_enabled = m_MultiThreadEnable.GetCheck() == BST_CHECKED;
	if (thread_enabled) {
		Utility::printLine("INFO: Multithreading enabled!");
	}
	else {
		Utility::printLine("INFO: Multithreading disabled!");
	}
	this->m_ga_ControlDlg.m_indEvalThreadCount.EnableWindow(thread_enabled);
	this->m_ga_ControlDlg.m_PopGenThreadCount.EnableWindow(thread_enabled);
}

// Display About Window as popup from button press
void MainDialog::OnBnClickedAboutButton() {
	CDialog aboutBox(IDD_ABOUTBOX);
	aboutBox.DoModal(); // not interested in response code or anything
}
