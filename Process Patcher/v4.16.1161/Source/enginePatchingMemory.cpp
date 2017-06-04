///////////////////////////////////////////////////////////
// Handles the reading and writing of the process memory //
///////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "enginePatching.h"
#include "enginePatchingMemory.h"
#include "enginePatchingModule.h"
#include "enginePatchingSupport.h"
#include "enginePatchingTargetProcess.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

// changes the memory protection back to its original settings
bool revertMemoryProtection(HANDLE processHandle, PVOID memoryAddress, unsigned long numberOfBytes, unsigned long newProtectionFlags)
{
	// only applicable on Windows NTx
	if (!ULib.OperatingSystemLibrary.isWindowsNTx()) return true;

	// revert the memory protection back to its original status before this patching engine altered it
	unsigned long oldProtectionFlags;
	return (VirtualProtectEx(processHandle, memoryAddress, numberOfBytes, newProtectionFlags, &oldProtectionFlags) != 0);
}

// alter the memory protection so that the patching engine
// can succeed. Call revertMemoryProtection to undo change
unsigned long alterMemoryProtection(HANDLE processHandle, PVOID memoryAddress, unsigned long numberOfBytes)
{
	// only applicable on Windows NTx
	if (!ULib.OperatingSystemLibrary.isWindowsNTx()) return NULL;

	// query the memory to see if it's protected
	MEMORY_BASIC_INFORMATION* memoryProtectionInformation = (MEMORY_BASIC_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(MEMORY_BASIC_INFORMATION));
	if (VirtualQueryEx(processHandle, memoryAddress, memoryProtectionInformation, sizeof(MEMORY_BASIC_INFORMATION)) == 0) return NULL;

	bool isPageNoAccess = false;
	bool isPageGuard = false;
	bool alterProtection = false;
	bool alterProtectionExecute = false;
	bool alterProtectionExecuteRead = false;
	bool alterProtectionReadOnly = false;

	// check for no access protection
	isPageNoAccess = ((memoryProtectionInformation->Protect & PAGE_NOACCESS) ? true : false);
	// check for guard protection
	isPageGuard = ((memoryProtectionInformation->Protect & PAGE_GUARD) ? true : false);
	// check for execute protection
	alterProtectionExecute = ((memoryProtectionInformation->Protect & PAGE_EXECUTE) ? true : false);
	// check for execute-read protection
	alterProtectionExecuteRead = ((memoryProtectionInformation->Protect & PAGE_EXECUTE_READ) ? true : false);
	// check for read-only protection
	alterProtectionReadOnly = ((memoryProtectionInformation->Protect & PAGE_READONLY) ? true : false);

	unsigned long oldProtectionFlags = memoryProtectionInformation->Protect;
	unsigned long newProtectionFlags = memoryProtectionInformation->Protect;
	ULib.HeapLibrary.freePrivateHeap(memoryProtectionInformation);

	// remove page no access protection
	if (isPageNoAccess) newProtectionFlags ^= PAGE_NOACCESS;
	// remove page guard protection
	if (isPageGuard) newProtectionFlags ^= PAGE_GUARD;
	// add execute read-write protection
	if (alterProtectionExecute)
	{
		newProtectionFlags ^= PAGE_EXECUTE;
		newProtectionFlags |= PAGE_EXECUTE_READWRITE;
		alterProtection = true;
	}
	// add execute read-write protection
	else if (alterProtectionExecuteRead)
	{
		newProtectionFlags ^= PAGE_EXECUTE_READ;
		newProtectionFlags |= PAGE_EXECUTE_READWRITE;
		alterProtection = true;
	}
	// add read-write protection
	else if (alterProtectionReadOnly)
	{
		newProtectionFlags ^= PAGE_READONLY;
		newProtectionFlags |= PAGE_READWRITE;
		alterProtection = true;
	}

	// alter the memory protection for the required memory addresses
	if ((alterProtection) && (newProtectionFlags != oldProtectionFlags)) return ((VirtualProtectEx(processHandle, memoryAddress, numberOfBytes, newProtectionFlags, &oldProtectionFlags) == 0) ? NULL : oldProtectionFlags);
	return NULL;
}

// read the process memory
bool readMemoryAddress(HANDLE processHandle, PVOID memoryAddress, unsigned long numberOfBytes, PVOID memoryBuffer, bool alterMemoryAccessProtection)
{
	__try
	{
		if (getMemoryAccessLock() != NULL) EnterCriticalSection(getMemoryAccessLock());

		unsigned long oldProtectionFlags = NULL;
		if (alterMemoryAccessProtection) oldProtectionFlags = alterMemoryProtection(processHandle, memoryAddress, numberOfBytes);
		unsigned long bytesRead = 0;
		bool resultReadMemory = ((ReadProcessMemory(processHandle, memoryAddress, memoryBuffer, numberOfBytes, (SIZE_T*)&bytesRead) == 0) ? false : true);
		if ((alterMemoryAccessProtection) && (oldProtectionFlags != NULL)) revertMemoryProtection(processHandle, memoryAddress, numberOfBytes, oldProtectionFlags);
		return ((resultReadMemory) && (bytesRead == numberOfBytes));
	}
	__finally
	{
		if (getMemoryAccessLock() != NULL) LeaveCriticalSection(getMemoryAccessLock());
	}
}

// write to the process memory
bool writeMemoryAddress(HANDLE processHandle, PVOID memoryAddress, unsigned long numberOfBytes, PVOID memoryBuffer, bool alterMemoryAccessProtection)
{
	__try
	{
		if (getMemoryAccessLock() != NULL) EnterCriticalSection(getMemoryAccessLock());

		unsigned long oldProtectionFlags = NULL;
		if (alterMemoryAccessProtection) oldProtectionFlags = alterMemoryProtection(processHandle, memoryAddress, numberOfBytes);
		unsigned long bytesWritten = 0;
		bool resultWriteMemory = ((WriteProcessMemory(processHandle, memoryAddress, memoryBuffer, numberOfBytes, (SIZE_T*)&bytesWritten) == 0) ? false : true);
		if ((alterMemoryAccessProtection) && (oldProtectionFlags != NULL)) revertMemoryProtection(processHandle, memoryAddress, numberOfBytes, oldProtectionFlags);
		return ((resultWriteMemory) && (bytesWritten == numberOfBytes));
	}
	__finally
	{
		if (getMemoryAccessLock() != NULL) LeaveCriticalSection(getMemoryAccessLock());
	}
}

// handles the patching of the memory addresses for a process or module
bool performMemoryAddressPatch(HANDLE processHandle, PATCHINGDETAILS_MEMORY* pMemoryDetails, PATCHINGDETAILS_FILEBASE* pFileBaseDetails)
{
	if (pMemoryDetails == NULL) return true;
	if (pMemoryDetails->numberOfAddresses <= 0) return true;

	// put the memory addresses into groups
	PVOID mpGroupAddress[MAXIMUM_MEMORYADDRESSES];
	int mpGroupLength[MAXIMUM_MEMORYADDRESSES];
	unsigned long mpGroupAddressVariation[MAXIMUM_MEMORYADDRESSES];
	int mpGroupIndex = 0;
	int mpGroupCount = 0;
	unsigned char* mpGroupPatched = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(pMemoryDetails->numberOfAddresses);
	unsigned char* mpGroupFound = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(pMemoryDetails->numberOfAddresses);
	DWORD currentAddress = 1;

	// cycle through the parsed memory addresses
	for (int i = 0; i < pMemoryDetails->numberOfAddresses; i++)
	{
		mpGroupPatched[i] = pMemoryDetails->patchedByte[i];

		// not a sequential address - start new group
		if (currentAddress != pMemoryDetails->memoryAddress[i])
		{
			mpGroupAddress[mpGroupCount] = UlongToPtr(pMemoryDetails->baseAddress + pMemoryDetails->memoryAddress[i]);
			mpGroupLength[mpGroupCount] = 1;
			mpGroupAddressVariation[mpGroupCount] = pMemoryDetails->memoryAddressVariation[i];
			mpGroupCount++;
		}
		// otherwise increase group length
		else if (mpGroupCount > 0)
		{
			mpGroupLength[mpGroupCount - 1]++;
			// store highest group variation
			if (pMemoryDetails->memoryAddressVariation[i] > mpGroupAddressVariation[mpGroupCount - 1]) mpGroupAddressVariation[mpGroupCount - 1] = pMemoryDetails->memoryAddressVariation[i];
		}

		currentAddress = pMemoryDetails->memoryAddress[i] + 1;
	}

	// display group information
	if ((ALLOW_MEMORY_INFORMATION) && (isDebugInformationEnabled()))
	{
		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		for (int i = 0; i < mpGroupCount; i++)
		{
			if (mpGroupLength[i] <= 1) sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Memory Patch %02d: 0x%X (%02d) [0x%02X]", i + 1, PtrToUlong(mpGroupAddress[i]), mpGroupLength[i], pMemoryDetails->expectedByte[mpGroupIndex]);
			else sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Memory Patch %02d: 0x%X (%02d) [0x%02X...0x%02X]", i + 1, PtrToUlong(mpGroupAddress[i]), mpGroupLength[i], pMemoryDetails->expectedByte[mpGroupIndex], pMemoryDetails->expectedByte[mpGroupIndex + mpGroupLength[i] - 1]);
			if (mpGroupAddressVariation[i] > 0) sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s (%c%d)", debugMessage, 0xF1, mpGroupAddressVariation[i]);
			displayConsoleMessage(debugMessage, true, true);
			mpGroupIndex += mpGroupLength[i];
		}
		ULib.HeapLibrary.freePrivateHeap(debugMessage);
		mpGroupIndex = 0;
	}

	int suspendThreadCount = 0;

	__try
	{
		bool memoryPatchSucceeded = false;
		bool foundMemoryMatch = false;

		// cycle through the memory address groups, performing a memory patch
		for (int i = 0; i < mpGroupCount; i++)
		{
			memoryPatchSucceeded = false;

			// while patching has failed
			while (!memoryPatchSucceeded)
			{
				// suspend the primary thread of the main process before patching its memory
				if ((getMainProcessThreadHandle() != NULL) && (suspendThreadCount == 0)) suspendThreadCount = SuspendThread(getMainProcessThreadHandle());

				// check if shuting down
				if (isProcessPatcherShutingDown()) return false;

				// has the main process terminated
				if (hasProcessTerminated(processHandle))
				{
					// if there is a retry count return to calling function
					if (pMemoryDetails->retryCount > 0) return false;
					// otherwise, report error
					displayErrorMessage(ERROR_PATCHING_ENGINE_PROCESSTERMINATED, true);
				}

				bool doMemoryVariation = (mpGroupAddressVariation[i] != 0);
				bool doingMemoryVariationBefore = false;
				bool doingMemoryVariationAfter = false;
				PVOID currentMemoryAddress = mpGroupAddress[i];
				unsigned long startMemoryAddress = PtrToUlong(mpGroupAddress[i]);

				do
				{
					// alter memory protection (for Windows NTx)
					unsigned long oldProtectionFlags = alterMemoryProtection(processHandle, currentMemoryAddress, mpGroupLength[i]);
					foundMemoryMatch = false;

					// read the process memory
					if (readMemoryAddress(processHandle, currentMemoryAddress, mpGroupLength[i], mpGroupFound, false))
					{
						foundMemoryMatch = true;

						// cycle through the memory bytes looking for a match
						for (int j = 0; j < mpGroupLength[i]; j++)
						{
							// only if not allowing any expected byte
							if ((pMemoryDetails->memoryByteOperator[mpGroupIndex + j] & MEMORYOPERATOR_EXPECTEDANY) == 0)
							{
								int byteOperator = pMemoryDetails->memoryByteOperator[mpGroupIndex + j] & 0xFFF0;
								unsigned char expectedCharacter = pMemoryDetails->expectedByte[mpGroupIndex + j];

								// comparision = or ==
								if (byteOperator == MEMORYOPERATOR_COMPEQ) foundMemoryMatch = (mpGroupFound[j] == expectedCharacter);
								// comparision <> or !=
								else if (byteOperator == MEMORYOPERATOR_COMPNOT) foundMemoryMatch = (mpGroupFound[j] != expectedCharacter);
								// comparision >=
								else if (byteOperator == MEMORYOPERATOR_COMPGTEQ) foundMemoryMatch = (mpGroupFound[j] >= expectedCharacter);
								// comparision <=
								else if (byteOperator == MEMORYOPERATOR_COMPLTEQ) foundMemoryMatch = (mpGroupFound[j] <= expectedCharacter);
								// comparision >
								else if (byteOperator == MEMORYOPERATOR_COMPGT) foundMemoryMatch = (mpGroupFound[j] > expectedCharacter);
								// comparision <
								else if (byteOperator == MEMORYOPERATOR_COMPLT) foundMemoryMatch = (mpGroupFound[j] < expectedCharacter);
								// comparision >< (between byte1 and byte2)
								else if (byteOperator == MEMORYOPERATOR_COMPBETWEEN) foundMemoryMatch = (mpGroupFound[j] >= expectedCharacter) && (mpGroupFound[j] <= pMemoryDetails->expectedByte2[mpGroupIndex + j]);
								// otherwise, do equal comparision
								else foundMemoryMatch = (mpGroupFound[j] == expectedCharacter);
							}

							// break on failure
							if (!foundMemoryMatch) break;

							// no patch required, just copy the found byte
							if ((pMemoryDetails->memoryByteOperator[mpGroupIndex + j] & MEMORYOPERATOR_EXPECTEDPATCH) == MEMORYOPERATOR_EXPECTEDPATCH) ((unsigned char*)&mpGroupPatched[mpGroupIndex])[j] = mpGroupFound[j];
						}
					}

					// wait until expected bytes are found
					if (foundMemoryMatch)
					{
						// write the patched bytes to the memory
						if (writeMemoryAddress(processHandle, currentMemoryAddress, mpGroupLength[i], (unsigned char*)&mpGroupPatched[mpGroupIndex], false))
						{
							memoryPatchSucceeded = true;
							mpGroupIndex += mpGroupLength[i];
						}
					}

					// revert memory protection back to previous settings (if required)
					if (oldProtectionFlags != NULL) revertMemoryProtection(processHandle, currentMemoryAddress, mpGroupLength[i], oldProtectionFlags);

					// if successfully patched memory address
					if (memoryPatchSucceeded) break;

					// if the first byte is found and patched (without being written successfully)
					if (((pMemoryDetails->memoryByteOperator[mpGroupIndex] & MEMORYOPERATOR_EXPECTEDANY) == 0) &&
						((pMemoryDetails->memoryByteOperator[mpGroupIndex] & MEMORYOPERATOR_EXPECTEDPATCH) == 0) &&
						(pMemoryDetails->expectedByte[mpGroupIndex] == mpGroupFound[0])) return false;

					// doing memory variation (search before and after the set memory address)
					if (doMemoryVariation)
					{
						// start searching before
						if ((!doingMemoryVariationBefore) && (!doingMemoryVariationAfter)) doingMemoryVariationBefore = true;

						// searching before
						if (doingMemoryVariationBefore)
						{
							unsigned long nextMemoryAddress = PtrToUlong(currentMemoryAddress) - 1;
							currentMemoryAddress = UlongToPtr(nextMemoryAddress);
							// finished searching before, search after
							if ((nextMemoryAddress <= 0) || (nextMemoryAddress < (startMemoryAddress - mpGroupAddressVariation[i])))
							{
								doingMemoryVariationBefore = false;
								doingMemoryVariationAfter = true;
								currentMemoryAddress = mpGroupAddress[i];
							}
						}
						// searching after
						if (doingMemoryVariationAfter)
						{
							unsigned long nextMemoryAddress = PtrToUlong(currentMemoryAddress) + 1;
							currentMemoryAddress = UlongToPtr(nextMemoryAddress);
							// finished searching after, abort search
							if (nextMemoryAddress > (startMemoryAddress + mpGroupAddressVariation[i]))
							{
								doingMemoryVariationBefore = false;
								doingMemoryVariationAfter = false;
								doMemoryVariation = false;
							}
						}
					}

				} while (doMemoryVariation);

				// if successfully patched memory address
				if (memoryPatchSucceeded) break;

				// check to see if the module still exists
				if ((pFileBaseDetails->sectionType == SECTIONTYPE_MODULE) && (!doesModuleExistInProcessMemory(getMainProcessId(), pFileBaseDetails))) return false;

				// synchronise the suspend and resume counts of the main process thread
				resumeThread(getMainProcessThreadHandle());

				// delay re-attempt (on failure)
				Sleep(PATCHINGENGINE_SLEEPDELAY);
			}
		}
	}
	__finally
	{
		ULib.HeapLibrary.freePrivateHeap(mpGroupPatched);
		ULib.HeapLibrary.freePrivateHeap(mpGroupFound);

		// synchronise the suspend and resume counts of the main process thread
		resumeThread(getMainProcessThreadHandle());
	}

	return true;
}