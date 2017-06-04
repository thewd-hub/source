//////////////////////////////////////
// Inno Setup Password Cracker      //
// (C)thewd, thewd@hotmail.com      //
//////////////////////////////////////
//////////////////////////////////////
// Disables the password dialog for //
// Inno Setup installation programs //
//////////////////////////////////////
#define API_PLUGIN_VERSION		API_PLUGIN_VERSION_0101

#include <windows.h>
#include "..\..\..\Process Patcher\apiPluginSupport.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

char* lastErrorMessage = NULL;
bool isDebugMode = false;

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
	pluginVersionInformation->pluginDescription = "Inno Setup Password Cracker";
	pluginVersionInformation->pluginVersion = "2.00";
	return TRUE;
}

////////////////////////
// execute the plugin //
////////////////////////
BOOL ExecutePlugin(PLUGIN_EXECUTION_INFORMATION* pluginExecutionInformation)
{
	// is debug mode?
	if (pluginExecutionInformation->GetPluginSettings() & PLUGINSETTINGS_DEBUG) isDebugMode = true;

	// wait for child process to start
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("");
	pluginExecutionInformation->functionsOther->DisplayConsoleMessage("Waiting for the Setup Initialisation to be Completed");
	pluginExecutionInformation->PluginHasInitialised();
	pluginExecutionInformation->functionsWaitingEvents->WaitForEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED);
	Sleep(500);

	// read memory bytes
	unsigned long startMemoryAddress = 0x430000;
	int numberOfMemoryAddress = 0x40000;
	unsigned char* memoryBuffer = (unsigned char*)HeapAlloc(GetProcessHeap(), NULL, numberOfMemoryAddress + 1);
	if (!pluginExecutionInformation->functionsOther->ReadMemoryAddress(pluginExecutionInformation->functionsChildProcess->GetProcessHandle(), startMemoryAddress, numberOfMemoryAddress, memoryBuffer, FALSE))
	{
		numberOfMemoryAddress = 0x30000;
		if (!pluginExecutionInformation->functionsOther->ReadMemoryAddress(pluginExecutionInformation->functionsChildProcess->GetProcessHandle(), startMemoryAddress, numberOfMemoryAddress, memoryBuffer, FALSE))
		{
			numberOfMemoryAddress = 0x20000;
			if (!pluginExecutionInformation->functionsOther->ReadMemoryAddress(pluginExecutionInformation->functionsChildProcess->GetProcessHandle(), startMemoryAddress, numberOfMemoryAddress, memoryBuffer, FALSE))
			{
				SetLastPluginError("Unable to access or read the child process memory");
				return FALSE;
			}
		}
	}

	char* debugMessage = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	bool foundAddressMatch = false;

	// cycle through addresses looking for a match
	for (int i = 0; i < numberOfMemoryAddress; i++)
	{
		// find sleep function associated with the password dialog (for invalid passwords)
		if ((memoryBuffer[i] == 0x68) &&
			(memoryBuffer[i + 1] == 0xEE) &&
			(memoryBuffer[i + 2] == 0x02) &&
			(memoryBuffer[i + 3] == 0x00) &&
			(memoryBuffer[i + 4] == 0x00) &&
			(memoryBuffer[i + 5] == 0xE8))
		{
			if (isDebugMode)
			{
				wsprintf(debugMessage, "Found Sleep Function (0x%X)", startMemoryAddress + i + 5);
				pluginExecutionInformation->functionsOther->DisplayConsoleMessage(debugMessage);
			}

			// find invalid password conditional jump & disable password dialog flag
			for (int j = i - 0x100; j < i; j++)
			{
				// conditional jump found
				if ((memoryBuffer[j] == 0x84) &&
					(memoryBuffer[j + 1] == 0xDB) &&
					(memoryBuffer[j + 2] == 0x74))
				{
					// found memory address of password dialog flag
					unsigned long memoryAddress = memoryBuffer[j + 6] + (memoryBuffer[j + 7] * 0x100) + (memoryBuffer[j + 8] * 0x10000);
					if (isDebugMode)
					{
						wsprintf(debugMessage, "Found Password Dialog Flag (0x%X)", memoryAddress);
						pluginExecutionInformation->functionsOther->DisplayConsoleMessage(debugMessage);
					}

					// disable password checking
					if ((memoryAddress > 0x440000) && (memoryAddress < 0x500000))
					{
						memoryBuffer[j + 3] = 0x00;
						// patch conditional jump
						pluginExecutionInformation->functionsOther->WriteMemoryAddress(pluginExecutionInformation->functionsChildProcess->GetProcessHandle(), (startMemoryAddress + j + 3), 1, &memoryBuffer[j + 3], TRUE);
						// toggle disable password dialog flag
						pluginExecutionInformation->functionsOther->WriteMemoryAddress(pluginExecutionInformation->functionsChildProcess->GetProcessHandle(), memoryAddress, 1, &memoryBuffer[j + 3], TRUE);
						foundAddressMatch = true;
						break;
					}
				}
			}
		}

		if (foundAddressMatch) break;
	}

	HeapFree(GetProcessHeap(), NULL, debugMessage);
	HeapFree(GetProcessHeap(), NULL, memoryBuffer);

	// unable to find match
	if (!foundAddressMatch)
	{
		SetLastPluginError("Unable to find an address match for this Inno Setup");
		return FALSE;
	}

	return TRUE;
}