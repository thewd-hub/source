#include "ckInfo.h"
#include "ckCustomInfoBytes.h"
#include "ckEncryption.h"
#include "ckEncryption6.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

void decryptCEDLicence(char* fileName, char* fileNameOutput)
{
	char* outputBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(outputBuffer, "Opening CED Licence File (%s)...", ULib.FileLibrary.stripPathFromFilename(fileName, true));
	ULib.ConsoleLibrary.displayConsoleMessage(outputBuffer);

	// read custom information bytes file
	unsigned long fileSize = 0;
	unsigned char* encryptedBytes = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if (encryptedBytes == NULL) displayErrorMessage(ERROR_CUSTOMINFOBYTES_FILENAME);
	//if (fileSize != 640) displayErrorMessage(ERROR_CUSTOMINFOBYTES_FILESIZE);

	// decrypt file buffer
	unsigned long tempBufferLength = 0;
	fileSize = 0x80;
	ULib.ConsoleLibrary.displayConsoleMessage(fileSize, false, true);
	unsigned char* tempBuffer = decryptKey6Stream(encryptedBytes, fileSize, &tempBufferLength, 0x80, KEYTYPE_CEDLICENCE);
	bool decryptionFailed = (tempBufferLength == 0);
	tempBufferLength = fileSize;
	ULib.ConsoleLibrary.displayConsoleMessage("+ Decrypting Custom Info Bytes - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(((decryptionFailed) ? "FAILED" : "OK"));
	//if (decryptionFailed) return;

	// validate decryption
	ULib.ConsoleLibrary.displayConsoleMessage("+ Validating Decrypted Bytes   - ", false);
	//if (tempBufferLength != 512) return ULib.ConsoleLibrary.displayConsoleMessage("FAILED (Incorrect Size)");
	//if ((getKeyCRC(tempBuffer, 512, KEYTYPE_CUSTOMINFOBYTES)) != (ULib.StringLibrary.getWORDFromBuffer(tempBuffer, 0x1FE, true))) return ULib.ConsoleLibrary.displayConsoleMessage("FAILED (Invalid CRC)");
	ULib.ConsoleLibrary.displayConsoleMessage("OK");

	// WORD at HEADER[2] is checked for match with request to CRYPKEY SERVICE
	// save decrypted buffer to file (ignoring header & crc value at end)
	ULib.ConsoleLibrary.displayConsoleMessage("+ Saving Decrypted Info Bytes  - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(((ULib.FileLibrary.writeBufferToFile(fileNameOutput, (unsigned char*)&tempBuffer[0], fileSize)) ? "OK" : "FAILED"));
	ULib.HeapLibrary.freePrivateHeap(tempBuffer);
}

//////////////////////////////////////////////////
// decrypt custom information bytes (from file) //
//////////////////////////////////////////////////
void decryptCustomInfoBytes(char* fileName, char* fileNameOutput)
{
	char* outputBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(outputBuffer, "Opening Custom Information Bytes File (%s)...", ULib.FileLibrary.stripPathFromFilename(fileName, true));
	ULib.ConsoleLibrary.displayConsoleMessage(outputBuffer);

	// read custom information bytes file
	unsigned long fileSize = 0;
	unsigned char* encryptedBytes = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if (encryptedBytes == NULL) displayErrorMessage(ERROR_CUSTOMINFOBYTES_FILENAME);
	if (fileSize != 640) displayErrorMessage(ERROR_CUSTOMINFOBYTES_FILESIZE);

	// decrypt file buffer
	unsigned long tempBufferLength = 0;
	unsigned char* tempBuffer = decryptKey6Stream(encryptedBytes, fileSize, &tempBufferLength, 0x80, KEYTYPE_CUSTOMINFOBYTES);
	bool decryptionFailed = (tempBufferLength == 0);
	ULib.ConsoleLibrary.displayConsoleMessage("+ Decrypting Custom Info Bytes - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(((decryptionFailed) ? "FAILED" : "OK"));
	if (decryptionFailed) return;

	// validate decryption
	ULib.ConsoleLibrary.displayConsoleMessage("+ Validating Decrypted Bytes   - ", false);
	if (tempBufferLength != 512) return ULib.ConsoleLibrary.displayConsoleMessage("FAILED (Incorrect Size)");
	if ((getKeyCRC(tempBuffer, 512, KEYTYPE_CUSTOMINFOBYTES)) != (ULib.StringLibrary.getWORDFromBuffer(tempBuffer, 0x1FE, true))) return ULib.ConsoleLibrary.displayConsoleMessage("FAILED (Invalid CRC)");
	ULib.ConsoleLibrary.displayConsoleMessage("OK");

	// WORD at HEADER[2] is checked for match with request to CRYPKEY SERVICE
	// save decrypted buffer to file (ignoring header & crc value at end)
	ULib.ConsoleLibrary.displayConsoleMessage("+ Saving Decrypted Info Bytes  - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(((ULib.FileLibrary.writeBufferToFile(fileNameOutput, (unsigned char*)&tempBuffer[0], 500)) ? "OK" : "FAILED"));
	ULib.HeapLibrary.freePrivateHeap(tempBuffer);
}