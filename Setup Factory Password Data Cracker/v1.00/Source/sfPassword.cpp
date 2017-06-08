/////////////////////////////////////////////////
// Setup Factory Password Data Cracker         //
// (C)thewd, thewd@hotmail.com                 //
/////////////////////////////////////////////////
/////////////////////////////////////////////////
// Extracts and decrypts the password data     //
// information stored within the setup factory //
// configuration file - irsetup.dat            //
// Supports v5.0.0+. Earlier versions except   //
// one password which is stored as plaintext   //
// (http://www.indigorose.com)                 //
//                                             //
// Tested with...                              //
// - v5.0.1.6                                  //
// - v6.0.0.2                                  //
/////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Unable to open or read the configuration file",
	"Doesn't appear to be a supported configuration file",
	"Unable to find the encrypted password data section"
};

CUtilitiesLibrary ULib;

//////////////////////////////////////////////
// load the file, determine the location of //
// the password data and extract the data   //
//////////////////////////////////////////////
void parseFileAndFindPasswordDataSection(char* fileName)
{
	// read in file contents (checking for a supported file)
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileSize <= 1000) || (fileBuffer == NULL)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	//if ((fileBuffer[2] != 0xFF) || (fileBuffer[3] != 0xFF) || (fileBuffer[4] != 0x03)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// try to find the password data offset
	unsigned long passwordDataOffset = 0;
	for (unsigned long i = 10; i < fileSize - 20; i++)
	{
		if ((fileBuffer[i] == 0xFF) &&
			(fileBuffer[i + 1] == 0xFF) &&
			(fileBuffer[i + 2] == 0x01))
		{
			if (ULib.StringLibrary.isStringMatch((char*)&fileBuffer[i + 6], "CPasswordData", false, 13))
			{
				passwordDataOffset = i - 2;
				break;
			}
		}
	}

	// unable to find password data section
	if (passwordDataOffset <= 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// the number of password sets
	unsigned long numberOfPasswordSets = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, passwordDataOffset);
	ULib.ConsoleLibrary.displayConsoleMessage("Number of Password Sets : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(numberOfPasswordSets);
	passwordDataOffset += 21;

	// cycle through each password sets
	for (unsigned long i = 0; i < numberOfPasswordSets; i++)
	{
		// get password set information (name, number of passwords, etc)
		int passwordSetNameLength = fileBuffer[passwordDataOffset++];
		char* passwordSetName = (char*)ULib.HeapLibrary.allocPrivateHeap(passwordSetNameLength + 5);
		char* passwordSetFilename = (char*)ULib.HeapLibrary.allocPrivateHeap(passwordSetNameLength + 5);
		memcpy(passwordSetName, &fileBuffer[passwordDataOffset], passwordSetNameLength);
		wsprintf(passwordSetFilename, "%s.txt", passwordSetName);
		passwordDataOffset += passwordSetNameLength;
		int numberOfPasswords = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, passwordDataOffset);
		passwordDataOffset += 2;

		// no passwords
		if (numberOfPasswords <= 0)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Ignoring Password Set   - ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(passwordSetName, false);
			ULib.ConsoleLibrary.displayConsoleMessage(" (", false);
			ULib.ConsoleLibrary.displayConsoleMessage(numberOfPasswords, false, false);
			ULib.ConsoleLibrary.displayConsoleMessage(")");
			passwordDataOffset += 2;
			continue;
		}

		ULib.ConsoleLibrary.displayConsoleMessage("Extracting Password Set - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordSetName, false);
		ULib.ConsoleLibrary.displayConsoleMessage(" (", false);
		ULib.ConsoleLibrary.displayConsoleMessage(numberOfPasswords, false, false);
		ULib.ConsoleLibrary.displayConsoleMessage(")");

		unsigned char* decryptedPasswords = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(numberOfPasswords * 300);
		int decryptedPasswordsIndex = 0;

		// cycle through each password (decrypting)
		for (int j = 0; j < numberOfPasswords; j++)
		{
			int currentPasswordLength = fileBuffer[passwordDataOffset++];
			for (int k = 0; k < currentPasswordLength; k++) decryptedPasswords[decryptedPasswordsIndex++] = (unsigned char)(fileBuffer[passwordDataOffset++] ^ 0x07);
			decryptedPasswords[decryptedPasswordsIndex++] = 0x0D;
			decryptedPasswords[decryptedPasswordsIndex++] = 0x0A;
		}

		// save the decrypted passwords
		ULib.FileLibrary.writeBufferToFile(passwordSetFilename, decryptedPasswords, decryptedPasswordsIndex);
		passwordDataOffset += 2;

		ULib.HeapLibrary.freePrivateHeap(decryptedPasswords);
		ULib.HeapLibrary.freePrivateHeap(passwordSetName);
		ULib.HeapLibrary.freePrivateHeap(passwordSetFilename);
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Setup Factory Password Data Cracker", "1.00", 2002);

	if (argc > 1) parseFileAndFindPasswordDataSection(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tsfPassword <setup configuration file>");

	ULib.ProcessLibrary.exitProcessHandler();
}