///////////////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for all patching types that patches the  //
// memory. Stores the information in the corresponding PATCHINGDETAILS structure //
///////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingMemory.h"

// attempt to parse all the valid parameters for all
// the patching types that patches memory addresses
bool parseMemoryParameters(char* stringBuffer, PATCHINGDETAILS_MEMORY* pMemoryDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// number of times in which to attempt to re-patch the memory addresses
	if ((parsingResult = parseStringParameter("RetryCount", pMemoryDetails->retryCount, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pMemoryDetails->retryCount = ULib.StringLibrary.getDWORDFromString(parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// the memory locations and details of the memory patch to perform
	if ((parsingResult = parseStringParameter("Address", pMemoryDetails->memoryAddress[pMemoryDetails->numberOfAddresses], parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		// get the string elements and parse
		char* stringElements[6];
		int numberOfStringElements = 0;
		ULib.StringLibrary.getStringElementsFromString(parsingString, stringElements, &numberOfStringElements, 6, ':');
		if (numberOfStringElements < 3) return setParsingErrorMessage("Error occurred parsing the Address parameter");

		char* stringExpectedByteElements[MAXIMUM_MEMORYADDRESSBYTES];
		char* stringPatchedByteElements[MAXIMUM_MEMORYADDRESSBYTES];
		int countExpectedByteElements = 0;
		int countPatchedByteElements = 0;

		// get individual items from the byte groups
		ULib.StringLibrary.getStringElementsFromString(stringElements[1], stringExpectedByteElements, &countExpectedByteElements, MAXIMUM_MEMORYADDRESSBYTES, ',');
		ULib.StringLibrary.getStringElementsFromString(stringElements[2], stringPatchedByteElements, &countPatchedByteElements, MAXIMUM_MEMORYADDRESSBYTES, ',');
		if (countExpectedByteElements != countPatchedByteElements) return setParsingErrorMessage("Error occurred parsing Address parameter - byte count differs");

		// get starting memory address
		unsigned long currentMemoryAddress = ULib.StringLibrary.getDWORDFromString(stringElements[0]);
		// get memory address variation
		unsigned long memoryAddressVariation = 0;
		if (numberOfStringElements >= 4) memoryAddressVariation = ULib.StringLibrary.getDWORDFromString(stringElements[3]);

		// cycle through each memory address
		for (int i = 0; i < countExpectedByteElements; i++)
		{
			pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = 0;
			pMemoryDetails->memoryAddress[pMemoryDetails->numberOfAddresses] = currentMemoryAddress;
			pMemoryDetails->memoryAddressVariation[pMemoryDetails->numberOfAddresses] = memoryAddressVariation;

			// allow any expected byte
			if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "."))
			{
				pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] |= MEMORYOPERATOR_EXPECTEDANY;
			}
			else
			{
				char* byteOffset = stringExpectedByteElements[i];
				pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPEQ;

				// comparision ==
				if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "==", false, 2))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPEQ;
					byteOffset += 2;
				}
				// comparision <> or !=
				else if ((ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "<>", false, 2)) ||
						(ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "!=", false, 2)))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPNOT;
					byteOffset += 2;
				}
				// comparision >=
				else if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], ">=", false, 2))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPGTEQ;
					byteOffset += 2;
				}
				// comparision <=
				else if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "<=", false, 2))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPLTEQ;
					byteOffset += 2;
				}
				// comparision >< (between byte1 and byte2)
				else if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "><", false, 2))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPBETWEEN;
					byteOffset += 2;

					// get the end byte
					char* byte2Offset = strstr(byteOffset, "-");
					if (byte2Offset == NULL) return setParsingErrorMessage("Error occurred parsing Address parameter - end byte not found");
					pMemoryDetails->expectedByte2[pMemoryDetails->numberOfAddresses] = ULib.StringLibrary.getBYTEFromString(++byte2Offset);
				}
				// comparision =
				else if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "=", false, 1))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPEQ;
					byteOffset++;
				}
				// comparision >
				else if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], ">", false, 1))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPGT;
					byteOffset++;
				}
				// comparision <
				else if (ULib.StringLibrary.isStringMatch(stringExpectedByteElements[i], "<", false, 1))
				{
					pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] = MEMORYOPERATOR_COMPLT;
					byteOffset++;
				}

				pMemoryDetails->expectedByte[pMemoryDetails->numberOfAddresses] = ULib.StringLibrary.getBYTEFromString(byteOffset);
			}

			// no patch required (keep as expected byte)
			if (ULib.StringLibrary.isStringMatch(stringPatchedByteElements[i], ".")) pMemoryDetails->memoryByteOperator[pMemoryDetails->numberOfAddresses] |= MEMORYOPERATOR_EXPECTEDPATCH;
			else pMemoryDetails->patchedByte[pMemoryDetails->numberOfAddresses] = ULib.StringLibrary.getBYTEFromString(stringPatchedByteElements[i]);

			ULib.HeapLibrary.freePrivateHeap(stringExpectedByteElements[i]);
			ULib.HeapLibrary.freePrivateHeap(stringPatchedByteElements[i]);

			pMemoryDetails->numberOfAddresses++;
			currentMemoryAddress++;

			// too many memory addresses for current section
			if (pMemoryDetails->numberOfAddresses > MAXIMUM_MEMORYADDRESSES) return setParsingErrorMessage("Exceeded the number of allowed Memory Addresses");
		}

		for (int i = 0; i < numberOfStringElements; i++) ULib.HeapLibrary.freePrivateHeap(stringElements[i]);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the compulsory parameters have been
// defined and all other parameters are correct
bool checkMemoryParameters(PATCHINGDETAILS_MEMORY* pMemoryDetails, PATCHINGDETAILS_BASE* pBaseDetails)
{
	// ignore all parsing errors
	if (pBaseDetails->ignoreParsingErrors) return true;
	// check section number
	if (pBaseDetails->sectionNumber <= 0) return false;

	// check for invalid memory addresses
	for (int i = 0; i < pMemoryDetails->numberOfAddresses; i++)
	{
		if (pMemoryDetails->memoryAddress[i] <= 0)
		{
			char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
			if (pBaseDetails->sectionType == SECTIONTYPE_TARGETPROCESS) ULib.StringLibrary.copyString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to determine Address information - Target Process Section");
			else if (pBaseDetails->sectionType == SECTIONTYPE_MODULE) sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to determine Address information - Module Section %d", pBaseDetails->sectionNumber);
			else sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Unable to determine the Address information");
			setParsingErrorMessage(errorMessage);
			ULib.HeapLibrary.freePrivateHeap(errorMessage);
			return false;
		}
	}

	return true;
}