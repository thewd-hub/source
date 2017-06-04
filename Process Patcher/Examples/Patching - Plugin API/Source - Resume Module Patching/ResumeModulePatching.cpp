//////////////////////////////////////////////////////////
// Resume Module Patching                               //
//                                                      //
// Demonstrates the control of the patching engine by   //
// resuming the patching engine for the module patching //
//////////////////////////////////////////////////////////
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
	pluginVersionInformation->pluginDescription = "Resume Module Patching";
	pluginVersionInformation->pluginVersion = "1.01";
	return TRUE;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	pluginExecutionInformation->functionsWaitingEvents->ResetEvent(EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY);
	pluginExecutionInformation->functionsWaitingEvents->ResetEvent(EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY);
	pluginExecutionInformation->PluginHasInitialised();

	MessageBox(NULL, "[All Module Patching Paused]\r\n\r\nModules 1 & 2 will be patched in intervals of 3 seconds after this Message Box has closed", "Plugin API Control", MB_OK);
	Sleep(3000);
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("Executing Module Section 1");
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY);
	Sleep(3000);
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("Executing Module Section 2");
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY);
	Sleep(1000);
	return TRUE;
}