////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for module type patches.  //
// Stores the information in the PATCHINGDETAILS_MODULE structure //
////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingModule.h"

// attempt to parse the valid parameters for the module types
bool parseModuleParameters(char* stringBuffer, PATCHINGDETAILS_MODULE* pModuleDetails)
{
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkModuleParameters(PATCHINGDETAILS_MODULE* pModuleDetails)
{
	// ignore all parsing errors
	if (pModuleDetails->ignoreParsingErrors) return true;
	// check section number
	if (pModuleDetails->sectionNumber <= 0) return false;
	return true;
}