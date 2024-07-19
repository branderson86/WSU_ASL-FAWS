////////////////////
// CameraControlDialog.h - dialog for the "Camera Settings" tab window
// Last edited: 09/28/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "afxwin.h"
#include "resource.h"

// CameraControlDialog dialog
class CameraControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(CameraControlDialog)

public:
	CameraControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CameraControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;

	// Dialog Data
	enum { IDD = IDD_CAMERA_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void setDefaultUI();

	// User Input Components //
	// Exposure setting in microseconds (us)
	CEdit m_initialExposureTimeInput;
	CEdit m_FramesPerSecond;
	CEdit m_gammaValue;
};
