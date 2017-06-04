//////////////////////////////////////
// Inject Module Plugin             //
// (C)thewd, thewd@hotmail.com      //
//////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0101

#include <windows.h>
#include "..\..\..\Process Patcher\apiPluginSupport.h"
#include "EliRT.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

char* lastErrorMessage = NULL;

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
void SetLastPluginError(char* errorMessage, int errorNumber)
{
	if (lastErrorMessage == NULL) lastErrorMessage = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	if (errorNumber == -1) strcpy_s(lastErrorMessage, 1024, errorMessage);
	else wsprintf(lastErrorMessage, "%s (Error #%d)", errorMessage, errorNumber);
}

/////////////////////////////////////////////////
// version information for the process patcher //
/////////////////////////////////////////////////
BOOL GetPluginVersionInformation(PLUGIN_VERSION_INFORMATION* pluginVersionInformation)
{
	pluginVersionInformation->apiPluginVersion = API_PLUGIN_VERSION;
	pluginVersionInformation->pluginDescription = "Inject Module";
	pluginVersionInformation->pluginVersion = "2.00";
	return TRUE;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	char* moduleName = NULL;

	// get the module name argument
	if (pluginExecutionInformation->GetPluginArgumentCount() >= 1)
	{
		moduleName = (char*)HeapAlloc(GetProcessHeap(), NULL, MAX_PATH + 16);
		strcpy_s(moduleName, MAX_PATH + 16, pluginExecutionInformation->GetPluginArgument(0));
	}

	// check module name has been parsed
	if (moduleName == NULL)
	{
		SetLastPluginError("Unable to retrieve the module name from the arguments", -1);
		return FALSE;
	}

	pluginExecutionInformation->PluginHasInitialised();

	// wait for process initialisation to be completed
	//pluginExecutionInformation->functionsWaitingEvents->WaitForEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED);

	// open handle to the process
	HANDLE processHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, TRUE, pluginExecutionInformation->functionsTargetProcess->GetProcessId());
	if (processHandle == NULL)
	{
		SetLastPluginError("Unable to access the target process", GetLastError());
		return FALSE;
	}

	// allocate remote memory space
	LPVOID remoteBuffer = xVirtualAllocEx(processHandle, NULL, MAX_PATH + 16, MEM_COMMIT, PAGE_READWRITE);
	if (remoteBuffer == NULL)
	{
		SetLastPluginError("Unable to allocate memory space within the process", GetLastError());
		return FALSE;
	}

	// write module name into the remote memory space
	if (!WriteProcessMemory(processHandle, remoteBuffer, moduleName, strlen(moduleName) + 1, NULL))
	{
		SetLastPluginError("Unable to write to the allocated memory space", GetLastError());
		return FALSE;
	}

	HeapFree(GetProcessHeap(), NULL, moduleName);

	// execute the remote thread
	unsigned long threadId;
	HANDLE threadHandle = xCreateRemoteThread(processHandle, NULL, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"),
		remoteBuffer, NULL, &threadId);
	if (threadHandle == NULL)
	{
		SetLastPluginError("Unable to create or execute the remote thread", GetLastError());
		return FALSE;
	}

	// wait for remote thread termination
	WaitForSingleObject(threadHandle, INFINITE);
	unsigned long threadExitCode = 0;
	GetExitCodeThread(threadHandle, &threadExitCode);
	CloseHandle(threadHandle);
	// check that the exit code contains the module address
	if (threadExitCode < 400000)
	{
		int lastErrorNumber = GetLastError();
		if (lastErrorNumber == 126) SetLastPluginError("Unable to load the specified module (Not Found)", -1);
		else SetLastPluginError("Fatal error occurred while executing the remote thread", lastErrorNumber);
		return FALSE;
	}

	// try to free the remote memory space
	xVirtualFreeEx(processHandle, remoteBuffer, 0, MEM_RELEASE);
	CloseHandle(processHandle);
	return TRUE;
}