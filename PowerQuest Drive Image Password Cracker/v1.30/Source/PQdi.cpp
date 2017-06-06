//////////////////////////////////////////////////
// PowerQuest Drive Image Password Cracker      //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Generates valid passwords for PowerQuest     //
// Drive Image & V2i Protector partition images //
//                                              //
// Tested with...                               //
// - v2.00, v2.01, v2.02                        //
// - v3.00, v3.01, v3.02                        //
// - v4.00                                      //
// - v5.01                                      //
// - v6.00                                      //
// - v7.00                                      //
//                                              //
// History                                      //
// v1.30 (26th April 2002)                      //
// - password recovery for v4.xx - v7.xx images //
// v1.20 (17th November 2001)                   //
// - added recognition for v4 & v5 images       //
// v1.10 (10th May 1999)                        //
// - improve password recovery capabilities     //
// - added support for Drive Image v2.xx        //
// v1.00 (28th April 1999)                      //
// - only supports Drive Image v3.00            //
// - initial release                            //
//////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "PQdi.h"
#include "PQdi2.h"
#include "PQdi4.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the PowerQuest Image file",
	"Unable to find any encrypted password",
	"Doesn't appear to be a supported Image file",
	"Unable to recover a suitable password",
	"Unable to open or read the dictionary wordlist",
	"A search method hasn't been specified (see options)\r\n"
};

#define VERSION_UNKNOWN		0
#define VERSION_2			2
#define VERSION_3			3
#define VERSION_4			4
#define VERSION_5			5
#define VERSION_6			6
#define VERSION_7			7

char* partitionName;
char* filesystemType;
char* partitionType;
unsigned char* passwordCRC;
int imageVersionFormat;

///////////////////////////////////////////////////////
// Opens the image and reads in the encrypted header //
///////////////////////////////////////////////////////
void parseImageFile(char* fileName)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, 0x1000);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// check image CRC
	if ((fileBuffer[1] == 0x89) &&
		(fileBuffer[2] == 0x26) &&
		(fileBuffer[3] == 0x12))
	{
		// v2.xx - v3.xx
		if (fileBuffer[0] == 0x77) imageVersionFormat = VERSION_3;
		// v4.xx - v7.xx
		else if (fileBuffer[0] == 0x78) imageVersionFormat = VERSION_7;
		// unknown version
		else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	}
	else
	{
		// invalid image
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	}

	passwordCRC = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(100);

	// v2.xx - v3.xx
	if ((imageVersionFormat == VERSION_2) || (imageVersionFormat == VERSION_3))
	{
		partitionName = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
		filesystemType = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
		partitionType = (char*)ULib.HeapLibrary.allocPrivateHeap(100);

		// determine the partition and file system type
		determineImageType(fileBuffer[0x810], filesystemType, fileBuffer[0x814], partitionType);
		// determine the partition name
		ULib.StringLibrary.copyString(partitionName, (char*)&fileBuffer[0x870]);
		// copy the password crc
		memcpy(passwordCRC, (void*)&fileBuffer[0x85C], 4);
	}
	// v4.xx - v7.xx
	if ((imageVersionFormat == VERSION_4) || (imageVersionFormat == VERSION_5) || (imageVersionFormat == VERSION_6) || (imageVersionFormat == VERSION_7))
	{
		// copy the password crc
		memcpy(passwordCRC, (void*)&fileBuffer[0x18], 4);
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////////////////////////////////////////
// Display the valid arguments for executing this application //
////////////////////////////////////////////////////////////////
void displayOptions(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tPQdi <image filename>");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t     [-bruteforce <start length> <end length> <charset>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t     [-intelliforce <start> <end> <charset> <tolerance level>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t     [-dictionary <dictionary filename>]");
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("PowerQuest Drive Image Password Cracker", "1.30", 1999);

	if (argc > 1)
	{
		if (argc > 2)
		{
			if ((ULib.StringLibrary.isStringMatch("-bruteforce", argv[2])) && (argc >= 6))
			{
				bool useLowercase = false;
				bool useUppercase = false;
				bool useNumbers = false;
				bool useSpecial = false;
				for (int index = 0; index < ULib.StringLibrary.getStringLength(argv[5]); index++)
				{
					if (((argv[5])[index]) == 'a') useLowercase = true;
					if (((argv[5])[index]) == 'A') useUppercase = true;
					if (((argv[5])[index]) == '1') useNumbers = true;
					if (((argv[5])[index]) == '!') useSpecial = true;
				}

				setSearchProperties(SEARCH_BRUTEFORCE, ULib.StringLibrary.getDWORDFromString(argv[3]), ULib.StringLibrary.getDWORDFromString(argv[4]), 0,
					useLowercase, useUppercase, useNumbers, useSpecial, NULL);
			}
			else if ((ULib.StringLibrary.isStringMatch("-intelliforce", argv[2])) && (argc >= 7))
			{
				bool useLowercase = false;
				bool useUppercase = false;
				for (int index = 0; index < ULib.StringLibrary.getStringLength(argv[5]); index++)
				{
					if (((argv[5])[index]) == 'a') useLowercase = true;
					if (((argv[5])[index]) == 'A') useUppercase = true;
				}

				setSearchProperties(SEARCH_INTELLIFORCE, ULib.StringLibrary.getDWORDFromString(argv[3]),
					ULib.StringLibrary.getDWORDFromString(argv[4]), ULib.StringLibrary.getDWORDFromString(argv[6]),
					useLowercase, useUppercase, false, false, NULL);
			}
			else if ((ULib.StringLibrary.isStringMatch("-dictionary", argv[2])) && (argc >= 4))
			{
				setSearchProperties(SEARCH_DICTIONARY, 0, 0, 0, false, false, false, false, argv[3]);
			}
		}

		parseImageFile(argv[1]);

		char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(argv[1], true);
		ULib.ConsoleLibrary.displayConsoleMessage("Image File Name\t\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(fileNameStripped);
		ULib.HeapLibrary.freePrivateHeap(fileNameStripped);

		// check for encrypted password
		if ((passwordCRC[0] == 0x00) &&
			(passwordCRC[1] == 0x00) &&
			(passwordCRC[2] == 0x00) &&
			(passwordCRC[3] == 0x00))
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
		}

		// v2.xx - v3.xx
		if ((imageVersionFormat == VERSION_2) || (imageVersionFormat == VERSION_3))
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Partition Name\t\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(partitionName);
			ULib.ConsoleLibrary.displayConsoleMessage("File System Type\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(filesystemType);
			ULib.ConsoleLibrary.displayConsoleMessage("Partition Type\t\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(partitionType);
			ULib.ConsoleLibrary.displayConsoleMessage("");
			ULib.HeapLibrary.freePrivateHeap(partitionName);
			ULib.HeapLibrary.freePrivateHeap(filesystemType);
			ULib.HeapLibrary.freePrivateHeap(partitionType);

			ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password...", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 25, false);

			char* recoveredPassword = NULL;
			recoveredPassword = findPasswordv2(passwordCRC);
			ULib.HeapLibrary.freePrivateHeap(passwordCRC);
			if (recoveredPassword == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

			ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password\t: '", false);
			ULib.ConsoleLibrary.displayConsoleMessage(recoveredPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'");
			ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
			MessageBeep(MB_ICONEXCLAMATION);
		}
		// v4.xx - v7.xx
		else if ((imageVersionFormat == VERSION_4) || (imageVersionFormat == VERSION_5) || (imageVersionFormat == VERSION_6) || (imageVersionFormat == VERSION_7))
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");
			char* recoveredPassword = NULL;
			recoveredPassword = findPasswordv4(passwordCRC);
			ULib.HeapLibrary.freePrivateHeap(passwordCRC);
			if (recoveredPassword[0] == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

			ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password\t: '", false);
			ULib.ConsoleLibrary.displayConsoleMessage(recoveredPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'");
			ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
			MessageBeep(MB_ICONEXCLAMATION);
		}
	}
	else
	{
		displayOptions();
	}

	ULib.ProcessLibrary.exitProcessHandler();
}