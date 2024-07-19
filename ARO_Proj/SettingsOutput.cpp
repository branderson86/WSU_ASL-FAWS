////////////////////
// Additional source file to export some of the contents of MainDialog.cpp for easier navigation
// SettingsOutput.cpp - behavior of the Save Settings and Load Settings buttons within MainDialog
// Last edited: 08/10/2021 by Andrew O'Kins
////////////////////
#include "stdafx.h"				// Required in source

#include "MainDialog.h"			// Header file for dialog functions
#include "SLMController.h"		// References to SLM controller for setting LUT files and power
#include "CameraController.h"   // References which build version
#include <fstream>				// for file i/o

#include "Utility.h"			// For printLine console output

#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

// Handle process of loading settings, requesting file to select and attempt load the contents
void MainDialog::OnBnClickedLoadSettings() {
	bool tryAgain; // Boolean that if true means to try another attempt to load a file
	CString fileName;
	LPWSTR p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
	std::string filePath;
	do {
		tryAgain = false; // Initially assuming that we won't be needing to try again

		// Default to .cfg file extension (this program uses that extension in generation of saving settings)
		static TCHAR BASED_CODE filterFiles[] = _T("Config Files (*.cfg)|*.cfg|ALL Files (*.*)|*.*||");

		CFileDialog dlgFile(TRUE, NULL, L"./", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filterFiles);

		OPENFILENAME& ofn = dlgFile.GetOFN();
		ofn.lpstrFile = p;
		ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

		if (dlgFile.DoModal() == IDOK) { // Given a path to file to use
			fileName = dlgFile.GetPathName();
			fileName.ReleaseBuffer();

			filePath = CT2A(fileName);

			if (!setUIfromFile(filePath)) {
				int err_response = MessageBox(
					(LPCWSTR)L"An error occurred while trying load the file\nTry Again?",
					(LPCWSTR)L"ERROR in file load!",
					MB_ICONERROR | MB_RETRYCANCEL);
				// Respond to decision
				switch (err_response) {
				case IDRETRY:
					tryAgain = true;
					break;
				default: // Cancel or other unknown response will not have try again to make sure not stuck in undesired loop
					tryAgain = false;
				}
			}
			else {
				Utility::printLine("INFO: Successfully loaded settings from " + filePath);
			}
		}
		else {
			tryAgain = false;
		}
	} while (tryAgain);
	this->m_outputControlDlg.OnBnClickedLogallFiles(); // Enable/Disable the output parameters accordingly
	this->OnBnClickedMultiThreadEnable(); // Enable/Disable the threading parameters accordingly
}

// Set the UI according to given file path, called from Load Settings
// Order dependent input from file!
// Input: filePath - string to file to load settings from
// Output: file is read and the identified variables are used to configure the GUI/SLMs
bool MainDialog::setUIfromFile(std::string filePath) {
	// Open file
	std::ifstream inputFile(filePath);
	std::string lineBuffer;
	try {// Read each line
		while (std::getline(inputFile, lineBuffer)) {
			// If not empty and not a commented out line
			if (lineBuffer != "" && (lineBuffer.find("#") != 0)) {
				// Locate the "=" and use as pivot where prior is the variable name and after is variable value
				int equals_pivot = int(lineBuffer.find("="));
				// Assumption made that there are no spaces from variable name to variable value, rather only occurring after a variable is assigned a value and afterwards may be an in-line comment
				int end_point = int(lineBuffer.find_first_of(" "));

				if (equals_pivot != end_point) {
					// With the two positions acquired, capture the varable's name and the value it is being assigned
					std::string variableName = lineBuffer.substr(0, equals_pivot);
					std::string variableValue = lineBuffer.substr(equals_pivot + 1, end_point - equals_pivot - 1);

					if (!setValueByName(variableName, variableValue)) {
						Utility::printLine("WARNING: Failure to interpret variable '" + variableName + "' with value '" + variableValue + "'! Continuing on to next variable");
					}
				}
			}
		}
	}
	catch (std::exception &e) {
		Utility::printLine("ERROR: " + std::string(e.what()));
		return false;
	}
	return true;
}

// Set a value in the UI according to a given name and value as string (used by setUIfromFile)
// Input: varName - string that should be identified to be a for a UI setting
//		  varValue - string containing the value that the associated variable should be assigned (exact type depends on variable identified)
// Output: variable associated with name is assigned varValue, returns false if error or inputs are empty
bool MainDialog::setValueByName(std::string name, std::string value) {
	CString valueStr(value.c_str());
	// Do a pre-emptive check if either name or value are empty and return false if either are
	if (name == "" || value == "") {
		return false;
	}
	else if (name == "multithreading") {
		this->m_MultiThreadEnable.SetCheck(value == "true");
	}
	else if (name == "algorithm") {
		switch (OptType(std::stoi(value))) {
		case(OptType::IA) :
			this->OnBnClickedOptButton();
			break;
		case(OptType::SGA) :
			this->OnBnClickedSgaButton();
			break;
		case(OptType::uGA) :
			this->OnBnClickedUgaButton();
			break;
		}
	}

	// Camera Dialog
	else if (name == "initialExposureTime")
		this->m_cameraControlDlg.m_initialExposureTimeInput.SetWindowTextW(valueStr);
	else if (name == "framesPerSecond")
		this->m_cameraControlDlg.m_FramesPerSecond.SetWindowTextW(valueStr);
	else if (name == "gamma")
		this->m_cameraControlDlg.m_gammaValue.SetWindowTextW(valueStr);
	// AOI Dialog
	else if (name == "leftAOI")
		this->m_aoiControlDlg.m_leftInput.SetWindowTextW(valueStr);
	else if (name == "rightAOI")
		this->m_aoiControlDlg.m_rightInput.SetWindowTextW(valueStr);
	else if (name == "widthAOI")
		this->m_aoiControlDlg.m_widthInput.SetWindowTextW(valueStr);
	else if (name == "heightAOI")
		this->m_aoiControlDlg.m_heightInput.SetWindowTextW(valueStr);
	// GA Optimization Dialog
	else if (name == "binSize")
		this->m_ga_ControlDlg.m_binSize.SetWindowTextW(valueStr);
	else if (name == "binNumber")
		this->m_ga_ControlDlg.m_numberBins.SetWindowTextW(valueStr);
	else if (name == "targetRadius")
		this->m_ga_ControlDlg.m_targetRadius.SetWindowTextW(valueStr);
	else if (name == "minFitness")
		this->m_ga_ControlDlg.m_minFitness.SetWindowTextW(valueStr);
	else if (name == "minSeconds")
		this->m_ga_ControlDlg.m_minSeconds.SetWindowTextW(valueStr);
	else if (name == "maxSeconds")
		this->m_ga_ControlDlg.m_maxSeconds.SetWindowTextW(valueStr);
	else if (name == "minGenerations")
		this->m_ga_ControlDlg.m_minGenerations.SetWindowTextW(valueStr);
	else if (name == "maxGenerations")
		this->m_ga_ControlDlg.m_maxGenerations.SetWindowTextW(valueStr);
	else if (name == "evalIndividualsThreadCount")
		this->m_ga_ControlDlg.m_indEvalThreadCount.SetWindowTextW(valueStr);
	else if (name == "popGenThreadCount")
		this->m_ga_ControlDlg.m_PopGenThreadCount.SetWindowTextW(valueStr);
	else if (name == "skipEliteReeval") {
		this->m_ga_ControlDlg.m_skipEliteReevaluation.SetCheck(valueStr == "true");
	}
	// IA Optimization Dialog
	else if (name == "ia_binNumber")
		this->m_ia_ControlDlg.m_numBins.SetWindowTextW(valueStr);
	else if (name == "ia_binSize")
		this->m_ia_ControlDlg.m_binSize.SetWindowTextW(valueStr);
	else if (name == "ia_phaseResolution")
		this->m_ia_ControlDlg.m_phaseResolution.SetWindowTextW(valueStr);
	else if (name == "ia_targetRadius")
		this->m_ia_ControlDlg.m_targetRadius.SetWindowTextW(valueStr);

	// SLM Dialog
	else if (name == "slmSelect")  {
		int selectID = std::stoi(value.c_str()) - 1; // Correct from base 1 index to 0 based
		if (selectID >= this->slmCtrl->boards.size() || selectID < 0) {
			Utility::printLine("WARNING: Load setting attempted to set select SLM out of bounds!");
			return false;
		}
		else {
			this->m_slmControlDlg.slmSelection_.SetCurSel(selectID);
		}
	}
	else if (name.substr(0, 14) == "slmLutFilePath") {
		// We are dealing with LUT file setting
		int boardID = std::stoi(name.substr(14, name.length() - 14)) - 1;
		if (this->slmCtrl != NULL) {
			if (boardID < this->slmCtrl->boards.size() && boardID >= 0) {
				// Resource for conversion https://stackoverflow.com/questions/258050/how-do-you-convert-cstring-and-stdstring-stdwstring-to-each-other
				CT2CA converString(valueStr);
				// Make an initial attempt using what was read by the file
				bool tryAgain = !this->m_slmControlDlg.attemptLUTload(boardID, std::string(converString));
				do {
					// If the assignment from the file failed, notify the user and allow them to set the LUT file
					if (tryAgain) {
						CString fileName;
						std::string filePath;
						LPWSTR p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
						// Initially only show LUT files (end in .LUT extension) but also provide option to show all files
						static TCHAR BASED_CODE filterFiles[] = _T("LUT Files (*.LUT)|*.LUT|ALL Files (*.*)|*.*||");
						// Construct and open standard Windows file dialog box with default filename being "./slm3986_at532_P8.LUT"
						CFileDialog dlgFile(TRUE, NULL, L"", OFN_FILEMUSTEXIST, filterFiles);

						OPENFILENAME& ofn = dlgFile.GetOFN();
						ofn.lpstrFile = p;
						ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

						if (dlgFile.DoModal() == IDOK) {
							fileName = dlgFile.GetPathName();
							fileName.ReleaseBuffer();
							filePath = CT2A(fileName);

							if (this->m_slmControlDlg.SLM_SetALLSame_.GetCheck() == BST_UNCHECKED) {
								// Get the SLM being assinged the LUT file, asssumes the index poistion of the selection is consistent with board selection
								//		for example if the user selects 1 (out of 2) the value should be 0.  This is done currently (June 15th as a shortcut instead of parsing text of selection)
								tryAgain = !this->m_slmControlDlg.attemptLUTload(this->m_slmControlDlg.slmSelectionID_, filePath);
							}
							else {
								// SLM set all setting is checked, so assign the same LUT file to all the boards
								for (int slmNum = 0; slmNum < this->slmCtrl->boards.size() && !tryAgain; slmNum++) {
									tryAgain = !this->m_slmControlDlg.attemptLUTload(slmNum, filePath);
								}
							}
							CString fileCS(this->slmCtrl->boards[this->m_slmControlDlg.slmSelectionID_]->LUTFileName.c_str());
							this->m_slmControlDlg.m_LUT_pathDisplay.SetWindowTextW(fileCS);
						}
						else {
							tryAgain = false;
						}
					}
				} while (tryAgain); // End of assigning LUT to board
			}
			// Out of bounds
			else {
				Utility::printLine("WARNING: Load setting attempted to assign LUT file out of bounds!");
				return false;
			}
		}
		else {
			return false;
		}
	}
	else if (name.substr(0, 10) == "slmPowered") {
		int boardID = std::stoi(name.substr(10, name.length() - 10)) - 1;

		if (this->slmCtrl != NULL) {
			if (boardID < this->slmCtrl->boards.size() && boardID >= 0) {
				// Resource for conversion https://stackoverflow.com/questions/258050/how-do-you-convert-cstring-and-stdstring-stdwstring-to-each-other
				CT2CA converString(valueStr);
				bool power = (value == "true");
				this->slmCtrl->setBoardPower(boardID, power);

				// Update power button label if select is correct
				if (boardID == this->m_slmControlDlg.slmSelectionID_) {
					CString settingPowerMsg;
					if (power) { settingPowerMsg = "Turn power OFF"; }
					else { settingPowerMsg = "Turn power ON"; }

					this->m_slmControlDlg.m_SlmPwrButton.SetWindowTextW(settingPowerMsg);
				}
			}
			else {
				Utility::printLine("WARNING: Load setting attempted to set power to a board out of bounds!");
			}
		}
	}
	else if (name.substr(0, 11) == "slmOptimize") {
		int boardID = std::stoi(name.substr(11, name.length() - 11)) - 1;

		if (this->slmCtrl != NULL) {
			if (boardID < this->slmCtrl->boards.size() && boardID >= 0) {
				// Resource for conversion https://stackoverflow.com/questions/258050/how-do-you-convert-cstring-and-stdstring-stdwstring-to-each-other
				CT2CA converString(valueStr);
				bool opt = (value == "true");
				this->slmCtrl->boards[boardID]->setOptimize(opt);

				// Update opt toggle button if current select is optimized
				if (boardID == this->m_slmControlDlg.slmSelectionID_) {
					this->m_slmControlDlg.m_optBoardCheck.SetCheck(boardID == this->m_slmControlDlg.slmSelectionID_);
				}
			}
			else {
				Utility::printLine("ERROR: A board set to be optimized is not connected!  If this is not intended then you are missing boards!");
			}
		}
	}

	else if (name == "SLMselectAll") {
		this->m_slmControlDlg.SLM_SetALLSame_.SetCheck(value == "true");
	}

	// Output Controls Dialog variables
	else if (name == "saveEliteImage") {
		this->m_outputControlDlg.m_SaveEliteImagesCheck.SetCheck(value == "true");
	}
	else if (name == "displayCamera") {
		this->m_outputControlDlg.m_displayCameraCheck.SetCheck(value == "true");
	}
	else if (name == "displaySLM") {
		this->m_outputControlDlg.m_displaySLM.SetCheck(value == "true");
	}
	else if (name == "logAllFilesEnable") {
		this->m_outputControlDlg.m_logAllFilesCheck.SetCheck(value == "true");
	}
	else if (name == "saveParameters") {
		this->m_outputControlDlg.m_SaveParameters.SetCheck(value == "true");
	}
	else if (name == "saveFinalImages") {
		this->m_outputControlDlg.m_SaveFinalImagesCheck.SetCheck(value == "true");
	}
	else if (name == "saveTimeVsFitness") {
		this->m_outputControlDlg.m_SaveTimeVFitnessCheck.SetCheck(value == "true");
	}
	else if (name == "saveExposureShortening") {
		this->m_outputControlDlg.m_SaveExposureShortCheck.SetCheck(value == "true");
	}
	else if (name == "saveEliteFreq") {
		this->m_outputControlDlg.m_eliteSaveFreq.SetWindowTextW(valueStr);
	}
	else if (name == "multiThreading") {
		this->m_MultiThreadEnable.SetCheck(value == "true");
	}
	else if (name == "outputFolder") {
		this->m_outputControlDlg.m_OutputLocationField.SetWindowTextW(valueStr);
	}
	else {	// Unidentfied variable name, return false
		return false;
	}
	return true;
}

// When the Save Settings button is pressed, prompt the user to give where to save the file to for storing all parameters & preferences
void MainDialog::OnBnClickedSaveSettings() {
	bool tryAgain;
	CString fileName;
	LPWSTR p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
	std::string filePath;
	do {
		tryAgain = false;

		// Default Save As dialog box with extension as .cfg
		CFileDialog dlgFile(FALSE, L"cfg");

		OPENFILENAME& ofn = dlgFile.GetOFN();
		ofn.lpstrFile = p;
		ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;

		// If action is OK
		if (dlgFile.DoModal() == IDOK) {
			fileName = dlgFile.GetPathName();
			fileName.ReleaseBuffer();

			filePath = CT2A(fileName);

			if (!saveUItoFile(filePath)) {
				int err_response = MessageBox(
					(LPCWSTR)L"An error occurred while trying save settings\nTry Again?",
					(LPCWSTR)L"ERROR in saving!",
					MB_ICONERROR | MB_RETRYCANCEL);
				// Respond to decision
				switch (err_response) {
				case IDRETRY:
					tryAgain = true;
					break;
				default: // Cancel or other unknown response will not have try again to make sure not stuck in undesired loop
					tryAgain = false;
				}
			}
			else {// Give success message when no issues
				Utility::printLine("INFO: Successfully saved settings to " + filePath);
				MessageBox((LPCWSTR)L"Successfully saved settings.",
					(LPCWSTR)L"Success!",
					MB_ICONINFORMATION | MB_OK);
				tryAgain = false;
			}
		}
		// If action is cancel
		else {
			tryAgain = false;
		}

	} while (tryAgain);
}

// Write current UI values with given file location
bool MainDialog::saveUItoFile(std::string filePath) {
	std::ofstream outFile; // output stream
	CString tempBuff; // Temporary hold of what's in dialog window of given variable
	outFile.open(filePath);

	outFile << "# ARO PROJECT CONFIGURATION FILE" << std::endl;
	// Give which version (dependent on SDK) the configuration is for
	outFile << "# For ";
#ifdef USE_SPINNAKER
	outFile << "SPINNAKER";
#endif
#ifdef USE_PICAM
	outFile << "PICAM";
#endif
	outFile << " version" << std::endl;

	// Main Dialog settings
	outFile << "# Optimization Algorithm" << std::endl;
	outFile << "algorithm=" << this->opt_selection_ << std::endl;

	outFile << "# Multithreading" << std::endl;
	outFile << "multiThreading=";
	if (this->m_MultiThreadEnable.GetCheck() == BST_CHECKED) { outFile << "true" << std::endl; }
	else { outFile << "false" << std::endl; }

	// Camera Dialog settings
	outFile << "# Camera Settings" << std::endl;
	this->m_cameraControlDlg.m_initialExposureTimeInput.GetWindowTextW(tempBuff);
	outFile << "initialExposureTime=" << _tstof(tempBuff) << std::endl;
	this->m_cameraControlDlg.m_FramesPerSecond.GetWindowTextW(tempBuff);
	outFile << "framesPerSecond=" << _tstof(tempBuff) << std::endl;;
	this->m_cameraControlDlg.m_gammaValue.GetWindowTextW(tempBuff);
	outFile << "gamma=" << _tstof(tempBuff) << std::endl;
	// AOI Dialog settings
	outFile << "# AOI Settings" << std::endl;
	this->m_aoiControlDlg.m_leftInput.GetWindowTextW(tempBuff);
	outFile << "leftAOI=" << _tstof(tempBuff) << std::endl;
	this->m_aoiControlDlg.m_rightInput.GetWindowTextW(tempBuff);
	outFile << "rightAOI=" << _tstof(tempBuff) << std::endl;
	this->m_aoiControlDlg.m_widthInput.GetWindowTextW(tempBuff);
	outFile << "widthAOI=" << _tstof(tempBuff) << std::endl;
	this->m_aoiControlDlg.m_heightInput.GetWindowTextW(tempBuff);
	outFile << "heightAOI=" << _tstof(tempBuff) << std::endl;

	// Optimization Dialog settings
	outFile << "# Genetic Algorithm Optimization Settings" << std::endl;
	this->m_ga_ControlDlg.m_binSize.GetWindowTextW(tempBuff);
	outFile << "binSize=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_numberBins.GetWindowTextW(tempBuff);
	outFile << "binNumber=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_targetRadius.GetWindowTextW(tempBuff);
	outFile << "targetRadius=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_minFitness.GetWindowTextW(tempBuff);
	outFile << "minFitness=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_minSeconds.GetWindowTextW(tempBuff);
	outFile << "minSeconds=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_maxSeconds.GetWindowTextW(tempBuff);
	outFile << "maxSeconds=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_minGenerations.GetWindowTextW(tempBuff);
	outFile << "minGenerations=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_maxGenerations.GetWindowTextW(tempBuff);
	outFile << "maxGenerations=" << _tstof(tempBuff) << std::endl;
	outFile << "skipEliteReeval=";
	if (this->m_ga_ControlDlg.m_skipEliteReevaluation.GetCheck() == BST_CHECKED) { outFile << "true" << std::endl; }
	else { outFile << "false" << std::endl; }
	this->m_ga_ControlDlg.m_indEvalThreadCount.GetWindowTextW(tempBuff);
	outFile << "evalIndividualsThreadCount=" << _tstof(tempBuff) << std::endl;
	this->m_ga_ControlDlg.m_PopGenThreadCount.GetWindowTextW(tempBuff);
	outFile << "popGenThreadCount=" << _tstof(tempBuff) << std::endl;

	outFile << "# Iterative Algorithm Optimization Settings" << std::endl;
	this->m_ia_ControlDlg.m_binSize.GetWindowTextW(tempBuff);
	outFile << "ia_binSize=" << _tstof(tempBuff) << std::endl;
	this->m_ia_ControlDlg.m_numBins.GetWindowTextW(tempBuff);
	outFile << "ia_binNumber=" << _tstof(tempBuff) << std::endl;
	this->m_ia_ControlDlg.m_targetRadius.GetWindowTextW(tempBuff);
	outFile << "ia_targetRadius=" << _tstof(tempBuff) << std::endl;
	this->m_ia_ControlDlg.m_phaseResolution.GetWindowTextW(tempBuff);
	outFile << "ia_phaseResolution=" << _tstof(tempBuff) << std::endl;

	// SLM Dialog settings
	outFile << "# SLM Configuration Settings" << std::endl;
	// SLM actively selected
	outFile << "slmSelect=" << this->m_slmControlDlg.slmSelectionID_ + 1 << std::endl;

	// Quick Check for SLMs
	if (this->slmCtrl != NULL) {
		// Output the LUT file paths being used for every board, if the SLM is powered or not, and if they are to be optimized
		for (int i = 0; i < this->slmCtrl->boards.size(); i++) {
			outFile << "slmLutFilePath" << std::to_string(i + 1) << "=" << this->slmCtrl->boards[i]->LUTFileName << "\n";
			outFile << "slmPowered" << std::to_string(i + 1) << "=";
			if (this->slmCtrl->boards[i]->isPoweredOn()) { outFile << "true\n"; }
			else { outFile << "false\n"; }
			outFile << "slmOptimize" << std::to_string(i + 1) << "=";
			if (this->slmCtrl->boards[i]->isToBeOptimized()) { outFile << "true\n"; }
			else { outFile << "false\n"; }
		}
	}
	outFile << "SLMselectAll=";
	if (this->m_slmControlDlg.SLM_SetALLSame_.GetCheck() == BST_CHECKED) { outFile << "true" << std::endl; }
	else { outFile << "false" << std::endl; }

	// Output Dialog
	outFile << "# Output Configuration Settings" << std::endl;
	outFile << "displayCamera=";
	if (this->m_outputControlDlg.m_displayCameraCheck.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	outFile << "displaySLM=";
	if (this->m_outputControlDlg.m_displaySLM.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	this->m_outputControlDlg.m_OutputLocationField.GetWindowTextW(tempBuff);
	std::string buffStr = CT2A(tempBuff);
	outFile << "outputFolder=" << buffStr << std::endl;

	outFile << "logAllFilesEnable=";
	if (this->m_outputControlDlg.m_logAllFilesCheck.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	outFile << "saveParameters=";
	if (this->m_outputControlDlg.m_SaveParameters.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	outFile << "saveFinalImages=";
	if (this->m_outputControlDlg.m_SaveFinalImagesCheck.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	outFile << "saveTimeVsFitness=";
	if (this->m_outputControlDlg.m_SaveTimeVFitnessCheck.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	outFile << "saveExposureShortening=";
	if (this->m_outputControlDlg.m_SaveExposureShortCheck.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	outFile << "saveEliteImage=";
	if (this->m_outputControlDlg.m_SaveEliteImagesCheck.GetCheck() == BST_CHECKED) { outFile << "true\n"; }
	else { outFile << "false\n"; }

	this->m_outputControlDlg.m_eliteSaveFreq.GetWindowTextW(tempBuff);
	outFile << "saveEliteFreq=" << _tstoi(tempBuff) << std::endl;

	// Done
	outFile.close();
	return true;
}
