///////////////////////////////////////////////////////
// InstallShield PackageForTheWeb Password Cracker   //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// Support Routines for PFTW v2.03+                  //
///////////////////////////////////////////////////////
#include <windows.h>
#include "ISpftw203.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

extern char* errorMessages[];

/////////////////////////////////////////////////////////
// preprocessing of pftw header is required for v2.03+ //
/////////////////////////////////////////////////////////
void headerPreProcessing(unsigned char* fileBuffer, int headerSize)
{
	for (int index = 0; index < headerSize; index++)
	{
		unsigned char xc = (unsigned char)((fileBuffer[index] >> 4) | (fileBuffer[index] << 4));
		fileBuffer[index] = (unsigned char)(~xc);
	}
}

///////////////////////////////////////////////////////////////////////////
// attempts to determine the possible password length for long passwords //
///////////////////////////////////////////////////////////////////////////
int determineLongPasswordLength(unsigned char* pftwHeaderEncrypted, unsigned char* pftwHeaderDecrypted)
{
	unsigned char* pftwHeader = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	int maximumHeaderSize = 54;
	int knownHeaderSize = 28;
	int passwordLength = maximumHeaderSize;

	// encrypt the decrypted header based on the encrypted data
	for (int i = 0; i < maximumHeaderSize; i++)
	{
		// swap high and low words of encrypted data
		unsigned char xc = (unsigned char)((pftwHeaderEncrypted[i] >> 4) | (pftwHeaderEncrypted[i] << 4));
		pftwHeader[i] = (unsigned char)(~pftwHeaderDecrypted[i] ^ xc);
	}

	// check password length 1
	if (pftwHeader[0] == pftwHeader[1] == pftwHeader[2] == pftwHeader[3])
	{
		ULib.HeapLibrary.freePrivateHeap(pftwHeader);
		return 1;
	}

	// check password length 2
	if ((pftwHeader[0] == pftwHeader[2]) && (pftwHeader[1] == pftwHeader[3]))
	{
		ULib.HeapLibrary.freePrivateHeap(pftwHeader);
		return 2;
	}

	// check for a repeat pattern (within the known characters)
	for (int i = 2; i < (knownHeaderSize - 1); i++)
	{
		if ((pftwHeader[i] == pftwHeader[0]) && (pftwHeader[i + 1] == pftwHeader[1]))
		{
			ULib.HeapLibrary.freePrivateHeap(pftwHeader);
			return i;
		}
	}

	// otherwise search further into the header
	for (int i = 0; i < (knownHeaderSize - 1); i++)
	{
		// CFHEADER_Flags
		if ((pftwHeader[0x1E] == pftwHeader[i]) && (pftwHeader[0x1F] == pftwHeader[i + 1])) passwordLength = 0x1E - i;
		// CFHEADER_SetId
		else if ((pftwHeader[0x20] == pftwHeader[i]) && (pftwHeader[0x21] == pftwHeader[i + 1])) passwordLength = 0x20 - i;
		// CFHEADER_ICabinet
		else if ((pftwHeader[0x22] == pftwHeader[i]) && (pftwHeader[0x23] == pftwHeader[i + 1])) passwordLength = 0x22 - i;
		// CFFILE_UncompressedOffset (LOWORD)
		else if ((pftwHeader[0x30] == pftwHeader[i]) && (pftwHeader[0x31] == pftwHeader[i + 1])) passwordLength = 0x30 - i;
		// CFFILE_UncompressedOffset (HIWORD)
		else if ((pftwHeader[0x32] == pftwHeader[i]) && (pftwHeader[0x33] == pftwHeader[i + 1])) passwordLength = 0x32 - i;
		// CFFILE_IFolder
		else if ((pftwHeader[0x34] == pftwHeader[i]) && (pftwHeader[0x35] == pftwHeader[i + 1])) passwordLength = 0x34 - i;

		if (passwordLength != maximumHeaderSize) break;
	}

	ULib.HeapLibrary.freePrivateHeap(pftwHeader);
	return passwordLength;
}

/////////////////////////////////////////////////
// decryption of password is possible since no //
// other password is acceptable as the cabinet //
// file is encrypted with this password CRC    //
/////////////////////////////////////////////////
char* findPassword_v203(unsigned long Decrypted_CFHEADER_CabLength, unsigned char* encryptedData, bool getLongPassword, bool ignorePasswordLength, bool dumpHeaders)
{
	// setup decrypted information
	unsigned short Decrypted_CFHEADER_NumberOfCFFILES = 0;
	unsigned long Decrypted_CFFOLDER_CFDATAOffset = 0;
	unsigned short Decrypted_CFFOLDER_NumberOfCFDATAS = 0;
	unsigned short Decrypted_CFFOLDER_CompressionType = 1;
	unsigned long Decrypted_CFFILE_UncompressedSize = 0;

	unsigned char Decrypted_CFHEADER_Signature[4] = {'M', 'S', 'C', 'F'}; // Microsoft CAB Signature (MSCF)
	unsigned char Decrypted_CFHEADER_Reserved1[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char Decrypted_CFHEADER_Reserved2[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char Decrypted_CFHEADER_CFFILEOffset[4] = {0x2C, 0x00, 0x00, 0x00}; // First File Offset
	unsigned char Decrypted_CFHEADER_Reserved3[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char Decrypted_CFHEADER_CabVersionMinor = 0x03; // x.03
	unsigned char Decrypted_CFHEADER_CabVersionMajor = 0x01; // 1.xx
	unsigned char Decrypted_CFHEADER_NumberOfCFFOLDERS[2] = {0x01, 0x00};
	unsigned char Decrypted_CFHEADER_Flags[2] = {0x00, 0x00};
	unsigned char Decrypted_CFHEADER_SetId[2] = {0x39, 0x30}; // 0x3039 (12345)
	unsigned char Decrypted_CFHEADER_ICabinet[2] = {0x00, 0x00};
	unsigned char Decrypted_CFFILE_UncompressedOffset[4] = {0x00, 0x00, 0x00, 0x00};
	unsigned char Decrypted_CFFILE_IFolder[2] = {0x00, 0x00};

	// created decrypted pftw header
	PFTWCAB DecryptedPFTWCab =
	{
		ULib.StringLibrary.getDWORDFromBuffer(Decrypted_CFHEADER_Signature),
		ULib.StringLibrary.getDWORDFromBuffer(Decrypted_CFHEADER_Reserved1),
		Decrypted_CFHEADER_CabLength,
		ULib.StringLibrary.getDWORDFromBuffer(Decrypted_CFHEADER_Reserved2),
		ULib.StringLibrary.getDWORDFromBuffer(Decrypted_CFHEADER_CFFILEOffset),
		ULib.StringLibrary.getDWORDFromBuffer(Decrypted_CFHEADER_Reserved3),
		Decrypted_CFHEADER_CabVersionMinor,
		Decrypted_CFHEADER_CabVersionMajor,
		ULib.StringLibrary.getWORDFromBuffer(Decrypted_CFHEADER_NumberOfCFFOLDERS),
		Decrypted_CFHEADER_NumberOfCFFILES,
		ULib.StringLibrary.getWORDFromBuffer(Decrypted_CFHEADER_Flags),
		ULib.StringLibrary.getWORDFromBuffer(Decrypted_CFHEADER_SetId),
		ULib.StringLibrary.getWORDFromBuffer(Decrypted_CFHEADER_ICabinet),
		Decrypted_CFFOLDER_CFDATAOffset,
		Decrypted_CFFOLDER_NumberOfCFDATAS,
		Decrypted_CFFOLDER_CompressionType,
		Decrypted_CFFILE_UncompressedSize,
		ULib.StringLibrary.getDWORDFromBuffer(Decrypted_CFFILE_UncompressedOffset),
		ULib.StringLibrary.getWORDFromBuffer(Decrypted_CFFILE_IFolder)
	};

	unsigned char CryptStr[4] = {0x13, 0x35, 0x86, 0x07}; // 0x07863513
	int headerSize = ((getLongPassword) ? 54 : 28);
	char* possiblePassword = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
	char* consoleInput = NULL;
	int possiblePasswordLength = headerSize;

	// get required information for long passwords
	if (getLongPassword)
	{
		// try to determine the password length
		if (!ignorePasswordLength) possiblePasswordLength = determineLongPasswordLength(encryptedData, (unsigned char*)&DecryptedPFTWCab);

		ULib.ConsoleLibrary.displayConsoleMessage("[Long Password Parameters (28-54 Characters)]");

		if (!ignorePasswordLength)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Possible Length    : ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(possiblePasswordLength, false, false);
			ULib.ConsoleLibrary.displayConsoleMessage(" characters");
		}

		if (possiblePasswordLength > 28)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Number Of Files    : ", false);
			consoleInput = (char*)ULib.ConsoleLibrary.getConsoleInput(10);
			Decrypted_CFHEADER_NumberOfCFFILES = ULib.StringLibrary.getWORDFromString(consoleInput);
			ULib.HeapLibrary.freePrivateHeap(consoleInput);
		}

		if (possiblePasswordLength > 36)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Filenames Length   : ", false);
			consoleInput = (char*)ULib.ConsoleLibrary.getConsoleInput(10);
			Decrypted_CFFOLDER_CFDATAOffset = 0x2C + (Decrypted_CFHEADER_NumberOfCFFILES * 17) + ULib.StringLibrary.getDWORDFromString(consoleInput);
			ULib.HeapLibrary.freePrivateHeap(consoleInput);
		}

		if (possiblePasswordLength > 40)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Number Of Chunks   : ", false);
			consoleInput = (char*)ULib.ConsoleLibrary.getConsoleInput(10);
			Decrypted_CFFOLDER_NumberOfCFDATAS = ULib.StringLibrary.getWORDFromString(consoleInput);
			ULib.HeapLibrary.freePrivateHeap(consoleInput);
		}

		if (possiblePasswordLength > 42)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Is Compressed?     : ", false);
			bool areFilesCompressed = ULib.ConsoleLibrary.getConsoleInputAsYesNo(true, true);
			Decrypted_CFFOLDER_CompressionType = (unsigned short)((areFilesCompressed) ? 1 : 0);
			ULib.ConsoleLibrary.displayConsoleMessage(((areFilesCompressed) ? "Yes" : "No"));
		}

		if (possiblePasswordLength > 44)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Size Of First File : ", false);
			consoleInput = (char*)ULib.ConsoleLibrary.getConsoleInput(10);
			Decrypted_CFFILE_UncompressedSize = ULib.StringLibrary.getDWORDFromString(consoleInput);
			ULib.HeapLibrary.freePrivateHeap(consoleInput);
		}

		ULib.ConsoleLibrary.displayConsoleMessage("");

		DecryptedPFTWCab.CFHEADER_NumberOfCFFILES = Decrypted_CFHEADER_NumberOfCFFILES;
		DecryptedPFTWCab.CFFOLDER_CFDATAOffset = Decrypted_CFFOLDER_CFDATAOffset;
		DecryptedPFTWCab.CFFOLDER_NumberOfCFDATAS = Decrypted_CFFOLDER_NumberOfCFDATAS;
		DecryptedPFTWCab.CFFOLDER_CompressionType = Decrypted_CFFOLDER_CompressionType;
		DecryptedPFTWCab.CFFILE_UncompressedSize = Decrypted_CFFILE_UncompressedSize;
	}

	unsigned char* pEnc = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(headerSize);
	unsigned char* pDec = (unsigned char*)&DecryptedPFTWCab;
	memcpy(pEnc, encryptedData, headerSize);

	// dump decrypted header
	if (dumpHeaders) ULib.FileLibrary.writeBufferToFile("pftwHeader.decrypted.txt", pDec, headerSize);

	__try
	{
		// encrypt the decrypted header based on the encrypted data
		for (int i = 0; i < headerSize; i++)
		{
			// swap high and low words of encrypted data
			unsigned char xc = (unsigned char)((pEnc[i] >> 4) | (pEnc[i] << 4));
			pDec[i] = (unsigned char)(~pDec[i] ^ xc);
		}

		// dump encrypted header
		if (dumpHeaders) ULib.FileLibrary.writeBufferToFile("pftwHeader.encrypted.txt", pDec, headerSize);

		// unable to find password length, try original search method
		if (possiblePasswordLength == headerSize)
		{
			// recover password and try to determine the password length
			for (possiblePasswordLength = 1; possiblePasswordLength <= headerSize; possiblePasswordLength++)
			{
				// decrypt file based on password size
				for (int i = 0; i < headerSize; i++) pEnc[i] = (unsigned char)(pDec[i] ^ CryptStr[(i % possiblePasswordLength) % 4]);

				if (ULib.StringLibrary.isStringMatch((char*)pEnc, (char*)pEnc + possiblePasswordLength, true, headerSize - possiblePasswordLength))
				{
					// password found
					ULib.StringLibrary.copyString(possiblePassword, (char*)pEnc, possiblePasswordLength);
					possiblePassword[possiblePasswordLength] = NULL;
					break;
				}
			}
		}
		else
		{
			// decrypt file based on password size
			for (int i = 0; i < headerSize; i++) pEnc[i] = (unsigned char)(pDec[i] ^ CryptStr[(i % possiblePasswordLength) % 4]);
			ULib.StringLibrary.copyString(possiblePassword, (char*)pEnc, possiblePasswordLength);
			possiblePassword[possiblePasswordLength] = NULL;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		possiblePassword[0] = NULL;
	}

	ULib.HeapLibrary.freePrivateHeap(pEnc);
	if (possiblePassword[0] == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	return possiblePassword;
}