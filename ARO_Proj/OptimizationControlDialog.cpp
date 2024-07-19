////////////////////
// OptimizationControlDialog.cpp - implementation for "Optimization Settings" tab window
// Last edited: 07/23/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"
#include "OptimizationControlDialog.h"
#include "afxdialogex.h"

// OptimizationControlDialog dialog

IMPLEMENT_DYNAMIC(OptimizationControlDialog, CDialogEx)

OptimizationControlDialog::OptimizationControlDialog(CWnd* pParent /*=NULL*/)
: CDialogEx(OptimizationControlDialog::IDD, pParent) {
	this->m_mainToolTips = new CToolTipCtrl();
}

BOOL OptimizationControlDialog::OnInitDialog() {
	this->m_mainToolTips->Create(this);
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_MIN_FITNESS_INPUT), L"Minimum fitness value that the GA must reach before finishing");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_MIN_SECONDS_INPUT), L"Minimum time in seconds that the GA must run");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_MIN_GEN_INPUT), L"Minimum number of generations that the GA must evaluate");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EDIT_BIN_SIZE), L"Size of the bins of an image");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EDIT_NUMBER_BINS), L"Square dimension of the image being made to optimize onto the SLMs");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EDIT_TARGET_RADIUS), L"Radius of image to focus for optimizing intensity of");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_MAX_GENERATIONS), L"Maximum number of generations to allow the GA to evaluate (0 indicates indefinite)");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_MAX_SEC_INPUT), L"Maximum time to allow the GA to evaluate (0 indicates indefinite)");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_SKIP_ELITE_CHECK), L"Skip individuals in a pool that already have a determined fitness");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_THREAD_COUNT_IND), L"Set number of threads used to evaluate individuals");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_THREAD_COUNT_GA), L"Set number of threads for generating next generation (divided across number of SLMs)");
	this->m_mainToolTips->Activate(true);

	return CDialogEx::OnInitDialog();
}

OptimizationControlDialog::~OptimizationControlDialog() {
	delete this->m_mainToolTips;
}

void OptimizationControlDialog::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MIN_FITNESS_INPUT, m_minFitness);
	DDX_Control(pDX, IDC_MIN_SECONDS_INPUT, m_minSeconds);
	DDX_Control(pDX, IDC_MIN_GEN_INPUT, m_minGenerations);
	DDX_Control(pDX, IDC_EDIT_BIN_SIZE, m_binSize);
	DDX_Control(pDX, IDC_EDIT_NUMBER_BINS, m_numberBins);
	DDX_Control(pDX, IDC_EDIT_TARGET_RADIUS, m_targetRadius);
	DDX_Control(pDX, IDC_MAX_GENERATIONS, m_maxGenerations);
	DDX_Control(pDX, IDC_MAX_SEC_INPUT, m_maxSeconds);
	DDX_Control(pDX, IDC_SKIP_ELITE_CHECK, m_skipEliteReevaluation);
	DDX_Control(pDX, IDC_THREAD_COUNT_IND, m_indEvalThreadCount);
	DDX_Control(pDX, IDC_THREAD_COUNT_GA, m_PopGenThreadCount);
}


BOOL OptimizationControlDialog::PreTranslateMessage(MSG* pMsg) {
	if (this->m_mainToolTips != NULL) {
		this->m_mainToolTips->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(OptimizationControlDialog, CDialogEx)
END_MESSAGE_MAP()
