////////////////////
// CameraDisplay.cpp - implementation of CameraDisplay class
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"
#include "CameraDisplay.h"
#include "Utility.h"

// Constructor
// Display has twice the dimensions of inputted image height and width
CameraDisplay::CameraDisplay(int input_image_height, int input_image_width, std::string display_name) :
							port_height_(input_image_height), port_width_(input_image_width), display_name_(display_name) {
	Utility::printLine("INFO: creating display " + display_name + " with - (" + std::to_string(input_image_width) + ", " + std::to_string(input_image_height) + ")");
	display_matrix_ = cv::Mat(port_height_, port_width_, CV_8UC3);
	_isOpened = false;
}

// If display is open, window is destroyed
CameraDisplay::~CameraDisplay() {
	if (this->_isOpened) {
		cv::destroyWindow(this->display_name_);
	}
}

void CameraDisplay::OpenDisplay(int window_width, int window_height) {
	if (!this->_isOpened)	{
		cv::namedWindow(this->display_name_, CV_WINDOW_NORMAL); //Create a window for the display
		cv::resizeWindow(this->display_name_, window_width, window_height); // Setting window to be statically 240x240
		this->_isOpened = true;
		imshow(this->display_name_, this->display_matrix_); // Setting window with current display content
		cv::waitKey(1);
		Utility::printLine("INFO: a display has been opened with name - " + this->display_name_);
	}
}

void CameraDisplay::resizeDisplay(int new_width, int new_height) {
	if (this->_isOpened) { // Only resize if the window has been opened
		cv::resizeWindow(this->display_name_, new_width, new_height);
	}
}

void CameraDisplay::CloseDisplay() {
	if (this->_isOpened) {
		cv::destroyWindow(this->display_name_);
		this->_isOpened = false;
	}
}

// Update the display contents, if display not open it is also opened
// Input: image - pointer to data that is passed into display_matrix_
void CameraDisplay::UpdateDisplay(unsigned char* image) {
	// Update the display matrix data, assuming that the inputted image is monochrome while display_matrix contains RGB (1 value input with 3 value location)
	for (int y = 0; y < this->port_height_; y++) {
		for (int x = 0; x < this->port_width_; x++) {
			int i = (y*this->port_width_ + x)*3; // Multiply by 3 to offset to appropriate pixel data location
			(this->display_matrix_.data[i]) = image[i/3];	// R
			(this->display_matrix_.data[i+1]) = image[i/3]; // G
			(this->display_matrix_.data[i+2]) = image[i/3]; // B
		}
	}
	if (!this->_isOpened)	{
		OpenDisplay(240,240);
	}
	cv::imshow(this->display_name_, this->display_matrix_);
	cv::waitKey(1);
}
