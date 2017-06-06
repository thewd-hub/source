/////////////////////////////////////////////////////////
// Intuit QuickBooks Password Cracker                  //
// (C)thewd, thewd@hotmail.com                         //
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// Recovers a password for Intuit QuickBooks databases //
//                                                     //
// Tested with...                                      //
// - v2.00                                             //
// - v3.00                                             //
// - v4.00                                             //
// - v5.00                                             //
// - v6.00                                             //
// - v8.00 (2000)                                      //
// - v10.00 (2002)                                     //
/////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\OpenSSL_DESLibrary.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Unable to open or read the Intuit QuickBooks Project",
	"Doesn't appear to be a supported Intuit QuickBooks Project",
	"Unable to find any encrypted password",
	"Unable to determine the QuickBooks Administrator Name"
};

#define VERSION_UNKNOWN		0
#define VERSION_2			2
#define VERSION_3			3
#define VERSION_4			4
#define VERSION_5			5
#define VERSION_6			6
#define VERSION_7			7
#define VERSION_8			8
#define VERSION_9			9
#define VERSION_10			10

CUtilitiesLibrary ULib;

/////////////////////////////////////////////
// decrypts a QuickBooks password (v4.xx+) //
/////////////////////////////////////////////
char* decryptPassword(unsigned char* encryptedPassword, int encryptedPasswordLength)
{
	char* decryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// check that we have an encrypted password
	if (encryptedPasswordLength <= 0) return NULL;

	for (int i = (encryptedPasswordLength - 1); i >= 0; i--)
	{
		if (encryptedPassword[i] == 0) continue;
		encryptedPassword[i] = (unsigned char)(((encryptedPassword[i] & 0xFF) << 4) | ((encryptedPassword[i] >> 4) & 0xFF));
		decryptedPassword[encryptedPasswordLength - 1 - i] = (unsigned char)(0xFF - encryptedPassword[i]);
	}

	return decryptedPassword;
}

////////////////////////////////////////////////////////////
// recovers a password for QuickBooks v2.xx & v3.xx files //
////////////////////////////////////////////////////////////
char* recoverQuickBooksPassword_v2andv3(unsigned char* fileBuffer, unsigned long fileBufferLength)
{
	char* decryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int baseOffset = 0x1DD3;

	// check that the file is long enough
	if ((unsigned long)(baseOffset + 50) >= fileBufferLength) return NULL;

	// recover password
	for (int i = 0; i < 11; i++) decryptedPassword[i] = fileBuffer[baseOffset + i];
	return decryptedPassword;
}

//////////////////////////////////////////////////////////////
// recovers a password for QuickBooks v4.xx and v5.xx files //
//////////////////////////////////////////////////////////////
char* recoverQuickBooksPassword_v4andv5(unsigned char* fileBuffer, unsigned long fileBufferLength)
{
	unsigned char* encryptedPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	char* decryptedPassword;
	int baseOffset = 0x1DD3;
	int passwordLength = 0;

	// check that the file is long enough
	if ((unsigned long)(baseOffset + 50) >= fileBufferLength) return NULL;

	// recover password
	for (int i = 0; i < 11; i++)
	{
		encryptedPassword[i] = fileBuffer[baseOffset + i];
		if (fileBuffer[baseOffset + i] == 0x00) break;
		passwordLength++;
	}

	decryptedPassword = decryptPassword(encryptedPassword, passwordLength);
	ULib.HeapLibrary.freePrivateHeap(encryptedPassword);
	return decryptedPassword;
}

/////////////////////////////////////////////////////
// recovers a password for QuickBooks v6.xx+ files //
/////////////////////////////////////////////////////
char* recoverQuickBooksPassword_v6(unsigned char* fileBuffer, unsigned long fileBufferLength)
{
	unsigned char* encryptedPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	char* decryptedPassword;
	char* administratorName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned long administratorSectionOffset = 0;
	bool isDESEncrypted = false;

	// find administration section
	for (unsigned long i = 0; i < (fileBufferLength - 35); i++)
	{
		if ((fileBuffer[i + 1] == 0x01) &&
			(fileBuffer[i + 2] == 0x80) &&
			(fileBuffer[i + 3] == 0x01) &&
			(fileBuffer[i + 7] == 0x1E))
		{
			// v6.xx+
			if ((fileBuffer[i + 0] >= 0x65) && (fileBuffer[i + 0] <= 0x69))
			{
				administratorSectionOffset = i;
				break;
			}
		}
	}

	// unable to find any administration section
	if (administratorSectionOffset == 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	// administration section is encrypted with DES
	if (fileBuffer[administratorSectionOffset + 0x09] > 0x02)
	{
		isDESEncrypted = true;

		__try
		{
			DES_key_schedule desSchedule;
			DES_cblock desKey;
			memcpy((void*)&desKey[0], (void*)&fileBuffer[0x1C8D], 4);
			memcpy((void*)&desKey[4], (void*)&fileBuffer[0x1C8D], 4);
			DES_set_key_unchecked(&desKey, &desSchedule);
			for (int i = 0; i < 8; i++) DES_encrypt1((DES_LONG*)&fileBuffer[administratorSectionOffset + 0x0A + (i * 8)], &desSchedule, DES_DECRYPT);
			for (int i = 0; i < 0x40; i++) fileBuffer[administratorSectionOffset + 0x0A + i] ^= i;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	// adjust baseOffset to Administrator Name
	int baseOffset = administratorSectionOffset + 0x0B;
	int passwordLength = 0;

	// check that the file is long enough
	if ((unsigned long)(baseOffset + 50) >= fileBufferLength) return NULL;

	// display the Administrator Name
	ULib.StringLibrary.copyString(administratorName, (char*)&fileBuffer[baseOffset], 50);
	ULib.ConsoleLibrary.displayConsoleMessage("Administrator Name : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(administratorName);

	// update the baseOffset to the password
	baseOffset += 0x1E;

	// recover password
	for (int i = 0; i < 11; i++)
	{
		encryptedPassword[i] = fileBuffer[baseOffset + i];
		if (fileBuffer[baseOffset + i] == 0x00) break;
		passwordLength++;
	}

	if (isDESEncrypted)
	{
		decryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap(passwordLength + 1);
		ULib.StringLibrary.copyString(decryptedPassword, (char*)encryptedPassword);
	}
	else
	{
		decryptedPassword = decryptedPassword = decryptPassword(encryptedPassword, passwordLength);
	}

	ULib.HeapLibrary.freePrivateHeap(administratorName);
	ULib.HeapLibrary.freePrivateHeap(encryptedPassword);
	return decryptedPassword;
}

////////////////////////////////////////////////////////////////////////
// determines the version and format for a particular QuickBooks file //
////////////////////////////////////////////////////////////////////////
int determineQuickBooksVersion(unsigned char* fileBuffer, unsigned long fileBufferLength)
{
	for (int i = (fileBufferLength - 4); i >= 0; i--)
	{
		if ((fileBuffer[i + 0] == 'V') && ((fileBuffer[i + 2] == '.') || (fileBuffer[i + 3] == '.')))
		{
			// v2.xx
			if (fileBuffer[i + 1] == '2') return VERSION_2;
			// v3.xx
			else if (fileBuffer[i + 1] == '3') return VERSION_3;
			// v4.xx
			else if (fileBuffer[i + 1] == '4') return VERSION_4;
			// v5.xx
			else if (fileBuffer[i + 1] == '5') return VERSION_5;
			// v6.xx
			else if (fileBuffer[i + 1] == '6') return VERSION_6;
			// v7.xx
			else if (fileBuffer[i + 1] == '7') return VERSION_7;
			// v8.xx (2000)
			else if (fileBuffer[i + 1] == '8') return VERSION_8;
			// v9.xx (2001)
			else if (fileBuffer[i + 1] == '9') return VERSION_9;
			// v10.xx (2002)
			else if ((fileBuffer[i + 1] == '1') && (fileBuffer[i + 2] == '0')) return VERSION_10;
		}
	}

	return 0;
}

//////////////////////////////////////////////
// load the file, determine the location of //
// the password and attempt to recover it   //
//////////////////////////////////////////////
void parseFileAndFindPassword(char* fileName)
{
	// determine the file size
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize <= 0)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// determine the QuickBooks file version
	int fileVersion = determineQuickBooksVersion(fileBuffer, fileSize);
	if (fileVersion == VERSION_UNKNOWN) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// display version information
	char* qbFormat = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(qbFormat, "Detected Format    : v%d.00", fileVersion);
	ULib.ConsoleLibrary.displayConsoleMessage(qbFormat);
	ULib.HeapLibrary.freePrivateHeap(qbFormat);

	// decrypted password
	char* decryptedPassword = NULL;

	// v2.xx & v3.xx
	if ((fileVersion == VERSION_2) || (fileVersion == VERSION_3)) decryptedPassword = recoverQuickBooksPassword_v2andv3(fileBuffer, fileSize);
	// v4.xx & v5.xx
	if ((fileVersion == VERSION_4) || (fileVersion == VERSION_5)) decryptedPassword = recoverQuickBooksPassword_v4andv5(fileBuffer, fileSize);
	// v6.xx+
	if (fileVersion >= VERSION_6) decryptedPassword = recoverQuickBooksPassword_v6(fileBuffer, fileSize);

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);

	// display password information
	if ((decryptedPassword == NULL) || (decryptedPassword[0] == NULL))
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password : '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(decryptedPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'");
		ULib.HeapLibrary.freePrivateHeap(decryptedPassword);
		MessageBeep(MB_ICONEXCLAMATION);
	}
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Intuit QuickBooks Password Cracker", "1.01", 2000);

	// if arguments are passed
	if (argc > 1) parseFileAndFindPassword(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tqbCrk <quickbooks project>");

	ULib.ProcessLibrary.exitProcessHandler();
}