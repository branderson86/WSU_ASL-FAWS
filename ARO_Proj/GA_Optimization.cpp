////////////////////
// GA_Optimization.cpp - Implementation of high level genetic algorithm optimization behavior
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"				// Required in source
#include "GA_Optimization.h"	// Header file

bool GA_Optimization::runOptimization() {
	Utility::printLine("INFO: Starting " + this->algorithm_name_ + " Optimization!");


	if (this->multithreadEnable) {
		Utility::printLine("INFO: The CPU being used has " + std::to_string(std::thread::hardware_concurrency()) + " logical processors");
		// Getting how many threads that the tasks will be using
		CString tempBuff;
		this->dlg->m_ga_ControlDlg.m_indEvalThreadCount.GetWindowTextW(tempBuff);
		this->indThreadCount = _tstoi(tempBuff);

		this->dlg->m_ga_ControlDlg.m_PopGenThreadCount.GetWindowTextW(tempBuff);
		this->gaPoolThreadCount = _tstoi(tempBuff);

		// If the indThreadCount and gaPoolThreadCount are less than what the hardware supports, than we don't need the additional threads to be created in the pool
		const int threadPool_size = std::min(int(std::thread::hardware_concurrency()), std::max(this->indThreadCount, this->gaPoolThreadCount));
		this->myThreadPool_ = new threadPool(threadPool_size);

		Utility::printLine("INFO: Using up to " + std::to_string(threadPool_size) + " threads");
	}
	else {
		this->indThreadCount = 1;
		this->gaPoolThreadCount = 1;
	}

	// Setup before optimization (see base class for implementation)
	if (!prepareSoftwareHardware()) {
		Utility::printLine("ERROR: Failed to prepare software or/and hardware for " + this->algorithm_name_ + " Optimization");
		return false;
	}
	// Setup variables that are of instance and depend on this specific optimization method (such as pop size)
	if (!setupInstanceVariables()) {
		Utility::printLine("ERROR: Failed to prepare values and files for " + this->algorithm_name_ + " Optimization");
		return false;
	}

	// Doubles to track time elapsed during optimization
	double opt_start, opt_end, generation_start, generation_end, individuals_start, individuals_end, nextGen_start, nextGen_end;
	try {	// Begin camera exception handling while optimization loop is going
		// Lambda function to access this instance of Optimization to perform runIndividual for group of individuals (fewer threads)
		// Input: threadID - this thread
		//		  numThreads - total number of threads being run
		// Captures: this - pointer to current GA_Optimization instance
		auto evaluateSubGroup = [this](const int threadID){
			int groupSize = (this->populationSize / this->indThreadCount);
			int remainder = this->populationSize - groupSize*this->indThreadCount;
			int start_index = threadID*groupSize;

			if (remainder != 0) {
				if (threadID < remainder) {
					groupSize++;
					start_index += threadID;
				}
				else {
					start_index += remainder;
				}
			}

			for (int id = start_index; id < start_index + groupSize && id < this->populationSize; id++) {
				if (this->skipEliteReevaluation == false || (this->skipEliteReevaluation == true && this->population[0]->getFitness(id) == -1)) {
					this->runIndividual(id);
				}
			}
		};

		Utility::printLine("INFO: Beginning optimization loop");
		this->timestamp = new TimeStampGenerator();		// Starting time stamp to track elapsed time
		opt_start = this->timestamp->MicroS_SinceStart();
		// Optimization loop for each generation
		for (this->curr_gen = 0; this->curr_gen < this->maxGenenerations && !this->stopConditionsMetFlag && !this->dlg->stopFlag; this->curr_gen++) {
			generation_start = this->timestamp->MicroS_SinceStart();
			individuals_start = generation_start;
			// Run each individual, giving them all fitness values as a result of their genome

			if (this->multithreadEnable == true) {
				// Parallel
				for (int indID = 0; indID < this->indThreadCount; indID++) {
					this->myThreadPool_->pushJob(std::bind(evaluateSubGroup, indID));
				}
				this->myThreadPool_->wait();
			}
			else {
				for (int indID = 0; indID < this->populationSize; indID++) {
					// If skipping already evaluated toggled and this individual already has a fitness (not initial -1) then skip
					if (this->skipEliteReevaluation == false || (this->skipEliteReevaluation == true && this->population[0]->getFitness(indID) == -1)) {
						this->runIndividual(indID); // Serial
					}
				}
			}
			individuals_end = this->timestamp->MicroS_SinceStart();

			// record how long it took to evaluate individuals
			if (this->logAllFiles || this->saveTimeVSFitness) {
				this->timePerGenFile << this->curr_gen + 1 << "," << individuals_end - individuals_start << ",";
			}
			// Perform GA crossover/breeding to produce next generation
			nextGen_start = this->timestamp->MicroS_SinceStart();
			for (int popID = 0; popID < this->population.size(); popID++) {
				this->population[popID]->nextGeneration();
			}
			nextGen_end = this->timestamp->MicroS_SinceStart();

			// Record how long it took to generate next generation
			if (this->logAllFiles || this->saveTimeVSFitness) {
				this->timePerGenFile << nextGen_end - nextGen_start << ",";
			}

			// Update displays with best individual now that they are done
			if (this->displayCamImage) {
				this->camDisplay->UpdateDisplay(this->bestImage->getRawData());
			}
			if (this->displaySLMImage) {
				for (int slmID = 0; slmID < this->popCount; slmID++) {
					this->scalers[slmID]->TranslateImage(this->population[slmID]->getGenome(this->populationSize - 1), this->slmScaledImages[slmID]);
					this->slmDisplayVector[slmID]->UpdateDisplay(this->slmScaledImages[slmID]);
				}
			}
			// Half exposure time if fitness value is too high
			if (this->shortenExposureFlag) {
				this->cc->HalfExposureTime();
				this->shortenExposureFlag = false;
				if (this->saveExposureShorten || this->logAllFiles) {
					this->efile << "Exposure shortened after gen: " << this->curr_gen + 1 << " with new ratio " << this->cc->GetExposureRatio() << std::endl;
				}
			}
			// Output to the terminal progress to help show progress
			if (this->curr_gen % 10 == 0) {
				Utility::printLine("INFO: Finished generation #" + std::to_string(this->curr_gen) + " with a fitness of " + std::to_string(this->population[0]->getFitness(this->populationSize - 1)));
			}
			// Check stop conditions, only assign true if we reached the condition
			this->stopConditionsMetFlag = stopConditionsReached((this->population[0]->getFitness(this->populationSize - 1)*this->cc->GetExposureRatio()), this->timestamp->S_SinceStart(), this->curr_gen + 1);

			// Record the time it took to perform this generation, then update start to now (for getting duration next generation)
			if (this->logAllFiles || this->saveTimeVSFitness) {
				generation_end = this->timestamp->MicroS_SinceStart();
				this->timePerGenFile << generation_end - generation_start << std::endl;
			}
		} // ... optimization loop

		if (this->logAllFiles || this->saveTimeVSFitness) {
			opt_end = this->timestamp->MicroS_SinceStart();
			this->timePerGenFile << "\nOverall Time in Microseconds," << opt_end - opt_start << std::endl;
		}

		// Cleanup & Save resulting instance
		if (shutdownOptimizationInstance()) {
			Utility::printLine("INFO: Successfully ended optimization instance and saved results");
		}
		else {
			Utility::printLine("WARNING: Failure to properly end optimization instance!");
		}
	}
	catch (std::exception &e) {
		Utility::printLine("ERROR: " + std::string(e.what()));
		return false;
	}

	//Reset UI State
	// Deallocate thread pool
	delete this->myThreadPool_;

	this->isWorking = false;
	this->dlg->disableMainUI(!isWorking);
	return true;
}

// Method for handling the execution of an individual
// Input: indID - index value for individual being run to determine fitness (for multithreading will be the thread id as well)
// Output: returns false if a critical error occurs, true otherwise
//	individual in population index indID will have assigned fitness according to result from cc
//	lastImgWidth,lastImgHeight updated according to result from cc
//     shortenExposureFlag is set to true if fitness value is high enough
//     stopConditionsMetFlag is set to true if conditions met
bool GA_Optimization::runIndividual(int indID) {

	ImageController * curImage = NULL;
	// Setting up mutex locks
	std::unique_lock<std::mutex> consoleLock(this->consoleMutex, std::defer_lock);
	std::unique_lock<std::mutex> hardwareLock(this->hardwareMutex, std::defer_lock);
	std::unique_lock<std::mutex> scalerLock(this->slmScalersMutex, std::defer_lock);

	hardwareLock.lock();

	// Pre end the result for the individual if the stop flag has been raised while waiting
	if (this->dlg->stopFlag == true) {
		hardwareLock.unlock();
		return true;
	}

	// Write translated image to SLM boards, assumes there are as many boards as populations (accessing optBoards)
	scalerLock.lock(); // Scaler lock as the scaler is closely used with the slm
	int * genome;
	for (int i = 0; i < this->popCount; i++) {
		// Scale the individual genome to fit SLMs
		genome = this->population[i]->getGenome(indID);

		this->scalers[i]->TranslateImage(genome, this->slmScaledImages[i]); // Translate the vector genome into char array image
		// Write to SLM, getting the board position according to optBoards and correcting to 0 base
		this->sc->writeImageToBoard(this->optBoards[i]->board_id, this->slmScaledImages[i]);
	}
	scalerLock.unlock();

	// Acquire image
	curImage = this->cc->AcquireImage();

	hardwareLock.unlock(); // Now done with the hardware

	// Giving error and ends early if there is no data
	if (curImage == NULL) {
		consoleLock.lock();
		Utility::printLine("ERROR: Image Acquisition has failed!");
		consoleLock.unlock();
		return false;
	}
	// Using the image data from resulting image to determine the fitness by intensity of the image within circle of target radius
	double fitness = Utility::FindAverageValue(curImage->getRawData(), curImage->getWidth(), curImage->getHeight(), this->cc->targetRadius);
	// Get current exposure setting of camera (relative to initial)
	double exposureTimesRatio = this->cc->GetExposureRatio();	// needed for proper fitness value across changing exposure time

	// Record files
	if (this->logAllFiles || this->saveTimeVSFitness) {
		std::unique_lock<std::mutex> tVfLock(this->timeVsFitMutex, std::defer_lock);
		tVfLock.lock();
		this->timeVsFitnessFile << this->timestamp->MS_SinceStart() << "," << fitness*exposureTimesRatio << "," << this->cc->finalExposureTime << "," << exposureTimesRatio << std::endl;
		tVfLock.unlock();
	}
	//Save elite info of last generation
	if (indID == (population[0]->getSize() - 1)) {
		if ((this->saveEliteImages) && (this->curr_gen % this->saveEliteFrequency == 0)) {
			// Save Info
			std::unique_lock<std::mutex> tFileLock(this->tfileMutex, std::defer_lock);
			tFileLock.lock();
			this->tfile << this->curr_gen << "," << fitness*exposureTimesRatio << std::endl;
			tFileLock.unlock();
			// Save camera image
			std::string curTime = Utility::getCurDateTime(); // Get current time to use as timeStamp
			this->cc->saveImage(curImage, std::string(this->outputFolder + curTime + "_" + this->algorithm_name_ + "_Gen_" + std::to_string(this->curr_gen + 1) + "_Elite_Camera" + ".bmp"));
			// Save SLM image(s)
			scalerLock.lock();
			for (int popID = 0; popID < this->popCount; popID++) {
				scalers[popID]->TranslateImage(this->population[popID]->getGenome(this->population[popID]->getSize() - 1), this->slmScaledImages[popID]);
				cv::Mat m_ary = cv::Mat(this->sc->getBoardWidth(popID), this->sc->getBoardHeight(popID), CV_8UC1, this->slmScaledImages[popID]);
				cv::imwrite(this->outputFolder + curTime + "_" + this->algorithm_name_ + "_Gen_" + std::to_string(this->curr_gen + 1) + "_Elite_SLM_" + std::to_string(this->optBoards[popID]->board_id) + ".bmp", m_ary);
			}
			scalerLock.unlock();
		}
		// Also save the image as current best regardless
		std::unique_lock<std::mutex> imageLock(this->imageMutex, std::defer_lock);
		imageLock.lock();
		delete this->bestImage;
		this->bestImage = curImage;
		imageLock.unlock();
	}

	// Update fitness for the individuals
	for (int popID = 0; popID < this->population.size(); popID++) {
		this->population[popID]->setFitness(indID, fitness * exposureTimesRatio);
	}
	// If the fitness value is too high, flag that the exposure needs to be shortened
	if (fitness > this->maxFitnessValue) {
		std::unique_lock<std::mutex> exposureFlagLock(this->exposureFlagMutex, std::defer_lock);
		exposureFlagLock.lock();
		this->shortenExposureFlag = true;
		exposureFlagLock.unlock();
	}
	// If the pointer to current image does not also point to the best image we are safe to delete
	if (curImage != this->bestImage) {
		delete curImage;
	}
	return true;
}
