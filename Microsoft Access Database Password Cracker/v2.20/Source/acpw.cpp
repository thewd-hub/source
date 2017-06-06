////////////////////////////////////////////////////////////
// Microsoft Access Database Password Cracker             //
// (C)thewd, thewd@hotmail.com                            //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Recovers passwords up to a length of 20 characters for //
// Microsoft Access databases (95, 97, 2000, 2002 & 2003) //
////////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Unable to open or read the Microsoft Access database",
	"Error reading data from the Microsoft Access database",
	"Unable to find any encrypted password"
};

bool DEBUG_MODE = false;

CUtilitiesLibrary ULib;
bool isAccess200XFormat = false;
unsigned char* recoveredPassword;

/////////////////////////////////////////
// is the database format Access 2000+ //
/////////////////////////////////////////
bool isAccess200X(unsigned char* fileBuffer)
{
	return (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x14) == 1);
}

/////////////////////////////////////////////////////////////////////
// decrypts the password from the database using the RC4 algorithm //
/////////////////////////////////////////////////////////////////////
void decryptAccessPassword(unsigned char* fileBuffer)
{
	unsigned char keyData[4] = {0xC7, 0xDA, 0x39, 0x6B};
	ULib.EncryptionLibrary.RC4Library.encryptBuffer(keyData, 4, fileBuffer, 0x18, 0x80);

	int fileOffset = 0;
	int index = 0;

	recoveredPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();

	if (isAccess200XFormat)
	{
		// retrieve XOR value
		fileOffset = 0x72;
		_int64 getXOR = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset + 4);
		getXOR = getXOR << 32;

		double tempXOR;
		__asm
		{
				fld getXOR
				fstp tempXOR
		}

		unsigned long xorValue = (unsigned long)tempXOR;
		if (DEBUG_MODE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Password XOR Value : 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(xorValue, true, true, ULib.ConsoleLibrary.DISPLAY_AS_WORD);
		}

		fileOffset = 0x42;
		index = 0;

		// is the database password-protected?
		unsigned long currentValue = (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset) ^ xorValue);

		if (currentValue != 0)
		{
			// decrypt the rest of the password
			int currentPasswordLength = 0;
			while (index < 40)
			{
				currentValue = (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset + index) ^ xorValue);
				ULib.StringLibrary.addDWORDToBuffer(fileBuffer, currentValue, fileOffset + index);
				recoveredPassword[currentPasswordLength++] = fileBuffer[fileOffset + index + 0];
				recoveredPassword[currentPasswordLength++] = fileBuffer[fileOffset + index + 2];
				index += 4;
			}
		}
	}
	else
	{
		fileOffset = 0x42;
		index = 0;
		int currentPasswordLength = 0;

		// decrypt password
		do
		{
			recoveredPassword[currentPasswordLength] = fileBuffer[fileOffset + index];
			currentPasswordLength++;
			index++;

		} while ((index < 20) && (recoveredPassword[currentPasswordLength - 1] != 0x00));
	}
}

///////////////////////////////////////////////////////
// Load this file, determine the format, and decrypt //
// the password at the specified file offset         //
///////////////////////////////////////////////////////
void parseFileAndDecrypt(char* fileName)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, 0x500);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	isAccess200XFormat = isAccess200X(fileBuffer);
	decryptAccessPassword(fileBuffer);

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Microsoft Access Database Password Cracker", "2.20", 1999);

	// if arguments are passed
	if (argc > 1)
	{
		DEBUG_MODE = ULib.ProcessLibrary.isDebugModeEnabled();

		parseFileAndDecrypt(argv[1]);

		if ((recoveredPassword != NULL) && (recoveredPassword[0] != NULL))
		{
			if (isAccess200XFormat) ULib.ConsoleLibrary.displayConsoleMessage("Database Format    : Access 2000/2002/2003");
			else ULib.ConsoleLibrary.displayConsoleMessage("Database Format    : Access 95/97");
			ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password : '", false);
			ULib.ConsoleLibrary.displayConsoleMessage((char*)recoveredPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'");
			MessageBeep(MB_ICONEXCLAMATION);
			ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
		}
		else
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2, false);
		}
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tacpw <access database>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}