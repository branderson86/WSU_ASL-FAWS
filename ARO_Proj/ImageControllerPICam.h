////////////////////
// ImageControllerPICam.h - Header file of ImageController with implementation using PICam SDK
// Last edited: 07/22/2021 by Andrew O'Kins
////////////////////

#ifndef IMAGE_CONTROLLER_PICAM_H_
#define IMAGE_CONTROLLER_PICAM_H_

#ifdef USE_PICAM // Only include implementation if building with PICam

#include <opencv2\core\core.hpp> // Using OpenCV to save image info
#include <opencv2\highgui\highgui.hpp>

// Note / TODO: Consider how to handle issue of image size received being larger (16 bit versus 8 bit)

class ImageController {
private:
	unsigned char * data_; // Raw data of the image - assuming that the image is 8 bit monochrome format (each element in array is a pixel)
	int width_;			   // Width of the image in pixels
	int height_;		   // Height of the image in pixels
	int size_;			   // Total size of the image in bytes (which should be with current format equal to width*height)
public:
	ImageController() {
		this->data_ = nullptr;
		this->size_ = 0;
	}

	// Constructor with set image to assign
	// Performs deep copy, original should be safe to release
	// Input:	rawData - pointer to image data (integer format, so needs to be compressed)
	//			size - number of elements in rawData
	//		    width - width of the image in pixels
	//			height - height of the image in piels
	ImageController(unsigned short * rawData, int size, int width, int height) {
		this->size_ = size;
		this->width_ = width;
		this->height_ = height;

		this->data_ = new unsigned char[size];
		for (int index = 0; index < size; index++) {
			// Attempting a kind of compression to convert short size value to byte size (1/2 the size) by dividing it down so max is reduced to 255 and so on.
			// Note that this not at all a lossless compression, but should work to having data comparable to Spinnaker's
			unsigned char byteData = unsigned char(rawData[index] / 257);
			this->data_[index] = byteData;
		}
	}

	// Copy constructor
	ImageController(ImageController & other) {
		this->width_ = other.getWidth();
		this->height_ = other.getHeight();
		this->size_ = other.getSize();
		unsigned char * otherData = other.getRawData();

		this->data_ = new unsigned char[this->size_];
		for (int index = 0; index < this->size_; index++) {
			this->data_[index] = otherData[index];
		}
	}

	// Desturctor
	~ImageController() {
		delete[] this->data_;
	}

	// Getter for release (used in copy constructor)
	const int getSize() {
		return this->size_;
	}

	// Returns pointer to data associated with the image
	unsigned char * getRawData() {
		return this->data_;
	}

	// Return width of the Image
	const int getWidth() {
		return this->width_;
	}

	// Return height of the image
	const int getHeight() {
		return this->height_;
	}

	// Output the image with given file path
	void saveImage(std::string path) {
		// PICam does not offer it's own method of saving images, so using OpenCV's
		cv::imwrite(path, cv::Mat(this->height_, this->width_, CV_8UC1, this->data_));
	}
};

#endif // inclusion for only PICam version

#endif // file inclusion
