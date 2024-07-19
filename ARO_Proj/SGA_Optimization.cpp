////////////////////
// SGA_Optimization.cpp - Optimization handler methods implementation for simple genetic algorithm
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"				// Required in source
#include "SGA_Optimization.h"	// Header file

// Method to setup specific properties runOptimziation() instance
bool SGA_Optimization::setupInstanceVariables() {
	// Setting population size as well as number of elite individuals kept in the genetic repopulation
	this->populationSize = 30;
	this->eliteSize = 5;

	// Get how many populations to have (same as number of boards being optimized)
	this->popCount = int(this->optBoards.size());

	// Setting population vector
	// For threadCount, it is the number of threads in total allowed divided by number of boards
	this->population.clear();
	for (int i = 0; i < this->popCount; i++) {
		this->population.push_back(new SGAPopulation<int>(this->cc->numberOfBinsY * this->cc->numberOfBinsX * this->cc->populationDensity,
			this->populationSize, this->eliteSize, this->acceptedSimilarity, this->multithreadEnable, (this->gaPoolThreadCount / int(this->optBoards.size())), this->myThreadPool_));
	}

	this->shortenExposureFlag = false;		// Set to true by individual if fitness is too high
	this->stopConditionsMetFlag = false;	// Set to true if a stop condition was reached by one of the individuals
	this->bestImage = NULL;
	// Setup image displays for camera and SLM
	// Open displays if preference is set
	if (this->displayCamImage) {
		this->camDisplay = new CameraDisplay(this->cc->cameraImageHeight, this->cc->cameraImageWidth, "Camera Display");
		this->camDisplay->OpenDisplay(240, 240);
	}
	this->slmDisplayVector.clear();
	if (this->displaySLMImage) {
		for (int displayNum = 0; displayNum < this->popCount; displayNum++) {
			int slmID = this->optBoards[displayNum]->board_id;
			this->slmDisplayVector.push_back(new CameraDisplay(this->sc->getBoardHeight(slmID - 1), this->sc->getBoardWidth(slmID - 1), ("SLM Display " + std::to_string(slmID)).c_str()));
			this->slmDisplayVector[displayNum]->OpenDisplay(240, 240);
		}
	}
	// Scaler Setup (using base class)
	this->slmScaledImages.clear();
	// Setup the scaled images vector
	this->slmScaledImages = std::vector<unsigned char*>(this->optBoards.size());
	this->scalers.clear();
	// Setup a vector of scalers for every board being optimized
	for (int i = 0; i < this->optBoards.size(); i++) {
		this->slmScaledImages[i] = new unsigned char[this->optBoards[i]->GetArea()];
		this->scalers.push_back(setupScaler(this->slmScaledImages[i], i));
	}

	// Start up the camera
	this->cc->startCamera();

	//Open up files to which progress will be logged
	if (this->logAllFiles || this->saveTimeVSFitness) {
		this->timePerGenFile.open(this->outputFolder + this->algorithm_name_ + "_timePerformance.txt");
		this->timePerGenFile << "SGA Generation,Individuals Time (microseconds),NextGeneration Time (microseconds),Overall Generation Time (microseconds),";
		// Also for easier tracking, outputing the thread counts as well
		this->timePerGenFile << "Eval Individuals Threads," << this->indThreadCount << ",Next Generation Threads, " << this->gaPoolThreadCount << "\n";

		this->timeVsFitnessFile.open(this->outputFolder + this->algorithm_name_ + "_time_vs_fitness.txt");
	}
	if (this->logAllFiles || this->saveExposureShorten) {
		this->efile.open(this->outputFolder + this->algorithm_name_ + "_exposure.txt");
	}
	if (this->logAllFiles || this->saveEliteImages) {
		this->tfile.open(this->outputFolder + this->algorithm_name_ + "_functionEvals_vs_fitness.txt");
	}

	return true; // Returning true if no issues met
}

// Method to clean up & save resulting runOptimziation() instance
bool SGA_Optimization::shutdownOptimizationInstance() {

	std::string curTime = Utility::getCurDateTime();

	// Only save images if not aborting (successful results)
	if (this->dlg->stopFlag == false && this->saveResultImages) {
		// Get elite info
		unsigned char* eliteImage = this->bestImage->getRawData();
		int imgHeight = this->bestImage->getHeight();
		int imgWidth = this->bestImage->getWidth();

		// Save how final optimization looks through camera
		this->cc->saveImage(this->bestImage, this->outputFolder + curTime + "_" + this->algorithm_name_ + "_Optimized.bmp");

		// Save final (most fit SLM images)
		for (int popID = 0; popID < this->population.size(); popID++) {
			scalers[popID]->TranslateImage(this->population[popID]->getGenome(this->population[popID]->getSize() - 1), this->slmScaledImages[popID]);
			cv::Mat m_ary = cv::Mat(512, 512, CV_8UC1, this->slmScaledImages[popID]);
			cv::imwrite(this->outputFolder + curTime + "_" + this->algorithm_name_ + "_phaseopt_SLM_" + std::to_string(this->optBoards[popID]->board_id) + ".bmp", m_ary);
		}
	}

	// Generic file renaming to have time stamps of run
	if (this->logAllFiles || this->saveTimeVSFitness) {
		this->timeVsFitnessFile << this->timestamp->MS_SinceStart() << " " << 0 << std::endl;
		this->timeVsFitnessFile.close();
		this->timePerGenFile.close();
		std::rename((this->outputFolder + this->algorithm_name_ + "_timePerformance.txt").c_str(), (this->outputFolder + curTime + "_" + this->algorithm_name_ + "_timePerformance.csv").c_str());
		std::rename((this->outputFolder + this->algorithm_name_ + "_time_vs_fitness.txt").c_str(), (this->outputFolder + curTime + "_" + this->algorithm_name_ + "_time_vs_fitness.txt").c_str());
	}
	if (this->logAllFiles || this->saveEliteImages) {
		this->tfile.close();
		std::rename((this->outputFolder + this->algorithm_name_ + "_functionEvals_vs_fitness.txt").c_str(), (this->outputFolder + curTime + "_" + this->algorithm_name_ + "_functionEvals_vs_fitness.txt").c_str());
	}
	if (this->logAllFiles || this->saveExposureShorten) {
		this->efile.close();
		std::rename((this->outputFolder + this->algorithm_name_ + "_exposure.txt").c_str(), (this->outputFolder + curTime + "_" + this->algorithm_name_ + "_exposure.txt").c_str());
	}
	if (this->logAllFiles || this->saveParametersPref) {
		saveParameters(curTime);
		CString buff;
		dlg->m_outputControlDlg.m_OutputLocationField.GetWindowTextW(buff);
		std::string path = CT2A(buff);
		path += curTime + "_" + this->algorithm_name_ + "_savedParameters.cfg";
		dlg->saveUItoFile(path);
	}

	// - image displays
	if (this->camDisplay != NULL) {
		this->camDisplay->CloseDisplay();
		delete this->camDisplay;
	}
	for (int i = 0; i < this->slmDisplayVector.size(); i++) {
		this->slmDisplayVector[i]->CloseDisplay();
		delete this->slmDisplayVector[i];
	}
	this->slmDisplayVector.clear();

	// - camera
	this->cc->stopCamera();
	// - pointers
	if (this->bestImage != NULL) {
		delete this->bestImage;
	}
	for (int i = 0; i < this->population.size(); i++) {
		delete this->population[i];
	}
	this->population.clear();

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
	return true;
}
