////////////////////
// Optimization.cpp - implementation for the Optimization's base class functions
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"				// Required in source
#include "Optimization.h"		// Header file
#include "Utility.h"			// use printLine()

Optimization::Optimization(MainDialog* dlg, CameraController* cc, SLMController* sc) {
	if (cc == nullptr) {
		Utility::printLine("WARNING: invalid camera controller passed to optimization!");
	}
	if (sc == nullptr) {
		Utility::printLine("WARNING: invalid SLM controller passed to optimization!");
	}
	this->cc = cc;
	this->sc = sc;
	this->dlg = dlg;

	// Read from the output dialog for output parameters
	prepareOutputSettings();

	// Check if using multithreading
	if (this->dlg->m_MultiThreadEnable.GetCheck() == BST_CHECKED) {
		this->multithreadEnable = true;
	}
	else {
		this->multithreadEnable = false;
	}
	// Check if skipping elite
	if (this->dlg->m_ga_ControlDlg.m_skipEliteReevaluation.GetCheck() == BST_CHECKED) {
		this->skipEliteReevaluation = true;
	}
	else {
		this->skipEliteReevaluation = false;
	}
}

// [SETUP]
// Draw from GUI the optimization stop conditions
bool Optimization::prepareStopConditions() {
	bool result = true;

	// Fitness to stop at
	try	{
		CString path;
		this->dlg->m_ga_ControlDlg.m_minFitness.GetWindowTextW(path);
		if (path.IsEmpty()){
			throw new std::exception();
		}
		this->fitnessToStop = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Can't Parse Minimum Fitness");
		result = false;
	}

	// Time (in sec) to stop at
	try	{
		CString path;
		this->dlg->m_ga_ControlDlg.m_minSeconds.GetWindowTextW(path);
		if (path.IsEmpty()) {
			throw new std::exception();
		}
		this->minSecondsToStop = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Can't Parse Minimum Seconds Elapsed");
		result = false;
	}
	try	{
		CString path;
		this->dlg->m_ga_ControlDlg.m_maxSeconds.GetWindowTextW(path);
		if (path.IsEmpty()) {
			throw new std::exception();
		}
		this->maxSecondsToStop = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Can't Parse Minimum Seconds Elapsed");
		result = false;
	}
	// Generations evaluations to at least do (minimum)
	try	{
		CString path;
		this->dlg->m_ga_ControlDlg.m_minGenerations.GetWindowTextW(path);
		if (path.IsEmpty()) {
			throw new std::exception();
		}
		this->genEvalToStop = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Can't Parse Minimum Generation Evaluations");
		result = false;
	}
	// Generations evaluations to stop at (maximum)
	try	{
		CString path;
		this->dlg->m_ga_ControlDlg.m_maxGenerations.GetWindowTextW(path);
		if (path.IsEmpty()) {
			throw new std::exception();
		}
		this->maxGenenerations = _tstof(path);
	}
	catch (...)	{
		Utility::printLine("ERROR: Can't Parse Maximum Generation Evaluations");
		result = false;
	}
	return result;
}

// Draw from GUI the output settings
bool Optimization::prepareOutputSettings() {
	// Display Camera
	if (this->dlg->m_outputControlDlg.m_displayCameraCheck.GetCheck() == BST_CHECKED) {
		this->displayCamImage = true;
	}
	else {
		this->displayCamImage = false;
	}
	// Display SLMs
	if (this->dlg->m_outputControlDlg.m_displaySLM.GetCheck() == BST_CHECKED) {
		this->displaySLMImage = true;
	}
	else {
		this->displaySLMImage = false;
	}
	// A check all Enable all option
	if (this->dlg->m_outputControlDlg.m_logAllFilesCheck.GetCheck() == BST_CHECKED) {
		this->logAllFiles = true;
		this->saveEliteImages = true;
	}
	else {
		this->logAllFiles = false;
	}
	if (this->dlg->m_outputControlDlg.m_SaveParameters.GetCheck() == BST_CHECKED) {
		this->saveParametersPref = true;
	}
	else {
		this->saveParametersPref = false;
	}
	// If this enable all checkbox isn't enabled, then we must check the more specific ones
	if (this->logAllFiles == false) {
		if (this->dlg->m_outputControlDlg.m_SaveFinalImagesCheck.GetCheck() == BST_CHECKED) {
			this->saveResultImages = true;
		}
		else {
			this->saveResultImages = false;
		}
		if (this->dlg->m_outputControlDlg.m_SaveEliteImagesCheck.GetCheck() == BST_CHECKED) {
			this->saveEliteImages = true;
		}
		else {
			this->saveEliteImages = false;
		}
		if (this->dlg->m_outputControlDlg.m_SaveExposureShortCheck.GetCheck() == BST_CHECKED) {
			this->saveExposureShorten = true;
		}
		else {
			this->saveExposureShorten = false;
		}
		if (this->dlg->m_outputControlDlg.m_SaveTimeVFitnessCheck.GetCheck() == BST_CHECKED) {
			this->saveTimeVSFitness = true;
		}
		else {
			this->saveTimeVSFitness = false;
		}
	}
	// Get where to store the outputs
	CString buff;
	this->dlg->m_outputControlDlg.m_OutputLocationField.GetWindowTextW(buff);
	this->outputFolder = CT2A(buff);
	// Get freqency of saving elite images (getting value regardless of if it is enabled or not)
	try	{
		this->dlg->m_outputControlDlg.m_eliteSaveFreq.GetWindowTextW(buff);
		if (buff.IsEmpty()) {
			throw new std::exception();
		}
		this->saveEliteFrequency = _tstoi(buff);
	}
	catch (...)	{
		Utility::printLine("ERROR: Failure to read save elite frequency! Disabling save elite");
		this->saveEliteImages = false;
	}

	return true;
}

// Setup camera, verify SLM is ready (setting up the board vector) and prepare stop conditions
bool Optimization::prepareSoftwareHardware() {
	Utility::printLine("INFO: Preparing equipment and software for optimization!");

	//Can't start operation if an optimization is already running 
	if (this->isWorking) {
		Utility::printLine("WARNING: cannot prepare hardware the second time!");
		return false;
	}
	Utility::printLine("INFO: No optimization running, able to perform setup!");

	// - configure equipment
	if (!this->cc->setupCamera())	{
		Utility::printLine("ERROR: Camera setup has failed!");
		return false;
	}
	Utility::printLine("INFO: Camera setup complete!");

	if (!this->sc->updateFromGUI()) {
		Utility::printLine("ERROR: SLM setup has failed!");
		return false;
	}

	// Get all the boards that are to be optimized
	this->optBoards.clear();
	for (int i = 0; i < this->sc->boards.size(); i++) {
		if (this->sc->boards[i]->isToBeOptimized()) {
			this->optBoards.push_back(this->sc->boards[i]);
		}
	}

	Utility::printLine("INFO: SLM setup complete!");
	// Inform the identified boards to optimize
	Utility::printLine("INFO: Optimizing " + std::to_string(this->optBoards.size()) + " board(s) at");
	for (int i = 0; i < this->optBoards.size(); i++) {
		Utility::printLine("INFO:   #" + std::to_string(this->optBoards[i]->board_id));
	}

	// - configure algorithm parameters
	if (!prepareStopConditions()) {
		Utility::printLine("ERROR: Preparing stop conditions has failed!");
		return false;
	}
	Utility::printLine("INFO: Hardware ready!");

	// - configure proper UI states
	this->isWorking = true;
	this->dlg->disableMainUI(!isWorking);

	return true;
}

// For a given board setup and return a scaler
// Input: slmNum (default 0 and 0 based) - index of board to set scaler with
//        slmImg - char pointer to array with size equal to total area of board
// Output: - returns pointer to a new ImageScaler based on width & height of SLM at slmNum
//		   - slmImg is filled with zeros
ImageScaler* Optimization::setupScaler(unsigned char *slmImg, int slmNum = 0) {
	int width = int(sc->getBoardWidth(slmNum));
	int height = int(sc->getBoardHeight(slmNum));

	ImageScaler* scaler = new ImageScaler(width, height, 1);
	scaler->SetBinSize(cc->binSizeX, cc->binSizeY);
	scaler->SetUsedBins(cc->numberOfBinsX, cc->numberOfBinsY);
	scaler->ZeroOutputImage(slmImg); // Initialize the slm image array to be all zeros

	return scaler;
}

// [SAVE/LOAD FEATURES]
// Output information of the parameters used in the optimization in to logs
void Optimization::saveParameters(std::string time) {
	std::ofstream paramFile(this->outputFolder + time + "_" + this->algorithm_name_ + "_Optimization_Parameters.txt");
	paramFile << "----------------------------------------------------------------" << std::endl;
	paramFile << "OPTIMIZATION SETTINGS:" << std::endl;
	paramFile << "Type - " << this->algorithm_name_ << std::endl;
	if (this->algorithm_name_ != "OPT5") {
		paramFile << "Stop Fitness - " << std::to_string(this->fitnessToStop) << std::endl;
		paramFile << "Min Stop Time - " << std::to_string(this->minSecondsToStop) << std::endl;
		paramFile << "Max Stop Time - " << std::to_string(this->maxSecondsToStop) << std::endl;
		paramFile << "Min Generation - " << std::to_string(this->genEvalToStop) << std::endl;
		paramFile << "Max Generation - " << std::to_string(this->maxGenenerations) << std::endl;;
	}
	paramFile << "----------------------------------------------------------------" << std::endl;
	paramFile << "CAMERA SETTINGS:" << std::endl;
	paramFile << "AOI x0 - " << std::to_string(this->cc->x0) << std::endl;
	paramFile << "AOI y0 - " << std::to_string(this->cc->y0) << std::endl;
	paramFile << "AOI Image Width - " << std::to_string(this->cc->cameraImageWidth) << std::endl;
	paramFile << "AOI Image Height - " << std::to_string(this->cc->cameraImageHeight) << std::endl;
	paramFile << "Acquisition Gamma - " << std::to_string(this->cc->gamma) << std::endl;
	paramFile << "Acquisition FPS - " << std::to_string(this->cc->fps) << std::endl;
	paramFile << "Acquisition Initial Exposure Time - " << std::to_string(this->cc->initialExposureTime) << std::endl;
	paramFile << "Number of Bins X - " << std::to_string(this->cc->numberOfBinsX) << std::endl;
	paramFile << "Number of Bins Y - " << std::to_string(this->cc->numberOfBinsY) << std::endl;
	paramFile << "Bins Size X - " << std::to_string(this->cc->numberOfBinsX) << std::endl;
	paramFile << "Bins Size Y - " << std::to_string(this->cc->numberOfBinsY) << std::endl;
	paramFile << "Target Radius - " << std::to_string(this->cc->targetRadius) << std::endl;
	paramFile << "----------------------------------------------------------------" << std::endl;
	paramFile << "SLM SETTINGS:" << std::endl;
	paramFile << "Board Amount - " << std::to_string(this->sc->numBoards) << std::endl;
	paramFile << "Number of Boards being Optimized - " << std::to_string(this->optBoards.size()) << std::endl;
	for (int i = 0; i < int(this->sc->numBoards); i++) {
		paramFile << "Board #" << i + 1 << "\tLUT filePath - " << this->sc->boards[i]->LUTFileName << std::endl;
		paramFile << "        '\t'" << "To Be Optimized - " << this->sc->boards[i]->isToBeOptimized() << std::endl;
	}
	paramFile.close();
}

//[CHECKS]
bool const Optimization::stopConditionsReached(double curFitness, double curSecPassed, double curGenerations) {
	// If reached fitness to stop and minimum time and minimum generations to perform
	if ((curFitness > this->fitnessToStop && curSecPassed > this->minSecondsToStop && curGenerations > this->genEvalToStop)) {
		return true;
	}
	// If the stop button was pressed
	if (dlg->stopFlag == true) {
		return true;
	}
	// If exceeded the maximum allowed time (negative or zero value indicated indefinite)
	if (this->maxSecondsToStop > 0 && curSecPassed >= this->maxSecondsToStop) {
		return true;
	}
	// If exceeded the maximum allowed time (negative or zero value indicated indefinite)
	if (this->maxGenenerations > 0 && curGenerations >= this->maxGenenerations) {
		return true;
	}
	return false;
}
