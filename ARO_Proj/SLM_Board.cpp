// [DESCRIPTION]
// Implementation file for the SLM_Board class

#include "stdafx.h"
#include "SLM_Board.h"
#include "Utility.h"

SLM_Board::SLM_Board() {};

// Constructor
SLM_Board::SLM_Board(bool isNematic, int width, int height, int boardID) {
	this->is_LC_Nematic = isNematic;
	this->imageWidth = width;
	this->imageHeight = height;
	this->powered_On = false;
	this->board_id = boardID;
	this->to_be_optimized_ = false;

	int boardArea = width * height;
	//Build paths to the calibrations for this SLM -- regional LUT included in Blink_SDK(), but need to pass NULL to that param to disable ODP. Might need to make a class.
	this->LUTFileName = "linear.LUT";
}

// Return area of board image (width*height)
int SLM_Board::GetArea() {
	return this->imageWidth * this->imageHeight;
}

bool SLM_Board::isPoweredOn() {
	return this->powered_On;
}

void SLM_Board::setPower(bool status) {
	this->powered_On = status;
}

void SLM_Board::setOptimize(bool optimized) {
	this->to_be_optimized_ = optimized;
	if (this->to_be_optimized_) {
		Utility::printLine("INFO: Board #" + std::to_string(this->board_id) + " is set to be optimized!");
	}
	else {
		Utility::printLine("INFO: Board #" + std::to_string(this->board_id) + " is set to NOT be optimized!");
	}
}

bool SLM_Board::isToBeOptimized() {
	return this->to_be_optimized_;
}
