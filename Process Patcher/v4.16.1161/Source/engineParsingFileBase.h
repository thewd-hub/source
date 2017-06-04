///////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for all file patching types.     //
// Stores the information in the corresponding PATCHINGDETAILS structure //
///////////////////////////////////////////////////////////////////////////

bool parseFileBaseParameters(char* stringBuffer, PATCHINGDETAILS_FILEBASE* pFileBaseDetails);
bool checkFileBaseParameters(PATCHINGDETAILS_FILEBASE* pFileBaseDetails);