////////////////////
// SLMControlDialog.h - dialog for the "SLM Settings" tab window
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "resource.h"
#include "afxwin.h"

class SLMController;

#include <string>
// SLMControlDialog dialog

class SLMControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(SLMControlDialog)
public:
	SLMControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SLMControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;

	SLMController* slmCtrl;

// Dialog Data
	enum { IDD = IDD_SLM_CONTROL };

	SLMController* getSLMCtrl();

	// Simple method for populating the selection list depending on number of boards connected
	// syntax for each item is "Board #[1 based index]"
	void populateSLMlist();

	// Method for encapsulating the UX of setting LUT file for given board and filepath
	// Input:
	//		slmNum - the board being assigned (0 based index)
	//		filePath - string to path of the LUT file being loaded
	// Output: returns true if error occurs and user selects retry, false otherwise
	//		When an error occurs in loading LUT file, gives message box with option to retry or cancel
	bool attemptLUTload(int i, std::string filePath);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// User Input Components //
		// Field to show current board's LUT file path (not active input)
	CEdit m_LUT_pathDisplay;
		// Button to power on/off the current board
	CButton m_SlmPwrButton;
		// Selection List for current SLM to set LUT files and power
		// DON'T USE GetCurSel() with this!  Use this->slmSelectionID_
	CComboBox slmSelection_;
		// If checked/TRUE then assign all config changes to all the boards (ignoring selection)
	CButton SLM_SetALLSame_;
		// Current selection according to combo box, use this if you want current selection rather than GetCurSel
		// Default in construction to value 0 (first board)
	int slmSelectionID_;

	// Turn the select SLM's power on/off accordingly
	afx_msg void OnBnClickedSlmPwrButton();
	// Open a file selection window to get a file path, if it fails prompt to try again
	afx_msg void OnBnClickedSetlut();

	// Update selection ID value and the GUI according to the board
	afx_msg void OnCbnSelchangeSlmSelect();
	afx_msg void OnCbnChangeSlmAll();

	afx_msg void OnBnClickedOptToggle();
	// If checked, the currently selected board(s) will be set to be included in the optimization
	CButton m_optBoardCheck;
};
