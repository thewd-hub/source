//////////////////////////////////////////////////////////////////////
// Handles the patching of the target process and any child process //
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingModule.h"
#include "enginePatchingProcess.h"
#include "enginePatchingSupport.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

class PATCHINGENGINE_TARGETPROCESS
{
	public:
		bool isDisplayDebugInformationEnabled;
		DWORD currentProcessSessionId;
		bool ignoreSessionId;
};

PATCHINGENGINE_TARGETPROCESS pTargetProcess;
PROCESS_INFORMATION_EX* pTargetProcessInformationEx = NULL;
PROCESS_INFORMATION_EX* pChildProcessInformationEx = NULL;

PROCESS_INFORMATION_EX* pAllProcessInformationEx[1024];
int dwAllProcessInformationEx = 0;

char* currentWindowName = NULL;
char* requiredWindowName = NULL;
bool foundRequiredWindowName = false;

// initialises the process patching structures before calling the execution thread
bool initialiseTargetProcessPatching(void)
{
	// is debug information enabled
	pTargetProcess.isDisplayDebugInformationEnabled = ((ALLOW_DEBUG_INFORMATION) && (isDebugInformationEnabled()));

	// get current process session id
	pTargetProcess.currentProcessSessionId = getProcessSessionId(GetCurrentProcessId());
	pTargetProcess.ignoreSessionId = getGlobalPatchingDetails()->ignoreSessionId;

	// initialise target process information
	pTargetProcessInformationEx = (PROCESS_INFORMATION_EX*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PROCESS_INFORMATION_EX));
	pTargetProcessInformationEx->lpFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	return true;
}

// uninitialise the process patching structures
bool uninitialiseTargetProcessPatching(void)
{
	if (pTargetProcessInformationEx != NULL)
	{
		if (pTargetProcessInformationEx->hProcess != NULL) CloseHandle(pTargetProcessInformationEx->hProcess);
		if (pTargetProcessInformationEx->hThread != NULL) CloseHandle(pTargetProcessInformationEx->hThread);
		ULib.HeapLibrary.freePrivateHeap(pTargetProcessInformationEx->lpFilename);
		ULib.HeapLibrary.freePrivateHeap(pTargetProcessInformationEx);
	}

	return true;
}

// returns the target process file name
char* getTargetProcessFilename(void)
{
	return ((pTargetProcessInformationEx == NULL) ? NULL : pTargetProcessInformationEx->lpFilename);
}

// returns the target process id
unsigned long getTargetProcessId(void)
{
	return ((pTargetProcessInformationEx == NULL) ? 0 : pTargetProcessInformationEx->dwProcessId);
}

// returns a handle to the target process
HANDLE getTargetProcessHandle(void)
{
	return ((pTargetProcessInformationEx == NULL) ? NULL : pTargetProcessInformationEx->hProcess);
}

// returns the id of the primary thread to the target process
unsigned long getTargetProcessThreadId(void)
{
	return ((pTargetProcessInformationEx == NULL) ? 0 : pTargetProcessInformationEx->dwThreadId);
}

// returns the handle of the primary thread to the target process
HANDLE getTargetProcessThreadHandle(void)
{
	return ((pTargetProcessInformationEx == NULL) ? NULL : pTargetProcessInformationEx->hThread);
}

// returns the target process session id
unsigned long getTargetProcessSessionId(void)
{
	if (pTargetProcessInformationEx == NULL) return 0;
	if (getTargetProcessId() == 0) return 0;
	return getProcessSessionId(getTargetProcessId());
}

// returns the child process file name
char* getChildProcessFilename(void)
{
	return ((pChildProcessInformationEx == NULL) ? NULL : pChildProcessInformationEx->lpFilename);
}

// returns the child process id
unsigned long getChildProcessId(void)
{
	return ((pChildProcessInformationEx == NULL) ? 0 : pChildProcessInformationEx->dwProcessId);
}

// returns a handle to the child process
HANDLE getChildProcessHandle(void)
{
	return ((pChildProcessInformationEx == NULL) ? NULL : pChildProcessInformationEx->hProcess);
}

// returns the id of the primary thread to the child process
unsigned long getChildProcessThreadId(void)
{
	return ((pChildProcessInformationEx == NULL) ? 0 : pChildProcessInformationEx->dwThreadId);
}

// returns the handle of the primary thread to the child process
HANDLE getChildProcessThreadHandle(void)
{
	return ((pChildProcessInformationEx == NULL) ? NULL : pChildProcessInformationEx->hThread);
}

// returns the child process session id
unsigned long getChildProcessSessionId(void)
{
	if (pChildProcessInformationEx == NULL) return 0;
	if (getChildProcessId() == 0) return 0;
	return getProcessSessionId(getChildProcessId());
}

// returns the main process file name
char* getMainProcessFilename(void)
{
	return ((pChildProcessInformationEx != NULL) ? getChildProcessFilename() : getTargetProcessFilename());
}

// returns the main process id
unsigned long getMainProcessId(void)
{
	return ((pChildProcessInformationEx != NULL) ? getChildProcessId() : getTargetProcessId());
}

// returns a handle to the main process
HANDLE getMainProcessHandle(void)
{
	return ((pChildProcessInformationEx != NULL) ? getChildProcessHandle() : getTargetProcessHandle());
}

// returns the id of the primary thread of the main process
unsigned long getMainProcessThreadId(void)
{
	return ((pChildProcessInformationEx != NULL) ? getChildProcessThreadId() : getTargetProcessThreadId());
}

// returns the handle of the primary thread of the main process
HANDLE getMainProcessThreadHandle(void)
{
	return ((pChildProcessInformationEx != NULL) ? getChildProcessThreadHandle() : getTargetProcessThreadHandle());
}

// returns the peb ldr address of the main process
PVOID getMainProcessPebLdrAddress(void)
{
	if (pChildProcessInformationEx != NULL)
	{
		if (pChildProcessInformationEx->pPebLdrAddress == 0) getPebInformation(getMainProcessHandle(), &pChildProcessInformationEx->pPebLdrAddress, &pChildProcessInformationEx->pImageBaseAddress);
		return pChildProcessInformationEx->pPebLdrAddress;
	}

	if (pTargetProcessInformationEx != NULL)
	{
		if (pTargetProcessInformationEx->pPebLdrAddress == 0) getPebInformation(getMainProcessHandle(), &pTargetProcessInformationEx->pPebLdrAddress, &pTargetProcessInformationEx->pImageBaseAddress);
		return pTargetProcessInformationEx->pPebLdrAddress;
	}

	return NULL;
}

// returns the image base address of the main process
PVOID getMainProcessImageBaseAddress(void)
{
	if (getMainProcessPebLdrAddress() != NULL)
	{
		if (pChildProcessInformationEx != NULL) return pChildProcessInformationEx->pImageBaseAddress;
		if (pTargetProcessInformationEx != NULL) return pTargetProcessInformationEx->pImageBaseAddress;
	}

	return NULL;
}

// display message box and wait for user to confirm ok
// (Argument - PATCHINGDETAILS_TARGETPROCESS)
void displayUserNotifyMessageBox(LPVOID args)
{
	PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails = (PATCHINGDETAILS_TARGETPROCESS*)args;

	char* displayUserNotifyMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
	// display standard message if no user defined message is supplied
	if ((pTargetProcessDetails->displayUserNotifyBoxMessage == NULL) || (pTargetProcessDetails->displayUserNotifyBoxMessage[0] == NULL)) sprintf_s(displayUserNotifyMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Waiting for User Notification before Proceeding. Press OK to proceed...");
	// otherwise, display user defined message along with the standard message
	else sprintf_s(displayUserNotifyMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Waiting for User Notification before Proceeding. Press OK to proceed...\r\n\r\n%s", pTargetProcessDetails->displayUserNotifyBoxMessage);
	MessageBox(GetDesktopWindow(), displayUserNotifyMessage, "Process Patcher", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL | MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST);
	ULib.HeapLibrary.freePrivateHeap(displayUserNotifyMessage);

	ExitThread(ERROR_SUCCESS);
}

// callback function used by EnumWindows for each window it finds on the current desktop.
// Determines whether we have found the window we are looking for, Otherwise, continue
BOOL CALLBACK EnumGlobalWindowsHandler(HWND hWnd, LPARAM lParam)
{
	if (IsWindowEnabled(hWnd))
	{
		if (GetWindowText(hWnd, currentWindowName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE) != 0)
		{
			// check for window name match
			foundRequiredWindowName = ULib.StringLibrary.isStringPatternMatch(currentWindowName, requiredWindowName);
			if ((foundRequiredWindowName) && (!pTargetProcess.ignoreSessionId))
			{
				// check session id matches current session id
				unsigned long processId = 0;
				GetWindowThreadProcessId(hWnd, &processId);
				foundRequiredWindowName = (getProcessSessionId(processId) == pTargetProcess.currentProcessSessionId);
			}
		}
	}

	return ((foundRequiredWindowName) ? FALSE : TRUE);
}

// callback function used by EnumWindows for each window it finds on the current desktop.
// Determines whether we have found the window we are looking for, Otherwise, continue
BOOL CALLBACK EnumLocalWindowsHandler(HWND hWnd, LPARAM lParam)
{
	// get window process id and check it matches main process
	unsigned long processId = 0;
	GetWindowThreadProcessId(hWnd, &processId);
	if (processId == getMainProcessId()) return EnumGlobalWindowsHandler(hWnd, lParam);
	return TRUE;
}

// remove all the processes from the dialog list contol and free used memory
void removeExistingProcessesFromList(HWND hDlgWnd)
{
	if (hDlgWnd != NULL) SendDlgItemMessage(hDlgWnd, 301, LB_RESETCONTENT, NULL, NULL);
	for (int i = 0; i < dwAllProcessInformationEx; i++)
	{
		if (pAllProcessInformationEx[i]->hProcess != NULL) CloseHandle(pAllProcessInformationEx[i]->hProcess);
		ULib.HeapLibrary.freePrivateHeap(pAllProcessInformationEx[i]->lpFilename);
		ULib.HeapLibrary.freePrivateHeap(pAllProcessInformationEx[i]);
	}
}

// add all the processes to the dialog list contol
void addExistingProcessesToList(HWND hDlgWnd)
{
	// get all the processes
	if ((!getAllProcessInformation(pAllProcessInformationEx, &dwAllProcessInformationEx, 1024)) || (dwAllProcessInformationEx <= 0)) displayErrorMessage(ERROR_PATCHING_CHILDPROCESS_DETERMINEPROCESSES, true);

	bool ignoreSessionId = getGlobalPatchingDetails()->ignoreSessionId;

	// add each process to the list box
	unsigned long listIndex;
	char* listString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 0; i < dwAllProcessInformationEx; i++)
	{
		char* fileNameLowerCase = ULib.FileLibrary.stripPathFromFilename(pAllProcessInformationEx[i]->lpFilename, true);
		if ((ignoreSessionId) && (ULib.OperatingSystemLibrary.isWindows2000(true))) sprintf_s(listString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s - %d (%d)", ((ULib.StringLibrary.isString(fileNameLowerCase, 1)) ? fileNameLowerCase : "<no name>"), pAllProcessInformationEx[i]->dwProcessId, pAllProcessInformationEx[i]->dwSessionId);
		else sprintf_s(listString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, ((ULib.OperatingSystemLibrary.isWindowsNTx()) ? "%s - %d" : "%s - 0x%x"), ((ULib.StringLibrary.isString(fileNameLowerCase, 1)) ? fileNameLowerCase : "<no name>"), pAllProcessInformationEx[i]->dwProcessId);
		listIndex = (unsigned long)SendDlgItemMessage(hDlgWnd, 301, LB_ADDSTRING, NULL, (LPARAM)listString);
		SendDlgItemMessage(hDlgWnd, 301, LB_SETITEMDATA, listIndex, (LPARAM)pAllProcessInformationEx[i]->dwProcessId);
		ULib.HeapLibrary.freePrivateHeap(fileNameLowerCase);
	}

	// disable continue button
	EnableWindow(GetDlgItem(hDlgWnd, 302), FALSE);
	ULib.HeapLibrary.freePrivateHeap(listString);
}

// message callback handler for selecting an existing process
LRESULT CALLBACK ExistingProcessDialogMessageHandler(HWND hDlgWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			addExistingProcessesToList(hDlgWnd);
			break;
		}
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				EnableWindow(GetDlgItem(hDlgWnd, 302), TRUE);
			}
			else if (wParam == IDCANCEL)
			{
				EndDialog(hDlgWnd, TRUE);
			}
			// continue
			else if (wParam == 302)
			{
				pTargetProcessInformationEx->dwProcessId = (DWORD)SendDlgItemMessage(hDlgWnd, 301, LB_GETITEMDATA, SendDlgItemMessage(hDlgWnd, 301, LB_GETCURSEL, NULL, NULL), NULL);
				EndDialog(hDlgWnd, TRUE);
			}
			// refresh
			else if (wParam == 303)
			{
				removeExistingProcessesFromList(hDlgWnd);
				addExistingProcessesToList(hDlgWnd);
			}

			break;
		}
		case WM_CLOSE:
			EndDialog(hDlgWnd, FALSE);
			break;
	}

	return FALSE;
}

// creates the target process and attempts to perform a patch on this process or child process
// (Argument - PATCHINGDETAILS_TARGETPROCESS)
void executeTargetProcessPatchingThread(LPVOID args)
{
	__try
	{
		PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails = (PATCHINGDETAILS_TARGETPROCESS*)args;

		// set the target process file name
		char* filePart = NULL;
		GetFullPathName(pTargetProcessDetails->fileName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, pTargetProcessInformationEx->lpFilename, &filePart);

		char* shortProcessFilename = ULib.FileLibrary.stripPathFromFilename(getTargetProcessFilename(), true);
		char* fullCommandLine = (char*)ULib.HeapLibrary.allocPrivateHeap((ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 2));
		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();

		// open existing process
		if (pTargetProcessDetails->openExistingProcess)
		{
			// display current processes
			if (ULib.StringLibrary.isStringMatch(pTargetProcessDetails->fileName, "<request>"))
			{
				pTargetProcessInformationEx->dwProcessId = 0;
				displayConsoleMessage("Paused Process Patching. Waiting for an Existing Process to be Selected", true, true);

				// open dialog box
				InitCommonControls();
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(300), GetDesktopWindow(), (DLGPROC)ExistingProcessDialogMessageHandler);
				removeExistingProcessesFromList(NULL);

				// get selected process id
				DWORD processId = pTargetProcessInformationEx->dwProcessId;
				if ((pTargetProcess.isDisplayDebugInformationEnabled) && (processId > 0))
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, ((ULib.OperatingSystemLibrary.isWindowsNTx()) ? "Finding Target Process (pid - %d)" : "Finding Target Process (pid - 0x%X)"), pTargetProcessInformationEx->dwProcessId);
					displayConsoleMessage(debugMessage, true, true);
				}

				// get process information
				ULib.HeapLibrary.freePrivateHeap(pTargetProcessInformationEx->lpFilename);
				ULib.HeapLibrary.freePrivateHeap(pTargetProcessInformationEx);
				pTargetProcessInformationEx = findProcessInformation(NULL, (PATCHINGDETAILS_FILEBASE*)pTargetProcessDetails, false, 0, processId);
				if (pTargetProcessInformationEx == NULL) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);
			}
			else
			{
				if ((pTargetProcess.isDisplayDebugInformationEnabled) && (ULib.StringLibrary.isString(shortProcessFilename, 1)))
				{
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Finding Target Process (%s)", shortProcessFilename);
					displayConsoleMessage(debugMessage, true, true);
				}

				// get process information
				ULib.HeapLibrary.freePrivateHeap(pTargetProcessInformationEx->lpFilename);
				ULib.HeapLibrary.freePrivateHeap(pTargetProcessInformationEx);
				pTargetProcessInformationEx = findProcessInformation(shortProcessFilename, (PATCHINGDETAILS_FILEBASE*)pTargetProcessDetails);
				if (pTargetProcessInformationEx == NULL) displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, true);
			}

			// update filename information
			ULib.HeapLibrary.freePrivateHeap(shortProcessFilename);
			shortProcessFilename = ULib.FileLibrary.stripPathFromFilename(getTargetProcessFilename(), true);
		}
		// executing target process
		else
		{
			if (pTargetProcess.isDisplayDebugInformationEnabled)
			{
				sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Executing Target Process (%s)", shortProcessFilename);
				displayConsoleMessage(debugMessage, true, true);
			}

			// check file size (if required)
			if (!checkFileSizeMatch(ULib.FileLibrary.getFileSize(getTargetProcessFilename()), (PATCHINGDETAILS_FILEBASE*)pTargetProcessDetails)) displayErrorMessage(((ULib.FileLibrary.doesFileExist(getTargetProcessFilename(), false)) ? ERROR_PATCHING_TARGETPROCESS_FILESIZE : ERROR_PATCHING_TARGETPROCESS_GENERAL), true);

			// process information
			PROCESS_INFORMATION processInformation;
			STARTUPINFO startupInformation;
			ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));
			ZeroMemory(&startupInformation, sizeof(STARTUPINFO));
			startupInformation.cb = sizeof(STARTUPINFO);

			// set the full command line which is used to execute the target process
			sprintf_s(fullCommandLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 2, "\"%s\" %s", getTargetProcessFilename(), pTargetProcessDetails->argumentsToForward);

			// process creation flags
			unsigned long processCreationFlags = CREATE_SUSPENDED | CREATE_NEW_CONSOLE | CREATE_SEPARATE_WOW_VDM | CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS;

			// create the process with a suspended primary thread
			if (CreateProcess(NULL, fullCommandLine, NULL, NULL, TRUE, processCreationFlags, NULL, getCurrentDirectory(), &startupInformation, &processInformation) == 0)
			{
				// handle error messages
				int lastErrorNumber = GetLastError();
				char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				if (lastErrorNumber == 2) sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s", shortProcessFilename);
				else sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Error #%d", lastErrorNumber);
				displayErrorMessage(ERROR_PATCHING_TARGETPROCESS_GENERAL, errorMessage, true);
			}

			// copy target process information
			pTargetProcessInformationEx->dwProcessId = processInformation.dwProcessId;
			pTargetProcessInformationEx->dwThreadId = processInformation.dwThreadId;
			DuplicateHandle(GetCurrentProcess(), processInformation.hProcess, GetCurrentProcess(), &pTargetProcessInformationEx->hProcess, NULL, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
			DuplicateHandle(GetCurrentProcess(), processInformation.hThread, GetCurrentProcess(), &pTargetProcessInformationEx->hThread, NULL, TRUE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
		}

		// attempt to hide this process patcher from the target process (if required)
		if ((!pTargetProcessDetails->openExistingProcess) && (getGlobalPatchingDetails()->enableStealthMode))
		{
			bool stealthModeSucceeded = attemptWin9xStealthMode(getTargetProcessId());
			if (pTargetProcess.isDisplayDebugInformationEnabled) displayConsoleMessage(((stealthModeSucceeded) ? "Stealth Mode Initialisation [Succeeded]" : "Stealth Mode Initialisation [Failed]"), true, true);
		}

		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_INITIALISED);

		// not opening an existing process
		if (!pTargetProcessDetails->openExistingProcess)
		{
			// if the target process is to remain suspended then wait for the
			// event to be signalled. A plugin must fire this waiting event
			if (pTargetProcessDetails->createSuspendedProcess)
			{
				// display debug information
				if (pTargetProcess.isDisplayDebugInformationEnabled) displayConsoleMessage("Suspended Target Process. Waiting for Resume Event to be Signalled", true, true);
				resetPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME);
			}

			waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_RESUME, true);
			ResumeThread(getTargetProcessThreadHandle());
		}

		// delay patching (in seconds)
		if (pTargetProcessDetails->waitForTimeDelay > 0)
		{
			// display debug information
			if (pTargetProcess.isDisplayDebugInformationEnabled)
			{
				sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Paused Process Patching. Waiting for Time Delay to Expire (%d seconds)", pTargetProcessDetails->waitForTimeDelay);
				displayConsoleMessage(debugMessage, true, true);
			}

			Sleep(pTargetProcessDetails->waitForTimeDelay * 1000);
		}

		// try to find the required window before proceeding with the patching engine
		if ((pTargetProcessDetails->waitForLocalWindowName != NULL) || (pTargetProcessDetails->waitForGlobalWindowName != NULL))
		{
			currentWindowName = (char*)ULib.HeapLibrary.allocPrivateHeap();
			requiredWindowName = (char*)ULib.HeapLibrary.allocPrivateHeap();

			bool isGlobalWindowName = ((pTargetProcessDetails->waitForGlobalWindowName != NULL) ? true : false);
			ULib.StringLibrary.copyString(requiredWindowName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, ((isGlobalWindowName) ? pTargetProcessDetails->waitForGlobalWindowName : pTargetProcessDetails->waitForLocalWindowName));

			// display debug information
			if (pTargetProcess.isDisplayDebugInformationEnabled)
			{
				// display required window name
				ULib.StringLibrary.copyString(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Paused Process Patching. Searching for a Window Name match");
				displayConsoleMessage(debugMessage, true, true);
			}

			// find window
			do
			{
				EnumWindows(((isGlobalWindowName) ? (WNDENUMPROC)EnumGlobalWindowsHandler : (WNDENUMPROC)EnumLocalWindowsHandler), NULL);
				if (hasProcessTerminated(getMainProcessHandle())) handleProcessPatchingFatalError(ERROR_PATCHING_TARGETPROCESS_MEMORYACCESS);
			} while (!foundRequiredWindowName);

			// display debug information
			if (pTargetProcess.isDisplayDebugInformationEnabled)
			{
				if (ULib.StringLibrary.doesStringContainPatternMatchingSymbols(requiredWindowName))
				{
					// display found window name
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Found Window Name using Pattern Matching (%s)", currentWindowName);
					displayConsoleMessage(debugMessage, true, true);
				}
			}

			ULib.HeapLibrary.freePrivateHeap(currentWindowName);
			ULib.HeapLibrary.freePrivateHeap(requiredWindowName);
		}

		// wait for user notifiction before patching the target process (if required)
		if (pTargetProcessDetails->displayUserNotifyBox)
		{
			// display debug information
			if (pTargetProcess.isDisplayDebugInformationEnabled) displayConsoleMessage("Paused Process Patching. Waiting for User Notification before Proceeding", true, true);

			HANDLE userNotifyThreadHandle;
			unsigned long userNotifyThreadId;
			userNotifyThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)displayUserNotifyMessageBox, pTargetProcessDetails, CREATE_SUSPENDED, &userNotifyThreadId);
			if ((userNotifyThreadId <= 0) || (userNotifyThreadHandle != NULL)) ResumeThread(userNotifyThreadHandle);
			WaitForSingleObject(userNotifyThreadHandle, INFINITE);
			CloseHandle(userNotifyThreadHandle);
		}

		// if the target process creates a child process then get the handle to this child process
		if ((pTargetProcessDetails->createsChildProcess) || (getGlobalPatchingDetails()->numberOfChildProcesses > 0))
		{
			// get process handle for the filename specified in the script file
			if ((getGlobalPatchingDetails()->numberOfChildProcesses > 0) && (!getChildProcessPatchingDetails()->isDisabled))
			{
				pTargetProcessDetails->createsChildProcess = true;
				pChildProcessInformationEx = findProcessInformation(getChildProcessPatchingDetails());
			}
			// otherwise, get the first child process found
			else if (pTargetProcessDetails->createsChildProcess)
			{
				pChildProcessInformationEx = findProcessInformation(getMainProcessId());
			}

			if (pChildProcessInformationEx == NULL) displayErrorMessage(ERROR_PATCHING_CHILDPROCESS_FINDPROCESS, true);
		}

		char* processFilename = ULib.FileLibrary.stripPathFromFilename(getMainProcessFilename(), true);

		// display debug information
		if (pTargetProcess.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Opening Process Handle (%s)", processFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		// unable to open the process handle
		if (getMainProcessHandle() == NULL) handleProcessPatchingFatalError(ERROR_PATCHING_TARGETPROCESS_MEMORYACCESS);

		// signal target process & child process initialisation waiting event
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_INITIALISED);
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_CHILDPROCESS_INITIALISED);

		// wait for the target process memory patching waiting event.
		// this event is originally signalled, unless reset by a plugin
		waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_READY, true);

		// display debug information
		if (pTargetProcess.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Attempting to Patch Process Memory (%s)", processFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		// no memory addresses to patch
		if (pTargetProcessDetails->numberOfAddresses <= 0)
		{
			if (pTargetProcess.isDisplayDebugInformationEnabled)
			{
				sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Patching Process Memory [No Patch Required] (%s)", processFilename);
				displayConsoleMessage(debugMessage, true, true);
			}
		}
		// do memory patch
		else
		{
			int memoryPatchRetryCount = 0;
			int memoryPatchFailCount = 0;
			bool memoryPatchSucceeded = false;
			bool processTerminated = false;

			// repeat this memory patching cycle ->retryCount times
			do
			{
				// patch memory bytes for the process
				memoryPatchSucceeded = performMemoryAddressPatch(getMainProcessHandle(), (PATCHINGDETAILS_MEMORY*)pTargetProcessDetails, (PATCHINGDETAILS_FILEBASE*)pTargetProcessDetails);

				// display debug information
				if (pTargetProcess.isDisplayDebugInformationEnabled)
				{
					// was the memory patch successful?
					sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Patching Process Memory [%s] (%s)", ((memoryPatchSucceeded) ? "Succeeded" : "Failed"), processFilename);
					displayConsoleMessage(debugMessage, true, true);
				}

				// check whether the process has terminated
				processTerminated = hasProcessTerminated(getMainProcessHandle());

				// if memory patching succeeded
				if (memoryPatchSucceeded)
				{
					if (processTerminated) break;
					memoryPatchRetryCount++;
					memoryPatchFailCount = 0;
				}
				else
				{
					// check if shuting down
					if (isProcessPatcherShutingDown()) break;

					// don't report error on patching retries
					if (memoryPatchRetryCount > 0)
					{
						if (processTerminated) break;
						Sleep(PATCHINGENGINE_SLEEPDELAY);
					}
					else
					{
						if (processTerminated) displayErrorMessage(ERROR_PATCHING_ENGINE_PROCESSTERMINATED, true);
						memoryPatchFailCount++;
					}
				}

				// if the memory patch fails more than 128 times in succession then abort
				if ((memoryPatchRetryCount <= 0) && (memoryPatchFailCount > 128))
				{
					if (pTargetProcess.isDisplayDebugInformationEnabled)
					{
						sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Patching Process Memory [Aborted] (%s)", processFilename);
						displayConsoleMessage(debugMessage, true, true);
					}

					break;
				}

			} while (memoryPatchRetryCount <= pTargetProcessDetails->retryCount);
		}

		// display debug information
		if (pTargetProcess.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Closing Process Handle (%s)", processFilename);
			displayConsoleMessage(debugMessage, true, true);
		}

		ULib.HeapLibrary.freePrivateHeap(fullCommandLine);
		ULib.HeapLibrary.freePrivateHeap(shortProcessFilename);
		ULib.HeapLibrary.freePrivateHeap(processFilename);
		ULib.HeapLibrary.freePrivateHeap(debugMessage);

		// signal waiting event
		setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_TARGETPROCESS_MEMORYPATCHING_FINISHED, true);
	}
	__except(handleCriticalException(GetExceptionInformation(), SECTIONTYPE_TARGETPROCESS))
	{
	}

	ExitThread(ERROR_SUCCESS);
}