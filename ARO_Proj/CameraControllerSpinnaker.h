////////////////////
// CameraControllerSpinnaker.h - Header file for the camera controller to Spinnaker version
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#ifndef CAMERA_CONTROLLER_SPINNAKER_H_
#define CAMERA_CONTROLLER_SPINNAKER_H_

#ifdef USE_SPINNAKER

#include <string>

#include "Spinnaker.h"
#include "SpinGenApi\SpinnakerGenApi.h"
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

#include "ImageControllerSpinnaker.h"	// For wrapping the input/output of image data

class MainDialog;

class CameraController {
public:
	//Image parameters (with defaults set)
	int x0 = 896;				//  Must be a factor of 4 (like 752)
	int y0 = 568;				//	Must be a factor of 2 (like 752)
	int cameraImageWidth = 64;	//	Must be a factor of 32 (like 64)
	int cameraImageHeight = 64;	//	Must be a factor of 2  (like 64)
	int populationDensity = 1;

	double gamma = 1.25;
	int fps = 200;
	// initial exposure time in microseconds that is set by GUI (us)
	double initialExposureTime = 2000;
	// currnet exposure time in microseconds (us)
	double finalExposureTime = 2000;

	int numberOfBinsX = 128;
	int numberOfBinsY = 128;
	int binSizeX = 4;
	int binSizeY = 4;

	//Image target settings
	int targetRadius = 5;

private:
	//UI/Equipment reference
	MainDialog* dlg;
	//Camera access using Spinnaker
	Spinnaker::SystemPtr system;
	Spinnaker::CameraList camList;
	Spinnaker::CameraPtr cam;

	//Logic control
	bool isCamCreated = false;
public:

	CameraController(MainDialog* dlg_);
	~CameraController();

	bool setupCamera();
	bool startCamera();
	bool saveImage(ImageController * curImage, std::string path);
	ImageController* AcquireImage();
	bool stopCamera();
	bool shutdownCamera();

	// [SETUP]
	bool UpdateImageParameters();
	bool UpdateConnectedCameraInfo();
	bool ConfigureCustomImageSettings();
	bool ConfigureExposureTime();

	// [UTILITY]
	int PrintDeviceInfo();
	// Return true if this controller has access to at least one camera
	bool hasCameras(); 

	bool SetExposure(double exposureTimeToSet);
	double GetExposureRatio();
	void HalfExposureTime();

	// [ACCESSOR(S)/MUTATOR(S)]
	bool GetCenter(int &x, int &y);
	bool GetFullImage(int &x, int &y);

};

#endif

#endif
