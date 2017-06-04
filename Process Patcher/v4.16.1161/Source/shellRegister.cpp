//////////////////////////////////////////////////////////////
// Registers/Unregisters the process patcher file extension //
//////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"
#include "shellregister.h"

// registers the shell file extension
void registerFileExtension(bool ignoreConsoleDisplay)
{
	// remove existing registry entries
	unregisterFileExtension(true);

	char* ppatcherModuleFilename = ULib.FileLibrary.getModuleFilename(NULL);
	char* defaultIconString = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 10);
	char* shellCommandsString = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 10);
	sprintf_s(defaultIconString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 10, "%s,1", ppatcherModuleFilename);
	sprintf_s(shellCommandsString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 10, "Execute Script|%s /executeScript \"%%1\",Execute Script (As Administrator)|%s /executeScript \"%%1\",Edit Script with Notepad|notepad.exe \"%%1\",Display Script Information|%s /executeScript \"%%1\" /displayScriptInfo,Encrypt Script|%s /executeScript \"%%1\" /encryptScript,Encrypt Script (Default Key)|%s /executeScript \"%%1\" /encryptScript /defaultKey", ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename);
	if (ALLOW_SCRIPT_DECRYPTION) sprintf_s(shellCommandsString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE * 10, "Execute Script|%s /executeScript \"%%1\",Execute Script (As Administrator)|%s /executeScript \"%%1\",Edit Script with Notepad|notepad.exe \"%%1\",Display Script Information|%s /executeScript \"%%1\" /displayScriptInfo,Encrypt Script|%s /executeScript \"%%1\" /encryptScript,Encrypt Script (Default Key)|%s /executeScript \"%%1\" /encryptScript /defaultKey,Decrypt Script|%s /executeScript \"%%1\" /decryptScript,Decrypt Script (Default Key)|%s /executeScript \"%%1\" /decryptScript /defaultKey", ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename, ppatcherModuleFilename);
	if (!ignoreConsoleDisplay) displayConsoleMessage("[Register Extension]");
	if (!ULib.RegistryLibrary.registerFileExtension(ppShellExtension, "Process Patcher Script File", defaultIconString, shellCommandsString, true)) displayErrorMessage(ERROR_SHELLEXTENSION_REGISTER, true);
	if (!ignoreConsoleDisplay) displayConsoleMessage("  Successfully registered the file extension");
	ULib.HeapLibrary.freePrivateHeap(ppatcherModuleFilename);
	ULib.HeapLibrary.freePrivateHeap(defaultIconString);
	ULib.HeapLibrary.freePrivateHeap(shellCommandsString);
}

// unregisters the shell file extension
void unregisterFileExtension(bool ignoreConsoleDisplay)
{
	bool unregisterFileExtensionResult = ULib.RegistryLibrary.unregisterFileExtension(ppShellExtension, (((ALLOW_SCRIPT_DECRYPTION) || (ignoreConsoleDisplay)) ? 7 : 5));

	// don't display any output
	if (!ignoreConsoleDisplay)
	{
		displayConsoleMessage("[Unregister Extension]");
		if (!unregisterFileExtensionResult) displayErrorMessage(ERROR_SHELLEXTENSION_UNREGISTER, true);
		else displayConsoleMessage("  Successfully unregistered the file extension");
	}
}