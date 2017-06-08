////////////////////////////////////////////////////////////
// Wise for Windows Installer Launcher Password Cracker   //
// (C)thewd, thewd@hotmail.com                            //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Recovers the setup launcher password for installations //
// created with Wise for Windows Installer and Wise for   //
// Visual Studio .NET                                     //
//                                                        //
// Tested with...                                         //
// - v1.0.1                                               //
// - v2.0.2                                               //
// - v3.0.0, v3.5.2                                       //
// - v4.0.0, v4.0.1, v4.0.2, v4.2.0, v4.2.1               //
// - v5.0.0, v5.0.0 (VS.NET)                              //
// - v6.0.0                                               //
////////////////////////////////////////////////////////////
#include <windows.h>
#include <imagehlp.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Unable to open or read the Wise launcher file",
	"Doesn't appear to be a supported Wise launcher file",
	"Unable to find any encrypted password",
	"Unable to recover a suitable password"
};

CUtilitiesLibrary ULib;
bool DEBUG_MODE = false;

////////////////////////////////////////////////////////////////
// attempts to recover the original password (xor encryption) //
////////////////////////////////////////////////////////////////
char* recoverPassword(unsigned char* encryptedPassword, int encryptedPasswordLength)
{
	int currentIndex = 0;
	int currentXORValue = 0x5BC25A;
	char* decryptedPassword = (char*)ULib.HeapLibrary.allocPrivateHeap(encryptedPasswordLength + 1);

	// decryption - in groups of 4
	for (int i = 0; i < encryptedPasswordLength / 4; i++)
	{
		currentXORValue = (((currentXORValue % 0xADC8) * 0xBC8F) - ((currentXORValue / 0xADC8) * 0xD47));
		decryptedPassword[currentIndex + 0] = (unsigned char)(encryptedPassword[currentIndex + 0] ^ (currentXORValue & 0xFF));
		decryptedPassword[currentIndex + 1] = (unsigned char)(encryptedPassword[currentIndex + 1] ^ ((currentXORValue >> 8) & 0xFF));
		decryptedPassword[currentIndex + 2] = (unsigned char)(encryptedPassword[currentIndex + 2] ^ ((currentXORValue >> 16) & 0xFF));
		decryptedPassword[currentIndex + 3] = (unsigned char)(encryptedPassword[currentIndex + 3] ^ ((currentXORValue >> 24) & 0xFF));
		currentIndex += 4;
	}

	// decryption - remaining characters
	for (; currentIndex < encryptedPasswordLength; currentIndex++)
	{
		currentXORValue = (((currentXORValue % 0xADC8) * 0xBC8F) - ((currentXORValue / 0xADC8) * 0xD47));
		decryptedPassword[currentIndex] = (unsigned char)(encryptedPassword[currentIndex] ^ ((currentXORValue % 0xFF) + 1));
	}

	return decryptedPassword;
}

//////////////////////////////////////////////
// load the file, determine the location of //
// the password and attempt to recover it   //
//////////////////////////////////////////////
void parseFileAndFindPassword(char* fileName)
{
	LOADED_IMAGE loadedImage;
	unsigned long pRSRCData = NULL;

	if (!(MapAndLoad(fileName, 0, &loadedImage, FALSE, TRUE))) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	PIMAGE_SECTION_HEADER pSectionHdr = loadedImage.Sections;
	pSectionHdr += loadedImage.NumberOfSections;

	for (unsigned int index = loadedImage.NumberOfSections; index > 0; index--, pSectionHdr--)
	{
		// find wise section
		if (ULib.StringLibrary.isStringMatch((char*)pSectionHdr->Name, ".WISE"))
		{
			pRSRCData = pSectionHdr->PointerToRawData;
			break;
		}
	}

	if (pRSRCData == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	UnMapAndLoad(&loadedImage);

	// display file offset to header
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("File Offset     : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(pRSRCData, true, true);
	}

	unsigned long fileOffset = pRSRCData;
	unsigned char* fileBufferStart = ULib.FileLibrary.readBufferFromFile(fileName, NULL, fileOffset, 0x1000);
	unsigned char* fileBuffer = fileBufferStart;
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// determine header offset
	int headerOffset = 0;
	for (int i = 0; i < 0x60; i++)
	{
		if (((fileBuffer[i] == 0x00) || (fileBuffer[i] == 0x01)) &&
			(fileBuffer[i + 1] >= 0x28) &&
			((fileBuffer[i + 2] == 0x27) || (fileBuffer[i + 2] == 0x01)))
		{
			headerOffset = i + 1;
			break;
		}
	}

	if (headerOffset <= 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	fileBuffer += headerOffset;

	// move through buffer to password location
	int lengthFilename = fileBuffer[0] + 1;
	int lengthProductCode = fileBuffer[1] + 1;
	int lengthVersion = fileBuffer[2] + 1;
	int lengthUnknown2 = fileBuffer[3] + 1;
	fileBuffer += lengthFilename + lengthProductCode + lengthVersion + lengthUnknown2;
	fileBuffer += 0x02;
	int lengthDetails = fileBuffer[0];
	int lengthPassword = fileBuffer[1];

	// display debug information
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Header Offset   : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(headerOffset, true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Data Length");
		ULib.ConsoleLibrary.displayConsoleMessage("  Filename      : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(lengthFilename - 1, true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("  Product Code  : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(lengthProductCode - 1, true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("  Version       : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(lengthVersion - 1, true, true);
		//ULib.ConsoleLibrary.displayConsoleMessage("  UNKNOWN       : 0x", false);
		//ULib.ConsoleLibrary.displayConsoleMessage(lengthUnknown2 - 1, true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("  Details       : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(lengthDetails, true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("  Password      : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(lengthPassword, true, true);
	}

	// no password found
	if (lengthPassword == 1) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// jump over the remaining flags
	int jumpCount = 0;

	do
	{
		fileBuffer++;
		jumpCount++;

		// gone too far?
		if (jumpCount > 0x20) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	} while ((fileBuffer[0] != 0x4D) && (fileBuffer[1] != 0x53));

	// copy product name
	char* productName = ULib.StringLibrary.getNextTokenString((char*)fileBuffer, 0x00);
	fileBuffer += lengthDetails;
	// copy encrypted password
	unsigned char* encryptedPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(lengthPassword + 1);
	memcpy(encryptedPassword, fileBuffer, lengthPassword);

	// display encrypted password
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password Offset : 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(fileOffset + (unsigned long)(fileBuffer - fileBufferStart), true, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Password Bytes  : ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(encryptedPassword, lengthPassword, 0, true);
		ULib.ConsoleLibrary.displayConsoleMessage("");
	}

	// recover password
	char* foundPassword = recoverPassword(encryptedPassword, (lengthPassword - 1));

	// display results
	if (foundPassword != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Product Name    : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage((char*)productName);
		ULib.ConsoleLibrary.displayConsoleMessage("Password Found  : '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(foundPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'");
		MessageBeep(MB_ICONEXCLAMATION);
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	}

	ULib.HeapLibrary.freePrivateHeap(encryptedPassword);
	ULib.HeapLibrary.freePrivateHeap(foundPassword);
	ULib.HeapLibrary.freePrivateHeap(fileBufferStart);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Wise for Windows Installer Launcher Password Cracker", "1.01", 2002);

	if (argc > 1)
	{
		// check whether the shift key is depressed (enable debug mode)
		DEBUG_MODE = ULib.ProcessLibrary.isDebugModeEnabled();
		parseFileAndFindPassword(argv[1]);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tWFWIlauncher <setup launcher>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}