//////////////////////////////////////////////////////////////////////////////////////
// Display a message if an error occurred while executing. Terminates if required   //
//////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "apiWin32.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

char* errorMessages[MAXIMUM_ERROR_MESSAGES];
bool ppShutingDown = false;

// initialise the error messages
void initialiseErrorMessages(void)
{
	// script file parsing
	errorMessages[ERROR_PARSING_SCRIPT_NOTFOUND] = "Unable to open or read the default/specified script file";
	errorMessages[ERROR_PARSING_GENERAL] = "Error occurred while parsing the script file, see documentation";
	errorMessages[ERROR_PARSING_SCRIPT_INVALID] = "Appears to be an invalid or unsupported script file";
	errorMessages[ERROR_PARSING_SCRIPT_UNSUPPORTED] = "This script file format is no longer supported";
	errorMessages[ERROR_PARSING_OS_UNSUPPORTED] = "The script file is not supported on this operating system";
	errorMessages[ERROR_PARSING_BLOCKEXECUTION] = "The script file execution has been blocked";

	// encryption support
	errorMessages[ERROR_SCRIPT_ENCRYPTION_VALIDKEY] = "You must supply a valid key for encryption to succeed";
	errorMessages[ERROR_SCRIPT_ENCRYPTION_VALIDLENGTH] = "Encryption key must be between 1 and 16 characters";
	errorMessages[ERROR_SCRIPT_ENCRYPTION_GENERAL] = "Error occurred while encrypting the script file";
	errorMessages[ERROR_SCRIPT_ENCRYPTION_BACKUP] = "Error occurred while creating a backup of the script file";
	errorMessages[ERROR_SCRIPT_ENCRYPTION_ALREADYENCRYPTED] = "The script file appears to be already encrypted";

	// decryption support
	errorMessages[ERROR_SCRIPT_DECRYPTION_VALIDKEY] = "You must supply a valid key for decryption to succeed";
	errorMessages[ERROR_SCRIPT_DECRYPTION_VALIDLENGTH] = "Decryption key must be between 1 and 16 characters";
	errorMessages[ERROR_SCRIPT_DECRYPTION_GENERAL] = "Error occurred while decrypting the script file";
	errorMessages[ERROR_SCRIPT_DECRYPTION_NOTENCRYPTED] = "The script file appears not to be currently encrypted";

	// critical exception handling
	errorMessages[ERROR_CRITICALEXCEPTION_ENGINE] = "A Critical Exception occurred while executing the patching engine";
	errorMessages[ERROR_CRITICALEXCEPTION_PROCESS] = "A Critical Exception occurred while patching the target process";
	errorMessages[ERROR_CRITICALEXCEPTION_MODULE] = "A Critical Exception occurred during the patching of a module";
	errorMessages[ERROR_CRITICALEXCEPTION_PLUGIN] = "A Critical Exception occurred during the execution of a plugin";
	errorMessages[ERROR_CRITICALEXCEPTION_REGISTRY] = "A Critical Exception occurred during the registry patching process";

	// patching engine error handling
	errorMessages[ERROR_PATCHING_ENGINE_PROCESSTERMINATED] = "Unable to finish patching engine (process terminated unexpectedly)";
	errorMessages[ERROR_PATCHING_ENGINE_TERMINATED] = "Unable to fully complete the process patcher patching engine";
	errorMessages[ERROR_PATCHING_ENGINE_WAITINGEVENTS] = "Error occurred accessing the patching engine waiting events";

	// target process patching
	errorMessages[ERROR_PATCHING_TARGETPROCESS_GENERAL] = "Unable to find or launch the target application process";
	errorMessages[ERROR_PATCHING_TARGETPROCESS_FILESIZE] = "The specified target process has an incorrect file size";
	errorMessages[ERROR_PATCHING_TARGETPROCESS_MEMORYACCESS] = "Error occurred while trying to access the process memory";
	errorMessages[ERROR_PATCHING_TARGETPROCESS_RECURSIVE] = "Unable to launch the target process - possible recursive patching";

	// child process patching
	errorMessages[ERROR_PATCHING_CHILDPROCESS_DETERMINEPROCESSES] = "Error occurred determining what processes are executing";
	errorMessages[ERROR_PATCHING_CHILDPROCESS_FINDPROCESS] = "Unable to find a child process executing on the local system";
	errorMessages[ERROR_PATCHING_CHILDPROCESS_MEMORYACCESS] = "Error occurred while trying to access the child process memory";
	errorMessages[ERROR_PATCHING_CHILDPROCESS_FILESIZE] = "The specified child process has an incorrect file size";

	// module patching
	errorMessages[ERROR_PATCHING_MODULE_GENERAL] = "Error occurred patching a module loaded by the process";
	errorMessages[ERROR_PATCHING_MODULE_NOTFOUND] = "Unable to find the module within the context of the process";
	errorMessages[ERROR_PATCHING_MODULE_DETERMINEMODULES] = "Error occurred determining what modules are used by the process";

	// plugin architecture
	errorMessages[ERROR_PATCHING_PLUGIN_GENERAL] = "Unable to find or access a supported plugin module";
	errorMessages[ERROR_PATCHING_PLUGIN_EXECUTION] = "Error occurred during the execution of the plugin module";
	errorMessages[ERROR_PATCHING_PLUGIN_APIVERSION] = "Unable to recognise the API_PLUGIN_VERSION of the plugin module";
	errorMessages[ERROR_PATCHING_PLUGIN_APIVERSION_0100] = "Plugins that use the v1.00 API are not supported after v4.10";
	errorMessages[ERROR_PATCHING_PLUGIN_EXPORTS_GETVERSION] = "Error accessing the GetPluginVersionInformation function export";
	errorMessages[ERROR_PATCHING_PLUGIN_EXPORTS_EXECUTEPLUGIN] = "Error accessing the ExecutePlugin function export";
	errorMessages[ERROR_PATCHING_PLUGIN_EXPORTS_GETERROR] = "Error accessing the GetLastPluginError function export";

	// registry patching
	errorMessages[ERROR_PATCHING_REGISTRY_GENERAL] = "Error occurred while patching a registry entry";
	errorMessages[ERROR_PATCHING_REGISTRY_CREATINGKEY] = "Error occurred while creating the specified registry key";
	errorMessages[ERROR_PATCHING_REGISTRY_SETTINGVALUE] = "Error occurred while setting the registry value";

	// accessing required System API functions
	errorMessages[ERROR_WIN32API_MODULE_TOOLHELP32] = "Error occurred initialising Toolhelp32 API (kernel32.dll)";
	errorMessages[ERROR_WIN32API_MODULE_NTDLL] = "Error occurred initialising Windows NT Layer API (ntdll.dll)";

	// registering/unregistering shell extension
	errorMessages[ERROR_SHELLEXTENSION_REGISTER] = "Error occurred registering the file extension";
	errorMessages[ERROR_SHELLEXTENSION_UNREGISTER] = "Error occurred unregistering the file extension";

	// Blowfish algorithm support
	errorMessages[ERROR_GENERAL_BLOWFISHENCRYPTION] = "The Blowfish algorithm is no longer supported after v2.50";

	// Custom Error Message
	errorMessages[ERROR_CUSTOM_MESSAGE] = "";
}

// function executed just before the exit process handler completes
void exitProcessCallbackFunction(void)
{
	if (isDebugInformationEnabled()) resetConsoleScreenBufferSize();
}

// checks to see if this process patcher is waiting to shutdown
bool isProcessPatcherShutingDown(void)
{
	return ppShutingDown;
}

// shutdown and terminate the process patcher
void shutdownProcessPatcher(bool showExitMessage)
{
	// already shuting down
	if (isProcessPatcherShutingDown()) return;
	ppShutingDown = true;
	terminatePatchingEngine();

	// don't show exit message if window is hidden but show window if required
	if (isProcessPatcherWindowHidden())
	{
		showProcessPatcherWindow();
		if (showExitMessage) showExitMessage = false;
	}

	// display exit message
	ULib.ProcessLibrary.exitProcessHandler(NULL, ((isDebugInformationEnabled()) ? ULib.ProcessLibrary.EXITPROCESS_DISPLAYMESSAGE_ALWAYS : ((showExitMessage) ? ULib.ProcessLibrary.EXITPROCESS_DEFAULT : ULib.ProcessLibrary.EXITPROCESS_DISPLAYMESSAGE_NEVER)), (CUtilitiesLibrary::CProcessLibrary::EXITPROCESS_CALLBACK_FUNCTION*)exitProcessCallbackFunction);
}

// returns an error message corresponding to the supplied error number
char* getErrorMessage(int errorNumber)
{
	return ((errorNumber < MAXIMUM_ERROR_MESSAGES) ? errorMessages[errorNumber] : NULL);
}

// display the error message and terminate the application if required
void displayErrorMessage(int errorNumber, char* errorMessage, char* additionalErrorMessage, bool exitProgram)
{
	__try
	{
		if (getDisplayErrorMessageLock() != NULL) EnterCriticalSection(getDisplayErrorMessageLock());

		// terminate the quit message thread
		if (exitProgram) terminateQuitMessageThread();

		// restore the console window to display the error message
		if (IsIconic(ULib.ConsoleLibrary.getConsoleWindowHandle())) ShowWindowAsync(ULib.ConsoleLibrary.getConsoleWindowHandle(), SW_RESTORE);

		// maximum errorMessage length should be 68 characters
		char* consoleErrorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		if (additionalErrorMessage != NULL) sprintf_s(consoleErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%sError #%02d: %s\r\n           (%s)", ((hasPatchingEngineStarted()) ? "\r\n" : ""), errorNumber, errorMessage, additionalErrorMessage);
		else sprintf_s(consoleErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%sError #%02d: %s", ((hasPatchingEngineStarted()) ? "\r\n" : ""), errorNumber, errorMessage);
		displayConsoleMessage(consoleErrorMessage);
		ULib.HeapLibrary.freePrivateHeap(consoleErrorMessage);
		MessageBeep(MB_ICONHAND);

		// terminate this process patcher and perform clean-up operations
		if (exitProgram) shutdownProcessPatcher(true);
	}
	__finally
	{
		if (getDisplayErrorMessageLock() != NULL) LeaveCriticalSection(getDisplayErrorMessageLock());
	}
}

// display the error message and terminate the application if required
void displayErrorMessage(int errorNumber, bool exitProgram)
{
	displayErrorMessage(errorNumber, getErrorMessage(errorNumber), NULL, exitProgram);
}

// display the error message and terminate the application if required
void displayErrorMessage(int errorNumber, char* additionalErrorMessage, bool exitProgram)
{
	displayErrorMessage(errorNumber, getErrorMessage(errorNumber), additionalErrorMessage, exitProgram);
}

// display the error message and terminate the application if required
void displayCustomErrorMessage(char* errorMessage, bool exitProgram)
{
	displayErrorMessage(ERROR_CUSTOM_MESSAGE, errorMessage, NULL, exitProgram);
}

// handles a critical exception that was generated through the execution of the patching threads
int handleCriticalException(EXCEPTION_POINTERS* exceptionInformation, int executionThreadType)
{
	char* exceptionErrorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// display information about common exceptions
	switch (exceptionInformation->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
		{
			if ((exceptionInformation->ExceptionRecord->ExceptionInformation[1] > 0) && ((exceptionInformation->ExceptionRecord->ExceptionInformation[0] == 0) || (exceptionInformation->ExceptionRecord->ExceptionInformation[0] == 1))) sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s Access Violation - 0x%X", ((exceptionInformation->ExceptionRecord->ExceptionInformation[0] == 0) ? "Read" : "Write"), exceptionInformation->ExceptionRecord->ExceptionInformation[1]);
			else if ((exceptionInformation->ExceptionRecord->ExceptionInformation[1] > 0) && (exceptionInformation->ExceptionRecord->ExceptionInformation[0] == 8)) sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "NX Access Violation - 0x%X", exceptionInformation->ExceptionRecord->ExceptionInformation[1]);
			else sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Access Violation - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Array Bounds Exceeded - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_BREAKPOINT:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Breakpoint Encountered - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Datatype Misalignment - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Divide By Zero - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_INT_OVERFLOW:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Buffer Overflow - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Floating Point Exception - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_GUARD_PAGE:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Guarded Page - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Illegal Instruction - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_INVALID_HANDLE:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Invalid Handle - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_IN_PAGE_ERROR:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "In Page Error - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Non-Continuable Exception - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_PRIV_INSTRUCTION:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Privileged Instruction - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_SINGLE_STEP:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Single Step - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
		case EXCEPTION_STACK_OVERFLOW:
		{
			sprintf_s(exceptionErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Stack Overflow - 0x%X", PtrToUlong(exceptionInformation->ExceptionRecord->ExceptionAddress));
			break;
		}
	}

	// display appropriate exception error message
	if ((executionThreadType == SECTIONTYPE_TARGETPROCESS) || (executionThreadType == SECTIONTYPE_CHILDPROCESS)) displayErrorMessage(ERROR_CRITICALEXCEPTION_PROCESS, ((ULib.StringLibrary.getStringLength(exceptionErrorMessage) > 0) ? exceptionErrorMessage : NULL), true);
	else if (executionThreadType == SECTIONTYPE_MODULE) displayErrorMessage(ERROR_CRITICALEXCEPTION_MODULE, ((ULib.StringLibrary.getStringLength(exceptionErrorMessage) > 0) ? exceptionErrorMessage : NULL), true);
	else if (executionThreadType == SECTIONTYPE_PLUGIN) displayErrorMessage(ERROR_CRITICALEXCEPTION_PLUGIN, ((ULib.StringLibrary.getStringLength(exceptionErrorMessage) > 0) ? exceptionErrorMessage : NULL), true);
	else if (executionThreadType == SECTIONTYPE_REGISTRYENTRY) displayErrorMessage(ERROR_CRITICALEXCEPTION_REGISTRY, ((ULib.StringLibrary.getStringLength(exceptionErrorMessage) > 0) ? exceptionErrorMessage : NULL), true);
	else displayErrorMessage(ERROR_CRITICALEXCEPTION_ENGINE, ((ULib.StringLibrary.getStringLength(exceptionErrorMessage) > 0) ? exceptionErrorMessage : NULL), true);
	return EXCEPTION_EXECUTE_HANDLER;
}