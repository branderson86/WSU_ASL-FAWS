////////////////////
// BruteForce_Optimization.h - header file for BF child class that encapsulates the BF optimization algorithm
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

#ifndef BRUTE_FORCE_OPTIMIZATION_H_
#define BRUTE_FORCE_OPTIMIZATION_H_

#include "Optimization.h"

class BruteForce_Optimization : public Optimization {
	unsigned int phaseResolution;
	std::ofstream lmaxfile;
	std::ofstream rtime;

	// Once finished, contains the resulting optimized SLM images for all the boards used
	std::vector<int*> finalImages_;
	// Record of best fitness overall during optimization
	double allTimeBestFitness;
public:
	// Constructor - inherits from base class
	BruteForce_Optimization(MainDialog* dlg, CameraController* cc, SLMController* sc) : Optimization(dlg, cc, sc) {
		this->algorithm_name_ = "IA";
	};

	// Method for executing the optimization
	// Output: returns true if successful ran without error, false if error occurs
	bool runOptimization();

	bool setupInstanceVariables();
	bool shutdownOptimizationInstance();

	// Run individual for BF refers to the board being used
	// Input: boardID - index of SLM board being used (0 based)
	// Output: Result stored in slmImg
	bool runIndividual(int boardID);

	// Initialize slmImg with 0's
	void setBlankSlmImg(int* slmImg);
};

#endif
