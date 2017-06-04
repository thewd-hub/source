///////////////////////////////////////////////////////////
// Utilities Library (C++)                               //
///////////////////////////////////////////////////////////

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef __STDC_WANT_SECURE_LIB__
#pragma message("*** using secure wrapper functions ***")
////////////////////////////////////////////
// wrapper functions for secure functions //
////////////////////////////////////////////
__inline char* strcpy_s(char* stringBufferTarget, int stringBufferTargetSize, char* stringBufferSource)
{
	stringBufferTargetSize;
	return strcpy(stringBufferTarget, stringBufferSource);
}

__inline char* strncpy_s(char* stringBufferTarget, int stringBufferTargetSize, char* stringBufferSource, int numberOfCharacters)
{
	stringBufferTargetSize;
	return strncpy(stringBufferTarget, stringBufferSource, numberOfCharacters);
}

__inline void* memcpy_s(void* stringBufferTarget, int stringBufferTargetSize, void* stringBufferSource, int numberOfBytes)
{
	stringBufferTargetSize;
	return memcpy(stringBufferTarget, stringBufferSource, numberOfBytes);
}

__inline char* strcat_s(char* stringBufferTarget, int stringBufferTargetSize, char* stringBufferSource)
{
	stringBufferTargetSize;
	return strcat(stringBufferTarget, stringBufferSource);
}

__inline char* strncat_s(char* stringBufferTarget, int stringBufferTargetSize, char* stringBufferSource, int numberOfCharacters)
{
	stringBufferTargetSize;
	return strncat(stringBufferTarget, stringBufferSource, numberOfCharacters);
}

__inline char* strtok_s(char* strToken, char* strDelimiter, char** tokenContext)
{
	tokenContext;
	return strtok(strToken, strDelimiter);
}

__inline int sprintf_s(char* stringBufferTarget, int stringBufferTargetSize, char* stringFormat, ...)
{
	stringBufferTargetSize;
	va_list args;
	va_start(args, stringFormat);
	int retValue = vsprintf(stringBufferTarget, stringFormat, args);
	va_end(args);
	return retValue;
}

#define sscanf_s sscanf
#endif

/////////////////////////////////////////
// utilities library class declaration //
/////////////////////////////////////////
class CUtilitiesLibrary
{
	public:
		CUtilitiesLibrary(void);

		////////////////////////////
		// heap memory management //
		////////////////////////////
		class CHeapLibrary
		{
			public:
				const static int DEFAULT_ALLOC_SIZE = 1024; // 1KB

				PVOID allocPrivateHeap(ULONG_PTR allocSize = DEFAULT_ALLOC_SIZE);
				PVOID reallocPrivateHeap(PVOID pMem, ULONG_PTR reallocSize);
				VOID freePrivateHeap(PVOID pMem);

		} HeapLibrary;

		/////////////////////////////////////
		// string and conversion functions //
		/////////////////////////////////////
		class CStringLibrary
		{
			public:
				int getStringLength(char* stringBuffer);
				bool isString(char* stringBuffer, int minimumLength = -1);
				bool isStringMatch(char* stringBuffer1, char* stringBuffer2, bool caseSensitive = false, int numberOfCharacters = -1);
				bool isStringPatternMatch(char* stringBuffer, char* stringPattern);
				bool isBufferMatch(unsigned char* buffer1, unsigned char* buffer2, int numberOfBytes);
				bool doesStringContainPatternMatchingSymbols(char* stringBuffer);

				bool copyString(char* stringBufferTarget, int stringBufferTargetSize, char* stringBufferSource, int numberOfCharacters = -1);
				bool copyString(char* stringBufferTarget, char* stringBufferSource, int numberOfCharacters = -1);
				bool appendString(char* stringBufferTarget, int stringBufferTargetSize, char* stringBufferSource, int numberOfCharacters = -1);
				bool appendString(char* stringBufferTarget, char* stringBufferSource, int numberOfCharacters = -1);
				bool copyBuffer(unsigned char* bufferTarget, int bufferTargetSize, unsigned char* bufferSource, int numberOfBytes);
				bool copyBuffer(unsigned char* bufferTarget, unsigned char* bufferSource, int numberOfBytes);

				wchar_t* convertAnsiToUnicode(char* pMessageAnsi, int stringLength = -1);
				char* convertUnicodeToAnsi(wchar_t* pMessageUnicode, int stringLength = -1);

				ULONG32 toBigEndian(ULONG32 longValue);
				unsigned long toBigEndian(unsigned long longValue);
				unsigned short toBigEndian(unsigned short shortValue);
				ULONG_PTR getULONGFromBuffer(unsigned char* buffer, int bufferOffset = 0, bool isBigEndian = false);
				unsigned long getDWORDFromBuffer(unsigned char* buffer, int bufferOffset = 0, bool isBigEndian = false);
				unsigned short getWORDFromBuffer(unsigned char* buffer, int bufferOffset = 0, bool isBigEndian = false);
				void addULONGToBuffer(unsigned char* buffer, ULONG_PTR longValue, int bufferOffset, bool isBigEndian);
				void addDWORDToBuffer(unsigned char* buffer, unsigned long longValue, int bufferOffset = 0, bool isBigEndian = false);
				void addWORDToBuffer(unsigned char* buffer, unsigned short shortValue, int bufferOffset = 0, bool isBigEndian = false);
				void addBYTEToBuffer(unsigned char* buffer, char highByte, char lowByte, int bufferOffset = 0);

				bool hasStringHexadecimalPrefix(char* stringBuffer);
				char* getStringFromNumber(unsigned long numberValue, bool asHexadecimal = false);
				char* getStringFromULONG(ULONG_PTR ulongValue, bool asHexadecimal = false);
				char* getStringFromDWORD(unsigned long dwordValue, bool asHexadecimal = false);
				char* getStringFromWORD(unsigned short wordValue, bool asHexadecimal = false);
				char* getStringFromBYTE(unsigned char byteChar, bool asHexadecimal = false);
				char* getStringFromFloat(float floatValue);
				ULONG_PTR getULONGFromString(char* stringBuffer, bool forceHexadecimal = false);
				unsigned long getDWORDFromString(char* stringBuffer, bool forceHexadecimal = false);
				unsigned short getWORDFromString(char* stringBuffer, bool forceHexadecimal = false, unsigned long forceLength = 0);
				unsigned char getBYTEFromString(char* stringBuffer, bool forceHexadecimal = false, unsigned long forceLength = 0);
				double getDoubleFromString(char* stringBuffer);
				float getFloatFromString(char* stringBuffer);

				void getStringElementsFromString(char* stringBuffer, char* stringElements[], int* numberOfStringElements, int maximumNumberOfStringElements, char stringDelimiter, char* stringDelimiterString = NULL);
				char* getNextTokenString(char* tokenString, char charSeparator);
				char* getString(char theCharacter, int strLength);

				char* trimStringSpaces(char* stringBuffer, bool leftTrim, bool rightTrim, bool trimInPlace = false);
				char* removeSpacesFromString(char* originalString);
				char* removeCharacterFromString(char* originalString, unsigned char theCharacter);
				char* replaceCharacterInString(char* theString, unsigned char originalCharacter, unsigned char newCharacter, bool replaceInPlace = false);
				char* replaceSubString(char* theString, char* oldSubString, char* newSubString, bool caseSensitive = false);
				//int replaceSubString2(char* theString, char* oldSubString, char* newSubString, char* newString);
				char* toUppercase(char* theString);
				char* toLowercase(char* theString);
				char* expandEnvironmentStrings(char* theString);

		} StringLibrary;

		////////////////////
		// file utilities //
		////////////////////
		class CFileLibrary
		{
			public:
				const static int FILEFORMAT_UNKNOWN = 0;
				const static int FILEFORMAT_PE = 1;
				const static int FILEFORMAT_NE = 2;

				bool Wow64DisableWow64FsRedirection(PVOID* oldValue);
				bool Wow64RevertWow64FsRedirection(PVOID oldValue);

				bool getOpenFileName(char* titleString, char* filterString, char* extensionString, char* initialDirectory, char* selectedFilename);

				char* getModuleFilename(HMODULE hModule);
				HMODULE moduleLoadLibrary(char* moduleFilename, bool* moduleLoaded = NULL);
				void moduleFreeLibrary(char* moduleFilename);

				char* getBackupFilename(char* fileName, char* backupExtension = NULL);
				char* getPathFromFilename(char* fileName);
				char* stripPathFromFilename(char* fileName, bool asLowercase = false);
				char* convertShortPathToLongPath(char* shortFilename);

				bool doesFileExist(char* fileName, bool forWriteAccess);
				unsigned long getFileSize(HANDLE fileHandle);
				unsigned long getFileSize(char* fileName);
				unsigned char* readBufferFromFile(HANDLE* fileHandle, char* fileName, unsigned long* fileSize = NULL, unsigned long fileOffset = 0, unsigned long fileBufferSize = 0, bool fromFileStart = true);
				unsigned char* readBufferFromFile(char* fileName, unsigned long* fileSize = NULL, unsigned long fileOffset = 0, unsigned long fileBufferSize = 0, bool fromFileStart = true);
				unsigned char* readBufferFromFile(char* fileName, unsigned long fileBufferSize);
				bool writeBufferToFile(HANDLE* fileHandle, char* fileName, unsigned char* fileBuffer, unsigned long fileBufferSize, unsigned long fileOffset = 0, bool openExistingIfFound = false, bool fromFileStart = true);
				bool writeBufferToFile(char* fileName, unsigned char* fileBuffer, unsigned long fileBufferSize, unsigned long fileOffset = 0, bool openExistingIfFound = false, bool fromFileStart = true);
				bool appendBufferToFile(HANDLE* fileHandle, char* fileName, unsigned char* fileBuffer, unsigned long fileBufferSize);
				bool appendBufferToFile(char* fileName, unsigned char* fileBuffer, unsigned long fileBufferSize);

				bool extractResourceToTemporaryFile(unsigned char* resourceStream, unsigned long resourceSize, char* fileName, char* prefixString = NULL, char* extensionString = NULL);
				unsigned char* getResourceFromFile(char* fileName, char* resourceName, char* resourceType, unsigned long* resourceSize = NULL);
				unsigned long determineFileFormat(char* fileName, unsigned long* numberOfPESections = NULL);

		} FileLibrary;

		////////////////////////
		// registry utilities //
		////////////////////////
		class CRegistryLibrary
		{
			public:
				const static int ACCESSMASK_DEFAULT = 0;
				const static int ACCESSMASK_READ = KEY_READ;
				const static int ACCESSMASK_ALL = KEY_ALL_ACCESS;
				const static int ACCESSMASK_WOW64_ACCESS32BITKEY = KEY_WOW64_32KEY;
				const static int ACCESSMASK_WOW64_ACCESS64BITKEY = KEY_WOW64_64KEY;

				HKEY getUserClassesRegistryHive(void);

				bool isRegistrySubKey(HKEY regHive, char* regSubKey, unsigned long accessMaskAdditional = ACCESSMASK_DEFAULT);
				bool isRegistryKey(HKEY regHive, char* regSubKey, char* regKey, unsigned long accessMaskAdditional = ACCESSMASK_DEFAULT);

				HKEY openRegistrySubKey(HKEY regHive, char* regSubKey, unsigned long accessMask = ACCESSMASK_READ);
				HKEY createRegistrySubKey(HKEY regHive, char* regSubKey, unsigned long accessMask = ACCESSMASK_ALL);

				bool deleteRegistrySubKey(HKEY regHive, char* regSubKey, char* regKey, unsigned long accessMaskAdditional = ACCESSMASK_DEFAULT);
				bool deleteRegistryKeyValue(HKEY hKey, char* regValue);

				unsigned char* getRegistryKeyValue(HKEY hKey, char* regValue, unsigned long* regValueLength, unsigned long* regValueType);
				char* getRegistryKeyValueString(HKEY hKey, char* regValue);
				bool setRegistryKeyValue(HKEY hKey, char* regValue, unsigned char* regValueData, unsigned long regValueType);

				bool appendRegistryKeyValue(HKEY hKey, char* regValue, char* regValueData, char regValueDataDelimiter);
				bool removeRegistryKeyValue(HKEY hKey, char* regValue, char* regValueData, char regValueDataDelimiter);

				bool registerFileExtension(char* fileExtension, char* fileExtensionDescription, char* fileExtensionDefaultIcon, char* fileExtensionShellCommands, bool createRunAsEntry = false);
				bool unregisterFileExtension(char* fileExtension, int numberOfFileExtensionShellCommands);

		} RegistryLibrary;

		///////////////////////
		// console utilities //
		///////////////////////
		class CConsoleLibrary
		{
			public:
				const static int DISPLAY_AS_DEFAULT = 0;
				const static int DISPLAY_AS_BYTE = 1;
				const static int DISPLAY_AS_WORD = 2;
				const static int DISPLAY_AS_DWORD = 3;

				HANDLE getConsoleInputHandle(void);
				HANDLE getConsoleOutputHandle(void);
				void setConsoleOutputHandle(HANDLE consoleOutputHandle);

				void hideConsoleCursor(void);
				void showConsoleCursor(void);

				unsigned char getConsoleInputAsKeyPress(bool showCursorPosition = false, char* allowedCharacterSet = NULL, unsigned char* unusedCharacters = NULL, unsigned long* numberOfUnusedCharacters = NULL);
				bool getConsoleInputAsYesNo(bool showCursorPosition, bool acceptEmptyInput = false);
				// formatting constants: A - alphabet character (uppercase), a - alphabet character (lowercase), L - alphabet character (uppercase or lowercase), 1 - numeric character, H - hexadecimal character, * - any character, ~ - ignore position (i.e. spacing)
				unsigned char* getConsoleInputAsFormattedString(char* formattingString, char* displayString, bool showCursorPosition = false, bool addNewLine = true, char* allowedCharacterSet = NULL, bool allowPrematureTermination = false, char* validationString = NULL, char* initialInputValues = NULL);
				unsigned char* getConsoleInput(int maximumNumberOfCharacters, bool showCursorPosition = true, bool addNewLine = true);

				void displayConsoleMessage(char* consoleMessage, bool addNewLine = true);
				void displayConsoleMessage(char* consoleMessage, int maximumMessageLength, bool addNewLine = true);
				void displayConsoleMessage(char consoleMessageCharacter, int repeatCount, bool addNewLine = true);
				void displayConsoleMessage(unsigned char consoleMessageCharacter, bool addNewLine = true);
				void displayConsoleMessage(unsigned long consoleMessageLong, bool asHexadecimal = false, bool addNewLine = true, int displayAs = DISPLAY_AS_DEFAULT);
				void displayHexadecimalString(unsigned char* hexString, int hexStringLength, int spacerInterval, bool addNewLine, char* prefixString = NULL, char* spacerCharacter = " ");

				void displayApplicationTitle(char* applicationTitle, char* applicationVersion, int startYear);
				void displayApplicationTitle(char* applicationTitle, char* applicationVersion, char* copyrightInformation);

				void displayErrorMessage(char* errorMessages[], int errorNumber, bool exitProgram = true, bool justErrorMessage = false);

				void enableConsoleControlHandler(void);
				void disableConsoleControlHandler(void);
				HWND getConsoleWindowHandle(void);

		} ConsoleLibrary;

		//////////////////////////////////
		// operating system information //
		//////////////////////////////////
		class COperatingSystemLibrary
		{
			public:
				float getInternetExplorerVersion(char* servicePack, unsigned long* buildVersion);
				char* getServicePack(void);

				bool isCurrentUserNTAdmin(void);

				bool isWindows64Bit(void);

				bool isWindows9x(void);
				bool isWindows95(bool orGreater = false);
				bool isWindows95OSR2(bool orGreater = false);
				bool isWindows98(bool orGreater = false);
				bool isWindows98SE(bool orGreater = false);
				bool isWindowsME(bool orGreater = false);

				bool isWindowsNTx(void);
				bool isWindowsNT4(bool orGreater = false);
				bool isWindows2000(bool orGreater = false);
				bool isWindowsXP(bool orGreater = false);
				bool isWindowsServer2003(bool orGreater = false);
				bool isWindowsVista(bool orGreater = false);
				bool isWindowsServer2008(bool orGreater = false);
				bool isWindows7(bool orGreater = false);
				bool isWindowsServer2008R2(bool orGreater = false);
				bool isWindows8(bool orGreater = false);
				bool isWindowsServer8(bool orGreater = false);

		} OperatingSystemLibrary;

		///////////////////////
		// process utilities //
		///////////////////////
		class CProcessLibrary
		{
			public:
				typedef VOID (CALLBACK* EXITPROCESS_CALLBACK_FUNCTION)(VOID);

				const static int EXITPROCESS_DEFAULT = 0;
				const static int EXITPROCESS_DISPLAYMESSAGE_ALWAYS = 1;
				const static int EXITPROCESS_DISPLAYMESSAGE_NEVER = 2;
				const static int EXITPROCESS_DELETESELF = 4;

				const static int VM_VMWARE = 1;
				const static int VM_VIRTUALPC = 2;
				const static int VM_VIRTUALSERVER = 2;
				const static int VM_DETECTSIDT = 3;
				const static int VM_DETECTSGDT = 4;
				const static int VM_DETECTSLDT = 5;

				bool isParentProcessWindowsExplorer(void);

				void setDefaultExitProcessFlags(int exitProcessFlags);
				void exitProcessHandler(char* exitProcessMessage = NULL, int exitProcessFlags = EXITPROCESS_DEFAULT, EXITPROCESS_CALLBACK_FUNCTION* exitProcessCallbackFunction = NULL);

				void enableUnhandledExceptionFilter(void);
				void disableUnhandledExceptionFilter(void);

				bool isDebugModeEnabled(void);
				bool isNonInteractiveServiceProcess(void);
				bool isInteractiveServiceProcess(void);
				bool isRemoteSession(void);
				bool isWOW64Process(void);
				bool is64BitProcess(void);
				bool isRunningInsideVirtualMachine(int vmId, int* vmVersionInt = NULL, char* vmVersionString = NULL);

		} ProcessLibrary;

		////////////////////////////
		// parsing engine library //
		////////////////////////////
		class CParsingEngine
		{
			public:
				const static int ERROR_PARSINGENGINE_SUCCEEDED = 0;
				const static int ERROR_PARSINGENGINE_UNKNOWNERROR = 1;
				const static int ERROR_PARSINGENGINE_FILENOTFOUND = 2;
				const static int ERROR_PARSINGENGINE_UNSUPPORTEDFILEFORMAT = 3;

				int getLastErrorNumber(void);

				void initialiseParsingEngine(char* fileName, char* fileHeader, char* supportedParameters);
				void destroyParsingEngine(void);

				bool parseFile(void);
				bool isParsedSectionName(char* sectionName);

				int getNumberOfParameters(char* parameterName = NULL);
				int getNumberOfSectionParameters(char* sectionName, char* parameterName = NULL);

				bool getNextParsedParameter(char* parameterName, char* parameterValue);
				bool getNextParsedParameter(char* sectionName, char* parameterName, char* parameterValue);

				bool getParsedParameter(char* sectionName, char* parameterName, char* parameterValue, int parameterIndex = 0);
				bool getParsedParameter(char* parameterName, char* parameterValue, int parameterIndex = 0);
				bool getParsedParameter(char* parameterName, unsigned long* parameterValue, int parameterIndex = 0, bool forceHexadecimal = false);

				char* removeCommentBlocks(unsigned char* fileBuffer, unsigned int fileBufferLength);

		} ParsingEngine;

		////////////////////////
		// encryption library //
		////////////////////////
		class CEncryptionLibrary
		{
			public:
				class CRC4Library
				{
					public:
						typedef struct _RC4_KEY
						{
							unsigned char state[256];
							unsigned char x;
							unsigned char y;
						} RC4_KEY;

						void encryptBuffer(unsigned char* encryptionKey, int encryptionKeyLength, unsigned char* memoryBuffer, int memoryBufferOffset, int memoryBufferLength);

				} RC4Library;

		} EncryptionLibrary;
};