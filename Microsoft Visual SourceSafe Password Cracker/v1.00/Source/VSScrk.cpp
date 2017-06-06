/////////////////////////////////////////////////////////
// Microsoft Visual SourceSafe Password Cracker        //
// (C)thewd, thewd@hotmail.com                         //
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// Recovers passwords up to a maximum length of 15     //
// characters for Microsoft Visual SourceSafe accounts //
/////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the Visual SourceSafe User Database",
	"Error reading data from the Visual SourceSafe User Database",
	"Doesn't appear to be a supported Visual SourceSafe User Database",
	"Unable to find a supported Visual SourceSafe CRC Table Database"
};

#define NUMBER_OF_CRC_ENTRIES	40000
char* crcTablePasswords[NUMBER_OF_CRC_ENTRIES];

/////////////////////////////////////////////////
// extract the crc table resource from this    //
// executable and store the corresponding data //
/////////////////////////////////////////////////
void getAndStoreCRCTableResource(void)
{
	unsigned long resourceSize = 0;
	unsigned char* crcTableResource = ULib.FileLibrary.getResourceFromFile(NULL, "CRCTABLE", RT_RCDATA, &resourceSize);
	if ((crcTableResource == NULL) || (resourceSize <= 100)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	// check for sourcesafe crc table database
	if (!ULib.StringLibrary.isStringMatch((char*)crcTableResource, "VSSCRCTABLE", false, 11)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	// check version for the crc table database
	unsigned long versionCrcTable = ULib.StringLibrary.getDWORDFromBuffer(crcTableResource, 0x0C);
	if (versionCrcTable != 1) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	// empty all entries
	for (unsigned int i = 0; i < NUMBER_OF_CRC_ENTRIES; i++) crcTablePasswords[i] = NULL;

	char* tempString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned short currentCRC = 0;

	// store the crc entries and corresponding passwords
	for (unsigned int i = 0; i < (resourceSize - 0x10); i++)
	{
		currentCRC = ULib.StringLibrary.getWORDFromBuffer(crcTableResource, (0x10 + i));
		i += 2;
		ULib.StringLibrary.copyString(tempString, (char*)&crcTableResource[0x10 + i]);
		i += ULib.StringLibrary.getStringLength(tempString);

		// abort on invalid entry
		if ((currentCRC < 0x4D00) && (currentCRC >= 0x7000)) break;

		// store entry
		if (crcTablePasswords[currentCRC] == NULL)
		{
			crcTablePasswords[currentCRC] = (char*)ULib.HeapLibrary.allocPrivateHeap(20);
			ULib.StringLibrary.copyString(crcTablePasswords[currentCRC], tempString);
		}
	}
}

//////////////////////////////////////////////////////////
// parse this user database file, determine the format, //
// and display all the accounts and decrypted password  //
//////////////////////////////////////////////////////////
void parseFileAndDecrypt(char* fileName)
{
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize < 300)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// check for sourcesafe database
	if (!ULib.StringLibrary.isStringMatch((char*)fileBuffer, "UserManagement@", false, 15)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	// determine version
	int dbVersion = 0;
	switch (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, 0x20))
	{
		// v5.xx
		case 0x05:
		{
			dbVersion = 5;
			break;
		}
		// v6.xx
		case 0x07:
		case 0x08:
		case 0x09:
		{
			dbVersion = 6;
			break;
		}
		// unable to recognise version
		default:
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		}
	}

	// load and store crc table
	getAndStoreCRCTableResource();

	// get number of accounts
	int currentOffset = 0;
	if (dbVersion == 5) currentOffset = 0x126;
	else if (dbVersion == 6) currentOffset = 0x3C;
	unsigned short numberOfValidAccounts = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset);

	ULib.ConsoleLibrary.displayConsoleMessage("VSS Format : v", false);
	ULib.ConsoleLibrary.displayConsoleMessage((unsigned long)dbVersion, false, false);
	ULib.ConsoleLibrary.displayConsoleMessage(".00");
	ULib.ConsoleLibrary.displayConsoleMessage("Accounts   : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((unsigned long)numberOfValidAccounts);

	// jump to accounts
	if (dbVersion == 5) currentOffset = 0x136;
	else if (dbVersion == 6) currentOffset = 0x7C;

	for (int i = 0; i < numberOfValidAccounts; i++)
	{
		char* accountName = NULL;
		bool isReadOnly = false;
		unsigned short passwordCRC = 0;

		if (dbVersion == 5)
		{
			accountName = (char*)&fileBuffer[currentOffset];
			passwordCRC = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset + 0x10);
			isReadOnly = (fileBuffer[currentOffset + 0x14] == 0x01) ? true : false;
		}
		else if (dbVersion == 6)
		{
			unsigned long accountCRC1 = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, currentOffset);
			unsigned long accountCRC2 = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, currentOffset + 0x04);
			if ((accountCRC1 != 0x38) || ((accountCRC2 & 0x0000FFFF) != 0x5555)) break;

			accountName = (char*)&fileBuffer[currentOffset + 0x08];
			passwordCRC = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset + 0x28);
			isReadOnly = (fileBuffer[currentOffset + 0x2A] == 0x01) ? true : false;
		}

		ULib.ConsoleLibrary.displayConsoleMessage("\r\nAccount    : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(accountName);

		ULib.ConsoleLibrary.displayConsoleMessage("Password   : ", false);
		if ((passwordCRC != 0x6E90) && (passwordCRC < NUMBER_OF_CRC_ENTRIES) && (crcTablePasswords[passwordCRC] != NULL)) ULib.ConsoleLibrary.displayConsoleMessage(crcTablePasswords[passwordCRC]);
		else ULib.ConsoleLibrary.displayConsoleMessage("<blank>");

		ULib.ConsoleLibrary.displayConsoleMessage("Permission : ", false);
		if (isReadOnly) ULib.ConsoleLibrary.displayConsoleMessage("Read-Only");
		else ULib.ConsoleLibrary.displayConsoleMessage("Read-Write");

		if (dbVersion == 5) currentOffset += 0x20;
		else if (dbVersion == 6) currentOffset += 0x40;
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Microsoft Visual SourceSafe Password Cracker", "1.00", 2001);

	// if arguments are passed
	if (argc > 1) parseFileAndDecrypt(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tVSScrk <user management database>");

	ULib.ProcessLibrary.exitProcessHandler();
}