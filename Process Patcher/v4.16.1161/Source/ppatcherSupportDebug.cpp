/////////////////////////////
// Debug support functions //
/////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "ppatcherSupport.h"
#include "ppatcherSupportDebug.h"

// check the system information class
void checkSystemInformationClass(void)
{
	if (!ALLOW_DEBUG_SUPPORT) return;

	char* SYSTEM_INFORMATION_CLASS_STRINGS[] = 
	{
		"SystemBasicInformation",
		"SystemProcessorInformation",
		"SystemPerformanceInformation",
		"SystemTimeOfDayInformation",
		"SystemPathInformation",
		"SystemProcessInformation",
		"SystemCallCountInformation",
		"SystemDeviceInformation",
		"SystemProcessorPerformanceInformation",
		"SystemFlagsInformation",
		"SystemCallTimeInformation",
		"SystemModuleInformation",
		"SystemLocksInformation",
		"SystemStackTraceInformation",
		"SystemPagedPoolInformation",
		"SystemNonPagedPoolInformation",
		"SystemHandleInformation",
		"SystemObjectInformation",
		"SystemPageFileInformation",
		"SystemVdmInstemulInformation",
		"SystemVdmBopInformation",
		"SystemFileCacheInformation",
		"SystemPoolTagInformation",
		"SystemInterruptInformation",
		"SystemDpcBehaviorInformation",
		"SystemFullMemoryInformation",
		"SystemLoadGdiDriverInformation",
		"SystemUnloadGdiDriverInformation",
		"SystemTimeAdjustmentInformation",
		"SystemSummaryMemoryInformation",
		"SystemNextEventIdInformation",
		"SystemEventIdsInformation",
		"SystemCrashDumpInformation",
		"SystemExceptionInformation",
		"SystemCrashDumpStateInformation",
		"SystemKernelDebuggerInformation",
		"SystemContextSwitchInformation",
		"SystemRegistryQuotaInformation",
		"SystemExtendServiceTableInformation",
		"SystemPrioritySeparation",
		"SystemPlugPlayBusInformation",
		"SystemDockInformation",
		"SystemPowerInformation",
		"SystemProcessorSpeedInformation",
		"SystemCurrentTimeZoneInformation",
		"SystemLookasideInformation",
		"SystemTimeSlipNotification",
		"SystemSessionCreate",
		"SystemSessionDetach",
		"SystemSessionInformation",
		"SystemRangeStartInformation",
		"SystemVerifierInformation",
		"SystemVerifierAddDriverInformation",
		"SystemSessionProcessInformation",
		"SystemLoadGdiDriverInSystemSpace",
		"SystemNumaProcessorMap",
		"SystemPrefetcherInformation",
		"SystemExtendedProcessInformation",
		"SystemRecommendedSharedDataAlignment",
		"SystemComPlusPackage",
		"SystemNumaAvailableMemoryNode",
		"SystemProcessorPowerInformation",
		"SystemEmulationBasicInformation",
		"SystemEmulationProcessorInformation",
		"SystemExtendedHandleInformation",
		"SystemLostDelayedWriteInformation",
		"SystemBigPoolInformation",
		"SystemSessionPoolTagInformation",
		"SystemSessionMappedViewInformation",
		"SystemHotpatchInformation",
		"SystemObjectSecurityMode",
		"SystemWatchdogTimerHandler",
		"SystemWatchdogTimerInformation",
		"SystemLogicalProcessorInformation",
		"SystemWow64SharedInformation",
		"SystemRegisterFirmwareTableInformati...",
		"SystemFirmwareTableInformation",
		"SystemModuleInformationEx",
		"SystemVerifierTriageInformation",
		"SystemSuperfetchInformation",
		"SystemMemoryListInformation",
		"SystemFileCacheInformationEx",
		"SystemThreadPriorityClientIdInformation",
		"SystemProcessorIdleCycleTimeInformation",
		"SystemVerifierCancellationInformation",
		"SystemProcessorPowerInformationEx",
		"SystemRefTraceInformation",
		"SystemSpecialPoolInformation",
		"SystemProcessIdInformation",
		"SystemErrorPortInformation",
		"SystemBootEnvironmentInformation",
		"SystemHypervisorInformation",
		"SystemVerifierInformationEx",
		"SystemTimeZoneInformation",
		"SystemImageFileExecutionOptionsInformation",
		"SystemCoverageInformation",
		"SystemPrefetchPathInformation",
		"SystemVerifierFaultsInformation",
		"SystemUnknown1",
		"SystemUnknown2",
		"SystemUnknown3",
		"SystemUnknown4",
		"SystemUnknown5",
		"SystemUnknown6",
		"SystemUnknown7",
		"SystemUnknown8",
		"SystemUnknown9",
		"SystemUnknown10"
	};

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage(" System Information Class\r\n==========================");
	ULib.ConsoleLibrary.displayConsoleMessage("");

	ULib.ConsoleLibrary.displayConsoleMessage(" Id    | Class Name                              | Supported?   | Class Size");
	ULib.ConsoleLibrary.displayConsoleMessage(" ------------------------------------------------------------------------------");

	char* outputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
	API_NTDLL_MODULE::NTQUERYSYSTEMINFORMATION pNtQuerySystemInformation = getNTDLLApiFunctions().getNtQuerySystemInformation();
	int numberOfClasses = (sizeof(SYSTEM_INFORMATION_CLASS_STRINGS) / sizeof(char*));

	NTSTATUS dwStatus;
	UCHAR* pSystemInformationClass1 = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap();
	UCHAR* pSystemInformationClass2 = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap();
	DWORD classSize1 = 0;
	DWORD classSize2 = 0;
	DWORD returnLength = 0;

	// cycle through each class
	for (int i = 0; i < numberOfClasses; i++)
	{
		// get class size
		classSize1 = 0;

		do
		{
			ZeroMemory(pSystemInformationClass1, 1024);
			ZeroMemory(pSystemInformationClass2, 1024);
			classSize2 = 0;

			// SystemTimeOfDayInformation
			if (i == 0x03) classSize2 = 128 - classSize1;
			// SystemProcessInformation
			if (i == 0x05) ((API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)pSystemInformationClass1)->NumberOfThreads = 1;
			// SystemSessionProcessInformation
			if (i == 0x35)
			{
				((API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)pSystemInformationClass2)->NumberOfThreads = 1;
				((API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION*)pSystemInformationClass1)->Buffer = pSystemInformationClass2;
				((API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION*)pSystemInformationClass1)->SizeOfBuf = 1024;
				((API_NTDLL_MODULE::SYSTEM_SESSION_PROCESS_INFORMATION*)pSystemInformationClass1)->SessionId = (DWORD)-1;
			}

			dwStatus = pNtQuerySystemInformation((API_NTDLL_MODULE::SYSTEM_INFORMATION_CLASS)i, pSystemInformationClass1, ((classSize2 > 0) ? classSize2 : classSize1), &returnLength);

			// special cases
			if (dwStatus == STATUS_INFO_LENGTH_MISMATCH)
			{
				// SystemProcessInformation
				if ((i == 0x05) && (((API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)pSystemInformationClass1)->NumberOfThreads == 0)) dwStatus = STATUS_SUCCESS;
				// SystemModuleInformation
				else if ((i == 0x0B) && (ULib.StringLibrary.getDWORDFromBuffer(pSystemInformationClass1, 0x0C) > 0)) dwStatus = STATUS_SUCCESS;
				// SystemLocksInformation
				else if ((i == 0x0C) && (ULib.StringLibrary.getDWORDFromBuffer(pSystemInformationClass1) > 0)) dwStatus = STATUS_SUCCESS;
				// SystemHandleInformation
				else if ((i == 0x10) && (ULib.StringLibrary.getDWORDFromBuffer(pSystemInformationClass1) > 0)) dwStatus = STATUS_SUCCESS;
				// SystemObjectInformation
				else if ((i == 0x11) && (ULib.StringLibrary.getDWORDFromBuffer(pSystemInformationClass1, 0x0C) > 0)) dwStatus = STATUS_SUCCESS;
				// SystemSessionProcessInformation
				else if ((i == 0x35) && (((API_NTDLL_MODULE::SYSTEM_PROCESS_INFORMATION*)pSystemInformationClass2)->NumberOfThreads == 0)) dwStatus = STATUS_SUCCESS;
			}
			else
			{
				// SystemTimeOfDayInformation
				if ((dwStatus == STATUS_SUCCESS) && (i == 0x03)) classSize1 = classSize2;
				// SystemCrashDumpInformation
				if ((dwStatus == STATUS_ACCESS_DENIED) && (i == 0x20)) dwStatus = STATUS_SUCCESS;
			}

			classSize1++;
		} while ((dwStatus == STATUS_INFO_LENGTH_MISMATCH) && (classSize1 <= 1024));

		// display info
		sprintf_s(outputMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, " 0x%02X  | %s%s | %s          | 0x%02X (%d)",
			i,
			SYSTEM_INFORMATION_CLASS_STRINGS[i],
			ULib.StringLibrary.getString(' ', 39 - ULib.StringLibrary.getStringLength(SYSTEM_INFORMATION_CLASS_STRINGS[i])),
			((dwStatus == STATUS_INVALID_INFO_CLASS) ? "No " : (((dwStatus == STATUS_NOT_IMPLEMENTED) || (dwStatus == STATUS_BREAKPOINT)) ? "N/A" : ((dwStatus == STATUS_SUCCESS) ? "Yes" : "???"))),
			--classSize1, classSize1);

		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
	}

	ULib.HeapLibrary.freePrivateHeap(pSystemInformationClass1);
	ULib.HeapLibrary.freePrivateHeap(pSystemInformationClass2);
}

// check the process information class
void checkProcessInformationClass(void)
{
	if (!ALLOW_DEBUG_SUPPORT) return;

	char* PROCESS_INFORMATION_CLASS_STRINGS[] = 
	{
		"ProcessBasicInformation",
		"ProcessQuotaLimits",
		"ProcessIoCounters",
		"ProcessVmCounters",
		"ProcessTimes",
		"ProcessBasePriority",
		"ProcessRaisePriority",
		"ProcessDebugPort",
		"ProcessExceptionPort",
		"ProcessAccessToken",
		"ProcessLdtInformation",
		"ProcessLdtSize",
		"ProcessDefaultHardErrorMode",
		"ProcessIoPortHandlers",
		"ProcessPooledUsageAndLimits",
		"ProcessWorkingSetWatch",
		"ProcessUserModeIOPL",
		"ProcessEnableAlignmentFaultFixup",
		"ProcessPriorityClass",
		"ProcessWx86Information",
		"ProcessHandleCount",
		"ProcessAffinityMask",
		"ProcessPriorityBoost",
		"ProcessDeviceMap",
		"ProcessSessionInformation",
		"ProcessForegroundInformation",
		"ProcessWow64Information",
		"ProcessImageFileName",
		"ProcessLUIDDeviceMapsEnabled",
		"ProcessBreakOnTermination",
		"ProcessDebugObjectHandle",
		"ProcessDebugFlags",
		"ProcessHandleTracing",
		"ProcessIoPriority",
		"ProcessExecuteFlags",
		"ProcessResourceManagement",
		"ProcessCookie",
		"ProcessImageInformation",
		"ProcessCycleTime",
		"ProcessPagePriority",
		"ProcessInstrumentationCallback",
		"ProcessUnknown1",
		"ProcessUnknown2",
		"ProcessUnknown3",
		"ProcessUnknown4",
		"ProcessUnknown5",
		"ProcessUnknown6",
		"ProcessUnknown7",
		"ProcessUnknown8",
		"ProcessUnknown9",
		"ProcessUnknown10"
	};

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage(" Process Information Class\r\n===========================");
	ULib.ConsoleLibrary.displayConsoleMessage("");

	ULib.ConsoleLibrary.displayConsoleMessage(" Id    | Class Name                              | Supported?   | Class Size");
	ULib.ConsoleLibrary.displayConsoleMessage(" ------------------------------------------------------------------------------");

	char* outputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
	API_NTDLL_MODULE::NTQUERYINFORMATIONPROCESS pNtQueryInformationProcess = getNTDLLApiFunctions().getNtQueryInformationProcess();
	int numberOfClasses = (sizeof(PROCESS_INFORMATION_CLASS_STRINGS) / sizeof(char*));

	NTSTATUS dwStatus;
	UCHAR* pProcessInformationClass1 = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap();
	DWORD classSize1 = 0;
	DWORD returnLength = 0;

	// cycle through each class
	for (int i = 0; i < numberOfClasses; i++)
	{
		// get class size
		classSize1 = 0;

		do
		{
			ZeroMemory(pProcessInformationClass1, 1024);

			dwStatus = pNtQueryInformationProcess(GetCurrentProcess(), (API_NTDLL_MODULE::PROCESS_INFORMATION_CLASS)i, pProcessInformationClass1, classSize1, &returnLength);

			// special cases
			if (dwStatus == STATUS_INFO_LENGTH_MISMATCH)
			{
			}
			else
			{
				// ProcessWorkingSetWatch
				if ((dwStatus == STATUS_UNSUCCESSFUL) && (i == 0x0F)) dwStatus = STATUS_INVALID_INFO_CLASS;
			}

			classSize1++;
		} while ((dwStatus == STATUS_INFO_LENGTH_MISMATCH) && (classSize1 <= 1024));

		// display info
		sprintf_s(outputMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, " 0x%02X  | %s%s | %s          | 0x%02X (%d)",
			i,
			PROCESS_INFORMATION_CLASS_STRINGS[i],
			ULib.StringLibrary.getString(' ', 39 - ULib.StringLibrary.getStringLength(PROCESS_INFORMATION_CLASS_STRINGS[i])),
			((dwStatus == STATUS_INVALID_INFO_CLASS) ? "No " : (((dwStatus == STATUS_NOT_IMPLEMENTED) || (dwStatus == STATUS_BREAKPOINT)) ? "N/A" : ((dwStatus == STATUS_SUCCESS) ? "Yes" : "???"))),
			--classSize1, classSize1);

		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
	}

	ULib.HeapLibrary.freePrivateHeap(pProcessInformationClass1);
}

// display debug support menu
void displayDebugSupport(void)
{
	if ((!ALLOW_DEBUG_SUPPORT) || (!ULib.OperatingSystemLibrary.isWindowsNTx())) return;

	SetConsoleCtrlHandler(NULL, TRUE);
	adjustConsoleScreenBufferSize(1000);
	unsigned char consoleInput = NULL;

	do
	{
		// display menu and wait for choice
		if (consoleInput != NULL)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");
			ULib.ConsoleLibrary.displayConsoleMessage(ULib.StringLibrary.getString('-', 79));
			ULib.ConsoleLibrary.displayConsoleMessage("");
		}
		ULib.ConsoleLibrary.displayConsoleMessage(" Debug Support Menu\r\n====================");
		ULib.ConsoleLibrary.displayConsoleMessage(" - 0: Quit");
		ULib.ConsoleLibrary.displayConsoleMessage(" - 1: System Information Class (Query)");
		ULib.ConsoleLibrary.displayConsoleMessage(" - 2: Process Information Class (Query)");
		consoleInput = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "012");

		// system information class
		if (consoleInput == '1') checkSystemInformationClass();
		// process information class
		else if (consoleInput == '2') checkProcessInformationClass();

	} while (consoleInput != '0');

	resetConsoleScreenBufferSize();
}