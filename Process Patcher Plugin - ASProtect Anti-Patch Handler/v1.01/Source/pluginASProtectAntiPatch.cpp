///////////////////////////////////////////////////////////
// ASProtect Anti-Patch Handler                          //
//                                                       //
// Plugin used to determine when the asprotect loader    //
// has finished and the memory patching engine can begin //
///////////////////////////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0101

#include <stdio.h>
#include <windows.h>
#include "..\..\..\Process Patcher\apiPluginSupport.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

char* lastErrorMessage = NULL;
char* debugMessage = NULL;
bool isDebugMode = false;
bool loaderFinished = false;

/////////////////
// entry point //
/////////////////
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID dwReserved)
{
	return TRUE;
}

///////////////////////////////////////////////
// if an error occurred, this is used by the //
// process patcher to get the error message  //
///////////////////////////////////////////////
LPSTR GetLastPluginError(void)
{
	return lastErrorMessage;
}

/////////////////////////////////////////////////
// set the error message should an error occur //
/////////////////////////////////////////////////
void SetLastPluginError(char* errorMessage)
{
	if (lastErrorMessage == NULL) lastErrorMessage = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	strcpy_s(lastErrorMessage, 1024, errorMessage);
}

/////////////////////////////////////////////////
// version information for the process patcher //
/////////////////////////////////////////////////
BOOL GetPluginVersionInformation(PLUGIN_VERSION_INFORMATION* pluginVersionInformation)
{
	pluginVersionInformation->apiPluginVersion = API_PLUGIN_VERSION;
	pluginVersionInformation->pluginDescription = "ASProtect Anti-Patch Handler";
	pluginVersionInformation->pluginVersion = "1.01";
	return TRUE;
}

///////////////////////////////////////////////////////////////////////
// waits for idle input and then flags that the loader has completed //
///////////////////////////////////////////////////////////////////////
void threadWaitForIdle(LPVOID args)
{
	PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation = (PLUGIN_EXECUTION_INFORMATION*)args;
	WaitForInputIdle(pluginExecutionInformation->functionsTargetProcess->GetProcessHandle(), INFINITE);
	if ((isDebugMode) && (!loaderFinished)) pluginExecutionInformation->functionsOther->DisplayConsoleMessage("Target Process Is Idle. Aborting...");
	loaderFinished = true;
	ExitThread(ERROR_SUCCESS);
}

///////////////////////////////////////////////////////
// determines, by looking at the EIP of the thread   //
// context whether the asprotect loader has finished //
// its initialisation and error checking routines.   //
// Returns TRUE when these conditions are met        //
///////////////////////////////////////////////////////
bool determineASProtectLoaderStatus(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation,
									unsigned long startingAddress,
									unsigned long endingAddress)
{
	if (!pluginExecutionInformation->functionsTargetProcess->GetThreadHandle()) return false;

	CONTEXT threadContext;
	threadContext.ContextFlags = CONTEXT_CONTROL;
	loaderFinished = false;

	// create wait for idle thread
	unsigned long threadId_WaitForIdle = 0;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadWaitForIdle, (LPVOID)pluginExecutionInformation, NULL, &threadId_WaitForIdle);

	do
	{
		Sleep(10);

		// suspend thread
		if (SuspendThread(pluginExecutionInformation->functionsTargetProcess->GetThreadHandle()) == -1) return false;

		// get thread context information
		if (GetThreadContext(pluginExecutionInformation->functionsTargetProcess->GetThreadHandle(), &threadContext) == 0) return false;

		// check for a EIP in the expected range
		if ((threadContext.Eip >= startingAddress) && (threadContext.Eip <= endingAddress)) loaderFinished = true;

		// don't resume thread if loader has finished (let the memory patching routine do it)
		if ((!loaderFinished) && (ResumeThread(pluginExecutionInformation->functionsTargetProcess->GetThreadHandle()) == -1)) return false;

	} while (!loaderFinished);

	return true;
}

////////////////////////////////////////////////////////////
// used to determine the starting and ending addresses    //
// from the supplied filename (if not supplied by script) //
////////////////////////////////////////////////////////////
bool determineMemoryAddresses(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation,
							  unsigned long* startingAddress,
							  unsigned long* endingAddress)
{
	// try to open the file and create mapping
	HANDLE fileHandle = CreateFile(pluginExecutionInformation->functionsTargetProcess->GetProcessFilename(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) return false;
	HANDLE hFileMapping = CreateFileMapping(fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hFileMapping == NULL) return false;
	LPVOID lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
	if (lpFileBase == NULL) return false;

	// is dos based file?
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
	if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;
	// is pe-format?
	PIMAGE_NT_HEADERS peHeader = (PIMAGE_NT_HEADERS)(((DWORD_PTR)lpFileBase) + (dosHeader->e_lfanew));
	if (peHeader->Signature != IMAGE_NT_SIGNATURE) return false;

	// determine entry point
	unsigned long dwEntryPoint = (ULONG)(peHeader->OptionalHeader.ImageBase + peHeader->OptionalHeader.AddressOfEntryPoint);

	// v1.00+
	if (dwEntryPoint > 0x401000)
	{
		*startingAddress = 0x401000;
		*endingAddress = dwEntryPoint - 1;
	}
	// v1.20+
	else if (dwEntryPoint == 0x401000)
	{
		// determine the location of the loader
		unsigned char* memoryBuffer = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x20);
		if (pluginExecutionInformation->functionsOther->ReadMemoryAddress(pluginExecutionInformation->functionsTargetProcess->GetProcessHandle(), dwEntryPoint, 0x10, memoryBuffer, TRUE))
		{
			// push operation
			if (memoryBuffer[0] == 0x68)
			{
				unsigned long dwLoaderStartAddress = (memoryBuffer[1] + (memoryBuffer[2] * 0x100) + (memoryBuffer[3] * 0x10000) + (memoryBuffer[4] * 0x1000000));
				*startingAddress = dwEntryPoint + 0x0C;
				*endingAddress = dwLoaderStartAddress - 1;
			}
		}

		HeapFree(GetProcessHeap(), NULL, memoryBuffer);
	}

	// display address information
	if (isDebugMode)
	{
		wsprintf(debugMessage, "Found Starting Address (0x%X)", *startingAddress);
		pluginExecutionInformation->functionsOther->DisplayConsoleMessage(debugMessage);
		wsprintf(debugMessage, "Found Ending Address (0x%X)", *endingAddress);
		pluginExecutionInformation->functionsOther->DisplayConsoleMessage(debugMessage);
	}

	UnmapViewOfFile(lpFileBase);
	CloseHandle(hFileMapping);
	CloseHandle(fileHandle);
	return true;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	unsigned long startingAddress = 0;
	unsigned long endingAddress = 0;

	// is debug mode?
	if (pluginExecutionInformation->GetPluginSettings() & PLUGINSETTINGS_DEBUG)
	{
		isDebugMode = true;
		debugMessage = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	}

	// reset resume and memory patching events
	pluginExecutionInformation->functionsWaitingEvents->ResetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME);
	pluginExecutionInformation->functionsWaitingEvents->ResetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
	pluginExecutionInformation->PluginHasInitialised();

	// wait until process is idle
	if ((pluginExecutionInformation->GetPluginArgumentCount() == 1) && (_strcmpi(pluginExecutionInformation->GetPluginArgument(0), "IDLE") == 0))
	{
		startingAddress = 0x401001;
		endingAddress = 0x401000;
	}
	// parse the memory addresses from the arguments
	else if (pluginExecutionInformation->GetPluginArgumentCount() >= 2)
	{
		// get starting memory address
		if ((pluginExecutionInformation->GetPluginArgument(0)[0] == '0') && ((pluginExecutionInformation->GetPluginArgument(0)[1] == 'x') || (pluginExecutionInformation->GetPluginArgument(0)[1] == 'X'))) sscanf_s(pluginExecutionInformation->GetPluginArgument(0), "%x", &startingAddress);
		else sscanf_s(pluginExecutionInformation->GetPluginArgument(0), "%d", &startingAddress);
		// get ending memory address
		if ((pluginExecutionInformation->GetPluginArgument(1)[0] == '0') && ((pluginExecutionInformation->GetPluginArgument(1)[1] == 'x') || (pluginExecutionInformation->GetPluginArgument(1)[1] == 'X'))) sscanf_s(pluginExecutionInformation->GetPluginArgument(1), "%x", &endingAddress);
		else sscanf_s(pluginExecutionInformation->GetPluginArgument(1), "%d", &endingAddress);
	}
	// otherwise, try to determine the addresses automatically
	else
	{
		if (!determineMemoryAddresses(pluginExecutionInformation, &startingAddress, &endingAddress))
		{
			SetLastPluginError("Error occurred determining the address of the loader");
			return FALSE;
		}
	}

	// sanity check the memory addresses
	if ((startingAddress < 0x401000) || (startingAddress >= 0x1000000) || (endingAddress < 0x401000) || (endingAddress >= 0x1000000))
	{
		SetLastPluginError("Error occurred parsing the addresses from the supplied arguments");
		return FALSE;
	}

	// set resume event
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME);

	// wait for loader to finish before starting the memory patching engine
	if (!determineASProtectLoaderStatus(pluginExecutionInformation, startingAddress, endingAddress))
	{
		SetLastPluginError("Error occurred while accessing the protection loader");
		return FALSE;
	}

	// loader has initialised, start the memory patching routine
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
	if (debugMessage != NULL) HeapFree(GetProcessHeap(), NULL, debugMessage);
	return TRUE;
}