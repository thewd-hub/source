///////////////////////////////////////////////
// Support functions for the process patcher //
///////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

// the original console screen buffer info
CONSOLE_SCREEN_BUFFER_INFO* csbiInfo = NULL;
// process patcher window hidden
bool ppWindowHidden = false;

// displays script information which includes author and contact information (if available)
void displayScriptInformation(void)
{
	PATCHINGDETAILS_GLOBAL* pGlobalDetails = getGlobalPatchingDetails();
	bool foundPatchInformation = false;

	// display information
	displayConsoleMessage("[Display Script Information]");

	// display patch information, if available
	if (ULib.StringLibrary.isString(pGlobalDetails->patchInformation, 1))
	{
		char* patchInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
		sprintf_s(patchInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "  Information : %s", pGlobalDetails->patchInformation);
		displayConsoleMessage(patchInformation);
		ULib.HeapLibrary.freePrivateHeap(patchInformation);
		foundPatchInformation = true;
	}
	// display author information, if available
	if (ULib.StringLibrary.isString(pGlobalDetails->patchAuthor, 1))
	{
		char* patchAuthor = (char*)ULib.HeapLibrary.allocPrivateHeap();
		sprintf_s(patchAuthor, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "  Author      : %s", pGlobalDetails->patchAuthor);
		displayConsoleMessage(patchAuthor);
		ULib.HeapLibrary.freePrivateHeap(patchAuthor);
		foundPatchInformation = true;
	}
	// display patch contact information, if available
	if (ULib.StringLibrary.isString(pGlobalDetails->patchContactInformation, 1))
	{
		char* patchContactInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
		sprintf_s(patchContactInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "  Contact     : %s", pGlobalDetails->patchContactInformation);
		displayConsoleMessage(patchContactInformation);
		ULib.HeapLibrary.freePrivateHeap(patchContactInformation);
		foundPatchInformation = true;
	}

	// we have not found any patch information
	if (!foundPatchInformation) displayConsoleMessage("  <Unable to find any Script Information>");
}

// output information to the console (ANSI STRING)
void displayConsoleMessage(char* debugMessage, bool createNewLine, bool isDebugMessage)
{
	// only display debug information, if allowed
	if ((isDebugMessage) && (!isDebugInformationEnabled())) return;

	// check if shuting down
	if (isProcessPatcherShutingDown()) return;

	__try
	{
		if (getDisplayConsoleMessageLock() != NULL) EnterCriticalSection(getDisplayConsoleMessageLock());

		if (debugMessage != NULL)
		{
			if (!isDebugMessage) ULib.ConsoleLibrary.displayConsoleMessage(debugMessage, createNewLine);
			else
			{
				char* outputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				sprintf_s(outputMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "=> %s", debugMessage);
				ULib.ConsoleLibrary.displayConsoleMessage(outputMessage, createNewLine);
				ULib.HeapLibrary.freePrivateHeap(outputMessage);
			}
		}
	}
	__finally
	{
		if (getDisplayConsoleMessageLock() != NULL) LeaveCriticalSection(getDisplayConsoleMessageLock());
	}
}

// shows the process patcher console window
void showProcessPatcherWindow(bool forceShowWindow)
{
	// only display window if launched from the command prompt or forced
	if ((ppWindowHidden) && ((!ULib.ProcessLibrary.isParentProcessWindowsExplorer()) || (forceShowWindow)))
	{
		ppWindowHidden = false;
		ShowWindow(ULib.ConsoleLibrary.getConsoleWindowHandle(), SW_SHOW);
	}
}

// hides the process patcher console window
void hideProcessPatcherWindow(void)
{
	if (!ppWindowHidden)
	{
		ppWindowHidden = true;
		ShowWindow(ULib.ConsoleLibrary.getConsoleWindowHandle(), SW_HIDE);
	}
}

// returns whether the process patcher console window is hidden
bool isProcessPatcherWindowHidden(void)
{
	return ppWindowHidden;
}

// adjusts the buffer size height by increasing the number of rows
void adjustConsoleScreenBufferSize(unsigned short numberOfRows)
{
	// get the current screen buffer information
	csbiInfo = (CONSOLE_SCREEN_BUFFER_INFO*)ULib.HeapLibrary.allocPrivateHeap(sizeof(CONSOLE_SCREEN_BUFFER_INFO));
	GetConsoleScreenBufferInfo(ULib.ConsoleLibrary.getConsoleOutputHandle(), csbiInfo);

	// set the new console properties
	CONSOLE_SCREEN_BUFFER_INFO* csbiInfoNew = (CONSOLE_SCREEN_BUFFER_INFO*)ULib.HeapLibrary.allocPrivateHeap(sizeof(CONSOLE_SCREEN_BUFFER_INFO));
	memcpy(csbiInfoNew, csbiInfo, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
	csbiInfoNew->dwSize.Y = numberOfRows;

	// set the console properties
	SetConsoleScreenBufferSize(ULib.ConsoleLibrary.getConsoleOutputHandle(), csbiInfoNew->dwSize);
	ULib.HeapLibrary.freePrivateHeap(csbiInfoNew);
}

// resets the buffer size to its original settings
void resetConsoleScreenBufferSize(void)
{
	if (csbiInfo != NULL)
	{
		SetConsoleScreenBufferSize(ULib.ConsoleLibrary.getConsoleOutputHandle(), csbiInfo->dwSize);
		ULib.HeapLibrary.freePrivateHeap(csbiInfo);
	}
}

// is the target process which is to be patched this process patcher
bool isTargetProcessPatcher(void)
{
	// get filesize and read last 0x2000 bytes
	unsigned long fileSize = ULib.FileLibrary.getFileSize(getTargetProcessPatchingDetails()->fileName);
	if (fileSize <= 0x2000) return false;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(getTargetProcessPatchingDetails()->fileName, NULL, fileSize - 0x2000, 0x2000, true);
	if (fileBuffer == NULL) return false;

	// check for ppatcher.exe as the OriginalFilename and thewd string
	bool foundOriginalFilename = false;
	bool foundNameString = false;
	for (int i = 0; i < 0x2000; i++)
	{
		if ((fileBuffer[i] == 'O') && (fileBuffer[i + 2] == 'r') && (wcscmp((PWSTR)&fileBuffer[i], L"OriginalFilename\0ppatcher.exe\0") == 0)) foundOriginalFilename = true;
		if ((fileBuffer[i] == 't') && (fileBuffer[i + 2] == 'h') && (fileBuffer[i + 4] == 'e') && (fileBuffer[i + 6] == 'w') && (fileBuffer[i + 8] == 'd')) foundNameString = true;
	}

	return ((foundOriginalFilename) && (foundNameString));
}