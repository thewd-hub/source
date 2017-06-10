////////////////////////////////////////////////////
// Symantec Decryption Utility                    //
// (C)thewd, thewd@hotmail.com                    //
////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Utility that can decrypt & encrypt certain     //
// files supplied with Symantec products          //
// - Symantec pcAnywhere binary files             //
// - Norton AntiVirus configuration files         //
// - Norton Internet Security configuration files //
// - Other Symantec shared files                  //
//                                                //
// Tested with...                                 //
// Symantec pcAnywhere v8.1.0 - v11.5.0           //
// Norton AntiVirus 2002 - 2005                   //
// Norton Internet Security 2003-2005             //
////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\md5.h"
#include "..\..\..\_Shared Libraries\OpenSSL_BlowfishLibrary.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"{CRLF}Unable to open or read the specified file",
	"{CRLF}Doesn't appear to be a supported file for decryption",
	"{CRLF}Doesn't appear to be a supported file for encryption",
	"{CRLF}Error occurred during the file decryption routine",
	"{CRLF}Error occurred while saving the decrypted file",
	"{CRLF}Error occurred during the file encryption routine",
	"{CRLF}Error occurred while saving the encrypted file",
	"{CRLF}Error occurred validating the decrypted data - different md5 hashes"
};

/////////////////////////////////////////////////////////
// function used to handle files encrypted using symc1 //
/////////////////////////////////////////////////////////
void handleSYMC1Files(char* fileName, unsigned char* fileBuffer, unsigned long fileSize, bool doDecryption)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Recognised Format   : SYMC (v1.00)");

	unsigned char md5Hash[16];
	unsigned char encryptedKey[8] = {0x72, 0x8E, 0xB6, 0x58, 0xBC, 0xAF, 0x89, 0xAF};
	BF_KEY* bfKey = (BF_KEY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(bf_key_st));
	BF_set_key(bfKey, 8, encryptedKey);

	// decryption routine
	if (doDecryption)
	{
		// generate validation hash
		MD5 md5;
		MD5_CONTEXT md5context;
		md5.MD5Init(&md5context);
		md5.MD5Update(&md5context, fileBuffer, 0x0C);
		md5.MD5Update(&md5context, (unsigned char*)&fileBuffer[0x1C], fileSize - 0x1C);
		md5.MD5Final(md5Hash, &md5context);

		// decrypt file
		fileBuffer[0x03] = 0x43;
		for (unsigned long i = 0x0C; i < fileSize; i += 8) BF_decrypt((BF_LONG*)&fileBuffer[i], bfKey);

		// validated decrypted data
		if (memcmp((void*)&md5Hash[0], (void*)&fileBuffer[0x0C], 16) != 0) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 7);

		// save the decrypted file
		char* outputFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputFilename, "%s.decrypted", fileName);
		if (!ULib.FileLibrary.writeBufferToFile(outputFilename, fileBuffer, 0x0C)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
		if (!ULib.FileLibrary.appendBufferToFile(outputFilename, (unsigned char*)&fileBuffer[0x1C], fileSize - 0x1C)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);

		char* outputFilenameStripped = ULib.FileLibrary.stripPathFromFilename(outputFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Decryption Complete - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilename);
	}
	// encryption routine
	else
	{
		// encrypt file
		fileBuffer[0x03] = 0xFE;
		for (unsigned long i = 0x0C; i < fileSize; i += 8) BF_encrypt((BF_LONG*)&fileBuffer[i], bfKey);

		// generate validation hash
		MD5 md5;
		MD5_CONTEXT md5context;
		md5.MD5Init(&md5context);
		md5.MD5Update(&md5context, fileBuffer, fileSize);
		md5.MD5Final(md5Hash, &md5context);

		// encrypt hash
		BF_encrypt((BF_LONG*)&md5Hash[0], bfKey);
		BF_encrypt((BF_LONG*)&md5Hash[8], bfKey);

		// save the encrypted file
		char* outputFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputFilename, "%s.encrypted", fileName);
		if (!ULib.FileLibrary.writeBufferToFile(outputFilename, fileBuffer, 0x0C)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);
		if (!ULib.FileLibrary.appendBufferToFile(outputFilename, (unsigned char*)&md5Hash[0], 0x10)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);
		if (!ULib.FileLibrary.appendBufferToFile(outputFilename, (unsigned char*)&fileBuffer[0x0C], fileSize - 0x0C)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);

		char* outputFilenameStripped = ULib.FileLibrary.stripPathFromFilename(outputFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Encryption Complete - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilename);
	}

	ULib.HeapLibrary.freePrivateHeap(bfKey);
}

////////////////////////////////////////////////////
// function used to handle pcAnywhere v8.xx files //
////////////////////////////////////////////////////
void handlePCA8Files(char* fileName, unsigned char* fileBuffer, unsigned long fileSize, bool doDecryption)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Recognised Format   : pcAnywhere (v8.00)");

	// decryption routine
	if (doDecryption)
	{
		// decrypt buffer
		for (unsigned long i = (fileSize - 1); i > 0; i--) fileBuffer[i] = (unsigned char)((fileBuffer[i] ^ fileBuffer[i - 1]) ^ ((i - 1) & 0xFF));
		fileBuffer[0] ^= 0xAB;

		// decryption failed
		if (fileBuffer[0] != fileBuffer[1]) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

		// save the decrypted file
		char* outputFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputFilename, "%s.decrypted", fileName);
		if (!ULib.FileLibrary.writeBufferToFile(outputFilename, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);

		char* outputFilenameStripped = ULib.FileLibrary.stripPathFromFilename(outputFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Decryption Complete - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilename);
	}
	// encryption routine
	else
	{
		// encrypt buffer
		fileBuffer[0] ^= 0xAB;
		for (unsigned long i = 1; i < fileSize; i++) fileBuffer[i] = (unsigned char)((fileBuffer[i] ^ fileBuffer[i - 1]) ^ ((i - 1) & 0xFF));

		// encryption failed
		if (fileBuffer[1] != 0xAB) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);

		// save the encrypted file
		char* outputFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputFilename, "%s.encrypted", fileName);
		if (!ULib.FileLibrary.writeBufferToFile(outputFilename, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);

		char* outputFilenameStripped = ULib.FileLibrary.stripPathFromFilename(outputFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Encryption Complete - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilename);
	}
}

///////////////////////////////////////////////////////////////////
// used to process the pcAnywhere file based on the supplied key //
///////////////////////////////////////////////////////////////////
void processPCA10Buffer(unsigned char* fileBuffer, unsigned long fileSize, char* keyBuffer, unsigned long keyLength)
{
	for (unsigned long i = 0; i < fileSize; i++) fileBuffer[i] ^= (keyBuffer[i % keyLength] | 0x80);
}

/////////////////////////////////////////////////////
// function used to handle pcAnywhere v10.xx files //
/////////////////////////////////////////////////////
void handlePCA10Files(char* fileName, unsigned char* fileBuffer, unsigned long fileSize, bool doDecryption)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Recognised Format   : pcAnywhere (v10.00)");

	// decryption routine
	if (doDecryption)
	{
		// decrypt buffer
		processPCA10Buffer(fileBuffer, fileSize, "PTD", 3);
		processPCA10Buffer(fileBuffer, fileSize, "MoNtAuK-MaKo-TyPhOoN-EcLiPsE-", 29);
		processPCA10Buffer(fileBuffer, fileSize, "PcAnYwHeRe+", 11);
		processPCA10Buffer(fileBuffer, fileSize, "SyMaNtEcCeTnAmYs@", 17);

		// decryption failed
		if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer) != 0x00002000) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

		// save the decrypted file
		char* outputFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputFilename, "%s.decrypted", fileName);
		if (!ULib.FileLibrary.writeBufferToFile(outputFilename, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);

		char* outputFilenameStripped = ULib.FileLibrary.stripPathFromFilename(outputFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Decryption Complete - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilename);
	}
	// encryption routine
	else
	{
		// encrypt buffer
		processPCA10Buffer(fileBuffer, fileSize, "SyMaNtEcCeTnAmYs@", 17);
		processPCA10Buffer(fileBuffer, fileSize, "PcAnYwHeRe+", 11);
		processPCA10Buffer(fileBuffer, fileSize, "MoNtAuK-MaKo-TyPhOoN-EcLiPsE-", 29);
		processPCA10Buffer(fileBuffer, fileSize, "PTD", 3);

		// encryption failed
		if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer) != 0x2B06011E) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);

		// save the encrypted file
		char* outputFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputFilename, "%s.encrypted", fileName);
		if (!ULib.FileLibrary.writeBufferToFile(outputFilename, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);

		char* outputFilenameStripped = ULib.FileLibrary.stripPathFromFilename(outputFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Encryption Complete - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(outputFilename);
	}
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Symantec Decryption Utility", "1.00", 2003);
	bool displayUsage = true;

	if (argc > 1)
	{
		displayUsage = false;
		bool doDecryption = (((argc > 2) && (ULib.StringLibrary.isStringMatch("/encrypt", argv[2]))) ? false : true);

		ULib.ConsoleLibrary.displayConsoleMessage("Opening Binary File : ", false);
		char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(argv[1], true);
		ULib.ConsoleLibrary.displayConsoleMessage(fileNameStripped);
		ULib.HeapLibrary.freePrivateHeap(fileNameStripped);

		// read the file
		unsigned long fileSize = 0;
		unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(argv[1], &fileSize);
		if ((fileBuffer == NULL) || (fileSize < 0x1C)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

		// decryption routines
		if (doDecryption)
		{
			// symc (v1.00)
			if ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer) == 0xFE4D5953) && (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4) == 0x00000001)) handleSYMC1Files(argv[1], fileBuffer, fileSize, true);
			// symantec pcAnywhere (v10.00 & v11.00)
			else if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer) == 0x2B06011E) handlePCA10Files(argv[1], fileBuffer, fileSize, true);
			// symantec pcAnywhere (v8.00 & v9.00)
			else if (fileBuffer[1] == 0xAB) handlePCA8Files(argv[1], fileBuffer, fileSize, true);
			// unknown version
			else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
		}
		// encryption routines
		else
		{
			// symc (v1.00)
			if ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer) == 0x434D5953) && (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4) == 0x00000001)) handleSYMC1Files(argv[1], fileBuffer, fileSize, false);
			// symantec pcAnywhere (v10.00 & v11.00)
			else if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer) == 0x00002000) handlePCA10Files(argv[1], fileBuffer, fileSize, false);
			// symantec pcAnywhere (v8.00 & v9.00)
			else if (fileBuffer[0] == fileBuffer[1]) handlePCA8Files(argv[1], fileBuffer, fileSize, false);
			// unknown version
			else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		}

		ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	}

	// display usage
	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : symDecrypt <supported file> [/encrypt]");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}