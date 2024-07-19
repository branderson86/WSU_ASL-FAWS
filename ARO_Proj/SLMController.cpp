////////////////////
// SLMController.cpp - implementation for controller that enacpsulates interactions with the SLMs
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"				// Required in source
#include "cdib.h"				// Used by blink to read in bitmaps

#include "MainDialog.h"
#include "ImageScaler.h"
#include "SLMController.h"		// Header file
#include "Utility.h"

#include <string>
#include <fstream>	// used to export information to file 

// Constructor
SLMController::SLMController() {
	unsigned int bits_per_pixel = 8U;			 //8 -> small SLM, 16 -> large SLM
	bool is_LC_Nematic = true;					 //HUGE TODO: perform this setup on evey board not just the beginning
	bool RAM_write_enable = true;
	bool use_GPU_if_available = true;
	size_t max_transiet_frames = 20U;
	const char* static_regional_lut_file = NULL; // NULL -> no overdrive, actual LUT file -> yes overdrive

	// Create the sdk that lets control the board(s)
	blink_sdk = new Blink_SDK(bits_per_pixel, &numBoards, &isBlinkSuccess, is_LC_Nematic, RAM_write_enable, use_GPU_if_available, max_transiet_frames, NULL);
	// Perform initial board info retrival and settings setup
	repopulateBoardList();

	for (int i = 0; i < this->boards.size(); i++) {
		setupSLM(i);
	}
	
	//Start with SLMs OFF
	this->setBoardPowerALL(false);
}

// Setup performed at start of controller
// Input:
//  repopulateBoardList - boolean if true will reset the board list
//	boardIDx - index for SLM (0 based)
bool SLMController::setupSLM(int boardIdx = 0) {
	//Check if board is avaliable
	if (this->boards.size() >= boardIdx || boardIdx < 0) {
		return false;
	}

	//Read default LUT File into the SLM board so it is applied to images automatically by the hardware, doesn't check for errors
	if (!AssignLUTFile(boardIdx, "")) {
		Utility::printLine("WARNING: Failure to assign default LUT file for SLM!");
		return false;
	}
	return true;
}

// Repopulate the array of boards with what is currently connected and with some default values
bool SLMController::repopulateBoardList() {
	// Clear board data
	for (int i = 0; i < this->boards.size(); i++) {
		delete this->boards[i];
	}
	this->boards.clear();

	// Go through and generate new board structs with default filenames
	for (unsigned int i = 1; i <= this->numBoards; i++) {
		SLM_Board *curBoard = new SLM_Board(true, this->blink_sdk->Get_image_width(i), this->blink_sdk->Get_image_height(i), i);

		//Add board info to board list
		this->boards.push_back(curBoard);
	}

	return true;
}

// Assign and load LUT file
// Input:
//		boardIdx - index for which board (0 based index)
//		path - string to file being loaded
//				if path is "" then defaults to "./slm3986_at532_P8.LUT"
// Output: If no errors, the SLM baord at boardIdx will be assigned the LUT file at path
//		returns true if no errors
//		returns false if an error occurs while attempting to load LUT file
bool SLMController::AssignLUTFile(int boardIdx, std::string path) {
	const char* LUT_file;
	if (path != "") {
		LUT_file = path.c_str();
		Utility::printLine("INFO: Setting LUT file path to what was provided by user input!");
	}
	else {
		LUT_file = "./slm3986_at532_P8.LUT";
		Utility::printLine("INFO: Setting LUT file path to default (given path was '')!");
	}

	//Write LUT file to the board
	try {
		if (!this->blink_sdk->Load_LUT_file(this->boards[boardIdx]->board_id, LUT_file)) {
			Utility::printLine("INFO: Failed to Load LUT file: " + std::string(LUT_file));
			return false;
		}
		this->boards[boardIdx]->LUTFileName = LUT_file;
		// Printing resulting file
		Utility::printLine("INFO: Loaded LUT file: " + std::string(LUT_file));
		return true;
	}
	catch (...) {
		Utility::printLine("ERROR: Failure to load LUT file: " + std::string(LUT_file));
		return false;
	}
}

// [DESTRUCTOR]
SLMController::~SLMController() {
	//Poweroff and deallokate sdk functionality
	blink_sdk->SLM_power(false);
	blink_sdk->~Blink_SDK();

	//De-allocate all memory allocated to store board information
	for (int i = 0; i < boards.size(); i++)
		delete boards[i];
}

// [UTILITY]

// Update the framerate for the boards according to the GUI to match camera setting
	// Returns true if no errors, false if error occurs
bool SLMController::updateFromGUI() {
	for (int i = 0; i < this->boards.size(); i++) {
		float fps;
		try	{
			unsigned short trueFrames = 3;	//3 -> non-overdrive operation, 5 -> overdrive operation (assign correct one)

			CString path("");
			this->dlg->m_cameraControlDlg.m_FramesPerSecond.GetWindowTextW(path);
			if (path.IsEmpty()) throw new std::exception();
			fps = float(_tstof(path));

			//IMPORTANT NOTE: if framerate is not the same framerate that was used in OnInitDialog AND the LC type is FLC 
			//				  then it is VERY IMPORTANT that true frames be recalculated prior to calling SetTimer such
			//				  that the FrameRate and TrueFrames are properly related
			if (!boards[i]->is_LC_Nematic) {
				trueFrames = blink_sdk->Compute_TF(float(fps));
			}
			this->blink_sdk->Set_true_frames(trueFrames);
		}
		catch (...)	{
			Utility::printLine("ERROR: Was unable to parse frame rate field for SLMs!");
			return false;
		}
	}

	return true; // no issues!
}


bool SLMController::IsAnyNematic() {
	for (int i = 0; 0 < boards.size(); i++) {
		if (boards[i]->is_LC_Nematic) {
			return true;
		}
	}
	return false;
}

bool SLMController::optimizeAny() {
	for (int i = 0; 0 < boards.size(); i++) {
		if (boards[i]->isToBeOptimized()) {
			return true;
		}
	}
	return false;
}

bool SLMController::slmCtrlReady() {
	if (blink_sdk == nullptr) {
		return false;
	}
	//TODO: add more conditions

	return true;
}

// Getter for board image width
// Input: boardIdx - index for board getting data from (0 based index)
// Output: the imageWidth property of the selected board (if invalid index value, returns -1)
int SLMController::getBoardWidth(int boardIdx) {
	if (boardIdx >= this->boards.size() || boardIdx < 0) {
		return -1;
	}
	return this->boards[boardIdx]->imageWidth;
}

// Getter for board image height
// Input: boardIdx - index for board getting data from (0 based index)
// Output: the imageHeight property of the selected board (if invalid index value, returns -1)
int SLMController::getBoardHeight(int boardIdx) {
	if (boardIdx >= this->boards.size() || boardIdx < 0) {
		return -1;
	}
	return this->boards[boardIdx]->imageHeight;
}

// Set power to all connected boards
// Input: isOn - power setting (true = on, false = off)
// Output: All SLMs available to the SDK are powered on
void SLMController::setBoardPowerALL(bool isOn) {
	if (blink_sdk != NULL) {
		blink_sdk->SLM_power(isOn);
		for (int i = 0; i < this->boards.size(); i++) {
			this->boards[i]->setPower(isOn);
		}
	}
	else {
		Utility::printLine("WARNING: SDK not avalible to power ON/OFF the boards!");
	}
}

// Set the power to a board
// Input: boardID - index of board being toggled (0 based index)
//		  isOn - power setting (true = on, false = off)
// Output: SLM is turned on/off accordingly
void SLMController::setBoardPower(int boardID, bool isOn) {
	if (blink_sdk != NULL) {
		blink_sdk->SLM_power(this->boards[boardID]->board_id, isOn);
		this->boards[boardID]->setPower(isOn);
	}
	else {
		Utility::printLine("WARNING: SDK not avalible to power ON/OFF the board!");
	}
}

// Write an image to a board
// Input:
//		slmNum - index for board to assing image to (1 based index)
//		image - pointer to array of image data to assign to board
// Output: Write image to board at slmNum, using that board's height for the image size
bool SLMController::writeImageToBoard(int slmNum, unsigned char * image) {
	if (slmNum < 1 || slmNum > this->boards.size()) {
		return false;
	}
	else {
		return this->blink_sdk->Write_image(slmNum, image, this->getBoardHeight(slmNum), false, false, 0);
	}
}
