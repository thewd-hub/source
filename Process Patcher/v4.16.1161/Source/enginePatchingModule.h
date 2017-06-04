////////////////////////////////////////////////////////////////////////
// Handles the patching of modules which are used by the main process //
////////////////////////////////////////////////////////////////////////

bool doesModuleExistInProcessMemory(unsigned long processId, PATCHINGDETAILS_FILEBASE* pFileBaseDetails);

bool initialiseModulePatching(void);
bool uninitialiseModulePatching(void);

void executeModulePatchingThread(LPVOID args);