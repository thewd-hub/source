///////////////////////////////////////////////////////
// Process Patcher                                   //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "encryptionSupport.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"
#include "ppatcherSupportDebug.h"
#include "shellRegister.h"

CUtilitiesLibrary ULib;

bool DISPLAY_DEBUG_INFORMATION = false;
bool DISPLAY_SCRIPT_INFORMATION = false;

CRITICAL_SECTION memoryAccessLock;
CRITICAL_SECTION moduleBaseAddressLock;
CRITICAL_SECTION displayConsoleMessageLock;
CRITICAL_SECTION displayErrorMessageLock;
bool initialisedCriticalSections = false;
bool imageDebuggerRunning = false;

char* forwardedCommandLine;
char* currentDirectory;

// initialise the process patcher critical sections
void initialiseCriticalSections(void)
{
	API_NTDLL_MODULE::RTLINITIALIZECRITICALSECTIONANDSPINCOUNT pRtlInitializeCriticalSectionAndSpinCount = getNTDLLApiFunctions().getRtlInitializeCriticalSectionAndSpinCount();
	API_NTDLL_MODULE::RTLENABLEEARLYCRITICALSECTIONEVENTCREATION pRtlEnableEarlyCriticalSectionEventCreation = getNTDLLApiFunctions().getRtlEnableEarlyCriticalSectionEventCreation();
	if (pRtlEnableEarlyCriticalSectionEventCreation != NULL) pRtlEnableEarlyCriticalSectionEventCreation();

	if (pRtlInitializeCriticalSectionAndSpinCount != NULL)
	{
		pRtlInitializeCriticalSectionAndSpinCount(&memoryAccessLock, 0x80000000);
		pRtlInitializeCriticalSectionAndSpinCount(&moduleBaseAddressLock, 0x80000000);
		pRtlInitializeCriticalSectionAndSpinCount(&displayConsoleMessageLock, 0x80000000);
		pRtlInitializeCriticalSectionAndSpinCount(&displayErrorMessageLock, 0x80000000);
	}
	else
	{
		InitializeCriticalSection(&memoryAccessLock);
		InitializeCriticalSection(&moduleBaseAddressLock);
		InitializeCriticalSection(&displayConsoleMessageLock);
		InitializeCriticalSection(&displayErrorMessageLock);
	}

	initialisedCriticalSections = true;
}

// delete the process patcher critical sections
void deleteCriticalSections(void)
{
	initialisedCriticalSections = false;
	DeleteCriticalSection(&memoryAccessLock);
	DeleteCriticalSection(&moduleBaseAddressLock);
	DeleteCriticalSection(&displayConsoleMessageLock);
	DeleteCriticalSection(&displayErrorMessageLock);
}

// get the critical section for accessing the memory space of a process or module
CRITICAL_SECTION* getMemoryAccessLock(void)
{
	return ((initialisedCriticalSections) ? &memoryAccessLock : NULL);
}

// get the critical section for finding the base address for a process module
CRITICAL_SECTION* getModuleBaseAddressLock(void)
{
	return ((initialisedCriticalSections) ? &moduleBaseAddressLock : NULL);
}

// get the critical section for the console display output
CRITICAL_SECTION* getDisplayConsoleMessageLock(void)
{
	return ((initialisedCriticalSections) ? &displayConsoleMessageLock : NULL);
}

// get the critical section for displaying error messages
CRITICAL_SECTION* getDisplayErrorMessageLock(void)
{
	return ((initialisedCriticalSections) ? &displayErrorMessageLock : NULL);
}

// returns the unrecognised command-line arguments
char* getForwardedCommandLine(void)
{
	return forwardedCommandLine;
}

// returns the current directory value
char* getCurrentDirectory(void)
{
	return currentDirectory;
}

// is the DISPLAY_DEBUG_INFORMATION flag set to true
bool isDebugInformationEnabled(void)
{
	return DISPLAY_DEBUG_INFORMATION;
}

// has the DISPLAY_SCRIPT_INFORMATION flag been set to true
bool isDisplayScriptInformationEnabled(void)
{
	return DISPLAY_SCRIPT_INFORMATION;
}

// is process patcher running as an image debugger
bool isProcessPatcherRunningAsImageDebugger(void)
{
	return imageDebuggerRunning;
}

// parse command line arguments and determine the correct function to execute
void main(int argc, char* argv[])
{
	// hide process patcher window
	if (argc > 1)
	{
		if ((ULib.StringLibrary.isStringMatch(argv[1], "/hideWindow", false)) ||
			(ULib.StringLibrary.isStringMatch(argv[1], "/imageDebugger", false)))
		{
			hideProcessPatcherWindow();
		}
	}

	// display process patcher title
	ULib.ConsoleLibrary.displayApplicationTitle("Process Patcher", ppFullApplicationVersion, "\b; (C)1999-2011 thewd, thewd@hotmail.com");

	// check whether the shift key is depressed (enable debug mode and show window)
	if (ALLOW_DEBUG_INFORMATION)
	{
		DISPLAY_DEBUG_INFORMATION = ULib.ProcessLibrary.isDebugModeEnabled();
		if (isDebugInformationEnabled()) showProcessPatcherWindow(true);
	}

	// initialise error messages
	initialiseErrorMessages();

	// initialise and retrieve the required api functions
	if (!initialiseKERNEL32ApiFunctions()) displayErrorMessage(ERROR_WIN32API_MODULE_TOOLHELP32, true);
	if (!initialiseNTDLLApiFunctions()) displayErrorMessage(ERROR_WIN32API_MODULE_NTDLL, true);

	// initialise critical sections
	initialiseCriticalSections();

	forwardedCommandLine = (char*)ULib.HeapLibrary.allocPrivateHeap();
	currentDirectory = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* scriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int finalRecognisedArgumentElementIndex = 1;

	// default script filename
	ULib.StringLibrary.copyString(scriptFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "ppatcher.ppc");
	GetCurrentDirectory(ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, currentDirectory);
	bool alteredCurrentDirectory = false;

	bool registerFileExt = false;
	bool unregisterFileExt = false;
	bool encryptScript = false;
	bool decryptScript = false;
	bool useDefaultEncryptionKey = false;
	bool showExitMessage = true;
	bool debugSupport = false;

	// automatically register file extension (if debug mode is enabled and run from window explorer without any parameters)
	if ((argc <= 1) && (ULib.ProcessLibrary.isDebugModeEnabled()) && (ULib.ProcessLibrary.isParentProcessWindowsExplorer())) registerFileExtension(true);

	// script filename (without /executeScript option)
	if ((argc > 1) && (ULib.FileLibrary.getFileSize(argv[1]) > 0))
	{
		char* filePart = NULL;
		if (GetFullPathName(argv[1], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, scriptFilename, &filePart) == 0) ULib.StringLibrary.copyString(scriptFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, argv[1]);
		ULib.HeapLibrary.freePrivateHeap(currentDirectory);
		currentDirectory = ULib.FileLibrary.getPathFromFilename(scriptFilename);
		finalRecognisedArgumentElementIndex++;
		alteredCurrentDirectory = true;
	}

	// if arguments are passed, parse them
	if (argc > 1)
	{
		for (int i = finalRecognisedArgumentElementIndex; i < argc; i++)
		{
			// show the valid arguments
			if (ULib.StringLibrary.isStringMatch(argv[i], "/?"))
			{
				displayConsoleMessage("Usage  : ppatcher [options] [arguments to forward]");
				displayConsoleMessage("Options:");
				displayConsoleMessage("\t /executeScript <script name>\t- specifed script filename");
				displayConsoleMessage("\t /displayScriptInfo\t\t- display script information");
				displayConsoleMessage("\t /encryptScript [/defaultKey]\t- encrypt script file (using rc4)");
				if (ALLOW_SCRIPT_DECRYPTION) displayConsoleMessage("\t /decryptScript [/defaultKey]\t- decrypt script file (using rc4)");
				displayConsoleMessage("\t /reg\t\t\t\t- register file extension");
				displayConsoleMessage("\t /unreg\t\t\t\t- unregister file extension");
				if (ALLOW_DEBUG_SUPPORT) displayConsoleMessage("\t /debugSupport\t\t\t- display debug information");
				ULib.ProcessLibrary.exitProcessHandler();
			}

			// display script information (if available)
			if ((ULib.StringLibrary.isStringMatch(argv[i], "/displayInfo")) || (ULib.StringLibrary.isStringMatch(argv[i], "/displayScriptInfo")))
			{
				DISPLAY_SCRIPT_INFORMATION = true;
				finalRecognisedArgumentElementIndex = (i + 1);
			}

			// register file extension
			if ((ULib.StringLibrary.isStringMatch(argv[i], "/shell")) || (ULib.StringLibrary.isStringMatch(argv[1], "/reg", false, 4)))
			{
				registerFileExt = true;
				unregisterFileExt = false;
				finalRecognisedArgumentElementIndex = (i + 1);
			}

			// unregister file extension
			if ((ULib.StringLibrary.isStringMatch(argv[i], "/noshell")) || (ULib.StringLibrary.isStringMatch(argv[1], "/unreg", false, 6)))
			{
				registerFileExt = false;
				unregisterFileExt = true;
				finalRecognisedArgumentElementIndex = (i + 1);
			}

			// the required script file name is different from 'ppatcher.ppc' and/or it's in another directory
			if ((ULib.StringLibrary.isStringMatch(argv[i], "/config")) || (ULib.StringLibrary.isStringMatch(argv[i], "/executeScript")))
			{
				// get the script file name
				if ((i + 1) < argc)
				{
					char* filePart = NULL;
					if (GetFullPathName(argv[i + 1], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, scriptFilename, &filePart) == 0) ULib.StringLibrary.copyString(scriptFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, argv[i + 1]);
					ULib.HeapLibrary.freePrivateHeap(currentDirectory);
					currentDirectory = ULib.FileLibrary.getPathFromFilename(scriptFilename);
					finalRecognisedArgumentElementIndex = (i + 2);
					alteredCurrentDirectory = true;
				}
				else
				{
					finalRecognisedArgumentElementIndex = (i + 1);
				}
			}

			// run using image file execution options debugger option (need to replace exe with ppc)
			if (ULib.StringLibrary.isStringMatch(argv[i], "/imageDebugger"))
			{
				// get the script file name (from the process name passed from explorer)
				if ((i + 1) < argc)
				{
					char* filePart = NULL;
					char* imageFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
					if (GetFullPathName(argv[i + 1], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, imageFilename, &filePart) == 0) ULib.StringLibrary.copyString(imageFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, argv[i + 1]);
					imageFilename = ULib.StringLibrary.replaceSubString(imageFilename, ".exe", ".ppc", false);
					ULib.StringLibrary.copyString(scriptFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, imageFilename);
					ULib.HeapLibrary.freePrivateHeap(imageFilename);
					ULib.HeapLibrary.freePrivateHeap(currentDirectory);
					currentDirectory = ULib.FileLibrary.getPathFromFilename(scriptFilename);
					finalRecognisedArgumentElementIndex = (i + 2);
					alteredCurrentDirectory = true;
					showExitMessage = false;
					imageDebuggerRunning = true;
				}
				else
				{
					finalRecognisedArgumentElementIndex = (i + 1);
				}
			}

			// encrypt the script file using RC4
			if ((ULib.StringLibrary.isStringMatch(argv[i], "/encrypt")) || (ULib.StringLibrary.isStringMatch(argv[i], "/encryptScript")))
			{
				encryptScript = true;
				finalRecognisedArgumentElementIndex = (i + 1);
			}

			// decrypt the script file using RC4 (if allowed)
			if (ALLOW_SCRIPT_DECRYPTION)
			{
				if ((ULib.StringLibrary.isStringMatch(argv[i], "/decrypt")) || (ULib.StringLibrary.isStringMatch(argv[i], "/decryptScript")))
				{
					decryptScript = true;
					finalRecognisedArgumentElementIndex = (i + 1);
				}
			}

			// debug support
			if (ALLOW_DEBUG_SUPPORT)
			{
				if (ULib.StringLibrary.isStringMatch(argv[i], "/debugSupport")) debugSupport = true;
			}

			// use the default key in the encryption and decryption routines
			if ((ULib.StringLibrary.isStringMatch(argv[i], "/default")) || (ULib.StringLibrary.isStringMatch(argv[i], "/defaultKey")))
			{
				useDefaultEncryptionKey = true;
				finalRecognisedArgumentElementIndex = (i + 1);
			}
		}

		// forward unrecognised arguments to the target process
		for (; finalRecognisedArgumentElementIndex < argc; finalRecognisedArgumentElementIndex++)
		{
			ULib.StringLibrary.appendString(forwardedCommandLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, argv[finalRecognisedArgumentElementIndex]);
			if (finalRecognisedArgumentElementIndex < (argc - 1)) ULib.StringLibrary.appendString(forwardedCommandLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, " ");
		}
	}

	// convert the short path script file name to the long path equivalent
	char* scriptFilenameLongPath = ULib.FileLibrary.convertShortPathToLongPath(scriptFilename);

	// alter the base directory to the same directory that the script file is in
	if (alteredCurrentDirectory) SetCurrentDirectory(currentDirectory);

	// create file extension
	if (registerFileExt)
	{
		registerFileExtension();
	}
	// remove file extension
	else if (unregisterFileExt)
	{
		unregisterFileExtension();
	}
	// display script information
	else if (isDisplayScriptInformationEnabled())
	{
		parseScriptFile(scriptFilenameLongPath);
		displayScriptInformation();
		cleanupParsedEntries();
	}
	// encrypt the script file
	else if (encryptScript)
	{
		encryptScriptFile(scriptFilenameLongPath, useDefaultEncryptionKey);
	}
	// decrypt the script file (if allowed)
	else if ((ALLOW_SCRIPT_DECRYPTION) && (decryptScript))
	{
		decryptScriptFile(scriptFilenameLongPath, useDefaultEncryptionKey);
	}
	else if ((ALLOW_DEBUG_SUPPORT) && (debugSupport))
	{
		displayDebugSupport();
	}
	else
	{
		// parse the script file, and if successful start the
		// patching engine, otherwise display the error message
		if (!parseScriptFile(scriptFilenameLongPath)) displayErrorMessage(ERROR_PARSING_GENERAL, getParsingErrorMessage(), true);

		// parsing engine succeeded - start the patching engine
		executePatchingEngine();
		showExitMessage = false;
	}

	// delete critical sections
	deleteCriticalSections();

	ULib.HeapLibrary.freePrivateHeap(currentDirectory);
	ULib.HeapLibrary.freePrivateHeap(scriptFilename);
	ULib.HeapLibrary.freePrivateHeap(scriptFilenameLongPath);
	ULib.HeapLibrary.freePrivateHeap(forwardedCommandLine);

	shutdownProcessPatcher(showExitMessage);
}