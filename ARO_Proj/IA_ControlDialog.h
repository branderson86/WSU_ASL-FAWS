////////////////////
// IA_OptimizationControlDialog.h - dialog for the settings of iterative algorithm optimization parameters
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "resource.h"
#include "afxwin.h"

// IA_OptimizationControlDialog dialog

class IA_ControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(IA_ControlDialog)

public:
	IA_ControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~IA_ControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_IA_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void setDefaultUI();
	// IA only parameter, phase resolution of a given bin in the generated optimal image
	CEdit m_phaseResolution;

	CEdit m_binSize;
	CEdit m_numBins;
	CEdit m_targetRadius;
};
