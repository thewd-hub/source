///////////////////////////////////////////////
// Execute Module                            //
// (C)thewd, thewd@hotmail.com               //
///////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"{CRLF}Unable to load the specified module library",
	"{CRLF}Unable to find the specified export function"
};

CUtilitiesLibrary ULib;

typedef DWORD (__cdecl* MODULEFUNCTION)(...);

// module filename, export function and address function
char* moduleFilename = NULL;
char* exportFunctionName = NULL;
DWORD memoryAddressRelative = 0;

// type of return value
#define RETURNVALUETYPE_UNKNOWN			0
#define RETURNVALUETYPE_VOID			1
#define RETURNVALUETYPE_STRING			2
#define RETURNVALUETYPE_DWORD			3
#define RETURNVALUETYPE_LPDWORD			4
#define RETURNVALUETYPE_BOOL			5

int returnValueType = RETURNVALUETYPE_UNKNOWN;

// function arguments
#define MAXIMUM_ARGUMENTS				100
#define ARGUMENTSVALUETYPE_UNKNOWN		0
#define ARGUMENTSVALUETYPE_NULL			1
#define ARGUMENTSVALUETYPE_STRING		2
#define ARGUMENTSVALUETYPE_DWORD		3
#define ARGUMENTSVALUETYPE_LPDWORD		4
#define ARGUMENTSVALUETYPE_BOOL			5
#define ARGUMENTSVALUETYPE_FILEHANDLE	6

int numberOfFunctionArguments = 0;
LPVOID functionArguments[MAXIMUM_ARGUMENTS];
int functionArgumentsType[MAXIMUM_ARGUMENTS];
char* functionArgumentsDetails[MAXIMUM_ARGUMENTS];

// breakpoint at start of function call routine
BOOL doBreakpoint = FALSE;
BOOL doPause = FALSE;

/////////////////////////////////////////////////////
// parse arguments passed through the command line //
/////////////////////////////////////////////////////
void parseArguments(int argc, char* argv[])
{
	// parse parameters and arguments
	for (int i = 1; i < argc; i++)
	{
		// maximum number of arguments
		if (numberOfFunctionArguments >= MAXIMUM_ARGUMENTS) break;
		if (!ULib.StringLibrary.isString(argv[i], 8)) continue;

		char* stringValue = ULib.StringLibrary.getNextTokenString(argv[i], '=');

		// module filename
		if (ULib.StringLibrary.isStringMatch(argv[i], "-moduleName=", false, 12)) moduleFilename = stringValue;
		// export function name
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-exportFunction=", false, 16)) exportFunctionName = stringValue;
		// memory address
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-memoryAddress=", false, 15)) memoryAddressRelative = ULib.StringLibrary.getDWORDFromString(stringValue);
		// return type
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-returnValue=", false, 13))
		{
			if (ULib.StringLibrary.isStringMatch(stringValue, "VOID")) returnValueType = RETURNVALUETYPE_VOID;
			else if (ULib.StringLibrary.isStringMatch(stringValue, "STRING")) returnValueType = RETURNVALUETYPE_STRING;
			else if (ULib.StringLibrary.isStringMatch(stringValue, "DWORD")) returnValueType = RETURNVALUETYPE_DWORD;
			else if (ULib.StringLibrary.isStringMatch(stringValue, "LPDWORD")) returnValueType = RETURNVALUETYPE_LPDWORD;
			else if (ULib.StringLibrary.isStringMatch(stringValue, "BOOL")) returnValueType = RETURNVALUETYPE_BOOL;
		}
		// argument - null
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-argNULL", false, 8))
		{
			functionArguments[numberOfFunctionArguments] = (LPVOID)NULL;
			functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_NULL;
			numberOfFunctionArguments++;
		}
		// argument - string
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-argSTRING=", false, 11))
		{
			functionArguments[numberOfFunctionArguments] = (LPVOID)ULib.HeapLibrary.allocPrivateHeap(1024 * 1000);
			ULib.StringLibrary.copyString((LPSTR)functionArguments[numberOfFunctionArguments], stringValue);
			functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_STRING;
			numberOfFunctionArguments++;
		}
		// argument - dword
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-argDWORD=", false, 10))
		{
			functionArguments[numberOfFunctionArguments] = ULongToPtr(ULib.StringLibrary.getDWORDFromString(stringValue));
			functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_DWORD;
			numberOfFunctionArguments++;
		}
		// argument - lpdword
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-argLPDWORD=", false,  12))
		{
			functionArguments[numberOfFunctionArguments] = (LPVOID)ULib.HeapLibrary.allocPrivateHeap(10);
			ULib.StringLibrary.addDWORDToBuffer((unsigned char*)functionArguments[numberOfFunctionArguments], ULib.StringLibrary.getDWORDFromString(stringValue));
			functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_LPDWORD;
			numberOfFunctionArguments++;
		}
		// argument - bool
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-argBOOL=", false,  9))
		{
			if (ULib.StringLibrary.isStringMatch(stringValue, "true"))
			{
				functionArguments[numberOfFunctionArguments] = (LPVOID)TRUE;
				functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_BOOL;
				numberOfFunctionArguments++;
			}
			else if (ULib.StringLibrary.isStringMatch(stringValue, "false"))
			{
				functionArguments[numberOfFunctionArguments] = (LPVOID)FALSE;
				functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_BOOL;
				numberOfFunctionArguments++;
			}
		}
		// argument - file handle
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-argFILEHANDLE=", false, 15))
		{
			functionArguments[numberOfFunctionArguments] = (LPVOID)CreateFile(stringValue, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			functionArgumentsType[numberOfFunctionArguments] = ARGUMENTSVALUETYPE_FILEHANDLE;
			functionArgumentsDetails[numberOfFunctionArguments] = (char*)ULib.HeapLibrary.allocPrivateHeap();
			ULib.StringLibrary.copyString(functionArgumentsDetails[numberOfFunctionArguments], ULib.FileLibrary.stripPathFromFilename(stringValue, true));
			numberOfFunctionArguments++;
		}
		// breakpoint
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-breakpoint")) doBreakpoint = TRUE;
		// pause
		else if (ULib.StringLibrary.isStringMatch(argv[i], "-promptexecution")) doPause = TRUE;
	}
}

//////////////////////////////////////////////////////
// executes the specifed function within the module //
//////////////////////////////////////////////////////
void executeModuleFunction(void)
{
	char* outputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();

	ULib.ConsoleLibrary.displayConsoleMessage("[Function Properties]");

	// load module
	char* moduleFilenameStripped = ULib.FileLibrary.stripPathFromFilename(moduleFilename, true);
	wsprintf(outputMessage, "  Module Filename    : %s", moduleFilenameStripped);
	ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
	ULib.HeapLibrary.freePrivateHeap(moduleFilenameStripped);
	HMODULE moduleLibrary = ULib.FileLibrary.moduleLoadLibrary(moduleFilename);
	if (moduleLibrary == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	wsprintf(outputMessage, "  Module Address     : %d (0x%X)", moduleLibrary, moduleLibrary);
	ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);

	// find export function
	MODULEFUNCTION moduleFunction = NULL;
	if (memoryAddressRelative > 0)
	{
		moduleFunction = (MODULEFUNCTION)(PtrToUlong(moduleLibrary) + memoryAddressRelative);
		wsprintf(outputMessage, "  Memory Address (R) : %d (0x%X)", memoryAddressRelative, memoryAddressRelative);
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		wsprintf(outputMessage, "  Memory Address (A) : %d (0x%X)", moduleFunction, moduleFunction);
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
	}
	else
	{
		wsprintf(outputMessage, "  Export Function    : %s", exportFunctionName);
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		moduleFunction = (MODULEFUNCTION)GetProcAddress(moduleLibrary, ((ULib.StringLibrary.hasStringHexadecimalPrefix(exportFunctionName)) ? MAKEINTRESOURCE(ULib.StringLibrary.getDWORDFromString(exportFunctionName)) : exportFunctionName));
		wsprintf(outputMessage, "  Export Address (A) : %d (0x%X)", moduleFunction, moduleFunction);
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		if (moduleFunction == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	// return value type
	ULib.ConsoleLibrary.displayConsoleMessage("  Return Value Type  : ", false);
	if (returnValueType == RETURNVALUETYPE_VOID) ULib.ConsoleLibrary.displayConsoleMessage("VOID");
	else if (returnValueType == RETURNVALUETYPE_STRING) ULib.ConsoleLibrary.displayConsoleMessage("STRING");
	else if (returnValueType == RETURNVALUETYPE_DWORD) ULib.ConsoleLibrary.displayConsoleMessage("DWORD");
	else if (returnValueType == RETURNVALUETYPE_LPDWORD) ULib.ConsoleLibrary.displayConsoleMessage("LPDWORD");
	else if (returnValueType == RETURNVALUETYPE_BOOL) ULib.ConsoleLibrary.displayConsoleMessage("BOOL");
	else ULib.ConsoleLibrary.displayConsoleMessage("UNKNOWN");

	// list function arguments
	for (int i = 0; i < numberOfFunctionArguments; i++)
	{
		if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_NULL) ULib.ConsoleLibrary.displayConsoleMessage("  Argument (NULL)    : NULL");
		else if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_STRING)
		{
			wsprintf(outputMessage, "  Argument (STRING)  : %s", (char*)functionArguments[i]);
			ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		}
		else if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_DWORD)
		{
			wsprintf(outputMessage, "  Argument (DWORD)   : %d (0x%X)", PtrToUlong(functionArguments[i]), PtrToUlong(functionArguments[i]));
			ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		}
		else if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_LPDWORD)
		{
			wsprintf(outputMessage, "  Argument (LPDWORD) : %d (0x%X)", ULib.StringLibrary.getDWORDFromBuffer((unsigned char*)functionArguments[i]), ULib.StringLibrary.getDWORDFromBuffer((unsigned char*)functionArguments[i]));
			ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		}
		else if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_BOOL)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("  Argument (BOOL)    : ", false);
			bool isArgumentTrue = ((((BOOL)PtrToInt(functionArguments[i])) == TRUE) ? true : false);
			if (isArgumentTrue) ULib.ConsoleLibrary.displayConsoleMessage("TRUE");
			else ULib.ConsoleLibrary.displayConsoleMessage("FALSE");
		}
		else if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_FILEHANDLE)
		{
			wsprintf(outputMessage, "  Argument (HANDLE)  : %s (0x%X)", functionArgumentsDetails[i], PtrToUlong(functionArguments[i]));
			ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		}
	}

	if (doPause)
	{
		ULib.ConsoleLibrary.getConsoleInputAsKeyPress();
	}

	ULib.ConsoleLibrary.displayConsoleMessage("\r\n[Executing Function]");
	DWORD functionReturnValue;

	__asm
	{
			push eax
			push ebx
			push ecx
			push edx

			mov ecx, [doBreakpoint]
			cmp ecx, 0x00
			je noBreakpoint
			int 3

		noBreakpoint:
			lea ebx, functionArguments
			mov ecx, [numberOfFunctionArguments]

		pushArgument:
			cmp ecx, 0x00
			je executeFunction
			dec ecx
			mov eax, [ebx + ecx * 4]
			push eax
			jmp pushArgument

		executeFunction:
			mov edx, [moduleFunction]
			call edx
			mov [functionReturnValue], eax

			pop edx
			pop ecx
			pop ebx
			pop eax
	}

	// display return value
	ULib.ConsoleLibrary.displayConsoleMessage("  Return Value       : ", false);
	if (returnValueType == RETURNVALUETYPE_VOID) ULib.ConsoleLibrary.displayConsoleMessage("VOID");
	else if (returnValueType == RETURNVALUETYPE_STRING) ULib.ConsoleLibrary.displayConsoleMessage((char*)UlongToPtr(functionReturnValue));
	else if (returnValueType == RETURNVALUETYPE_DWORD)
	{
		wsprintf(outputMessage, "%d (0x%X)", functionReturnValue, functionReturnValue);
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
	}
	else if (returnValueType == RETURNVALUETYPE_LPDWORD)
	{
		wsprintf(outputMessage, "%d (0x%X)", ULib.StringLibrary.getDWORDFromBuffer((unsigned char*)UlongToPtr(functionReturnValue)), ULib.StringLibrary.getDWORDFromBuffer((unsigned char*)UlongToPtr(functionReturnValue)));
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
	}
	else if (returnValueType == RETURNVALUETYPE_BOOL)
	{
		if ((BOOL)functionReturnValue) ULib.ConsoleLibrary.displayConsoleMessage("TRUE");
		else ULib.ConsoleLibrary.displayConsoleMessage("FALSE");
	}
	else ULib.ConsoleLibrary.displayConsoleMessage(functionReturnValue, true, true);

	// cleanup list function arguments
	for (int i = 0; i < numberOfFunctionArguments; i++)
	{
		if (functionArgumentsType[i] == ARGUMENTSVALUETYPE_FILEHANDLE) CloseHandle((HANDLE)functionArguments[i]);
	}

	ULib.HeapLibrary.freePrivateHeap(outputMessage);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Execute Module", "1.00", 2002);

	if (argc > 3)
	{
		parseArguments(argc, argv);
		executeModuleFunction();
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : executeModule <required parameters> [<function arguments>]");
		ULib.ConsoleLibrary.displayConsoleMessage("Options:");
		ULib.ConsoleLibrary.displayConsoleMessage("\t [Required Parameters]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -moduleName=<module filename>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -exportFunction=<export function name>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -memoryAddress=<memory address of function>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -returnValue=<VOID|STRING|DWORD|LPDWORD|BOOL>\r\n");
		ULib.ConsoleLibrary.displayConsoleMessage("\t [Function Arguments]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -argNULL");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -argSTRING=<string argument>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -argDWORD=<dword argument>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -argLPDWORD=<dword argument>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -argBOOL=<bool argument - TRUE|FALSE>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -argFILEHANDLE=<filename>\r\n");
		ULib.ConsoleLibrary.displayConsoleMessage("\t [Optional Parameters]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -breakpoint");
		ULib.ConsoleLibrary.displayConsoleMessage("\t -promptexecution");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}