///////////////////////////////////////////////
// RTPatch Password Cracker                  //
// (C)thewd, thewd@hotmail.com               //
///////////////////////////////////////////////
///////////////////////////////////////////////
// Recovers passwords up to a maximum length //
// of 255 characters for RTPatch patch files //
//                                           //
// Username: 090700EVAL / Password: RU328629 //
// http://www.pocketsoft.com/eval/rtpwin.exe //
// ftp://ftp.pocketsoft.com/rtpwin.exe       //
// ftp://ftp.pocketsoft.com/rtpunix.exe      //
///////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the RTPatch patch file",
	"Error reading data from the RTPatch patch file",
	"Doesn't appear to be a supported RTPatch patch file",
	"Unable to find an encrypted password in the RTPatch patch file"
};

////////////////////////////////////////
// is the patch file format supported //
////////////////////////////////////////
void determineRTPatchVersion(unsigned char* fileBuffer, unsigned long* passwordOffset)
{
	if (passwordOffset == NULL) return;
	// v3.00 - v6.00
	*passwordOffset = 0x22;
	// v6.50+
	if (fileBuffer[*passwordOffset] == 0x00) *passwordOffset = 0x26;
}

////////////////////////////////////////////////////
// decrypts the password for a RTPatch patch file //
////////////////////////////////////////////////////
char* decryptRTPatchPassword(unsigned char* fileBuffer)
{
	unsigned char baseValue = 0x31;
	unsigned long encryptedPasswordOffset = 0;
	determineRTPatchVersion(fileBuffer, &encryptedPasswordOffset);
	char* recoveredPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int recoveredPasswordLength = fileBuffer[encryptedPasswordOffset++];
	int maximumPasswordLength = ((recoveredPasswordLength > 255) ? 255 : recoveredPasswordLength);

	for (int i = 0; i < maximumPasswordLength; i++)
	{
		recoveredPassword[i] = (char)(fileBuffer[encryptedPasswordOffset + i] - (baseValue++));
		if (recoveredPassword[i] == 0x00) break;
	}

	return recoveredPassword;
}

/////////////////////////////////////////////////////
// load the patch file, determine if it's password //
// protected and call the decryption routine       //
/////////////////////////////////////////////////////
char* parseFileAndDecrypt(char* fileName)
{
	unsigned long fileSize = ULib.FileLibrary.getFileSize(fileName);
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, ((fileSize > 500) ? 500 : fileSize));
	if ((fileBuffer == NULL) || (fileSize < 4)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// the patch file isn't encrypted
	if ((fileBuffer[0x04] & 0x08) == 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	char* recoveredPassword = decryptRTPatchPassword(fileBuffer);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	return recoveredPassword;
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("RTPatch Password Cracker", "1.00", 2000);

	// if arguments are passed
	if (argc > 1)
	{
		char* recoveredPassword = parseFileAndDecrypt(argv[1]);

		if (recoveredPassword[0] != NULL)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password : '", false);
			ULib.ConsoleLibrary.displayConsoleMessage(recoveredPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'");
			MessageBeep(MB_ICONEXCLAMATION);
		}
		else
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3, false);
		}

		ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tRTpatch <protected patch file>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}