////////////////////
// GA_Optimization.h - header file for the base genetic algorithm optimization class that inherits from virtual optimization class
//					 - GA_Optimization incorporates population pointer and implements general GA runOptimization behavior to reduce duplicate code
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#include "Optimization.h"
#include "Population.h"

#include "threadPool.h"

#ifndef GA_OPTIMIZATION_H_
#define GA_OPTIMIZATION_H_

class GA_Optimization : public Optimization {
protected:
	// Vector to hold genetic algorithm's populations
	std::vector<Population<int>*> population;
	threadPool * myThreadPool_;

	int populationSize;	// Size of the populations being used (number of individuals in a population class)
	int popCount;		// Number of populations working with (should be equal to number of boards being optimized)
	int eliteSize;		// Number of elite individuals within the population (should be less than populationSize)
	int curr_gen;		// Current generation being evaluated (start at 0)
	int indThreadCount;	// Number of threads to use when evaluating individuals
	int gaPoolThreadCount;	// Number of threads to use when generating the next generation

	// GA specific output file stream
	std::ofstream timePerGenFile;		// Record time it took to perform each generation during optimization

	// Mutexes to protect critical sections when multithreading
	std::mutex hardwareMutex;						// Mutex to protect critical section of accessing SLM and Camera data
	std::mutex consoleMutex, imageMutex;			// Mutex to protect console output and bestImage values
	std::mutex tfileMutex, timeVsFitMutex;			// Mutex to protect file i/o
	std::mutex exposureFlagMutex;					// Mutex to protect important flag(s)
	std::mutex slmScalersMutex; // Mutex to protect the usage of the the SLM scalers (which are used in both for hardware and in image output)

	// Method for handling the execution of an individual
	// Input:
	//		indID - index value for individual being run to determine fitness (for multithreading will be the thread id as well)
	//		popID - index value for population being run to determine fitness
	// Output: returns false if a critical error occurs, true otherwise
	//		individual in population index indID will have assigned fitness according to result from cc
	//		lastImgWidth,lastImgHeight updated according to result from cc
	//		shortenExposureFlag is set to true if fitness value is high enough
	//		stopConditionsMetFlag is set to true if conditions met
	bool runIndividual(int indID);

public:
	// Constructor - inherits from base class
	GA_Optimization(MainDialog* dlg, CameraController* cc, SLMController* sc) : Optimization(dlg, cc, sc) {
	};

	// Run genetic algorithm (used by both SGA and uGA)
	bool runOptimization();
};

#endif
