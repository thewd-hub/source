//////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for child process type patches. //
// Stores the information in the PATCHINGDETAILS_CHILDPROCESS structure //
//////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingChildProcess.h"

// attempt to parse the valid parameters for the child process types
bool parseChildProcessParameters(char* stringBuffer, PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails)
{
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkChildProcessParameters(PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails)
{
	// ignore all parsing errors
	if (pChildProcessDetails->ignoreParsingErrors) return true;
	// check section number
	if (pChildProcessDetails->sectionNumber <= 0) return false;
	return true;
}