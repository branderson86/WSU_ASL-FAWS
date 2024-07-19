// IA_OptimizationControlDialog.cpp : implementation file
//

#include "stdafx.h"
#include "IA_ControlDialog.h"
#include "afxdialogex.h"

// IA_OptimizationControlDialog dialog

IMPLEMENT_DYNAMIC(IA_ControlDialog, CDialogEx)

IA_ControlDialog::IA_ControlDialog(CWnd* pParent /*=NULL*/)
: CDialogEx(IA_ControlDialog::IDD, pParent) {
	this->m_mainToolTips = new CToolTipCtrl();
}

IA_ControlDialog::~IA_ControlDialog() {
	delete this->m_mainToolTips;
}

BOOL IA_ControlDialog::OnInitDialog() {
	this->m_mainToolTips->Create(this);
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EDIT_BIN_SIZE), L"Size of the bins of an image");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EDIT_NUMBER_BINS), L"Square dimension of the image being made to optimize onto the SLMs");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_PHASE_RESOLUTION), L"Set the depth resolution of the optimal image (do not exceed 16!)");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EDIT_TARGET_RADIUS), L"Radius of image to focus for optimizing intensity of");
	this->m_mainToolTips->Activate(true);

	return CDialogEx::OnInitDialog();
}

BOOL IA_ControlDialog::PreTranslateMessage(MSG* pMsg) {
	if (this->m_mainToolTips != NULL) {
		this->m_mainToolTips->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void IA_ControlDialog::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_BIN_SIZE, m_binSize);
	DDX_Control(pDX, IDC_EDIT_NUMBER_BINS, m_numBins);
	DDX_Control(pDX, IDC_PHASE_RESOLUTION, m_phaseResolution);
	DDX_Control(pDX, IDC_EDIT_TARGET_RADIUS, m_targetRadius);
}

void IA_ControlDialog::setDefaultUI() {
	this->m_binSize.SetWindowTextW(_T("16"));
	this->m_numBins.SetWindowTextW(_T("32"));
	this->m_phaseResolution.SetWindowTextW(_T("16"));
	this->m_targetRadius.SetWindowTextW(_T("2"));
}

BEGIN_MESSAGE_MAP(IA_ControlDialog, CDialogEx)
END_MESSAGE_MAP()

// IA_OptimizationControlDialog message handlers
