/////////////////////////////////////////////////////////
// InstallShield Professional Cabinet Password Cracker //
// (C)thewd, thewd@hotmail.com                         //
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// Recovers the main password for password-protected   //
// InstallShield Professional Cabinets (v5.50-v7.00)   //
//                                                     //
// Tested with...                                      //
// - v5.53                                             //
// - v6.00, v6.03, v6.10, v6.20, v6.22, v6.31          //
// - v7.00, v7.10 (password encrypted using MD5)       //
// - v17.00       (password encrypted using MD5)       //
/////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

typedef struct ISCabHeader
{
	DWORD ISCabSignature;
	BYTE ISCabFormatMinorVersion;
	BYTE ISCabFormatMajorVersion;
	WORD ISCabHeaderUnknown;
	DWORD ISCabIsPasswordProtected;
} ISCABHEADER;

ISCABHEADER cabHeader;
CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the specified file header",
	"Doesn't appear to be a supported InstallShield Professional Cabinet",
	"Unable to recognise the InstallShield Professional Cabinet format",
	"This InstallShield Professional Cabinet is not password protected",
	"Unable to recover the password. v7.10+ cabinets use MD5 encryption"
};

/////////////////////////////////////////////////////
// retrieve header information from the IS Cabinet //
/////////////////////////////////////////////////////
unsigned char* determineCabinetFormat(char* fileName)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, 400);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	cabHeader.ISCabSignature = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer);
	cabHeader.ISCabFormatMinorVersion = fileBuffer[0x04];
	cabHeader.ISCabFormatMajorVersion = fileBuffer[0x05];
	cabHeader.ISCabHeaderUnknown = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, 0x06);
	cabHeader.ISCabIsPasswordProtected = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x08);
	return fileBuffer;
}

///////////////////////////////////////////////////////////
// check for a valid Cabinet, and return password offset //
///////////////////////////////////////////////////////////
int checkCabinetFormat(void)
{
	// ISc(
	if (cabHeader.ISCabSignature != 0x28635349) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	// Not Password Protected
	if (cabHeader.ISCabIsPasswordProtected != 0x00000100) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	unsigned long verISCab = (cabHeader.ISCabFormatMajorVersion << 8) + cabHeader.ISCabFormatMinorVersion;

	// v5.5x
	if ((cabHeader.ISCabHeaderUnknown == 0x0100) && (cabHeader.ISCabFormatMajorVersion == 0x52)) return 0x3C;
	// v6.xx & v7.xx+
	else if ((cabHeader.ISCabHeaderUnknown == 0x0100) && (cabHeader.ISCabFormatMajorVersion >= 0x60)) return 0x54;
	// v10.xx+
	else if ((cabHeader.ISCabHeaderUnknown == 0x0400) && (verISCab >= 1000)) return 0x54;
	// v10.xx+
	else if ((cabHeader.ISCabHeaderUnknown == 0x0200) && (verISCab >= 1000)) return 0x54;
	// v10.xx+
	else if ((cabHeader.ISCabHeaderUnknown == 0x0100) && (verISCab >= 1000)) return 0x54;
	// unknown version
	else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	return 0;
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("InstallShield Professional Cabinet Password Cracker", "1.00", 1999);

	if (argc > 1)
	{
		unsigned char* fileBuffer = determineCabinetFormat(argv[1]);
		int passwordOffset = checkCabinetFormat();
		unsigned char* passwordOffsetBuffer = (unsigned char*)&fileBuffer[passwordOffset];
		bool md5Encrypted = false;

		// check whether the password has been encrypted
		if ((ULib.StringLibrary.getWORDFromBuffer(passwordOffsetBuffer, 0x0E) >= 0x0100) && (ULib.StringLibrary.getDWORDFromBuffer(passwordOffsetBuffer + 0x10) == 0))
		{
			for (int i = 0; i < 16; i++)
			{
				if ((passwordOffsetBuffer[i] < 0x20) || (passwordOffsetBuffer[i] >= 0x80))
				{
					md5Encrypted = true;
					break;
				}
			}
		}

		// check for recognised passwords
		if (md5Encrypted)
		{
			// password - blank
			if (memcmp(passwordOffsetBuffer, "\xD4\x1D\x8C\xD9\x8F\x00\xB2\x04\xE9\x80\x09\x98\xEC\xF8\x42\x7E", 16) == 0)
			{
				passwordOffsetBuffer[0] = NULL;
				md5Encrypted = false;
			}
			// password - *******
			else if (memcmp(passwordOffsetBuffer, "\x2E\x77\x1F\xE4\xF4\x35\x45\x32\xDB\xC4\x9C\x9C\x9A\x45\xE8\x1F", 16) == 0)
			{
				ULib.StringLibrary.copyString((char*)passwordOffsetBuffer, "*******\0");
				md5Encrypted = false;
			}
			// password - helloworld
			else if (memcmp(passwordOffsetBuffer, "\x26\x61\xD1\x27\xB1\x21\xC8\xFB\xB4\x33\x78\x28\x29\xF8\x13\x93", 16) == 0)
			{
				ULib.StringLibrary.copyString((char*)passwordOffsetBuffer, "helloworld\0");
				md5Encrypted = false;
			}
		}

		if (md5Encrypted)
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4, false);
			ULib.ConsoleLibrary.displayConsoleMessage("           (MD5 Hash - ", false);
			ULib.ConsoleLibrary.displayHexadecimalString(passwordOffsetBuffer, 16, 0, false);
			ULib.ConsoleLibrary.displayConsoleMessage(")");
		}
		else
		{
			char* passwordResult = (char*)ULib.HeapLibrary.allocPrivateHeap();
			wsprintf(passwordResult, "Media Password : '%s'", (char*)passwordOffsetBuffer);
			ULib.ConsoleLibrary.displayConsoleMessage(passwordResult);
			ULib.HeapLibrary.freePrivateHeap(passwordResult);
			MessageBeep(MB_ICONEXCLAMATION);
		}

		ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tIScab <installshield cabinet header>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}