//////////////////////////////////////////////////////
// InstallShield InstallFromTheWeb Password Cracker //
// (C)thewd, thewd@hotmail.com                      //
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
// Recover passwords for installations created with //
// InstallShield InstallFromTheWeb v2.xx & v3.xx    //
// (v1.xx doesn't provide a facility for passwords) //
//                                                  //
// Tested with...                                   //
// - v2.20, v2.30, v3.00, v3.10                     //
//////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"

char* errorMessages[] =
{
	"Doesn't appear to be a valid InstallFromTheWeb .wis file",
	"Unable to find any encrypted password crc within the file",
	"Unable to recover a suitable password"
};

CUtilitiesLibrary ULib;

unsigned long crcTable[256];
unsigned long requiredpasswordCRC;

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
	unsigned long baseCRC = 0xFFFFFFFF;
	unsigned long newCRC;

	for (int index = 0; index < currentPasswordLength; index++)
	{
		newCRC = baseCRC;
		__asm shr newCRC, 8
		baseCRC =  newCRC ^ crcTable[(baseCRC & 0xFF) ^ currentPassword[index]];
	}

	__asm not baseCRC
	int byte1 = baseCRC & 0xFF;
	byte1 = (_rotl(byte1, 3) ^ 0x74) & 0xFF;
	int byte2 = baseCRC >> 8 & 0xFF;
	byte2 = (_rotr(byte2, 5) ^ 0x74) & 0xFF;
	int byte3 = baseCRC >> 16 & 0xFF;
	byte3 = (_rotl(byte3, 7) ^ 0x74) & 0xFF;
	int byte4 = baseCRC >> 24 & 0xFF;
	byte4 = (_rotr(byte4, 3) ^ 0x74) & 0xFF;

	return (byte1 * 0x100) + byte2 + (byte3 * 0x1000000) + (byte4 * 0x10000);
}

///////////////////////////////////////////////////
// Begin the search for the correct password.    //
// Initially uses the IntelliForce engine to     //
// recover a password up to 6 characters If this //
// fails then proceed with the BruteForce engine //
///////////////////////////////////////////////////
char* findPassword(void)
{
	char* currentWord = NULL;
	bool foundPassword = false;
	char* recoveredPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();

	ULib.ConsoleLibrary.hideConsoleCursor();
	ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (1/2)...", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 31, false);

	IntelliForceEngine IFengine;
	IFengine.initialiseCharacterSet(true, false);
	IFengine.setToleranceLevel(7);

	for (int currentWordLength = 1; currentWordLength <= 6; currentWordLength++)
	{
		IFengine.setCurrentWordLength(currentWordLength);

		do
		{
			currentWord = IFengine.getNextWord();
			if (generateCRC(currentWord, currentWordLength) == requiredpasswordCRC)
			{
				ULib.StringLibrary.copyString(recoveredPassword, currentWord);
				foundPassword = true;
				break;
			}

		} while (currentWord[0] != NULL);

		if (foundPassword) break;
	}

	IFengine.destroyCharacterSet();
	if (foundPassword) return recoveredPassword;

	BruteForceEngine BFengine;
	BFengine.initialiseCharacterSet(true, true, true, false);

	ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (2/2)...", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 31, false);

	for (int currentWordLength = 1; currentWordLength <= 4; currentWordLength++)
	{
		BFengine.setCurrentWordLength(currentWordLength);

		do
		{
			currentWord = BFengine.getNextWord();
			if (generateCRC(currentWord, currentWordLength) == requiredpasswordCRC)
			{
				ULib.StringLibrary.copyString(recoveredPassword, currentWord);
				foundPassword = true;
				break;
			}

		} while (currentWord[0] != NULL);

		if (foundPassword) break;
	}

	BFengine.destroyCharacterSet();
	return recoveredPassword;
}

//////////////////////////////////////////////////////
// Begin the process of finding a suitable password //
//////////////////////////////////////////////////////
void beginPasswordSearch(char* passwordCRC)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Password CRC\t: ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(passwordCRC);
	ULib.ConsoleLibrary.displayConsoleMessage("");

	requiredpasswordCRC = ULib.StringLibrary.getDWORDFromString(passwordCRC);
	if (requiredpasswordCRC <= 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	char* recoveredPassword = findPassword();
	if (recoveredPassword[0] == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	ULib.ConsoleLibrary.displayConsoleMessage("Password Found\t: '", false);
	ULib.ConsoleLibrary.displayConsoleMessage(recoveredPassword, false);
	ULib.ConsoleLibrary.displayConsoleMessage("'                   ");
	MessageBeep(MB_ICONEXCLAMATION);
	ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("InstallShield InstallFromTheWeb Password Cracker", "1.00", 1999);
	bool displayUsage = true;

	if (argc > 2)
	{
		initialiseCRCTable();

		if (ULib.StringLibrary.isStringMatch("-crc", argv[1]))
		{
			beginPasswordSearch(argv[2]);
			displayUsage = false;
		}
		else if (ULib.StringLibrary.isStringMatch("-wis", argv[1]))
		{
			// get the full path to the fileName - argv[2]
			char* fullpathFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
			GetFullPathName(argv[2], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fullpathFilename, NULL);

			// check that the file is an InstallFromTheWeb WIS file
			char* iftwHeader = (char*)ULib.HeapLibrary.allocPrivateHeap();
			GetPrivateProfileString("InstallFromTheWeb", "Version", "NotFound", iftwHeader, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fullpathFilename);
			if ((iftwHeader == NULL) || (ULib.StringLibrary.isStringMatch(iftwHeader, "NotFound"))) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
			ULib.HeapLibrary.freePrivateHeap(iftwHeader);

			// check if the Password key is in the Options section
			// if not, then the installation is not password protected
			// if it is, then retrieve the CRC value
			char* passwordCRC = (char*)ULib.HeapLibrary.allocPrivateHeap();
			GetPrivateProfileString("Options", "Password", "NotFound", passwordCRC, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fullpathFilename);
			ULib.HeapLibrary.freePrivateHeap(fullpathFilename);
			if (ULib.StringLibrary.isStringMatch(passwordCRC, "NotFound")) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

			beginPasswordSearch(passwordCRC);
			ULib.HeapLibrary.freePrivateHeap(passwordCRC);
			displayUsage = false;
		}
	}

	// display usage
	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tISiftw [-crc <password crc>]");
		ULib.ConsoleLibrary.displayConsoleMessage("      \t       [-wis <configuraion filename>]");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}