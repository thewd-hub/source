///////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for all patching types.          //
// Stores the information in the corresponding PATCHINGDETAILS structure //
///////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingBase.h"

// attempt to parse the valid parameters for all the patching types
bool parseBaseParameters(char* stringBuffer, PATCHINGDETAILS_BASE* pBaseDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// used to determine whether to ignore any parsing engine error
	// for a compulsory parameter within a particular section.
	// This parameter disables this section error checking
	if ((parsingResult = parseStringParameter("IgnoreParsingErrors", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if ((parsingResult == PARSINGENGINE_PARSINGOK) && (ULib.StringLibrary.isStringMatch(parsingString, "true")))
	{
		pBaseDetails->ignoreParsingErrors = true;
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// this parameter disables the current section
	if ((parsingResult = parseStringParameter("IsDisabled", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pBaseDetails->isDisabled = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkBaseParameters(PATCHINGDETAILS_BASE* pBaseDetails)
{
	// ignore all parsing errors
	if (pBaseDetails->ignoreParsingErrors) return true;
	// check section number
	if (pBaseDetails->sectionNumber <= 0) return false;
	return true;
}