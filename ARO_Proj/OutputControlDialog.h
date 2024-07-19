////////////////////
// OutputControlDialog.h - dialog for the "Output Settings" tab window
// Last edited: 09/28/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "afxwin.h"
#include "resource.h"

// OutputControlDialog dialog
class OutputControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(OutputControlDialog)
public:
	OutputControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~OutputControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;

	// Dialog Data
	enum { IDD = IDD_OUTPUT_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void setDefaultUI();

	// User Input Components //
	// If checked, then when running the optimization produce a live camera display
	CButton m_displayCameraCheck;
	// If checked, set to display SLMs
	CButton m_displaySLM;
	// If checked, output various logging files to record performance of the optimization
	CButton m_logAllFilesCheck;
	// If checked, the optimization should save the images of elite individual during optimization
	CButton m_SaveEliteImagesCheck;
	// If checked, will output various timing info in timeVSfitness file
	CButton m_SaveTimeVFitnessCheck;
	// If checked, outputs to a file whenever exposure is shortened during optimization
	CButton m_SaveExposureShortCheck;
	// If checked, save the resulting optimized image (pretty much should always be true)
	CButton m_SaveFinalImagesCheck;
	// Gives where the user wants to store the output contents, expected to be a path to a usable folder (not a file!)
	CEdit m_OutputLocationField;
	// Feld for how frequent to save elite images during optimization if enabled
	CEdit m_eliteSaveFreq;
	// If enabled, output the current settings in a readable file
	CButton m_SaveParameters;

	// OutputControlDialog message handlers //
	// When the output location button is pressed, open a folder selection window to set new path
	// and update this->m_OutputLocationField accordingly
	afx_msg void OnBnClickedOutputLocationButton();
	// When enable all is toggled, disable interactivity with the sub selection options
	// Also calls OnBnClickedSaveEliteimage()
	afx_msg void OnBnClickedLogallFiles();
	// if the save elite images option is enabled, enable interactivity with the frequency field
	afx_msg void OnBnClickedSaveEliteimage();
};
