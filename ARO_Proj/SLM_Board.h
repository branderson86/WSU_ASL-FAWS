////////////////////
// SLM_Board.h - handler for the properties of a SLM board
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#ifndef SLM_BOARD_
#define SLM_BOARD_

#include <string>

// Class to hold info for an SLM
struct SLM_Board {
	bool is_LC_Nematic;
	int imageWidth;
	int imageHeight;
	// Track if this board has been powered on or not (default start with false)
	bool powered_On;
	// If true, this board is to be optimized when running the optimization
	bool to_be_optimized_;

	int board_id; // The index (1 based) for where this board is in Blink

	// Filepath to LUT file being used
	std::string LUTFileName;

	// Defualt constructor does not initialize anything
	SLM_Board();
	// Constructor
	SLM_Board(bool isNematic, int width, int height, int boardID);
	// Getter for area (image's width*height)
	int GetArea();
	// Getter for if powered
	bool isPoweredOn();
	// Setter for if powered (should be only done within SLM_Controller!)
	void setPower(bool status);
	// Getter for if optiimized
	bool isToBeOptimized();
	// Setter for if the board is to be powered or not
	void setOptimize(bool opt);
};

#endif
