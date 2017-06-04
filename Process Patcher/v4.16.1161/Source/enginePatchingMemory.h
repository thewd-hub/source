///////////////////////////////////////////////////////////
// Handles the reading and writing of the process memory //
///////////////////////////////////////////////////////////

bool readMemoryAddress(HANDLE processHandle, PVOID memoryAddress, unsigned long numberOfBytes, PVOID memoryBuffer, bool alterMemoryAccessProtection);
bool writeMemoryAddress(HANDLE processHandle, PVOID memoryAddress, unsigned long numberOfBytes, PVOID memoryBuffer, bool alterMemoryAccessProtection);

bool performMemoryAddressPatch(HANDLE processHandle, PATCHINGDETAILS_MEMORY* pMemoryDetails, PATCHINGDETAILS_FILEBASE* pFileBaseDetails);