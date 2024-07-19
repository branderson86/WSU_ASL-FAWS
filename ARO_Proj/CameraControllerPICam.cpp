////////////////////
// CameraControllerPICam.cpp - implementation of CameraController using PICam SDK
// Last edited: 08/13/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"				// Required in source
#include "CameraController.h"	// Header file (also will define if using PICam or Spinnaker)

#ifdef USE_PICAM // Only include this implementation content if using PICam

#include "MainDialog.h"
#include "Utility.h"

CameraController::CameraController(MainDialog* dlg_) {
	this->dlg = dlg_;
	this->libraryInitialized = false;
	this->buffer_.memory = NULL;

	this->UpdateConnectedCameraInfo();
}

CameraController::~CameraController() {
	this->shutdownCamera();
}

// Call all configuration and setups to make sure it is ready before starting
bool CameraController::setupCamera() {
	if (this->camera_ == NULL) {
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

	if (!ConfigureCustomImageSettings()) {
		return false;
	}

	if (!ConfigureExposureTime()) {
		return false;
	}
	Utility::printLine("INFO: Camera has been setup!");
	return true; 
}

// Start acquisition process
bool CameraController::startCamera() {
	// Begin acquisition management when camera has been configured and is now ready to being acquiring images.

	// Get the size that a readout would be
	piint readout_size = 0;
	Picam_GetParameterIntegerValue(this->camera_, PicamParameter_ReadoutStride, &readout_size);
	
	// Commiting buffer parameter
	const PicamParameter* failed_parameters;
	piint failed_parameters_count;
	PicamError err = Picam_CommitParameters(this->camera_, &failed_parameters, &failed_parameters_count);

	// - print any invalid parameters
	if (failed_parameters_count > 0) {
		Utility::printLine("ERROR: invalid following parameters to commit prior to starting the camera!");
	}

	// - free picam-allocated resources for parameters
	Picam_DestroyParameters(failed_parameters);

	// Setting up the buffer, making the available memory size sufficient
	if (this->buffer_.memory == NULL) {
		delete [] this->buffer_.memory;
	}
	this->buffer_.memory = new char[readout_size*2];
	this->buffer_.memory_size = readout_size*2;

	// Setup acquisition buffer
	err = PicamAdvanced_SetAcquisitionBuffer(this->camera_, &this->buffer_);
	if (err != PicamError_None) {
		Utility::printLine("ERROR: Failed to set buffer for starting camera acquisition!");
		return false;
	}

	// Starting acquisition now that buffer has been setup!
	err = Picam_StartAcquisition(this->camera_);
	if (err != PicamError_None) {
		Utility::printLine("ERROR: Failed to start acquisition!");
		return false;
	}
	else {
		Utility::printLine("INFO: Successfully started acquisition");
		return true;
	}
}

// Get most recent image
ImageController* CameraController::AcquireImage() {
	// Get most recent image and return within ImageController class
	PicamAvailableData curImageData;
	PicamAcquisitionStatus curr_status;
	PicamError result;

	// Get the frame (size of image data itself in bytes) and readout (image and meta data)
	piint frame_size = 0;
	Picam_GetParameterIntegerValue(this->camera_, PicamParameter_FrameSize, &frame_size);
	piint readout_size = 0;
	Picam_GetParameterIntegerValue(this->camera_, PicamParameter_ReadoutStride, &readout_size);

	int num_pixels = int(frame_size) / 2; // Number of pixels is half the size in bytes (2-byte depth for each pixel)

	// Grab an image
	// Attempt for acquisiton that is using (hopefully faster) asynchronous approach
	try {
		result = Picam_WaitForAcquisitionUpdate(this->camera_, -1, &curImageData, &curr_status);
	}
	catch (std::exception& e) {
		Utility::printLine(e.what());
	}
	// Doing a check to make sure no issues (no reported errors and that there is at least one readout to get image from)
	if (result != PicamError_None || curImageData.readout_count < 1 || curr_status.errors != PicamAcquisitionErrorsMask_None) {
		Utility::printLine("ERROR: Failed to acquire data from camera!");
		return NULL;
	}

	// Getting a pointer to the most recent frame (our most recent image data) by skipping older readouts (simple method should have readout_count == 1)
		// Cast to char to offset by bytes
	unsigned char* curr_frame = (unsigned char*)curImageData.initial_readout;
	curr_frame = curr_frame + readout_size*(curImageData.readout_count - 1);

	// Copy data into ImageController, but be sure to convert from 2 byte elements to 1 byte
		// Casting the frame pointer as type unsigned short (2 byte elements)
	return new ImageController((unsigned short *)curr_frame, num_pixels, this->cameraImageWidth, this->cameraImageHeight);
}

// Stop acquisition process (but still holds camera instance and other resources)
bool CameraController::stopCamera() {
	// TODO: Check if acquisition is going beforehand

	// End acquisition management but still need to have camera online for another run if needed
	Picam_StopAcquisition(this->camera_);
	
	// Have to iterate through wait acquisition update until the running bool is false!
	PicamAvailableData curImageData;
	PicamAcquisitionStatus curr_status;
	PicamError result;

	do {
		result = Picam_WaitForAcquisitionUpdate(this->camera_, -1, &curImageData, &curr_status);
	} while (curr_status.running == pibln(true));

	// Deallocate the buffer
	delete[] this->buffer_.memory;
	this->buffer_.memory = NULL;
	this->buffer_.memory_size = 0;


	Utility::printLine("INFO: Stopped acquisition");

	return true;
}


// [UTILITY]
int CameraController::PrintDeviceInfo() {
	Utility::printLine();
	Utility::printLine("*** CAMERA INFORMATION ***");

	// Get camrea id to get the name and serial number
	PicamCameraID id;
	//Picam_GetCameraID(this->camera_, &id);

	Picam_ConnectDemoCamera(
		PicamModel_PIMax41024I,
		"12345",
		&id);

	// Display Exposure time and calculated Framerate
	Utility::printLine("Camera: " + std::string(id.sensor_name));
	Utility::printLine("Camera SN: " + std::string(id.serial_number));
	Utility::printLine("Exposure time: " + std::to_string(getFloatParameterValue(PicamParameter_ExposureTime)) + " milliseconds");
	Utility::printLine("Calculated readout time: " + std::to_string(getFloatParameterValue(PicamParameter_ReadoutTimeCalculation)) + " milliseconds");
	Utility::printLine("Calculated frame time: " + std::to_string(float(1000.0f / getFloatParameterValue(PicamParameter_FrameRateCalculation))) + " milliseconds per frame");

	Picam_DestroyCameraIDs(&id); // freeing resources

	Utility::printLine("");
	return 0;
}

piint CameraController::getIntParameterValue(PicamParameter parameter) {
	piint value;
	PicamError errmsg = Picam_GetParameterIntegerValue(this->camera_, parameter, &value);
	if (errmsg == PicamError_None) {
		return value;
	}
	else {
		Utility::printLine("WARNING: Failed to get an integer parameter! Error code: " + errmsg);
		return 0;
	}
}

piflt CameraController::getFloatParameterValue(PicamParameter parameter) {
	piflt value;
	PicamError errmsg = Picam_GetParameterFloatingPointValue(this->camera_, parameter, &value);
	if (errmsg == PicamError_None) {
		return value;
	}
	else {
		Utility::printLine("WARNING: Failed to get an integer parameter! Error code: " + errmsg);
		return 0;
	}
}

std::string CameraController::getStringParameterValue(PicamEnumeratedType type, PicamParameter parameterVal) {
	const pichar * str_buff;
	Picam_GetEnumerationString(type, parameterVal, &str_buff);
	
	std::string result(str_buff);
	Picam_DestroyString(str_buff);
	return result;
}

bool CameraController::shutdownCamera() {
	// TODO: Add check for if acquisition is still occurring and stop if so

	Utility::printLine("INFO: Beginning to shutdown camera!");
	// Clear out camera
	pibln connected;
	// Release if already connected to a camera
	Picam_IsCameraConnected(this->camera_, &connected);
	if (connected) {
		Picam_CloseCamera(this->camera_);
	}

	// Deallocate resources from library
	Picam_UninitializeLibrary();

	delete this->libraryInitialized;
	Utility::printLine("INFO: Finished shutting down camera!");

	if (this->buffer_.memory != NULL) {
		delete[] this->buffer_.memory;
	}

	return true;
}

// [SETUP]
// Update parameters according to GUI
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
		Utility::printLine("ERROR: Was unable to parse bin size input feild!");
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
		Utility::printLine("ERROR: Was unable to parse integration radius input feild!");
		result = false;
	}

	return result;
}

// Reconnect camera
bool CameraController::UpdateConnectedCameraInfo() {
	PicamError err;
	// Initialize library if needed
	Picam_IsLibraryInitialized(this->libraryInitialized);
	if (!this->libraryInitialized) {
		Picam_InitializeLibrary();
		Picam_IsLibraryInitialized(this->libraryInitialized);
	}

	// Release if already connected to a camera
	pibln connected;
	err = Picam_IsCameraConnected(&this->camera_, &connected);
	if (connected) {
		Picam_CloseCamera(this->camera_);
	}

	const PicamCameraID* id = 0;
	piint id_count = 0;

	// Connect this camera to the first available
	
	// get the ids of all cameras
	if (Picam_GetAvailableCameraIDs(&id, &id_count) != PicamError_None) {
		Utility::printLine("WARNING: Error in getting available cameras!");
	}
	// Connect to first one if available, if this fails will setup demo camera to debug with
	if (PicamAdvanced_OpenCameraDevice(id, &this->camera_) != PicamError_None) {
		#ifdef _DEBUG // In debug build, will setup demo camera
		
		// If no successfully connected camera, will create a demo camera for the purposes of demonstrating the program and debugging
		// Demo setup is based on provided PICam example within its acquire.cpp source file
		Utility::printLine("WARNING: Failed to connect to a camera, creating demo camera for debugging/demo purporses");
		
		PicamCameraID id;
		PicamError demoConnectErr = Picam_ConnectDemoCamera(PicamModel_Pixis100B, "12345", &id);
		if (demoConnectErr == PicamError_None) {
			Picam_OpenCamera(&id, &this->camera_); // connecting to demo camera
			Utility::printLine("INFO: Current demo using model Pixis100B, serial number 12345");
		}
		else {
			std::string errMsg = "ERROR: " + std::to_string(demoConnectErr);
			Utility::printLine(errMsg);
		}
		
		#else // If _DEBUG not defined, this is release version and we would want to return false (error) if open camera failed
		Utility::printLine("ERROR: Failed to open camera!");
		return false;
		#endif
	}
	else {
		Utility::printLine("INFO: Connected to " + std::string(id->sensor_name));
		
	}
	// Free up ids now that we are connected
	Picam_DestroyCameraIDs(id);

	// Check to make sure connected
	Picam_IsCameraConnected(this->camera_, &connected);

	if (!connected) {
		Utility::printLine("ERROR: Failed to open camera!");
		return false;
	}
	return true; // No errors and now connected to camera!
}

// Set ROI parameters, image format, etc.
bool CameraController::ConfigureCustomImageSettings() {
	// Set Pixel format to smallest available size (which is 2 bytes in size, will need to descale when getting images to 1 byte)
	if (Picam_SetParameterIntegerValue(this->camera_, PicamParameter_PixelFormat, PicamPixelFormat_Monochrome16Bit) != PicamError_None) {
		Utility::printLine("ERROR: Failed to set pixel format to mono 16 bit!");
		return false;
	}
	// Read one full frame at a time
	if (Picam_SetParameterIntegerValue(this->camera_, PicamParameter_ReadoutControlMode, PicamReadoutControlMode_FullFrame) != PicamError_None) {
		Utility::printLine("ERROR: Failed to set readout control mode to full frame!");
		return false;
	}

	// Setting readout to 0 for indefinite acquisition
	if (Picam_SetParameterLargeIntegerValue(this->camera_, PicamParameter_ReadoutCount, 0) != PicamError_None) {
		Utility::printLine("ERROR: Failed to set readout to continuous!");
		return false;
	}

	// ROI according to GUI, implementation approach based on PICam example rois.cpp //

	// Checking if the GUI setup is invalid by checking against max dimensions and if our ROI is going out of bounds
	int x_max, y_max;
	this->GetFullImage(x_max, y_max);
	if ((this->x0 + this->cameraImageWidth) > x_max || (this->y0 + this->cameraImageHeight) > y_max) {
		Utility::printLine("ERROR: Set ROI exceeds the camera constraints!  Defaulting to entire window for debug");
		this->x0 = 0;
		this->y0 = 0;
		this->cameraImageWidth = x_max;
		this->cameraImageHeight = y_max;
	}
	/* Get the orinal ROI */
	const PicamRois* region;
	if (Picam_GetParameterRoisValue(this->camera_, PicamParameter_Rois, &region) != PicamError_None) {
		Utility::printLine("ERROR: Failed to receive region");
		return false;
	}
	// Set the ROI
	if (region->roi_count == 1) {
		region->roi_array[0].x = this->x0;
		region->roi_array[0].y = this->y0;
		region->roi_array[0].height = this->cameraImageHeight;
		region->roi_array[0].width = this->cameraImageWidth;

		region->roi_array[0].x_binning = 1;
		region->roi_array[0].y_binning = 1;
	}
	PicamError errMsg = Picam_SetParameterRoisValue(this->camera_, PicamParameter_Rois, region);
	if (errMsg != PicamError_None) {
		Utility::printLine("ERROR: Failed to set ROI");
		return false;
	}
	
	// TODO: Figure out these two parameters
	// Set gamma
		// Unsure of comparable setting for PICam


	// Set FPS
		// Unsure if there is a comparable setting for PICam
	
	// Setting initial exposure and commiting parameters //
	this->ConfigureExposureTime();

	// Print resulting Device info
	if (PrintDeviceInfo() == -1) {
		Utility::printLine("WARNING: Couldn't display camera information!");
	}

	return true;
}

// Method of saving an image to a given file path
bool CameraController::saveImage(ImageController * curImage, std::string path) {
	if (curImage == NULL || curImage == nullptr) {
		Utility::printLine("ERROR: Attempted to save image with null pointer!");
		return false;
	}
	curImage->saveImage(path);
	return true;
}

// [ACCESSOR(S)/MUTATOR(S)]
bool CameraController::GetCenter(int &x, int &y) {
	int fullWidth = -1;
	int fullHeight = -1;

	if (!GetFullImage(fullWidth, fullHeight)) {
		Utility::printLine("ERROR: failed to get max dimensions for getting center values");
		return false;
	}

	x = fullWidth / 2;
	y = fullHeight / 2;
	return true;
}

bool CameraController::GetFullImage(int &x, int &y) {
	// Get the ROI constraints
	const PicamRoisConstraint * constraint;

	if (Picam_GetParameterRoisConstraint(this->camera_, PicamParameter_Rois, PicamConstraintCategory_Required, &constraint) != PicamError_None) {
		Utility::printLine("ERROR: Failed to get ROI constraints!");
		return false;
	}
	// Set what the max dimensions are
	x = int(constraint->width_constraint.maximum);
	y = int(constraint->height_constraint.maximum);

	/* Clean up constraints after using them */
	Picam_DestroyRoisConstraints(constraint);
	return true;
}

// Return true if this controller has access to at least one camera
bool CameraController::hasCameras() {
	pibln connected;
	// Get if the camera is connected
	Picam_IsCameraConnected(this->camera_, &connected);
	return connected != 0;
}

// Exposure settings //

// Setter for exposure setting
// Input: exposureTimeToSet - time to set in microseconds
bool CameraController::SetExposure(double exposureTimeToSet) {
	// PICam deals with exposure time in milliseconds, so need to divide the input by 1000
	PicamError errMsg = Picam_SetParameterFloatingPointValue(this->camera_, PicamParameter_ExposureTime, exposureTimeToSet / 1000);
	if (errMsg != PicamError_None) {
		Utility::printLine("ERROR: Failed to set exposure parameter!");
		return false;
	}

	// Commit the change to exposure parameter
	const PicamParameter* failed_parameters;
	piint failed_parameters_count;
	errMsg = Picam_CommitParameters(this->camera_, &failed_parameters, &failed_parameters_count);

	// - print any invalid parameters
	if (failed_parameters_count > 0) {
		Utility::printLine("ERROR: invalid following parameters to commit!");
	}

	// - free picam-allocated resources
	Picam_DestroyParameters(failed_parameters);

	return true;
}

// Reset the exposure time back to initial
bool CameraController::ConfigureExposureTime() {
	finalExposureTime = initialExposureTime;
	return SetExposure(finalExposureTime);
}

// Get the multiplier for exposure having been halved
double CameraController::GetExposureRatio() {
	return initialExposureTime / finalExposureTime;
}

// half the current exposure time setting
void CameraController::HalfExposureTime() {
	finalExposureTime /= 2;
	if (!SetExposure(finalExposureTime)) {
		Utility::printLine("ERROR: wasn't able to half the exposure time!");
	}
}

#endif // End of PICam implementation of CameraController
