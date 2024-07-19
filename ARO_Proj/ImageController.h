#ifndef IMAGE_CONTROLLER_H_
#define IMAGE_CONTROLLER_H_

#include "Utility.h"

#include "Spinnaker.h"
#include "SpinGenApi\SpinnakerGenApi.h"
using namespace Spinnaker::GenApi;

// Class to encaspsulate interactions required to accessing image data and current SDK
//		(this is so that optimization classes aren't relying on an SDK's specific behaviors)
class ImageController {
private:
	Spinnaker::ImagePtr image_; // Pointer to Image in Spinnaker SDK
	bool needRelease;
public:
	ImageController() {
		this->image_ = Spinnaker::Image::Create();
		this->needRelease = false;
	}

	// Constructor with set image to assign
	// Performs deep copy, original should be safe to release
	// Input:	setImage - imageptr to copy from
	//			release - set to true if this image when done with will need to be released from buffer with Release() call
	ImageController(Spinnaker::ImagePtr& setImage, bool release) {
		this->image_ = Spinnaker::Image::Create();
		this->image_->DeepCopy(setImage);
		this->needRelease = release;
	}

	ImageController(ImageController & other) {
		if (this->needRelease) 
			this->image_->Release();
		this->image_->~IImage();

		this->image_ = Spinnaker::Image::Create();
		this->image_->DeepCopy(other.getImage());
		this->needRelease = other.getReleaseBool();
	}

	// Desturctor - checks if need to call Release()
	~ImageController() {
		if (this->needRelease)
			this->image_->Release();
		this->image_->~IImage();
	}

	Spinnaker::ImagePtr getImage() {
		return this->image_;
	}

	const bool getReleaseBool() {
		return this->needRelease;
	}

	// Returns pointer to data associated with the image
	template <typename T>
	T * getRawData() {
		return static_cast<T*>(this->image_->GetData());
	}

	// Return width of the Image
	const int getWidth() {
		return int(this->image_->GetWidth());
	}

	// Return height of the image
	const int getHeight() {
		return int(this->image_->GetHeight());
	}

	// Use the SDK's method of saving the image
	void saveImage(std::string path) {
		this->image_->Save(path.c_str(), Spinnaker::ImageFileFormat::FROM_FILE_EXT);
	}
};

#endif
