//////////////////////////////////////////////////
// PowerQuest Drive Image Password Cracker      //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////
#include <windows.h>
#include "PQdi.h"
#include "PQdi4.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"
#include "..\..\..\_Shared Libraries\DictionaryEngine.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;
extern char* errorMessages[];

int selectedSearchMethod = SEARCH_UNKNOWN;

unsigned long requiredPasswordCRC = 0;
int startWordLength = 0;
int endWordLength = 0;
int wordToleranceLevel = 7;
bool useLowercaseCharset = false;
bool useUppercaseCharset = false;
bool useNumbersCharset = false;
bool useSpecialCharset = false;
char* dictionaryFilename = NULL;

unsigned long crcTable[256];

////////////////////////////////////////////////////////////////
// sets the search properties from the command line arguments //
////////////////////////////////////////////////////////////////
void setSearchProperties(int searchMethod, int startLength, int endLength, int toleranceLevel, bool useLowercase,
						 bool useUppercase, bool useNumbers, bool useSpecial, char* dictionaryName)
{
	selectedSearchMethod = searchMethod;
	startWordLength = startLength;
	endWordLength = endLength;
	wordToleranceLevel = toleranceLevel;
	useLowercaseCharset = useLowercase;
	useUppercaseCharset = useUppercase;
	useNumbersCharset = useNumbers;
	useSpecialCharset = useSpecial;
	dictionaryFilename = dictionaryName;
}

///////////////////////////////////////////
// display password cracking information //
///////////////////////////////////////////
void displayInformation(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Search Engine\t\t: ", false);
	if (selectedSearchMethod == SEARCH_BRUTEFORCE) ULib.ConsoleLibrary.displayConsoleMessage("BruteForce Engine");
	else if (selectedSearchMethod == SEARCH_INTELLIFORCE) ULib.ConsoleLibrary.displayConsoleMessage("IntelliForce Engine");
	else if (selectedSearchMethod == SEARCH_DICTIONARY) ULib.ConsoleLibrary.displayConsoleMessage("Dictionary Engine");
	else ULib.ConsoleLibrary.displayConsoleMessage("");
	if (selectedSearchMethod == SEARCH_DICTIONARY)
	{
		char* dictionaryFilenameStripped = ULib.FileLibrary.stripPathFromFilename(dictionaryFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Dictionary\t\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(dictionaryFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(dictionaryFilenameStripped);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Search Length\t\t: ", false);
		if (startWordLength == endWordLength)
		{
			ULib.ConsoleLibrary.displayConsoleMessage(startWordLength, false, false);
			ULib.ConsoleLibrary.displayConsoleMessage(" characters");
		}
		else
		{
			ULib.ConsoleLibrary.displayConsoleMessage(startWordLength, false, false);
			ULib.ConsoleLibrary.displayConsoleMessage(" to ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(endWordLength, false, false);
			ULib.ConsoleLibrary.displayConsoleMessage(" character(s)");
		}
		ULib.ConsoleLibrary.displayConsoleMessage("Character Set\t\t: ", false);
		if (useLowercaseCharset) ULib.ConsoleLibrary.displayConsoleMessage("Lowercase ", false);
		if (useUppercaseCharset) ULib.ConsoleLibrary.displayConsoleMessage("Uppercase ", false);
		if (useNumbersCharset) ULib.ConsoleLibrary.displayConsoleMessage("Numbers ", false);
		if (useSpecialCharset) ULib.ConsoleLibrary.displayConsoleMessage("Special ", false);
		if (selectedSearchMethod == SEARCH_INTELLIFORCE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");
			ULib.ConsoleLibrary.displayConsoleMessage("Tolerance Level\t\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(wordToleranceLevel, false, false);
		}

		ULib.ConsoleLibrary.displayConsoleMessage("");
	}

	ULib.ConsoleLibrary.displayConsoleMessage("Required CRC\t\t: 0x", false);
	ULib.ConsoleLibrary.displayConsoleMessage(requiredPasswordCRC, true, true);
	ULib.ConsoleLibrary.displayConsoleMessage("");
}

/////////////////////////////////////////
// create and initialise the CRC table //
/////////////////////////////////////////
void initialiseCRCTable(void)
{
	for (int i = 0; i <= 255; i++)
	{
		unsigned long j = i;

		for (int k = 8; k > 0; k--)
		{
			if (j & 0x01)
			{
				__asm shr j, 1
				j = j ^ 0xEDB88320;
			}
			else
			{
				__asm shr j, 1
			}
		}

		crcTable[i] = j;
	}
}

///////////////////////////////////////////////////////
// Generate the CRC for the password currentPassword //
///////////////////////////////////////////////////////
unsigned long __inline generateCRC(char* currentPassword, int currentPasswordLength)
{
	unsigned long baseCRC = 0x00FFFFFF;

	switch (currentPasswordLength)
	{
		case 1:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 2:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 3:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 4:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 5:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 6:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 7:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 8:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 9:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 10:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 11:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 10]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 12:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 10]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 11]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 13:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 10]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 11]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 12]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 14:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 10]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 11]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 12]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 13]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 15:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 10]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 11]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 12]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 13]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 14]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		case 16:
		{
			__asm
			{
					push eax
					push ebx
					push ecx

					lea ebx, [crcTable]
					xor ecx, ecx
					dec cl

					mov eax, [currentPassword]
					xor cl, [eax + 0]
					mov eax, [baseCRC]
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 1]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 2]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 3]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 4]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 5]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 6]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 7]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 8]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 9]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 10]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 11]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 12]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 13]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 14]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					push eax

					mov eax, [currentPassword]
					xor cl, [eax + 15]
					pop eax
					xor eax, [ebx + ecx * 4]
					mov cl, al
					shr eax, 8
					xor eax, [ebx + ecx * 4]

					mov [baseCRC], eax

					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
	}

	return baseCRC;
}

///////////////////////////////////////////////////
// this method may not recover the password that //
// was selected at creation time. What this does //
// is to generate a valid password that matches  //
// the CRC stored in the encrypted header        //
///////////////////////////////////////////////////
char* findPasswordv4(unsigned char* passwordCRC)
{
	requiredPasswordCRC = ULib.StringLibrary.getDWORDFromBuffer(passwordCRC);
	// to save work, NOT the requiredPasswordCRC
	// value, and not the generated CRC
	__asm not requiredPasswordCRC;

	if (selectedSearchMethod != SEARCH_UNKNOWN) displayInformation();
	initialiseCRCTable();

	ULib.ConsoleLibrary.hideConsoleCursor();

	char* recoveredPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
	bool foundPassword = false;
	char* currentWord;

	// do bruteforce search
	if (selectedSearchMethod == SEARCH_BRUTEFORCE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 25, false);

		BruteForceEngine BFengine;
		BFengine.initialiseCharacterSet(useLowercaseCharset, useUppercaseCharset, useNumbersCharset, useSpecialCharset);

		for (int currentWordLength = startWordLength; currentWordLength <= endWordLength; currentWordLength++)
		{
			BFengine.setCurrentWordLength(currentWordLength);

			do
			{
				currentWord = BFengine.getNextWord();
				if (generateCRC(currentWord, currentWordLength) == requiredPasswordCRC)
				{
					ULib.StringLibrary.copyString(recoveredPassword, currentWord);
					foundPassword = true;
					break;
				}

			} while (currentWord[0] != NULL);

			if (foundPassword) break;
		}

		BFengine.destroyCharacterSet();
	}
	// do intelliforce search
	else if (selectedSearchMethod == SEARCH_INTELLIFORCE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 25, false);

		IntelliForceEngine IFengine;
		IFengine.initialiseCharacterSet(useLowercaseCharset, useUppercaseCharset);
		IFengine.setToleranceLevel(wordToleranceLevel);

		for (int currentWordLength = startWordLength; currentWordLength <= endWordLength; currentWordLength++)
		{
			IFengine.setCurrentWordLength(currentWordLength);

			do
			{
				currentWord = IFengine.getNextWord();
				if (generateCRC(currentWord, currentWordLength) == requiredPasswordCRC)
				{
					ULib.StringLibrary.copyString(recoveredPassword, currentWord);
					foundPassword = true;
					break;
				}

			} while (currentWord[0] != NULL);

			if (foundPassword) break;
		}

		IFengine.destroyCharacterSet();
	}
	// do dictionary search
	else if (selectedSearchMethod == SEARCH_DICTIONARY)
	{
		unsigned long currentWordLength;

		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 25, false);

		DictionaryEngine DictEngine;
		if (!DictEngine.initialiseDictionaryEngine(dictionaryFilename, DICTIONARYENGINE_FILTER_ALL))
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
		}

		do
		{
			currentWord = DictEngine.getNextWord(&currentWordLength);
			if (generateCRC(currentWord, currentWordLength) == requiredPasswordCRC)
			{
				ULib.StringLibrary.copyString(recoveredPassword, currentWord);
				foundPassword = true;
				break;
			}

		} while (currentWord[0] != NULL);

		DictEngine.destroyDictionaryEngine();
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5, false);
		displayOptions();
		ULib.ProcessLibrary.exitProcessHandler();
	}

	ULib.ConsoleLibrary.showConsoleCursor();
	return recoveredPassword;
}