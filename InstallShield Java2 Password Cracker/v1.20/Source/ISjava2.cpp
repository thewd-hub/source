///////////////////////////////////////////
// InstallShield Java2 Password Cracker  //
// (C)thewd, thewd@hotmail.com           //
///////////////////////////////////////////
///////////////////////////////////////////
// Recovers password for installations   //
// created with InstallShield Java v2.xx //
//                                       //
// Tested with...                        //
// - v2.50, v2.51                        //
///////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"
#include "..\..\..\_Shared Libraries\DictionaryEngine.h"

char* errorMessages[] =
{
	"Unable to open or read the dictionary wordlist",
	"Unable to recover a suitable password"
};

CUtilitiesLibrary ULib;
BruteForceEngine BFengine;
IntelliForceEngine IFengine;
DictionaryEngine DictEngine;

#define SEARCH_UNKNOWN			0
#define SEARCH_BRUTEFORCE		1
#define SEARCH_INTELLIFORCE		2
#define SEARCH_DICTIONARY		3
int selectedSearchMethod = SEARCH_UNKNOWN;

unsigned long crcTable[256];
unsigned long currentpasswordCRC = 0;
unsigned long requiredpasswordCRC = 0;

int maxProgressBar = 0;

int startWordLength = 1;
int endWordLength = 16;
int toleranceLevel = 7;
bool useLowercase = false;
bool useUppercase = false;
bool useNumbers = false;
bool useSpecial = false;
char* dictionaryName = NULL;

void checkPassword(char* currentPassword, int currentPasswordLength, bool checkOnlyOnePassword);
void displayResults(char* foundPassword, bool passwordFound);

///////////////////////////////////////////
// display password cracking information //
///////////////////////////////////////////
void displayInformation(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Search Engine\t: ", false);
	if (selectedSearchMethod == SEARCH_BRUTEFORCE) ULib.ConsoleLibrary.displayConsoleMessage("BruteForce Engine");
	else if (selectedSearchMethod == SEARCH_INTELLIFORCE) ULib.ConsoleLibrary.displayConsoleMessage("IntelliForce Engine");
	else if (selectedSearchMethod == SEARCH_DICTIONARY) ULib.ConsoleLibrary.displayConsoleMessage("Dictionary Engine");
	else ULib.ConsoleLibrary.displayConsoleMessage("");
	if (selectedSearchMethod == SEARCH_DICTIONARY)
	{
		char* dictionaryNameStripped = ULib.FileLibrary.stripPathFromFilename(dictionaryName, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Dictionary\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(dictionaryNameStripped);
		ULib.HeapLibrary.freePrivateHeap(dictionaryNameStripped);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Search Length\t: ", false);
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
		ULib.ConsoleLibrary.displayConsoleMessage("Character Set\t: ", false);
		if (useLowercase) ULib.ConsoleLibrary.displayConsoleMessage("Lowercase ", false);
		if (useUppercase) ULib.ConsoleLibrary.displayConsoleMessage("Uppercase ", false);
		if (useNumbers) ULib.ConsoleLibrary.displayConsoleMessage("Numbers ", false);
		if (useSpecial) ULib.ConsoleLibrary.displayConsoleMessage("Special ", false);
		if (selectedSearchMethod == SEARCH_INTELLIFORCE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");
			ULib.ConsoleLibrary.displayConsoleMessage("Tolerance Level\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(toleranceLevel, false, false);
		}
		ULib.ConsoleLibrary.displayConsoleMessage("");
	}

	ULib.ConsoleLibrary.displayConsoleMessage("Required CRC\t: ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(requiredpasswordCRC);
}

/////////////////////////////////////////////////////////////////
// Displays the progress bar for each word length to be tested //
/////////////////////////////////////////////////////////////////
void displayProgressBar(void)
{
	maxProgressBar = 50;
	ULib.ConsoleLibrary.displayConsoleMessage("[----|----|----|----|----|----|----|----|----|----]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', maxProgressBar, false);
}

//////////////////////////////////////////////////////////
// increase the scale as the password cracking proceeds //
//////////////////////////////////////////////////////////
void updateProgressBar(void)
{
	int numberOfStars = 0;
	if (selectedSearchMethod == SEARCH_BRUTEFORCE) numberOfStars = ((int)BFengine.getPercentageDone() / 2);
	else if (selectedSearchMethod == SEARCH_INTELLIFORCE) numberOfStars = ((int)IFengine.getPercentageDone() / 2);

	if (numberOfStars < maxProgressBar)
	{
		ULib.ConsoleLibrary.displayConsoleMessage('*', numberOfStars, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', numberOfStars, false);
	}
}

//////////////////////////////////////////////////////////////
// if the scale isn't complete (for some reason) at the end //
// of trying a word length then fill the remaining slots    //
//////////////////////////////////////////////////////////////
void completeProgressBar(void)
{
	int progressBarLoopCount = 1;

	while (progressBarLoopCount < maxProgressBar)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("*", false);
		progressBarLoopCount++;
	}

	ULib.ConsoleLibrary.displayConsoleMessage("] Done!", false);
}

/////////////////////////////////////////
// create and initialise the CRC table //
/////////////////////////////////////////
void initialiseCRCTable(void)
{
	for (int i = 255; i >= 0; i--)
	{
		int j = i << 24;

		for (int k = 8; k > 0; k--)
		{
			if (j < 0)
			{
				j <<= 1;
				j ^= 0x4C11DB7;
			}
			else
			{
				j <<= 1;
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
	unsigned long baseCRC = 0x31415926;

	for (int index = 0; index < currentPasswordLength; index++)
	{
		baseCRC = ((baseCRC << 8) ^ (crcTable[((baseCRC >> 24) ^ currentPassword[index]) & 0xFF]));
	}

	return baseCRC;
}

///////////////////////////////////////////////
// Begin the search for the correct password //
///////////////////////////////////////////////
void beginPasswordSearch(void)
{
	char* currentPassword = NULL;
	bool foundPassword = false;

	displayInformation();

	ULib.ConsoleLibrary.hideConsoleCursor();

	// do bruteforce search
	if (selectedSearchMethod == SEARCH_BRUTEFORCE)
	{
		BFengine.initialiseCharacterSet(useLowercase, useUppercase, useNumbers, useSpecial);

		for (int currentWordLength = startWordLength; currentWordLength <= endWordLength; currentWordLength++)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("\r\nCurrent Length\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(currentWordLength, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(" ", false);

			displayProgressBar();
			BFengine.setCurrentWordLength(currentWordLength);
			char oldLetter = NULL;

			do
			{
				currentPassword = BFengine.getNextWord();

				// only update progress bar when the first letter changes
				if (oldLetter != currentPassword[currentWordLength - 1])
				{
					updateProgressBar();
					oldLetter = currentPassword[currentWordLength - 1];
				}

				if (generateCRC(currentPassword, currentWordLength) == requiredpasswordCRC)
				{
					foundPassword = true;
					break;
				}

			} while (currentPassword[0] != NULL);

			completeProgressBar();
			if (foundPassword) break;
		}

		if (foundPassword) displayResults(currentPassword, true);
		BFengine.destroyCharacterSet();
	}
	else if (selectedSearchMethod == SEARCH_INTELLIFORCE)
	{
		IFengine.initialiseCharacterSet(useLowercase, useUppercase);
		IFengine.setToleranceLevel(toleranceLevel);

		for (int currentWordLength = startWordLength; currentWordLength <= endWordLength; currentWordLength++)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("\r\nCurrent Length\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(currentWordLength, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(" ", false);

			displayProgressBar();
			IFengine.setCurrentWordLength(currentWordLength);
			char oldLetter = NULL;

			do
			{
				currentPassword = IFengine.getNextWord();
				// only update progress bar when the last letter changes
				if (oldLetter != currentPassword[0])
				{
					updateProgressBar();
					oldLetter = currentPassword[0];
				}

				if (generateCRC(currentPassword, currentWordLength) == requiredpasswordCRC)
				{
					foundPassword = true;
					break;
				}

			} while (currentPassword[0] != NULL);

			completeProgressBar();
			if (foundPassword) break;
		}

		if (foundPassword) displayResults(currentPassword, true);
		IFengine.destroyCharacterSet();
	}
	else if (selectedSearchMethod == SEARCH_DICTIONARY)
	{
		unsigned long currentWordLength;
		if (!DictEngine.initialiseDictionaryEngine(dictionaryName, DICTIONARYENGINE_FILTER_ALL))
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
		}

		ULib.ConsoleLibrary.displayConsoleMessage("\r\nSearching Dictionary...", false);

		do
		{
			currentPassword = DictEngine.getNextWord(&currentWordLength);

			if (generateCRC(currentPassword, currentWordLength) == requiredpasswordCRC)
			{
				foundPassword = true;
				break;
			}

		} while (currentPassword[0] != NULL);

		if (foundPassword) displayResults(currentPassword, true);
		DictEngine.destroyDictionaryEngine();
	}

	ULib.ConsoleLibrary.showConsoleCursor();
	if (!foundPassword) displayResults(currentPassword, false);
}

/////////////////////////////////////////////////
// method to check if the currentPassword is   //
// the correct one based on comparing the CRCs //
/////////////////////////////////////////////////
void checkPassword(char* currentPassword, int currentPasswordLength, bool checkOnlyOnePassword)
{
	currentpasswordCRC = generateCRC(currentPassword, currentPasswordLength);

	if (checkOnlyOnePassword)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password\t: '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(currentPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'");
		ULib.ConsoleLibrary.displayConsoleMessage("Expected CRC\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(currentpasswordCRC);
		ULib.ConsoleLibrary.displayConsoleMessage("Required CRC\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(requiredpasswordCRC, false, false);
		if (currentpasswordCRC == requiredpasswordCRC) displayResults(currentPassword, true);
		else displayResults(currentPassword, false);
	}
}

////////////////////////////////////////////////
// Display the results of the single password //
// check or the brute-force approach          //
////////////////////////////////////////////////
void displayResults(char* foundPassword, bool passwordFound)
{
	ULib.ConsoleLibrary.displayConsoleMessage("\r\n\r\n", false);

	if (passwordFound)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password Found\t: '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(foundPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'");
		MessageBeep(MB_ICONEXCLAMATION);
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}
}

///////////////////////////////////////////////
// Perform system benchmark for the password //
// checking engine (passwords per seconds)   //
///////////////////////////////////////////////
void performBenchmark(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Benchmarking...", false);

	BFengine.initialiseCharacterSet(true, true, true, false);
	BFengine.setCurrentWordLength(5);

	double startTime, endTime, elapsedTime;
	int counter = 0;
	int iterations = 10000000;
	char* currentPassword;
	startTime = GetTickCount();

	do
	{
		counter++;
		currentPassword = BFengine.getNextWord();
		checkPassword(currentPassword, 5, false);

	} while (counter < iterations);

	endTime = GetTickCount();
	BFengine.destroyCharacterSet();

	elapsedTime = (endTime - startTime) / 1000;
	char* benchmarkStat = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
	wsprintf(benchmarkStat, " %d passwords/sec (approx)", (unsigned long)(iterations / elapsedTime));
	ULib.ConsoleLibrary.displayConsoleMessage(benchmarkStat);
	ULib.HeapLibrary.freePrivateHeap(benchmarkStat);
}

////////////////////////////////////////////////////////////////
// Display the valid arguments for executing this application //
////////////////////////////////////////////////////////////////
void displayOptions(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tISjava2 [-bruteforce <start> <end> <charset> <required crc>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t        [-intelliforce <start> <end> <charset> <tolerance level> <crc>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t        [-dictionary <dictionary filename> <required crc>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t        [-single <password> <required crc>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t        [-benchmark]");
	ULib.ProcessLibrary.exitProcessHandler();
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("InstallShield Java2 Password Cracker", "1.20", 1999);
	initialiseCRCTable();

	if (argc > 1)
	{
		if ((ULib.StringLibrary.isStringMatch("-bruteforce", argv[1])) && (argc >= 6))
		{
			startWordLength = ULib.StringLibrary.getDWORDFromString(argv[2]);
			endWordLength = ULib.StringLibrary.getDWORDFromString(argv[3]);
			for (int index = 0; index < ULib.StringLibrary.getStringLength(argv[4]); index++)
			{
				if (((argv[4])[index]) == 'a') useLowercase = true;
				if (((argv[4])[index]) == 'A') useUppercase = true;
				if (((argv[4])[index]) == '1') useNumbers = true;
				if (((argv[4])[index]) == '!') useSpecial = true;
			}

			requiredpasswordCRC = ULib.StringLibrary.getDWORDFromString(argv[5]);
			selectedSearchMethod = SEARCH_BRUTEFORCE;
			beginPasswordSearch();
		}
		else if ((ULib.StringLibrary.isStringMatch("-intelliforce", argv[1])) && (argc >= 7))
		{
			startWordLength = ULib.StringLibrary.getDWORDFromString(argv[2]);
			endWordLength = ULib.StringLibrary.getDWORDFromString(argv[3]);
			for (int index = 0; index < ULib.StringLibrary.getStringLength(argv[4]); index++)
			{
				if (((argv[4])[index]) == 'a') useLowercase = true;
				if (((argv[4])[index]) == 'A') useUppercase = true;
			}

			toleranceLevel = ULib.StringLibrary.getDWORDFromString(argv[5]);
			requiredpasswordCRC = ULib.StringLibrary.getDWORDFromString(argv[6]);
			selectedSearchMethod = SEARCH_INTELLIFORCE;
			beginPasswordSearch();
		}
		else if ((ULib.StringLibrary.isStringMatch("-dictionary", argv[1])) && (argc >= 4))
		{
			dictionaryName = argv[2];
			requiredpasswordCRC = ULib.StringLibrary.getDWORDFromString(argv[3]);
			selectedSearchMethod = SEARCH_DICTIONARY;
			beginPasswordSearch();
		}
		else if ((ULib.StringLibrary.isStringMatch("-single", argv[1])) && (argc >= 4))
		{
			char* testPassword = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
			ULib.StringLibrary.copyString(testPassword, argv[2]);
			int testPasswordLength = ULib.StringLibrary.getStringLength(testPassword);
			requiredpasswordCRC = ULib.StringLibrary.getDWORDFromString(argv[3]);
			checkPassword(testPassword, testPasswordLength, true);
		}
		else if (ULib.StringLibrary.isStringMatch("-benchmark", argv[1]))
		{
			performBenchmark();
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