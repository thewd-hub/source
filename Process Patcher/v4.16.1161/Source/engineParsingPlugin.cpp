////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for supported plugins.    //
// Stores the information in the PATCHINGDETAILS_PLUGIN structure //
////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingPlugin.h"

// attempt to parse the valid parameters for the supported plugins
bool parsePluginParameters(char* stringBuffer, PATCHINGDETAILS_PLUGIN* pPluginDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// arguments to forward to the plugin
	if ((parsingResult = parseStringParameter("Arguments", pPluginDetails->argumentsToForward, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pPluginDetails->argumentsToForward = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pPluginDetails->argumentsToForward, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// minimum plugin version required for this patch to succeed
	if ((parsingResult = parseStringParameter("PluginVersion", (unsigned long)pPluginDetails->minimumPluginVersionRequired, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pPluginDetails->minimumPluginVersionRequired = ULib.StringLibrary.getFloatFromString(parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkPluginParameters(PATCHINGDETAILS_PLUGIN* pPluginDetails)
{
	// ignore all parsing errors
	if (pPluginDetails->ignoreParsingErrors) return true;
	// check section number
	if (pPluginDetails->sectionNumber <= 0) return false;
	return true;
}