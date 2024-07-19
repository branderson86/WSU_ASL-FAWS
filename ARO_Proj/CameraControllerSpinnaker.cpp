////////////////////
// CameraControllerSpinnaker.cpp - implementation of CameraController using Spinnaker SDK
// Last edited: 07/21/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"				// Required in source
#include "CameraController.h"
#include "MainDialog.h"
#include "Utility.h"

#ifdef USE_SPINNAKER // Only include this implementation if using Spinnaker

// [CONSTRUCTOR(S)]
CameraController::CameraController(MainDialog* dlg_) {
	//Camera access
	this->dlg = dlg_;
	UpdateConnectedCameraInfo();
}

//[DESTRUCTOR]
CameraController::~CameraController() {
	Utility::printLine("INFO: Beginning to shutdown camera!");
	if (this->isCamCreated) {
		//stopCamera();
		shutdownCamera();
	}
	Utility::printLine("INFO: Finished shutting down camera!");
}

// [CAMERA CONTROL]
// Connect to camera (if not already) and pull info from GUI to configure the camera
bool CameraController::setupCamera() {
	// Connect to the camera if the cam pointer is null
	if (cam == NULL) {
		UpdateConnectedCameraInfo();
	}
	// Quit if don't have a reference to UI (latest parameters)
	if (!dlg) {
		return false;
	}
	// Update image parameters according to dialog inputs
	if (!UpdateImageParameters()) {
		return false;
	}
	// Check for any possible issue with camera and restart if needed
	if (!cam->IsValid() || !cam->IsInitialized()) {
		shutdownCamera();
		if (!UpdateConnectedCameraInfo()) {
			return false;
		}
	}
	
	if (!ConfigureCustomImageSettings()) {
		return false;
	}
	
	if (!ConfigureExposureTime()) {
		return false;
	}

	isCamCreated = true;
	return true;
}

bool CameraController::startCamera() {
	try	{
		INodeMap &nodeMap = cam->GetNodeMap();
		INodeMap &TLnodeMap = cam->GetTLStreamNodeMap(); // For managing Transport Layer Stream nodes (Buffer Handler for example)
		// Set acquisition mode to singleframe
		// - retrieve enumerationg node to set
		CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode"); 
		if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode)) {
			Utility::printLine("Unable to set acquisition mode (enum retrieval).");
			return false;
		}
		// - retrieve "continuous" entry node from enumeration node
		CEnumEntryPtr ptrAcquisitionModeType = ptrAcquisitionMode->GetEntryByName("Continuous");
		if (!IsAvailable(ptrAcquisitionModeType) || !IsReadable(ptrAcquisitionModeType)) {
			Utility::printLine("Unable to set acquisition mode (entry retrieval).");
			return false;
		}
		// - retrieve integer value from entry node "continuous"
		int64_t acquisitionModeTypeValue = ptrAcquisitionModeType->GetValue();
		// - set the retrieved integer as the correct node value
		ptrAcquisitionMode->SetIntValue(acquisitionModeTypeValue);

		// Setting buffer handler
		//		Spinnaker defaults to OldestFirst, changed to NewestOnly as we are only interested in current image for individual being run
		CEnumerationPtr ptrSBufferHandler = TLnodeMap.GetNode("StreamBufferHandlingMode");
		if (!IsAvailable(ptrSBufferHandler) || !IsWritable(ptrSBufferHandler)) {
			Utility::printLine("ERROR: Unable to set buffer handler mode (node retrieval).");
			return false;
		}
		else {
			ptrSBufferHandler->SetIntValue(Spinnaker::StreamBufferHandlingMode_NewestOnly);
		}
		//Begin Aquisition
		cam->BeginAcquisition();
		Utility::printLine("INFO: Successfully began acquiring images!");
	}
	catch (Spinnaker::Exception &e)	{
		Utility::printLine("ERROR: Camera could not start - /n" + std::string(e.what()));
		return false;
	}
	return true; // no errors!
}

bool CameraController::stopCamera() {
	cam->EndAcquisition();
	return true;
}

//Releases camera references - have to call setup camera again if need to use camer after this call
bool CameraController::shutdownCamera() {
	//release camera
	cam->DeInit();
	//release system
	cam = NULL;
	camList.Clear();
	system->ReleaseInstance();
	isCamCreated = false;
	return true;
}

// Save an image with template file name
// Input: curImage - image pointer to save
//		  path - string to where and name of the image is to be saved
// Output: curImage is saved at path
bool CameraController::saveImage(ImageController * curImage, std::string path) {
	if (curImage == NULL) {
		Utility::printLine("ERROR: save image given is invalid!");
		return false;
	}
	else {
		try {
			curImage->saveImage(path);
		}
		catch (Spinnaker::Exception &e) {
			Utility::printLine("ERROR: "+ std::string(e.what()));
		}
	}
	return true;
}

//AcquireImages: get one image from the camera
ImageController * CameraController::AcquireImage() {
	try {
		// Retrieve next received image
		Spinnaker::ImagePtr curImage = cam->GetNextImage();

		// Ensure image completion
		if (curImage->IsIncomplete()) {
			//TODO: implement proper handling of incomplete images (retake of image)
			Utility::printLine("ERROR: Image incomplete: " + std::string(Spinnaker::Image::GetImageStatusDescription(curImage->GetImageStatus())));
		}
		
		// Assign the converted output image to outImage, since this is a converted image we don't need to release it
			// Resource reference says so in example conversion to mono 8 -> http://softwareservices.flir.com/Spinnaker/latest/_acquisition_8cpp-example.html
		ImageController* outImage = new ImageController(curImage->Convert(Spinnaker::PixelFormat_Mono8), false);
		// Release from the buffer
		curImage->Release();

		return outImage;
	}
	catch (Spinnaker::Exception &e) {
		Utility::printLine("ERROR: " + std::string(e.what()));
		return NULL;
	}
}

// [CAMERA SETUP]
// Pull camera settings from CameraControlDialog and AOIControlDialog
bool CameraController::UpdateImageParameters() {
	bool result = true;
	// Frames per second
	try	{
		CString path("");
		dlg->m_cameraControlDlg.m_FramesPerSecond.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		fps = _tstoi(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse the frames per second time input field!");
		result = false;
	}
	// Gamma value
	try	{
		CString path("");
		dlg->m_cameraControlDlg.m_gammaValue.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		gamma = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse the gamma input field!");
		result = false;
	}
	// Initial exposure time
	try	{
		CString path("");
		dlg->m_cameraControlDlg.m_initialExposureTimeInput.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		initialExposureTime = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse the initial exposure time input field!");
		result = false;
	}
	// Get all AOI settings
	try	{
		CString path("");
		// Left offset
		dlg->m_aoiControlDlg.m_leftInput.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		x0 = _tstoi(path);
		path = L"";
		// Top Offset
		dlg->m_aoiControlDlg.m_rightInput.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		y0 = _tstoi(path);
		path = L"";
		// Width of AOI
		dlg->m_aoiControlDlg.m_widthInput.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		cameraImageWidth = _tstoi(path);
		path = L"";
		// Hieght of AOI
		dlg->m_aoiControlDlg.m_heightInput.GetWindowTextW(path);
		if (path.IsEmpty()) throw new std::exception();
		cameraImageHeight = _tstoi(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse AOI settings!");
		result = false;
	}

	// Number of image bins X and Y (ASK: if actually need to be thesame)
	try	{
		CString path("");
		if (this->dlg->opt_selection_ == MainDialog::OptType::IA) {
			dlg->m_ia_ControlDlg.m_numBins.GetWindowTextW(path);
		}
		else {
			dlg->m_ga_ControlDlg.m_numberBins.GetWindowTextW(path);
		}
		if (path.IsEmpty()) throw new std::exception();
		numberOfBinsX = _tstoi(path);
		numberOfBinsY = numberOfBinsX; // Number of bins in Y direction is equal to in X direction (square)
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse the number of bins input field!");
		result = false;
	}
	//Size of bins X and Y (ASK: if actually thesame xy? and isn't stating the # of bins already determine size?)
	try	{
		CString path("");
		if (this->dlg->opt_selection_ == MainDialog::OptType::IA) {
			dlg->m_ia_ControlDlg.m_binSize.GetWindowTextW(path);
		}
		else {
			dlg->m_ga_ControlDlg.m_binSize.GetWindowTextW(path);
		}
		if (path.IsEmpty()) throw new std::exception();
		binSizeX = _tstoi(path);
		binSizeY = binSizeX; // Square shape in size
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse bin size input field!");
		result = false;
	}
	// Integration/target radius
	try	{
		CString path("");
		if (this->dlg->opt_selection_ == MainDialog::OptType::IA) {
			dlg->m_ia_ControlDlg.m_targetRadius.GetWindowTextW(path);
		}
		else {
			dlg->m_ga_ControlDlg.m_targetRadius.GetWindowTextW(path);
		}
		if (path.IsEmpty()) throw new std::exception();
		targetRadius = _tstoi(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Was unable to parse integration radius input field!");
		result = false;
	}

	return result;
}

//GetConnectedCameraInfo: used to proccess/store data about all currently connected cameras
bool CameraController::UpdateConnectedCameraInfo() {
	try	{
		//Spinaker system object w/ camera list
		system = Spinnaker::System::GetInstance();
		this->camList = system->GetCameras();

		if (system == NULL)	{
			Utility::printLine("ERROR: Camera system not avaliable!");
			return false;
		}

		int camAmount = this->camList.GetSize();
		if (camAmount <= 0)	{
			Utility::printLine("ERROR: No cameras avaliable!");
			//clear camera list before releasing system
			camList.Clear();
			system->ReleaseInstance();
			return false;
		}
		else {
			Utility::printLine("INFO: There are " + std::to_string(camAmount) + " camera(s) avaliable! Using camera at index 0!");
		}

		//Get camera reference
		this->cam = camList.GetByIndex(0);
		if (!cam.IsValid())	{
			Utility::printLine("ERROR: Retrieved Camera not Valid!");
			return false;
		}
		else {
			Utility::printLine("INFO: Retrieved Camera is Valid!");
		}
		//Initialize camera
		try {
			cam->Init();
		}
		catch (Spinnaker::Exception &e) {
			Utility::printLine("ERROR: Spinnaker failure in initialization - " + std::string(e.what()));
			return false;
		}
		// Checking to make sure it was initialized
		if (!cam->IsInitialized())	{
			Utility::printLine("ERROR: Retrieved Camera could not be initialized!");
			return false;
		}
		else {
			Utility::printLine("INFO: Retrieved Camera was initialized!");
		}
	}
	catch (Spinnaker::Exception &e)	{
		Utility::printLine("ERROR: " + std::string(e.what()));
		return false;
	}

	return true;
}

/* ConfigureCustomImageSettings()
 * @return - TRUE if succesful, FALSE if failed */
bool CameraController::ConfigureCustomImageSettings() {
	// REFERENCES: try to look through these if current implementation one malfunctions:
	// 1) http://perk-software.cs.queensu.ca/plus/doc/nightly/dev/vtkPlusSpinnakerVideoSource_8cxx_source.html
	// 2) Useful for getting node info: https://www.flir.com/support-center/iis/machine-vision/application-note/spinnaker-nodes/
	// 3) Use SpinView program to find correct node names and value types 
	
	//Check current camera is valid
	if (!cam->IsValid() || !cam->IsInitialized()) {
		Utility::printLine("ERROR: trying to configure invalid or nonexistant camera");
		return false;
	}
	
	//XY factor check (axes offsets need to be certain factor to be valid see above)
	if (x0 % 4 != 0)
		x0 -= x0 % 4;
	if (y0 % 2 != 0)
		y0 -= y0 % 2;

	try	{
		//Apply mono 8 pixel format
		if (cam->PixelFormat != NULL && cam->PixelFormat.GetAccessMode() == RW)
			cam->PixelFormat.SetValue(Spinnaker::PixelFormat_Mono8);
		else
			Utility::printLine("ERROR: Pixel format not available...");

		//Apply initial zero offset in x direction (needed to minimize AOI errors)
		if (cam->OffsetX != NULL && cam->OffsetX.GetAccessMode() == RW) {
			cam->OffsetX.SetValue(0);
		}
		else {
			Utility::printLine("ERROR: OffsetX not available for initial setup");
		}
		//Apply initial zero offset in y direction (needed to minimize AOI errors)
		if (cam->OffsetY != NULL && cam->OffsetY.GetAccessMode() == RW) {
			cam->OffsetY.SetValue(0);
		}
		else {
			Utility::printLine("ERROR: OffsetY not available for initial setup");
		}
		//Apply target image width
		if (cam->Width != NULL && cam->Width.GetAccessMode() == RW  && cam->Width.GetInc() != 0 && cam->Width.GetMax() != 0) {
			cam->Width.SetValue(cameraImageWidth);
		}
		else {
			Utility::printLine("ERROR: Width not available to be set");
		}
		//Apply target image height
		if (cam->Height != NULL && cam->Height.GetAccessMode() == RW && cam->Height.GetInc() != 0 && cam->Height.GetMax() != 0) {
			cam->Height.SetValue(cameraImageHeight);
		}
		else {
			Utility::printLine("ERROR: Height not available");
		}
		//Apply final offset in x direction
		if (cam->OffsetX != NULL && cam->OffsetX.GetAccessMode() == RW) {
			cam->OffsetX.SetValue(x0);
		}
		else {
			Utility::printLine("ERROR: Final OffsetX not available");
		}
		//Apply final offset in y direction
		if (cam->OffsetY != NULL && cam->OffsetY.GetAccessMode() == RW) {
			cam->OffsetY.SetValue(y0);
		}
		else {
			Utility::printLine("ERROR: OffsetY not available");
		}
		INodeMap & nodeMap = cam->GetNodeMap();
		
		//Enable Manual Frame Rate Setting
		bool setFrameRate = true;
		Spinnaker::GenApi::CBooleanPtr FrameRateEnablePtr = cam->GetNodeMap().GetNode("AcquisitionFrameRateEnabled");
		if (Spinnaker::GenApi::IsAvailable(FrameRateEnablePtr) && Spinnaker::GenApi::IsWritable(FrameRateEnablePtr)) {
			FrameRateEnablePtr->SetValue(true);
			Utility::printLine("INFO: Set Framerate Manual Enble to True!");
		}
		else {
			Utility::printLine("ERROR: Unable to set Frame Rate Enable to True!");
			setFrameRate = false;
		}

		// Disable Auto Frame Rate Control
		if (setFrameRate) {
			// Enumeration node
			CEnumerationPtr ptrFrameAuto = cam->GetNodeMap().GetNode("AcquisitionFrameRateAuto");
			if (Spinnaker::GenApi::IsAvailable(ptrFrameAuto) && Spinnaker::GenApi::IsWritable(ptrFrameAuto))
			{
				//EnumEntry node (always associated with an Enumeration node)
				CEnumEntryPtr ptrFrameAutoOff = ptrFrameAuto->GetEntryByName("Off");
				//Turn off Auto Gain
				ptrFrameAuto->SetIntValue(ptrFrameAutoOff->GetValue());
				if (ptrFrameAuto->GetIntValue() == ptrFrameAutoOff->GetValue())	{
					Utility::printLine("INFO: Set auto acquisition framerate mode to off!");
				}
				else {
					Utility::printLine("WARNING: Auto acquistion framerate mode was not set to 'off'!");
					setFrameRate = false;
				}
			}
			else {
				Utility::printLine("ERROR: Unable to set auto acquisition framerate mode to 'off'!");
				setFrameRate = false;
			}
		}

		//Set the correct framerate given by the user thrugh the UI
		if (setFrameRate) {
			CFloatPtr ptrFrameRateSetting = cam->GetNodeMap().GetNode("AcquisitionFrameRate");
			if (Spinnaker::GenApi::IsAvailable(ptrFrameRateSetting) && Spinnaker::GenApi::IsWritable(ptrFrameRateSetting))	{
				ptrFrameRateSetting->SetValue(fps);
				if (ptrFrameRateSetting->GetValue() == fps)
					Utility::printLine("INFO: Set FPS of camera to " + std::to_string(fps) + "!");
				else
					Utility::printLine("WARNING: Was unable to set the FPS of camera to " + std::to_string(fps) + " it is actually " + std::to_string(ptrFrameRateSetting->GetValue()) + "!");
			}
			else {
				Utility::printLine("ERROR: Unable to set fps node not avaliable!");
			}
		}

		//Enable manual gamma adjustment:
		Spinnaker::GenApi::CBooleanPtr GammaEnablePtr = cam->GetNodeMap().GetNode("GammaEnabled");
		if (Spinnaker::GenApi::IsAvailable(GammaEnablePtr) && Spinnaker::GenApi::IsWritable(GammaEnablePtr)) {
			GammaEnablePtr->SetValue(true);
			Utility::printLine("INFO: Set manual gamma enable to true");

			CFloatPtr ptrGammaSetting = cam->GetNodeMap().GetNode("Gamma");
			if (Spinnaker::GenApi::IsAvailable(ptrGammaSetting) && Spinnaker::GenApi::IsWritable(ptrGammaSetting))	{
				ptrGammaSetting->SetValue(gamma);
				if (ptrGammaSetting->GetValue() == gamma)
					Utility::printLine("INFO: Set Gamma of camera to " + std::to_string(gamma) + "!");
				else
					Utility::printLine("WARNING: Was unable to set the Gamma of camera to " + std::to_string(gamma) + " it is actually " + std::to_string(ptrGammaSetting->GetValue()) + "!");
			}
		}
		else {
			Utility::printLine("ERROR: Unable to set manual gamma enable to true");
		}

		if (PrintDeviceInfo() == -1) {
			Utility::printLine("WARNING: Couldn't display camera information!");
		}
	}
	catch (Spinnaker::Exception &e)	{
		Utility::printLine("ERROR: Exception while setting camera options \n" + std::string(e.what()));
		return false;
	}

	return true;
}

//REFERENCE: From Spinaker Documentation Aqciusition.cpp Example
// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.
int CameraController::PrintDeviceInfo() {
	Utility::printLine();
	Utility::printLine("*** CAMERA INFORMATION ***");
	try	{
		INodeMap &nodeMap = cam->GetNodeMap();
		INodeMap & nodeMapTLDevice = cam->GetTLDeviceNodeMap();
		FeatureList_t features;
		CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
		if (IsAvailable(category) && IsReadable(category)) {
			category->GetFeatures(features);
			FeatureList_t::const_iterator it;
			for (it = features.begin(); it != features.end(); ++it)	{
				CNodePtr pfeatureNode = *it;
				std::string name(pfeatureNode->GetName().c_str());
				Utility::printLine(name + " : ");

				CValuePtr pValue = (CValuePtr)pfeatureNode;
				if (IsReadable(pValue))	{
					std::string value(pValue->ToString().c_str());
					Utility::printLine(value);
				}
				else
					Utility::printLine("Node not readable");
			}
		}
		else {
			Utility::printLine("Device control information not available.");
		}
	}
	catch (Spinnaker::Exception &e) {
		Utility::printLine("ERROR: " + std::string(e.what()));
		Utility::printLine();
		return -1;
	}

	Utility::printLine();
	return 0;
}

/* ConfigureExposureTime: resets the cameras exposure time
 * @return - TRUE if success, FALSE if failed */
bool CameraController::ConfigureExposureTime() {
	finalExposureTime = initialExposureTime;
	
	return SetExposure(finalExposureTime);
}

bool CameraController::hasCameras() {
	if (this->cam->IsValid() != NULL) {
		return true;
	}
	else {
		return false;
	}
}

// [UTILITY]
// GetExposureRatio: calculates how many times exposure was cut in half
// @returns - the ratio of starting and final exosure time 
double CameraController::GetExposureRatio() {
	return initialExposureTime / finalExposureTime;
}

/* SetExposure: configure a custom exposure time. Automatic exposure is turned off, then the custom setting is applied.
* @param exposureTimeToSet - self explanatory (in microseconds = 10^-6 seconds)
* @return FALSE if failed, TRUE if succeded */
bool CameraController::SetExposure(double exposureTimeToSet) {
	//Constraint exposure time from going lower than camera limit
	//TODO: determine this lower bound for the camera we are using

	try {
		INodeMap &nodeMap = cam->GetNodeMap();

		// Turn off automatic exposure mode
		CEnumerationPtr ptrExposureAuto = nodeMap.GetNode("ExposureAuto");
		if (!IsAvailable(ptrExposureAuto) || !IsWritable(ptrExposureAuto)) {
			Utility::printLine("Unable to disable automatic exposure (node retrieval)");
			return false;
		}
		CEnumEntryPtr ptrExposureAutoOff = ptrExposureAuto->GetEntryByName("Off");
		if (!IsAvailable(ptrExposureAutoOff) || !IsReadable(ptrExposureAutoOff)) {
			Utility::printLine("Unable to disable automatic exposure (enum entry retrieval)");
			return false;
		}
		ptrExposureAuto->SetIntValue(ptrExposureAutoOff->GetValue());

		// Set exposure manually
		CFloatPtr ptrExposureTime = nodeMap.GetNode("ExposureTime");
		if (!IsAvailable(ptrExposureTime) || !IsWritable(ptrExposureTime)) {
			Utility::printLine("ERROR: Unable to set exposure time.");
			return false;
		}
		// Ensure new time does not exceed max set to max if does
		const double exposureTimeMax = ptrExposureTime->GetMax();
		if (exposureTimeToSet > exposureTimeMax) {
			exposureTimeToSet = exposureTimeMax;
			Utility::printLine("WARNING: Exposure time of " + std::to_string(exposureTimeToSet) + " is to big. Exposure set too max of " + std::to_string(exposureTimeMax));
		}
		ptrExposureTime->SetValue(exposureTimeToSet);
	}
	catch (Spinnaker::Exception &e)	{
		Utility::printLine("ERROR: Cannot Set Exposure Time:\n" + std::string(e.what()));
		return false;
	}

	return true;
}

void CameraController::HalfExposureTime() {
	finalExposureTime /= 2;
	if (!SetExposure(finalExposureTime))
		Utility::printLine("ERROR: wasn't able to half the exposure time!");
}

// [ACCESSOR(S)/MUTATOR(S)]
bool CameraController::GetCenter(int &x, int &y) {
	int fullWidth = -1;
	int fullHeight = -1;
	
	if (!GetFullImage(fullWidth, fullHeight))
	{
		Utility::printLine("ERROR: failed to get max dimensions for getting center values");
		return false;
	}

	x = fullWidth / 2;
	y = fullHeight / 2;
	return true;
}

bool CameraController::GetFullImage(int &x, int &y) {
	if (cam->IsValid() && cam->IsInitialized()) {
		CIntegerPtr ptrWidth = cam->GetNodeMap().GetNode("WidthMax");
		x = int(ptrWidth->GetValue());

		CIntegerPtr ptrHeight = cam->GetNodeMap().GetNode("HeightMax");
		y = int(ptrHeight->GetValue());
	}
	else {
		Utility::printLine("ERROR: camera that was retrived for gathering info is not valid!");
		return false;
	}

	return true;
}

#endif
