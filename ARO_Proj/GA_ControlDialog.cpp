////////////////////
// OptimizationControlDialog.cpp - implementation for "Optimization Settings" tab window
// Last edited: 09/28/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"
#include "GA_ControlDialog.h"
#include "afxdialogex.h"

#include <thread> // for hadware_concurrency()
#include <string> // for to_string()

// OptimizationControlDialog dialog

IMPLEMENT_DYNAMIC(GA_ControlDialog, CDialogEx)

GA_ControlDialog::GA_ControlDialog(CWnd* pParent /*=NULL*/)
: CDialogEx(GA_ControlDialog::IDD, pParent) {
	this->m_mainToolTips = new CToolTipCtrl();
}

BOOL GA_ControlDialog::OnInitDialog() {
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

GA_ControlDialog ::~GA_ControlDialog() {
	delete this->m_mainToolTips;
}

void GA_ControlDialog::DoDataExchange(CDataExchange* pDX) {
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


BOOL GA_ControlDialog::PreTranslateMessage(MSG* pMsg) {
	if (this->m_mainToolTips != NULL) {
		this->m_mainToolTips->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void GA_ControlDialog::setDefaultUI() {
	this->m_binSize.SetWindowTextW(_T("16"));
	this->m_numberBins.SetWindowTextW(_T("32"));
	this->m_targetRadius.SetWindowTextW(_T("2"));
	this->m_minFitness.SetWindowTextW(_T("0"));
	this->m_minSeconds.SetWindowTextW(_T("60"));
	this->m_maxSeconds.SetWindowTextW(_T("0")); // 0 or less indicates indefinite
	this->m_minGenerations.SetWindowTextW(_T("0"));
	this->m_maxGenerations.SetWindowTextW(_T("3000")); // 0 or less indicates indefinite
	// Setting default values to be dependent on hardware
	CString hardwareThreads = CString(std::to_string(std::thread::hardware_concurrency()).c_str());
	this->m_indEvalThreadCount.SetWindowTextW(hardwareThreads);
	this->m_PopGenThreadCount.SetWindowTextW(hardwareThreads);
}

BEGIN_MESSAGE_MAP(GA_ControlDialog, CDialogEx)
END_MESSAGE_MAP()
