///////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for all file patching types.     //
// Stores the information in the corresponding PATCHINGDETAILS structure //
///////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingFileBase.h"

// use the file open dialog box so the user can specify the file name
bool getRequestedFilename(PATCHINGDETAILS_FILEBASE* pFileBaseDetails, char* parsingString)
{
	bool isTargetProcessSection = (pFileBaseDetails->sectionType == SECTIONTYPE_TARGETPROCESS);
	bool isChildProcessSection = (pFileBaseDetails->sectionType == SECTIONTYPE_CHILDPROCESS);
	bool isModuleSection = (pFileBaseDetails->sectionType == SECTIONTYPE_MODULE);
	bool isPluginSection = (pFileBaseDetails->sectionType == SECTIONTYPE_PLUGIN);

	if (isTargetProcessSection) return getFilenameUsingExplorer("Process Patcher - Select Target Process", "Executables (*.exe)\0*.exe\0\0", "exe", parsingString);
	else if (isChildProcessSection) return getFilenameUsingExplorer("Process Patcher - Select Child Process", "Executables (*.exe)\0*.exe\0\0", "exe", parsingString);
	else if ((isModuleSection) || (isPluginSection))
	{
		char* titleString = (char*)ULib.HeapLibrary.allocPrivateHeap();
		sprintf_s(titleString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Process Patcher - Select %s Library (Section %d)", ((isModuleSection) ? "Module" : "Plugin"), pFileBaseDetails->sectionNumber);
		bool ofnResult = getFilenameUsingExplorer(titleString, "Dynamic Link Libraries (*.dll)\0*.dll\0\0", "dll", parsingString);
		ULib.HeapLibrary.freePrivateHeap(titleString);
		return ofnResult;
	}

	return false;
}

// attempt to parse the valid parameters for all the file-based patching types
bool parseFileBaseParameters(char* stringBuffer, PATCHINGDETAILS_FILEBASE* pFileBaseDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// filename associated with the current section
	if ((parsingResult = parseStringParameter("Filename", pFileBaseDetails->fileName, parsingString, stringBuffer, true)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		bool storeParsedFilename = true;
		// request filename (if required)
		if ((ULib.StringLibrary.getStringLength(parsingString) <= 0) || (ULib.StringLibrary.isStringMatch(parsingString, "<request>")))
		{
			// just store if opening an existing process, otherwise use explorer to select filename
			PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails = getTargetProcessPatchingDetails();
			if ((pTargetProcessDetails != NULL) && (pTargetProcessDetails->openExistingProcess)) ULib.StringLibrary.copyString(parsingString, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "<request>");
			else if (!getRequestedFilename(pFileBaseDetails, parsingString)) storeParsedFilename = false;
		}

		// store the filename
		if (storeParsedFilename)
		{
			pFileBaseDetails->fileName = (char*)ULib.HeapLibrary.allocPrivateHeap();
			ULib.StringLibrary.copyString(pFileBaseDetails->fileName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
			// checks whether the Filename relies on pattern-matching
			pFileBaseDetails->isPatternMatchingFilename = ULib.StringLibrary.doesStringContainPatternMatchingSymbols(pFileBaseDetails->fileName);
		}

		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// file size associated with the current section (but not plugin sections)
	if ((parsingResult = parseStringParameter("Filesize", pFileBaseDetails->fileSize, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if ((parsingResult == PARSINGENGINE_PARSINGOK) && (pFileBaseDetails->sectionType != SECTIONTYPE_PLUGIN))
	{
		char* parsingStringRemoved = ULib.StringLibrary.removeCharacterFromString(parsingString, ',');

		// check for file size ranges
		char* stringFileSizeElements[3];
		int countFileSizeElements = 0;
		ULib.StringLibrary.getStringElementsFromString(parsingStringRemoved, stringFileSizeElements, &countFileSizeElements, 3, '-');
		if (countFileSizeElements >= 2)
		{
			pFileBaseDetails->fileSize = ULib.StringLibrary.getDWORDFromString(stringFileSizeElements[0]);
			pFileBaseDetails->fileSizeUpperLimit = ULib.StringLibrary.getDWORDFromString(stringFileSizeElements[1]);
			// check fileSize is < fileSizeUpperLimit
			if (pFileBaseDetails->fileSize > pFileBaseDetails->fileSizeUpperLimit)
			{
				pFileBaseDetails->fileSize = 0;
				pFileBaseDetails->fileSizeUpperLimit = 0;
			}
		}
		else
		{
			pFileBaseDetails->fileSize = ULib.StringLibrary.getDWORDFromString(parsingStringRemoved);
			pFileBaseDetails->fileSizeUpperLimit = 0;
		}

		for (int i = 0; i < countFileSizeElements; i++) ULib.HeapLibrary.freePrivateHeap(stringFileSizeElements[i]);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingStringRemoved);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkFileBaseParameters(PATCHINGDETAILS_FILEBASE* pFileBaseDetails)
{
	// ignore all parsing errors
	if (pFileBaseDetails->ignoreParsingErrors) return true;
	// check section number
	if (pFileBaseDetails->sectionNumber <= 0) return false;

	// unable to find the Filename parameter for the current section
	if ((pFileBaseDetails->fileName == NULL) || (pFileBaseDetails->fileName[0] == NULL))
	{
		char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		if (pFileBaseDetails->sectionType == SECTIONTYPE_TARGETPROCESS) ULib.StringLibrary.copyString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find the Filename parameter - Target Process Section");
		else if (pFileBaseDetails->sectionType == SECTIONTYPE_CHILDPROCESS) ULib.StringLibrary.copyString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find the Filename parameter - Child Process Section");
		else if (pFileBaseDetails->sectionType == SECTIONTYPE_MODULE) sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find Filename parameter - Module Section %d", pFileBaseDetails->sectionNumber);
		else if (pFileBaseDetails->sectionType == SECTIONTYPE_PLUGIN) sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find Filename parameter - Plugin Section %d", pFileBaseDetails->sectionNumber);
		else sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find required Filename parameter");
		setParsingErrorMessage(errorMessage);
		ULib.HeapLibrary.freePrivateHeap(errorMessage);
		return false;
	}

	return true;
}