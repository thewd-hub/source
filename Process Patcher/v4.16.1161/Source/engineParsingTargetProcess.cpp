/////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for the target process. Stores //
// the information in the PATCHINGDETAILS_TARGETPROCESS structure      //
/////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingTargetProcess.h"

// attempt to parse the valid parameters for the target process type
bool parseTargetProcessParameters(char* stringBuffer, PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// display name for the process to execute
	if ((parsingResult = parseStringParameter("DisplayName", pTargetProcessDetails->displayName, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->displayName = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pTargetProcessDetails->displayName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// arguments which are forwarded to the target process
	if ((parsingResult = parseStringParameter("Arguments", pTargetProcessDetails->argumentsToForward, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->argumentsToForward = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pTargetProcessDetails->argumentsToForward, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// opens an existing process (i.e. a process that is already executing)
	if ((parsingResult = parseStringParameter("OpenExistingProcess", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->openExistingProcess = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// does the target process create a child process which is the actual target application
	if ((parsingResult = parseStringParameter("CreatesChildProcess", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->createsChildProcess = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// create the target process and suspend the primary thread
	if ((parsingResult = parseStringParameter("CreateSuspendedProcess", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->createSuspendedProcess = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// wait for a window to be displayed that matches the defined
	// name before attempting to patch the process (local window)
	if ((parsingResult = parseStringParameter("WaitForWindowName", pTargetProcessDetails->waitForLocalWindowName, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->waitForLocalWindowName = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pTargetProcessDetails->waitForLocalWindowName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// wait for a window to be displayed that matches the defined
	// name before attempting to patch the process (global window)
	if ((parsingResult = parseStringParameter("WaitForGlobalWindowName", pTargetProcessDetails->waitForGlobalWindowName, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->waitForGlobalWindowName = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pTargetProcessDetails->waitForGlobalWindowName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// wait a number of seconds before attempting to patch the process
	if ((parsingResult = parseStringParameter("WaitForTimeDelay", pTargetProcessDetails->waitForTimeDelay, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->waitForTimeDelay = ULib.StringLibrary.getDWORDFromString(parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// wait for a user response before attempting to patch the process
	if ((parsingResult = parseStringParameter("UserNotify", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->displayUserNotifyBox = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// message to display to the user, giving instructions on when to continue the patch
	if ((parsingResult = parseStringParameter("UserNotify-Message", pTargetProcessDetails->displayUserNotifyBoxMessage, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->displayUserNotifyBoxMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pTargetProcessDetails->displayUserNotifyBoxMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// message to display to the user, giving instructions on when to continue the patch (Old Parameter)
	if ((parsingResult = parseStringParameter("UserNotifyMessage", pTargetProcessDetails->displayUserNotifyBoxMessage, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pTargetProcessDetails->displayUserNotifyBoxMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pTargetProcessDetails->displayUserNotifyBoxMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkTargetProcessParameters(PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails)
{
	// ignore all parsing errors
	if (pTargetProcessDetails->ignoreParsingErrors) return true;
	// check section number
	if (pTargetProcessDetails->sectionNumber <= 0) return false;

	// has display name been defined
	if ((pTargetProcessDetails->displayName == NULL) || (pTargetProcessDetails->displayName[0] == NULL))
	{
		char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find the DisplayName parameter - Target Process Section");
		setParsingErrorMessage(errorMessage);
		ULib.HeapLibrary.freePrivateHeap(errorMessage);
		return false;
	}

	return true;
}