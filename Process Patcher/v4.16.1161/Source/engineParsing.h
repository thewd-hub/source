/////////////////////////////////////////////////////////////////////
// Parses the script file and stores the information in the        //
// corresponding PATCHINGDETAILS structures. Decrypts if necessary //
/////////////////////////////////////////////////////////////////////

// script header - decrypted
#define SCRIPTHEADER_DECID					"#Process Patcher Configuration File"
// script header - encrypted
#define SCRIPTHEADER_ENCID					"Process Patcher\x1A"
// default encryption/decryption key
#define SCRIPT_DEFAULTKEY					"kf$fd3rL:56"

#define PARSINGENGINE_PARSINGOK				0x00000000
#define PARSINGENGINE_PARSINGERROR			0x00000001
#define PARSINGENGINE_PARSINGNOTFOUND		0x00000002

// maximum number of sections allowed
#define MAXIMUM_TARGETPROCESSES				1
#define MAXIMUM_CHILDPROCESSES				1
#define MAXIMUM_MODULES						5
#define MAXIMUM_PLUGINS						5
#define MAXIMUM_REGISTRYENTRIES				5
#define MAXIMUM_SECTIONS					MAXIMUM_TARGETPROCESSES + \
											MAXIMUM_CHILDPROCESSES + \
											MAXIMUM_MODULES + \
											MAXIMUM_PLUGINS + \
											MAXIMUM_REGISTRYENTRIES

// the different types of sections
#define SECTIONTYPE_UNDEFINED				0x00000000
#define SECTIONTYPE_TARGETPROCESS			0x00000001
#define SECTIONTYPE_CHILDPROCESS			0x00000002
#define SECTIONTYPE_MODULE					0x00000003
#define SECTIONTYPE_PLUGIN					0x00000004
#define SECTIONTYPE_REGISTRYENTRY			0x00000005

// maximum properties for memory and registry actions
#define MAXIMUM_MEMORYADDRESSBYTES			128
#define MAXIMUM_MEMORYADDRESSES				4096
#define MAXIMUM_REGISTRYACTIONS				4096

// memory byte operators
#define MEMORYOPERATOR_EXPECTEDANY			0x00000001 // can be any byte (don't care)
#define MEMORYOPERATOR_EXPECTEDPATCH		0x00000002 // no memory patch (keep as expected byte)
#define MEMORYOPERATOR_COMPEQ				0x00000010 // comparision = or ==
#define MEMORYOPERATOR_COMPNOT	 			0x00000020 // comparision <> or !=
#define MEMORYOPERATOR_COMPGT	 			0x00000030 // comparision >
#define MEMORYOPERATOR_COMPGTEQ	 			0x00000040 // comparision >=
#define MEMORYOPERATOR_COMPLT	 			0x00000050 // comparision <
#define MEMORYOPERATOR_COMPLTEQ	 			0x00000060 // comparision <=
#define MEMORYOPERATOR_COMPBETWEEN 			0x00000070 // comparision >< (between byte1 and byte2)

// operating system platforms
#define OS_UNDEFINED						0x00000000
#define OS_WINDOWS_95						0x00000001
#define OS_WINDOWS_98						0x00000002
#define OS_WINDOWS_ME						0x00000004
#define OS_WINDOWS_NT4						0x00000008
#define OS_WINDOWS_2000						0x00000010
#define OS_WINDOWS_XP						0x00000020
#define OS_WINDOWS_SERVER2003				0x00000040
#define OS_WINDOWS_VISTA					0x00000080
#define OS_WINDOWS_SERVER2008				0x00000100
#define OS_WINDOWS_7						0x00000200
#define OS_WINDOWS_SERVER2008R2				0x00000400

#define BLOCKEXECUTION_UNDEFINED			0x00000000
#define BLOCKEXECUTION_64BIT				0x00000001
#define BLOCKEXECUTION_WOW64				0x00000002

// base class for all PATCHINGDETAILS
class PATCHINGDETAILS_BASE
{
	public:
		int sectionType;
		int sectionNumber;
		bool ignoreParsingErrors;
		bool isDisabled;
};

// base class for file-based PATCHINGDETAILS structures
class PATCHINGDETAILS_FILEBASE:
public PATCHINGDETAILS_BASE
{
	public:
		char* fileName;
		unsigned long fileSize;
		unsigned long fileSizeUpperLimit;
		bool isPatternMatchingFilename;
};

// base class for all memory-based PATCHINGDETAILS structures
class PATCHINGDETAILS_MEMORY
{
	public:
		int numberOfAddresses;
		unsigned long baseAddress;
		unsigned long memoryAddress[MAXIMUM_MEMORYADDRESSES];
		unsigned long memoryAddressVariation[MAXIMUM_MEMORYADDRESSES];
		unsigned char expectedByte[MAXIMUM_MEMORYADDRESSES];
		unsigned char expectedByte2[MAXIMUM_MEMORYADDRESSES];
		unsigned char patchedByte[MAXIMUM_MEMORYADDRESSES];
		int memoryByteOperator[MAXIMUM_MEMORYADDRESSES];
		int retryCount;
};

// patching details for the target process
class PATCHINGDETAILS_TARGETPROCESS:
public PATCHINGDETAILS_FILEBASE,
public PATCHINGDETAILS_MEMORY
{
	public:
		char* displayName;
		char* argumentsToForward;

		bool openExistingProcess;
		bool createsChildProcess;
		bool createSuspendedProcess;
		bool displayUserNotifyBox;
		char* displayUserNotifyBoxMessage;
		char* waitForLocalWindowName;
		char* waitForGlobalWindowName;
		unsigned long waitForTimeDelay;
};

// patching details for child process types
class PATCHINGDETAILS_CHILDPROCESS:
public PATCHINGDETAILS_FILEBASE
{
};

// patching details for module types
class PATCHINGDETAILS_MODULE:
public PATCHINGDETAILS_FILEBASE,
public PATCHINGDETAILS_MEMORY
{
};

// patching details for plugin types
class PATCHINGDETAILS_PLUGIN:
public PATCHINGDETAILS_FILEBASE
{
	public:
		char* argumentsToForward;
		float minimumPluginVersionRequired;
};

// patching details for registry entry types
class PATCHINGDETAILS_REGISTRYENTRY:
public PATCHINGDETAILS_BASE
{
	public:
		int numberOfRegistryActions;
		bool use32BitView;
		bool use64BitView;
		int registryAction[MAXIMUM_REGISTRYACTIONS];
		HKEY registryHive[MAXIMUM_REGISTRYACTIONS];
		char* registrySubKey[MAXIMUM_REGISTRYACTIONS];
		char* registryValueName[MAXIMUM_REGISTRYACTIONS];
		unsigned char* registryValueData[MAXIMUM_REGISTRYACTIONS];
		int registryValueDataLength[MAXIMUM_REGISTRYACTIONS];
		int registryValueType[MAXIMUM_REGISTRYACTIONS];
};

// global patching details
class PATCHINGDETAILS_GLOBAL
{
	public:
		char* scriptFilename;

		char* patchInformation;
		char* patchAuthor;
		char* patchContactInformation;

		float minimumProcessPatcherVersionRequired;
		unsigned long supportedOperatingSystemPlatforms;
		bool enableStealthMode;

		int numberOfTargetProcesses;
		int numberOfChildProcesses;
		int numberOfModules;
		int numberOfPlugins;
		int numberOfRegistryEntries;

		bool ignoreSessionId;
		unsigned long blockExecution;
};

bool isScriptEncrypted(void);

PATCHINGDETAILS_GLOBAL* getGlobalPatchingDetails(void);
PATCHINGDETAILS_TARGETPROCESS* getTargetProcessPatchingDetails(void);
PATCHINGDETAILS_CHILDPROCESS* getChildProcessPatchingDetails(void);
PATCHINGDETAILS_MODULE* getModulePatchingDetails(int indexNumber);
PATCHINGDETAILS_PLUGIN* getPluginPatchingDetails(int indexNumber);
PATCHINGDETAILS_REGISTRYENTRY* getRegistryEntryPatchingDetails(int indexNumber);

int parseStringParameter(char* parameterName, char* parameterTargetString, char* targetString, char* stringBuffer, bool allowBlankValues = false);
int parseStringParameter(char* parameterName, unsigned long parameterTargetLong, char* targetString, char* stringBuffer);

bool setParsingErrorMessage(char* errorMessage);
char* getParsingErrorMessage(void);

bool getFilenameUsingExplorer(char* titleString, char* filterString, char* extensionString, char* selectedFilename);
bool parseScriptFile(char* scriptFilename);
void cleanupParsedEntries(void);