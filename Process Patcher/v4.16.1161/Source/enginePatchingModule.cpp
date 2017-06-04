////////////////////////////////////////////////////////////////////////
// Handles the patching of modules which are used by the main process //
////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingModule.h"
#include "enginePatchingSupport.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

class PATCHINGENGINE_MODULE
{
	public:
		API_KERNEL32_MODULE::CREATETOOLHELP32SNAPSHOT CreateToolhelp32Snapshot;
		API_KERNEL32_MODULE::MODULE32FIRST Module32First;
		API_KERNEL32_MODULE::MODULE32NEXT Module32Next;
		bool isDisplayDebugInformationEnabled;
};

PATCHINGENGINE_MODULE pModule;

typedef struct _IGNORE_MODULE
{
	DWORD moduleBaseAddress;
	DWORD moduleBaseSize;
} IGNORE_MODULE;

// initialises the module patching structures before calling the execution thread
bool initialiseModulePatching(void)
{
	// is debug information enabled
	pModule.isDisplayDebugInformationEnabled = ((ALLOW_DEBUG_INFORMATION) && (isDebugInformationEnabled()));

	// for Windows 9x
	if (ULib.OperatingSystemLibrary.isWindows9x())
	{
		// retrieve Toolhelp32 APIs from KERNEL32.DLL
		pModule.CreateToolhelp32Snapshot = getKERNEL32ApiFunctions().getCreateToolhelp32Snapshot();
		pModule.Module32First = getKERNEL32ApiFunctions().getModule32First();
		pModule.Module32Next = getKERNEL32ApiFunctions().getModule32Next();

		if ((pModule.CreateToolhelp32Snapshot == NULL) ||
			(pModule.Module32First == NULL) ||
			(pModule.Module32Next == NULL))
		{
			displayErrorMessage(ERROR_WIN32API_MODULE_TOOLHELP32, true);
		}
	}

	return true;
}

// uninitialise the module patching structures
bool uninitialiseModulePatching(void)
{
	return true;
}

// retrieves the base address for a module that is within the process memory
// returns the error number if an error occurred
unsigned long getBaseAddressForProcessModule(unsigned long processId, PATCHINGDETAILS_FILEBASE* pFileBaseDetails, IGNORE_MODULE* ignoreModules[], int* ignoreModulesCount)
{
	if (pFileBaseDetails == NULL) return NULL;
	unsigned long moduleBaseAddress = 0;

	// for Windows 9x
	if (ULib.OperatingSystemLibrary.isWindows9x())
	{
		// create a snapshot of all the modules within the process context
		HANDLE moduleSnapshot = pModule.CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
		if (moduleSnapshot == INVALID_HANDLE_VALUE) return ERROR_PATCHING_MODULE_DETERMINEMODULES;

		char* shortModuleFilename = ULib.FileLibrary.stripPathFromFilename(pFileBaseDetails->fileName);
		API_KERNEL32_MODULE::MODULEENTRY32 mEntry;
		mEntry.dwSize = sizeof(API_KERNEL32_MODULE::MODULEENTRY32);
		pModule.Module32First(moduleSnapshot, &mEntry);

		// cycle through each module checking for a match
		do
		{
			// check for modules to ignore
			bool ignoreModule = false;
			if (ignoreModules != NULL)
			{
				for (int i = 0; i < *ignoreModulesCount; i++)
				{
					if (ignoreModule) break;
					if ((PtrToUlong(mEntry.modBaseAddr) == ignoreModules[i]->moduleBaseAddress) &&
						(mEntry.modBaseSize == ignoreModules[i]->moduleBaseSize))
					{
						ignoreModule = true;
					}
				}
			}

			// check for a module match
			if (!ignoreModule)
			{
				char* foundShortModuleFilename = ULib.FileLibrary.stripPathFromFilename(mEntry.szExePath);

				// have we found a module filename match (and check file size if required)
				if ((ULib.StringLibrary.isStringPatternMatch(foundShortModuleFilename, shortModuleFilename)) && (checkFileSizeMatch(ULib.FileLibrary.getFileSize(mEntry.szExePath), pFileBaseDetails)))
				{
					// update filename
					ULib.HeapLibrary.freePrivateHeap(pFileBaseDetails->fileName);
					pFileBaseDetails->fileName = ULib.FileLibrary.stripPathFromFilename(mEntry.szExePath, true);

					// display the found module filename if it contains any wildcard symbols
					if ((ignoreModules != NULL) && (pModule.isDisplayDebugInformationEnabled) && (pFileBaseDetails->isPatternMatchingFilename))
					{
						char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
						sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Found Module using Pattern Matching (%s)", pFileBaseDetails->fileName);
						displayConsoleMessage(debugMessage, true, true);
						ULib.HeapLibrary.freePrivateHeap(debugMessage);
					}

					moduleBaseAddress = PtrToUlong(mEntry.modBaseAddr);
					ULib.HeapLibrary.freePrivateHeap(foundShortModuleFilename);
					break;
				}
				// add module to ignore list
				else if ((ignoreModules != NULL) && (*ignoreModulesCount < 2048))
				{
					ignoreModules[*ignoreModulesCount] = (IGNORE_MODULE*)ULib.HeapLibrary.allocPrivateHeap(sizeof(IGNORE_MODULE));
					ignoreModules[*ignoreModulesCount]->moduleBaseAddress = PtrToUlong(mEntry.modBaseAddr);
					ignoreModules[*ignoreModulesCount]->moduleBaseSize = mEntry.modBaseSize;
					(*ignoreModulesCount)++;
				}

				ULib.HeapLibrary.freePrivateHeap(foundShortModuleFilename);
			}

		} while (pModule.Module32Next(moduleSnapshot, &mEntry));

		ULib.HeapLibrary.freePrivateHeap(shortModuleFilename);
		CloseHandle(moduleSnapshot);
	}
	// for Windows NTx
	else if (ULib.OperatingSystemLibrary.isWindowsNTx())
	{
		// get peb ldr address
		API_NTDLL_MODULE::PEB_LDR_DATA* pPebLdr = (API_NTDLL_MODULE::PEB_LDR_DATA*)getMainProcessPebLdrAddress();
		PVOID pImageBaseAddress = getMainProcessImageBaseAddress();
		HANDLE processHandle = getMainProcessHandle();
		if ((pPebLdr == NULL) || (processHandle == NULL)) return NULL;

		// peb ldr structures
		API_NTDLL_MODULE::PEB_LDR_DATA* pLdr = (API_NTDLL_MODULE::PEB_LDR_DATA*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::PEB_LDR_DATA));
		API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY* pLdrEntry = (API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY));
		char* shortModuleFilename = ULib.FileLibrary.stripPathFromFilename(pFileBaseDetails->fileName);
		PWSTR moduleFilenameW = (PWSTR)ULib.HeapLibrary.allocPrivateHeap();

		// read peb ldr
		if (readMemoryAddress(processHandle, pPebLdr, sizeof(API_NTDLL_MODULE::PEB_LDR_DATA), pLdr, false))
		{
			API_NTDLL_MODULE::LIST_ENTRY* pNext = pLdr->InLoadOrderModuleList.Flink;
			API_NTDLL_MODULE::LIST_ENTRY* pHead = &pPebLdr->InLoadOrderModuleList;

			// cycle through the module list
			while (pNext != pHead)
			{
				// read module entry
				if (readMemoryAddress(processHandle, CONTAINING_RECORD(pNext, API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks), sizeof(API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY), pLdrEntry, false))
				{
					// check for modules to ignore (including executable module)
					bool ignoreModule = (pLdrEntry->DllBase == pImageBaseAddress);
					if (ignoreModules != NULL)
					{
						for (int i = 0; i < *ignoreModulesCount; i++)
						{
							if (ignoreModule) break;
							if ((PtrToUlong(pLdrEntry->DllBase) == ignoreModules[i]->moduleBaseAddress) &&
								(pLdrEntry->SizeOfImage == ignoreModules[i]->moduleBaseSize))
							{
								ignoreModule = true;
							}
						}
					}

					// check for a module match
					if (!ignoreModule)
					{
						// get filename
						ZeroMemory(moduleFilenameW, pLdrEntry->BaseDllName.Length + 2);
						if (readMemoryAddress(processHandle, (PVOID)pLdrEntry->BaseDllName.Buffer, pLdrEntry->BaseDllName.Length, moduleFilenameW, false))
						{
							char* moduleFilenameA = ULib.StringLibrary.convertUnicodeToAnsi(moduleFilenameW, pLdrEntry->BaseDllName.Length);

							// have we found a module filename match (and check file size if required)
							if ((ULib.StringLibrary.isStringPatternMatch(moduleFilenameA, shortModuleFilename)) && (checkFileSizeMatch(ULib.FileLibrary.getFileSize(moduleFilenameA), pFileBaseDetails)))
							{
								// update filename
								ULib.HeapLibrary.freePrivateHeap(pFileBaseDetails->fileName);
								pFileBaseDetails->fileName = ULib.FileLibrary.stripPathFromFilename(moduleFilenameA, true);

								// display the found module filename if it contains any wildcard symbols
								if ((ignoreModules != NULL) && (pModule.isDisplayDebugInformationEnabled) && (pFileBaseDetails->isPatternMatchingFilename))
								{
									char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
									sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Found Module using Pattern Matching (%s)", pFileBaseDetails->fileName);
									displayConsoleMessage(debugMessage, true, true);
									ULib.HeapLibrary.freePrivateHeap(debugMessage);
								}

								moduleBaseAddress = PtrToUlong(pLdrEntry->DllBase);
								ULib.HeapLibrary.freePrivateHeap(moduleFilenameA);
								break;
							}
							// add module to ignore list
							else if ((ignoreModules != NULL) && (*ignoreModulesCount < 2048))
							{
								ignoreModules[*ignoreModulesCount] = (IGNORE_MODULE*)ULib.HeapLibrary.allocPrivateHeap(sizeof(IGNORE_MODULE));
								ignoreModules[*ignoreModulesCount]->moduleBaseAddress = PtrToUlong(pLdrEntry->DllBase);
								ignoreModules[*ignoreModulesCount]->moduleBaseSize = pLdrEntry->SizeOfImage;
								(*ignoreModulesCount)++;
							}

							ULib.HeapLibrary.freePrivateHeap(moduleFilenameA);
						}
					}
				}

				pNext = pLdrEntry->InLoadOrderLinks.Flink;
			}
		}

		ULib.HeapLibrary.freePrivateHeap(pLdr);
		ULib.HeapLibrary.freePrivateHeap(pLdrEntry);
		ULib.HeapLibrary.freePrivateHeap(moduleFilenameW);
		ULib.HeapLibrary.freePrivateHeap(shortModuleFilename);
	}

	return moduleBaseAddress;
}

// checks to see if a module is still present in the process memory space
bool doesModuleExistInProcessMemory(unsigned long processId, PATCHINGDETAILS_FILEBASE* pFileBaseDetails)
{
	// get the base address of the module. If the result is < MAXIMUM_ERROR_MESSAGES then the module wasn't found
	return (getBaseAddressForProcessModule(processId, pFileBaseDetails, NULL, 0) < MAXIMUM_ERROR_MESSAGES);
}

// performs a patch on a module used by the process
// (Argument - PATCHINGDETAILS_MODULE)
void executeModulePatchingThread(LPVOID args)
{
	__try
	{
		PATCHINGDETAILS_MODULE* pModuleDetails = (PATCHINGDETAILS_MODULE*)args;

		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* shortModuleFilename = ULib.FileLibrary.stripPathFromFilename(pModuleDetails->fileName, true);

		// wait for main process initialisation waiting event to be signalled
		waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED, false);

		// wait for a module patching waiting event.
		// these events are originally signalled, unless reset by a plugin
		if (pModuleDetails->sectionNumber == 1) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE1_PATCHING_READY, false);
		else if (pModuleDetails->sectionNumber == 2) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE2_PATCHING_READY, false);
		else if (pModuleDetails->sectionNumber == 3) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE3_PATCHING_READY, false);
		else if (pModuleDetails->sectionNumber == 4) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE4_PATCHING_READY, false);
		else if (pModuleDetails->sectionNumber == 5) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE5_PATCHING_READY, false);

		// display debug information
		if (pModule.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Opening Module Handle (%s)", shortModuleFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		int memoryPatchRetryCount = 0;
		bool memoryPatchSucceeded = false;
		unsigned long moduleBaseAddress;

		// list of modules to ignore
		int ignoreModulesCount = 0;
		IGNORE_MODULE* ignoreModules[2048];

		// repeat this memory patching cycle ->retryCount times
		do
		{
			memoryPatchSucceeded = false;
			moduleBaseAddress = 0;

			__try
			{
				// retrieve the base address for the module (if found)
				if (getModuleBaseAddressLock() != NULL) EnterCriticalSection(getModuleBaseAddressLock());
				moduleBaseAddress = getBaseAddressForProcessModule(getMainProcessId(), (PATCHINGDETAILS_FILEBASE*)pModuleDetails, ignoreModules, &ignoreModulesCount);
			}
			__finally
			{
				if (getModuleBaseAddressLock() != NULL) LeaveCriticalSection(getModuleBaseAddressLock());
			}

			// if the module base address hasn't been found
			if (moduleBaseAddress < MAXIMUM_ERROR_MESSAGES)
			{
				// don't report errors on patching retries (moduleBaseAddress contains error number)
				if ((memoryPatchRetryCount <= 0) && (moduleBaseAddress > 0)) handleProcessPatchingFatalError(moduleBaseAddress);
				// check if process has terminated
				if (hasProcessTerminated(getMainProcessHandle())) break;
				// check if shuting down
				if (isProcessPatcherShutingDown()) break;
				// delay enumeration
				Sleep(PATCHINGENGINE_SLEEPDELAY);
				continue;
			}

			// display debug information
			if (pModule.isDisplayDebugInformationEnabled)
			{
				// update short filename
				ULib.HeapLibrary.freePrivateHeap(shortModuleFilename);
				shortModuleFilename = ULib.FileLibrary.stripPathFromFilename(pModuleDetails->fileName, true);

				sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Attempting to Patch Module Memory (%s)", shortModuleFilename);
				displayConsoleMessage(debugMessage, true, true);
			}

			// no memory addresses to patch
			if (pModuleDetails->numberOfAddresses <= 0)
			{
				if (pModule.isDisplayDebugInformationEnabled)
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Patching Module Memory [No Patch Required] (%s)", shortModuleFilename);
					displayConsoleMessage(debugMessage, true, true);
				}

				break;
			}
			// do memory patch
			else
			{
				pModuleDetails->baseAddress = moduleBaseAddress;
				memoryPatchSucceeded = performMemoryAddressPatch(getMainProcessHandle(), (PATCHINGDETAILS_MEMORY*)pModuleDetails, (PATCHINGDETAILS_FILEBASE*)pModuleDetails);

				// display debug information
				if (pModule.isDisplayDebugInformationEnabled)
				{
					// did the memory patch succeeded
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Patching Module Memory [%s] (%s)", ((memoryPatchSucceeded) ? "Succeeded" : "Failed"), shortModuleFilename);
					displayConsoleMessage(debugMessage, true, true);
				}

				// reached the retryCount limit
				if (memoryPatchRetryCount > pModuleDetails->retryCount) break;

				// has the main process terminated
				if (hasProcessTerminated(getMainProcessHandle())) break;

				// check if shuting down
				if (isProcessPatcherShutingDown()) break;

				// only increment memoryPatchRetryCount if the memory patch was successful
				if (memoryPatchSucceeded) memoryPatchRetryCount++;
			}

		} while (memoryPatchRetryCount <= pModuleDetails->retryCount);

		// display debug information
		if (pModule.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Closing Module Handle (%s)", shortModuleFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		for (int i = 0; i < ignoreModulesCount; i++) ULib.HeapLibrary.freePrivateHeap(ignoreModules[i]);
		ULib.HeapLibrary.freePrivateHeap(shortModuleFilename);
		ULib.HeapLibrary.freePrivateHeap(debugMessage);

		// signal waiting events
		if (pModuleDetails->sectionNumber == 1) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE1_PATCHING_FINISHED, true);
		else if (pModuleDetails->sectionNumber == 2) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE2_PATCHING_FINISHED, true);
		else if (pModuleDetails->sectionNumber == 3) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE3_PATCHING_FINISHED, true);
		else if (pModuleDetails->sectionNumber == 4) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE4_PATCHING_FINISHED, true);
		else if (pModuleDetails->sectionNumber == 5) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_MODULE5_PATCHING_FINISHED, true);
	}
	__except(handleCriticalException(GetExceptionInformation(), SECTIONTYPE_MODULE))
	{
	}

	ExitThread(ERROR_SUCCESS);
}