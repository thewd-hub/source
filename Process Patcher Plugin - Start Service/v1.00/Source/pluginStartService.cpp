//////////////////////////////////////////////////////////////////
// Start Service                                                //
//                                                              //
// Plugin used to start the specified service (Windows NT only) //
//////////////////////////////////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0101

#include <stdio.h>
#include <windows.h>
#include "..\..\..\Process Patcher\apiPluginSupport.h"

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
	pluginVersionInformation->pluginDescription = "Start Service";
	pluginVersionInformation->pluginVersion = "1.00";
	return TRUE;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	// get service name
	if (pluginExecutionInformation->GetPluginArgumentCount() < 1)
	{
		SetLastPluginError("Unable to retrieve the service name from the plugin arguments", -1);
		return FALSE;
	}

	// open handle to service control manager
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
	{
		SetLastPluginError("Unable to establish a connection to the service control manager", GetLastError());
		return FALSE;
	}
	// open handle to service
	SC_HANDLE schService = OpenService(schSCManager, pluginExecutionInformation->GetPluginArgument(0), SERVICE_START);
	if (schService == NULL)
	{
		CloseServiceHandle(schSCManager);
		SetLastPluginError("Unable to open the specified service", GetLastError());
		return FALSE;
	}
	// start the service
	if ((StartService(schService, 0, NULL) == 0) && (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		SetLastPluginError("Unable to start the specified service", GetLastError());
		return FALSE;
	}

	// pause execution
	if (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING) Sleep(1000);
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	pluginExecutionInformation->PluginHasInitialised();
	return TRUE;
}