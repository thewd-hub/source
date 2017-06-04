///////////////////////////////////////////////////
// Displays information about the parent process //
///////////////////////////////////////////////////
#include <windows.h>
#include "DisplayParentProcess.h"

HANDLE privateHeap;

/////////////////////////////////////////////////////////
// is the current operating system platform Windows NT //
/////////////////////////////////////////////////////////
bool isWinNT(void)
{
	OSVERSIONINFO* osVersion = (OSVERSIONINFO*)HeapAlloc(privateHeap, HEAP_ZERO_MEMORY, sizeof(OSVERSIONINFO));
	osVersion->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(osVersion) != 0)
	{
		if ((osVersion->dwPlatformId == VER_PLATFORM_WIN32_NT) && (osVersion->dwMajorVersion >= 4))
		{
			HeapFree(privateHeap, NULL, osVersion);
			return true;
		}
	}

	HeapFree(privateHeap, NULL, osVersion);
	return false;
}

////////////////////////////////////////////////
// get details about the parent process for a //
// particular process with a known processId  //
////////////////////////////////////////////////
bool determineParentProcessEntry32(PROCESSENTRY32* parentProcessEntry, unsigned long childProcessId)
{
	if (parentProcessEntry == NULL) return false;

	// for Windows 9x
	if (!isWinNT())
	{
		HMODULE krn32Module = GetModuleHandle("kernel32.dll");
		if (krn32Module == NULL) return false;

		// retrieve Toolhelp32 APIs from KERNEL32.DLL
		CREATETOOLHELP32SNAPSHOT CreateToolhelp32Snapshot = (CREATETOOLHELP32SNAPSHOT)GetProcAddress(krn32Module, "CreateToolhelp32Snapshot");
		PROCESS32FIRST Process32First = (PROCESS32FIRST)GetProcAddress(krn32Module, "Process32First");
		PROCESS32NEXT Process32Next = (PROCESS32NEXT)GetProcAddress(krn32Module, "Process32Next");
		if ((CreateToolhelp32Snapshot == NULL) || (Process32First == NULL) || (Process32Next == NULL)) return false;

		// create a snapshot of the main processes running on the local system
		HANDLE processSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (processSnap == INVALID_HANDLE_VALUE) return false;

		unsigned long parentProcessId = 0;
		bool foundChildProcess = false;
		bool foundParentProcess = false;

		// cycle through each process checking if it's the child process we are looking for
		parentProcessEntry->dwSize = sizeof(PROCESSENTRY32);
		Process32First(processSnap, parentProcessEntry);
		do
		{
			// have we found the process we are looking for?
			if (parentProcessEntry->th32ProcessID == childProcessId)
			{
				parentProcessId = parentProcessEntry->th32ParentProcessID;
				foundChildProcess = true;
				break;
			}

		} while (Process32Next(processSnap, parentProcessEntry));

		// if the child process has not been found
		if (!foundChildProcess)
		{
			CloseHandle(processSnap);
			return false;
		}

		// now cycle through the processes again looking for the parent process
		Process32First(processSnap, parentProcessEntry);
		do
		{
			// have we found the process we are looking for?
			if (parentProcessEntry->th32ProcessID == parentProcessId)
			{
				foundParentProcess = true;
				break;
			}

		} while (Process32Next(processSnap, parentProcessEntry));

		// free snapshot32 handle
		CloseHandle(processSnap);

		// if the parent process has not been found
		if (!foundParentProcess) return false;
		return true;
	}
	// for Windows NTx
	else
	{
		// Load PSAPI and NTDLL modules
		HMODULE ntdllModule = GetModuleHandle("ntdll.dll");
		HMODULE psapiModule = LoadLibrary("psapi.dll");
		if ((ntdllModule == NULL) || (psapiModule == NULL)) return false;

		// retrieve APIs
		NTQUERYINFORMATIONPROCESS NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress(ntdllModule, "NtQueryInformationProcess");
		GETMODULEBASENAME GetModuleBaseName = (GETMODULEBASENAME)GetProcAddress(psapiModule, "GetModuleBaseNameA");
		if ((NtQueryInformationProcess == NULL) || (GetModuleBaseName == NULL)) return false;

		// retrieve information about parent process
		PROCESS_INFORMATION_CLASS processInformationClass;
		PROCESS_BASIC_INFORMATION processBasicInformation;
		HANDLE processHandle;

		char* baseName = (char*)HeapAlloc(privateHeap, HEAP_ZERO_MEMORY, 1024);
		unsigned char* infoBuffer = (unsigned char*)HeapAlloc(privateHeap, HEAP_ZERO_MEMORY, 1024);

		ZeroMemory(&processInformationClass, sizeof(PROCESS_INFORMATION_CLASS));
		ZeroMemory(&processBasicInformation, sizeof(PROCESS_BASIC_INFORMATION));

		// open handle to child process
		processHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, childProcessId);
		if (processHandle == NULL) return false;

		// retrieve basic information about the child process
		if (NtQueryInformationProcess(processHandle, ProcessBasicInformation, &processBasicInformation, sizeof(processBasicInformation), NULL) < 0)
		{
			CloseHandle(processHandle);
			return false;
		}

		CloseHandle(processHandle);

		// open the parent process and retrieve the base name
		processHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, (DWORD)processBasicInformation.InheritedFromUniqueProcessId);
		if (processHandle == NULL) return false;

		// get base name
		if (GetModuleBaseName(processHandle, NULL, baseName, 1024) == 0)
		{
			CloseHandle(processHandle);
			return false;
		}

		CloseHandle(processHandle);

		// copy basic information to the PROCESSENTRY32 structure
		parentProcessEntry->dwSize = sizeof(PROCESSENTRY32);
		parentProcessEntry->th32ProcessID = (DWORD)processBasicInformation.InheritedFromUniqueProcessId;
		strcpy(parentProcessEntry->szExeFile, baseName);

		HeapFree(privateHeap, NULL, infoBuffer);
		HeapFree(privateHeap, NULL, baseName);
		return true;
	}
}

////////////////////////////
// main program execution //
////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	privateHeap = HeapCreate(NULL, 100000, 0);

	PROCESSENTRY32 pEntry;
	if (!determineParentProcessEntry32(&pEntry, GetCurrentProcessId()))
	{
		MessageBox(NULL, "Unable to determine the Parent Process Information", "DisplayParentProcess", MB_OK | MB_ICONSTOP);
	}
	else
	{
		char* processInfo = (char*)HeapAlloc(privateHeap, HEAP_ZERO_MEMORY, 1024);
		wsprintf(processInfo, "Current Process Id - 0x%X\r\nParent Process Id - 0x%X\r\nParent File Name - %s", GetCurrentProcessId(), pEntry.th32ProcessID, pEntry.szExeFile);
		MessageBox(NULL, processInfo, "DisplayParentProcess", MB_OK | MB_ICONINFORMATION);
		HeapFree(privateHeap, NULL, processInfo);
	}

	HeapDestroy(privateHeap);
	return ERROR_SUCCESS;
}