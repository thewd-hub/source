//////////////////////////////////////////////////////////
// PowerQuest BootMagic Password Cracker                //
// (C)thewd, thewd@hotmail.com                          //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Recovers Configuration, Boot and Partition passwords //
// created by BootMagic v2.xx & v6.xx & v7.xx & v8.xx   //
//                                                      //
// Tested with...                                       //
// - v2.00                                              //
// - v6.00                                              //
// - v7.00, v7.01                                       //
// - v8.00                                              //
//////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;
bool displayFirstItem = false;

char* errorMessages[] =
{
	"Unable to open or read the BootMagic Configuration File",
	"Doesn't appear to be a valid BootMagic Configuration File"
};

const int NoOfCycles[16] =
{
	3, 5, 2, 7, 1, 6, 4, 5,
	2, 7, 4, 3, 7, 1, 6, 4
};
const unsigned char XORvalues[16] =
{
	0x23, 0x93, 0xF3, 0x78, 0xB5, 0x51, 0xA4, 0xB2,
	0x84, 0x72, 0xDA, 0x2B, 0x8C, 0x92, 0xC9, 0x25
};
const unsigned char NoPasswordBuffer[17] =
{
	0x64, 0x9C, 0xFC, 0xF0, 0xDA, 0x45, 0x4A, 0x95,
	0x21, 0xE4, 0xAD, 0x65, 0x19, 0x49, 0x27, 0x52,
	0x00,
};

////////////////////////////////////////////////
// recovers password from the encrypted bytes //
////////////////////////////////////////////////
void recoverPassword(unsigned char* passwordBuffer, char* passwordString)
{
	if (displayFirstItem) ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("Section  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(passwordString);
	ULib.ConsoleLibrary.displayConsoleMessage("Password : ", false);
	displayFirstItem = true;

	if (ULib.StringLibrary.isStringMatch((char*)passwordBuffer, (char*)NoPasswordBuffer))
	{
		ULib.ConsoleLibrary.displayConsoleMessage("No Password");
	}
	else
	{
		// recover password
		unsigned char currentValue;

		for (int i = 0; i < 16; i++)
		{
			currentValue = passwordBuffer[i];

			for (int j = 0; j < NoOfCycles[i]; j++)
			{
				if (currentValue & 0x80) currentValue = (unsigned char)((currentValue * 2) + 1);
				else currentValue = (unsigned char)(currentValue * 2);
			}

			passwordBuffer[i] = (unsigned char)(currentValue ^ XORvalues[i]);
		}

		passwordBuffer[16] = NULL;
		ULib.ConsoleLibrary.displayConsoleMessage((char*)passwordBuffer);
	}
}

/////////////////////////////////////////////////////////
// Opens the configuration file and parses the entries //
/////////////////////////////////////////////////////////
void parseConfigurationFile(char* fileName)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// check for BootMagic Configuration File
	char lineSeparator[] = "\r\n";
	char* buffer = strtok((char*)fileBuffer, lineSeparator);
	if (buffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	if (!ULib.StringLibrary.isStringMatch(buffer, "VERSION 2")) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	while (buffer != NULL)
	{
		buffer = strtok(NULL, lineSeparator);
		if (buffer != NULL)
		{
			// BootMagic Configuration Section
			if (ULib.StringLibrary.isStringMatch(buffer, "--Config Item--"))
			{
				unsigned char* configurationPasswordBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
				unsigned char* startupbootPasswordBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();

				// padding
				buffer = strtok(NULL, lineSeparator);
				// BootMagic Configuration Password
				ULib.StringLibrary.addDWORDToBuffer(configurationPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x00);
				ULib.StringLibrary.addDWORDToBuffer(configurationPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x04);
				ULib.StringLibrary.addDWORDToBuffer(configurationPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x08);
				ULib.StringLibrary.addDWORDToBuffer(configurationPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x0C);
				// BootMagic Startup Boot Password
				ULib.StringLibrary.addDWORDToBuffer(startupbootPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x00);
				ULib.StringLibrary.addDWORDToBuffer(startupbootPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x04);
				ULib.StringLibrary.addDWORDToBuffer(startupbootPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x08);
				ULib.StringLibrary.addDWORDToBuffer(startupbootPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x0C);

				recoverPassword(configurationPasswordBuffer, "Configuration");
				recoverPassword(startupbootPasswordBuffer, "Startup Boot");

				ULib.HeapLibrary.freePrivateHeap(configurationPasswordBuffer);
				ULib.HeapLibrary.freePrivateHeap(startupbootPasswordBuffer);
			}
			// Menu Item (Partition)
			if (ULib.StringLibrary.isStringMatch(buffer, "--Menu Item--"))
			{
				char* partitionName = strtok(NULL, lineSeparator);
				unsigned char* partitionPasswordBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();

				// padding
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				buffer = strtok(NULL, lineSeparator);
				// Partition Password
				ULib.StringLibrary.addDWORDToBuffer(partitionPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x00);
				ULib.StringLibrary.addDWORDToBuffer(partitionPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x04);
				ULib.StringLibrary.addDWORDToBuffer(partitionPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x08);
				ULib.StringLibrary.addDWORDToBuffer(partitionPasswordBuffer, ULib.StringLibrary.getDWORDFromString(strtok(NULL, lineSeparator), true), 0x0C);

				buffer = strtok(NULL, lineSeparator);
				char* displayString = (char*)ULib.HeapLibrary.allocPrivateHeap();
				wsprintf(displayString, "Partition (%s - %s)", partitionName, strtok(NULL, lineSeparator));
				recoverPassword(partitionPasswordBuffer, displayString);

				ULib.HeapLibrary.freePrivateHeap(displayString);
				ULib.HeapLibrary.freePrivateHeap(partitionPasswordBuffer);
			}
		}
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("PowerQuest BootMagic Password Cracker", "1.00", 1999);

	if (argc > 1) parseConfigurationFile(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tPQbm <configuration file>");

	ULib.ProcessLibrary.exitProcessHandler();
}