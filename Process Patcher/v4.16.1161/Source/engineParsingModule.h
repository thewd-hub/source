////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for module type patches.  //
// Stores the information in the PATCHINGDETAILS_MODULE structure //
////////////////////////////////////////////////////////////////////

bool parseModuleParameters(char* stringBuffer, PATCHINGDETAILS_MODULE* pModuleDetails);
bool checkModuleParameters(PATCHINGDETAILS_MODULE* pModuleDetails);