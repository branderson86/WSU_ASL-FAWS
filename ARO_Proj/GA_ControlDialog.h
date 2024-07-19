////////////////////
// GA_ControlDialog .h - dialog for settings of the genetic algorithms parameters
// Last edited: 09/28/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "resource.h"
#include "afxwin.h"

// OptimizationControlDialog dialog
class GA_ControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(GA_ControlDialog)
public:
	GA_ControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~GA_ControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;

	// Dialog Data
	enum { IDD = IDD_GA_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void setDefaultUI();

	// User Input Components //
	CEdit m_minFitness;
	CEdit m_minSeconds;
	CEdit m_minGenerations;
	CEdit m_binSize;
	CEdit m_numberBins;
	CEdit m_targetRadius;
	// GUI input for maximum number of generations, set to 0 for indefinite
	CEdit m_maxGenerations;
	// The maximum amount of time (in seconds) to run the optimization algorithm, set to 0 for indefinite time
	CEdit m_maxSeconds;
	// If toggled, will skip the elite individuals that were copied over
	CButton m_skipEliteReevaluation;
	// The number of threads to use when evaluating individuals in GA
	CEdit m_indEvalThreadCount;
	// The number of threads to use for generating pools
	CEdit m_PopGenThreadCount;
};
