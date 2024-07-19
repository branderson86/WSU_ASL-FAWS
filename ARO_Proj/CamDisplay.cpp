#include "stdafx.h"
#include "CamDisplay.h"
#include "Utility.h"

// Constructor
// Display has twice the dimensions of inputted image height and width
CameraDisplay::CameraDisplay(int input_image_height, int input_image_width, std::string display_name) :	//ASK why twice larger?
							port_height_(input_image_height), port_width_(input_image_width), display_name_(display_name) {
	Utility::printLine("INFO: creating display " + display_name + " with - (" + std::to_string(input_image_width) + ", " + std::to_string(input_image_height) + ")");
	display_matrix_ = cv::Mat(port_height_, port_width_, CV_8UC3);
	_isOpened = false;
}

// If display is open, window is destroyed
CameraDisplay::~CameraDisplay() {
	if (_isOpened) {
		cv::destroyWindow(display_name_);
	}
}

void CameraDisplay::OpenDisplay() {
	if (!_isOpened)	{
		cv::namedWindow(display_name_, CV_WINDOW_AUTOSIZE); //Create a window for the display
		imshow(display_name_, display_matrix_);
		cv::waitKey(1);
		_isOpened = true;
		Utility::printLine("INFO: a display has been opened with name - " + display_name_);
	}
}

void CameraDisplay::CloseDisplay() {
	if (_isOpened) {
		cv::destroyWindow(display_name_);
		_isOpened = false;
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

	if (!_isOpened)	{
		OpenDisplay();
	}
	else {
		cv::imshow(this->display_name_, this->display_matrix_);
		cv::waitKey(1);
	}
}
