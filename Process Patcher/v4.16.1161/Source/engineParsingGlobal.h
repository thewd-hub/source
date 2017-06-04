///////////////////////////////////////////////////////////////////////////////////////
// Parses the parameters that are not associated with a particular section but are   //
// Global parameters. Stores the information in the PATCHINGDETAILS_GLOBAL structure //
///////////////////////////////////////////////////////////////////////////////////////

bool parseGlobalParameters(char* stringBuffer, PATCHINGDETAILS_GLOBAL* pGlobalDetails);
bool checkGlobalParameters(PATCHINGDETAILS_GLOBAL* pGlobalDetails);
bool checkOperatingSystemPlatform(PATCHINGDETAILS_GLOBAL* pGlobalDetails);