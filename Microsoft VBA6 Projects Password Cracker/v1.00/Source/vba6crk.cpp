/////////////////////////////////////////////////////
// Microsoft VBA6 Projects Password Cracker        //
// (C)thewd, thewd@hotmail.com                     //
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Replaces a encrypted password for VBA6 Projects //
// with a new known password, i.e. 007             //
//                                                 //
// Tested with...                                  //
// - v6.0.8435, v6.0.8714                          //
/////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the Microsoft VBA Project",
	"Error reading data from the Microsoft VBA Project",
	"Doesn't appear to be a supported VBA6 Project",
	"Error occurred while creating the backup file",
	"Error occurred updating the original document"
};

// encrypted bytes for the password '007'
const unsigned char newPassword[74] =
{
	0x42, 0x41, 0x42, 0x38, 0x31, 0x36, 0x42, 0x39, 0x45, 0x45, 0x44,
	0x36, 0x45, 0x45, 0x44, 0x36, 0x31, 0x31, 0x32, 0x41, 0x45, 0x46,
	0x44, 0x36, 0x35, 0x46, 0x32, 0x37, 0x42, 0x42, 0x36, 0x36, 0x36,
	0x46, 0x31, 0x30, 0x33, 0x43, 0x46, 0x30, 0x39, 0x31, 0x30, 0x38,
	0x34, 0x45, 0x34, 0x32, 0x31, 0x30, 0x42, 0x34, 0x37, 0x37, 0x38,
	0x37, 0x42, 0x46, 0x34, 0x38, 0x39, 0x35, 0x42, 0x35, 0x32, 0x39,
	0x30, 0x32, 0x36, 0x37, 0x34, 0x30, 0x33, 0x38
};

/////////////////////////////////////////////////////////////////////
// find the location within the document of the encrypted password //
/////////////////////////////////////////////////////////////////////
unsigned long findPasswordOffset(unsigned char* buffer, unsigned long bufferLength)
{
	for (unsigned long count = 0; count < bufferLength; count++)
	{
		if ((buffer[count + 0] == 'D') &&
			(buffer[count + 1] == 'P') &&
			(buffer[count + 2] == 'B') &&
			(buffer[count + 3] == '=') &&
			(buffer[count + 4] == '"'))
		{
			return (count + 5);
		}
	}

	return 0;
}

///////////////////////////////////////////////////
// find out the length of the encrypted password //
///////////////////////////////////////////////////
int determinePasswordLength(unsigned char* buffer, unsigned long passwordOffset)
{
	for (unsigned long count = passwordOffset; count < (passwordOffset + 100); count++)
	{
		if ((buffer[count + 0] == '"') &&
			(buffer[count + 1] == 0x0D) &&
			(buffer[count + 2] == 0x0A))
		{
			return (count - passwordOffset);
		}
	}

	return 0;
}

///////////////////////////////////////////////////
// Load this file, determine the location of the //
// password and its length, and then change it   //
///////////////////////////////////////////////////
void parseFileAndChangePassword(char* fileName)
{
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize <= 0)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	unsigned long passwordOffset = findPasswordOffset(fileBuffer, fileSize);
	int passwordLength = determinePasswordLength(fileBuffer, passwordOffset);
	if ((passwordOffset == 0) || (passwordLength == 0)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	ULib.ConsoleLibrary.displayConsoleMessage("Changing Password to '007'");

	unsigned char* newPasswordBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(100);
	// change password from encrypted original to new encrypted password - 007
	for (int count = 0; count < passwordLength; count++)
	{
		if (count >= 74) newPasswordBuffer[count] = 0xFF;
		else newPasswordBuffer[count] = newPassword[count];
	}

	// create backup file
	char* backupFilename = ULib.FileLibrary.getBackupFilename(fileName);
	ULib.ConsoleLibrary.displayConsoleMessage("Creating Backup File...");
	if (CopyFile(fileName, backupFilename, TRUE) == 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	ULib.HeapLibrary.freePrivateHeap(backupFilename);

	ULib.ConsoleLibrary.displayConsoleMessage("Writing Changes to Original File...");

	// update document with the new password
	if (!ULib.FileLibrary.writeBufferToFile(fileName, fileBuffer, passwordOffset)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	if (!ULib.FileLibrary.appendBufferToFile(fileName, newPasswordBuffer, passwordLength)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	if (!ULib.FileLibrary.appendBufferToFile(fileName, (unsigned char*)&fileBuffer[passwordOffset + passwordLength], fileSize - passwordOffset + passwordLength)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);

	ULib.HeapLibrary.freePrivateHeap(newPasswordBuffer);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Microsoft VBA6 Projects Password Cracker", "1.00", 1999);

	// if arguments are passed
	if (argc > 1) parseFileAndChangePassword(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tvba6crk <vba6 project document>");

	ULib.ProcessLibrary.exitProcessHandler();
}