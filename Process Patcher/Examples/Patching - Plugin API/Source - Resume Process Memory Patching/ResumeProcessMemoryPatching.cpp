/////////////////////////////////////////////////////////////////
// Resume Process Memory Patching                              //
//                                                             //
// Demonstrates the control of the patching engine by allowing //
// the memory patching to proceed for the target process       //
/////////////////////////////////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0101

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
	pluginVersionInformation->pluginDescription = "Resume Process Memory Patching";
	pluginVersionInformation->pluginVersion = "1.01";
	return TRUE;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	pluginExecutionInformation->functionsWaitingEvents->ResetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
	pluginExecutionInformation->PluginHasInitialised();

	MessageBox(NULL, "[Target Process Memory Patching Paused]\r\n\r\nWill continue 3 seconds after this Message Box has closed", "Plugin API Control", MB_OK);
	Sleep(3000);
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY);
	return TRUE;
}