////////////////////////////////////////////////
// PowerQuest PartitionMagic Password Cracker //
// (C)thewd, thewd@hotmail.com                //
////////////////////////////////////////////////
////////////////////////////////////////////////
// Recovers the password that restricts       //
// access to the PartitionMagic application   //
//                                            //
// Tested with...                             //
// - v6.00                                    //
// - v7.00, v7.01                             //
// - v8.00, v8.01, v8.02                      //
////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Doesn't appear to be a valid PartitionMagic password file",
	"Unable to find any encrypted password within the file",
	"Unable to recover a suitable password"
};

CUtilitiesLibrary ULib;

//////////////////////////////////////////////////////////
// decrypt the encrypted password and return the result //
//////////////////////////////////////////////////////////
char* recoverPassword(char* encryptedPassword)
{
	char* decryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 0; i < ULib.StringLibrary.getStringLength(encryptedPassword); i++) decryptedPassword[i] = (char)(encryptedPassword[i] ^ 0x0F);
	return decryptedPassword;
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("PowerQuest PartitionMagic Password Cracker", "1.00", 2000);

	if (argc > 1)
	{
		// get the full path to the fileName - argv[1]
		char* fullpathFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		GetFullPathName(argv[1], ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fullpathFilename, NULL);

		// check that the file is a PartitionMagic Password file
		char* pmHeader = (char*)ULib.HeapLibrary.allocPrivateHeap();
		GetPrivateProfileString("Password", "Value", "NotFound", pmHeader, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fullpathFilename);
		if ((pmHeader == NULL) || (ULib.StringLibrary.isStringMatch(pmHeader, "NotFound"))) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

		// check if the Value key contains a password value
		// if not, then the it is not password protected
		// if it is, then retrieve the encrypted value
		char* encryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
		GetPrivateProfileString("Password", "Value", "NotFound", encryptedPassword, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fullpathFilename);
		if ((ULib.StringLibrary.isStringMatch(encryptedPassword, "NotFound")) || (encryptedPassword[0] == NULL)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
		ULib.HeapLibrary.freePrivateHeap(fullpathFilename);
		ULib.HeapLibrary.freePrivateHeap(pmHeader);

		char* decryptedPassword = recoverPassword(encryptedPassword);

		if (decryptedPassword[0] != NULL)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Password Found\t: '", false);
			ULib.ConsoleLibrary.displayConsoleMessage(decryptedPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'");
			MessageBeep(MB_ICONEXCLAMATION);
		}
		else
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		}

		ULib.HeapLibrary.freePrivateHeap(encryptedPassword);
		ULib.HeapLibrary.freePrivateHeap(decryptedPassword);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tPQpm <configuration file>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}