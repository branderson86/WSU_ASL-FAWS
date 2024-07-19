////////////////////
// CameraControlDialog.cpp - implementation for camera settings tab window
// Last edited: 09/28/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"
#include "afxdialogex.h"
#include "CameraController.h"
#include "CameraControlDialog.h"

IMPLEMENT_DYNAMIC(CameraControlDialog, CDialogEx)

CameraControlDialog::CameraControlDialog(CWnd* pParent /*=NULL*/) : CDialogEx(CameraControlDialog::IDD, pParent) {
	this->m_mainToolTips = new CToolTipCtrl();
}

BOOL CameraControlDialog::OnInitDialog() {
	this->m_mainToolTips->Create(this);
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EXPOSURE_TIME_INPUT),L"The intial exposure used, when intensity is too high the exposure will be halved each time during optimization");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_FPS_INPUT),			L"The rate at which images are acquired, this is framerate is also attempted to be applied to the SLMs to match");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_GMMA_VALUE_INPUT),	L"Gamma setting for the camera");
	this->m_mainToolTips->Activate(true);

	return CDialogEx::OnInitDialog();
}

CameraControlDialog::~CameraControlDialog() {
	delete this->m_mainToolTips;
}

void CameraControlDialog::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPOSURE_TIME_INPUT, m_initialExposureTimeInput);
	DDX_Control(pDX, IDC_FPS_INPUT, m_FramesPerSecond);
	DDX_Control(pDX, IDC_GMMA_VALUE_INPUT, m_gammaValue);
}

BEGIN_MESSAGE_MAP(CameraControlDialog, CDialogEx)
END_MESSAGE_MAP()

BOOL CameraControlDialog::PreTranslateMessage(MSG* pMsg) {
	if (this->m_mainToolTips != NULL) {
		this->m_mainToolTips->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CameraControlDialog::setDefaultUI() {
	this->m_initialExposureTimeInput.SetWindowTextW(_T("2000"));
	this->m_FramesPerSecond.SetWindowTextW(_T("200"));
	this->m_gammaValue.SetWindowTextW(_T("1.00"));
}
