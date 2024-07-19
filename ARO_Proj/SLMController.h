////////////////////
// SLMController.h - header for controller that enacpsulates interactions with the SLMs
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#ifndef SLM_CONTROLLER_H_
#define SLM_CONTROLLER_H_

#include "SLM_Board.h"
#include "Blink_SDK.h"

#include <vector>

class MainDialog;

// Class to encapsulate interactions with SLM boards
class SLMController {
private:
	//UI Reference
	MainDialog* dlg;
public:
	//Board control
	Blink_SDK* blink_sdk;			//Library that controls the SLMs
	bool isBlinkSuccess = true;		//TRUE -> if SLM control wrapper was constructed correctly
	//Board parameters
	unsigned int numBoards = 0;		//Number of boards populated after creation of the SDK

	//Board references
	std::vector<SLM_Board*> boards;
	// Constructor
	SLMController();
	// Destructor
	~SLMController();

	// Setup performed at start of controller
	// Input:
	//  repopulateBoardList - boolean if true will reset the board list
	//	boardIDx - index for SLM (0 based)
	bool setupSLM(int boardIdx);

	// Repopulate the array of boards with what is currently connected and with some default values
	bool repopulateBoardList();

	bool IsAnyNematic();
	// Return true if at least one board is set to be optimized
	bool optimizeAny();

	// Update the framerate for the boards according to the GUI to match camera setting
	// Returns true if no errors, false if error occurs
	bool updateFromGUI();

	// Assign and load LUT file
	// Input:
	//		boardIdx - index for which board (0 based index)
	//		path - string to file being loaded
	//				if path is "" then defaults to "./slm3986_at532_P8.LUT"
	// Output: If no errors, the SLM baord at boardIdx will be assigned the LUT file at path
	//		returns true if no errors
	//		returns false if an error occurs while attempting to load LUT file
	bool AssignLUTFile(int boardIdx, std::string);

	bool slmCtrlReady();

	int getBoardWidth(int boardIdx);
	int getBoardHeight(int boardIdx);
	void setBoardPowerALL(bool isOn);
	// Set power to a specific board
	// Input:
	//		boardID - index for which board (0 based index)
	//		isOn - boolean for if on or off toggle
	void setBoardPower(int boardID, bool isOn);

	void SetMainDlg(MainDialog* dlg_) { dlg = dlg_; }

	// Write an image to a board
	// Input:
	//		slmNum - index for which board (0 based index)
	//		image - pointer to array of image to assign to board
	// Output: Write image to board at slmNum, using that board's height for the image size
	bool writeImageToBoard(int slmNum, unsigned char * image);
};

#endif
