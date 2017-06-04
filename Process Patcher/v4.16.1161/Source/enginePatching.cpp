////////////////////////////////////////////////////////////
// Handles the patching threads that are created to patch //
// the target process, modules and execute plugin modules //
////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingRegistryEntry.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingModule.h"
#include "enginePatchingPlugin.h"
#include "enginePatchingProcess.h"
#include "enginePatchingRegistryEntry.h"
#include "enginePatchingSupport.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

char* eventNames[EVENT_PATCHINGENGINE_PLUGINS_INITIALISED + 1];
bool isPatchingEngineRunning = false;
HANDLE displayQuitMessageThreadHandle = NULL;

// patching threads
HANDLE executionThreads[MAXIMUM_SECTIONS];
unsigned long executionThreadsIds[MAXIMUM_SECTIONS];
int executionThreadsCount = 0;

// returns whether the patching engine has started
bool hasPatchingEngineStarted(void)
{
	return isPatchingEngineRunning;
}

// displays a quit message if waiting for a module, plugin, etc to complete execution
// (Argument - number of milliseconds to wait)
void executeQuitMessageThread(LPVOID args)
{
	int numMilliseconds = PtrToInt(args);

	// wait numMilliseconds before displaying the 'Waiting for...' message
	if (numMilliseconds > 0) Sleep(numMilliseconds);

	// wait for patching engine initialisation waiting event before displaying
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, false);

	// display waiting message
	if ((getGlobalPatchingDetails()->numberOfModules <= 0) && (getGlobalPatchingDetails()->numberOfPlugins <= 0)) displayConsoleMessage("\r\nWaiting for the Patching Engine to Complete Execution... Press CTRL+C to Quit");
	else if (getGlobalPatchingDetails()->numberOfPlugins <= 0) displayConsoleMessage("\r\nWaiting for a Module to be Initialised & Patched... Press CTRL+C to Quit");
	else if (getGlobalPatchingDetails()->numberOfModules <= 0) displayConsoleMessage("\r\nWaiting for a Plugin to Complete Execution... Press CTRL+C to Quit");
	else displayConsoleMessage("\r\nWaiting for a Module or Plugin to Complete Execution... Press CTRL+C to Quit");

	// sleep a further (numMilliseconds * 2) before minimising the console window
	if (numMilliseconds > 0) Sleep(numMilliseconds * 2);
	// check that the console window isn't already minimised
	if (!IsIconic(ULib.ConsoleLibrary.getConsoleWindowHandle())) ShowWindowAsync(ULib.ConsoleLibrary.getConsoleWindowHandle(), SW_SHOWMINNOACTIVE);

	ExitThread(ERROR_SUCCESS);
}

// terminates the quit message thread if it is still running
void terminateQuitMessageThread(void)
{
	if (displayQuitMessageThreadHandle != NULL)
	{
		if (!hasThreadTerminated(displayQuitMessageThreadHandle)) TerminateThread(displayQuitMessageThreadHandle, 1);
		CloseHandle(displayQuitMessageThreadHandle);
	}
}

// used to terminate the process patcher when a serious
// error occurs or the user interrups the patching process
BOOL WINAPI ProcessPatcherControlHandler(DWORD ctrlEvent)
{
	// quit process patcher without giving an error message
	if (ctrlEvent == CTRL_C_EVENT) shutdownProcessPatcher(true);
	else displayErrorMessage(ERROR_PATCHING_ENGINE_TERMINATED, true);
	return TRUE;
}

// returns the event name for the specified eventId
char* getWaitingEventName(unsigned long eventId)
{
	if (eventId == EVENT_PATCHINGENGINE_INITIALISED) return eventNames[EVENT_PATCHINGENGINE_INITIALISED];
	else if (eventId == EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME) return eventNames[EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME];
	else if (eventId == EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED) return eventNames[EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED];
	else if (eventId == EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED) return eventNames[EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED];
	else if (eventId == EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE3_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_MODULE3_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE4_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_MODULE4_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE5_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_MODULE5_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE1_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_MODULE1_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE2_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_MODULE2_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE3_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_MODULE3_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE4_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_MODULE4_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_MODULE5_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_MODULE5_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_READY) return eventNames[EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_READY];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_FINISHED) return eventNames[EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_FINISHED];
	else if (eventId == EVENT_PATCHINGENGINE_PLUGINS_INITIALISED) return eventNames[EVENT_PATCHINGENGINE_PLUGINS_INITIALISED];
	else return NULL;
}

// called by the patching engine threads to wait
// for a specific patching engine event to be signalled
void waitForPatchingEngineWaitingEvent(unsigned long eventId, bool checkProcessStatus)
{
	// open handle to the patching engine waiting event
	HANDLE eventPatchingEngineEventHandle = OpenEvent(EVENT_ALL_ACCESS, TRUE, getWaitingEventName(eventId));
	if (eventPatchingEngineEventHandle == NULL) displayErrorMessage(ERROR_PATCHING_ENGINE_WAITINGEVENTS, true);

	// wait forever for the event to be set
	if (!checkProcessStatus) WaitForSingleObject(eventPatchingEngineEventHandle, INFINITE);
	// otherwise wait, but check the target process status every second
	else
	{
		int waitEventResult;

		do
		{
			// check every second whether the main process has terminated
			if (hasProcessTerminated(getMainProcessHandle())) displayErrorMessage(ERROR_PATCHING_ENGINE_PROCESSTERMINATED, true);
			waitEventResult = WaitForSingleObject(eventPatchingEngineEventHandle, 1000);
		} while (waitEventResult == WAIT_TIMEOUT);
	}

	CloseHandle(eventPatchingEngineEventHandle);
}

// used to set a waiting event by signalling the event
void setPatchingEngineWaitingEvent(unsigned long eventId, bool handleEventErrors)
{
	// open handle to the patching engine waiting event
	HANDLE eventPatchingEngineEventHandle = OpenEvent(EVENT_ALL_ACCESS, TRUE, getWaitingEventName(eventId));
	if (eventPatchingEngineEventHandle != NULL)
	{
		// set event
		SetEvent(eventPatchingEngineEventHandle);
		CloseHandle(eventPatchingEngineEventHandle);
	}
	else if (handleEventErrors)
	{
		displayErrorMessage(ERROR_PATCHING_ENGINE_WAITINGEVENTS, true);
	}
}

// used to reset a waiting event so that threads can wait until it has been signalled again
void resetPatchingEngineWaitingEvent(unsigned long eventId, bool handleEventErrors)
{
	// open handle to the patching engine waiting event
	HANDLE eventPatchingEngineEventHandle = OpenEvent(EVENT_ALL_ACCESS, TRUE, getWaitingEventName(eventId));
	if (eventPatchingEngineEventHandle != NULL)
	{
		// reset event
		ResetEvent(eventPatchingEngineEventHandle);
		CloseHandle(eventPatchingEngineEventHandle);
	}
	else if (handleEventErrors)
	{
		displayErrorMessage(ERROR_PATCHING_ENGINE_WAITINGEVENTS, true);
	}
}

// used to create a waiting event based on the supplied information
HANDLE createWaitingEvent(unsigned long eventId, char* eventName, char* eventNameConstant, bool isSignalled)
{
	// allocate memory and check event name exists
	eventNames[eventId] = (char*)ULib.HeapLibrary.allocPrivateHeap(256);
	if (getWaitingEventName(eventId) == NULL) displayErrorMessage(ERROR_PATCHING_ENGINE_WAITINGEVENTS, true);
	// create name and event
	sprintf_s(eventNames[eventId], 256, "%s.%s", eventNameConstant, eventName);
	HANDLE eventHandle = CreateEvent(NULL, TRUE, isSignalled, getWaitingEventName(eventId));
	if (eventHandle == NULL) displayErrorMessage(ERROR_PATCHING_ENGINE_WAITINGEVENTS, true);
	return eventHandle;
}

// terminates the patching engine
void terminatePatchingEngine(void)
{
	if (!hasPatchingEngineStarted()) return;

	// set target process resume waiting event
	setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME, false);
	Sleep(100);

	// terminate the threads (except the current thread)
	DWORD currentThreadId = GetCurrentThreadId();
	for (int i = 0; i < executionThreadsCount; i++)
	{
		if ((executionThreadsIds[i] != currentThreadId) && (!hasThreadTerminated(executionThreads[i]))) TerminateThread(executionThreads[i], 1);
	}

	// synchronise the suspend and resume counts of the main process thread
	resumeThread(getMainProcessThreadHandle());
}

// creates and executes the plugin threads then waits until signalled before
// proceeding with the target process, module and registry patching threads
void executePatchingEngine(void)
{
	// is debug information enabled
	bool isDisplayDebugInformationEnabled = ((ALLOW_DEBUG_INFORMATION) && (isDebugInformationEnabled()));

	// are we trying to create and patch a process that is this process patcher
	if (isTargetProcessPatcher()) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_RECURSIVE, true);

	if (isDisplayDebugInformationEnabled)
	{
		adjustConsoleScreenBufferSize(300);
		if ((ULib.OperatingSystemLibrary.isWindows9x()) | (ULib.OperatingSystemLibrary.isCurrentUserNTAdmin())) displayConsoleMessage("Process Patching Engine Started (Administrator User Rights)", true, true);
		else displayConsoleMessage("Process Patching Engine Started (Standard User Rights)", true, true);
	}

	// create event name constant
	char* eventNameConstant = (char*)ULib.HeapLibrary.allocPrivateHeap(128);
	sprintf_s(eventNameConstant, 126, "ProcessPatcher.%08X", GetTickCount());

	// create the waiting events
	HANDLE eventPatchingEngineInitialised = createWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED, "PatchingEngine.Initialised", eventNameConstant, false);
	HANDLE eventPatchingEngineTargetProcessResume = createWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME, "PatchingEngine.TargetProcess.Resume", eventNameConstant, true);
	HANDLE eventPatchingEngineTargetProcessInitialised = createWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED, "PatchingEngine.TargetProcess.Initialised", eventNameConstant, false);
	HANDLE eventPatchingEngineTargetProcessChildProcessInitialised = createWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED, "PatchingEngine.TargetProcess.ChildProcess.Initialised", eventNameConstant, false);
	HANDLE eventPatchingEngineTargetProcessMemoryPatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY, "PatchingEngine.TargetProcess.MemoryPatching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineTargetProcessMemoryPatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED, "PatchingEngine.TargetProcess.MemoryPatching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineModule1PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY, "PatchingEngine.Module1.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineModule2PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY, "PatchingEngine.Module2.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineModule3PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE3_PATCHING_READY, "PatchingEngine.Module3.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineModule4PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE4_PATCHING_READY, "PatchingEngine.Module4.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineModule5PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE5_PATCHING_READY, "PatchingEngine.Module5.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineModule1PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE1_PATCHING_FINISHED, "PatchingEngine.Module1.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineModule2PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE2_PATCHING_FINISHED, "PatchingEngine.Module2.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineModule3PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE3_PATCHING_FINISHED, "PatchingEngine.Module3.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineModule4PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE4_PATCHING_FINISHED, "PatchingEngine.Module4.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineModule5PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_MODULE5_PATCHING_FINISHED, "PatchingEngine.Module5.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineRegistry1PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_READY, "PatchingEngine.Registry1.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineRegistry2PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_READY, "PatchingEngine.Registry2.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineRegistry3PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_READY, "PatchingEngine.Registry3.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineRegistry4PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_READY, "PatchingEngine.Registry4.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineRegistry5PatchingReady = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_READY, "PatchingEngine.Registry5.Patching.Ready", eventNameConstant, true);
	HANDLE eventPatchingEngineRegistry1PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_FINISHED, "PatchingEngine.Registry1.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineRegistry2PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_FINISHED, "PatchingEngine.Registry2.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineRegistry3PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_FINISHED, "PatchingEngine.Registry3.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineRegistry4PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_FINISHED, "PatchingEngine.Registry4.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEngineRegistry5PatchingFinished = createWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_FINISHED, "PatchingEngine.Registry5.Patching.Finished", eventNameConstant, false);
	HANDLE eventPatchingEnginePluginsInitialised = createWaitingEvent(EVENT_PATCHINGENGINE_PLUGINS_INITIALISED, "PatchingEngine.Plugins.Initialised", eventNameConstant, false);

	// request the SE_DEBUG_PRIVILEGE privilege
	getSEDebugPrivilege();

	// initialise the patching engine threads
	initialiseTargetProcessPatching();
	initialiseProcessPatching();
	initialiseModulePatching();
	initialisePluginExecution();
	initialiseRegistryEntryPatching();

	isPatchingEngineRunning = true;

	// activate and handle control events
	SetConsoleCtrlHandler(ProcessPatcherControlHandler, TRUE);

	// don't create 'Waiting for' message thread if displaying debug information
	if (!isDisplayDebugInformationEnabled)
	{
		// create 'Waiting for...' message thread and wait for 10 seconds before displaying the message
		int numMilliseconds = 10000;
		unsigned long displayQuitMessageThreadId = 0;
		displayQuitMessageThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)executeQuitMessageThread, IntToPtr(numMilliseconds), CREATE_SUSPENDED, &displayQuitMessageThreadId);
		if ((displayQuitMessageThreadId > 0) && (displayQuitMessageThreadHandle != NULL))
		{
			SetThreadPriority(displayQuitMessageThreadHandle, THREAD_PRIORITY_IDLE);
			ResumeThread(displayQuitMessageThreadHandle);
		}
	}

	// cycle through each section, determine the patching type and execute the corresponding thread
	char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
	// used to store the thread number of the target process so that
	// it can be resumed once all the other threads have been created
	int targetProcessThreadIndex = -1;

	PATCHINGDETAILS_GLOBAL* pGlobalDetails = getGlobalPatchingDetails();
	int numberOfTargetProcesses = pGlobalDetails->numberOfTargetProcesses;
	int numberOfModules = pGlobalDetails->numberOfModules;
	int numberOfPlugins = pGlobalDetails->numberOfPlugins;
	int numberOfRegistryEntries = pGlobalDetails->numberOfRegistryEntries;

	// display execution information
	char* executeInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* scriptFilenameStripped = ULib.FileLibrary.stripPathFromFilename(pGlobalDetails->scriptFilename, true);
	sprintf_s(executeInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Executing Script : %s", (((getTargetProcessPatchingDetails() != NULL) && (getTargetProcessPatchingDetails()->displayName != NULL)) ? getTargetProcessPatchingDetails()->displayName : scriptFilenameStripped));
	displayConsoleMessage(executeInformation);
	ULib.HeapLibrary.freePrivateHeap(executeInformation);
	ULib.HeapLibrary.freePrivateHeap(scriptFilenameStripped);

	// create threads for all plugin types
	for (int currentSectionIndex = 0; currentSectionIndex < numberOfPlugins; currentSectionIndex++)
	{
		PATCHINGDETAILS_PLUGIN* pPluginDetails = getPluginPatchingDetails(currentSectionIndex);
		if ((pPluginDetails != NULL) && (pPluginDetails->sectionType == SECTIONTYPE_PLUGIN))
		{
			// if section is disabled
			if (pPluginDetails->isDisabled)
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Plugin Section %d [Disabled]", pPluginDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// if the current section doesn't have a valid filename
			else if ((pPluginDetails->fileName == NULL) || (pPluginDetails->fileName[0] == NULL))
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Plugin Section %d [Invalid Filename]", pPluginDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// otherwise, create thread
			else
			{
				executionThreads[executionThreadsCount] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)executePluginThread, pPluginDetails, CREATE_SUSPENDED, &executionThreadsIds[executionThreadsCount]);
				if ((executionThreadsIds[executionThreadsCount] <= 0) || (executionThreads[executionThreadsCount] == NULL)) displayErrorMessage(ERROR_PATCHING_PLUGIN_GENERAL, true);
				SetThreadPriority(executionThreads[executionThreadsCount], THREAD_PRIORITY_NORMAL);
				ResumeThread(executionThreads[executionThreadsCount]);
				executionThreadsCount++;
			}
		}
	}

	// wait for all the plugins to initialise
	if (executionThreadsCount == 0) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_PLUGINS_INITIALISED, false);
	waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_PLUGINS_INITIALISED, false);

	// create thread for the target process (if applicable)
	if (numberOfTargetProcesses > 0)
	{
		PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails = getTargetProcessPatchingDetails();
		if ((pTargetProcessDetails != NULL) && (pTargetProcessDetails->sectionType == SECTIONTYPE_TARGETPROCESS))
		{
			// if section is disabled
			if (pTargetProcessDetails->isDisabled)
			{
				if (isDisplayDebugInformationEnabled)
				{
					ULib.StringLibrary.copyString(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Target Process Section [Disabled]");
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// if the current section doesn't have a valid filename
			else if ((pTargetProcessDetails->fileName == NULL) || (pTargetProcessDetails->fileName[0] == NULL))
			{
				if (isDisplayDebugInformationEnabled)
				{
					ULib.StringLibrary.copyString(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Target Process Section [Invalid Filename]");
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// otherwise, create thread
			else
			{
				executionThreads[executionThreadsCount] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)executeTargetProcessPatchingThread, pTargetProcessDetails, CREATE_SUSPENDED, &executionThreadsIds[executionThreadsCount]);
				if ((executionThreadsIds[executionThreadsCount] <= 0) || (executionThreads[executionThreadsCount] == NULL)) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);
				SetThreadPriority(executionThreads[executionThreadsCount], THREAD_PRIORITY_NORMAL);
				// store the target process thread count position so it
				// can be resumed once all other threads have been created
				targetProcessThreadIndex = executionThreadsCount;
				executionThreadsCount++;
			}
		}
	}

	// create threads for all module types
	for (int currentSectionIndex = 0; currentSectionIndex < numberOfModules; currentSectionIndex++)
	{
		PATCHINGDETAILS_MODULE* pModuleDetails = getModulePatchingDetails(currentSectionIndex);
		if ((pModuleDetails != NULL) && (pModuleDetails->sectionType == SECTIONTYPE_MODULE))
		{
			// if section is disabled
			if (pModuleDetails->isDisabled)
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Module Section %d [Disabled]", pModuleDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// if the current section doesn't have a valid filename
			else if ((pModuleDetails->fileName == NULL) || (pModuleDetails->fileName[0] == NULL))
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Module Section %d [Invalid Filename]", pModuleDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// otherwise, create thread
			else
			{
				executionThreads[executionThreadsCount] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)executeModulePatchingThread, pModuleDetails, CREATE_SUSPENDED, &executionThreadsIds[executionThreadsCount]);
				if ((executionThreadsIds[executionThreadsCount] <= 0) || (executionThreads[executionThreadsCount] == NULL)) displayErrorMessage(ERROR_PATCHING_MODULE_GENERAL, true);
				SetThreadPriority(executionThreads[executionThreadsCount], THREAD_PRIORITY_NORMAL);
				ResumeThread(executionThreads[executionThreadsCount]);
				executionThreadsCount++;
			}
		}
	}

	// create threads for all registry entry types
	for (int currentSectionIndex = 0; currentSectionIndex < numberOfRegistryEntries; currentSectionIndex++)
	{
		PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails = getRegistryEntryPatchingDetails(currentSectionIndex);
		if ((pRegistryEntryDetails != NULL) && (pRegistryEntryDetails->sectionType == SECTIONTYPE_REGISTRYENTRY))
		{
			// if section is disabled
			if (pRegistryEntryDetails->isDisabled)
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Registry Section %d [Disabled]", pRegistryEntryDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// if section has an undefined registry action
			else if (pRegistryEntryDetails->registryAction == REGISTRYENTRY_ACTION_UNDEFINED)
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Registry Section %d [Invalid Action]", pRegistryEntryDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// if section has an unrecognised registry path
			else if ((pRegistryEntryDetails->registryHive == REGISTRYENTRY_HIVE_UNDEFINED) ||
				(pRegistryEntryDetails->registrySubKey == NULL) ||
				(pRegistryEntryDetails->registrySubKey[0] == NULL))
			{
				if (isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Ignoring Registry Section %d [Invalid Registry Path]", pRegistryEntryDetails->sectionNumber);
					displayConsoleMessage(debugMessage, true, true);
				}
			}
			// otherwise, create thread
			else
			{
				executionThreads[executionThreadsCount] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)executeRegistryEntryPatchingThread, pRegistryEntryDetails, CREATE_SUSPENDED, &executionThreadsIds[executionThreadsCount]);
				if ((executionThreadsIds[executionThreadsCount] <= 0) || (executionThreads[executionThreadsCount] == NULL)) displayErrorMessage(ERROR_PATCHING_REGISTRY_GENERAL, true);
				SetThreadPriority(executionThreads[executionThreadsCount], THREAD_PRIORITY_ABOVE_NORMAL);
				ResumeThread(executionThreads[executionThreadsCount]);
				executionThreadsCount++;
			}
		}
	}

	// if we haven't got a target process patching thread then signal the appropriate waiting events now
	if (targetProcessThreadIndex == -1)
	{
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED);
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME);
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED);
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED);
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED);
	}
	else
	{
		// resume the target process thread
		ResumeThread(executionThreads[targetProcessThreadIndex]);
	}

	// wait for all the threads to complete before continuing
	if (executionThreadsCount > 0)
	{
		WaitForMultipleObjects(executionThreadsCount, executionThreads, TRUE, INFINITE);
		for (executionThreadsCount--; executionThreadsCount >= 0; executionThreadsCount--) CloseHandle(executionThreads[executionThreadsCount]);
	}

	// terminate 'Waiting for...' message thread if it was originally created and it still exists
	terminateQuitMessageThread();

	// disable handling of control events
	SetConsoleCtrlHandler(ProcessPatcherControlHandler, FALSE);

	uninitialiseTargetProcessPatching();
	uninitialiseProcessPatching();
	uninitialiseModulePatching();
	uninitialisePluginExecution();
	uninitialiseRegistryEntryPatching();

	// close handles to all the waiting events
	CloseHandle(eventPatchingEngineInitialised);
	CloseHandle(eventPatchingEngineTargetProcessResume);
	CloseHandle(eventPatchingEngineTargetProcessInitialised);
	CloseHandle(eventPatchingEngineTargetProcessChildProcessInitialised);
	CloseHandle(eventPatchingEngineTargetProcessMemoryPatchingReady);
	CloseHandle(eventPatchingEngineTargetProcessMemoryPatchingFinished);
	CloseHandle(eventPatchingEngineModule1PatchingReady);
	CloseHandle(eventPatchingEngineModule2PatchingReady);
	CloseHandle(eventPatchingEngineModule3PatchingReady);
	CloseHandle(eventPatchingEngineModule4PatchingReady);
	CloseHandle(eventPatchingEngineModule5PatchingReady);
	CloseHandle(eventPatchingEngineModule1PatchingFinished);
	CloseHandle(eventPatchingEngineModule2PatchingFinished);
	CloseHandle(eventPatchingEngineModule3PatchingFinished);
	CloseHandle(eventPatchingEngineModule4PatchingFinished);
	CloseHandle(eventPatchingEngineModule5PatchingFinished);
	CloseHandle(eventPatchingEngineRegistry1PatchingReady);
	CloseHandle(eventPatchingEngineRegistry2PatchingReady);
	CloseHandle(eventPatchingEngineRegistry3PatchingReady);
	CloseHandle(eventPatchingEngineRegistry4PatchingReady);
	CloseHandle(eventPatchingEngineRegistry5PatchingReady);
	CloseHandle(eventPatchingEngineRegistry1PatchingFinished);
	CloseHandle(eventPatchingEngineRegistry2PatchingFinished);
	CloseHandle(eventPatchingEngineRegistry3PatchingFinished);
	CloseHandle(eventPatchingEngineRegistry4PatchingFinished);
	CloseHandle(eventPatchingEngineRegistry5PatchingFinished);
	CloseHandle(eventPatchingEnginePluginsInitialised);

	// close thread and process handles
	if (getChildProcessThreadHandle() != NULL) CloseHandle(getChildProcessThreadHandle());
	if (getChildProcessHandle() != NULL) CloseHandle(getChildProcessHandle());
	if (getTargetProcessThreadHandle() != NULL) CloseHandle(getTargetProcessThreadHandle());
	if (getTargetProcessHandle() != NULL) CloseHandle(getTargetProcessHandle());

	// memory cleanup
	ULib.HeapLibrary.freePrivateHeap(debugMessage);

	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_INITIALISED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE3_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE4_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE5_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE1_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE2_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE3_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE4_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_MODULE5_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_READY));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_FINISHED));
	ULib.HeapLibrary.freePrivateHeap(getWaitingEventName(EVENT_PATCHINGENGINE_PLUGINS_INITIALISED));
	ULib.HeapLibrary.freePrivateHeap(eventNameConstant);
	cleanupParsedEntries();

	isPatchingEngineRunning = false;

	// display debug information
	if (isDisplayDebugInformationEnabled) displayConsoleMessage("Process Patching Engine Finished", true, true);
}