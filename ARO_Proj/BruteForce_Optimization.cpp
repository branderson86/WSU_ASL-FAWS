////////////////////
// BruteForce_Optimization.cpp - implementation for brute force algorithm
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"						// Required in source
#include "BruteForce_Optimization.h"	// Header file
#include "Utility.h"					// Utility methods

#include <chrono>
#include <string>

bool BruteForce_Optimization::runOptimization() {
	Utility::printLine("INFO: Starting" +this->algorithm_name_+ "Optimization!");
	//Setup before optimization (see base class for implementation)
	if (!prepareSoftwareHardware()) {
		Utility::printLine("ERROR: Failed to prepare software or/and hardware for Brute Force Optimization");
		return false;
	}
	// Setup variables that are of instance and depend on this specific optimization method (such as pop size)
	if (!setupInstanceVariables()) {
		Utility::printLine("ERROR: Failed to prepare values and files for" +this->algorithm_name_+ "Optimization");
		return false;
	}
	this->timestamp = new TimeStampGenerator();
	// Optimize the selected boards by iterating through the vector that only holds boards to be optimized and access there IDs
	for (int boardIndex = 0; boardIndex < this->optBoards.size() && this->dlg->stopFlag == false; boardIndex++) {
		Utility::printLine("INFO: Currently optimizing board #" + std::to_string(this->optBoards[boardIndex]->board_id));
		runIndividual(this->optBoards[boardIndex]->board_id);
		Utility::printLine("INFO: Finished optimizing board #" + std::to_string(this->optBoards[boardIndex]->board_id));
	}
	// Cleanup
	return shutdownOptimizationInstance();;
}


// Run individual for BF refers to the board being used
// Input: indID - index of SLM board being used (1 based)
// Output: Result added to finalImages_ vector
bool BruteForce_Optimization::runIndividual(int boardID) {
	// Bound check to make sure that the boardID is a valid board
	if (boardID < 1 || boardID > this->sc->boards.size()) {
		Utility::printLine("ERROR: Attempting to optimize a non-existent board (#"+ std::to_string(boardID) + "), ignoring");
		return false;
	}
	int slmWidth = this->sc->getBoardWidth(boardID);
	int slmHeight = this->sc->getBoardHeight(boardID);

	// Initialize array for storing slm images
	int * slmImg = new int[this->cc->numberOfBinsY * this->cc->numberOfBinsX * this->cc->populationDensity];
	
	//Initialize array of SLM image with 0s (note that the size of slmImg is dependent on the camera and not SLM!)
	setBlankSlmImg(slmImg);

	bool endOpt = false;
	try {
		// Iterate through columns
		for (int binCol = 0; binCol < this->cc->numberOfBinsX && !endOpt; binCol++) {
			// Iterate through rows
			for (int binRow = 0; binRow < this->cc->numberOfBinsY && !endOpt; binRow++) {
				int binValMax = 0;
				double fitValMax = 0;
				// Current bin
				int binIndex = (binCol + binRow*this->cc->numberOfBinsX)*this->cc->populationDensity;

				// Find max phase for this bin
				for (int curBinVal = 0; curBinVal < 256 && !endOpt; curBinVal += this->phaseResolution) {
					// Abort if stop button was pressed
					if (dlg->stopFlag == true) {
						return true;
					}

					ImageController * curImage;

					// Assign at current bin the new value to test
					slmImg[binIndex] = curBinVal;

					// Scale and Write to board
					this->scalers[boardID]->TranslateImage(slmImg, this->slmScaledImages[boardID]);

					this->usingHardware = true;

					this->sc->writeImageToBoard(boardID, this->slmScaledImages[boardID]);

					//Acquire camera image
					curImage = this->cc->AcquireImage();
					this->usingHardware = false;

					if (curImage == NULL)	{
						Utility::printLine("ERROR: Image Acquisition has failed!");
						continue;
					}
					unsigned char* camImg = curImage->getRawData();
					// Display cam image
					if (this->displayCamImage) {
						this->camDisplay->UpdateDisplay(camImg);
					}
					if (this->displaySLMImage) {
						this->slmDisplayVector[0]->UpdateDisplay(this->slmScaledImages[boardID]);
					}
					// Determine fitness

					double exposureTimesRatio = this->cc->GetExposureRatio();
					double fitness = Utility::FindAverageValue(camImg, curImage->getWidth(), curImage->getHeight(), this->cc->targetRadius);

					//Record current performance to file //Ask what kind of calcualtion is this?
					double ms = boardID*this->phaseResolution + curBinVal / this->phaseResolution;
					if (this->logAllFiles || this->saveTimeVSFitness) {
						this->timeVsFitnessFile << this->timestamp->MS_SinceStart() << " " << fitness * this->cc->GetExposureRatio() << " " << this->cc->GetExposureRatio() << std::endl;
						this->tfile << ms << " " << fitness * this->cc->GetExposureRatio() << " " << this->cc->GetExposureRatio() << std::endl;
					}
					// Keep record of the best fitness value and image
					if (fitness * exposureTimesRatio > fitValMax) {
						binValMax = curBinVal;
						fitValMax = fitness * exposureTimesRatio;
					}
					if (fitValMax > this->allTimeBestFitness) {
						this->bestImage = curImage;

					}
					// Halve the exposure time if over max fitness allowed
					if (fitness > this->maxFitnessValue) {
						this->cc->HalfExposureTime();
					}
					// Deallocate current image if not the best one
					if (curImage != this->bestImage) {
						delete curImage;
					}
					// Get stop flag to check if should continue or abort
					endOpt = dlg->stopFlag;
				}  // ... curBinVal loop

				if (fitValMax > this->allTimeBestFitness) {
					this->allTimeBestFitness = fitValMax;
					Utility::printLine("INFO: Current best fitness value updated to - " + std::to_string(allTimeBestFitness));
				}

				slmImg[binIndex] = binValMax;

				// Save progress data
				if (this->logAllFiles) {
					lmaxfile << binValMax << " " << fitValMax << std::endl;
					rtime << this->timestamp->MS_SinceStart() << " ms  " << fitValMax << "   " << this->cc->finalExposureTime << std::endl;
				}
			} // ... binRow loop
		} // ... binCol loop

		this->finalImages_.push_back(slmImg);
		slmImg = NULL;
	}
	catch (std::exception &e) {
		Utility::printLine("ERROR: "+this->algorithm_name_+"ran into issue with board #" + std::to_string(boardID));
		Utility::printLine(std::string(e.what()));
		return false;
	}
	// With no errors, slmImg should now contain optimal image
	return true;
}

bool BruteForce_Optimization::setupInstanceVariables() {
	this->cc->startCamera(); // setup camera
	if (this->logAllFiles || this->saveTimeVSFitness) {
		this->tfile.open(this->outputFolder + this->algorithm_name_ +"_functionEvals_vs_fitness.txt");
		this->timeVsFitnessFile.open(this->outputFolder + this->algorithm_name_ +"_time_vs_fitness.txt");
	}
	// Setup displays
	if (this->displayCamImage) {
		this->camDisplay = new CameraDisplay(this->cc->cameraImageHeight, this->cc->cameraImageWidth, "Camera Display");
		this->camDisplay->OpenDisplay(240, 240);
	}
	if (this->displaySLMImage) {
		this->slmDisplayVector.push_back(new CameraDisplay(this->sc->getBoardHeight(0), this->sc->getBoardWidth(0), "SLM Display"));
		this->slmDisplayVector[0]->OpenDisplay(240, 240);
	}

	// Scaler Setup (using base class)
	this->slmScaledImages.clear();
	// Setup the scaled images vector
	this->slmScaledImages = std::vector<unsigned char*>(this->sc->boards.size());
	this->scalers.clear();
	// Setup a vector for every board and initializing all slmScaledImages to 0s
	for (int i = 0; i < sc->boards.size(); i++) {
		this->slmScaledImages[i] = new unsigned char[this->sc->boards[i]->GetArea()];
		this->scalers.push_back(setupScaler(this->slmScaledImages[i], i));
	}

	CString path("");
	dlg->m_cameraControlDlg.m_FramesPerSecond.GetWindowTextW(path);
	this->phaseResolution = _tstoi(path);

	this->allTimeBestFitness = 0;

	// Open files for logging algorithm progress 
	if (this->logAllFiles || this->saveTimeVSFitness) {
		this->lmaxfile.open(this->outputFolder + "lmax.txt");
		this->rtime.open(this->outputFolder + "Opt_rtime.txt");
	}
	return true;
}

bool BruteForce_Optimization::shutdownOptimizationInstance() {
	// - log files close
	if (this->logAllFiles) {
		this->lmaxfile.close();
		this->rtime.close();
	}
	std::string curTime = Utility::getCurDateTime();
	// Generic file renaming to include time stamps
	if (this->logAllFiles || this->saveTimeVSFitness) {
		this->timeVsFitnessFile.close();
		this->tfile.close();
		std::rename((this->outputFolder + "Opt_functionEvals_vs_fitness.txt").c_str(), (this->outputFolder + curTime + "_OPT5_functionEvals_vs_fitness.txt").c_str());
		std::rename((this->outputFolder + "Opt_time_vs_fitness.txt").c_str(), (this->outputFolder + curTime + "_OPT5_time_vs_fitness.txt").c_str());
		std::rename((this->outputFolder + "lmax.txt").c_str(), (this->outputFolder + curTime + "_OPT5_lmax.txt").c_str());
		std::rename((this->outputFolder + "Opt_rtime.txt").c_str(), (this->outputFolder + curTime + "_OPT5_rtime.txt").c_str());
		//Record total time taken for optimization
		std::ofstream tfile2(this->outputFolder + curTime + "_OPT5_time.txt");
		tfile2 << this->timestamp->MS_SinceStart() << std::endl;
		tfile2.close();
	}

	if (this->logAllFiles || this->saveParametersPref) {
		saveParameters(curTime);
		CString buff;
		dlg->m_outputControlDlg.m_OutputLocationField.GetWindowTextW(buff);
		std::string path = CT2A(buff);
		path += curTime + "_OPT5_savedParameters.cfg";
		dlg->saveUItoFile(path);
	}

	// Save how final optimization looks through camera
	if (this->bestImage != NULL && this->saveResultImages) {
		unsigned char* camImg = this->bestImage->getRawData();
		cv::Mat Opt_ary = cv::Mat(this->bestImage->getHeight(), this->bestImage->getWidth(), CV_8UC1, camImg);
		cv::imwrite(this->outputFolder + curTime + "_OPT5_Optimized.bmp", Opt_ary);
	}

	// - camera shutdown
	this->cc->stopCamera();

	// - memory deallocation
	if (this->bestImage != NULL) {
		delete this->bestImage;
	}
	if (this->camDisplay != NULL) {
		this->camDisplay->CloseDisplay();
		delete this->camDisplay;
	}
	if (this->slmDisplayVector[0] != NULL) {
		this->slmDisplayVector[0]->CloseDisplay();
		delete this->slmDisplayVector[0];
	}
	this->slmDisplayVector.clear();

	if (this->timestamp != NULL) {
		delete this->timestamp;
	}
	// Delete all the scalers in the vector
	for (int i = 0; i < this->scalers.size(); i++) {
		delete this->scalers[i];
	}
	this->scalers.clear();
	// Delete all the scaled image pointers in the vector
	for (int i = 0; i < this->slmScaledImages.size(); i++) {
		delete[] this->slmScaledImages[i];
	}
	this->slmScaledImages.clear();

	//Record the final (most fit) slm images followed by deleting them
	for (int i = int(this->finalImages_.size())-1; i >= 0; i--) {
		// Save image
		if (this->logAllFiles || this->saveResultImages) {
			cv::Mat m_ary = cv::Mat(512, 512, CV_8UC1, this->finalImages_[i]);
			cv::imwrite(this->outputFolder + curTime + "_OPT5_phaseopt_" + std::to_string(this->optBoards[i]->board_id)+".bmp", m_ary);
		}
		if (this->finalImages_[i] != NULL) {
			delete[] this->finalImages_[i]; // deallocate then
		}
		this->finalImages_.pop_back();  // remove from vector
	}
	this->finalImages_.clear();

	//Reset UI State
	this->isWorking = false;
	this->dlg->disableMainUI(!isWorking);
	return true;
}

void BruteForce_Optimization::setBlankSlmImg(int * slmImg) {
	for (int y = 0; y < this->cc->numberOfBinsY; y++) {
		for (int x = 0; x < this->cc->numberOfBinsX; x++) {
			int index = (x + y*this->cc->numberOfBinsX)*this->cc->populationDensity;
			slmImg[index] = 0;
		}
	}
}
