////////////////////
// OutputControlDialog.cpp - implementation for "Optimization Settings" tab window
// Last edited: 09/28/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"
#include "OutputControlDialog.h"
#include "afxdialogex.h"
#include "Utility.h"

// OutputControlDialog dialog

#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

IMPLEMENT_DYNAMIC(OutputControlDialog, CDialogEx)

OutputControlDialog::OutputControlDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(OutputControlDialog::IDD, pParent) {
	this->m_mainToolTips = new CToolTipCtrl();
}

BOOL OutputControlDialog::OnInitDialog() {
	this->m_mainToolTips->Create(this);
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_LOGALL_FILES),		L"Enable all outputs");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_OUTPUT_PARAMETERS),	L"Save current settings in both loadable and readable files for reference");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_SAVE_ELITEIMAGE),	L"Save elite results across generations during optimization");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_SAVE_FINALIMAGE),	L"Save final optimized results");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_SAVE_TIMEVFIT),		L"Record time performances");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_DISPLAY_CAM),		L"Display the camera as the optimization runs");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_DISPLAY_SLM),		L"Display the SLMs being optimized as the optimization runs");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_OUTPUT_LOCATION),	L"Directory where the outputs will be stored");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_EXPOSURE_FILE),		L"Record when the exposure is changed during optimization");
	this->m_mainToolTips->AddTool(this->GetDlgItem(IDC_ELITE_SAVE_FREQ),	L"How frequent to save elite results (units of generation)");
	this->m_mainToolTips->Activate(true);

	return CDialogEx::OnInitDialog();
}

OutputControlDialog::~OutputControlDialog() {
	delete this->m_mainToolTips;
}

void OutputControlDialog::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DISPLAY_CAM, m_displayCameraCheck);
	DDX_Control(pDX, IDC_DISPLAY_SLM, m_displaySLM);
	DDX_Control(pDX, IDC_LOGALL_FILES, m_logAllFilesCheck);
	DDX_Control(pDX, IDC_SAVE_ELITEIMAGE, m_SaveEliteImagesCheck);
	DDX_Control(pDX, IDC_SAVE_TIMEVFIT, m_SaveTimeVFitnessCheck);
	DDX_Control(pDX, IDC_SAVE_FINALIMAGE, m_SaveFinalImagesCheck);
	DDX_Control(pDX, IDC_EXPOSURE_FILE, m_SaveExposureShortCheck);
	DDX_Control(pDX, IDC_OUTPUT_LOCATION, m_OutputLocationField);
	DDX_Control(pDX, IDC_ELITE_SAVE_FREQ, m_eliteSaveFreq);
	DDX_Control(pDX, IDC_OUTPUT_PARAMETERS, m_SaveParameters);
}


BEGIN_MESSAGE_MAP(OutputControlDialog, CDialogEx)
	ON_BN_CLICKED(IDC_OUTPUT_LOCATION_BUTTON, &OutputControlDialog::OnBnClickedOutputLocationButton)
	ON_BN_CLICKED(IDC_LOGALL_FILES, &OutputControlDialog::OnBnClickedLogallFiles)
	ON_BN_CLICKED(IDC_SAVE_ELITEIMAGE, &OutputControlDialog::OnBnClickedSaveEliteimage)
END_MESSAGE_MAP()

BOOL OutputControlDialog::PreTranslateMessage(MSG* pMsg) {
	if (this->m_mainToolTips != NULL) {
		this->m_mainToolTips->RelayEvent(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

// OutputControlDialog message handlers

// When the output location button is pressed, open a folder selection window to set new path
// and update this->m_OutputLocationField accordingly
void OutputControlDialog::OnBnClickedOutputLocationButton() {
	bool tryAgain;
	CString folderInput;
	do {
		tryAgain = false;
		LPWSTR p = folderInput.GetBuffer(FILE_LIST_BUFFER_SIZE);
		// https://www.codeproject.com/Tips/993640/Using-MFC-CFolderPickerDialog
		CString current;
		this->m_OutputLocationField.GetWindowTextW(current);
		CFolderPickerDialog dlgFolder(current);
		if (dlgFolder.DoModal() == IDOK) {
			folderInput = dlgFolder.GetPathName();

			folderInput += _T("\\");
			this->m_OutputLocationField.SetWindowTextW(folderInput);

			Utility::printLine("INFO: Updated output path to '" + std::string(CT2A(folderInput)) + "'!");
		}

	} while (tryAgain == true);
}

// When enable all is toggled, disable interactivity with the sub selection options
// Also calls OnBnClickedSaveEliteimage()
void OutputControlDialog::OnBnClickedLogallFiles() {
	bool enable = !(this->m_logAllFilesCheck.GetCheck() == BST_CHECKED);
	// If enable all is checked, then the sub-options are disabled
	this->m_SaveEliteImagesCheck.EnableWindow(enable);
	this->m_SaveFinalImagesCheck.EnableWindow(enable);
	this->m_SaveTimeVFitnessCheck.EnableWindow(enable);
	this->m_SaveExposureShortCheck.EnableWindow(enable);
	this->m_SaveParameters.EnableWindow(enable);
	this->OnBnClickedSaveEliteimage();
}

// if the save elite images option is enabled, enable interactivity with the frequency field
void OutputControlDialog::OnBnClickedSaveEliteimage() {
	bool enabled = this->m_SaveEliteImagesCheck.GetCheck() == BST_CHECKED || this->m_logAllFilesCheck.GetCheck() == BST_CHECKED;
	this->m_eliteSaveFreq.EnableWindow(enabled);
}

void OutputControlDialog::setDefaultUI() {
	// Default to saving images
	this->m_logAllFilesCheck.SetCheck(BST_CHECKED);
	// Default to displaying camera
	this->m_displayCameraCheck.SetCheck(BST_CHECKED);
	// Default to not displaying SLM
	this->m_displaySLM.SetCheck(BST_CHECKED);
	this->m_OutputLocationField.SetWindowTextW(_T(".\\logs\\"));
	this->m_eliteSaveFreq.SetWindowTextW(_T("10"));
	// Calling methods to disable/enable appropriate fields
	this->OnBnClickedLogallFiles();
	this->OnBnClickedSaveEliteimage();
}
