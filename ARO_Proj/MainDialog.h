////////////////////
// MainDialog.h - header file for the main dialog of the program
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#ifndef MAIN_DIALOG_
#define MAIN_DIALOG_

#include "afxwin.h"
#include "afxdialogex.h"
#include "afxcmn.h"
#include <stdlib.h>
#include <stdio.h>

#include "GA_ControlDialog.h"
#include "IA_ControlDialog.h"
#include "SLMControlDialog.h"
#include "CameraControlDialog.h"
#include "AOIControlDialog.h"
#include "OutputControlDialog.h"

class SLMController;
class CameraController;

class MainDialog : public CDialog {
public:
	// [GLOBAL PARAMETERS]
	SLMController* slmCtrl;
	CameraController* camCtrl;
	FILE* fp;

	// [CONSTRUCTOR(S)]
	// Standard Constructor
	MainDialog(CWnd* pParent = nullptr);

	// [UI UPDATES]
	void disableMainUI(bool isMainEnabled);
	// Set the UI to default values
	void setDefaultUI();
	// Set the UI according to given file path
	bool setUIfromFile(std::string filePath);

	// Set a value in the UI according to a given name and value as string (used by setUIfromFile)
	// Input: varName - string that should be identified to be a for a UI setting
	//		  varValue - string containing the value that the associated variable should be assigned (exact type depends on variable identified)
	// Output: variable associated with name is assigned varValue, returns false if error or inputs are empty
	bool setValueByName(std::string varName, std::string varValue);
	// Write current UI values with given file location
	bool saveUItoFile(std::string filePath);

	// Handle process of loading settings, requesting file to select and attempt load
	afx_msg void OnBnClickedLoadSettings();
	// Handle process of saving settings, requesting file path/name to save to
	afx_msg void OnBnClickedSaveSettings();

	// DIALOG DATA
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AROMAIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSlmPwrButton();
	virtual void OnOK();
	afx_msg void OnClose();

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;
	BOOL virtual PreTranslateMessage(MSG* pMsg);
public:

	// Enumeration for type of optimizations to select
	enum OptType {
		NONE,
		IA,
		SGA,
		uGA
	};
	OptType opt_selection_; // Current selected optimization algorithm

	CButton m_uGAButton; // Select uGA button
	CButton m_SGAButton; // Select SGA button
	CButton m_OptButton; // Select OPT5 (BruteForce) button
	CButton m_StartStopButton; // Start selected optimization button (or if opt is running will stop)
	CButton m_MultiThreadEnable; // If checked, perform the optimizations with multithreading where possible

	// Thread property to run the optimization algorithm through once committed to running
	CWinThread* runOptThread;

	// OnBnClickedUgaButton: Select the uGA Algorithm Button
	afx_msg void OnBnClickedUgaButton();
	//OnBnClickedSgaButton: Select the SGA Algorithm Button
	afx_msg void OnBnClickedSgaButton();
	//OnBnClickedOptButton: Select the OPT5 Algorithm Button
	afx_msg void OnBnClickedOptButton();
	afx_msg void OnBnClickedMultiThreadEnable();
	// Start the selected optimization if haven't started, or attempt to stop if already running by setting flag
	afx_msg void OnBnClickedStartStopButton();

	// Sub dialogs
		// Optimization settings when a GA is chosen
	GA_ControlDialog  m_ga_ControlDlg;
		// Optimization settings when IA is chosen
	IA_ControlDialog m_ia_ControlDlg;
	SLMControlDialog m_slmControlDlg;
	CameraControlDialog m_cameraControlDlg;
	AOIControlDialog m_aoiControlDlg;
	OutputControlDialog m_outputControlDlg;

	CWnd* m_pwndShow;
	CTabCtrl m_TabControl;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);

	// boolean variable accessed by the optimization method to determine if it should prematurely stop or not
	bool stopFlag = false;
	// boolean to track if the optimization aglorithm is running or not (used in start/stop button to determine action)
	bool running_optimization_;
	// Store output of optimization thread so that the gui can access it when finished
	bool opt_success;

	// Display About Window as popup from button press
	afx_msg void OnBnClickedAboutButton();
};

// Worker thread process for running optimization while MainDialog continues listening for other input
// Input: instance - pointer to MainDialog instance that called this method (will be cast to MainDialgo*)
// Output: optimization according to dlg.opt_selection_ is performed
UINT __cdecl optThreadMethod(LPVOID instance);

#endif
