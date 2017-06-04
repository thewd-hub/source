///////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for all patching types.          //
// Stores the information in the corresponding PATCHINGDETAILS structure //
///////////////////////////////////////////////////////////////////////////

bool parseBaseParameters(char* stringBuffer, PATCHINGDETAILS_BASE* pBaseDetails);
bool checkBaseParameters(PATCHINGDETAILS_BASE* pBaseDetails);