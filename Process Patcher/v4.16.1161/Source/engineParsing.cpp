/////////////////////////////////////////////////////////////////////
// Parses the script file and stores the information in the        //
// corresponding PATCHINGDETAILS structures. Decrypts if necessary //
/////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "encryptionSupport.h"
#include "engineParsing.h"
#include "engineParsingBase.h"
#include "engineParsingChildProcess.h"
#include "engineParsingFileBase.h"
#include "engineParsingGlobal.h"
#include "engineParsingMemory.h"
#include "engineParsingModule.h"
#include "engineParsingPlugin.h"
#include "engineParsingRegistryEntry.h"
#include "engineParsingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

PATCHINGDETAILS_GLOBAL* pGlobalDetails;
PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails[MAXIMUM_TARGETPROCESSES];
PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails[MAXIMUM_CHILDPROCESSES];
PATCHINGDETAILS_MODULE* pModuleDetails[MAXIMUM_MODULES];
PATCHINGDETAILS_PLUGIN* pPluginDetails[MAXIMUM_PLUGINS];
PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails[MAXIMUM_REGISTRYENTRIES];

bool scriptIsEncrypted = false;
char* parsingErrorMessage = NULL;

// returns whether the script file is encrypted
bool isScriptEncrypted(void)
{
	return scriptIsEncrypted;
}

// returns the PATCHINGDETAILS_GLOBAL structure
PATCHINGDETAILS_GLOBAL* getGlobalPatchingDetails(void)
{
	return pGlobalDetails;
}

// returns the PATCHINGDETAILS_TARGETPROCESS structure
PATCHINGDETAILS_TARGETPROCESS* getTargetProcessPatchingDetails(void)
{
	return ((pGlobalDetails->numberOfTargetProcesses > 0) ? pTargetProcessDetails[0] : NULL);
}

// returns the PATCHINGDETAILS_CHILDPROCESS structure
PATCHINGDETAILS_CHILDPROCESS* getChildProcessPatchingDetails(void)
{
	return ((pGlobalDetails->numberOfChildProcesses > 0) ? pChildProcessDetails[0] : NULL);
}

// returns the PATCHINGDETAILS_MODULE structure for the corresponding index
PATCHINGDETAILS_MODULE* getModulePatchingDetails(int indexNumber)
{
	return (((indexNumber >= MAXIMUM_MODULES) || (indexNumber >= pGlobalDetails->numberOfModules)) ? NULL : pModuleDetails[indexNumber]);
}

// returns the PATCHINGDETAILS_PLUGIN structure for the corresponding index
PATCHINGDETAILS_PLUGIN* getPluginPatchingDetails(int indexNumber)
{
	return (((indexNumber >= MAXIMUM_PLUGINS) || (indexNumber >= pGlobalDetails->numberOfPlugins)) ? NULL : pPluginDetails[indexNumber]);
}

// returns the PATCHINGDETAILS_REGISTRYENTRY structure for the corresponding index
PATCHINGDETAILS_REGISTRYENTRY* getRegistryEntryPatchingDetails(int indexNumber)
{
	return (((indexNumber >= MAXIMUM_REGISTRYENTRIES) || (indexNumber >= pGlobalDetails->numberOfRegistryEntries)) ? NULL : pRegistryEntryDetails[indexNumber]);
}

// returns the custom error message giving further details about the parsing error
char* getParsingErrorMessage(void)
{
	return parsingErrorMessage;
}

// set the custom parsing error message
bool setParsingErrorMessage(char* errorMessage)
{
	if (parsingErrorMessage == NULL) parsingErrorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.copyString(parsingErrorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, errorMessage);
	return false;
}

// used to parse the current buffer string for a supported parameter (STRING)
int parseStringParameter(char* parameterName, char* parameterTargetString,
						 char* targetString, char* stringBuffer, bool allowBlankValues)
{
	if ((parameterName == NULL) || (stringBuffer == NULL) || (targetString == NULL)) return PARSINGENGINE_PARSINGERROR;

	// add '=' to the end of the parameter
	char* expectedParameterName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.copyString(expectedParameterName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parameterName);
	ULib.StringLibrary.appendString(expectedParameterName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "=");

	// check for a matching parameter
	if (ULib.StringLibrary.isStringMatch(stringBuffer, expectedParameterName, false, ULib.StringLibrary.getStringLength(expectedParameterName)))
	{
		ULib.HeapLibrary.freePrivateHeap(expectedParameterName);
		stringBuffer = ULib.StringLibrary.getNextTokenString(stringBuffer, '=');
		// unable to find the value
		if ((stringBuffer == NULL) && (!allowBlankValues))
		{
			char* errorText = (char*)ULib.HeapLibrary.allocPrivateHeap();
			sprintf_s(errorText, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to retrieve the value of the %s parameter", parameterName);
			setParsingErrorMessage(errorText);
			ULib.HeapLibrary.freePrivateHeap(errorText);
			return PARSINGENGINE_PARSINGERROR;
		}

		// already stored information
		if (parameterTargetString != NULL)
		{
			char* errorText = (char*)ULib.HeapLibrary.allocPrivateHeap();
			sprintf_s(errorText, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "The value of the %s parameter has already been set", parameterName);
			setParsingErrorMessage(errorText);
			ULib.HeapLibrary.freePrivateHeap(errorText);
			return PARSINGENGINE_PARSINGERROR;
		}

		ULib.StringLibrary.copyString(targetString, 0, stringBuffer);
		return PARSINGENGINE_PARSINGOK;
	}

	ULib.HeapLibrary.freePrivateHeap(expectedParameterName);
	return PARSINGENGINE_PARSINGNOTFOUND;
}

// used to parse the current buffer string for a supported parameter (UNSIGNED LONG)
int parseStringParameter(char* parameterName, unsigned long parameterTargetLong,
						 char* targetString, char* stringBuffer)
{
	if (parameterTargetLong <= 0) return parseStringParameter(parameterName, (char*)NULL, targetString, stringBuffer);
	else return parseStringParameter(parameterName, (char*)"", targetString, stringBuffer);
}

// use the file open dialog box so the user can specify the script file
bool getFilenameUsingExplorer(char* titleString, char* filterString, char* extensionString, char* selectedFilename)
{
	return ULib.FileLibrary.getOpenFileName(titleString, filterString, extensionString, getCurrentDirectory(), selectedFilename);
}

// retrieves the file handle to the script specified, or if
// not found, opens the file dialog to select another script
HANDLE openFileHandleToScriptFile(char* scriptFilename, char* selectedScriptFilename)
{
	// open the file handle
	HANDLE fileHandle = CreateFile(scriptFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		ULib.StringLibrary.copyString(selectedScriptFilename, 0, scriptFilename);
		return fileHandle;
	}

	// process patcher window is hidden - don't show file dialog
	if (isProcessPatcherWindowHidden())
	{
		return NULL;
	}

	// otherwise, open file dialog
#ifdef _M_X64
	getFilenameUsingExplorer("Select Script File", "Script Files (*.ppc;*.ppc64)\0*.ppc;*.ppc64\0\0", ppShellExtension, selectedScriptFilename);
#else
	getFilenameUsingExplorer("Select Script File", "Script Files (*.ppc)\0*.ppc\0\0", ppShellExtension, selectedScriptFilename);
#endif
	fileHandle = CreateFile(selectedScriptFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	return fileHandle;
}

// Reads the scripts file and parses the parameters. Decrypts, if necessary
// Supports the parsing of entries for the target process, child process,
// modules, supported plugins and registry entries (since v3.00)
//
// Can parse the following parameters...
// (*) denotes a compulsory parameters for that section
//
// [Global Parameters]
// - Version
// - SupportedPlatforms // v3.30
// - StealthMode // v3.60
// - PatchInformation // v3.60
// - PatchAuthor // v3.60
// - PatchContactInformation // v3.60
// - IgnoreSessionId // v4.15.1157
// - BlockExecution // v4.16.1161
//
// [Common Parameters]
// - IgnoreParsingErrors // v4.00
// - IsDisabled // v4.00
//
// [Target Process Parameters]
// - DisplayName (*)
// - Filename (*)
// - Filesize
// - Arguments
// - Address
// - RetryCount // v3.60
// - OpenExistingProcess // v4.12
// - CreateSuspendedProcess // v4.00 (depreciated)
// - CreatesChildProcess // v3.10
// - UserNotify // v3.10
// - UserNotify-Message // v3.70
// - WaitForWindowName // v3.90
// - WaitForGlobalWindowName // v4.10
// - WaitForTimeDelay // v4.13
//
// [Child Process Parameters]
// - Filename (*)
// - Filesize
//
// [Module Parameters]
// - Filename (*)
// - Filesize // v4.15.1159
// - Address
// - RetryCount // v3.60
//
// [Plugin Parameters] // v4.10
// - Filename (*)
// - PluginVersion
// - Arguments
//
// [Registry Parameters] // v4.00
// - Action (*)
// - Use32BitView // v4.16.1161
// - Use64BitView // v4.16.1161
//
bool parseScriptFile(char* scriptFilename)
{
	unsigned char* fileBuffer = NULL;
	unsigned int fileSize = 0;
	unsigned long actuallyRead;

	// open file specified or selected using explorer
	char* selectedScriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	HANDLE fileHandle = openFileHandleToScriptFile(scriptFilename, selectedScriptFilename);
	if (fileHandle == INVALID_HANDLE_VALUE) displayErrorMessage(ERROR_PARSING_SCRIPT_NOTFOUND, true);

	// get the file size
	fileSize = ULib.FileLibrary.getFileSize(fileHandle);
	fileBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(fileSize + 1);

	if (fileBuffer == NULL)
	{
		CloseHandle(fileHandle);
		displayErrorMessage(ERROR_PARSING_SCRIPT_NOTFOUND, true);
	}

	// read the script file (and check for encryption)
	ReadFile(fileHandle, fileBuffer, fileSize, &actuallyRead, NULL);
	scriptIsEncrypted = isFileBufferEncrypted(fileBuffer);

	// script file is encrypted
	if (scriptIsEncrypted)
	{
		// do not read encryption header
		ZeroMemory(fileBuffer, fileSize);
		fileSize -= 0x10;
		SetFilePointer(fileHandle, 0x10, NULL, FILE_BEGIN);
		ReadFile(fileHandle, fileBuffer, fileSize, &actuallyRead, NULL);

		// try defaultKey first, otherwise ask user for the correct key
		encryptFileBuffer(fileBuffer, fileSize, true, false);
		// check for successful decryption (using default key)
		if (!ULib.StringLibrary.isStringMatch((char*)fileBuffer, SCRIPTHEADER_DECID, false, ULib.StringLibrary.getStringLength(SCRIPTHEADER_DECID)))
		{
			// try decryption again, but using the user supplied key
			SetFilePointer(fileHandle, 0x10, NULL, FILE_BEGIN);
			ReadFile(fileHandle, fileBuffer, fileSize, &actuallyRead, NULL);
			encryptFileBuffer(fileBuffer, fileSize, false, false);
			displayConsoleMessage("");
			// check for successful decryption
			if (!ULib.StringLibrary.isStringMatch((char*)fileBuffer, SCRIPTHEADER_DECID, false, ULib.StringLibrary.getStringLength(SCRIPTHEADER_DECID)))
			{
				CloseHandle(fileHandle);
				displayErrorMessage(ERROR_SCRIPT_DECRYPTION_GENERAL, true);
			}
		}
	}

	CloseHandle(fileHandle);

	// details about the types of patching to do and other global information
	pGlobalDetails = (PATCHINGDETAILS_GLOBAL*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PATCHINGDETAILS_GLOBAL));
	pGlobalDetails->scriptFilename = NULL;
	pGlobalDetails->patchInformation = NULL;
	pGlobalDetails->patchAuthor = NULL;
	pGlobalDetails->patchContactInformation = NULL;
	pGlobalDetails->minimumProcessPatcherVersionRequired = 0;
	pGlobalDetails->supportedOperatingSystemPlatforms = OS_UNDEFINED;
	pGlobalDetails->enableStealthMode = false;
	pGlobalDetails->numberOfTargetProcesses = 0;
	pGlobalDetails->numberOfChildProcesses = 0;
	pGlobalDetails->numberOfModules = 0;
	pGlobalDetails->numberOfPlugins = 0;
	pGlobalDetails->numberOfRegistryEntries = 0;
	pGlobalDetails->ignoreSessionId = false;
	pGlobalDetails->blockExecution = BLOCKEXECUTION_UNDEFINED;

	// initialise the section numbers
	int totalNumberOfSections = 0;
	int currentSectionType = SECTIONTYPE_UNDEFINED;
	int currentTargetProcessSectionNumber = -1;
	int currentChildProcessSectionNumber = -1;
	int currentModuleSectionNumber = -1;
	int currentPluginSectionNumber = -1;
	int currentRegistryEntrySectionNumber = -1;

	// get the script file header
	char lineSeparator[] = "\r\n";
	// get first line of script (removing comment blocks if supported file format)
	char* fileBufferNoComments = ULib.ParsingEngine.removeCommentBlocks(fileBuffer, fileSize);
	char* stringBufferContext;
	char* stringBuffer = strtok_s(((strstr((char*)fileBuffer, "// Friendly Name") != NULL) ? (char*)fileBuffer : fileBufferNoComments), lineSeparator, &stringBufferContext);

	// is this a valid script file?
	if ((stringBuffer != NULL) && (!ULib.StringLibrary.isStringMatch(stringBuffer, SCRIPTHEADER_DECID, false, ULib.StringLibrary.getStringLength(SCRIPTHEADER_DECID)))) displayErrorMessage(ERROR_PARSING_SCRIPT_INVALID, true);

	// store script filename
	pGlobalDetails->scriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.copyString(pGlobalDetails->scriptFilename, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, selectedScriptFilename);
	ULib.HeapLibrary.freePrivateHeap(selectedScriptFilename);

	// current line
	char* currentLine = (char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.copyString(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, stringBuffer, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE - 1);

	// read all the memory addresses from the old script file format
	bool parseOldScriptMemoryAddresses = false;
	int parseOldScriptMemoryAddressesNumber = 0;

	// parse the script file
	while ((currentLine != NULL) && (currentLine[0] != NULL))
	{
		// get the next line from the script file
		stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
		if (stringBuffer == NULL) break;
		ZeroMemory(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE);
		ULib.StringLibrary.copyString(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, stringBuffer, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE - 1);
		// remove trailing whitespace
		ULib.StringLibrary.trimStringSpaces(currentLine, false, true, true);

		////////////////////////////
		// old script file format //
		////////////////////////////

		// minimum prcoess patcher version
		if (ULib.StringLibrary.isStringMatch(currentLine, "[Version]"))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			sprintf_s(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Version=%s", stringBuffer);
		}
		// display name
		else if ((ULib.StringLibrary.isStringMatch(currentLine, "[Friendly Name]")) ||
			(ULib.StringLibrary.isStringMatch(currentLine, "// Friendly Name")))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			sprintf_s(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "DisplayName=%s", stringBuffer);
		}
		// filename
		else if ((ULib.StringLibrary.isStringMatch(currentLine, "[Program Filename]")) ||
			(ULib.StringLibrary.isStringMatch(currentLine, "// Program Filename")))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			sprintf_s(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Filename=%s", stringBuffer);
		}
		// filesize
		else if (ULib.StringLibrary.isStringMatch(currentLine, "[Program Filesize]"))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			sprintf_s(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Filesize=%s", stringBuffer);
		}
		// arguments
		else if (ULib.StringLibrary.isStringMatch(currentLine, "[Program Arguments]"))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			sprintf_s(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Arguments=%s", stringBuffer);
		}
		// number of memory addresses
		else if ((ULib.StringLibrary.isStringMatch(currentLine, "[Number of Bytes]")) ||
			(ULib.StringLibrary.isStringMatch(currentLine, "// Number of Bytes", false, 18)))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			parseOldScriptMemoryAddressesNumber = ULib.StringLibrary.getDWORDFromString(stringBuffer);
		}
		// memory address start
		else if ((ULib.StringLibrary.isStringMatch(currentLine, "[Memory Addresses]")) ||
			(ULib.StringLibrary.isStringMatch(currentLine, "// Memory Addresses", false, 18)))
		{
			// get the next line from the script file
			stringBuffer = strtok_s(NULL, lineSeparator, &stringBufferContext);
			if (stringBuffer == NULL) break;
			parseOldScriptMemoryAddresses = true;
		}

		// parse memory addresses (specified in number of bytes parameter)
		if ((parseOldScriptMemoryAddresses) && (parseOldScriptMemoryAddressesNumber > 0))
		{
			sprintf_s(currentLine, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Address=%s", stringBuffer);
			parseOldScriptMemoryAddressesNumber--;
		}

		// Handles the different section types until one of the
		// below section ids are found, all previous information
		// is associated with the target process and global properties
		//
		// This changes the context of the information that
		// follows the section heading. Supports the Target Process,
		// Child Process, Modules, Plugins and Registry Entries

		// Target Process Section (default first section)
		if (totalNumberOfSections == 0)
		{
			currentTargetProcessSectionNumber++;
			pGlobalDetails->numberOfTargetProcesses++;
			totalNumberOfSections++;

			if (pGlobalDetails->numberOfTargetProcesses > MAXIMUM_TARGETPROCESSES) return setParsingErrorMessage("Maximum number of Target Processes has been exceeded");

			// initialise target process section information
			pTargetProcessDetails[currentTargetProcessSectionNumber] = (PATCHINGDETAILS_TARGETPROCESS*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PATCHINGDETAILS_TARGETPROCESS));
			pTargetProcessDetails[currentTargetProcessSectionNumber]->sectionType = SECTIONTYPE_TARGETPROCESS;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->sectionNumber = pGlobalDetails->numberOfTargetProcesses;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->displayName = NULL;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->fileName = NULL;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->fileSize = 0;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->fileSizeUpperLimit = 0;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->argumentsToForward = NULL;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->numberOfAddresses = 0;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->baseAddress = 0;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->retryCount = 0;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->openExistingProcess = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->createsChildProcess = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->createSuspendedProcess = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->displayUserNotifyBox = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->displayUserNotifyBoxMessage = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->waitForLocalWindowName = NULL;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->waitForGlobalWindowName = NULL;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->waitForTimeDelay = 0;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->ignoreParsingErrors = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->isDisabled = false;
			pTargetProcessDetails[currentTargetProcessSectionNumber]->isPatternMatchingFilename = false;

			currentSectionType = SECTIONTYPE_TARGETPROCESS;
			// don't continue othewise the first entry will be missed
			if (totalNumberOfSections > 1) continue;
		}
		// Child Process Section
		else if ((ULib.StringLibrary.isStringMatch(currentLine, "[ChildProcess", false, 13)) ||
			(ULib.StringLibrary.isStringMatch(currentLine, "[Child Process", false, 14)))
		{
			currentChildProcessSectionNumber++;
			pGlobalDetails->numberOfChildProcesses++;
			totalNumberOfSections++;

			if (pGlobalDetails->numberOfChildProcesses > MAXIMUM_CHILDPROCESSES) return setParsingErrorMessage("Maximum number of Child Processes has been exceeded");

			// initialise child process section information
			pChildProcessDetails[currentChildProcessSectionNumber] = (PATCHINGDETAILS_CHILDPROCESS*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PATCHINGDETAILS_CHILDPROCESS));
			pChildProcessDetails[currentChildProcessSectionNumber]->sectionType = SECTIONTYPE_CHILDPROCESS;
			pChildProcessDetails[currentChildProcessSectionNumber]->sectionNumber = pGlobalDetails->numberOfChildProcesses;
			pChildProcessDetails[currentChildProcessSectionNumber]->fileName = NULL;
			pChildProcessDetails[currentChildProcessSectionNumber]->fileSize = 0;
			pChildProcessDetails[currentChildProcessSectionNumber]->fileSizeUpperLimit = 0;
			pChildProcessDetails[currentChildProcessSectionNumber]->ignoreParsingErrors = false;
			pChildProcessDetails[currentChildProcessSectionNumber]->isPatternMatchingFilename = false;

			currentSectionType = SECTIONTYPE_CHILDPROCESS;
			continue;
		}
		// Module Section
		else if (ULib.StringLibrary.isStringMatch(currentLine, "[Module", false, 7))
		{
			currentModuleSectionNumber++;
			pGlobalDetails->numberOfModules++;
			totalNumberOfSections++;

			if (pGlobalDetails->numberOfModules > MAXIMUM_MODULES) return setParsingErrorMessage("Maximum number of Modules has been exceeded");

			// initialise module section information
			pModuleDetails[currentModuleSectionNumber] = (PATCHINGDETAILS_MODULE*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PATCHINGDETAILS_MODULE));
			pModuleDetails[currentModuleSectionNumber]->sectionType = SECTIONTYPE_MODULE;
			pModuleDetails[currentModuleSectionNumber]->sectionNumber = pGlobalDetails->numberOfModules;
			pModuleDetails[currentModuleSectionNumber]->fileName = NULL;
			pModuleDetails[currentModuleSectionNumber]->fileSize = 0;
			pModuleDetails[currentModuleSectionNumber]->fileSizeUpperLimit = 0;
			pModuleDetails[currentModuleSectionNumber]->numberOfAddresses = 0;
			pModuleDetails[currentModuleSectionNumber]->baseAddress = 0;
			pModuleDetails[currentModuleSectionNumber]->retryCount = 0;
			pModuleDetails[currentModuleSectionNumber]->ignoreParsingErrors = false;
			pModuleDetails[currentModuleSectionNumber]->isDisabled = false;
			pModuleDetails[currentModuleSectionNumber]->isPatternMatchingFilename = false;

			currentSectionType = SECTIONTYPE_MODULE;
			continue;
		}
		// Plugin Section
		else if (ULib.StringLibrary.isStringMatch(currentLine, "[Plugin", false, 7))
		{
			currentPluginSectionNumber++;
			pGlobalDetails->numberOfPlugins++;
			totalNumberOfSections++;

			if (pGlobalDetails->numberOfPlugins > MAXIMUM_PLUGINS) return setParsingErrorMessage("Maximum number of Plugins has been exceeded");

			// initialise plugin section information
			pPluginDetails[currentPluginSectionNumber] = (PATCHINGDETAILS_PLUGIN*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PATCHINGDETAILS_PLUGIN));
			pPluginDetails[currentPluginSectionNumber]->sectionType = SECTIONTYPE_PLUGIN;
			pPluginDetails[currentPluginSectionNumber]->sectionNumber = pGlobalDetails->numberOfPlugins;
			pPluginDetails[currentPluginSectionNumber]->fileName = NULL;
			pPluginDetails[currentPluginSectionNumber]->fileSize = 0;
			pPluginDetails[currentPluginSectionNumber]->fileSizeUpperLimit = 0;
			pPluginDetails[currentPluginSectionNumber]->argumentsToForward = NULL;
			pPluginDetails[currentPluginSectionNumber]->minimumPluginVersionRequired = 0;
			pPluginDetails[currentPluginSectionNumber]->ignoreParsingErrors = false;
			pPluginDetails[currentPluginSectionNumber]->isDisabled = false;
			pPluginDetails[currentPluginSectionNumber]->isPatternMatchingFilename = false;

			currentSectionType = SECTIONTYPE_PLUGIN;
			continue;
		}
		// Registry Entry Section
		else if (ULib.StringLibrary.isStringMatch(currentLine, "[Registry", false, 9))
		{
			currentRegistryEntrySectionNumber++;
			pGlobalDetails->numberOfRegistryEntries++;
			totalNumberOfSections++;

			if (pGlobalDetails->numberOfRegistryEntries > MAXIMUM_REGISTRYENTRIES) return setParsingErrorMessage("Maximum number of Registry Entries has been exceeded");

			// initialise registry entry section information
			pRegistryEntryDetails[currentRegistryEntrySectionNumber] = (PATCHINGDETAILS_REGISTRYENTRY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PATCHINGDETAILS_REGISTRYENTRY));
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->sectionType = SECTIONTYPE_REGISTRYENTRY;
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->sectionNumber = pGlobalDetails->numberOfRegistryEntries;
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->numberOfRegistryActions = 0;
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->ignoreParsingErrors = false;
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->isDisabled = false;
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->use32BitView = false;
			pRegistryEntryDetails[currentRegistryEntrySectionNumber]->use64BitView = false;

			currentSectionType = SECTIONTYPE_REGISTRYENTRY;
			continue;
		}

		// parse the global parameters
		if (!parseGlobalParameters(currentLine, getGlobalPatchingDetails())) return false;

		// parse target process section parameters
		if (currentSectionType == SECTIONTYPE_TARGETPROCESS)
		{
			// parse base parameters
			if (!parseBaseParameters(currentLine, pTargetProcessDetails[currentTargetProcessSectionNumber])) return false;
			// parse file-based parameters
			if (!parseFileBaseParameters(currentLine, pTargetProcessDetails[currentTargetProcessSectionNumber])) return false;
			// parse memory patching parameters
			if (!parseMemoryParameters(currentLine, pTargetProcessDetails[currentTargetProcessSectionNumber])) return false;
			// parse parameters for target process types only
			if (!parseTargetProcessParameters(currentLine, pTargetProcessDetails[currentTargetProcessSectionNumber])) return false;
			continue;
		}
		// parse child process section parameters
		else if (currentSectionType == SECTIONTYPE_CHILDPROCESS)
		{
			// parse base parameters
			if (!parseBaseParameters(currentLine, pChildProcessDetails[currentChildProcessSectionNumber])) return false;
			// parse file-based parameters
			if (!parseFileBaseParameters(currentLine, pChildProcessDetails[currentChildProcessSectionNumber])) return false;
			// parse parameters for child process types only
			if (!parseChildProcessParameters(currentLine, pChildProcessDetails[currentChildProcessSectionNumber])) return false;
			continue;
		}
		// parse module section parameters
		else if (currentSectionType == SECTIONTYPE_MODULE)
		{
			// parse base parameters
			if (!parseBaseParameters(currentLine, pModuleDetails[currentModuleSectionNumber])) return false;
			// parse file-based parameters
			if (!parseFileBaseParameters(currentLine, pModuleDetails[currentModuleSectionNumber])) return false;
			// parse memory patching parameters
			if (!parseMemoryParameters(currentLine, pModuleDetails[currentModuleSectionNumber])) return false;
			// parse parameters for module types only
			if (!parseModuleParameters(currentLine, pModuleDetails[currentModuleSectionNumber])) return false;
			continue;
		}
		// parse plugin section parameters
		else if (currentSectionType == SECTIONTYPE_PLUGIN)
		{
			// parse base parameters
			if (!parseBaseParameters(currentLine, pPluginDetails[currentPluginSectionNumber])) return false;
			// parse file-based parameters
			if (!parseFileBaseParameters(currentLine, pPluginDetails[currentPluginSectionNumber])) return false;
			// parse parameters for plugin types only
			if (!parsePluginParameters(currentLine, pPluginDetails[currentPluginSectionNumber])) return false;
			continue;
		}
		// parse registry entry section parameters
		else if (currentSectionType == SECTIONTYPE_REGISTRYENTRY)
		{
			// parse base parameters
			if (!parseBaseParameters(currentLine, pRegistryEntryDetails[currentRegistryEntrySectionNumber])) return false;
			// parse parameters for registry entry types only
			if (!parseRegistryEntryParameters(currentLine, pRegistryEntryDetails[currentRegistryEntrySectionNumber])) return false;
			continue;
		}
	}

	ULib.HeapLibrary.freePrivateHeap(currentLine);
	ULib.HeapLibrary.freePrivateHeap(fileBufferNoComments);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);

	// if patching information is to be displayed then do not check for parsing errors
	if (isDisplayScriptInformationEnabled()) return true;

	// add the command line arguments to the argumentsToForward parameter
	if (getTargetProcessPatchingDetails() != NULL)
	{
		if (getTargetProcessPatchingDetails()->argumentsToForward == NULL) getTargetProcessPatchingDetails()->argumentsToForward = (char*)ULib.HeapLibrary.allocPrivateHeap();
		else ULib.StringLibrary.appendString(getTargetProcessPatchingDetails()->argumentsToForward, 0, " ");
		ULib.StringLibrary.appendString(getTargetProcessPatchingDetails()->argumentsToForward, 0, getForwardedCommandLine());
	}

	////////////////////
	// ERROR CHECKING //
	////////////////////

	// only allow upto MAXIMUM_SECTIONS (i.e. threads)
	if (totalNumberOfSections > MAXIMUM_SECTIONS) return setParsingErrorMessage("Maximum number of Sections has been exceeded");

	// check global parameters
	if (!checkGlobalParameters(getGlobalPatchingDetails())) return false;

	// check target process section parameters
	if (pGlobalDetails->numberOfTargetProcesses > 0)
	{
		// check base parameters
		if (!checkBaseParameters(getTargetProcessPatchingDetails())) return false;
		// check file-based parameters
		if (!checkFileBaseParameters(getTargetProcessPatchingDetails())) return false;
		// check memory patching parameters
		if (!checkMemoryParameters(getTargetProcessPatchingDetails(), getTargetProcessPatchingDetails())) return false;
		// check parameters specific to process type patches
		if (!checkTargetProcessParameters(getTargetProcessPatchingDetails())) return false;
	}

	// check child process section parameters
	if (pGlobalDetails->numberOfChildProcesses > 0)
	{
		// check base parameters
		if (!checkBaseParameters(getChildProcessPatchingDetails())) return false;
		// check file-based parameters
		if (!checkFileBaseParameters(getChildProcessPatchingDetails())) return false;
		// check parameters specific to child process type patches
		if (!checkChildProcessParameters(getChildProcessPatchingDetails())) return false;
	}

	// check module section parameters
	for (int indexNumber = 0; indexNumber < pGlobalDetails->numberOfModules; indexNumber++)
	{
		// check base parameters
		if (!checkBaseParameters(getModulePatchingDetails(indexNumber))) return false;
		// check file-based parameters
		if (!checkFileBaseParameters(getModulePatchingDetails(indexNumber))) return false;
		// check memory patching parameters
		if (!checkMemoryParameters(getModulePatchingDetails(indexNumber), getModulePatchingDetails(indexNumber))) return false;
		// check parameters specific to module type patches
		if (!checkModuleParameters(getModulePatchingDetails(indexNumber))) return false;
	}

	// check plugin section parameters
	for (int indexNumber = 0; indexNumber < pGlobalDetails->numberOfPlugins; indexNumber++)
	{
		// check base parameters
		if (!checkBaseParameters(getPluginPatchingDetails(indexNumber))) return false;
		// check file-based parameters
		if (!checkFileBaseParameters(getPluginPatchingDetails(indexNumber))) return false;
		// check parameters specific to plugin type patches
		if (!checkPluginParameters(getPluginPatchingDetails(indexNumber))) return false;
	}

	// check registry entry section parameters
	for (int indexNumber = 0; indexNumber < pGlobalDetails->numberOfRegistryEntries; indexNumber++)
	{
		// check base parameters
		if (!checkBaseParameters(getRegistryEntryPatchingDetails(indexNumber))) return false;
		// check parameters specific to registry entry type patches
		if (!checkRegistryEntryParameters(getRegistryEntryPatchingDetails(indexNumber))) return false;
	}

	// check that this patch is compatible with the current operating system
	if (!checkOperatingSystemPlatform(getGlobalPatchingDetails())) return false;
	return true;
}

// frees the memory used by the parsing engine
void cleanupParsedEntries(void)
{
	if (pGlobalDetails->numberOfTargetProcesses > 0)
	{
		PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails = getTargetProcessPatchingDetails();
		if ((pTargetProcessDetails != NULL) && (pTargetProcessDetails->sectionType == SECTIONTYPE_TARGETPROCESS))
		{
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails->displayName);
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails->fileName);
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails->argumentsToForward);
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails->displayUserNotifyBoxMessage);
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails->waitForLocalWindowName);
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails->waitForGlobalWindowName);
			ULib.HeapLibrary.freePrivateHeap(pTargetProcessDetails);
		}
	}
	if (pGlobalDetails->numberOfChildProcesses > 0)
	{
		PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails = getChildProcessPatchingDetails();
		if ((pChildProcessDetails != NULL) && (pChildProcessDetails->sectionType == SECTIONTYPE_CHILDPROCESS))
		ULib.HeapLibrary.freePrivateHeap(pChildProcessDetails->fileName);
		ULib.HeapLibrary.freePrivateHeap(pChildProcessDetails);
	}
	for (int i = 0; i < pGlobalDetails->numberOfModules; i++)
	{
		PATCHINGDETAILS_MODULE* pModuleDetails = getModulePatchingDetails(i);
		if ((pModuleDetails == NULL) || (pModuleDetails->sectionType != SECTIONTYPE_MODULE)) continue;
		ULib.HeapLibrary.freePrivateHeap(pModuleDetails->fileName);
		ULib.HeapLibrary.freePrivateHeap(pModuleDetails);
	}
	for (int i = 0; i < pGlobalDetails->numberOfPlugins; i++)
	{
		PATCHINGDETAILS_PLUGIN* pPluginDetails = getPluginPatchingDetails(i);
		if ((pPluginDetails == NULL) || (pPluginDetails->sectionType != SECTIONTYPE_PLUGIN)) continue;
		ULib.HeapLibrary.freePrivateHeap(pPluginDetails->fileName);
		ULib.HeapLibrary.freePrivateHeap(pPluginDetails->argumentsToForward);
		ULib.HeapLibrary.freePrivateHeap(pPluginDetails);
	}
	for (int i = 0; i < pGlobalDetails->numberOfRegistryEntries; i++)
	{
		PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails = getRegistryEntryPatchingDetails(i);
		if ((pRegistryEntryDetails == NULL) || (pRegistryEntryDetails->sectionType != SECTIONTYPE_REGISTRYENTRY)) continue;
		for (int j = 0; j < pRegistryEntryDetails->numberOfRegistryActions; j++)
		{
			ULib.HeapLibrary.freePrivateHeap(pRegistryEntryDetails->registrySubKey[j]);
			ULib.HeapLibrary.freePrivateHeap(pRegistryEntryDetails->registryValueName[j]);
			ULib.HeapLibrary.freePrivateHeap(pRegistryEntryDetails->registryValueData[j]);
		}
		ULib.HeapLibrary.freePrivateHeap(pRegistryEntryDetails);
	}

	ULib.HeapLibrary.freePrivateHeap(pGlobalDetails->scriptFilename);
	ULib.HeapLibrary.freePrivateHeap(pGlobalDetails->patchInformation);
	ULib.HeapLibrary.freePrivateHeap(pGlobalDetails->patchAuthor);
	ULib.HeapLibrary.freePrivateHeap(pGlobalDetails->patchContactInformation);
	ULib.HeapLibrary.freePrivateHeap(pGlobalDetails);
}