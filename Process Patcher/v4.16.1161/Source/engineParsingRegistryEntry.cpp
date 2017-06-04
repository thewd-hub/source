///////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for registry entries. Stores //
// the information in the PATCHINGDETAILS_REGISTRYENTRY structure    //
///////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingRegistryEntry.h"

// used to determine the correct format and length of the registry data,
// based on the type. Only done when type and data have been parsed
bool determineRegistryDataAndLength(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails)
{
	if ((pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions] == NULL) ||
		(pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REGISTRYENTRY_TYPE_UNDEFINED))
	{
		return false;
	}

	// determine the correct format and length of data
	if ((pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_SZ) ||
		(pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_EXPAND_SZ))
	{
		pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions] = ULib.StringLibrary.getStringLength((char*)pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions]);
		pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions]++;
	}
	else if (pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_MULTI_SZ)
	{
		pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions] = ULib.StringLibrary.getStringLength((char*)pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions]);
		pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions] += 2;
	}
	else if ((pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_DWORD) ||
		(pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_DWORD_LITTLE_ENDIAN) ||
		(pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_DWORD_BIG_ENDIAN))
	{
		pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions] = 4;
		unsigned long currentRegistryValue = ULib.StringLibrary.getDWORDFromString((char*)pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions]);
		pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions][0] = (unsigned char)(currentRegistryValue & 0xFF);
		pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions][1] = (unsigned char)((currentRegistryValue >> 8) & 0xFF);
		pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions][2] = (unsigned char)((currentRegistryValue >> 16) & 0xFF);
		pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions][3] = (unsigned char)((currentRegistryValue >> 24) & 0xFF);
	}
	else if (pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] == REG_BINARY)
	{
		pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions] = 0;

		if (ULib.StringLibrary.getStringLength((char*)pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions]) > 0)
		{
			// get the string elements, and parse
			char* stringElements[1024];
			int numberOfStringElements = 0;
			ULib.StringLibrary.getStringElementsFromString((char*)pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions], stringElements, &numberOfStringElements, 1024, ',');
			for (int i = 0; i < numberOfStringElements; i++)
			{
				pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions][pRegistryEntryDetails->registryValueDataLength[pRegistryEntryDetails->numberOfRegistryActions]++] = (unsigned char)(ULib.StringLibrary.getDWORDFromString(stringElements[i]) & 0xFF);
				ULib.HeapLibrary.freePrivateHeap(stringElements[i]);
			}
		}
	}

	return true;
}

// attempt to parse the valid parameters for the registry entry types
bool parseRegistryEntryParameters(char* stringBuffer, PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// use 32-bit alternative view of the registry (if running under 64-bit process)
	if ((parsingResult = parseStringParameter("Use32BitView", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pRegistryEntryDetails->use32BitView = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// use 64-bit alternative view of the registry (if running under WOW64 process)
	if ((parsingResult = parseStringParameter("Use64BitView", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pRegistryEntryDetails->use64BitView = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// action to perform to the registry key/value
	if ((parsingResult = parseStringParameter("Action", pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions], parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		// correct number of arguments for required action
		//
		//    Action   | Key/Value Name | Value Data | Value Type
		// -------------------------------------------------------
		// CreateKey   |      Yes       |     No     |     No
		// DeleteKey   |      Yes       |     No     |     No
		// DeleteValue |      Yes       |     No     |     No
		// SetValue    |      Yes       |     Yes    |     Yes
		//
		// Value Types: REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ, REG_BINARY
		//              REG_DWORD, REG_DWORD_LITTLE_ENDIAN, REG_DWORD_BIG_ENDIAN

		// get the string elements, and parse
		int numberOfStringElements = 0;
		char* stringElements[16];
		ULib.StringLibrary.getStringElementsFromString(parsingString, stringElements, &numberOfStringElements, 16, ',');

		// invalid number of required arguments
		if (numberOfStringElements < 1) return setParsingErrorMessage("Invalid number of arguments for the Registry Action");

		// determine registry action
		ULib.StringLibrary.trimStringSpaces(stringElements[0], true, true, true);
		if (ULib.StringLibrary.isStringMatch(stringElements[0], "CreateKey")) pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_ACTION_CREATEKEY;
		else if (ULib.StringLibrary.isStringMatch(stringElements[0], "DeleteKey")) pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_ACTION_DELETEKEY;
		else if (ULib.StringLibrary.isStringMatch(stringElements[0], "DeleteValue")) pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_ACTION_DELETEVALUE;
		else if (ULib.StringLibrary.isStringMatch(stringElements[0], "SetValue")) pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_ACTION_SETVALUE;
		else pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_ACTION_UNDEFINED;

		// determine key/value name
		if (numberOfStringElements < 2) return setParsingErrorMessage("Unable to parse the registry key name or value name");

		// get the path elements and parse (registry keys have a trailing slash, values don't)
		int numberOfPathElements = 0;
		char* pathElements[128];
		ULib.StringLibrary.getStringElementsFromString(stringElements[1], pathElements, &numberOfPathElements, 128, '\\');

		if (numberOfPathElements < 2) return setParsingErrorMessage("Unable to parse the registry key name or value name");

		// get registry hive key
		ULib.StringLibrary.trimStringSpaces(pathElements[0], true, true, true);
		if ((ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_CLASSES_ROOT")) || (ULib.StringLibrary.isStringMatch(pathElements[0], "HKCR"))) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_CLASSES_ROOT;
		else if (ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_CURRENT_CONFIG")) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_CURRENT_CONFIG;
		else if ((ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_CURRENT_USER")) || (ULib.StringLibrary.isStringMatch(pathElements[0], "HKCU"))) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_CURRENT_USER;
		else if ((ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_LOCAL_MACHINE")) || (ULib.StringLibrary.isStringMatch(pathElements[0], "HKLM"))) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_LOCAL_MACHINE;
		else if (ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_USERS")) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_USERS;
		// Windows NTx
		else if (ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_PERFORMANCE_DATA")) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_PERFORMANCE_DATA;
		// Windows 9x
		else if (ULib.StringLibrary.isStringMatch(pathElements[0], "HKEY_DYN_DATA")) pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = HKEY_DYN_DATA;
		else pRegistryEntryDetails->registryHive[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_HIVE_UNDEFINED;

		int numberOfKeyElements = numberOfPathElements;
		// assume it's a value name (when deleting or setting a key value)
		// (trailing slash - default/unnamed key value)
		if (((pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] == REGISTRYENTRY_ACTION_DELETEVALUE) ||
			(pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] == REGISTRYENTRY_ACTION_SETVALUE)) &&
			(stringElements[1][ULib.StringLibrary.getStringLength(stringElements[1]) - 1] != '\\'))
		{
			numberOfKeyElements--;
		}

		pRegistryEntryDetails->registrySubKey[pRegistryEntryDetails->numberOfRegistryActions] = (char*)ULib.HeapLibrary.allocPrivateHeap();
		for (int i = 1; i < numberOfKeyElements; i++)
		{
			ULib.StringLibrary.appendString(pRegistryEntryDetails->registrySubKey[pRegistryEntryDetails->numberOfRegistryActions], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, pathElements[i]);
			ULib.StringLibrary.appendString(pRegistryEntryDetails->registrySubKey[pRegistryEntryDetails->numberOfRegistryActions], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "\\");
		}

		// parsing finished, unless action is DeleteValue
		if (pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] == REGISTRYENTRY_ACTION_DELETEVALUE)
		{
			pRegistryEntryDetails->registryValueName[pRegistryEntryDetails->numberOfRegistryActions] = (char*)ULib.HeapLibrary.allocPrivateHeap();
			// determine value name
			if (numberOfKeyElements != numberOfPathElements)
			{
				// store the last key element as the value name
				ULib.StringLibrary.copyString(pRegistryEntryDetails->registryValueName[pRegistryEntryDetails->numberOfRegistryActions], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, pathElements[numberOfPathElements - 1]);
			}
		}

		// parsing finished, unless action is SetValue
		if (pRegistryEntryDetails->registryAction[pRegistryEntryDetails->numberOfRegistryActions] == REGISTRYENTRY_ACTION_SETVALUE)
		{
			pRegistryEntryDetails->registryValueName[pRegistryEntryDetails->numberOfRegistryActions] = (char*)ULib.HeapLibrary.allocPrivateHeap();
			// determine value name
			if (numberOfKeyElements != numberOfPathElements)
			{
				// store the last key element as the value name
				ULib.StringLibrary.copyString(pRegistryEntryDetails->registryValueName[pRegistryEntryDetails->numberOfRegistryActions], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, pathElements[numberOfPathElements - 1]);
			}

			// registry value data
			if (numberOfStringElements < 3) return setParsingErrorMessage("Unable to parse the data for the registry value");

			// store the value data element
			pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions] = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
			ULib.StringLibrary.copyString((char*)pRegistryEntryDetails->registryValueData[pRegistryEntryDetails->numberOfRegistryActions], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, stringElements[2]);

			// registry data type
			if (numberOfStringElements < 4) return setParsingErrorMessage("Unable to parse the specified registry value type");

			// string keys
			ULib.StringLibrary.trimStringSpaces(stringElements[3], true, true, true);
			if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_SZ")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_SZ;
			else if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_EXPAND_SZ")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_EXPAND_SZ;
			else if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_MULTI_SZ")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_MULTI_SZ;
			// dword keys
			else if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_DWORD")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_DWORD;
			else if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_DWORD_LITTLE_ENDIAN")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_DWORD_LITTLE_ENDIAN;
			else if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_DWORD_BIG_ENDIAN")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_DWORD_BIG_ENDIAN;
			// binary key
			else if (ULib.StringLibrary.isStringMatch(stringElements[3], "REG_BINARY")) pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REG_BINARY;
			else pRegistryEntryDetails->registryValueType[pRegistryEntryDetails->numberOfRegistryActions] = REGISTRYENTRY_TYPE_UNDEFINED;

			// try to determine the data format length (and whether required parameters have been parsed)
			determineRegistryDataAndLength(pRegistryEntryDetails);
		}

		pRegistryEntryDetails->numberOfRegistryActions++;

		// too many registry entries for current section
		if (pRegistryEntryDetails->numberOfRegistryActions > MAXIMUM_REGISTRYACTIONS) return setParsingErrorMessage("Exceeded the number of allowed Registry Actions");

		// cleanup allocated memory
		for (int i = 0; i < numberOfStringElements; i++) ULib.HeapLibrary.freePrivateHeap(stringElements[i]);
		for (int i = 0; i < numberOfPathElements; i++) ULib.HeapLibrary.freePrivateHeap(pathElements[i]);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkRegistryEntryParameters(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails)
{
	// ignore all parsing errors
	if (pRegistryEntryDetails->ignoreParsingErrors) return true;
	// check section number
	if (pRegistryEntryDetails->sectionNumber <= 0) return false;

	// no registry entries found
	if (pRegistryEntryDetails->numberOfRegistryActions <= 0)
	{
		char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to find any Action parameters - Registry Section %d", pRegistryEntryDetails->sectionNumber);
		setParsingErrorMessage(errorMessage);
		ULib.HeapLibrary.freePrivateHeap(errorMessage);
		return false;
	}

	// cycle throught registry entries, checking for invalid properties
	for (int i = 0; i < pRegistryEntryDetails->numberOfRegistryActions; i++)
	{
		// unable to find the registry action property
		if (pRegistryEntryDetails->registryAction[i] == REGISTRYENTRY_ACTION_UNDEFINED)
		{
			char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
			sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to parse or determine the Action - Registry Section %d", pRegistryEntryDetails->sectionNumber);
			setParsingErrorMessage(errorMessage);
			ULib.HeapLibrary.freePrivateHeap(errorMessage);
			return false;
		}

		// unable to find the registry key property
		if ((pRegistryEntryDetails->registryHive[i] == REGISTRYENTRY_HIVE_UNDEFINED) ||
			(pRegistryEntryDetails->registrySubKey[i] == NULL) || (pRegistryEntryDetails->registrySubKey[i][0] == NULL))
		{
			char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
			sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to parse or determine the key name - Registry Section %d", pRegistryEntryDetails->sectionNumber);
			setParsingErrorMessage(errorMessage);
			ULib.HeapLibrary.freePrivateHeap(errorMessage);
			return false;
		}

		// check required parameters for SetValue action
		if (pRegistryEntryDetails->registryAction[i] == REGISTRYENTRY_ACTION_SETVALUE)
		{
			if (pRegistryEntryDetails->registryValueName[i] == NULL)
			{
				char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to parse or determine the value name - Registry Section %d", pRegistryEntryDetails->sectionNumber);
				setParsingErrorMessage(errorMessage);
				ULib.HeapLibrary.freePrivateHeap(errorMessage);
				return false;
			}

			if (pRegistryEntryDetails->registryValueData[i] == NULL)
			{
				char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to parse or determine the value data - Registry Section %d", pRegistryEntryDetails->sectionNumber);
				setParsingErrorMessage(errorMessage);
				ULib.HeapLibrary.freePrivateHeap(errorMessage);
				return false;
			}

			if (pRegistryEntryDetails->registryValueType[i] == REGISTRYENTRY_TYPE_UNDEFINED)
			{
				char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to parse or determine the value type - Registry Section %d", pRegistryEntryDetails->sectionNumber);
				setParsingErrorMessage(errorMessage);
				ULib.HeapLibrary.freePrivateHeap(errorMessage);
				return false;
			}
		}

		// check required parameters for DeleteValue action
		if (pRegistryEntryDetails->registryAction[i] == REGISTRYENTRY_ACTION_DELETEVALUE)
		{
			if (pRegistryEntryDetails->registryValueName[i] == NULL)
			{
				char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
				sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to parse the Value Name property - Registry Section %d", pRegistryEntryDetails->sectionNumber);
				setParsingErrorMessage(errorMessage);
				ULib.HeapLibrary.freePrivateHeap(errorMessage);
				return false;
			}
		}
	}

	return true;
}