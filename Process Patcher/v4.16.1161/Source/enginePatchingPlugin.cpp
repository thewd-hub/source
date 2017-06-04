/////////////////////////////////////////////////////////////////////////////
// Executes a plugin that supports the process patcher plugin architecture //
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingPlugin.h"
#include "enginePatchingProcess.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

class PATCHINGENGINE_PLUGIN
{
	public:
		bool isDisplayDebugInformationEnabled;
};

PATCHINGENGINE_PLUGIN pPlugin;

// plugin initialisation flags
bool plugin1Initialised = true;
bool plugin2Initialised = true;
bool plugin3Initialised = true;
bool plugin4Initialised = true;
bool plugin5Initialised = true;

// plugin arguments
const int MAXIMUM_PLUGIN_ARGUMENTS = 1000;
int plugin1ArgumentCount = 0;
int plugin2ArgumentCount = 0;
int plugin3ArgumentCount = 0;
int plugin4ArgumentCount = 0;
int plugin5ArgumentCount = 0;
char* plugin1Arguments[MAXIMUM_PLUGIN_ARGUMENTS];
char* plugin2Arguments[MAXIMUM_PLUGIN_ARGUMENTS];
char* plugin3Arguments[MAXIMUM_PLUGIN_ARGUMENTS];
char* plugin4Arguments[MAXIMUM_PLUGIN_ARGUMENTS];
char* plugin5Arguments[MAXIMUM_PLUGIN_ARGUMENTS];

// initialises the plugin structures before calling the execution thread
bool initialisePluginExecution(void)
{
	// is debug information enabled
	pPlugin.isDisplayDebugInformationEnabled = ((ALLOW_DEBUG_INFORMATION) && (isDebugInformationEnabled()));
	int numberOfPlugins = getGlobalPatchingDetails()->numberOfPlugins;
	if (numberOfPlugins >= 1) plugin1Initialised = false;
	if (numberOfPlugins >= 2) plugin2Initialised = false;
	if (numberOfPlugins >= 3) plugin3Initialised = false;
	if (numberOfPlugins >= 4) plugin4Initialised = false;
	if (numberOfPlugins >= 5) plugin5Initialised = false;
	return true;
}

// uninitialise the plugin structures
bool uninitialisePluginExecution(void)
{
	return true;
}

// each plugin calls this function indirectly to initialise itself
void PluginFunction_PluginHasInitialised(int pluginId)
{
	if (pluginId == 1) plugin1Initialised = true;
	else if (pluginId == 2) plugin2Initialised = true;
	else if (pluginId == 3) plugin3Initialised = true;
	else if (pluginId == 4) plugin4Initialised = true;
	else if (pluginId == 5) plugin5Initialised = true;

	// signal waiting event when all plugins have initialised
	if ((plugin1Initialised) &&
		(plugin2Initialised) &&
		(plugin3Initialised) &&
		(plugin4Initialised) &&
		(plugin5Initialised))
	{
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_PLUGINS_INITIALISED, false);
	}
}

// returns the plugin settings
DWORD PluginFunction_GetPluginSettings(VOID)
{
	DWORD pluginSettings = 0;
	if (pPlugin.isDisplayDebugInformationEnabled) pluginSettings |= PLUGINSETTINGS_DEBUG;
	return pluginSettings;
}

// plugin 1 calls this function to initialise itself
VOID PluginFunction_PluginHasInitialised_1(VOID)
{
	PluginFunction_PluginHasInitialised(1);
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, false);
}

// plugin 2 calls this function to initialise itself
VOID PluginFunction_PluginHasInitialised_2(VOID)
{
	PluginFunction_PluginHasInitialised(2);
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, false);
}

// plugin 3 calls this function to initialise itself
VOID PluginFunction_PluginHasInitialised_3(VOID)
{
	PluginFunction_PluginHasInitialised(3);
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, false);
}

// plugin 4 calls this function to initialise itself
VOID PluginFunction_PluginHasInitialised_4(VOID)
{
	PluginFunction_PluginHasInitialised(4);
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, false);
}

// plugin 5 calls this function to initialise itself
VOID PluginFunction_PluginHasInitialised_5(VOID)
{
	PluginFunction_PluginHasInitialised(5);
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, false);
}

// return the argument count for plugin 1
DWORD PluginFunction_GetArgumentCount_1(VOID)
{
	return plugin1ArgumentCount;
}

// return the argument count for plugin 2
DWORD PluginFunction_GetArgumentCount_2(VOID)
{
	return plugin2ArgumentCount;
}

// return the argument count for plugin 3
DWORD PluginFunction_GetArgumentCount_3(VOID)
{
	return plugin3ArgumentCount;
}

// return the argument count for plugin 4
DWORD PluginFunction_GetArgumentCount_4(VOID)
{
	return plugin4ArgumentCount;
}

// return the argument count for plugin 5
DWORD PluginFunction_GetArgumentCount_5(VOID)
{
	return plugin5ArgumentCount;
}

// return the argument, based on index for plugin 1
LPSTR PluginFunction_GetArgument_1(DWORD argumentIndex)
{
	return (((argumentIndex < 0) || (argumentIndex >= (DWORD)plugin1ArgumentCount))? NULL : plugin1Arguments[argumentIndex]);
}

// return the argument, based on index for plugin 2
LPSTR PluginFunction_GetArgument_2(DWORD argumentIndex)
{
	return (((argumentIndex < 0) || (argumentIndex >= (DWORD)plugin2ArgumentCount))? NULL : plugin2Arguments[argumentIndex]);
}

// return the argument, based on index for plugin 3
LPSTR PluginFunction_GetArgument_3(DWORD argumentIndex)
{
	return (((argumentIndex < 0) || (argumentIndex >= (DWORD)plugin3ArgumentCount))? NULL : plugin3Arguments[argumentIndex]);
}

// return the argument, based on index for plugin 4
LPSTR PluginFunction_GetArgument_4(DWORD argumentIndex)
{
	return (((argumentIndex < 0) || (argumentIndex >= (DWORD)plugin4ArgumentCount))? NULL : plugin4Arguments[argumentIndex]);
}

// return the argument, based on index for plugin 5
LPSTR PluginFunction_GetArgument_5(DWORD argumentIndex)
{
	return (((argumentIndex < 0) || (argumentIndex >= (DWORD)plugin5ArgumentCount))? NULL : plugin5Arguments[argumentIndex]);
}

// used by plugins to get the event name from event id
LPSTR PluginFunction_GetEventName(DWORD eventId)
{
	return getWaitingEventName(eventId);
}

// used by plugins to wait for a specified plugin event
BOOL PluginFunction_WaitForEvent(DWORD eventId)
{
	HANDLE eventHandle = OpenEvent(EVENT_ALL_ACCESS, TRUE, getWaitingEventName(eventId));
	if (eventHandle == NULL) return FALSE;
	WaitForSingleObject(eventHandle, INFINITE);
	CloseHandle(eventHandle);
	return TRUE;
}

// used by plugins to set the specified plugin event
BOOL PluginFunction_SetEvent(DWORD eventId)
{
	HANDLE eventHandle = OpenEvent(EVENT_ALL_ACCESS, TRUE, getWaitingEventName(eventId));
	if (eventHandle == NULL) return FALSE;
	SetEvent(eventHandle);
	CloseHandle(eventHandle);
	return TRUE;
}

// used by plugins to reset the specified plugin event
BOOL PluginFunction_ResetEvent(DWORD eventId)
{
	HANDLE eventHandle = OpenEvent(EVENT_ALL_ACCESS, TRUE, getWaitingEventName(eventId));
	if (eventHandle == NULL) return FALSE;
	ResetEvent(eventHandle);
	CloseHandle(eventHandle);
	return TRUE;
}

// used by plugins to read bytes from the process memory
BOOL PluginFunction_ReadMemoryAddress(HANDLE processHandle, DWORD memoryAddress, DWORD numberOfBytes, UCHAR* memoryBuffer, BOOL alterMemoryAccessProtection)
{
	return ((readMemoryAddress(processHandle, UlongToPtr(memoryAddress), numberOfBytes, memoryBuffer, ((alterMemoryAccessProtection == TRUE) ? true : false))) ? TRUE : FALSE);
}

// used by plugins to write bytes to the process memory
BOOL PluginFunction_WriteMemoryAddress(HANDLE processHandle, DWORD memoryAddress, DWORD numberOfBytes, UCHAR* memoryBuffer, BOOL alterMemoryAccessProtection)
{
	return ((writeMemoryAddress(processHandle, UlongToPtr(memoryAddress), numberOfBytes, memoryBuffer, ((alterMemoryAccessProtection == TRUE) ? true : false))) ? TRUE : FALSE);
}

// used indirectly by plugins to display a message in the console window
VOID PluginFunction_DisplayConsoleMessage(unsigned long pluginSectionNumber, LPSTR outputMessageString)
{
	if (!ULib.StringLibrary.isString(outputMessageString, 1))
	{
		displayConsoleMessage("");
	}
	else
	{
		int outputMessageLength = ULib.StringLibrary.getStringLength(outputMessageString);
		char* outputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap(outputMessageLength + 14);
		sprintf_s(outputMessage, outputMessageLength + 12, "(P%d) %s", pluginSectionNumber, outputMessageString);
		displayConsoleMessage(outputMessage);
		ULib.HeapLibrary.freePrivateHeap(outputMessage);
	}
}

// used by plugin 1 to display a message in the console window
VOID PluginFunction_DisplayConsoleMessage_1(LPSTR outputMessageString)
{
	PluginFunction_DisplayConsoleMessage(1, outputMessageString);
}

// used by plugin 2 to display a message in the console window
VOID PluginFunction_DisplayConsoleMessage_2(LPSTR outputMessageString)
{
	PluginFunction_DisplayConsoleMessage(2, outputMessageString);
}

// used by plugin 3 to display a message in the console window
VOID PluginFunction_DisplayConsoleMessage_3(LPSTR outputMessageString)
{
	PluginFunction_DisplayConsoleMessage(3, outputMessageString);
}

// used by plugin 4 to display a message in the console window
VOID PluginFunction_DisplayConsoleMessage_4(LPSTR outputMessageString)
{
	PluginFunction_DisplayConsoleMessage(4, outputMessageString);
}

// used by plugin 5 to display a message in the console window
VOID PluginFunction_DisplayConsoleMessage_5(LPSTR outputMessageString)
{
	PluginFunction_DisplayConsoleMessage(5, outputMessageString);
}

// executes a plugin that supports the process patcher
// plugin architecture, using the standard API calls
// (Argument - PATCHINGDETAILS_PLUGIN)
void executePluginThread(LPVOID args)
{
	__try
	{
		PATCHINGDETAILS_PLUGIN* pPluginDetails = (PATCHINGDETAILS_PLUGIN*)args;

		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* shortPluginFilename = ULib.FileLibrary.stripPathFromFilename(pPluginDetails->fileName, true);

		// display debug information
		if (pPlugin.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Executing Plugin (%s)", shortPluginFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		// attempt to load the plugin into memory
		HMODULE pluginModule = ULib.FileLibrary.moduleLoadLibrary(pPluginDetails->fileName);
		if (pluginModule == NULL) displayErrorMessage(ERROR_PATCHING_PLUGIN_GENERAL, true);

		// get the base APIs from the plugin module
		GETPLUGINVERSIONINFORMATION GetPluginVersionInformation = (GETPLUGINVERSIONINFORMATION)GetProcAddress(pluginModule, "GetPluginVersionInformation");
		EXECUTEPLUGIN ExecutePlugin = (EXECUTEPLUGIN)GetProcAddress(pluginModule, "ExecutePlugin");
		GETLASTPLUGINERROR GetLastPluginError = (GETLASTPLUGINERROR)GetProcAddress(pluginModule, "GetLastPluginError");

		// check that the base API exports have been found
		if (GetPluginVersionInformation == NULL) displayErrorMessage(ERROR_PATCHING_PLUGIN_EXPORTS_GETVERSION, true);
		if (ExecutePlugin == NULL) displayErrorMessage(ERROR_PATCHING_PLUGIN_EXPORTS_EXECUTEPLUGIN, true);
		if (GetLastPluginError == NULL) displayErrorMessage(ERROR_PATCHING_PLUGIN_EXPORTS_GETERROR, true);

		PLUGIN_VERSION_INFORMATION* pluginVersionInformation = (PLUGIN_VERSION_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PLUGIN_VERSION_INFORMATION));

		// get plugin version information
		GetPluginVersionInformation(pluginVersionInformation);

		// not supported (v1.00)
		if (pluginVersionInformation->apiPluginVersion == API_PLUGIN_VERSION_0100) displayErrorMessage(ERROR_PATCHING_PLUGIN_APIVERSION_0100, true);
		// unrecognised version
		else if ((pluginVersionInformation->apiPluginVersion != API_PLUGIN_VERSION_0101) &&
			(pluginVersionInformation->apiPluginVersion != API_PLUGIN_VERSION_0102) &&
			(pluginVersionInformation->apiPluginVersion != API_PLUGIN_VERSION_0103))
		{
			displayErrorMessage(ERROR_PATCHING_PLUGIN_APIVERSION, "Supports Plugin API v1.01 - v1.03", true);
		}

		// execution information and plugin functions
		PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation = (PLUGIN_EXECUTION_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PLUGIN_EXECUTION_INFORMATION));
		PLUGIN_FUNCTIONS_TARGETPROCESS* pluginFunctions_TargetProcess = (PLUGIN_FUNCTIONS_TARGETPROCESS*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PLUGIN_FUNCTIONS_TARGETPROCESS));
		PLUGIN_FUNCTIONS_CHILDPROCESS* pluginFunctions_ChildProcess = (PLUGIN_FUNCTIONS_CHILDPROCESS*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PLUGIN_FUNCTIONS_CHILDPROCESS));
		PLUGIN_FUNCTIONS_WAITINGEVENTS* pluginFunctions_WaitingEvents = (PLUGIN_FUNCTIONS_WAITINGEVENTS*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PLUGIN_FUNCTIONS_WAITINGEVENTS));
		PLUGIN_FUNCTIONS_OTHERS* pluginFunctions_Others = (PLUGIN_FUNCTIONS_OTHERS*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PLUGIN_FUNCTIONS_OTHERS));

		// v1.01+
		if (pluginVersionInformation->apiPluginVersion >= API_PLUGIN_VERSION_0101)
		{
			// target process function declarations
			pluginFunctions_TargetProcess->GetProcessFilename = &getTargetProcessFilename;
			pluginFunctions_TargetProcess->GetProcessId = &getTargetProcessId;
			pluginFunctions_TargetProcess->GetProcessHandle = &getTargetProcessHandle;
			pluginFunctions_TargetProcess->GetThreadId = &getTargetProcessThreadId;
			pluginFunctions_TargetProcess->GetThreadHandle = &getTargetProcessThreadHandle;
			pluginFunctions_TargetProcess->GetProcessSessionId = &getTargetProcessSessionId;
			pluginExecutionInformation->functionsTargetProcess = pluginFunctions_TargetProcess;

			// child process function declarations
			pluginFunctions_ChildProcess->GetProcessFilename = &getChildProcessFilename;
			pluginFunctions_ChildProcess->GetProcessId = &getChildProcessId;
			pluginFunctions_ChildProcess->GetProcessHandle = &getChildProcessHandle;
			// v1.02+
			if (pluginVersionInformation->apiPluginVersion >= API_PLUGIN_VERSION_0102)
			{
				pluginFunctions_ChildProcess->GetThreadId = &getChildProcessThreadId;
				pluginFunctions_ChildProcess->GetThreadHandle = &getChildProcessThreadHandle;
				pluginFunctions_ChildProcess->GetProcessSessionId = &getChildProcessSessionId;
			}
			pluginExecutionInformation->functionsChildProcess = pluginFunctions_ChildProcess;

			// waiting events function declarations
			pluginFunctions_WaitingEvents->GetEventName = &PluginFunction_GetEventName;
			pluginFunctions_WaitingEvents->WaitForEvent = &PluginFunction_WaitForEvent;
			pluginFunctions_WaitingEvents->SetEvent = &PluginFunction_SetEvent;
			pluginFunctions_WaitingEvents->ResetEvent = &PluginFunction_ResetEvent;
			pluginExecutionInformation->functionsWaitingEvents = pluginFunctions_WaitingEvents;

			// other function declarations
			pluginFunctions_Others->ReadMemoryAddress = &PluginFunction_ReadMemoryAddress;
			pluginFunctions_Others->WriteMemoryAddress = &PluginFunction_WriteMemoryAddress;
			if (pPluginDetails->sectionNumber == 1) pluginFunctions_Others->DisplayConsoleMessage = &PluginFunction_DisplayConsoleMessage_1;
			else if (pPluginDetails->sectionNumber == 2) pluginFunctions_Others->DisplayConsoleMessage = &PluginFunction_DisplayConsoleMessage_2;
			else if (pPluginDetails->sectionNumber == 3) pluginFunctions_Others->DisplayConsoleMessage = &PluginFunction_DisplayConsoleMessage_3;
			else if (pPluginDetails->sectionNumber == 4) pluginFunctions_Others->DisplayConsoleMessage = &PluginFunction_DisplayConsoleMessage_4;
			else if (pPluginDetails->sectionNumber == 5) pluginFunctions_Others->DisplayConsoleMessage = &PluginFunction_DisplayConsoleMessage_5;
			pluginExecutionInformation->functionsOther = pluginFunctions_Others;

			// plugin has initialised
			if (pPluginDetails->sectionNumber == 1) pluginExecutionInformation->PluginHasInitialised = &PluginFunction_PluginHasInitialised_1;
			else if (pPluginDetails->sectionNumber == 2) pluginExecutionInformation->PluginHasInitialised = &PluginFunction_PluginHasInitialised_2;
			else if (pPluginDetails->sectionNumber == 3) pluginExecutionInformation->PluginHasInitialised = &PluginFunction_PluginHasInitialised_3;
			else if (pPluginDetails->sectionNumber == 4) pluginExecutionInformation->PluginHasInitialised = &PluginFunction_PluginHasInitialised_4;
			else if (pPluginDetails->sectionNumber == 5) pluginExecutionInformation->PluginHasInitialised = &PluginFunction_PluginHasInitialised_5;

			// plugin settings
			pluginExecutionInformation->GetPluginSettings = &PluginFunction_GetPluginSettings;

			// plugin arguments
			if (pPluginDetails->sectionNumber == 1)
			{
				ULib.StringLibrary.getStringElementsFromString(pPluginDetails->argumentsToForward, plugin1Arguments, &plugin1ArgumentCount, MAXIMUM_PLUGIN_ARGUMENTS, ' ');
				pluginExecutionInformation->GetPluginArgumentCount = &PluginFunction_GetArgumentCount_1;
				pluginExecutionInformation->GetPluginArgument = &PluginFunction_GetArgument_1;
			}
			else if (pPluginDetails->sectionNumber == 2)
			{
				ULib.StringLibrary.getStringElementsFromString(pPluginDetails->argumentsToForward, plugin2Arguments, &plugin2ArgumentCount, MAXIMUM_PLUGIN_ARGUMENTS, ' ');
				pluginExecutionInformation->GetPluginArgumentCount = &PluginFunction_GetArgumentCount_2;
				pluginExecutionInformation->GetPluginArgument = &PluginFunction_GetArgument_2;
			}
			else if (pPluginDetails->sectionNumber == 3)
			{
				ULib.StringLibrary.getStringElementsFromString(pPluginDetails->argumentsToForward, plugin3Arguments, &plugin3ArgumentCount, MAXIMUM_PLUGIN_ARGUMENTS, ' ');
				pluginExecutionInformation->GetPluginArgumentCount = &PluginFunction_GetArgumentCount_3;
				pluginExecutionInformation->GetPluginArgument = &PluginFunction_GetArgument_3;
			}
			else if (pPluginDetails->sectionNumber == 4)
			{
				ULib.StringLibrary.getStringElementsFromString(pPluginDetails->argumentsToForward, plugin4Arguments, &plugin4ArgumentCount, MAXIMUM_PLUGIN_ARGUMENTS, ' ');
				pluginExecutionInformation->GetPluginArgumentCount = &PluginFunction_GetArgumentCount_4;
				pluginExecutionInformation->GetPluginArgument = &PluginFunction_GetArgument_4;
			}
			else if (pPluginDetails->sectionNumber == 5)
			{
				ULib.StringLibrary.getStringElementsFromString(pPluginDetails->argumentsToForward, plugin5Arguments, &plugin5ArgumentCount, MAXIMUM_PLUGIN_ARGUMENTS, ' ');
				pluginExecutionInformation->GetPluginArgumentCount = &PluginFunction_GetArgumentCount_5;
				pluginExecutionInformation->GetPluginArgument = &PluginFunction_GetArgument_5;
			}
		}

		// get plugin version
		float pluginVersion = 0;
		if (pluginVersionInformation->pluginVersion != NULL)
		{
			char* pluginVersionString = (char*)ULib.HeapLibrary.allocPrivateHeap();
			if (pluginVersionInformation->apiPluginVersion >= API_PLUGIN_VERSION_0103) sprintf_s(pluginVersionString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%d.%02d", pluginVersionInformation->pluginVersion >> 8, pluginVersionInformation->pluginVersion & 0xFF);
			else ULib.StringLibrary.copyString(pluginVersionString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, (LPSTR)ULongToPtr(pluginVersionInformation->pluginVersion));
			pluginVersion = ULib.StringLibrary.getFloatFromString(pluginVersionString);
			ULib.HeapLibrary.freePrivateHeap(pluginVersionString);
		}

		// display plugin information
		if (pluginVersionInformation->pluginDescription != NULL)
		{
			char* displayPluginInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();

			// display plugin description & version information
			if (pluginVersion > 0) sprintf_s(displayPluginInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Executing Plugin : %s v%.2f (P%d)", pluginVersionInformation->pluginDescription, pluginVersion, pPluginDetails->sectionNumber);
			else sprintf_s(displayPluginInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Executing Plugin : %s (P%d)", pluginVersionInformation->pluginDescription, pPluginDetails->sectionNumber);
			if (isDebugInformationEnabled()) sprintf_s(displayPluginInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s [API:v%d.%02d]", displayPluginInformation, pluginVersionInformation->apiPluginVersion >> 8, pluginVersionInformation->apiPluginVersion & 0xFF);
			displayConsoleMessage(displayPluginInformation, true, false);

			ULib.HeapLibrary.freePrivateHeap(displayPluginInformation);
		}

		// check plugin version information (if required)
		if ((pluginVersion > 0) && (pPluginDetails->minimumPluginVersionRequired > 0))
		{
			if (pluginVersion < pPluginDetails->minimumPluginVersionRequired)
			{
				char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s v%.2f+ plugin is required", pluginVersionInformation->pluginDescription, pPluginDetails->minimumPluginVersionRequired);
				displayCustomErrorMessage(errorMessage, true);
				ULib.HeapLibrary.freePrivateHeap(errorMessage);
			}
		}

		// execute the plugin module
		if (!ExecutePlugin(pluginExecutionInformation)) displayErrorMessage(ERROR_PATCHING_PLUGIN_EXECUTION, GetLastPluginError(), true);

		// display debug information
		if (pPlugin.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Terminating Plugin (%s)", shortPluginFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		ULib.HeapLibrary.freePrivateHeap(pluginFunctions_Others);
		ULib.HeapLibrary.freePrivateHeap(pluginFunctions_WaitingEvents);
		ULib.HeapLibrary.freePrivateHeap(pluginFunctions_ChildProcess);
		ULib.HeapLibrary.freePrivateHeap(pluginFunctions_TargetProcess);
		ULib.HeapLibrary.freePrivateHeap(pluginExecutionInformation);
		ULib.HeapLibrary.freePrivateHeap(pluginVersionInformation);

		ULib.HeapLibrary.freePrivateHeap(debugMessage);
		ULib.HeapLibrary.freePrivateHeap(shortPluginFilename);
	}
	__except(handleCriticalException(GetExceptionInformation(), SECTIONTYPE_PLUGIN))
	{
	}

	ExitThread(ERROR_SUCCESS);
}