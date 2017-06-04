////////////////////////////////////////////////////////
// Searches for information about executing processes //
////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingProcess.h"
#include "enginePatchingSupport.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

class PATCHINGENGINE_PROCESS
{
	public:
		API_KERNEL32_MODULE::CREATETOOLHELP32SNAPSHOT CreateToolhelp32Snapshot;
		API_KERNEL32_MODULE::PROCESS32FIRST Process32First;
		API_KERNEL32_MODULE::PROCESS32NEXT Process32Next;
		API_KERNEL32_MODULE::MODULE32FIRST Module32First;
		API_KERNEL32_MODULE::MODULE32NEXT Module32Next;
		API_NTDLL_MODULE::NTQUERYSYSTEMINFORMATION NtQuerySystemInformation;

		bool isDisplayDebugInformationEnabled;
		DWORD currentProcessSessionId;
		bool ignoreSessionId;
		DWORD obfuscatorValue;
};

PATCHINGENGINE_PROCESS pProcess;

// initialises the process structures before executing the functions
bool initialiseProcessPatching(void)
{
	// is debug information enabled
	pProcess.isDisplayDebugInformationEnabled = ((ALLOW_DEBUG_INFORMATION) && (isDebugInformationEnabled()));

	// for Windows 9x
	if (ULib.OperatingSystemLibrary.isWindows9x())
	{
		// retrieve Toolhelp32 APIs from KERNEL32.DLL
		pProcess.CreateToolhelp32Snapshot = getKERNEL32ApiFunctions().getCreateToolhelp32Snapshot();
		pProcess.Process32First = getKERNEL32ApiFunctions().getProcess32First();
		pProcess.Process32Next = getKERNEL32ApiFunctions().getProcess32Next();
		pProcess.Module32First = getKERNEL32ApiFunctions().getModule32First();
		pProcess.Module32Next = getKERNEL32ApiFunctions().getModule32Next();

		if ((pProcess.CreateToolhelp32Snapshot == NULL) ||
			(pProcess.Process32First == NULL) ||
			(pProcess.Process32Next == NULL) ||
			(pProcess.Module32First == NULL) ||
			(pProcess.Module32Next == NULL))
		{
			displayErrorMessage(ERROR_WIN32API_MODULE_TOOLHELP32, true);
		}

		// get the obfuscator value (so kernel32.dll process is ignored)
		pProcess.obfuscatorValue = getObfuscatorValue_9x();
	}
	// for Windows NTx
	else if (ULib.OperatingSystemLibrary.isWindowsNTx())
	{
		// retrieve APIs from NTDLL.DLL
		pProcess.NtQuerySystemInformation = getNTDLLApiFunctions().getNtQuerySystemInformation();
		if (pProcess.NtQuerySystemInformation == NULL) displayErrorMessage(ERROR_WIN32API_MODULE_NTDLL, true);

		// get current process session id
		pProcess.currentProcessSessionId = getProcessSessionId(GetCurrentProcessId());
		pProcess.ignoreSessionId = getGlobalPatchingDetails()->ignoreSessionId;
	}

	return true;
}

// uninitialise the process patching structures
bool uninitialiseProcessPatching(void)
{
	return true;
}

// has the process we are looking for been found
bool isProcessFilenameMatch(char* shortProcessFilename, char* foundProcessFilename)
{
	char* foundShortProcessFilename = ULib.FileLibrary.stripPathFromFilename(foundProcessFilename);
	bool matchResult = ULib.StringLibrary.isStringPatternMatch(foundShortProcessFilename, shortProcessFilename);
	ULib.HeapLibrary.freePrivateHeap(foundShortProcessFilename);
	return matchResult;
}

// populates the process information structure from the supplied information
PROCESS_INFORMATION_EX* getProcessInformationEx(char* fileName, unsigned long processId, HANDLE processHandle, bool getThreadInformation, API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION* pSystemProcessInformation)
{
	// initialise process information
	PROCESS_INFORMATION_EX* pProcessInformationEx = (PROCESS_INFORMATION_EX*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PROCESS_INFORMATION_EX));
	pProcessInformationEx->pPebLdrAddress = 0;
	pProcessInformationEx->pImageBaseAddress = 0;

	// session id
	pProcessInformationEx->dwSessionId = ((pSystemProcessInformation != NULL) ? pSystemProcessInformation->SessionId : 0);

	// process id, handle and filename
	pProcessInformationEx->dwProcessId = processId;
	if (processHandle != NULL) DuplicateHandle(GetCurrentProcess(), processHandle, GetCurrentProcess(), &pProcessInformationEx->hProcess, NULL, TRUE, DUPLICATE_SAME_ACCESS);
	pProcessInformationEx->lpFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.copyString(pProcessInformationEx->lpFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fileName);

	// thread id and handle
	if (getThreadInformation)
	{
		if (ULib.OperatingSystemLibrary.isWindows9x())
		{
			pProcessInformationEx->dwThreadId = getPrimaryThreadId_9x(pProcessInformationEx->dwProcessId);
			pProcessInformationEx->hThread = getPrimaryThreadHandle_9x(pProcessInformationEx->hProcess, pProcessInformationEx->dwThreadId);
		}
		else
		{
			pProcessInformationEx->dwThreadId = getPrimaryThreadId_NTx(pSystemProcessInformation);
			pProcessInformationEx->hThread = getPrimaryThreadHandle_NTx(pProcessInformationEx->dwProcessId, pProcessInformationEx->dwThreadId);
		}
	}

	return pProcessInformationEx;
}

// scans all the running processes and store the filename and process id
bool getAllProcessInformation(PROCESS_INFORMATION_EX* pInformationEx[], int* numberOfProcesses, int maximumProcesses)
{
	*numberOfProcesses = 0;

	// for Windows 9x
	if (ULib.OperatingSystemLibrary.isWindows9x())
	{
		// create a snapshot of the processes running on the local system
		HANDLE processSnapshot = pProcess.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (processSnapshot == INVALID_HANDLE_VALUE) return false;

		// cycle through each process
		API_KERNEL32_MODULE::PROCESSENTRY32 pEntry;
		pEntry.dwSize = sizeof(API_KERNEL32_MODULE::PROCESSENTRY32);
		pProcess.Process32First(processSnapshot, &pEntry);

		do
		{
			// ignore current process & kernel32.dll process
			if ((pEntry.th32ProcessID != GetCurrentProcessId()) && (pEntry.th32ParentProcessID != pProcess.obfuscatorValue))
			{
				// store process information
				pInformationEx[*numberOfProcesses] = getProcessInformationEx(pEntry.szExeFile, pEntry.th32ProcessID);

				(*numberOfProcesses)++;
				if (*numberOfProcesses >= maximumProcesses) break;
			}

		} while (pProcess.Process32Next(processSnapshot, &pEntry));

		// free snapshot32 handle
		CloseHandle(processSnapshot);
		return true;
	}
	// for Windows NTx
	else if (ULib.OperatingSystemLibrary.isWindowsNTx())
	{
		// system processes
		NTSTATUS dwStatus;
		DWORD cbSystemProcessInformation = 0x100000;
		LPVOID pvSystemProcessInformation = NULL;
		API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION* pSystemSessionProcessInformation = (API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION));

		do
		{
			ULib.HeapLibrary.freePrivateHeap(pvSystemProcessInformation);
			pvSystemProcessInformation = ULib.HeapLibrary.allocPrivateHeap(cbSystemProcessInformation);

			// retrieve all the processes (in the current session)
			/*if ((!pProcess.ignoreSessionId) && (pProcess.currentProcessSessionId >= 0) && (ULib.OperatingSystemLibrary.isWindows2000(true)))
			{
				pSystemSessionProcessInformation->SessionId = pProcess.currentProcessSessionId;
				pSystemSessionProcessInformation->Buffer = pvSystemProcessInformation;
				pSystemSessionProcessInformation->SizeOfBuf = cbSystemProcessInformation;
				if (ULib.OperatingSystemLibrary.isWindows64Bit()) dwStatus = pProcess.NtQuerySystemInformation(API_NTDLL_MODULE::SystemSessionProcessInformation, pSystemSessionProcessInformation, cbSystemProcessInformation, NULL);
				else dwStatus = pProcess.NtQuerySystemInformation(API_NTDLL_MODULE::SystemSessionProcessInformation, pSystemSessionProcessInformation, sizeof(API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION), NULL);
			}
			// retrieve all the processes
			else
			{*/
				dwStatus = pProcess.NtQuerySystemInformation(API_NTDLL_MODULE::SystemProcessInformation, pvSystemProcessInformation, cbSystemProcessInformation, NULL);
			//}

			if (dwStatus == STATUS_INFO_LENGTH_MISMATCH) cbSystemProcessInformation += 0x100000;
		} while (dwStatus == STATUS_INFO_LENGTH_MISMATCH);

		// error getting system processes
		if (!NT_SUCCESS(dwStatus))
		{
			ULib.HeapLibrary.freePrivateHeap(pSystemSessionProcessInformation);
			ULib.HeapLibrary.freePrivateHeap(pvSystemProcessInformation);
			return false;
		}

		// cycle through each process
		DWORD dwSystemProcessInformationOffset = 0;
		API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION* pSystemProcessInformation = (API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)pvSystemProcessInformation;
		bool bSkipProcess = false;

		for (;;)
		{
			bSkipProcess = false;

			// ignore idle process, system process & current process
			if ((pSystemProcessInformation->UniqueProcessId > 8) && (pSystemProcessInformation->UniqueProcessId != GetCurrentProcessId()))
			{
				// check for session id (if applicable)
				if ((!pProcess.ignoreSessionId) && (pProcess.currentProcessSessionId >= 0) && (ULib.OperatingSystemLibrary.isWindows2000(true)))
				{
					if (pProcess.currentProcessSessionId != pSystemProcessInformation->SessionId)
					{
						bSkipProcess = true;
					}
				}

				// store process information
				if (!bSkipProcess)
				{
					char* processFilename = ULib.StringLibrary.convertUnicodeToAnsi(pSystemProcessInformation->ImageName.Buffer, pSystemProcessInformation->ImageName.Length);
					pInformationEx[*numberOfProcesses] = getProcessInformationEx(processFilename, pSystemProcessInformation->UniqueProcessId);
					pInformationEx[*numberOfProcesses]->dwSessionId = pSystemProcessInformation->SessionId;
					ULib.HeapLibrary.freePrivateHeap(processFilename);

					(*numberOfProcesses)++;
					if (*numberOfProcesses >= maximumProcesses) break;
				}
			}

			// next process
			if (pSystemProcessInformation->NextEntryOffset <= 0) break;
			dwSystemProcessInformationOffset += pSystemProcessInformation->NextEntryOffset;
			pSystemProcessInformation = (API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)((BYTE*)pvSystemProcessInformation + dwSystemProcessInformationOffset);
		}

		ULib.HeapLibrary.freePrivateHeap(pSystemSessionProcessInformation);
		ULib.HeapLibrary.freePrivateHeap(pvSystemProcessInformation);
		return true;
	}

	return false;
}

// scans the running processes looking for a process that matches
PROCESS_INFORMATION_EX* findProcessInformation(char* shortProcessFilename, PATCHINGDETAILS_FILEBASE* pFileBaseDetails, bool findChildProcess, unsigned long parentProcessId, unsigned long selectedProcessId)
{
	bool useParentProcessId = ((parentProcessId > 0) && (shortProcessFilename == NULL));
	bool useSelectedProcessId = ((selectedProcessId > 0) && (!useParentProcessId));
	if ((!useParentProcessId) && (!useSelectedProcessId) && (shortProcessFilename == NULL)) return NULL;

	// list of processes to ignore
	int ignoreProcessesCount = 0;
	DWORD ignoreProcesses[2048][2];

	// for Windows 9x
	if (ULib.OperatingSystemLibrary.isWindows9x())
	{
		HANDLE processSnapshot;
		API_KERNEL32_MODULE::PROCESSENTRY32 pEntry;
		bool foundProcess = false;

		// repeat until process is found running (for child processes only)
		do
		{
			// has the parent process terminated
			if ((findChildProcess) && (hasProcessTerminated(getMainProcessHandle()))) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);

			// create a snapshot of the processes running on the local system
			processSnapshot = pProcess.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (processSnapshot == INVALID_HANDLE_VALUE) handleProcessPatchingFatalError(ERROR_PATCHING_CHILDPROCESS_DETERMINEPROCESSES);

			// cycle through each process checking if it's the process we are looking for
			pEntry.dwSize = sizeof(API_KERNEL32_MODULE::PROCESSENTRY32);
			pProcess.Process32First(processSnapshot, &pEntry);

			do
			{
				// check for processes to ignore (including current process & kernel32.dll process)
				bool ignoreProcess = ((pEntry.th32ProcessID == GetCurrentProcessId()) || (pEntry.th32ParentProcessID == pProcess.obfuscatorValue));
				for (int i = 0; i < ignoreProcessesCount; i++)
				{
					if (ignoreProcess) break;
					if ((pEntry.th32ProcessID == ignoreProcesses[i][0]) &&
						(pEntry.th32ParentProcessID == ignoreProcesses[i][1]))
					{
						ignoreProcess = true;
					}
				}

				// check for a process match
				if (!ignoreProcess)
				{
					// using the selected process id
					if (useSelectedProcessId)
					{
						// check filesize if required
						if ((pEntry.th32ProcessID == selectedProcessId) && (checkFileSizeMatch(ULib.FileLibrary.getFileSize(pEntry.szExeFile), pFileBaseDetails)))
						{
							foundProcess = true;
							break;
						}
					}
					// using the parent process id
					else if (useParentProcessId)
					{
						if (pEntry.th32ParentProcessID == parentProcessId)
						{
							foundProcess = true;
							break;
						}
					}
					// using process filename
					else
					{
						// check filename and filesize
						if ((isProcessFilenameMatch(shortProcessFilename, pEntry.szExeFile)) && (checkFileSizeMatch(ULib.FileLibrary.getFileSize(pEntry.szExeFile), pFileBaseDetails)))
						{
							foundProcess = true;
							break;
						}
						// add process to ignore list
						else if (ignoreProcessesCount < 2048)
						{
							ignoreProcesses[ignoreProcessesCount][0] = pEntry.th32ProcessID;
							ignoreProcesses[ignoreProcessesCount][1] = pEntry.th32ParentProcessID;
							ignoreProcessesCount++;
						}
					}
				}

			} while (pProcess.Process32Next(processSnapshot, &pEntry));

			// free snapshot32 handle
			CloseHandle(processSnapshot);

			// process not found
			if (!foundProcess)
			{
				// check if shuting down
				if (isProcessPatcherShutingDown()) return NULL;
				// if not looking for a child process - abort
				if (!findChildProcess) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);
				// report error if parent process has terminated
				if ((useParentProcessId) && (hasProcessTerminated(parentProcessId))) handleProcessPatchingFatalError(ERROR_PATCHING_CHILDPROCESS_FINDPROCESS);
				// delay enumeration
				Sleep(PATCHINGENGINE_SLEEPDELAY);
			}

		} while (!foundProcess);

		// open handle to the found process
		HANDLE processHandle = OpenProcess(PATCHINGENGINE_DEFAULTPROCESSACCESS, TRUE, pEntry.th32ProcessID);
		if (processHandle == NULL)
		{
			if (findChildProcess) handleProcessPatchingFatalError(ERROR_PATCHING_CHILDPROCESS_MEMORYACCESS);
			else displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_MEMORYACCESS, true);
		}

		// get process information
		PROCESS_INFORMATION_EX* pProcessInformationEx = getProcessInformationEx(pEntry.szExeFile, pEntry.th32ProcessID, processHandle, true);
		CloseHandle(processHandle);
		return pProcessInformationEx;
	}
	// for Windows NTx
	else if (ULib.OperatingSystemLibrary.isWindowsNTx())
	{
		// found process information
		bool foundProcess = false;
		char* processFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		PWSTR processFilenameW = (PWSTR)ULib.HeapLibrary.allocPrivateHeap();
		HANDLE processHandle = NULL;
		DWORD processId = 0;

		// system processes
		NTSTATUS dwStatus;
		DWORD cbSystemProcessInformation = 0x100000;
		LPVOID pvSystemProcessInformation = NULL;
		API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION* pSystemProcessInformation = NULL;
		API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION* pSystemSessionProcessInformation = (API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION));

		// peb ldr structures
		PVOID pPebLdr = NULL;
		PVOID pImageBaseAddress = NULL;
		API_NTDLL_MODULE::PEB_LDR_DATA* pLdr = (API_NTDLL_MODULE::PEB_LDR_DATA*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::PEB_LDR_DATA));
		API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY* pLdrEntry = (API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY));

		// repeat until process is found running (for child processes only)
		do
		{
			// has the parent process terminated
			if ((findChildProcess) && (hasProcessTerminated(getMainProcessHandle()))) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);

			do
			{
				ULib.HeapLibrary.freePrivateHeap(pvSystemProcessInformation);
				pvSystemProcessInformation = ULib.HeapLibrary.allocPrivateHeap(cbSystemProcessInformation);

				// retrieve all the processes (in the current session)
				/*if ((!pProcess.ignoreSessionId) && (pProcess.currentProcessSessionId >= 0) && (ULib.OperatingSystemLibrary.isWindows2000(true)))
				{
					pSystemSessionProcessInformation->SessionId = pProcess.currentProcessSessionId;
					pSystemSessionProcessInformation->Buffer = pvSystemProcessInformation;
					pSystemSessionProcessInformation->SizeOfBuf = cbSystemProcessInformation;
					if (ULib.OperatingSystemLibrary.isWindows64Bit()) dwStatus = pProcess.NtQuerySystemInformation(API_NTDLL_MODULE::SystemSessionProcessInformation, pSystemSessionProcessInformation, cbSystemProcessInformation, NULL);
					else dwStatus = pProcess.NtQuerySystemInformation(API_NTDLL_MODULE::SystemSessionProcessInformation, pSystemSessionProcessInformation, sizeof(API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION), NULL);
				}
				// retrieve all the processes
				else
				{*/
					dwStatus = pProcess.NtQuerySystemInformation(API_NTDLL_MODULE::SystemProcessInformation, pvSystemProcessInformation, cbSystemProcessInformation, NULL);
				//}

				if (dwStatus == STATUS_INFO_LENGTH_MISMATCH) cbSystemProcessInformation += 0x100000;
			} while (dwStatus == STATUS_INFO_LENGTH_MISMATCH);

			// check for other errors
			if (!NT_SUCCESS(dwStatus)) handleProcessPatchingFatalError(ERROR_PATCHING_CHILDPROCESS_DETERMINEPROCESSES);

			// cycle through each process
			DWORD dwSystemProcessInformationOffset = 0;
			pSystemProcessInformation = (API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)pvSystemProcessInformation;

			for (;;)
			{
				// check for processes to ignore (including idle process, system process & current process)
				bool ignoreProcess = ((pSystemProcessInformation->UniqueProcessId <= 8) || (pSystemProcessInformation->UniqueProcessId == GetCurrentProcessId()));
				for (int i = 0; i < ignoreProcessesCount; i++)
				{
					if (ignoreProcess) break;
					if ((pSystemProcessInformation->UniqueProcessId == ignoreProcesses[i][0]) &&
						(pSystemProcessInformation->CreateTime.LowPart == ignoreProcesses[i][1]))
					{
						ignoreProcess = true;
					}
				}

				// check for session id (if applicable)
				if ((!pProcess.ignoreSessionId) && (pProcess.currentProcessSessionId >= 0) && (ULib.OperatingSystemLibrary.isWindows2000(true)))
				{
					if (pProcess.currentProcessSessionId != pSystemProcessInformation->SessionId)
					{
						ignoreProcess = true;
					}
				}

				// check for a process match
				if (!ignoreProcess)
				{
					// using the parent process id or selected process id
					if ((useParentProcessId) || (useSelectedProcessId))
					{
						// found child process or selected process
						if (((useParentProcessId) && (pSystemProcessInformation->InheritedFromUniqueProcessId == parentProcessId)) ||
							((useSelectedProcessId) && (pSystemProcessInformation->UniqueProcessId == selectedProcessId)))
						{
							// open handle to the process
							processHandle = OpenProcess(PATCHINGENGINE_DEFAULTPROCESSACCESS, TRUE, pSystemProcessInformation->UniqueProcessId);
							if (processHandle != NULL)
							{
								// read peb ldr
								getPebInformation(processHandle, &pPebLdr, &pImageBaseAddress);
								if (readMemoryAddress(processHandle, pPebLdr, sizeof(API_NTDLL_MODULE::PEB_LDR_DATA), pLdr, false))
								{
									API_NTDLL_MODULE::LIST_ENTRY* pNext = pLdr->InLoadOrderModuleList.Flink;

									// read first module entry (process filename)
									if (readMemoryAddress(processHandle, CONTAINING_RECORD(pNext, API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks), sizeof(API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY), pLdrEntry, false))
									{
										// get executable filename
										ZeroMemory(processFilenameW, pLdrEntry->FullDllName.Length + 2);
										if (readMemoryAddress(processHandle, (PVOID)pLdrEntry->FullDllName.Buffer, pLdrEntry->FullDllName.Length, processFilenameW, false))
										{
											processId = pSystemProcessInformation->UniqueProcessId;
											char* processFilenameA = ULib.StringLibrary.convertUnicodeToAnsi(processFilenameW, pLdrEntry->FullDllName.Length);
											ULib.StringLibrary.copyString(processFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, processFilenameA);
											ULib.HeapLibrary.freePrivateHeap(processFilenameA);
											foundProcess = true;

											// check filesize if required (selected process id only)
											if (useSelectedProcessId)
											{
												foundProcess = (checkFileSizeMatch(ULib.FileLibrary.getFileSize(processFilename), pFileBaseDetails));
												// add process to ignore list
												if ((!foundProcess) && (ignoreProcessesCount < 2048))
												{
													ignoreProcesses[ignoreProcessesCount][0] = pSystemProcessInformation->UniqueProcessId;
													ignoreProcesses[ignoreProcessesCount][1] = pSystemProcessInformation->CreateTime.LowPart;
													ignoreProcessesCount++;
												}
											}
										}
									}
								}

								if (foundProcess) break;
								CloseHandle(processHandle);
							}
						}
					}
					// using the process filename
					else
					{
						processHandle = OpenProcess(PATCHINGENGINE_DEFAULTPROCESSACCESS, TRUE, pSystemProcessInformation->UniqueProcessId);
						if (processHandle != NULL)
						{
							// read peb ldr
							getPebInformation(processHandle, &pPebLdr, &pImageBaseAddress);
							if (readMemoryAddress(processHandle, pPebLdr, sizeof(API_NTDLL_MODULE::PEB_LDR_DATA), pLdr, false))
							{
								API_NTDLL_MODULE::LIST_ENTRY* pNext = pLdr->InLoadOrderModuleList.Flink;

								// read first module entry (process filename)
								if (readMemoryAddress(processHandle, CONTAINING_RECORD(pNext, API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks), sizeof(API_NTDLL_MODULE::LDR_DATA_TABLE_ENTRY), pLdrEntry, false))
								{
									// get filename
									ZeroMemory(processFilenameW, pLdrEntry->FullDllName.Length + 2);
									if (readMemoryAddress(processHandle, (PVOID)pLdrEntry->FullDllName.Buffer, pLdrEntry->FullDllName.Length, processFilenameW, false))
									{
										char* processFilenameA = ULib.StringLibrary.convertUnicodeToAnsi(processFilenameW, pLdrEntry->FullDllName.Length);
										// check filename and filesize for match
										if ((isProcessFilenameMatch(shortProcessFilename, processFilenameA)) && (checkFileSizeMatch(ULib.FileLibrary.getFileSize(processFilenameA), pFileBaseDetails)))
										{
											processId = pSystemProcessInformation->UniqueProcessId;
											ULib.StringLibrary.copyString(processFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, processFilenameA);
											foundProcess = true;
										}
										// add process to ignore list
										else if (ignoreProcessesCount < 2048)
										{
											ignoreProcesses[ignoreProcessesCount][0] = pSystemProcessInformation->UniqueProcessId;
											ignoreProcesses[ignoreProcessesCount][1] = pSystemProcessInformation->CreateTime.LowPart;
											ignoreProcessesCount++;
										}
										ULib.HeapLibrary.freePrivateHeap(processFilenameA);
									}
								}
							}

							if (foundProcess) break;
							CloseHandle(processHandle);
						}
					}
				}

				// next process
				if (pSystemProcessInformation->NextEntryOffset <= 0) break;
				dwSystemProcessInformationOffset += pSystemProcessInformation->NextEntryOffset;
				pSystemProcessInformation = (API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)((BYTE*)pvSystemProcessInformation + dwSystemProcessInformationOffset);
			}

			// process not found
			if (!foundProcess)
			{
				// check if shuting down
				if (isProcessPatcherShutingDown()) return NULL;
				// if not looking for a child process - abort
				if (!findChildProcess) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);
				// report error if parent process has terminated
				if ((useParentProcessId) && (hasProcessTerminated(parentProcessId))) handleProcessPatchingFatalError(ERROR_PATCHING_CHILDPROCESS_FINDPROCESS);
				// delay enumeration
				Sleep(PATCHINGENGINE_SLEEPDELAY);
			}

		} while (!foundProcess);

		// check validity of the process handle
		if (processHandle == NULL)
		{
			if (findChildProcess) handleProcessPatchingFatalError(ERROR_PATCHING_CHILDPROCESS_MEMORYACCESS);
			else displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_MEMORYACCESS, true);
		}

		// get process and thread information
		PROCESS_INFORMATION_EX* pProcessInformationEx = getProcessInformationEx(processFilename, processId, processHandle, true, pSystemProcessInformation);
		pProcessInformationEx->pPebLdrAddress = pPebLdr;
		pProcessInformationEx->pImageBaseAddress = pImageBaseAddress;
		CloseHandle(processHandle);

		ULib.HeapLibrary.freePrivateHeap(pLdr);
		ULib.HeapLibrary.freePrivateHeap(pLdrEntry);
		ULib.HeapLibrary.freePrivateHeap(processFilenameW);
		ULib.HeapLibrary.freePrivateHeap(processFilename);
		ULib.HeapLibrary.freePrivateHeap(pSystemSessionProcessInformation);
		ULib.HeapLibrary.freePrivateHeap(pvSystemProcessInformation);
		return pProcessInformationEx;
	}

	return NULL;
}

// if the target process creates a child process which is actually
// the main process then we need to uncover the process information
PROCESS_INFORMATION_EX* findProcessInformation(PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails)
{
	if (pChildProcessDetails == NULL) return NULL;

	// remove the path from the required process file name
	char* shortProcessFilename = ULib.FileLibrary.stripPathFromFilename(pChildProcessDetails->fileName, true);

	// display debug information
	if (pProcess.isDisplayDebugInformationEnabled)
	{
		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Waiting for a Process to Initialise (%s)", shortProcessFilename);
		displayConsoleMessage(debugMessage, true, true);
		ULib.HeapLibrary.freePrivateHeap(debugMessage);
	}

	// get process information
	PROCESS_INFORMATION_EX* pProcessInformationEx = findProcessInformation(pChildProcessDetails->fileName, (PATCHINGDETAILS_FILEBASE*)pChildProcessDetails, true);
	if (pProcessInformationEx == NULL) return NULL;

	// if the process file name contains any wildcard symbols then report the new filename found
	if ((pProcess.isDisplayDebugInformationEnabled) && (pChildProcessDetails->isPatternMatchingFilename))
	{
		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(pProcessInformationEx->lpFilename, true);
		sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Found Process using Pattern Matching (%s)", fileNameStripped);
		displayConsoleMessage(debugMessage, true, true);
		ULib.HeapLibrary.freePrivateHeap(fileNameStripped);
		ULib.HeapLibrary.freePrivateHeap(debugMessage);
	}

	return pProcessInformationEx;
}

// if the target process creates a child process which is actually
// the main process then we need to uncover the process information
PROCESS_INFORMATION_EX* findProcessInformation(unsigned long parentProcessId)
{
	// display debug information
	if (pProcess.isDisplayDebugInformationEnabled) displayConsoleMessage("Waiting for any Child Process to Initialise", true, true);

	PROCESS_INFORMATION_EX* pProcessInformationEx = findProcessInformation(NULL, 0, true, parentProcessId);
	if (pProcessInformationEx == NULL) return NULL;

	// display debug information
	if (pProcess.isDisplayDebugInformationEnabled)
	{
		// display debug information
		if (pProcessInformationEx->lpFilename == NULL)
		{
			displayConsoleMessage("Found Child Process", true, true);
		}
		else
		{
			char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
			char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(pProcessInformationEx->lpFilename, true);
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Found Child Process (%s)", fileNameStripped);
			displayConsoleMessage(debugMessage, true, true);
			ULib.HeapLibrary.freePrivateHeap(fileNameStripped);
			ULib.HeapLibrary.freePrivateHeap(debugMessage);
		}
	}

	return pProcessInformationEx;
}