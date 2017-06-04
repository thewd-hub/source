//////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for child process type patches. //
// Stores the information in the PATCHINGDETAILS_CHILDPROCESS structure //
//////////////////////////////////////////////////////////////////////////

bool parseChildProcessParameters(char* stringBuffer, PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails);
bool checkChildProcessParameters(PATCHINGDETAILS_CHILDPROCESS* pChildProcessDetails);