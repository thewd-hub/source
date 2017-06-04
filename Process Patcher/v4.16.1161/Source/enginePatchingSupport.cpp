//////////////////////////////////////////////////
// Support routines the process patching engine //
//////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingSupport.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

unsigned long handleObfuscatorValue = NULL;
unsigned long parentPDBValue = NULL;

// checks to see if the filesize retrieved matches which the settings in the script
bool checkFileSizeMatch(unsigned long fileSize, PATCHINGDETAILS_FILEBASE* pFileBaseDetails)
{
	if (pFileBaseDetails == NULL) return true;
	if (pFileBaseDetails->fileSize == 0) return true;
	if (pFileBaseDetails->fileSizeUpperLimit == 0) return (pFileBaseDetails->fileSize == fileSize);
	if (pFileBaseDetails->fileSizeUpperLimit > 0) return ((fileSize >= pFileBaseDetails->fileSize) && (fileSize <= pFileBaseDetails->fileSizeUpperLimit));
	return true;
}

// checks to see whether a process has terminated using the process handle
bool hasProcessTerminated(HANDLE processHandle)
{
	if (processHandle == NULL) return true;
	unsigned long processExitCode = 0;
	if (GetExitCodeProcess(processHandle, &processExitCode) == 0) return true;
	return (processExitCode != STILL_ACTIVE);
}

// checks to see whether a process has terminated by opening the process with processId
bool hasProcessTerminated(unsigned long processId)
{
	if (processId <= 8) return false;
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
	bool hasProcessTerminatedResult = hasProcessTerminated(processHandle);
	CloseHandle(processHandle);
	return hasProcessTerminatedResult;
}

// checks to see whether a thread has terminated
bool hasThreadTerminated(HANDLE threadHandle)
{
	if (threadHandle == NULL) return true;
	unsigned long threadExitCode = 0;
	if (GetExitCodeThread(threadHandle, &threadExitCode) == 0) return true;
	return (threadExitCode != STILL_ACTIVE);
}

// resume the specified thread
void resumeThread(HANDLE threadHandle)
{
	// synchronise the suspend and resume count
	if ((threadHandle != NULL) && (!hasThreadTerminated(threadHandle)))
	{
		int suspendThreadCount = 0;

		do
		{
			suspendThreadCount = ResumeThread(threadHandle);
			if (suspendThreadCount <= 1) suspendThreadCount = 0; // thread was not suspended or thread has resumed
		} while (suspendThreadCount > 0);
	}
}

// displays an error message if a fatal error occurred during the patching engine
void handleProcessPatchingFatalError(int errorNumber)
{
	displayErrorMessage(((hasProcessTerminated(getMainProcessHandle())) ? ERROR_PATCHING_ENGINE_PROCESSTERMINATED : errorNumber), true);
}

// determine the required process properties
bool determineWin9xProcessProperties(void)
{
	// already determined process properties
	if ((handleObfuscatorValue != NULL) && (parentPDBValue != NULL)) return true;

	unsigned long currentProcessId = GetCurrentProcessId();

#ifndef _M_X64
	__asm
	{
		push eax
		mov eax, currentProcessId
		xor eax, fs:[0x00000030]
		mov [handleObfuscatorValue], eax
		pop eax
	}
#endif

	parentPDBValue = (((API_KERNEL32_MODULE::PDB_9x*)UlongToPtr(handleObfuscatorValue ^ currentProcessId))->ParentPDB);
	return true;
}

// get the process database for the specified process id
API_KERNEL32_MODULE::PDB_9x* getPDB_9x(unsigned long processId)
{
	// try to get the required process properties (obfuscator & parent process)
	determineWin9xProcessProperties();
	if (handleObfuscatorValue == NULL) return NULL;

	// allocate memory for process database
	API_KERNEL32_MODULE::PDB_9x* pPDB = (API_KERNEL32_MODULE::PDB_9x*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_KERNEL32_MODULE::PDB_9x));
	ULONG pPDBReqLength = (ULONG)(PtrToUlong(&pPDB->DebuggerContextBlock) - PtrToUlong(&pPDB->U1.Win95A.DatabaseType));

	// determine the pdb address for the process
	unsigned long pPDBAddress = processId ^ handleObfuscatorValue;
	// read from memory - the process database
	if (!readMemoryAddress(GetCurrentProcess(), UlongToPtr(pPDBAddress), pPDBReqLength, (unsigned char*)pPDB, false)) return NULL;
	// check for a valid process database (Win95A - 5, Win95B/98/ME - 6)
	if ((pPDB->U1.Win95A.DatabaseType != 5) && (pPDB->U1.Win9x.DatabaseType != 6)) return NULL;

	return pPDB;
}

// returns the obfuscator value for Windows 9x
DWORD getObfuscatorValue_9x(void)
{
	// only valid on Windows 9x
	if (!ULib.OperatingSystemLibrary.isWindows9x()) return 0;

	// try to get the required process properties (obfuscator & parent process)
	determineWin9xProcessProperties();
	return handleObfuscatorValue;
}

// attempts to enter stealth mode, where the parent process of the
// target process is changed to the parent process of this app
bool attemptWin9xStealthMode(unsigned long processId)
{
	// only valid on Windows 9x
	if (!ULib.OperatingSystemLibrary.isWindows9x()) return false;

	// get process database
	API_KERNEL32_MODULE::PDB_9x* pPDB = getPDB_9x(processId);
	if (pPDB == NULL) return NULL;

	// determine the memory address of the parent pdb value
	unsigned long pPDBParentPDBValueAddress = (processId ^ handleObfuscatorValue) + PtrToLong(&pPDB->ParentPDB) - PtrToLong(pPDB);
	// write the parent pdb value back to the process's memory
	if (!writeMemoryAddress(GetCurrentProcess(), UlongToPtr(pPDBParentPDBValueAddress), 4, (unsigned char*)&parentPDBValue, false)) return false;
	ULib.HeapLibrary.freePrivateHeap(pPDB);
	return true;
}

// attempt to get the SE_DEBUG_PRIVILEGE privilege
bool getSEDebugPrivilege(void)
{
	// only valid on Windows NTx
	if (!ULib.OperatingSystemLibrary.isWindowsNTx()) return true;

	// request the SE_DEBUG_PRIVILEGE privilege
	BOOL bWasEnabled;
	API_NTDLL_MODULE::RTLADJUSTPRIVILEGE RtlAdjustPrivilege = getNTDLLApiFunctions().getRtlAdjustPrivilege();
	if ((RtlAdjustPrivilege != NULL) && (!(NTSTATUS(RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &bWasEnabled))))) return false;
	return true;
}

// returns the primary thread id for the specified process
unsigned long getPrimaryThreadId_9x(unsigned long processId)
{
	// only valid on Windows 9x
	if (!ULib.OperatingSystemLibrary.isWindows9x()) return NULL;

	// get process database
	API_KERNEL32_MODULE::PDB_9x* pPDB = getPDB_9x(processId);
	if (pPDB == NULL) return NULL;

	// read from memory - the thread list table
	unsigned char* threadBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(16);
	if (!readMemoryAddress(GetCurrentProcess(), UlongToPtr(pPDB->ThreadList), 4, threadBuffer, false)) return NULL;

	// read from memory - the first entry in the thread list table
	unsigned long tltEntryAddress = ULib.StringLibrary.getDWORDFromBuffer(threadBuffer);
	if (!readMemoryAddress(GetCurrentProcess(), UlongToPtr(tltEntryAddress), 12, threadBuffer, false)) return NULL;

	// read from memory - the thread database
	unsigned long tdbAddress = ULib.StringLibrary.getDWORDFromBuffer(threadBuffer, 0x08);
	if (!readMemoryAddress(GetCurrentProcess(), UlongToPtr(tdbAddress), 4, threadBuffer, false)) return NULL;

	// check for a valid thread database (Win95A - 6, Win95B/98/ME - 7)
	if ((pPDB->U1.Win95A.DatabaseType == 5) && (ULib.StringLibrary.getDWORDFromBuffer(threadBuffer) != 6)) return NULL;
	if ((pPDB->U1.Win9x.DatabaseType == 6) && (ULib.StringLibrary.getWORDFromBuffer(threadBuffer) != 7)) return NULL;

	ULib.HeapLibrary.freePrivateHeap(pPDB);
	ULib.HeapLibrary.freePrivateHeap(threadBuffer);
	return (tdbAddress ^ handleObfuscatorValue);
}

// returns the primary thread handle for the specified thread
HANDLE getPrimaryThreadHandle_9x(HANDLE processHandle, unsigned long threadId)
{
	// only valid on Windows 9x
	if ((threadId <= 0) || (!ULib.OperatingSystemLibrary.isWindows9x())) return NULL;

	// if Windows ME use the OpenThread kernel32 export, otherwise create handle manually
	API_KERNEL32_MODULE::OPENTHREAD OpenThread = getKERNEL32ApiFunctions().getOpenThread();
	if (OpenThread != NULL) return OpenThread(THREAD_ALL_ACCESS, TRUE, threadId);
	else
	{
		// get process database
		API_KERNEL32_MODULE::PDB_9x* pPDB = getPDB_9x(GetCurrentProcessId());
		if (pPDB == NULL) return NULL;

		HANDLE threadHandle = NULL;
		bool isWin95A = (pPDB->U1.Win95A.DatabaseType == 5);

		// duplicate the lowest handle within the process (Win95A - 1, Win95B/98/ME - 4)
		if (!DuplicateHandle(processHandle, UlongToHandle((isWin95A) ? 1 : 4), GetCurrentProcess(), &threadHandle, THREAD_ALL_ACCESS, TRUE, NULL)) return NULL;

		__try
		{
			// locate the entry within the handle table
			LPDWORD handleEntry = (LPDWORD)UlongToPtr(pPDB->HandleTable + (((HandleToUlong(threadHandle)) >> ((isWin95A) ? 0 : 2)) * 8) + 8);
			// decrement the reference count within the PDB (which was duplicated above)
			if (isWin95A) ((API_KERNEL32_MODULE::PDB_9x*)UlongToPtr(*handleEntry))->U1.Win95A.ReferenceCount--;
			else ((API_KERNEL32_MODULE::PDB_9x*)UlongToPtr(*handleEntry))->U1.Win9x.ReferenceCount--;
			// change the handle entry so that it points to the thread database
			*handleEntry = (DWORD)(threadId ^ handleObfuscatorValue);
			// increment the reference count within the TDB
			if (isWin95A) ((API_KERNEL32_MODULE::TDB_9x*)UlongToPtr(*handleEntry))->U1.Win95A.ReferenceCount++;
			else ((API_KERNEL32_MODULE::TDB_9x*)UlongToPtr(*handleEntry))->U1.Win9x.ReferenceCount++;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			threadHandle = NULL;
		}

		ULib.HeapLibrary.freePrivateHeap(pPDB);
		return threadHandle;
	}
}

// returns the primary thread id for the specified process
unsigned long getPrimaryThreadId_NTx(API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION* pSystemProcessInformation)
{
	// only valid on Windows NTx
	if (!ULib.OperatingSystemLibrary.isWindowsNTx()) return 0;
	if ((pSystemProcessInformation == NULL) || (pSystemProcessInformation->NumberOfThreads <= 0)) return 0;

	UCHAR* pSystemProcessInformationOffset = ((UCHAR*)&pSystemProcessInformation->PrivatePageCount) + 4;
	ULONG pSystemProcessInformationSearchLength = (ULONG)(PtrToUlong(pSystemProcessInformation->ImageName.Buffer) - PtrToUlong(pSystemProcessInformationOffset));
	ULONG pSystemProcessInformationSearchMaxLength = ((pSystemProcessInformation->NumberOfThreads * sizeof(API_NTDLL_MODULE::SYSTEM_THREAD_INFORMATION)) + sizeof(API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION));
	if (pSystemProcessInformationSearchLength > pSystemProcessInformationSearchMaxLength) pSystemProcessInformationSearchLength = pSystemProcessInformationSearchMaxLength;
	for (unsigned long i = 0; i < pSystemProcessInformationSearchLength; i++)
	{
		API_NTDLL_MODULE::SYSTEM_THREAD_INFORMATION* pSystemThreadInformation = (API_NTDLL_MODULE::SYSTEM_THREAD_INFORMATION*)&pSystemProcessInformationOffset[i];
		if (pSystemThreadInformation->ClientId.UniqueProcess == pSystemProcessInformation->UniqueProcessId) return pSystemThreadInformation->ClientId.UniqueThread;
	}

	return 0;
}

// returns the primary thread handle for the specified thread
HANDLE getPrimaryThreadHandle_NTx(unsigned long processId, unsigned long threadId)
{
	// only valid on Windows NTx
	if ((threadId <= 0) || (!ULib.OperatingSystemLibrary.isWindowsNTx())) return NULL;

	// retrieve NtOpenThread API from NTDLL.DLL
	API_NTDLL_MODULE::NTOPENTHREAD NtOpenThread = getNTDLLApiFunctions().getNtOpenThread();
	if (NtOpenThread == NULL) return NULL;

	// initialise the required structures
	HANDLE threadHandle = NULL;
	API_NTDLL_MODULE::OBJECT_ATTRIBUTES* objectAttributes = (API_NTDLL_MODULE::OBJECT_ATTRIBUTES*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::OBJECT_ATTRIBUTES));
	CLIENT_ID* clientId = (CLIENT_ID*)ULib.HeapLibrary.allocPrivateHeap(sizeof(CLIENT_ID));
	objectAttributes->Length = sizeof(API_NTDLL_MODULE::OBJECT_ATTRIBUTES);
	objectAttributes->Attributes = OBJ_INHERIT;
	clientId->UniqueProcess = processId;
	clientId->UniqueThread = threadId;

	// get the thread handle
	if (!NT_SUCCESS(NtOpenThread(&threadHandle, THREAD_ALL_ACCESS, objectAttributes, clientId))) return NULL;
	ULib.HeapLibrary.freePrivateHeap(objectAttributes);
	ULib.HeapLibrary.freePrivateHeap(clientId);
	return threadHandle;
}

// returns the session id for the specified process
DWORD getProcessSessionId(DWORD processId)
{
	API_NTDLL_MODULE::NTQUERYINFORMATIONPROCESS pNtQueryInformationProcess = getNTDLLApiFunctions().getNtQueryInformationProcess();
	if ((!ULib.OperatingSystemLibrary.isWindows2000(true)) || (pNtQueryInformationProcess == NULL)) return 0;

	DWORD sessionId = 0;
	API_NTDLL_MODULE::PROCESS_SESSION_INFORMATION* pProcessSessionInformation = (API_NTDLL_MODULE::PROCESS_SESSION_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::PROCESS_SESSION_INFORMATION));
	HANDLE processHandle = ((processId == GetCurrentProcessId()) ? GetCurrentProcess() : OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId));
	if (processHandle != NULL)
	{
		if (NT_SUCCESS(pNtQueryInformationProcess(GetCurrentProcess(), API_NTDLL_MODULE::ProcessSessionInformation, pProcessSessionInformation, sizeof(API_NTDLL_MODULE::PROCESS_SESSION_INFORMATION), NULL))) sessionId = pProcessSessionInformation->SessionId;
		if (processId != GetCurrentProcessId()) CloseHandle(processHandle);
	}

	ULib.HeapLibrary.freePrivateHeap(pProcessSessionInformation);
	return sessionId;
}

// return the peb ldr address and image base address for the specifed process
bool getPebInformation(HANDLE processHandle, PVOID* pPebLdr, PVOID* dwImageBaseAddress)
{
	if (!ULib.OperatingSystemLibrary.isWindowsNTx()) return false;
	API_NTDLL_MODULE::NTQUERYINFORMATIONPROCESS pNtQueryInformationProcess = getNTDLLApiFunctions().getNtQueryInformationProcess();
	if (pNtQueryInformationProcess == NULL) return false;

	// peb structures
	API_NTDLL_MODULE::PROCESS_BASIC_INFORMATION* pProcessBasicInformation = (API_NTDLL_MODULE::PROCESS_BASIC_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::PROCESS_BASIC_INFORMATION));
	API_NTDLL_MODULE::PEB* pPeb = (API_NTDLL_MODULE::PEB*)ULib.HeapLibrary.allocPrivateHeap(sizeof(API_NTDLL_MODULE::PEB));
	ULONG pPebReqLength = (ULONG)(PtrToUlong(&pPeb->ProcessParameters) - PtrToUlong(&pPeb->InheritedAddressSpace));

	// get peb address and read
	if ((NT_SUCCESS(pNtQueryInformationProcess(processHandle, API_NTDLL_MODULE::ProcessBasicInformation, pProcessBasicInformation, sizeof(API_NTDLL_MODULE::PROCESS_BASIC_INFORMATION), NULL))) && (readMemoryAddress(processHandle, pProcessBasicInformation->PebBaseAddress, pPebReqLength, pPeb, false)))
	{
		if (pPebLdr != NULL) *pPebLdr = pPeb->Ldr;
		if (dwImageBaseAddress != NULL) *dwImageBaseAddress = pPeb->ImageBaseAddress;
	}

	ULib.HeapLibrary.freePrivateHeap(pPeb);
	ULib.HeapLibrary.freePrivateHeap(pProcessBasicInformation);
	return true;
}