///////////////////////////////////////////////////////
// InstallShield PackageForTheWeb Password Cracker   //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// Recovers password for self-extracting executables //
// packaged with InstallShield PackageForTheWeb      //
//                                                   //
// Tested with...                                    //
// - v1.02, v1.21, v1.32                             //
// - v2.02, v2.03, v2.04, v2.05, v2.10               //
// - v3.00                                           //
// - v4.00, v4.01                                    //
//                                                   //
// v2.03+ - password <= 28 characters are guaranteed //
///////////////////////////////////////////////////////
#include <windows.h>
#include "ISpftw2.h"
#include "ISpftw203.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include <imagehlp.h>

char* errorMessages[] =
{
	"Unable to open or read the PackageForTheWeb executable",
	"Doesn't appear to be a supported PackageForTheWeb cabinet file",
	"Unable to recognise the PackageForTheWeb cabinet format",
	"This PackageForTheWeb cabinet file is not password protected",
	"Unable to recover a suitable password"
};

CUtilitiesLibrary ULib;

char* pftwVersion;
char* productName;
char* productVersion;
char* companyName;
char* pftwPassword;

bool pftw_v203 = false;
bool pftw_v203_GetLongPassword = false;
bool pftw_v203_IgnorePasswordLength = false;
bool pftw_v203_DumpHeaders = false;

/////////////////////////////////////////////////////
// checks for a valid PFTW header and determines   //
// the format version. Also retrieves the password //
// or password crc, product name, product version  //
// and company information from the header         //
/////////////////////////////////////////////////////
void decryptSection(unsigned char* fileBuffer, int headerSize, unsigned char* encryptedData)
{
	// v2.03+ requires pre-processing before
	// the following decryption can proceed
	if ((fileBuffer[0] == 0xDC) &&
		(fileBuffer[1] == 0xED) &&
		(fileBuffer[2] == 0xBD))
	{
		headerPreProcessing(fileBuffer, headerSize);
		pftw_v203 = true;
	}

	// decrypt the package information including password
	char currentChar = 'a';
	for (int index = 0; index < headerSize; index++)
	{
		fileBuffer[index] = (unsigned char)(fileBuffer[index] ^ currentChar);
		if (++currentChar > 'z') currentChar = 'a';
	}

	// check for PFTW Header Id
	if ((fileBuffer[0] != 'S') &&
		(fileBuffer[1] != 'C') &&
		(fileBuffer[2] != 'G'))
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	// check for the rest of the PFTW Header Id and where the sections begin
	int index = 0;
	bool PFTWHeaderFound = false;
	while (index < 14)
	{
		if ((fileBuffer[index + 0] == 0x97) &&
			(fileBuffer[index + 1] == 0x01) &&
			(fileBuffer[index + 2] == 0x96))
		{
			PFTWHeaderFound = true;
			break;
		}

		index++;
	}

	if (!PFTWHeaderFound) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	bool passwordFound = false;
	int numberOfSections = fileBuffer[index + 3];
	int sectionOffset = (index + 4);

	if (!pftw_v203)
	{
		if (sectionOffset == 11) pftwVersion = "v1.0x";
		if (sectionOffset == 12) pftwVersion = "v1.3x";
	}

	companyName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	productName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	productVersion = (char*)ULib.HeapLibrary.allocPrivateHeap();
	pftwPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();

	int section = 1;
	bool finished = false;

	while ((section <= numberOfSections) && (!finished))
	{
		if (sectionOffset > (headerSize - 4)) break;
		int sectionNumber = fileBuffer[sectionOffset++];
		int sectionLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, sectionOffset++);
		if ((sectionOffset + sectionLength) >= headerSize) break;

		if (sectionNumber == 0x0C) passwordFound = true;
		// if the password hasn't been found by now then it is not password protected
		// or this is not a recognised pftw file format
		sectionOffset++;

		if (sectionNumber == 0x03) ULib.StringLibrary.copyString(companyName, (char*)&fileBuffer[sectionOffset], sectionLength);
		else if (sectionNumber == 0x04) ULib.StringLibrary.copyString(productName, (char*)&fileBuffer[sectionOffset], sectionLength);
		else if (sectionNumber == 0x05) ULib.StringLibrary.copyString(productVersion, (char*)&fileBuffer[sectionOffset], sectionLength);
		else if (sectionNumber == 0x0C) ULib.StringLibrary.copyString(pftwPassword, (char*)&fileBuffer[sectionOffset], sectionLength);

		sectionOffset += sectionLength;
		section++;
	}

	// password not found
	if (!passwordFound)
	{
		if (pftw_v203) pftwVersion = "v2.00 - v4.01";
		pftwPassword = NULL;
	}
	else
	{
		// v2.00 - v2.02, password is no longer stored in clear text but as a CRC.
		// Must brute force a password. v2.03+ password can be decrypted
		if (((pftwPassword[0] == 'F') && (pftwPassword[1] == '4')) || (pftw_v203))
		{
			// v2.03+
			if (pftw_v203)
			{
				pftwVersion = "v2.03 - v4.01";
				ULib.HeapLibrary.freePrivateHeap(pftwPassword);
				pftwPassword = findPassword_v203(ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4), encryptedData, pftw_v203_GetLongPassword, pftw_v203_IgnorePasswordLength, pftw_v203_DumpHeaders);
			}
			else
			{
				for (int index = 0; index < 6; index++)
				{
					int tempValue = pftwPassword[index];
					if (tempValue > 0x39) tempValue -= 0x07;
					tempValue -= 0x30;
					pftwPassword[index] = (char)tempValue;
				}

				int passwordCRC = pftwPassword[5] +
					pftwPassword[4] * 0x10 +
					pftwPassword[3] * 0x100 +
					pftwPassword[2] * 0x1000 +
					pftwPassword[1] * 0x10000 +
					pftwPassword[0] * 0x100000;

				pftwVersion = "v2.00 - v2.02";
				ULib.HeapLibrary.freePrivateHeap(pftwPassword);
				pftwPassword = findPassword_v2(passwordCRC);
			}
		}
	}
}

//////////////////////////////////////////////////
// All PFTW packages are PE format. Determines  //
// the file offset of the PFTW header and reads //
// the header into a buffer                     //
//////////////////////////////////////////////////
void parsePEImageAndRecoverPassword(char* fileName)
{
	LOADED_IMAGE loadedImage;
	unsigned long pRSRCData = NULL;

	if (!(MapAndLoad(fileName, 0, &loadedImage, FALSE, TRUE))) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	PIMAGE_SECTION_HEADER pSectionHdr = loadedImage.Sections;
	pSectionHdr += loadedImage.NumberOfSections;

	for (unsigned int index = loadedImage.NumberOfSections; index > 0; index--, pSectionHdr--)
	{
		// v1.0x uses the .reloc section
		if (ULib.StringLibrary.isStringMatch((char*)pSectionHdr->Name, ".reloc"))
		{
			pRSRCData = pSectionHdr->PointerToRawData + pSectionHdr->SizeOfRawData;
			break;
		}

		// v1.3x uses the _cabinet section
		if (ULib.StringLibrary.isStringMatch((char*)pSectionHdr->Name, "_cabinet"))
		{
			pRSRCData = pSectionHdr->PointerToRawData + 0x0C;
			break;
		}

		// otherwise use the .rsrc section for all other versions
		if (ULib.StringLibrary.isStringMatch((char*)pSectionHdr->Name, ".rsrc"))
		{
			pRSRCData = pSectionHdr->PointerToRawData + pSectionHdr->SizeOfRawData;
			break;
		}
	}

	if (pRSRCData == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	UnMapAndLoad(&loadedImage);

	unsigned long fileOffset = pRSRCData;
	unsigned char* bufferSize = ULib.FileLibrary.readBufferFromFile(fileName, NULL, fileOffset, 4);
	if (bufferSize == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	unsigned long headerSize = ULib.StringLibrary.getDWORDFromBuffer(bufferSize);
	ULib.HeapLibrary.freePrivateHeap(bufferSize);
	fileOffset += 4;

	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, NULL, fileOffset, headerSize);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	fileOffset += headerSize;

	// read 54 bytes (maximum guaranteed password is 28 characters) of encrypted data for decryption process for v2.03+
	unsigned char* encryptedData = ULib.FileLibrary.readBufferFromFile(fileName, NULL, fileOffset, 54);
	if (encryptedData == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// decrypt the section retrieved
	decryptSection(fileBuffer, headerSize, encryptedData);
	ULib.HeapLibrary.freePrivateHeap(encryptedData);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("InstallShield PackageForTheWeb Password Cracker", "1.02", 1999);

	if (argc > 1)
	{
		// check for additional parameters
		for (int i = 2; i < argc; i++)
		{
			if (ULib.StringLibrary.isStringMatch(argv[i], "/longPassword")) pftw_v203_GetLongPassword = true;
			if (ULib.StringLibrary.isStringMatch(argv[i], "/ignoreLength")) pftw_v203_IgnorePasswordLength = true;
			if (ULib.StringLibrary.isStringMatch(argv[i], "/dumpHeaders")) pftw_v203_DumpHeaders = true;
		}

		parsePEImageAndRecoverPassword(argv[1]);

		char* passwordResult = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(passwordResult, "PFTW Header Format : %s\r\n", pftwVersion);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordResult);
		wsprintf(passwordResult, "Product Title      : %s", productName);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordResult);
		wsprintf(passwordResult, "Product Version    : %s", productVersion);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordResult);
		wsprintf(passwordResult, "Company Title      : %s\r\n", companyName);
		ULib.ConsoleLibrary.displayConsoleMessage(passwordResult);
		ULib.HeapLibrary.freePrivateHeap(companyName);
		ULib.HeapLibrary.freePrivateHeap(productName);
		ULib.HeapLibrary.freePrivateHeap(productVersion);

		// package is not password protected
		if (pftwPassword == NULL)
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
		}
		else
		{
			wsprintf(passwordResult, "Recovered Password : '%s'%s", pftwPassword, (((pftw_v203) && (!pftw_v203_GetLongPassword)) ? " (Max 28 Characters)" : ""));
			ULib.ConsoleLibrary.displayConsoleMessage(passwordResult);
			MessageBeep(MB_ICONEXCLAMATION);
		}

		ULib.HeapLibrary.freePrivateHeap(pftwPassword);
		ULib.HeapLibrary.freePrivateHeap(passwordResult);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tISpftw <pftw executable> [/longPassword] [/ignoreLength] [/dumpHeaders]");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}