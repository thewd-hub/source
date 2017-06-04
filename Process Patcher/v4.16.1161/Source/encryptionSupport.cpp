//////////////////////////////////////////////////////////////////
// Uses the RC4 algorithm to encrypt or decrypt the script file //
// (Supported by Process Patcher v2.50+                         //
//////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "encryptionSupport.h"
#include "engineParsing.h"
#include "errorMessageHandler.h"
#include "ppatcherSupport.h"

// checks the start of the buffer to see whether the file is encrypted
bool isFileBufferEncrypted(unsigned char* fileBuffer)
{
	// process patcher script isn't encrypted
	if (ULib.StringLibrary.isStringMatch((char*)fileBuffer, SCRIPTHEADER_DECID, false, ULib.StringLibrary.getStringLength(SCRIPTHEADER_DECID))) return false;
	// already encrypted with RC4 algorithm (by checking the encryption header)
	if (ULib.StringLibrary.isStringMatch((char*)fileBuffer, SCRIPTHEADER_ENCID, false, ULib.StringLibrary.getStringLength(SCRIPTHEADER_ENCID))) return true;
	// check for Blowfish encryption (algorithm is no longer supported)
	if ((fileBuffer[0] == 'H') && (fileBuffer[1] == 'Y')) displayErrorMessage(ERROR_GENERAL_BLOWFISHENCRYPTION, true);
	// unknown or unsupported script file
	displayErrorMessage(ERROR_PARSING_SCRIPT_INVALID, true);
	return false;
}

// decrypt or encrypt the script file (in memory) using the RC4 algorithm
void encryptFileBuffer(unsigned char* fileBuffer, unsigned long fileBufferLength, bool tryDefaultKey, bool isEncryptionKey)
{
	char* correctKey = NULL;

	// if we are trying the default key
	if (tryDefaultKey)
	{
		correctKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(correctKey, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, SCRIPT_DEFAULTKEY);
	}
	else
	{
		// ask the user for the encryption/decryption key
		displayConsoleMessage(((isEncryptionKey) ? "Encryption Key   : " : "Decryption Key   : "), false);
		correctKey = (char*)ULib.ConsoleLibrary.getConsoleInput(16);
	}

	// invalid key entered
	if (correctKey == NULL)
	{
		displayConsoleMessage("");
		displayErrorMessage(((isEncryptionKey) ? ERROR_SCRIPT_ENCRYPTION_VALIDKEY : ERROR_SCRIPT_DECRYPTION_VALIDKEY), true);
	}
	// key length
	int correctKeyLength = ULib.StringLibrary.getStringLength(correctKey);
	// check key length
	if ((correctKeyLength <= 0) || (correctKeyLength > 16))
	{
		displayConsoleMessage("");
		displayErrorMessage(((isEncryptionKey) ? ERROR_SCRIPT_ENCRYPTION_VALIDLENGTH : ERROR_SCRIPT_DECRYPTION_VALIDLENGTH), true);
	}

	// rc4 the file buffer using the supplied key
	ULib.EncryptionLibrary.RC4Library.encryptBuffer((unsigned char*)correctKey, correctKeyLength, fileBuffer, 0, fileBufferLength);
	ULib.HeapLibrary.freePrivateHeap(correctKey);
}

// encrypts the script file with the supplied encryption key
void encryptScriptFile(char* scriptFilename, bool useDefaultKey)
{
	// display information
	char* scriptFilenameStripped = ULib.FileLibrary.stripPathFromFilename(scriptFilename, true);
	displayConsoleMessage("[Encrypting Script File]");
	displayConsoleMessage("  Script Filename  : ", false);
	displayConsoleMessage(scriptFilenameStripped);
	ULib.HeapLibrary.freePrivateHeap(scriptFilenameStripped);

	// open and read script file
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(scriptFilename, &fileSize);
	if ((fileBuffer == NULL) || (fileSize <= 0))
	{
		displayConsoleMessage("");
		displayErrorMessage(ERROR_PARSING_SCRIPT_NOTFOUND, true);
	}

	// is the script file already encrypted?
	if (isFileBufferEncrypted(fileBuffer))
	{
		displayConsoleMessage("");
		displayErrorMessage(ERROR_SCRIPT_ENCRYPTION_ALREADYENCRYPTED, true);
	}

	// encrypt the script file
	if (!useDefaultKey) displayConsoleMessage("  ", false);
	encryptFileBuffer(fileBuffer, fileSize, useDefaultKey, true);

	displayConsoleMessage("  Encrypting File  - ", false);

	// create backup copy of the script file
	char* backupScriptFilename = ULib.FileLibrary.getBackupFilename(scriptFilename);
	if (CopyFile(scriptFilename, backupScriptFilename, TRUE) == 0)
	{
		displayConsoleMessage("FAILED");
		displayConsoleMessage("");
		displayErrorMessage(ERROR_SCRIPT_ENCRYPTION_BACKUP, true);
	}
	ULib.HeapLibrary.freePrivateHeap(backupScriptFilename);

	// write encryption header
	if (!ULib.FileLibrary.writeBufferToFile(scriptFilename, (unsigned char*)SCRIPTHEADER_ENCID, 0x10))
	{
		displayConsoleMessage("FAILED");
		displayConsoleMessage("");
		displayErrorMessage(ERROR_SCRIPT_ENCRYPTION_GENERAL, true);
	}
	// write the rc4 encryption buffer
	if (!ULib.FileLibrary.appendBufferToFile(scriptFilename, fileBuffer, fileSize))
	{
		displayConsoleMessage("FAILED");
		displayConsoleMessage("");
		displayErrorMessage(ERROR_SCRIPT_ENCRYPTION_GENERAL, true);
	}

	displayConsoleMessage("OK");
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

// decrypt the script file with the decryption key
void decryptScriptFile(char* scriptFilename, bool useDefaultKey)
{
	if (ALLOW_SCRIPT_DECRYPTION)
	{
		// display information
		char* scriptFilenameStripped = ULib.FileLibrary.stripPathFromFilename(scriptFilename, true);
		displayConsoleMessage("[Decrypting Script File]");
		displayConsoleMessage("  Script Filename  : ", false);
		displayConsoleMessage(scriptFilenameStripped);
		ULib.HeapLibrary.freePrivateHeap(scriptFilenameStripped);

		// open and read script file
		unsigned long fileSize = 0;
		unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(scriptFilename, &fileSize);
		if ((fileBuffer == NULL) || (fileSize <= 0))
		{
			displayConsoleMessage("");
			displayErrorMessage(ERROR_PARSING_SCRIPT_NOTFOUND, true);
		}

		// is the script file already encrypted?
		if (!isFileBufferEncrypted(fileBuffer))
		{
			displayConsoleMessage("");
			displayErrorMessage(ERROR_SCRIPT_DECRYPTION_NOTENCRYPTED, true);
		}

		// decrypt the script file
		fileSize -= 0x10;
		if (!useDefaultKey) displayConsoleMessage("  ", false);
		encryptFileBuffer((unsigned char*)&fileBuffer[0x10], fileSize, useDefaultKey, false);

		displayConsoleMessage("  Decrypting File  - ", false);

		// check for successful decryption
		if (!ULib.StringLibrary.isStringMatch((char*)&fileBuffer[0x10], SCRIPTHEADER_DECID, false, ULib.StringLibrary.getStringLength(SCRIPTHEADER_DECID)))
		{
			displayConsoleMessage("FAILED");
			displayConsoleMessage("");
			displayErrorMessage(ERROR_SCRIPT_DECRYPTION_GENERAL, true);
		}

		// write encryption header
		if (!ULib.FileLibrary.writeBufferToFile(scriptFilename, (unsigned char*)&fileBuffer[0x10], fileSize))
		{
			displayConsoleMessage("FAILED");
			displayConsoleMessage("");
			displayErrorMessage(ERROR_SCRIPT_DECRYPTION_GENERAL, true);
		}

		displayConsoleMessage("OK");
		ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	}
}