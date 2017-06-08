///////////////////////////////////////////////
// File Patcher                              //
// (C)thewd, thewd@hotmail.com               //
///////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Unable to open or read the target file",
	"Error occurred reading bytes from the file",
	"Error patching the file - file size is incorrect",
	"Unable to create a backup copy of the original file",
	"Error occurred while patching the file - expected bytes don't match",
	"Error occurred while applying the patch to the target file",
	"Error occurred while patching the file - already fully patched",
	"Error occurred while parsing the specified script file",
	"Error occurred parsing the script - Unable to open script file",
	"Error occurred parsing the script - Unsupported file format",
	"Error occurred parsing the script - Unrecognised Filename parameter",
	"Error occurred parsing the script - Unrecognised Filesize parameter",
	"Error occurred parsing the script - No Address parameters found"
};

CUtilitiesLibrary ULib;

//////////////////////////////////////////////////
// returns the parsed parameter value, if found //
//////////////////////////////////////////////////
char* getParsedParameter(char* parameterName, int parameterIndex)
{
	char* parameterValue = (char*)ULib.HeapLibrary.allocPrivateHeap();
	bool parsedParameterResult = ULib.ParsingEngine.getParsedParameter(parameterName, parameterValue, parameterIndex);
	if (ULib.StringLibrary.getStringLength(parameterValue) <= 0) parsedParameterResult = false;
	if (parsedParameterResult) return parameterValue;
	ULib.HeapLibrary.freePrivateHeap(parameterValue);
	return NULL;
}

//////////////////////////////////////////////////
// returns the parsed parameter value, if found //
//////////////////////////////////////////////////
char* getParsedParameter(char* parameterName)
{
	return getParsedParameter(parameterName, 0);
}

//////////////////////////////////////////////////////////
// returns patching information from the address string //
//////////////////////////////////////////////////////////
bool getPatchingInformation(char* addressString, unsigned long fileSize,
							unsigned long* patchAddress, unsigned char* originalByte, unsigned char* newByte)
{
	*patchAddress = 0;
	*originalByte = 0;
	*newByte = 0;

	if (!ULib.StringLibrary.isString(addressString, 1)) return false;

	// get string elements
	int addressInformationCount = 0;
	char* addressInformation[4];
	ULib.StringLibrary.getStringElementsFromString(addressString, addressInformation, &addressInformationCount, 4, ':');
	if (addressInformationCount != 3)
	{
		for (int i = 0; i < addressInformationCount; i++) ULib.HeapLibrary.freePrivateHeap(addressInformation[i]);
		return false;
	}

	// parse information
	*patchAddress = ULib.StringLibrary.getDWORDFromString(addressInformation[0]);
	*originalByte = ULib.StringLibrary.getBYTEFromString(addressInformation[1]);
	*newByte = ULib.StringLibrary.getBYTEFromString(addressInformation[2]);
	for (int i = 0; i < addressInformationCount; i++) ULib.HeapLibrary.freePrivateHeap(addressInformation[i]);

	if (*patchAddress >= fileSize) return false;
	else return true;
}

////////////////////////////////////////////////////////////
// searches for the original bytes and patches in the     //
// new bytes. If the last byte is already patched then it //
// is assumed that this file is already patched. Can undo //
// this patch to restore the file to its original state   //
////////////////////////////////////////////////////////////
void patchFile(char* fileName, unsigned long fileSize, int numberOfAddresses)
{
	unsigned long diskFileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &diskFileSize);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	if (fileSize != diskFileSize) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	unsigned long patchAddress = 0;
	unsigned char originalByte = 0;
	unsigned char newByte = 0;

	char* addressParameter = getParsedParameter("Address", numberOfAddresses - 1);
	if (!getPatchingInformation(addressParameter, fileSize, &patchAddress, &originalByte, &newByte)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	ULib.HeapLibrary.freePrivateHeap(addressParameter);

	// check if already patched (last address)
	if (fileBuffer[patchAddress] == newByte)
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6, false);

		// remove patch?
		ULib.ConsoleLibrary.displayConsoleMessage("\r\nRemove Patch (Y/N)? ", false);
		bool removePatch = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		ULib.ConsoleLibrary.displayConsoleMessage(((removePatch) ? "Yes" : "No"));

		if (removePatch)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");

			// patch file to original state
			for (int i = 0; i < numberOfAddresses; i++)
			{
				addressParameter = getParsedParameter("Address", i);
				if (!getPatchingInformation(addressParameter, fileSize, &patchAddress, &originalByte, &newByte)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
				ULib.HeapLibrary.freePrivateHeap(addressParameter);

				// already original byte
				if (fileBuffer[patchAddress] == originalByte) continue;
				else if (fileBuffer[patchAddress] == newByte) fileBuffer[patchAddress] = originalByte;
				else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
			}

			if (!ULib.FileLibrary.writeBufferToFile(fileName, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);
			ULib.ConsoleLibrary.displayConsoleMessage("The target file was successfully restored!");
		}
	}
	// check for expected byte at last address
	else if (fileBuffer[patchAddress] == originalByte)
	{
		// patch file?
		ULib.ConsoleLibrary.displayConsoleMessage("Apply Patch (Y/N)? ", false);
		bool applyPatch = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		ULib.ConsoleLibrary.displayConsoleMessage(((applyPatch) ? "Yes" : "No"));

		if (applyPatch)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");

			// patch file
			for (int i = 0; i < numberOfAddresses; i++)
			{
				addressParameter = getParsedParameter("Address", i);
				if (!getPatchingInformation(addressParameter, fileSize, &patchAddress, &originalByte, &newByte)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
				ULib.HeapLibrary.freePrivateHeap(addressParameter);

				// already patched
				if (fileBuffer[patchAddress] == newByte) continue;
				else if (fileBuffer[patchAddress] == originalByte) fileBuffer[patchAddress] = newByte;
				else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
			}

			// create backup file
			char* backupFilename = ULib.FileLibrary.getBackupFilename(fileName);
			if (CopyFile(fileName, backupFilename, TRUE) == 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
			ULib.HeapLibrary.freePrivateHeap(backupFilename);

			if (!ULib.FileLibrary.writeBufferToFile(fileName, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);
			ULib.ConsoleLibrary.displayConsoleMessage("The target file was successfully patched!");
		}
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

// register the .fpc shell extension
void registerShellExtension(bool ignoreConsoleOutput)
{
	ULib.RegistryLibrary.unregisterFileExtension("fpc", 2);
	char* patcherModuleFilename = ULib.FileLibrary.getModuleFilename(NULL);
	char* defaultIconString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* shellCommandsString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(defaultIconString, "%s,1", patcherModuleFilename);
	wsprintf(shellCommandsString, "Execute File Patching Script|%s /executeScript \"%%1\",Edit with Notepad|notepad.exe \"%%1\"", patcherModuleFilename);
	if ((!ULib.RegistryLibrary.registerFileExtension("fpc", "File Patcher Script File", defaultIconString, shellCommandsString)) && (!ignoreConsoleOutput)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 7);
	else if (!ignoreConsoleOutput) ULib.ConsoleLibrary.displayConsoleMessage("Successfully registered the file extension .fpc");
	ULib.HeapLibrary.freePrivateHeap(patcherModuleFilename);
	ULib.HeapLibrary.freePrivateHeap(defaultIconString);
	ULib.HeapLibrary.freePrivateHeap(shellCommandsString);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("File Patcher", "2.01", 1999);
	bool displayUsage = true;
	bool useScriptFilename = false;
	char* scriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// use open dialog to select script filename
	if ((argc <= 1) && (ULib.ProcessLibrary.isParentProcessWindowsExplorer()))
	{
		// automatically register file extension (if debug mode is enabled and run from window explorer without any parameters)
		if (ULib.ProcessLibrary.isDebugModeEnabled()) registerShellExtension(true);
		if (ULib.FileLibrary.getOpenFileName("Select Script File", "Script Files (*.fpc)\0*.fpc\0\0", "fpc", NULL, scriptFilename)) useScriptFilename = true;
	}
	// script filename (without /executeScript option)
	else if ((argc > 1) && (ULib.FileLibrary.getFileSize(argv[1]) > 0))
	{
		ULib.StringLibrary.copyString(scriptFilename, argv[1]);
		useScriptFilename = true;
	}

	// if arguments are passed
	if (((argc > 2) && (ULib.StringLibrary.isStringMatch("/executeScript", argv[1]))) || (useScriptFilename))
	{
		// initialise parsing engine
		if (!useScriptFilename) ULib.StringLibrary.copyString(scriptFilename, argv[2]);
		ULib.ParsingEngine.initialiseParsingEngine(scriptFilename, "#File Patcher Script", "ScriptName,ScriptDescription,Filename,Filesize,Address");
		// parse the script, handling any errors that occur
		if (!ULib.ParsingEngine.parseFile())
		{
			int lastErrorNumber = ULib.ParsingEngine.getLastErrorNumber();
			if (lastErrorNumber == ULib.ParsingEngine.ERROR_PARSINGENGINE_FILENOTFOUND) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 8);
			else if (lastErrorNumber == ULib.ParsingEngine.ERROR_PARSINGENGINE_UNSUPPORTEDFILEFORMAT) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 9);
			else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 7);
		}

		// display script name (or filename)
		ULib.ConsoleLibrary.displayConsoleMessage("Executing Script   : ", false);
		char* scriptNameParameter = getParsedParameter("ScriptName");
		char* scriptDescriptionParameter = getParsedParameter("ScriptDescription");
		if (scriptNameParameter != NULL)
		{
			ULib.ConsoleLibrary.displayConsoleMessage(scriptNameParameter);
			ULib.HeapLibrary.freePrivateHeap(scriptNameParameter);
		}
		else
		{
			char* fileNameLong = ULib.FileLibrary.convertShortPathToLongPath(scriptFilename);
			char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(fileNameLong, true);
			ULib.ConsoleLibrary.displayConsoleMessage(fileNameStripped);
			ULib.HeapLibrary.freePrivateHeap(fileNameStripped);
			ULib.HeapLibrary.freePrivateHeap(fileNameLong);
		}

		// display script description
		if (scriptDescriptionParameter != NULL)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Script Description : ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(scriptDescriptionParameter);
			ULib.HeapLibrary.freePrivateHeap(scriptDescriptionParameter);
		}

		ULib.ConsoleLibrary.displayConsoleMessage("");

		// get required parameters
		char* fileNameParameter = getParsedParameter("Filename");
		char* fileSizeParameterString = getParsedParameter("Filesize");
		unsigned long fileSizeParameter = ULib.StringLibrary.getDWORDFromString(fileSizeParameterString);
		int numberOfAddressesParameter = ULib.ParsingEngine.getNumberOfParameters("Address");
		if (fileNameParameter == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 10);
		if (fileSizeParameter <= 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 11);
		if ((numberOfAddressesParameter <= 0) || (numberOfAddressesParameter > 100)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 12);

		// set the current directory to directory that contains the script
		char* fileNamePath = ULib.FileLibrary.getPathFromFilename(scriptFilename);
		if (ULib.StringLibrary.isString(fileNamePath, 1)) SetCurrentDirectory(fileNamePath);
		ULib.HeapLibrary.freePrivateHeap(fileNamePath);

		// patch file
		patchFile(fileNameParameter, fileSizeParameter, numberOfAddressesParameter);

		ULib.HeapLibrary.freePrivateHeap(fileSizeParameterString);
		ULib.HeapLibrary.freePrivateHeap(fileNameParameter);

		// destroy parsing engine
		ULib.ParsingEngine.destroyParsingEngine();
		displayUsage = false;
	}
	else if (argc > 1)
	{
		// register file extension
		if (ULib.StringLibrary.isStringMatch("/reg", argv[1], false, 4))
		{
			registerShellExtension(false);
			displayUsage = false;
		}
		// unregister file extension
		else if (ULib.StringLibrary.isStringMatch("/unreg", argv[1], false, 6))
		{
			if (!ULib.RegistryLibrary.unregisterFileExtension("fpc", 2)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 8);
			else ULib.ConsoleLibrary.displayConsoleMessage("Successfully unregistered the file extension .fpc");
			displayUsage = false;
		}
	}

	// display usage information
	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : patcher [options]");
		ULib.ConsoleLibrary.displayConsoleMessage("Options:");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /executeScript <script filename>\t- execute specified script file");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /registerExtension\t\t\t- register extension .fpc");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /unregisterExtension\t\t\t- unregister extension .fpc");
	}

	ULib.HeapLibrary.freePrivateHeap(scriptFilename);
	ULib.ProcessLibrary.exitProcessHandler();
}