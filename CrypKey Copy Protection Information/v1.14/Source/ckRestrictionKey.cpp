#include "ckInfo.h"
#include "ckEncryption.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include <time.h>

extern CUtilitiesLibrary ULib;

// restriction key class
class CCrypKey_RestrictionKey
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_RestrictionKey
	{
		unsigned char encryptionSeed;
		unsigned short keyTimestampLow;
		unsigned short restrictionsUsed;
		unsigned char licenceCount;
		bool isNetworkLicence;
		unsigned long siteKeyTimestamp;
		unsigned short siteKeyLocationId;
		unsigned short crcValue;
	} CrypKey_RestrictionKey;
	#pragma pack()

	CrypKey_RestrictionKey* rKey;

public:
	CCrypKey_RestrictionKey(unsigned char* decryptedBuffer) { rKey = (CrypKey_RestrictionKey*)decryptedBuffer; }

	void formatRestrictionKey(char** outputBuffer)
	{
		unsigned char* rKeyBuffer = (unsigned char*)rKey;
		wsprintf(*outputBuffer, "                       %02X%02X %02X%02X %02X%02X %02X%02X%02X%02X %02X%02X %02X%02X",
			rKeyBuffer[1], rKeyBuffer[2],
			rKeyBuffer[3], rKeyBuffer[4],
			rKeyBuffer[5], rKeyBuffer[6],
			rKeyBuffer[7], rKeyBuffer[8], rKeyBuffer[9], rKeyBuffer[10],
			rKeyBuffer[11], rKeyBuffer[12],
			rKeyBuffer[13], rKeyBuffer[14]);
	}

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(rKey->crcValue); }
	unsigned long getTimestampLow(void) { return rKey->keyTimestampLow; }
	unsigned long getRestrictionsUsed(void) { return rKey->restrictionsUsed; }
	bool isNetworkLicence(void) { return rKey->isNetworkLicence; }
	unsigned long getLicenceCount(void) { return ((isNetworkLicence()) ? ((rKey->licenceCount * -1) & 0xFF) : rKey->licenceCount); }
	long getSiteKeyTimestamp(void) { return (rKey->siteKeyTimestamp - 0x83AB750E); }
	unsigned long getSiteKeyLocationId(void) { return rKey->siteKeyLocationId; }
};

// restriction key class (v6.00)
class CCrypKey_RestrictionKey6
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_RestrictionKey6
	{
		unsigned char encryptionSeed1;
		unsigned char encryptionSeed2;
		unsigned char encryptionSeed3;
		unsigned short siteKeyLocationId;
		unsigned short restrictionsUsed;
		unsigned char licenceCount;
		bool isNetworkLicence;
		unsigned short _reserved1;
		unsigned long siteKeyTimestamp;
		unsigned short restrictionKeyLocationId;
		unsigned short _reserved2[4];
		unsigned char siteKeyEncryptionKeyId1;
		unsigned char siteKeyEncryptionKeyId2;
		unsigned short _reserved3[8];
		unsigned short crcValue;
	} CrypKey_RestrictionKey6;
	#pragma pack()

	CrypKey_RestrictionKey6* rKey;

public:
	CCrypKey_RestrictionKey6(unsigned char* decryptedBuffer) { rKey = (CrypKey_RestrictionKey6*)decryptedBuffer; }

	void formatRestrictionKey(char** outputBuffer1, char** outputBuffer2)
	{
		unsigned char* rKeyBuffer = (unsigned char*)rKey;
		wsprintf(*outputBuffer1, "                    %02X%02X %02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X %02X%02X%02X%02X%02X%02X%02X%02X",
			rKeyBuffer[3], rKeyBuffer[4],
			rKeyBuffer[5], rKeyBuffer[6],
			rKeyBuffer[7], rKeyBuffer[8],
			rKeyBuffer[9], rKeyBuffer[10],
			rKeyBuffer[11], rKeyBuffer[12], rKeyBuffer[13], rKeyBuffer[14],
			rKeyBuffer[15], rKeyBuffer[16], rKeyBuffer[17], rKeyBuffer[18], rKeyBuffer[19],
			rKeyBuffer[20], rKeyBuffer[21], rKeyBuffer[22], rKeyBuffer[23], rKeyBuffer[24]);
		wsprintf(*outputBuffer2, "                    %02X%02X %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X %02X%02X",
			rKeyBuffer[25],
			rKeyBuffer[26],
			rKeyBuffer[27], rKeyBuffer[28], rKeyBuffer[29], rKeyBuffer[30],
			rKeyBuffer[31],	rKeyBuffer[32], rKeyBuffer[33], rKeyBuffer[34],
			rKeyBuffer[35], rKeyBuffer[36],	rKeyBuffer[37], rKeyBuffer[38],
			rKeyBuffer[39], rKeyBuffer[40], rKeyBuffer[41], rKeyBuffer[42],
			rKeyBuffer[43], rKeyBuffer[44]);
	}

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(rKey->crcValue); }
	unsigned long getSiteKeyLocationId(void) { return rKey->siteKeyLocationId; }
	unsigned long getRestrictionKeyLocationId(void) { return rKey->restrictionKeyLocationId; }
	long getSiteKeyTimestamp(void) { return (rKey->siteKeyTimestamp - 0x83AB750E); }
	unsigned long getRestrictionsUsed(void) { return rKey->restrictionsUsed; }
	bool isNetworkLicence(void) { return rKey->isNetworkLicence; }
	unsigned long getLicenceCount(void) { return ((isNetworkLicence()) ? ((rKey->licenceCount * -1) & 0xFF) : rKey->licenceCount); }
	unsigned long getSiteKeyEncryptionKeyId1(void) { return (rKey->siteKeyEncryptionKeyId1); }
	unsigned long getSiteKeyEncryptionKeyId2(void) { return (rKey->siteKeyEncryptionKeyId2); }
};

////////////////////////////////////////////////////////////
// display information retrieved from the restriction key //
////////////////////////////////////////////////////////////
void showRestrictionKeyInformation(char* restrictionKeyString)
{
	unsigned char* restrictionKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* restrictionKeyDecrypted = NULL;

	// check restriction key length
	int restrictionKeyLength = ULib.StringLibrary.getStringLength(restrictionKeyString);
	bool isRestrictionKey6 = (restrictionKeyLength == 90);
	if ((isRestrictionKey6) && (restrictionKeyLength != 90)) displayErrorMessage(ERROR_RESTRICTIONKEY_LENGTH);
	else if ((!isRestrictionKey6) && (restrictionKeyLength != 30)) displayErrorMessage(ERROR_RESTRICTIONKEY_LENGTH);

	// convert restriction key string into bytes
	for (int i = 0; i < restrictionKeyLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(restrictionKey, restrictionKeyString[i], restrictionKeyString[i + 1], (i / 2));

	// decrypt restriction key
	restrictionKeyLength = ((isRestrictionKey6) ? 45 : 15);
	restrictionKeyDecrypted = decryptKey(restrictionKey, restrictionKeyLength, ((isRestrictionKey6) ? KEYTYPE_RESTRICTIONKEY6 : KEYTYPE_RESTRICTIONKEY));

	// v6.00
	if (isRestrictionKey6)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(restrictionKey, 25, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("                 ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&restrictionKey[25], 20, 2, true);

		// initialise restriction key class
		CCrypKey_RestrictionKey6 rKey(restrictionKeyDecrypted);

		ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(restrictionKeyDecrypted, 25, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("                 ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&restrictionKeyDecrypted[25], 20, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(restrictionKeyDecrypted, restrictionKeyLength, rKey.getCRCValue(), KEYTYPE_RESTRICTIONKEY6)) ? "OK" : "FAILED");
		ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

		// format restriction key
		char* rKeyFormatted_L1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* rKeyFormatted_L2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		rKey.formatRestrictionKey(&rKeyFormatted_L1, &rKeyFormatted_L2);

		// get site key timestamp (date isn't returned correctly either from the crypkey libraries)
		struct tm tmTime;
		time_t tTime = rKey.getSiteKeyTimestamp();
		gmtime_s(&tmTime, &tTime);
		char* dateString = (char*)ULib.HeapLibrary.allocPrivateHeap();
		strftime(dateString, 200, "%d %b %Y / %H:%M", &tmTime);

		// generate output and display
		char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine6 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine7 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine8 = (char*)ULib.HeapLibrary.allocPrivateHeap();

		char* restrictionsUsedPaddedString = createPaddedStringFromValue(rKey.getRestrictionsUsed(), 5, 'อ', false);
		char* licenceCountPaddedString = createPaddedStringFromValue(rKey.getLicenceCount(), 4, 'อ', false);
		wsprintf(outputLine1, "                    ฬออผ ศออน ฬออออออผ ศออออออน ฬออผ ศออออออออออออออผ");
		wsprintf(outputLine2, "                    ศอออออป บ บ ษอออออออออออออผ ศออออ Key Location Id - 0x%04X", rKey.getRestrictionKeyLocationId());
		wsprintf(outputLine3, "  Location Id - 0x%04X อออผ บ บ ศออออ Site Key Timestamp - %s", rKey.getSiteKeyLocationId(), dateString);
		wsprintf(outputLine4, "  Restrictions Used - %d %sผ บ", rKey.getRestrictionsUsed(), restrictionsUsedPaddedString);
		wsprintf(outputLine5, "  Number Of Licences - %d %sออน", rKey.getLicenceCount(), licenceCountPaddedString);
		wsprintf(outputLine6, "  Network Licence? - %sอออออผ ษอออออ Site Key Encryption Key Ids - 0x%02X:0x%02X", ((rKey.isNetworkLicence()) ? "Yes " : "No อ"), rKey.getSiteKeyEncryptionKeyId1(), rKey.getSiteKeyEncryptionKeyId2());
		wsprintf(outputLine7, "                    ษอออออออออออผ                         ษออ Key CRC - 0x%04X", rKey.getCRCValue());
		wsprintf(outputLine8, "                    ฬออป ษออออออออออออออออออออออออออออออป ฬออป");
		ULib.HeapLibrary.freePrivateHeap(restrictionsUsedPaddedString);
		ULib.HeapLibrary.freePrivateHeap(licenceCountPaddedString);

		ULib.ConsoleLibrary.displayConsoleMessage(rKeyFormatted_L1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine7);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine8);
		ULib.ConsoleLibrary.displayConsoleMessage(rKeyFormatted_L2);

		ULib.HeapLibrary.freePrivateHeap(rKeyFormatted_L1);
		ULib.HeapLibrary.freePrivateHeap(rKeyFormatted_L2);
		ULib.HeapLibrary.freePrivateHeap(dateString);
		ULib.HeapLibrary.freePrivateHeap(outputLine1);
		ULib.HeapLibrary.freePrivateHeap(outputLine2);
		ULib.HeapLibrary.freePrivateHeap(outputLine3);
		ULib.HeapLibrary.freePrivateHeap(outputLine4);
		ULib.HeapLibrary.freePrivateHeap(outputLine5);
		ULib.HeapLibrary.freePrivateHeap(outputLine6);
		ULib.HeapLibrary.freePrivateHeap(outputLine7);
		ULib.HeapLibrary.freePrivateHeap(outputLine8);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(restrictionKey, restrictionKeyLength, 2, true);

		// initialise restriction key class
		CCrypKey_RestrictionKey rKey(restrictionKeyDecrypted);

		ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(restrictionKeyDecrypted, restrictionKeyLength, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(restrictionKeyDecrypted, restrictionKeyLength, rKey.getCRCValue(), KEYTYPE_RESTRICTIONKEY)) ? "OK" : "FAILED");
		ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

		// format restriction key
		char* rKeyFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
		rKey.formatRestrictionKey(&rKeyFormatted);

		// get site key timestamp
		struct tm tmTime;
		time_t tTime = rKey.getSiteKeyTimestamp();
		gmtime_s(&tmTime, &tTime);
		char* dateString = (char*)ULib.HeapLibrary.allocPrivateHeap();
		strftime(dateString, 200, "%d %b %Y / %H:%M", &tmTime);

		// generate output and display
		char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();

		char* restrictionsUsedPaddedString = createPaddedStringFromValue(rKey.getRestrictionsUsed(), 8, 'อ', false);
		char* licenceCountPaddedString = createPaddedStringFromValue(rKey.getLicenceCount(), 4, 'อ', false);
		wsprintf(outputLine1, "                       ศออน ศออน ศออน ฬออออออผ ฬออผ ฬออผ");
		wsprintf(outputLine2, "  Location Id - 0x%04X อออผ    บ    บ บ        บ    ศออออออออ Key CRC - 0x%04X", rKey.getTimestampLow(), rKey.getCRCValue());
		wsprintf(outputLine3, "  Restrictions Used - %d %sผ    บ บ        ศอออออ Key Location Id - 0x%04X", rKey.getRestrictionsUsed(), restrictionsUsedPaddedString, rKey.getSiteKeyLocationId());
		wsprintf(outputLine4, "  Number Of Licences - %d %sออออออออน ศอออ Key Timestamp - %s", rKey.getLicenceCount(), licenceCountPaddedString, dateString);
		wsprintf(outputLine5, "  Network Licence? - %sอออออออออออผ", ((rKey.isNetworkLicence()) ? "Yes " : "No อ"));
		ULib.HeapLibrary.freePrivateHeap(restrictionsUsedPaddedString);
		ULib.HeapLibrary.freePrivateHeap(licenceCountPaddedString);

		ULib.ConsoleLibrary.displayConsoleMessage(rKeyFormatted);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);

		ULib.HeapLibrary.freePrivateHeap(rKeyFormatted);
		ULib.HeapLibrary.freePrivateHeap(dateString);
		ULib.HeapLibrary.freePrivateHeap(outputLine1);
		ULib.HeapLibrary.freePrivateHeap(outputLine2);
		ULib.HeapLibrary.freePrivateHeap(outputLine3);
		ULib.HeapLibrary.freePrivateHeap(outputLine4);
		ULib.HeapLibrary.freePrivateHeap(outputLine5);
	}

	ULib.HeapLibrary.freePrivateHeap(restrictionKey);
	ULib.HeapLibrary.freePrivateHeap(restrictionKeyDecrypted);
}