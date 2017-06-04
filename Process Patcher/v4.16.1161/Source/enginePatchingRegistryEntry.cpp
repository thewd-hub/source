//////////////////////////////////////////////
// Handles the patching of registry entries //
//////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingRegistryEntry.h"
#include "enginePatching.h"
#include "enginePatchingRegistryEntry.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

class PATCHINGENGINE_REGISTRYENTRY
{
	public:
		bool isDisplayDebugInformationEnabled;
};

PATCHINGENGINE_REGISTRYENTRY pRegistryEntry;

// initialises the registry entry patching structures before calling the execution thread
bool initialiseRegistryEntryPatching(void)
{
	// is debug information enabled
	pRegistryEntry.isDisplayDebugInformationEnabled = ((ALLOW_DEBUG_INFORMATION) && (isDebugInformationEnabled()));
	return true;
}

// uninitialise the registry entry patching
bool uninitialiseRegistryEntryPatching(void)
{
	return true;
}

// gets the correct mapping for the registry hive
HKEY getRegistryHive(HKEY regHive)
{
	return (((regHive == HKEY_CLASSES_ROOT) && (ULib.OperatingSystemLibrary.isWindows2000(true))) ? HKEY_CURRENT_USER : regHive);
}

// gets the correct mapping for the registry sub key
char* getRegistrySubKey(HKEY regHive, char* regSubKey)
{
	char* fullRegSubKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	// Windows 2000+ mapping of HKEY_CLASSES_ROOT is different from previous versions.
	if ((regHive == HKEY_CLASSES_ROOT) && (ULib.OperatingSystemLibrary.isWindows2000(true))) sprintf_s(fullRegSubKey, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "%s\\%s", "Software\\Classes", regSubKey);
	else ULib.StringLibrary.copyString(fullRegSubKey, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, regSubKey);
	// remove the final slash
	int fullRegSubKeyLength = ULib.StringLibrary.getStringLength(fullRegSubKey);
	if ((fullRegSubKeyLength > 0) && (fullRegSubKey[fullRegSubKeyLength - 1] == '\\')) fullRegSubKey[fullRegSubKeyLength - 1] = NULL;
	return fullRegSubKey;
}

// creates the registry key specified in the patching details
bool registryActionCreateKey(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails, int registryEntryIndex)
{
	if ((pRegistryEntryDetails == NULL) || (pRegistryEntryDetails->registryAction[registryEntryIndex] != REGISTRYENTRY_ACTION_CREATEKEY)) return false;

	// create registry key
	char* regSubKey = getRegistrySubKey(pRegistryEntryDetails->registryHive[registryEntryIndex], pRegistryEntryDetails->registrySubKey[registryEntryIndex]);
	unsigned long accessMask = KEY_ALL_ACCESS | ((pRegistryEntryDetails->use64BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS64BITKEY : ((pRegistryEntryDetails->use32BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS32BITKEY : 0));
	HKEY hKey = ULib.RegistryLibrary.createRegistrySubKey(getRegistryHive(pRegistryEntryDetails->registryHive[registryEntryIndex]), regSubKey, accessMask);
	if (hKey == NULL)
	{
		ULib.HeapLibrary.freePrivateHeap(regSubKey);
		displayErrorMessage(ERROR_PATCHING_REGISTRY_CREATINGKEY, true);
	}

	ULib.HeapLibrary.freePrivateHeap(regSubKey);
	RegCloseKey(hKey);
	return true;
}

// deletes the registry key specified in the patching details
bool registryActionDeleteKey(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails, int registryEntryIndex)
{
	if ((pRegistryEntryDetails == NULL) || (pRegistryEntryDetails->registryAction[registryEntryIndex] != REGISTRYENTRY_ACTION_DELETEKEY)) return false;

	// delete registry key
	char* regSubKey = getRegistrySubKey(pRegistryEntryDetails->registryHive[registryEntryIndex], pRegistryEntryDetails->registrySubKey[registryEntryIndex]);
	unsigned long accessMask = ((pRegistryEntryDetails->use64BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS64BITKEY : ((pRegistryEntryDetails->use32BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS32BITKEY : 0));
	ULib.RegistryLibrary.deleteRegistrySubKey(getRegistryHive(pRegistryEntryDetails->registryHive[registryEntryIndex]), NULL, regSubKey, accessMask);
	ULib.HeapLibrary.freePrivateHeap(regSubKey);
	return true;
}

// sets the registry value specified in the patching details
bool registryActionSetValue(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails, int registryEntryIndex)
{
	if ((pRegistryEntryDetails == NULL) || (pRegistryEntryDetails->registryAction[registryEntryIndex] != REGISTRYENTRY_ACTION_SETVALUE)) return false;

	// create/open registry key
	char* regSubKey = getRegistrySubKey(pRegistryEntryDetails->registryHive[registryEntryIndex], pRegistryEntryDetails->registrySubKey[registryEntryIndex]);
	unsigned long accessMask = KEY_ALL_ACCESS | ((pRegistryEntryDetails->use64BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS64BITKEY : ((pRegistryEntryDetails->use32BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS32BITKEY : 0));
	HKEY hKey = ULib.RegistryLibrary.createRegistrySubKey(getRegistryHive(pRegistryEntryDetails->registryHive[registryEntryIndex]), regSubKey, accessMask);
	if (hKey == NULL)
	{
		accessMask = KEY_SET_VALUE | ((pRegistryEntryDetails->use64BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS64BITKEY : ((pRegistryEntryDetails->use32BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS32BITKEY : 0));
		hKey = ULib.RegistryLibrary.openRegistrySubKey(getRegistryHive(pRegistryEntryDetails->registryHive[registryEntryIndex]), regSubKey, accessMask);
		if (hKey == NULL)
		{
			ULib.HeapLibrary.freePrivateHeap(regSubKey);
			displayErrorMessage(ERROR_PATCHING_REGISTRY_CREATINGKEY, true);
		}
	}

	ULib.HeapLibrary.freePrivateHeap(regSubKey);

	if (RegSetValueEx(hKey, pRegistryEntryDetails->registryValueName[registryEntryIndex], 0,
		pRegistryEntryDetails->registryValueType[registryEntryIndex],
		pRegistryEntryDetails->registryValueData[registryEntryIndex],
		pRegistryEntryDetails->registryValueDataLength[registryEntryIndex]) != ERROR_SUCCESS)
	{
		displayErrorMessage(ERROR_PATCHING_REGISTRY_SETTINGVALUE, true);
	}

	RegCloseKey(hKey);
	return true;
}

// delete the registry value specified in the patching details
bool registryActionDeleteValue(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails, int registryEntryIndex)
{
	if ((pRegistryEntryDetails == NULL) || (pRegistryEntryDetails->registryAction[registryEntryIndex] != REGISTRYENTRY_ACTION_DELETEVALUE)) return false;

	// open existing registry key
	char* regSubKey = getRegistrySubKey(pRegistryEntryDetails->registryHive[registryEntryIndex], pRegistryEntryDetails->registrySubKey[registryEntryIndex]);
	unsigned long accessMask = KEY_SET_VALUE | ((pRegistryEntryDetails->use64BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS64BITKEY : ((pRegistryEntryDetails->use32BitView) ? ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS32BITKEY : 0));
	HKEY hKey = ULib.RegistryLibrary.openRegistrySubKey(getRegistryHive(pRegistryEntryDetails->registryHive[registryEntryIndex]), regSubKey, accessMask);
	if (hKey == NULL)
	{
		ULib.HeapLibrary.freePrivateHeap(regSubKey);
		return false;
	}

	ULib.HeapLibrary.freePrivateHeap(regSubKey);
	RegDeleteValue(hKey, pRegistryEntryDetails->registryValueName[registryEntryIndex]);
	RegCloseKey(hKey);
	return true;
}

// executes the patching of a registry entry
// (Argument - PATCHINGDETAILS_REGISTRYENTRY)
void executeRegistryEntryPatchingThread(LPVOID args)
{
	__try
	{
		PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails = (PATCHINGDETAILS_REGISTRYENTRY*)args;

		char* debugMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();

		// wait for the registry entry patching waiting event.
		// these events are originally signalled, unless reset by a plugin
		if (pRegistryEntryDetails->sectionNumber == 1) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_READY, false);
		else if (pRegistryEntryDetails->sectionNumber == 2) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_READY, false);
		else if (pRegistryEntryDetails->sectionNumber == 3) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_READY, false);
		else if (pRegistryEntryDetails->sectionNumber == 4) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_READY, false);
		else if (pRegistryEntryDetails->sectionNumber == 5) waitForPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_READY, false);

		// display debug information
		if (pRegistryEntry.isDisplayDebugInformationEnabled)
		{
			sprintf_s(debugMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Executing Registry Section %d", pRegistryEntryDetails->sectionNumber);
			displayConsoleMessage(debugMessage, true, true);
		}

		// cycle through the registry entries calling the appropriate action functions
		for (int registryEntryIndex = 0; registryEntryIndex < pRegistryEntryDetails->numberOfRegistryActions; registryEntryIndex++)
		{
			if (pRegistryEntryDetails->registryAction[registryEntryIndex] == REGISTRYENTRY_ACTION_CREATEKEY) registryActionCreateKey(pRegistryEntryDetails, registryEntryIndex);
			else if (pRegistryEntryDetails->registryAction[registryEntryIndex] == REGISTRYENTRY_ACTION_DELETEKEY) registryActionDeleteKey(pRegistryEntryDetails, registryEntryIndex);
			else if (pRegistryEntryDetails->registryAction[registryEntryIndex] == REGISTRYENTRY_ACTION_SETVALUE) registryActionSetValue(pRegistryEntryDetails, registryEntryIndex);
			else if (pRegistryEntryDetails->registryAction[registryEntryIndex] == REGISTRYENTRY_ACTION_DELETEVALUE) registryActionDeleteValue(pRegistryEntryDetails, registryEntryIndex);
		}

		ULib.HeapLibrary.freePrivateHeap(debugMessage);

		// signal waiting events
		if (pRegistryEntryDetails->sectionNumber == 1) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY1_PATCHING_FINISHED, true);
		else if (pRegistryEntryDetails->sectionNumber == 2) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY2_PATCHING_FINISHED, true);
		else if (pRegistryEntryDetails->sectionNumber == 3) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY3_PATCHING_FINISHED, true);
		else if (pRegistryEntryDetails->sectionNumber == 4) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY4_PATCHING_FINISHED, true);
		else if (pRegistryEntryDetails->sectionNumber == 5) setPatchingEngineWaitingEvent(EVENT_PATCHINGENGINE_REGISTRY5_PATCHING_FINISHED, true);
	}
	__except(handleCriticalException(GetExceptionInformation(), SECTIONTYPE_REGISTRYENTRY))
	{
	}

	ExitThread(ERROR_SUCCESS);
}