////////////////////
// uGA_Optimization.h - handler for micro genetic algorithm that inherits from base GA_Optimization class
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#ifndef UGA_OPTIMIZATION_H_
#define UGA_OPTIMIZATION_H_

#include "GA_Optimization.h"
#include "uGA_Population.h"

class uGA_Optimization : public GA_Optimization {
	// Method to setup specific properties for uGA
	bool setupInstanceVariables();

	// Method to clean up & save resulting uGA instance
	bool shutdownOptimizationInstance();

public:
	// Constructor - inherits from base class
	uGA_Optimization(MainDialog* dlg, CameraController* cc, SLMController* sc) : GA_Optimization(dlg, cc, sc) {
		this->algorithm_name_ = "uGA";
	};
};

#endif
