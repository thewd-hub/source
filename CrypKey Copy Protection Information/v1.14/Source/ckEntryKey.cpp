#include "ckInfo.h"
#include "ckEncryption.h"
#include "ckEncryption6.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

// entry key class
class CCrypKey_EntryKey
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_EntryKey
	{
		unsigned char encryptionSeed;
		union _U1
		{
			unsigned short siteCodeId;
			unsigned short siteCodeEncryptionKeyIds;
		} U1;
		unsigned short keyLocationId;
		unsigned short crcValue;
	} CrypKey_EntryKey;
	#pragma pack()

	CrypKey_EntryKey* eKey;

public:
	CCrypKey_EntryKey(unsigned char* decryptedBuffer) { eKey = (CrypKey_EntryKey*)decryptedBuffer; }

	void formatEntryKey(char** outputBuffer)
	{
		unsigned char* eKeyBuffer = (unsigned char*)eKey;
		wsprintf(*outputBuffer, "                              %02X%02X %02X%02X %02X%02X",
			eKeyBuffer[1], eKeyBuffer[2],
			eKeyBuffer[3], eKeyBuffer[4],
			eKeyBuffer[5], eKeyBuffer[6]);
	}

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(eKey->crcValue); }
	unsigned long getSiteCodeId(void) { return (eKey->U1.siteCodeId & 0x7F); }
	unsigned long getSiteCodeEncryptionKeyId1(void) { return (eKey->U1.siteCodeEncryptionKeyIds & 0x1F); }
	unsigned long getSiteCodeEncryptionKeyId2(void) { return ((eKey->U1.siteCodeEncryptionKeyIds >> 0x05) & 0x1F); }
	unsigned long getKeyLocationId(void) { return eKey->keyLocationId; }
};

//////////////////////////////////////////////////////
// display information retrieved from the entry key //
//////////////////////////////////////////////////////
void showEntryKeyInformation(char* entryKeyString)
{
	unsigned char* entryKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* entryKeyDecrypted = NULL;
	int entryKeyLength = 0;
	bool isEntryKey7 = false;

	// get entry key length
	entryKeyLength = ULib.StringLibrary.getStringLength(entryKeyString);

	// check for file being specified
	if ((ULib.FileLibrary.doesFileExist(entryKeyString, false)) || (entryKeyLength != 14))
	{
		isEntryKey7 = true;
		entryKeyLength = 7;

		char* outputBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(outputBuffer, "Read Key File  - %s", ULib.FileLibrary.stripPathFromFilename(entryKeyString, true));
		ULib.ConsoleLibrary.displayConsoleMessage(outputBuffer);
		ULib.HeapLibrary.freePrivateHeap(outputBuffer);

		unsigned long keyBufferLength = 0;
		unsigned char* keyBuffer = ULib.FileLibrary.readBufferFromFile(entryKeyString, &keyBufferLength);
		if ((keyBuffer == NULL) || (keyBufferLength <= 0)) displayErrorMessage(ERROR_GENERALFILE_FILENAME);

		unsigned char* encryptedBytes = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(keyBufferLength);
		unsigned long encryptedBytesCount = keyBufferLength / 2;
		for (unsigned long i = 0; i < keyBufferLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(encryptedBytes, keyBuffer[i], keyBuffer[i + 1], (i / 2));

		unsigned long tempBufferLength1 = 0;
		unsigned long tempBufferLength2 = 0;
		unsigned char* tempBuffer1 = decryptKey6Stream(encryptedBytes, encryptedBytesCount, &tempBufferLength1, 0x80, KEYTYPE_ENTRYKEY7);
		//ULib.FileLibrary.writeBufferToFile("2.clf", tempBuffer1, tempBufferLength1, 0, false, true);
		unsigned char* tempBuffer2 = decryptKey6Stream(tempBuffer1, tempBufferLength1, &tempBufferLength2, 0x80, KEYTYPE_ENTRYKEY7);
		entryKeyDecrypted = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
		if (tempBufferLength2 == 6) ULib.StringLibrary.copyBuffer((unsigned char*)&entryKeyDecrypted[1], 8, tempBuffer2, 6);
		ULib.HeapLibrary.freePrivateHeap(tempBuffer1);
		ULib.HeapLibrary.freePrivateHeap(tempBuffer2);
	}
	else
	{
		// convert entry key string into bytes
		for (int i = 0; i < entryKeyLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(entryKey, entryKeyString[i], entryKeyString[i + 1], (i / 2));

		// decrypt entry key
		entryKeyLength = 7;
		entryKeyDecrypted = decryptKey(entryKey, entryKeyLength, KEYTYPE_ENTRYKEY);

		ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(entryKey, entryKeyLength, 2, true);
	}

	// initialise entry key class
	CCrypKey_EntryKey eKey(entryKeyDecrypted);

	ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(entryKeyDecrypted, entryKeyLength, 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(entryKeyDecrypted, entryKeyLength, eKey.getCRCValue(), KEYTYPE_ENTRYKEY)) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

	// format entry key
	char* eKeyFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	eKey.formatEntryKey(&eKeyFormatted);

	// generate output and display
	char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();

	wsprintf(outputLine1, "                              ศออน ฬออผ ฬออผ");
	wsprintf(outputLine2, "  Site Code Id - 0x%02X อออออออออออน บ    ศออออออออ Key CRC - 0x%04X", eKey.getSiteCodeId(), eKey.getCRCValue());
	wsprintf(outputLine3, "                      ษออออออออออผ ศอออออ Key Location Id - 0x%04X", eKey.getKeyLocationId());
	wsprintf(outputLine4, "                      ศออ Site Code Encryption Key Ids - 0x%02X:0x%02X", eKey.getSiteCodeEncryptionKeyId1(), eKey.getSiteCodeEncryptionKeyId2());

	ULib.ConsoleLibrary.displayConsoleMessage(eKeyFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);

	ULib.HeapLibrary.freePrivateHeap(eKeyFormatted);
	ULib.HeapLibrary.freePrivateHeap(outputLine1);
	ULib.HeapLibrary.freePrivateHeap(outputLine2);
	ULib.HeapLibrary.freePrivateHeap(outputLine3);
	ULib.HeapLibrary.freePrivateHeap(outputLine4);

	ULib.HeapLibrary.freePrivateHeap(entryKey);
	ULib.HeapLibrary.freePrivateHeap(entryKeyDecrypted);
}