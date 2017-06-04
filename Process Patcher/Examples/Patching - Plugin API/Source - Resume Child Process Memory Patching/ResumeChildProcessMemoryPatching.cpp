/////////////////////////////////////////////////////////////////
// Resume Child Process Memory Patching                        //
//                                                             //
// Demonstrates the control of the patching engine by allowing //
// the memory patching to proceed for the child process        //
/////////////////////////////////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0103

#include <windows.h>
#include "..\apiPluginSupport.h"

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
	pluginVersionInformation->pluginDescription = "Resume Child Process Memory Patching";
	pluginVersionInformation->pluginVersion = API_PLUGIN_VERSION_0103;
	return TRUE;
}

//////////////////////////////////////////////////
// removes the path and lowercases the filename //
//////////////////////////////////////////////////
char* stripPathFromFilenameAndLowercase(char* fileName)
{
	if (fileName == NULL) return NULL;
	char* fileNameNoPath = strrchr(fileName, '\\');
	return CharLower(((fileNameNoPath == NULL) ? fileName : (++fileNameNoPath)));
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	pluginExecutionInformation->functionsWaitingEvents->ResetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
	pluginExecutionInformation->PluginHasInitialised();

	// wait for child process to initialise
	pluginExecutionInformation->functionsWaitingEvents->WaitForEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED);
	char* outputMessage = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	bool isWindowsNT = (!(GetVersion() & 0x80000000));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("");
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("[Parent Process]");
	wsprintf(outputMessage, "Filename  : %s", stripPathFromFilenameAndLowercase(pluginExecutionInformation->functionsTargetProcess->GetProcessFilename()));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	if (isWindowsNT) wsprintf(outputMessage, "P.Id      : %d (0x%X)", pluginExecutionInformation->functionsTargetProcess->GetProcessId(), pluginExecutionInformation->functionsTargetProcess->GetProcessId());
	else wsprintf(outputMessage, "P.Id      : 0x%X", pluginExecutionInformation->functionsTargetProcess->GetProcessId());
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	wsprintf(outputMessage, "P.Handle  : %d (0x%X)", HandleToUlong(pluginExecutionInformation->functionsTargetProcess->GetProcessHandle()), HandleToUlong(pluginExecutionInformation->functionsTargetProcess->GetProcessHandle()));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	wsprintf(outputMessage, "P.Session : %d", pluginExecutionInformation->functionsTargetProcess->GetProcessSessionId());
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	if (isWindowsNT) wsprintf(outputMessage, "T.Id      : %d (0x%X)", pluginExecutionInformation->functionsTargetProcess->GetThreadId(), pluginExecutionInformation->functionsTargetProcess->GetThreadId());
	else wsprintf(outputMessage, "T.Id      : 0x%X", pluginExecutionInformation->functionsTargetProcess->GetThreadId());
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	wsprintf(outputMessage, "T.Handle  : %d (0x%X)", HandleToUlong(pluginExecutionInformation->functionsTargetProcess->GetThreadHandle()), HandleToUlong(pluginExecutionInformation->functionsTargetProcess->GetThreadHandle()));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("[Child Process]");
	wsprintf(outputMessage, "Filename  : %s", stripPathFromFilenameAndLowercase(pluginExecutionInformation->functionsChildProcess->GetProcessFilename()));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	if (isWindowsNT) wsprintf(outputMessage, "P.Id      : %d (0x%X)", pluginExecutionInformation->functionsChildProcess->GetProcessId(), pluginExecutionInformation->functionsChildProcess->GetProcessId());
	else wsprintf(outputMessage, "P.Id      : 0x%X", pluginExecutionInformation->functionsChildProcess->GetProcessId());
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	wsprintf(outputMessage, "P.Handle  : %d (0x%X)", HandleToUlong(pluginExecutionInformation->functionsChildProcess->GetProcessHandle()), HandleToUlong(pluginExecutionInformation->functionsChildProcess->GetProcessHandle()));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	wsprintf(outputMessage, "P.Session : %d", pluginExecutionInformation->functionsChildProcess->GetProcessSessionId());
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	if (isWindowsNT) wsprintf(outputMessage, "T.Id      : %d (0x%X)", pluginExecutionInformation->functionsChildProcess->GetThreadId(), pluginExecutionInformation->functionsChildProcess->GetThreadId());
	else wsprintf(outputMessage, "T.Id      : 0x%X", pluginExecutionInformation->functionsChildProcess->GetThreadId());
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	wsprintf(outputMessage, "T.Handle  : %d (0x%X)", HandleToUlong(pluginExecutionInformation->functionsChildProcess->GetThreadHandle()), HandleToUlong(pluginExecutionInformation->functionsChildProcess->GetThreadHandle()));
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage(outputMessage);
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("");
	HeapFree(GetProcessHeap(), NULL, outputMessage);

	MessageBox(NULL, "[Child Process Memory Patching Paused]\r\n\r\nWill continue 3 seconds after this Message Box has closed", "Plugin API Control", MB_OK);
	Sleep(3000);
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
	return TRUE;
}