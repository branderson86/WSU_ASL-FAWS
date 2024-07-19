////////////////////
// CameraController.h - file to determine which CameraController wrapper to include for building
// When using camera controller, this header file should be what you include rather than from a specific SDK!
// Last edited: 08/02/2021 by Andrew O'Kins
////////////////////

// Forward declarations of the classes to be defined
class CameraController;
class ImageController;

// Define which version to build with (Spinnaker or PICam?) cannot be both
#define USE_PICAM
//#define USE_SPINNAKER

// Spinnaker version
#ifdef USE_SPINNAKER
	#include "CameraControllerSpinnaker.h"
	#include "ImageControllerSpinnaker.h"
	#ifdef USE_PICAM// Performing additional check to see if other version is also being includes (which would be bad!)
		#error "Cannot have Spinnaker and PICam versions in same build!"
	#endif
#endif
// PICam version
#ifdef USE_PICAM
	#include "CameraControllerPICam.h"
	#include "ImageControllerPICam.h"
	#ifdef USE_SPINNAKER // Performing additional check to see if other version is also being includes (which would be bad!)
		#error "Cannot have Spinnaker and PICam versions in same build!"
	#endif
#endif
