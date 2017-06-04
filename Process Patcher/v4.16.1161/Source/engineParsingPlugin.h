////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for supported plugins.    //
// Stores the information in the PATCHINGDETAILS_PLUGIN structure //
////////////////////////////////////////////////////////////////////

bool parsePluginParameters(char* stringBuffer, PATCHINGDETAILS_PLUGIN* pPluginDetails);
bool checkPluginParameters(PATCHINGDETAILS_PLUGIN* pPluginDetails);