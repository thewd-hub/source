//////////////////////////////////////////////////////////////////////////////
// Loads the required modules and initialises the appropriate api functions //
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"

API_KERNEL32_MODULE apiKERNEL32;
API_NTDLL_MODULE apiNTDLL;

// return the instance of the KERNEL32 module class
API_KERNEL32_MODULE getKERNEL32ApiFunctions(void)
{
	return apiKERNEL32;
}

// load the kernel32.dll module and retrieve the required APIs
bool initialiseKERNEL32ApiFunctions(void)
{
	// only applicable on Windows 9x
	if (!ULib.OperatingSystemLibrary.isWindows9x()) return true;

	// load the kernel.dll module
	HMODULE kernel32Module = ULib.FileLibrary.moduleLoadLibrary("kernel32.dll");
	if (kernel32Module == NULL) return false;

	// retrieve APIs from kernel32.dll
	apiKERNEL32.setCreateToolhelp32Snapshot(GetProcAddress(kernel32Module, "CreateToolhelp32Snapshot"));
	apiKERNEL32.setProcess32First(GetProcAddress(kernel32Module, "Process32First"));
	apiKERNEL32.setProcess32Next(GetProcAddress(kernel32Module, "Process32Next"));
	apiKERNEL32.setModule32First(GetProcAddress(kernel32Module, "Module32First"));
	apiKERNEL32.setModule32Next(GetProcAddress(kernel32Module, "Module32Next"));
	apiKERNEL32.setOpenThread(GetProcAddress(kernel32Module, "OpenThread"));
	return true;
}

// return the instance of the NTDLL module class
API_NTDLL_MODULE getNTDLLApiFunctions(void)
{
	return apiNTDLL;
}

// load the ntdll.dll module and retrieve the required APIs
bool initialiseNTDLLApiFunctions(void)
{
	// only applicable on Windows NTx
	if (!ULib.OperatingSystemLibrary.isWindowsNTx()) return true;

	// load the ntdll.dll module
	HMODULE ntdllModule = ULib.FileLibrary.moduleLoadLibrary("ntdll.dll");
	if (ntdllModule == NULL) return false;

	// retrieve APIs from ntdll.dll
	apiNTDLL.setNtQuerySystemInformation(GetProcAddress(ntdllModule, "NtQuerySystemInformation"));
	apiNTDLL.setNtQueryInformationProcess(GetProcAddress(ntdllModule, "NtQueryInformationProcess"));
	apiNTDLL.setNtOpenThread(GetProcAddress(ntdllModule, "NtOpenThread"));
	apiNTDLL.setRtlAdjustPrivilege(GetProcAddress(ntdllModule, "RtlAdjustPrivilege"));
	apiNTDLL.setRtlInitializeCriticalSectionAndSpinCount(GetProcAddress(ntdllModule, "RtlInitializeCriticalSectionAndSpinCount"));
	apiNTDLL.setRtlEnableEarlyCriticalSectionEventCreation(GetProcAddress(ntdllModule, "RtlEnableEarlyCriticalSectionEventCreation"));
	return true;
}