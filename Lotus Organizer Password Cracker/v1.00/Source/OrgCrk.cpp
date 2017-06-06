///////////////////////////////////////////////////////
// Lotus Organizer Password Cracker                  //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// Decrypt passwords for Lotus Organizer files       //
// Can recover Owner, Assistant and Reader passwords //
//                                                   //
// Tested with...                                    //
// - v2.10                                           //
// - v4.10                                           //
// - v5.00, v5.01                                    //
// - v6.00                                           //
///////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Unable to open or read the Lotus Organizer file",
	"Doesn't appear to be a supported Lotus Organizer file",
	"Unable to recognise the format of this Organizer file"
};

CUtilitiesLibrary ULib;

char* ownerPassword;
char* assistantPassword;
char* readerPassword;

///////////////////////////////////////////////////
// Decrypt the passwords stored in the organizer //
// file. Owner, Assistant and Reader passwords   //
///////////////////////////////////////////////////
char* recoverPassword(unsigned char* buffer, int startingPosition)
{
	char* passwordBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int tempPassword;
	int index = 0;

	if ((buffer[startingPosition] == 0x00) || (buffer[startingPosition] == 0x80))
	{
		ULib.StringLibrary.copyString(passwordBuffer, "<password blank>");
		return passwordBuffer;
	}

	while (index < 40)
	{
		tempPassword = ((buffer[startingPosition + index + 1] * 0x100) + buffer[startingPosition + index]);
		tempPassword = ((tempPassword & 0x8000) >> 0x0F) | (tempPassword * 2);
		passwordBuffer[index] = (char)(tempPassword & 0xFF);
		passwordBuffer[index + 1] = (char)((tempPassword & 0xFF00) >> 8);
		index += 2;
	}

	return passwordBuffer;
}

///////////////////////////////////////////////////////////
// read the organizer file. Check the file header for    //
// a recognised format and recover the passwords stored, //
// if available. Return the Organizer file format        //
///////////////////////////////////////////////////////////
char* parseOrganizerFile(char* fileName)
{
	unsigned char* fileBuffer= ULib.FileLibrary.readBufferFromFile(fileName, 0x200);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// check image CRC
	if ((fileBuffer[1] == 0x4B) &&
		(fileBuffer[2] == 0x04) &&
		(fileBuffer[3] == 0xB3))
	{
		if ((fileBuffer[0] != 0x10) &&
			(fileBuffer[0] != 0x18) &&
			(fileBuffer[0] != 0x20) &&
			(fileBuffer[0] != 0x28) &&
			(fileBuffer[0] != 0x30))
		{
			// unrecognized format
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		}
	}
	else
	{
		// invalid image
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	ownerPassword = recoverPassword(fileBuffer, 0x6C);
	assistantPassword = recoverPassword(fileBuffer, 0x190);
	readerPassword = recoverPassword(fileBuffer, 0x1B0);

	unsigned long formatVersion = fileBuffer[0];
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	if (formatVersion == 0x10) return "v2.xx";
	if (formatVersion == 0x18) return "v3.xx";
	if (formatVersion == 0x20) return "v4.xx";
	if (formatVersion == 0x28) return "v5.xx";
	if ((formatVersion >= 0x30) && (formatVersion <= 0x40)) return "v6.xx";
	return "Unknown";
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Lotus Organizer Password Cracker", "1.00", 1999);

	if (argc > 1)
	{
		char* organizerFormat = parseOrganizerFile(argv[1]);

		ULib.ConsoleLibrary.displayConsoleMessage("Organizer Format   : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(organizerFormat);
		ULib.ConsoleLibrary.displayConsoleMessage("Owner Password     : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(ownerPassword);
		ULib.ConsoleLibrary.displayConsoleMessage("Assistant Password : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(assistantPassword);
		ULib.ConsoleLibrary.displayConsoleMessage("Reader Password    : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(readerPassword);
		MessageBeep(MB_ICONEXCLAMATION);
		ULib.HeapLibrary.freePrivateHeap(ownerPassword);
		ULib.HeapLibrary.freePrivateHeap(assistantPassword);
		ULib.HeapLibrary.freePrivateHeap(readerPassword);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tOrgCrk <lotus organizer file>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}