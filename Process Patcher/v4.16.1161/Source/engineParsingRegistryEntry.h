///////////////////////////////////////////////////////////////////////
// Parses the parameters that are valid for registry entries. Stores //
// the information in the PATCHINGDETAILS_REGISTRYENTRY structure    //
///////////////////////////////////////////////////////////////////////

#define REGISTRYENTRY_ACTION_UNDEFINED		0x00000000
#define REGISTRYENTRY_ACTION_CREATEKEY		0x00000001
#define REGISTRYENTRY_ACTION_DELETEKEY		0x00000002
#define REGISTRYENTRY_ACTION_DELETEVALUE	0x00000003
#define REGISTRYENTRY_ACTION_SETVALUE		0x00000004

#define REGISTRYENTRY_HIVE_UNDEFINED		0x00000000

#define REGISTRYENTRY_TYPE_UNDEFINED		0x00000000

bool parseRegistryEntryParameters(char* stringBuffer, PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails);
bool checkRegistryEntryParameters(PATCHINGDETAILS_REGISTRYENTRY* pRegistryEntryDetails);