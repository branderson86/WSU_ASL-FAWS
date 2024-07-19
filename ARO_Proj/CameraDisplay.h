////////////////////
// CameraDisplay header - class to wrap the process of displaying image data to the user
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#ifndef GRAPHICS_DISPLAY_H_
#define GRAPHICS_DISPLAY_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <string>

class CameraDisplay {
private:
	// Dimensions of the display window contents
	int port_height_;
	int port_width_;
	// Matrix to display
	cv::Mat display_matrix_;
	// Window name (also used in controlling the appropriate window)
	std::string display_name_;
	// true if the window has been created
	bool _isOpened;
public:
	// Constructor
	// Display has twice the dimensions of inputted image height and width
	CameraDisplay(int input_image_height, int input_image_width, std::string display_name);

	// If display is open, window is destroyed
	~CameraDisplay();

	// Open the window display with set window dimensions (default 240x240)
	void OpenDisplay(int window_width = 240, int window_height = 240);
	// Resize the display with set window dimensions
	void resizeDisplay(int new_width, int new_height);
	// Close the window display
	void CloseDisplay();

	// Update the display contents, if display not open it is also opened
	// Input: image - pointer to data that is passed into display_matrix_
	void UpdateDisplay(unsigned char* image);
};

#endif
