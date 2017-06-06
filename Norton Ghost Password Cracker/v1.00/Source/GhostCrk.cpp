/////////////////////////////////////////////////
// Norton Ghost Password Cracker               //
// (C)thewd, thewd@hotmail.com                 //
/////////////////////////////////////////////////
/////////////////////////////////////////////////
// Retrieves passwords for Norton Ghost images //
// that have been password protected           //
// (Ghost v4.1a+ supports password protection) //
//                                             //
// - v4.1a+ (maximum 10 characters)            //
// - v5.1c+ (maximum 11 characters)            //
// - v7.00+ (maximum 10 characters)            //
//                                             //
// Tested with...                              //
// - v5.0e, v5.1b, v5.1c, v5.1d (2000)         //
// - v6.0a, v6.03, v6.50                       //
// - v7.00 (2002), v7.50, v7.60 (2003), v7.70  //
// - v8.00                                     //
// - v9.00 (not supported)                     //
/////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the Norton Ghost image",
	"Unable to find any encrypted password",
	"Doesn't appear to be a supported Norton Ghost image",
	"Unable to recover a suitable password",
	"Unable to parse span extent file. Use base Ghost image",
	"Unable to proceed. Norton Ghost v9.00 images are not supported{CRLF}(see PowerQuest Drive Image Password Cracker)"
};

unsigned long crcPasswordTable[256];
unsigned long crcDecryptionTable[256];
unsigned char encryptedBytes[20];

void displayResults(char* foundPassword, bool passwordFound);

//////////////////////////////////////////////////
// create and initialise the Password CRC table //
//////////////////////////////////////////////////
void createPasswordTable(void)
{
	for (int i = 255; i >= 0; i--)
	{
		int j = i << 24;

		for (int k = 8; k > 0; k--)
		{
			if (j < 0) j = j << 1 ^ 0x4C11DB7;
			else j <<= 1;
		}

		crcPasswordTable[i] = j;
	}
}

////////////////////////////////////////////////////////
// creates a table of CRC values that is used to      //
// decrypt the password header within the ghost image //
////////////////////////////////////////////////////////
void createDecryptionTable(void)
{
	for (int i = 0; i <= 255; i++)
	{
		unsigned long x1 = 0x7A2;
		unsigned long x2 = (i * 0x100);

		for (int j = 8; j > 0; j--)
		{
			if (!((x1 ^ x2) & 0x8000)) __asm shl x1, 1
			else x1 = (x1 + x1) ^ 0x1021;
			__asm shl x2, 1
		}

		crcDecryptionTable[i] = (x1 & 0x0000FFFF);
	}
}

///////////////////////////////////////////////////////
// Generate the CRC for the password currentPassword //
// (Only concerned with the last 2 bytes)            //
///////////////////////////////////////////////////////
unsigned long __inline generatePasswordCRC(char* currentPassword, int currentPasswordLength)
{
	unsigned long baseCRC = 0;
	unsigned long tempCRC;

	for (int index = 0; index < currentPasswordLength; index++)
	{
		tempCRC = baseCRC;
		__asm shr tempCRC, 0x18
		baseCRC = (baseCRC << 0x08) ^ crcPasswordTable[currentPassword[index] ^ tempCRC];
	}

	tempCRC = baseCRC;
	__asm shr baseCRC, 0x10
	return ((baseCRC ^ tempCRC) & 0x0000FFFF);
}

////////////////////////////////////////////////
// checks to see if the current password CRC  //
// generates the text string 'BinaryResearch' //
// out of the encrypted bytes                 //
////////////////////////////////////////////////
bool __inline checkPasswordCRC(unsigned long passwordCRC)
{
	char* decryptedBytes = (char*)ULib.HeapLibrary.allocPrivateHeap(20);
	unsigned long x1, x2;

	x1 = passwordCRC;

	for (int i = 0; i < 14; i++)
	{
		x1 = (x1 & 0x0000FFFF);
		x2 = x1;
		__asm shr x2, 0x08
		x1 = (x1 & 0x000000FF);
		decryptedBytes[i] = (char)(encryptedBytes[i] ^ x1); // decryptedChar
		x1 = (crcDecryptionTable[x2 ^ encryptedBytes[i]] ^ (x1 << 8)); // new CRC value
	}

	if (ULib.StringLibrary.isStringMatch(decryptedBytes, "BinaryResearch"))
	{
		ULib.HeapLibrary.freePrivateHeap(decryptedBytes);
		return true;
	}
	else
	{
		ULib.HeapLibrary.freePrivateHeap(decryptedBytes);
		return false;
	}
}

///////////////////////////////////////////////
// Begin the search for the correct password //
///////////////////////////////////////////////
void findPassword(void)
{
	char* currentWord = NULL;
	bool foundPassword = false;

	// determine what the last bytes must
	// be to decrypt the encrypted byte to 'B'
	unsigned char lastByte = (unsigned char)(encryptedBytes[0] ^ 'B');

	ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (1/2)...", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 31, false);

	ULib.ConsoleLibrary.hideConsoleCursor();

	IntelliForceEngine IFengine;
	IFengine.initialiseCharacterSet(false, true);
	IFengine.setToleranceLevel(7);

	for (int currentWordLength = 1; currentWordLength <= 6; currentWordLength++)
	{
		IFengine.setCurrentWordLength(currentWordLength);

		do
		{
			currentWord = IFengine.getNextWord();
			unsigned long currentCRC = generatePasswordCRC(currentWord, currentWordLength);
			if ((currentCRC & 0x000000FF) == lastByte)
			{
				if (checkPasswordCRC(currentCRC))
				{
					foundPassword = true;
					break;
				}
			}

		} while (currentWord[0] != NULL);

		if (foundPassword) break;
	}

	if (foundPassword) displayResults(currentWord, true);
	IFengine.destroyCharacterSet();

	if (!foundPassword)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (2/2)...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 31, false);

		BruteForceEngine BFengine;
		BFengine.initialiseCharacterSet(false, true, true, false);

		for (int currentWordLength = 1; currentWordLength <= 6; currentWordLength++)
		{
			BFengine.setCurrentWordLength(currentWordLength);

			do
			{
				currentWord = BFengine.getNextWord();
				unsigned long currentCRC = generatePasswordCRC(currentWord, currentWordLength);
				if ((currentCRC & 0x000000FF) == lastByte)
				{
					if (checkPasswordCRC(currentCRC))
					{
						foundPassword = true;
						break;
					}
				}

			} while (currentWord[0] != NULL);

			if (foundPassword) break;
		}

		if (!foundPassword) displayResults(currentWord, false);
		else displayResults(currentWord, true);

		BFengine.destroyCharacterSet();
	}

	ULib.ConsoleLibrary.showConsoleCursor();
}

/////////////////////////////////////////////////////
// Display the results of the brute-force approach //
/////////////////////////////////////////////////////
void displayResults(char* foundPassword, bool passwordFound)
{
	if (passwordFound)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password Found : '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(foundPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'                    ");
		MessageBeep(MB_ICONEXCLAMATION);
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	}
}

//////////////////////////////////////////////////////
// Opens the image and read in the encrypted header //
//////////////////////////////////////////////////////
void parseImageFile(char* fileName)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, 0x800);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// check for Norton Ghost v9.00+ images
	if ((fileBuffer[0] == 0x78) && (fileBuffer[1] == 0x89) && (fileBuffer[2] == 0x26) && (fileBuffer[3] == 0x12)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);
	// check image CRC
	if ((fileBuffer[0] != 0xFE) || (fileBuffer[1] != 0xEF)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	// check for span extent file
	if (fileBuffer[2] == 0x09) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	// check for no password
	if (fileBuffer[0x0B] == 0x00) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	//bool hasBootPart = (fileBuffer[0x31] != 0x00);
	//bool explorerCompiled = (fileBuffer[0x32] != 0x00);
	//bool biosLocked = (fileBuffer[0x36] != 0x00);

	encryptedBytes[0x00] = fileBuffer[0x0C];
	encryptedBytes[0x01] = fileBuffer[0x0D];
	encryptedBytes[0x02] = fileBuffer[0x0E];
	encryptedBytes[0x03] = fileBuffer[0x0F];
	encryptedBytes[0x04] = fileBuffer[0x10];
	encryptedBytes[0x05] = fileBuffer[0x11];
	encryptedBytes[0x06] = fileBuffer[0x12];
	encryptedBytes[0x07] = fileBuffer[0x13];
	encryptedBytes[0x08] = fileBuffer[0x14];
	encryptedBytes[0x09] = fileBuffer[0x15];
	encryptedBytes[0x0A] = fileBuffer[0x16];
	encryptedBytes[0x0B] = fileBuffer[0x17];
	encryptedBytes[0x0C] = fileBuffer[0x18];
	encryptedBytes[0x0D] = fileBuffer[0x19];
	encryptedBytes[0x0E] = fileBuffer[0x1A];
	encryptedBytes[0x0F] = 0x00;
	encryptedBytes[0x10] = 0x00;

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Norton Ghost Password Cracker", "1.00", 1999);

	if (argc > 1)
	{
		createPasswordTable();
		createDecryptionTable();
		parseImageFile(argv[1]);
		findPassword();
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tGhostCrk <ghost image file>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}