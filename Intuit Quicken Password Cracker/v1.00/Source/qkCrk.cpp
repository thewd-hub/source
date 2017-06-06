/////////////////////////////////////////////////////
// Intuit Quicken Password Cracker                 //
// (C)thewd, thewd@hotmail.com                     //
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Recovers passwords up to a maximum length of 16 //
// characters for Intuit Quicken 2000+ databases   //
/////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"
#include "..\..\..\_Shared Libraries\DictionaryEngine.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the dictionary wordlist"
};

void displayResults(char* foundPassword, bool passwordFound);

//////////////////////////////////////////////
// determine the CRC for a certain password //
//////////////////////////////////////////////
unsigned long __inline determineCRC(char* currentPassword, unsigned long passwordLength, unsigned long passwordCRC)
{
	unsigned long currentCRC = 0;

	__asm
	{
			push eax
			push esi
			push edi
			push edx
			push ebx
			push ecx
			xor eax, eax
			xor ecx, ecx
			mov esi, [passwordLength]
			mov ebx, currentPassword
			mov edi, 0x06EB
		crcLoop:
			movsx edx, [ebx + ecx]
			sub edx, 0x0020
			imul edx, edi
			add eax, edx
			cmp [passwordCRC], eax
			jl breakLoop
			add edi, 0x002B
			inc ecx
			cmp ecx, esi
			jl crcLoop
			mov [currentCRC], eax
		breakLoop:
			pop ecx
			pop ebx
			pop edx
			pop edi
			pop esi
			pop eax
	}

	return currentCRC;
}

/////////////////////////////////////////////////////////////////
// attempt to recover the password using the dictionary engine //
/////////////////////////////////////////////////////////////////
void recoverPasswordUsingDictionary(unsigned long passwordCRC, char* dictionaryFilename)
{
	char* currentWord;
	unsigned long currentWordLength;
	bool foundPassword = false;

	DictionaryEngine DictEngine;
	if (!DictEngine.initialiseDictionaryEngine(dictionaryFilename, DICTIONARYENGINE_FILTER_UPPERCASE)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	ULib.ConsoleLibrary.hideConsoleCursor();

	ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (Dictionary)...", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 38, false);

	do
	{
		currentWord = DictEngine.getNextWord(&currentWordLength);
		if (determineCRC(currentWord, currentWordLength, passwordCRC) == passwordCRC)
		{
			foundPassword = true;
			break;
		}

	} while (currentWord[0] != NULL);

	ULib.ConsoleLibrary.showConsoleCursor();

	if (!foundPassword) displayResults(currentWord, false);
	else displayResults(currentWord, true);

	DictEngine.destroyDictionaryEngine();
}

////////////////////////////////////////////////
// attempt to recover the password using the  //
// brute-force engine starting at startLength //
// and ending when endLength is reached       //
////////////////////////////////////////////////
void recoverPasswordUsingBruteForce(unsigned long passwordCRC, unsigned long startLength, unsigned long endLength)
{
	char* currentWord = NULL;
	bool foundPassword = false;

	BruteForceEngine BFengine;
	BFengine.initialiseCharacterSet(false, true, true, true);

	ULib.ConsoleLibrary.hideConsoleCursor();

	for (unsigned long currentWordLength = startLength; currentWordLength <= endLength; currentWordLength++)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (Length - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(currentWordLength, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		ULib.ConsoleLibrary.displayConsoleMessage(")...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 39, false);
		BFengine.setCurrentWordLength(currentWordLength);

		do
		{
			currentWord = BFengine.getNextWord();
			if (determineCRC(currentWord, currentWordLength, passwordCRC) == passwordCRC)
			{
				foundPassword = true;
				break;
			}

		} while (currentWord[0] != NULL);

		if (foundPassword) break;
	}

	ULib.ConsoleLibrary.showConsoleCursor();

	if (!foundPassword) displayResults(currentWord, false);
	else displayResults(currentWord, true);

	BFengine.destroyCharacterSet();
}

////////////////////////////////////////////////////////////
// attempt to recover the password using the intelliforce //
// engine starting at length startLength and ending when  //
// endLength is reached and tried, using toleranceLevel   //
////////////////////////////////////////////////////////////
void recoverPasswordUsingIntelliForce(unsigned long passwordCRC, unsigned long startLength, unsigned long endLength, unsigned long toleranceLevel)
{
	char* currentWord = NULL;
	bool foundPassword = false;

	IntelliForceEngine IFengine;
	IFengine.initialiseCharacterSet(false, true);
	IFengine.setToleranceLevel(toleranceLevel);

	ULib.ConsoleLibrary.hideConsoleCursor();

	for (unsigned long currentWordLength = startLength; currentWordLength <= endLength; currentWordLength++)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (Length - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(currentWordLength, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		ULib.ConsoleLibrary.displayConsoleMessage(")...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 39, false);
		IFengine.setCurrentWordLength(currentWordLength);

		do
		{
			currentWord = IFengine.getNextWord();
			if (determineCRC(currentWord, currentWordLength, passwordCRC) == passwordCRC)
			{
				foundPassword = true;
				break;
			}

		} while (currentWord[0] != NULL);

		if (foundPassword) break;
	}

	ULib.ConsoleLibrary.showConsoleCursor();

	if (!foundPassword) displayResults(currentWord, false);
	else displayResults(currentWord, true);

	IFengine.destroyCharacterSet();
}

//////////////////////////////////////////////////////////
// Display the results of the password recovery process //
//////////////////////////////////////////////////////////
void displayResults(char* foundPassword, bool passwordFound)
{
	if (passwordFound)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password Found : '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(foundPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'                     ");
		MessageBeep(MB_ICONEXCLAMATION);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Warning: Unable to recover a Suitable Password");
	}
}

////////////////////////////////////////////////////////
// display program options is no arguments are passed //
////////////////////////////////////////////////////////
void displayOptions(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tqkCrk [-bruteforce <password crc> <start length> <end length>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t      [-intelliforce <crc> <start length> <end length> <tolerance>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t      [-dictionary <password crc> <dictionary filename>]");
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Intuit Quicken Password Cracker", "1.00", 1999);

	// if arguments are passed
	if (argc > 1)
	{
		if ((ULib.StringLibrary.isStringMatch("-bruteforce", argv[1])) && (argc > 4))
		{
			unsigned long passwordCRC = ULib.StringLibrary.getDWORDFromString(argv[2], true);
			unsigned long startLength = ULib.StringLibrary.getDWORDFromString(argv[3]);
			unsigned long endLength = ULib.StringLibrary.getDWORDFromString(argv[4]);
			recoverPasswordUsingBruteForce(passwordCRC, startLength, endLength);
		}
		else if ((ULib.StringLibrary.isStringMatch("-intelliforce", argv[1])) && (argc > 5))
		{
			unsigned long passwordCRC = ULib.StringLibrary.getDWORDFromString(argv[2], true);
			unsigned long startLength = ULib.StringLibrary.getDWORDFromString(argv[3]);
			unsigned long endLength = ULib.StringLibrary.getDWORDFromString(argv[4]);
			unsigned long toleranceLevel = ULib.StringLibrary.getDWORDFromString(argv[5]);
			recoverPasswordUsingIntelliForce(passwordCRC, startLength, endLength, toleranceLevel);
		}
		else if ((ULib.StringLibrary.isStringMatch("-dictionary", argv[1])) && (argc > 3))
		{
			unsigned long passwordCRC = ULib.StringLibrary.getDWORDFromString(argv[2], true);
			recoverPasswordUsingDictionary(passwordCRC, argv[3]);
		}
		else
		{
			displayOptions();
		}
	}
	else
	{
		displayOptions();
	}

	ULib.ProcessLibrary.exitProcessHandler();
}