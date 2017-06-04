///////////////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for all patching types that patches the  //
// memory. Stores the information in the corresponding PATCHINGDETAILS structure //
///////////////////////////////////////////////////////////////////////////////////

bool parseMemoryParameters(char* stringBuffer, PATCHINGDETAILS_MEMORY* pMemoryDetails);
bool checkMemoryParameters(PATCHINGDETAILS_MEMORY* pMemoryDetails, PATCHINGDETAILS_BASE* pBaseDetails);