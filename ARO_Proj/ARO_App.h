////////////////////
// ARO_App.h : main header file for the ARO Project application 
// here we inherit from the base class CWinApp to begin with a 
// Windows App template
////////////////////

#include "MainDialog.h"

class ARO_AppApp : public CWinApp
{
public:
	// [CONSTRUCTOR(S)]
	ARO_AppApp();

	// [INITIALIZER]
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

// Commit to singleton design
extern ARO_AppApp theApp;
