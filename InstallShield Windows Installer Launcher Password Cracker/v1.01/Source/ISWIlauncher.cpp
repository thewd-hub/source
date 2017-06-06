///////////////////////////////////////////////////////////////
// InstallShield Windows Installer Launcher Password Cracker //
// (C)thewd, thewd@hotmail.com                               //
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// Recovers the setup launcher password for installations    //
// based on Windows Installer                                //
//                                                           //
// Tested with...                                            //
// - InstallShield Express v4.00                             //
// - InstallShield Professional v7.00                        //
// - InstallShield Developer v8.00                           //
///////////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"

char* errorMessages[] =
{
	"Unable to open or read the InstallShield launcher file",
	"Doesn't appear to be a supported InstallShield launcher file",
	"Unable to find any encrypted password",
	"Unable to recover a suitable password"
};

CUtilitiesLibrary ULib;
unsigned long crcTable[256];

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
char* findPassword(unsigned long requiredpasswordCRC)
{
	char* currentWord = NULL;
	char* recoveredPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
	bool foundPassword = false;

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

	ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (2/2)...", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 31, false);

	BruteForceEngine BFengine;
	BFengine.initialiseCharacterSet(true, true, true, false);

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

//////////////////////////////////////////////
// load the file, determine the location of //
// the password and attempt to recover it   //
//////////////////////////////////////////////
void parseFileAndFindPassword(char* fileName)
{
	unsigned long fileSize = ULib.FileLibrary.getFileSize(fileName);
	if (fileSize <= 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	bool foundInfoHeading = false;
	bool foundKeyHeading = false;
	unsigned char* encryptedPasswordString = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(20);

	unsigned long bufferBlockSize = 5120000;
	unsigned long currentFileIndex = 0;
	unsigned char* fileBuffer = NULL;

	// parse the executable file in blocks
	while (currentFileIndex < fileSize)
	{
		if (bufferBlockSize >= (fileSize - currentFileIndex)) bufferBlockSize = fileSize - currentFileIndex;
		fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, NULL, currentFileIndex, bufferBlockSize);
		if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

		for (unsigned long i = 0; i < bufferBlockSize; i++)
		{
			if ((fileBuffer[i + 0] == '[') &&
				(fileBuffer[i + 1] == 'I') &&
				(fileBuffer[i + 2] == 'n') &&
				(fileBuffer[i + 3] == 'f') &&
				(fileBuffer[i + 4] == 'o') &&
				(fileBuffer[i + 5] == ']'))
			{
				foundInfoHeading = true;
			}

			if ((foundInfoHeading) &&
				(fileBuffer[i + 0] == '[') &&
				(fileBuffer[i + 1] == 'K') &&
				(fileBuffer[i + 2] == 'E') &&
				(fileBuffer[i + 3] == 'Y') &&
				(fileBuffer[i + 4] == ']'))
			{
				foundKeyHeading = true;

				unsigned long j = 0;
				// store the encrypted password
				for (i += 16; i < (i + 20); i++)
				{
					if (fileBuffer[i] < 0x20) break;
					encryptedPasswordString[j] = fileBuffer[i];
					j++;
				}

				break;
			}
		}

		ULib.HeapLibrary.freePrivateHeap(fileBuffer);
		currentFileIndex += bufferBlockSize;
		if (currentFileIndex < fileSize) currentFileIndex -= 128;

		if ((foundInfoHeading) && (foundKeyHeading)) break;
	}

	// error checking
	if (!foundInfoHeading) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	if (!foundKeyHeading) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	if (encryptedPasswordString[0] == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	if (ULib.StringLibrary.getStringLength((char*)encryptedPasswordString) <= 8) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// convert password to integer
	unsigned long encryptedPasswordCRC = ULib.StringLibrary.getDWORDFromString((char*)encryptedPasswordString);
	// find password
	initialiseCRCTable();

	ULib.ConsoleLibrary.displayConsoleMessage("Password CRC\t: ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((char*)encryptedPasswordString);
	ULib.ConsoleLibrary.displayConsoleMessage("");

	char* foundPassword = findPassword(encryptedPasswordCRC);

	// display results
	if (foundPassword[0] != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password Found\t: '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(foundPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'                   ");
		MessageBeep(MB_ICONEXCLAMATION);
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	}

	ULib.HeapLibrary.freePrivateHeap(foundPassword);
	ULib.HeapLibrary.freePrivateHeap(encryptedPasswordString);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("InstallShield Windows Installer Launcher Password Cracker", "1.01", 2001);

	if (argc > 1) parseFileAndFindPassword(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tISWIlauncher <setup launcher>");

	ULib.ProcessLibrary.exitProcessHandler();
}