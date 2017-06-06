///////////////////////////////////////////////
// MYOB Accounting Projects Password Cracker //
// (C)thewd, thewd@hotmail.com               //
///////////////////////////////////////////////
///////////////////////////////////////////////
// Recovers passwords for MYOB projects      //
// up to 11 characters                       //
//                                           //
// Tested with...                            //
// - v8.00                                   //
// - v9.00                                   //
// - v10.00                                  //
// - v11.00                                  //
// - v12.00                                  //
// - v13.00                                  //
///////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the MYOB Project",
	"Doesn't appear to be a supported MYOB Project",
	"Unable to find any encrypted passwords",
	"Unable to recover a suitable password"
};

bool DEBUG_MODE = false;

///////////////////////////////////////////////////
// decrypted the password and returns the result //
///////////////////////////////////////////////////
unsigned char* recoverPassword(unsigned char* encryptedPassword)
{
	unsigned char* decryptedPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char decryptionValues[4] = {0x73, 0x6A, 0x63, 0x6B};

	// decrypt the password
	for (int i = 0; i < 11; i++)
	{
		if (encryptedPassword[i] == NULL)
		{
			decryptedPassword[i] = NULL;
			break;
		}

		decryptedPassword[i] = (unsigned char)(encryptedPassword[i] + decryptionValues[i % 4]);
	}

	return decryptedPassword;
}

///////////////////////////////////////////////
// Load the file, determine the location of  //
// the passwords and attempt to recover them //
///////////////////////////////////////////////
void parseFileAndFindPassword(char* fileName)
{
	// read the file header
	unsigned char* fileHeader = ULib.FileLibrary.readBufferFromFile(fileName, 0x1300);
	if (fileHeader == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	unsigned long headerId1 = ULib.StringLibrary.getDWORDFromBuffer(fileHeader, 0x12C9);
	unsigned long headerId2 = ULib.StringLibrary.getDWORDFromBuffer(fileHeader, 0x12CD);
	unsigned long passwordSectionOffset = ULib.StringLibrary.getDWORDFromBuffer(fileHeader, 0x12D1);
	ULib.HeapLibrary.freePrivateHeap(fileHeader);

	// check for compliance
	if ((headerId1 != 0xFFFFFFFF) || (headerId2 != 0x00002000)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// display section offset
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Section Offset\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordSectionOffset, true, true);
	}

	// read password section header
	unsigned char* passwordSectionHeader = ULib.FileLibrary.readBufferFromFile(fileName, NULL, passwordSectionOffset, 0x30);
	if (passwordSectionHeader == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// unable to find password section - search whole file
	if ((ULib.StringLibrary.getWORDFromBuffer(passwordSectionHeader) != 0xFAFA) ||
		(ULib.StringLibrary.getWORDFromBuffer(passwordSectionHeader, 0x1E) != 0x001C) ||
		(ULib.StringLibrary.getWORDFromBuffer(passwordSectionHeader, 0x26) != 0x00))
	{
		unsigned long fileBufferSize = 0;
		unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileBufferSize);
		passwordSectionOffset = 0;

		// find password section
		for (unsigned long i = (fileBufferSize - 0x22); i > 0; i--)
		{
			if (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, i) == 0xFAFA)
			{
				headerId2 = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, i + 2);
				if ((headerId2 >= 0x02A2) && (headerId2 < 0x0300) && (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, i + 0x1E) == 0x1C) && (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, i + 0x26) == 0x00))
				{
					// display section offset
					if (DEBUG_MODE)
					{
						ULib.ConsoleLibrary.displayConsoleMessage("Section Offset\t: 0x", false);
						ULib.ConsoleLibrary.displayConsoleMessage(i, true, true);
					}

					// update password section offset and section header
					passwordSectionOffset = i;
					ULib.HeapLibrary.freePrivateHeap(passwordSectionHeader);
					passwordSectionHeader = ULib.FileLibrary.readBufferFromFile(fileName, NULL, passwordSectionOffset, 0x30);
					break;
				}
			}
		}

		ULib.HeapLibrary.freePrivateHeap(fileBuffer);
		if (passwordSectionOffset <= 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	unsigned long passwordSectionSize = ULib.StringLibrary.getDWORDFromBuffer(passwordSectionHeader, 0x06);
	unsigned long passwordCount = ULib.StringLibrary.getDWORDFromBuffer(passwordSectionHeader, 0x2A);
	ULib.HeapLibrary.freePrivateHeap(passwordSectionHeader);

	// display password section information
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Section Size\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordSectionSize, true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Password Count\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordCount, false, true);
		ULib.ConsoleLibrary.displayConsoleMessage("", true);
	}

	// unable to find any encrypted passwords
	if (passwordCount == 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// read password section
	unsigned char* passwordSectionBuffer = ULib.FileLibrary.readBufferFromFile(fileName, NULL, passwordSectionOffset, passwordSectionSize);
	if (passwordSectionBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	unsigned char* encryptedPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned long currentOffset = 0x62;
	unsigned long recoveredPasswords = 0;

	// recover all found passwords
	for (unsigned long i = 0; i < passwordCount; i++)
	{
		if (passwordSectionBuffer[currentOffset] == NULL) break;

		CopyMemory(encryptedPassword, &passwordSectionBuffer[currentOffset], 20);
		unsigned char* decryptedPassword = recoverPassword(encryptedPassword);

		if (decryptedPassword[0] == NULL)
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
		}
		else
		{
			ULib.ConsoleLibrary.displayConsoleMessage(((i == 0) ? "Master Password\t: '" : "Sub Password\t: '"), false);
			ULib.ConsoleLibrary.displayConsoleMessage((char*)decryptedPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'");
			if (i == 0) MessageBeep(MB_ICONEXCLAMATION);
			recoveredPasswords++;
		}

		ULib.HeapLibrary.freePrivateHeap(decryptedPassword);
		currentOffset += 0x80;
	}

	if (recoveredPasswords == 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	ULib.HeapLibrary.freePrivateHeap(passwordSectionBuffer);
	ULib.HeapLibrary.freePrivateHeap(encryptedPassword);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("MYOB Accounting Projects Password Cracker", "1.02", 2000);

	// if arguments are passed
	if (argc > 1)
	{
		DEBUG_MODE = ULib.ProcessLibrary.isDebugModeEnabled();
		parseFileAndFindPassword(argv[1]);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tMYOBcrk <MYOB project file>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}