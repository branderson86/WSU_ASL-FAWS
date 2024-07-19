////////////////////
// SGA_Optimization.h - handler for simple genetic algorithm that inherits from base GA_Optimization class
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#ifndef SGA_OPTIMIZATION_H_
#define SGA_OPTIMIZATION_H_

#include "GA_Optimization.h"
#include "SGA_Population.h"

class SGA_Optimization : public GA_Optimization {
	// Method to setup specific properties for SGA
	bool setupInstanceVariables();

	// Method to clean up & save resulting SGA instance
	bool shutdownOptimizationInstance();

public:
	// Constructor - inherits from base class
	SGA_Optimization(MainDialog* dlg, CameraController* cc, SLMController* sc) : GA_Optimization(dlg, cc, sc) {
		this->algorithm_name_ = "SGA";
	};
};

#endif
