////////////////////////////////////////////////
// Dumps the memory of a process from a start //
// address for a certain length to a filename //
////////////////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0101

#include <stdio.h>
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
	pluginVersionInformation->pluginDescription = "Dump Process Memory";
	pluginVersionInformation->pluginVersion = "1.01";
	return TRUE;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	if (pluginExecutionInformation->GetPluginArgumentCount() < 3)
	{
		SetLastPluginError("Usage: <output filename> <memory address> <number of bytes>");
		return FALSE;
	}

	char* outputFilename = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	strcpy(outputFilename, pluginExecutionInformation->GetPluginArgument(0));
	unsigned int fileOffset;
	unsigned int fileLength;

	// get file offset;
	if ((pluginExecutionInformation->GetPluginArgument(1)[0] == '0') && ((pluginExecutionInformation->GetPluginArgument(1)[1] == 'x') || (pluginExecutionInformation->GetPluginArgument(1)[1] == 'X'))) sscanf(pluginExecutionInformation->GetPluginArgument(1), "%x", &fileOffset);
	else sscanf(pluginExecutionInformation->GetPluginArgument(1), "%d", &fileOffset);

	// get file length
	if ((pluginExecutionInformation->GetPluginArgument(2)[0] == '0') && ((pluginExecutionInformation->GetPluginArgument(2)[1] == 'x') || (pluginExecutionInformation->GetPluginArgument(2)[1] == 'X'))) sscanf(pluginExecutionInformation->GetPluginArgument(2), "%x", &fileLength);
	else sscanf(pluginExecutionInformation->GetPluginArgument(2), "%d", &fileLength);

	// check for valid parameters
	if ((outputFilename[0] == NULL) || (fileOffset == 0) || (fileLength == 0))
	{
		SetLastPluginError("Error occurred parsing the plugin arguments");
		return FALSE;
	}

	pluginExecutionInformation->PluginHasInitialised();

	MessageBox(NULL, "Press OK when ready to Dump Process Memory", "Dump Memory", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

	unsigned char* buffer = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileLength + 1);
	unsigned long bytesRead = 0;

	if (ReadProcessMemory(pluginExecutionInformation->functionsTargetProcess->GetProcessHandle(), UlongToPtr(fileOffset), buffer, fileLength, (SIZE_T*)&bytesRead) == 0)
	{
		SetLastPluginError("Error occurred reading memory of the process");
		return FALSE;
	}

	unsigned long actuallyWritten;
	HANDLE fileHandle = CreateFile(outputFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		SetLastPluginError("Error occurred writing memory bytes to output file");
		return FALSE;
	}
	WriteFile(fileHandle, buffer, fileLength, &actuallyWritten, NULL);
	CloseHandle(fileHandle);
	if (actuallyWritten != fileLength)
	{
		SetLastPluginError("Error occurred writing memory bytes to output file");
		return FALSE;
	}

	HeapFree(GetProcessHeap(), NULL, outputFilename);
	HeapFree(GetProcessHeap(), NULL, buffer);

	MessageBox(NULL, "Successfully written memory bytes to output file", "Dump Memory", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

	// fire resume event (if required)
	pluginExecutionInformation->functionsWaitingEvents->SetEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME);
	return TRUE;
}