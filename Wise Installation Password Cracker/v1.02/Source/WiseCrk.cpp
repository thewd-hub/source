//////////////////////////////////////////////////
// WiseCRK                                      //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Uncompresses the Wise Installation Script    //
// and searches for an encrypted password       //
// Can extract this script to disk if necessary //
//////////////////////////////////////////////////
//////////////////////////////////////////////////
// based on exwise v0.5: (c) Andrew de Quincey  //
//                                              //
// portions based on funzip.c by M. Adler et al //
// portions based on Info-ZIP v5.31             //
//////////////////////////////////////////////////
#include <windows.h>
#include "WiseInflate.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

#define GetFilePointer(fileHandle) SetFilePointer(fileHandle, 0, NULL, FILE_CURRENT)

char* errorMessages[] =
{
	"Unable to open or read the Wise Installer executable",
	"Doesn't appear to be a supported Wise Installer package",
	"Unable to recognise the Wise Installer format",
	"This installation doesn't appear to be password protected",
	"Unable to decrypt the Wise installation script",
	"Installation script appears to be corrupt",
	"Unable to extract the installation script to the specified file"
};

// for NE format files only
int offsetTableNE[][2] =
{
	// {file offset, script offset}
	{0x2988, 0x3000},
	{0x2D88, 0x3800},
	{0x2F78, 0x3A00},
	{0x2FE0, 0x3660},
	{0x3070, 0x36F0},
	{0x30F0, 0x3770},
	{0x3114, 0x3780},
	{0x3130, 0x37B0},
	{0x3150, 0x37D0},
	{0x3454, 0x3C10},
	{0x34C4, 0x3C80},
	{0x47B0, 0x6E00},
	{-1, -1}
};

bool DEBUG_MODE = false;

/////////////////////////////////////////////////////////
// the main program execution. Opens the file and      //
// determines the file format. Extracts the compressed //
// installation script and searches for an encrypted   //
// password. Able to extract this script file to disk  //
/////////////////////////////////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Wise Installer Password Cracker", "1.02", 1999);

	if (argc <= 1)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tWiseCRK <setup filename> [/extract <output filename>]");
		ULib.ProcessLibrary.exitProcessHandler();
	}

	char* scriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned long numberOfFileSections = 0;
	unsigned long bytesRead;
	int i = 0;
	unsigned char* tempBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(1000000);
	unsigned char* scriptBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(1000000);
	unsigned char* scriptFlags = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(10);
	char* decryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();
	bool embeddedSetup = false;
	bool extractScript = false;
	unsigned long scriptOffset = 0;
	bool isPKZipMode = false;

	for (i = 0; i < argc; i++)
	{
		// extract installation script
		if ((ULib.StringLibrary.isStringMatch("/extract", argv[i])) && (i < (argc - 1)))
		{
			ULib.StringLibrary.copyString((char*)scriptFilename, argv[i + 1]);
			extractScript = true;
		}
	}

	// check whether the shift key is depressed (enable debug mode)
	DEBUG_MODE = ULib.ProcessLibrary.isDebugModeEnabled();

	// open file handle
	HANDLE fileHandle = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// determine file format
	int fileFormat = ULib.FileLibrary.determineFileFormat(argv[1], &numberOfFileSections);
	if (fileFormat == ULib.FileLibrary.FILEFORMAT_UNKNOWN) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// NE File Format
	if (fileFormat == ULib.FileLibrary.FILEFORMAT_NE)
	{
		i = 0;

		while (offsetTableNE[i][0] != -1)
		{
			SetFilePointer(fileHandle, offsetTableNE[i][0], NULL, FILE_BEGIN);
			ReadFile(fileHandle, tempBuffer, 0x04, &bytesRead, NULL);
			if (ULib.StringLibrary.getWORDFromBuffer(tempBuffer) == offsetTableNE[i][1])
			{
				// seek to the start of the data then
				SetFilePointer(fileHandle, offsetTableNE[i][1], NULL, FILE_BEGIN);
				break;
			}

			i++;
		}

		if (offsetTableNE[i][0] == -1) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

		if (DEBUG_MODE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("File Format\t: NE");
			ULib.ConsoleLibrary.displayConsoleMessage("File Offset\t: 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(offsetTableNE[i][0], true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
			ULib.ConsoleLibrary.displayConsoleMessage("Script Offset\t: 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(offsetTableNE[i][1], true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		}
	}
	// PE File Format
	else if (fileFormat == ULib.FileLibrary.FILEFORMAT_PE)
	{
		int rawdataOffset = 0;
		int rawdataSize = 0;
		int embeddedFileOffset = 0;
		scriptOffset = 0;

		// check for embedded setup
		unsigned char* embeddedSetupBuffer = ULib.FileLibrary.getResourceFromFile(argv[1], (LPSTR)101, RT_RCDATA);
		if (embeddedSetupBuffer != NULL) embeddedSetup = true;

		// setup is embedded, find offset
		if (embeddedSetup)
		{
			SetFilePointer(fileHandle, 0x02, NULL, FILE_BEGIN);
			// find the embedded file (PE)
			for (i = 0; i < 0x5000; i++)
			{
				ReadFile(fileHandle, tempBuffer, 0x60, &bytesRead, NULL);
				if ((tempBuffer[0] == 0x4D) &&
					(tempBuffer[1] == 0x5A) &&
					(tempBuffer[0x50] == 'i'))
				{
					embeddedFileOffset = i + 2;
					break;
				}

				SetFilePointer(fileHandle, -0x5F, NULL, FILE_CURRENT);
			}

			// unable to find embedded setup
			if (i >= 0x5000) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		}

		ReadFile(fileHandle, tempBuffer, 0x5000, &bytesRead, NULL);
		for (i = 0; i < 5000; i++)
		{
			// find section offset
			if ((tempBuffer[i] == '.') &&
				(tempBuffer[i + 1] == 't') &&
				(tempBuffer[i + 2] == 'e') &&
				(tempBuffer[i + 3] == 'x') &&
				(tempBuffer[i + 4] == 't'))
			{
				break;
			}
		}

		// find last section
		for (; numberOfFileSections > 1; numberOfFileSections--) i += 0x28;

		if (i >= 5000) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

		int baseOffset = i;
		// determine installation offset
		while (tempBuffer[i] != 0x00) i++;
		i += 16 - (i - baseOffset);

		// installation script size & raw offset
		rawdataSize = ULib.StringLibrary.getDWORDFromBuffer(tempBuffer, i);
		i += 4;
		rawdataOffset = ULib.StringLibrary.getDWORDFromBuffer(tempBuffer, i);

		if (DEBUG_MODE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("File Format\t: PE");
			ULib.ConsoleLibrary.displayConsoleMessage("RawData Offset\t: 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(rawdataOffset, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
			ULib.ConsoleLibrary.displayConsoleMessage("RawData Size\t: 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(rawdataSize, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
			if (embeddedSetup)
			{
				ULib.ConsoleLibrary.displayConsoleMessage("Embedded Offset\t: 0x", false);
				ULib.ConsoleLibrary.displayConsoleMessage(embeddedFileOffset, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
			}

			ULib.ConsoleLibrary.displayConsoleMessage("Script Offset\t: 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(rawdataSize + rawdataOffset + embeddedFileOffset, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		}

		if ((rawdataSize == 0x00) || (rawdataSize > 0xFFFF) || (rawdataOffset == 0x00)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

		// jump to script offset
		SetFilePointer(fileHandle, rawdataSize + rawdataOffset + embeddedFileOffset, NULL, FILE_BEGIN);
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	// see if a dll filename was supplied
	ReadFile(fileHandle, tempBuffer, 0x01, &bytesRead, NULL);
	// jump over dll filename and size (PE) / or move back a byte (NE)
	if (tempBuffer[0] != 0)
	{
		if (fileFormat == ULib.FileLibrary.FILEFORMAT_NE)
		{
			SetFilePointer(fileHandle, -1, NULL, FILE_CURRENT);
			scriptOffset--;
		}
		else if (fileFormat == ULib.FileLibrary.FILEFORMAT_PE)
		{
			SetFilePointer(fileHandle, tempBuffer[0], NULL, FILE_CURRENT);
			SetFilePointer(fileHandle, 4, NULL, FILE_CURRENT);
		}
	}

	// flags
	ReadFile(fileHandle, scriptFlags, 0x04, &bytesRead, NULL);
	// colours
	ReadFile(fileHandle, tempBuffer, 12, &bytesRead, NULL);
	// extra
	ReadFile(fileHandle, tempBuffer, 8, &bytesRead, NULL);
	// archive
	ReadFile(fileHandle, tempBuffer, 56, &bytesRead, NULL);
	// adjust current position (for older versions)
	if (fileFormat == ULib.FileLibrary.FILEFORMAT_NE)
	{
		if (ULib.StringLibrary.getWORDFromBuffer(tempBuffer, 32) == 0x17A) SetFilePointer(fileHandle, -16, NULL, FILE_CURRENT);
		if (ULib.StringLibrary.getWORDFromBuffer(tempBuffer, 36) == 0x17A) SetFilePointer(fileHandle, -12, NULL, FILE_CURRENT);
		if (ULib.StringLibrary.getWORDFromBuffer(tempBuffer, 40) == 0x17A) SetFilePointer(fileHandle, -8, NULL, FILE_CURRENT);
		if (ULib.StringLibrary.getWORDFromBuffer(tempBuffer, 44) == 0x17A) SetFilePointer(fileHandle, -4, NULL, FILE_CURRENT);
	}
	// script offset
	ReadFile(fileHandle, tempBuffer, 0x04, &bytesRead, NULL);
	SetFilePointer(fileHandle, -4, NULL, FILE_CURRENT);
	scriptOffset = ULib.StringLibrary.getDWORDFromBuffer(tempBuffer);

	// check for PKZIP mode
	if (scriptFlags[1] & 1) isPKZipMode = 1;

	// jump over startup information
	if ((scriptOffset & 0xFFFF0000) == 0x0000)
	{
		// script offset
		SetFilePointer(fileHandle, 4, NULL, FILE_CURRENT);
		// script file size
		SetFilePointer(fileHandle, 4, NULL, FILE_CURRENT);
		SetFilePointer(fileHandle, 2, NULL, FILE_CURRENT);

		// display font
		do { ReadFile(fileHandle, tempBuffer, 0x01, &bytesRead, NULL); } while (tempBuffer[0] != 0x00);
		// title message
		do { ReadFile(fileHandle, tempBuffer, 0x01, &bytesRead, NULL); } while (tempBuffer[0] != 0x00);
		// startup message
		do { ReadFile(fileHandle, tempBuffer, 0x01, &bytesRead, NULL); } while (tempBuffer[0] != 0x00);
		// jump to script offset
		SetFilePointer(fileHandle, scriptOffset, NULL, FILE_CURRENT);
	}

	unsigned long streamCRC = 0;
	unsigned long scriptCRC = 0;
	unsigned long scriptCompressedSize = 0;
	unsigned long scriptExpandedSize = 0;

	// skip PK header if in PKZip mode
	if (isPKZipMode)
	{
		ReadFile(fileHandle, tempBuffer, 0x1E, &bytesRead, NULL);
		SetFilePointer(fileHandle, (tempBuffer[0x1A] | (tempBuffer[0x1B] << 8)), NULL, FILE_CURRENT);
		SetFilePointer(fileHandle, (tempBuffer[0x1C] | (tempBuffer[0x1D] << 8)), NULL, FILE_CURRENT);
		scriptCRC = *((unsigned long*)(tempBuffer + 0x0E));
	}

	// read stream buffer
	unsigned long currentScriptOffset = GetFilePointer(fileHandle);
	ReadFile(fileHandle, scriptBuffer, 50000, &bytesRead, NULL);
	SetFilePointer(fileHandle, -50000, NULL, FILE_CURRENT);

	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Script Start\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(currentScriptOffset, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		ULib.ConsoleLibrary.displayConsoleMessage("");
	}

	// inflate stream
	int inflateResult = inflateWiseScript(argv[1], currentScriptOffset, scriptBuffer, &scriptCompressedSize, &scriptExpandedSize, &streamCRC);
	if (inflateResult != -1) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, inflateResult);

	// jump to the end of the script file
	SetFilePointer(fileHandle, scriptCompressedSize, NULL, FILE_CURRENT);
	// read end of file
	ReadFile(fileHandle, tempBuffer, 0x04, &bytesRead, NULL);

	// if we're in non-pkzip mode, this is the CRC of the file
	if (!isPKZipMode)
	{
		scriptCRC = *((unsigned long*)tempBuffer);
		// check for crc values
		if (scriptCRC != streamCRC)
		{
			SetFilePointer(fileHandle, -3, NULL, FILE_CURRENT);
			ReadFile(fileHandle, tempBuffer, 0x04, &bytesRead, NULL);
			scriptCRC = *((unsigned long*)tempBuffer);
		}
	}

	CloseHandle(fileHandle);

	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Compressed Size\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(scriptCompressedSize, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		ULib.ConsoleLibrary.displayConsoleMessage("Expanded Size\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(scriptExpandedSize, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		ULib.ConsoleLibrary.displayConsoleMessage("Stream CRC\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(streamCRC, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		ULib.ConsoleLibrary.displayConsoleMessage("Expected CRC\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(scriptCRC, true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		ULib.ConsoleLibrary.displayConsoleMessage("");
	}

	// check CRC
	if (streamCRC != scriptCRC) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);

	// save script file (if required)
	if (extractScript)
	{
		char* scriptFilenameStripped = ULib.FileLibrary.stripPathFromFilename(scriptFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Extracting Installation Script (", false);
		ULib.ConsoleLibrary.displayConsoleMessage(scriptFilenameStripped, false);
		ULib.ConsoleLibrary.displayConsoleMessage(")...");
		ULib.HeapLibrary.freePrivateHeap(scriptFilenameStripped);

		unsigned long bytesWritten = 0;
		HANDLE scriptFileHandle = CreateFile(scriptFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fileHandle == INVALID_HANDLE_VALUE) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);
		WriteFile(scriptFileHandle, scriptBuffer, scriptExpandedSize, &bytesWritten, NULL);
		CloseHandle(scriptFileHandle);
	}

	// default password offset
	int passwordOffset = 0x11;

	// check if timestamp is stored here (use 0x25 as passwordOffset)
	if ((scriptBuffer[passwordOffset] != 0x00) || (scriptBuffer[passwordOffset + 1] != 0x00))
	{
		unsigned long possibleTimestamp = ULib.StringLibrary.getDWORDFromBuffer(scriptBuffer, passwordOffset);
		if ((possibleTimestamp > 0x30000000) && (possibleTimestamp < 0x50000000)) passwordOffset = 0x25;
	}

	// check we are not in the middle of the install.log string
	if (!ULib.StringLibrary.isStringMatch((char*)&scriptBuffer[passwordOffset], "TALL.LOG", false, 8))
	{
		// store decrypted password
		for (i = 0; i < 70; i++)
		{
			if (scriptBuffer[passwordOffset + i] == 0x00) break;
			decryptedPassword[i] = (unsigned char)(scriptBuffer[passwordOffset + i] ^ 0xFF);
		}
	}

	if (decryptedPassword[0] == 0x00) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	MessageBeep(MB_ICONEXCLAMATION);
	ULib.ConsoleLibrary.displayConsoleMessage("Password Found\t: '", false);
	ULib.ConsoleLibrary.displayConsoleMessage(decryptedPassword, false);
	ULib.ConsoleLibrary.displayConsoleMessage("'");

	ULib.HeapLibrary.freePrivateHeap(scriptFilename);
	ULib.HeapLibrary.freePrivateHeap(tempBuffer);
	ULib.HeapLibrary.freePrivateHeap(scriptBuffer);
	ULib.HeapLibrary.freePrivateHeap(scriptFlags);
	ULib.HeapLibrary.freePrivateHeap(decryptedPassword);

	ULib.ProcessLibrary.exitProcessHandler();
}