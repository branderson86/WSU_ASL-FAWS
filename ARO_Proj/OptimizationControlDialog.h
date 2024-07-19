////////////////////
// OptimizationControlDialog.h - dialog for the "Optimization Settings" tab window
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "resource.h"
#include "afxwin.h"

// OptimizationControlDialog dialog
class OptimizationControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(OptimizationControlDialog)
public:
	OptimizationControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~OptimizationControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;

	// Dialog Data
	enum { IDD = IDD_OPTIMIZATION_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
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
