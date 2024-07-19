////////////////////
// AOIControlDIalog.h - dialog for the "AOI Settings" tab window
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#pragma once

#include "resource.h"
#include "afxwin.h"

class CameraController;

// AOIControlDialog dialog
class AOIControlDialog : public CDialogEx {
	DECLARE_DYNAMIC(AOIControlDialog)

public:
	AOIControlDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~AOIControlDialog();

	BOOL virtual PreTranslateMessage(MSG* pMsg);

	CameraController* cc;
	// Tool tips to help inform the user about a control
	CToolTipCtrl * m_mainToolTips;

	// Dialog Data
	enum { IDD = IDD_AOI_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void setDefaultUI();

	// User Input Components //
	// AOI horizontal offset
	CEdit m_leftInput;
	// AOI vertical offset
	CEdit m_rightInput;
	// AOI width
	CEdit m_widthInput;
	// AOI height
	CEdit m_heightInput;
	// Button to set max image AOI
	CButton m_maxImageSizeBtn;
	// Button to center the AOI within the camera's full image
	CButton m_centerAOIBtn;

	void SetCameraController(CameraController* cc);

	// Set the AOI fields
	// Input: x - the horizontal offset in pixels
	//		  y - the vertical offset in pixels
	//	  width - width of the AOI image in pixels
	//	 height - height of the AOI image in pixels
	// Output: The appropriate AOI fields are updated to the inputted values
	void SetAOIFeilds(int x, int y, int width, int height);

	// Evant handlers for the buttons //

	// Input: The text fields for width and height
	// Output: The x,y offsets (and width/height if necessary) to have the AOI be centered within the image
	afx_msg void OnBnClickedCenterAoiButton();

	// Set the AOI fields to use the entire image of the camera
	afx_msg void OnBnClickedMaxImageSizeButton();
};
