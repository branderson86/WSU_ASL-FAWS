////////////////////
// Optimization.h - header file for the base optimization class
// Last edited: 08/12/2021 by Andrew O'Kins
////////////////////

#ifndef OPTIMIZATION_H_
#define OPTIMIZATION_H_

#include <string> // outputFolder property
#include <fstream>// used to export information to file for debugging
#include <vector> // For managing ind_threads
#include <thread> // For ind_threads used in runOptimization
#include <mutex>  // Mutexes to protect identified critical sections

#include "MainDialog.h"			// used for UI reference
#include "CameraController.h"	// pointer to access custom interface with camera and images
#include "SLMController.h"		// pointer to access custom interface with slm
#include "Timing.h"				// contains time keeping functions
#include "ImageScaler.h"		// changes size of image to fit slm
#include "CameraDisplay.h"		// display Camera & SLM images to the user in distinct windows

class Optimization {
protected:
	std::string algorithm_name_; // String that gives an identifying label for the algorithm being run ("uGA" for example)
	//Object references
	MainDialog* dlg;		// The GUI to draw the desired settings from
	CameraController* cc;	// Interface with camera hardware
	SLMController* sc;		// Interface with SLM hardware

	//Base algorithm parameters
	double acceptedSimilarity = .97;  // images considered the same when reach this threshold (has to be less than 1)
	double maxFitnessValue = 200;  // max allowed fitness value - when reached exposure is halved (TODO: check this feature)
	double maxGenenerations = 3000; // max number of generations to perform

	//Base algorithm stop conditions
	double fitnessToStop = 0;
	double minSecondsToStop = 60;
	double maxSecondsToStop = 0;
	double genEvalToStop = 0; // minimum number of generations to do

	//Preference-type parameters
	bool displayCamImage = true;  // TRUE -> opens a window showing the camera image
	bool displaySLMImage = false; // TURE -> opens window showing best for currently being optimized SLM
	bool logAllFiles = true;	 // TRUE -> output all logging files to record performance
	bool saveEliteImages = false; // TRUE -> save images of the fittest individual of each gen
	int  saveEliteFrequency = 10;	 // How often per generation to save elite images if enabled
	bool saveResultImages = true;	 // TRUE -> Will save results
	bool saveParametersPref = true;  // TRUE -> Output parameters used
	bool saveTimeVSFitness = true;	 // TRUE -> Output timing performance
	bool saveExposureShorten = true; // TRUE -> Output when exposure is shortened
	bool multithreadEnable = true;  // TRUE -> use multithreading
	bool skipEliteReevaluation = false; // TRUE -> Will skip running elite individuals that should already have a fitness value

	//Instance variables (used during optimization process)
	// Values assigned within setupInstanceVariables(), then if needed cleared in shutdownOptimizationInstance()
	bool isWorking = false;		// true if currently actively running the optimization algorithm
	bool usingHardware = false; // debug flag of using hardware currently in a run of an individual (to know if accidentally having two threads use hardware at once!)
	bool shortenExposureFlag;   // Set to true by individual if fitness is too high
	bool stopConditionsMetFlag; // Set to true if a stop condition was reached by one of the individuals
	CameraDisplay * camDisplay; // Display for camera
	std::vector<CameraDisplay *> slmDisplayVector; // Display for SLM (currently [June 24th 2021] only board at index 0)
	TimeStampGenerator * timestamp; // Timer to track and store elapsed time as the algorithm executes

	ImageController * bestImage; // Current camera image found to have best resulting fitness from elite individuals
	std::vector<ImageScaler*> scalers; // Image scalers for each SLM (each SLM may have different dimensions so can't have just one)
	std::vector<unsigned char*> slmScaledImages; // To easily store the scaled images from individual to what will be written
	std::vector<SLM_Board*> optBoards; // Vector to hold pointers of boards taken from SLMController that are to be optimized (do not delete the boards here!)

	// Logging file streams
	std::ofstream tfile;				// Record elite individual progress over generations
	std::ofstream timeVsFitnessFile;	// Recording general fitness progress
	std::ofstream efile;				// Exposure file to record when exposure is shortened
	std::string outputFolder;	// string containing path to folder to save outputs to

	// Methods for use in runOptimization()
	// Pull GUI settings to set parameters
	bool prepareStopConditions();
	// Setup camera and slm controllers
	bool prepareSoftwareHardware();
	// Pull GUI settings for output settings such as save images
	bool prepareOutputSettings();

	// Creates a scaler with given SLMController
	// Input: slmImg - array that will be storing scalled image to be initialized with 0's
	//		  slmNum - index for board that will be scaling to, 0 based (defaults to 0)
	// Output: returns scaler that will scale
	ImageScaler* setupScaler(unsigned char *slmImg, int slmNum);

	// Check to see if we have reached the end condition
	// Input:
	//		curFitness - the current fitness to compare against fitnessToStop
	//		curSecPassed - current passed time in seconds to compare against secondsToStop
	//		curGenerations - the current generation that has been evaluated to compare against genEvalsToStop
	// Output: returns true if either input is greater than compared against OR the MainDialog's stopFlag has been set to true
	bool const stopConditionsReached(double curFitness, double curSecPassed, double curGenerations);

	// Save the various setting parameters used in this optimization
	// Stores the values with formatting in "this->outputFolder/[time]_[optType]_Optimization_Parameters.txt"
	// Input: time - the current time as a string label
	void saveParameters(std::string time);

	// Methods relying on implementation from child classes
	virtual bool setupInstanceVariables() = 0;		 // Setting up properties used in runOptimization()
	virtual bool shutdownOptimizationInstance() = 0; // Cleaning up properties as well as final saving for runOptimization()
	virtual bool runIndividual(int indID) = 0;		 // Method for handling the execution of an individual
public:
	// Constructor
	Optimization(MainDialog* dlg, CameraController* cc, SLMController* sc);

	// Method for performing the optimization
	// Output: returns true if successful ran without error, false if error occurs
	virtual bool runOptimization() = 0;
};

#endif
