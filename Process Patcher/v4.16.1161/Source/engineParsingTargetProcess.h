/////////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for the target process. Stores //
// the information in the PATCHINGDETAILS_TARGETPROCESS structure      //
/////////////////////////////////////////////////////////////////////////

bool parseTargetProcessParameters(char* stringBuffer, PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails);
bool checkTargetProcessParameters(PATCHINGDETAILS_TARGETPROCESS* pTargetProcessDetails);