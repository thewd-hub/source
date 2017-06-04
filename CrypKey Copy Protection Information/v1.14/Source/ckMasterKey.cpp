#include "ckInfo.h"
#include "ckEncryption.h"
#include "ckEncryption6.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

// purchased modules
#define MASTERKEY_MODULES_NETWORK		0x8000
#define MASTERKEY_MODULES_NT16			0x4000
#define MASTERKEY_MODULES_NT32			0x2000

// master key features - v6.00+
#define MASTERKEY_FEATURES_USBKEY		0x01
#define MASTERKEY_FEATURES_STATICKEY	0x02
#define MASTERKEY_FEATURES_LICENCEFILE	0x04 // account numbers 448 & 484
#define MASTERKEY_FEATURES_VMGUARD		0x08
#define MASTERKEY_FEATURES_UNKNOWN1		0x10

// master key class
class CCrypKey_MasterKey
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_MasterKey
	{
		unsigned short encryptionSeed;
		unsigned short nonLicencedModules;
		unsigned short accountInformation;
		char productFilename[14];
		unsigned short crcValue;
	} CrypKey_MasterKey;
	#pragma pack()

	CrypKey_MasterKey* mKey;

public:
	CCrypKey_MasterKey(unsigned char* decryptedBuffer) { mKey = (CrypKey_MasterKey*)decryptedBuffer; }

	void formatMasterKey(char** outputBuffer)
	{
		unsigned char* mKeyBuffer = (unsigned char*)mKey;
		wsprintf(*outputBuffer, "                 %02X%02X %02X%02X %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X %02X%02X",
			mKeyBuffer[2], mKeyBuffer[3],
			mKeyBuffer[4], mKeyBuffer[5],
			mKeyBuffer[6], mKeyBuffer[7], mKeyBuffer[8], mKeyBuffer[9], mKeyBuffer[10],
			mKeyBuffer[11], mKeyBuffer[12], mKeyBuffer[13], mKeyBuffer[14], mKeyBuffer[15],
			mKeyBuffer[16], mKeyBuffer[17], mKeyBuffer[18], mKeyBuffer[19],
			mKeyBuffer[20], mKeyBuffer[21]);
	}

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(mKey->crcValue); }
	bool isModuleLicenced(unsigned long moduleId) { return ((mKey->nonLicencedModules & moduleId) ? false : true); }
	unsigned long getAccountNumber(void) { return (mKey->accountInformation & 0x03FF); }
	unsigned long getApplicationId(void) { return (mKey->accountInformation >> 0x0A); }
	char* getCompanyName(void) { return getCompanyNameFromAccountNumber(getAccountNumber(), 36); }
	char* getProductFilename(void) { return ((mKey->productFilename[0] > 0x20) ? (char*)&mKey->productFilename[0] : ""); }

	void setCRCValue(unsigned short crcValue) { mKey->crcValue = ULib.StringLibrary.toBigEndian(crcValue); }
	void setAccountNumber(unsigned short accountNumber) { mKey->accountInformation |= (accountNumber & 0x03FF); }
	void setApplicationId(unsigned char applicationId) { mKey->accountInformation |= (applicationId << 0x0A); }
	void setProductFilename(char* productFilename)
	{
		unsigned long productFilenameLength = ULib.StringLibrary.getStringLength(productFilename);
		if (productFilenameLength > 12) productFilenameLength = 12;
		memcpy((char*)&mKey->productFilename, CharUpper(productFilename), productFilenameLength);
	}
};

// master key class (v6.00)
class CCrypKey_MasterKey6
{
public:
	#pragma pack(1)
	typedef struct _CrypKey_MasterKey6
	{
		unsigned short encryptionSeed;
		unsigned short nonLicencedModules;
		unsigned short accountInformation;
		unsigned long staticKey; // 0x028D [0x0D:0x14]
		unsigned long supportedFeatures; // also contains 0x41 - version, 0x03 - unknown
		unsigned short maxModuleCount;
		unsigned short netSiteKeySeats;
		unsigned short netMaxSeats;
		unsigned short _reserved2[9];
		char productFilename[14];
		unsigned short _reserved3[32];
		unsigned short crcValue;
	} CrypKey_MasterKey6;
	#pragma pack()

	CrypKey_MasterKey6* mKey;

public:
	CCrypKey_MasterKey6(unsigned char* decryptedBuffer) { mKey = (CrypKey_MasterKey6*)decryptedBuffer; }

	void formatMasterKey(char** outputBuffer)
	{
		unsigned char* mKeyBuffer = (unsigned char*)mKey;
		wsprintf(*outputBuffer, "                 %02X%02X %02X%02X %02X%02X%02X%02X %02X%02X:%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X %02X%02X",
			mKeyBuffer[2], mKeyBuffer[3],
			mKeyBuffer[4], mKeyBuffer[5],
			mKeyBuffer[10], mKeyBuffer[11], mKeyBuffer[12], mKeyBuffer[13],
			mKeyBuffer[14], mKeyBuffer[15], mKeyBuffer[18], mKeyBuffer[19],
			mKeyBuffer[38], mKeyBuffer[39], mKeyBuffer[40], mKeyBuffer[41], mKeyBuffer[42],
			mKeyBuffer[43], mKeyBuffer[44], mKeyBuffer[45], mKeyBuffer[46], mKeyBuffer[47],
			mKeyBuffer[48], mKeyBuffer[49], mKeyBuffer[50], mKeyBuffer[51],
			mKeyBuffer[116], mKeyBuffer[117]);
	}

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(mKey->crcValue); }
	bool isModuleLicenced(unsigned long moduleId) { return ((mKey->nonLicencedModules & moduleId) ? false : true); }
	unsigned long getAccountNumber(void) { return (mKey->accountInformation & 0x03FF); }
	unsigned long getApplicationId(void) { return (mKey->accountInformation >> 0x0A); }
	char* getCompanyName(void) { return getCompanyNameFromAccountNumber(getAccountNumber(), 36); }
	char* getProductFilename(void) { return ((mKey->productFilename[0] > 0x20) ? (char*)&mKey->productFilename[0] : ""); }
	unsigned long getMasterKeyVersion(void) { return ((mKey->supportedFeatures >> 0x08) & 0xFF); }
	unsigned long getMasterKeyVersionMajor(void) { return (getMasterKeyVersion() / 10); }
	unsigned long getMasterKeyVersionMinor(void) { return (getMasterKeyVersion() % 10); }
	bool isSKWKey(void) { return (((mKey->accountInformation & 0xFC00) == 0x0C00) ? true : false); }
	bool isMasterKey7(void) { return (((getMasterKeyVersion() >= 0x46) || (isSKWKey())) ? true : false); }
	unsigned long getSupportFeatures(void) { return mKey->supportedFeatures; }
	unsigned long getMaximumModuleCount(void) { return mKey->maxModuleCount; }
	unsigned long getNetSiteKeySeats(void) { return mKey->netSiteKeySeats; }
	unsigned long getNetMaxSeats(void) { return mKey->netMaxSeats; }
	bool isLicenceFileOptionAvailable(void) { return (((mKey->supportedFeatures & 0xFF) & MASTERKEY_FEATURES_LICENCEFILE) == MASTERKEY_FEATURES_LICENCEFILE); }
	bool isVMGuardOptionAvailable(void) { return (((mKey->supportedFeatures & 0xFF) & MASTERKEY_FEATURES_VMGUARD) == MASTERKEY_FEATURES_VMGUARD); }

	// static key functions
	bool isUSBKeyAvailable(void) { return (((mKey->supportedFeatures & 0xFF) & MASTERKEY_FEATURES_USBKEY) == MASTERKEY_FEATURES_USBKEY); }
	bool isStaticKeyAvailable(void) { return ((mKey->staticKey > 0) || (((mKey->supportedFeatures & 0xFF) & MASTERKEY_FEATURES_STATICKEY) == MASTERKEY_FEATURES_STATICKEY) || (isSKWKey())); }
	unsigned long getStaticKeyEncryptionKeyId1(void) { return ((mKey->staticKey & 0xFFFF) & 0x1F); }
	unsigned long getStaticKeyEncryptionKeyId2(void) { return (((mKey->staticKey & 0xFFFF) >> 5) & 0x1F); }
};

///////////////////////////////////////////////////////
// display information retrieved from the master key //
///////////////////////////////////////////////////////
void showMasterKeyInformation(char* masterKeyString)
{
	char* masterKeyStringCopy = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* masterKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* masterKeyDecrypted = NULL;

	// check master key length
	int masterKeyLength = ULib.StringLibrary.getStringLength(masterKeyString);
	// handle v6 master keys that are only 255 characters long (rather than 256 characters)
	if (masterKeyLength == 255)
	{
		ULib.StringLibrary.copyString(masterKeyStringCopy, "0", 1);
		ULib.StringLibrary.appendString(masterKeyStringCopy, masterKeyString, masterKeyLength);
		masterKeyLength = 256;
	}
	else
	{
		ULib.StringLibrary.copyString(masterKeyStringCopy, masterKeyString, masterKeyLength);
	}
	if ((masterKeyLength != 256) && (masterKeyLength != 44)) displayErrorMessage(ERROR_MASTERKEY_LENGTH);

	// convert master key string into bytes
	for (int i = 0; i < masterKeyLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(masterKey, masterKeyStringCopy[i], masterKeyStringCopy[i + 1], (i / 2));

	// v6.00
	bool isMasterKey6 = (masterKeyLength == 256);
	if (isMasterKey6)
	{
		// decrypt master key
		int masterKeyOffset = 10;
		masterKeyLength = 128;
		masterKeyDecrypted = decryptKey6(masterKey, masterKeyLength, KEYTYPE_MASTERKEY6, 0, 0);
		masterKeyLength -= masterKeyOffset;

		ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(masterKey, 12, 2, false);
		ULib.ConsoleLibrary.displayConsoleMessage(" <..> ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&masterKey[118], 10, 2, true);

		// initialise master key class
		CCrypKey_MasterKey6 mKey((unsigned char*)&masterKeyDecrypted[10]);

		ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&masterKeyDecrypted[masterKeyOffset], 24, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("                 ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&masterKeyDecrypted[24 + masterKeyOffset], 24, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("                 ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&masterKeyDecrypted[48 + masterKeyOffset], 24, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("                 ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&masterKeyDecrypted[72 + masterKeyOffset], 24, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("                 ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&masterKeyDecrypted[96 + masterKeyOffset], 22, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect((unsigned char*)&masterKeyDecrypted[masterKeyOffset], masterKeyLength, mKey.getCRCValue(), KEYTYPE_MASTERKEY6)) ? "OK" : "FAILED");
		ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

		// format master key
		char* mKeyFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
		mKey.formatMasterKey(&mKeyFormatted);

		// generate output and display
		char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine6 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine7 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine8 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine9 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine10 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine11 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine12 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine13 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* companyName = mKey.getCompanyName();
		char* masterKeyVersion = (char*)ULib.HeapLibrary.allocPrivateHeap();
		bool isCompanyNameAvailable = (ULib.StringLibrary.getStringLength(companyName) > 0);
		char* staticKeyOutput = NULL;
		if ((mKey.isStaticKeyAvailable()) && (mKey.getStaticKeyEncryptionKeyId1() > 0) && (mKey.getStaticKeyEncryptionKeyId2() > 0))
		{
			staticKeyOutput = (char*)ULib.HeapLibrary.allocPrivateHeap();
			wsprintf(staticKeyOutput, " [0x%02X:0x%02X]    ", mKey.getStaticKeyEncryptionKeyId1(), mKey.getStaticKeyEncryptionKeyId2());
		}
		else
		{
			staticKeyOutput = ULib.StringLibrary.getString(' ', 16);
		}

		if ((mKey.isMasterKey7()) && (mKey.getMasterKeyVersion() > 0))
		{
			wsprintf(masterKeyVersion, " (v%d.%d)", mKey.getMasterKeyVersionMajor(), mKey.getMasterKeyVersionMinor());
		}

		char* productFilenamePadding = ULib.StringLibrary.getString('อ', 12 - ULib.StringLibrary.getStringLength(mKey.getProductFilename()));
		char* maxModuleCountPadding = createPaddedStringFromValue(mKey.getMaximumModuleCount(), 5, ' ', false);
		wsprintf(outputLine1, "                 ศออน ฬออผ ฬออออออผ ศอออออออน ฬออออออออออออออออออออออออออผ ศออน");
		wsprintf(outputLine2, "  ษอ Allow Modules อผ บ    บ ษออออออออออออออผ ศอออ%s Filename - %s    บ", productFilenamePadding, mKey.getProductFilename());
		wsprintf(outputLine3, "  ฬอ WinNT16? - %s   บ    บ ฬออ Maximum Modules - %d %s     CRC - 0x%04X อออผ", ((mKey.isModuleLicenced(MASTERKEY_MODULES_NT16)) ? "Yes" : "No "), mKey.getMaximumModuleCount(), maxModuleCountPadding, mKey.getCRCValue());
		wsprintf(outputLine4, "  ฬอ WinNT32? - %s   บ    บ ศออ Network Seats (Site Key: %d / Max: %d)", ((mKey.isModuleLicenced(MASTERKEY_MODULES_NT32)) ? "Yes" : "No "), mKey.getNetSiteKeySeats(), mKey.getNetMaxSeats());
		wsprintf(outputLine5, "  ศอ Network? - %s   บ    ฬออออ Static Key? - %s%s", ((mKey.isModuleLicenced(MASTERKEY_MODULES_NETWORK)) ? "Yes" : "No "), ((mKey.isStaticKeyAvailable()) ? "Yes" : "No "), staticKeyOutput);
		wsprintf(outputLine6, "                      บ    ฬออออ USBKey? - %s", ((mKey.isUSBKeyAvailable()) ? "Yes" : "No "));
		wsprintf(outputLine7, "                      บ    ฬออออ VM Protection? - %s", ((mKey.isVMGuardOptionAvailable()) ? "Yes" : "No "));
		wsprintf(outputLine8, "                      บ    ฬออออ Licence File (CLF)? - %s", ((mKey.isLicenceFileOptionAvailable()) ? "Yes" : "No "));
		wsprintf(outputLine9, "                      บ    ศออออ v7+ Key? - %s%s", ((mKey.isMasterKey7()) ? "Yes" : "No "), masterKeyVersion);
		wsprintf(outputLine10, "                      ฬอออ Account Number - %d", mKey.getAccountNumber());
		wsprintf(outputLine11, "                      ฬอออ Application Id - %d", mKey.getApplicationId());
		wsprintf(outputLine12, "                      %cอออ Company Number - 7956%d", ((isCompanyNameAvailable) ? 'ฬ' : 'ศ'), mKey.getAccountNumber());
		wsprintf(outputLine13, "                      ศอออ Company Name - %s", companyName);
		ULib.HeapLibrary.freePrivateHeap(productFilenamePadding);
		ULib.HeapLibrary.freePrivateHeap(maxModuleCountPadding);

		ULib.ConsoleLibrary.displayConsoleMessage(mKeyFormatted);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine7);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine8);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine9);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine10);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine11);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine12);
		if (isCompanyNameAvailable) ULib.ConsoleLibrary.displayConsoleMessage(outputLine13);

		ULib.HeapLibrary.freePrivateHeap(mKeyFormatted);
		ULib.HeapLibrary.freePrivateHeap(staticKeyOutput);
		ULib.HeapLibrary.freePrivateHeap(masterKeyVersion);
		ULib.HeapLibrary.freePrivateHeap(companyName);
		ULib.HeapLibrary.freePrivateHeap(outputLine1);
		ULib.HeapLibrary.freePrivateHeap(outputLine2);
		ULib.HeapLibrary.freePrivateHeap(outputLine3);
		ULib.HeapLibrary.freePrivateHeap(outputLine4);
		ULib.HeapLibrary.freePrivateHeap(outputLine5);
		ULib.HeapLibrary.freePrivateHeap(outputLine6);
		ULib.HeapLibrary.freePrivateHeap(outputLine7);
		ULib.HeapLibrary.freePrivateHeap(outputLine8);
		ULib.HeapLibrary.freePrivateHeap(outputLine9);
		ULib.HeapLibrary.freePrivateHeap(outputLine10);
		ULib.HeapLibrary.freePrivateHeap(outputLine11);
		ULib.HeapLibrary.freePrivateHeap(outputLine12);
		ULib.HeapLibrary.freePrivateHeap(outputLine13);
	}
	else
	{
		// decrypt master key
		masterKeyLength = 22;
		masterKeyDecrypted = decryptKey(masterKey, masterKeyLength, KEYTYPE_MASTERKEY);

		ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(masterKey, masterKeyLength, 2, true);

		// initialise master key class
		CCrypKey_MasterKey mKey(masterKeyDecrypted);

		ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
		ULib.ConsoleLibrary.displayHexadecimalString(masterKeyDecrypted, masterKeyLength, 2, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
		ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(masterKeyDecrypted, masterKeyLength, mKey.getCRCValue(), KEYTYPE_MASTERKEY)) ? "OK" : "FAILED");
		ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

		// format master key
		char* mKeyFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
		mKey.formatMasterKey(&mKeyFormatted);

		// generate output and display
		char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* outputLine6 = (char*)ULib.HeapLibrary.allocPrivateHeap();
		char* companyName = mKey.getCompanyName();
		bool isCompanyNameAvailable = (ULib.StringLibrary.getStringLength(companyName) > 0);

		char* productFilenamePadding = ULib.StringLibrary.getString('อ', 12 - ULib.StringLibrary.getStringLength(mKey.getProductFilename()));
		wsprintf(outputLine1, "                 ศออน ฬออผ ฬออออออออออออออออออออออออออผ ฬออผ");
		wsprintf(outputLine2, "  ษอ Allow Modules อผ บ    ศอออ%s Filename - %s ศออออ Key CRC - 0x%04X", productFilenamePadding, mKey.getProductFilename(), mKey.getCRCValue());
		wsprintf(outputLine3, "  ฬอ WinNT16? - %s   ฬอออ Account Number - %d", ((mKey.isModuleLicenced(MASTERKEY_MODULES_NT16)) ? "Yes" : "No "), mKey.getAccountNumber());
		wsprintf(outputLine4, "  ฬอ WinNT32? - %s   ฬอออ Application Id - %d", ((mKey.isModuleLicenced(MASTERKEY_MODULES_NT32)) ? "Yes" : "No "), mKey.getApplicationId());
		wsprintf(outputLine5, "  ศอ Network? - %s   %cอออ Company Number - 7956%d", ((mKey.isModuleLicenced(MASTERKEY_MODULES_NETWORK)) ? "Yes" : "No "), ((isCompanyNameAvailable) ? 'ฬ' : 'ศ'), mKey.getAccountNumber());
		wsprintf(outputLine6, "                      ศอออ Company Name - %s", companyName);
		ULib.HeapLibrary.freePrivateHeap(productFilenamePadding);

		ULib.ConsoleLibrary.displayConsoleMessage(mKeyFormatted);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
		if (isCompanyNameAvailable) ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);

		ULib.HeapLibrary.freePrivateHeap(mKeyFormatted);
		ULib.HeapLibrary.freePrivateHeap(companyName);
		ULib.HeapLibrary.freePrivateHeap(outputLine1);
		ULib.HeapLibrary.freePrivateHeap(outputLine2);
		ULib.HeapLibrary.freePrivateHeap(outputLine3);
		ULib.HeapLibrary.freePrivateHeap(outputLine4);
		ULib.HeapLibrary.freePrivateHeap(outputLine5);
		ULib.HeapLibrary.freePrivateHeap(outputLine6);
	}

	ULib.HeapLibrary.freePrivateHeap(masterKeyStringCopy);
	ULib.HeapLibrary.freePrivateHeap(masterKey);
	ULib.HeapLibrary.freePrivateHeap(masterKeyDecrypted);
}

///////////////////////////////////////////////////////////
// create a new master key from the information supplied //
///////////////////////////////////////////////////////////
void createMasterKey(void)
{
	unsigned char* masterKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* masterKeyEncrypted = NULL;
	unsigned char* masterKeyDecrypted = NULL;
	int masterKeyLength = 22;

	// initialise master key class
	CCrypKey_MasterKey mKey(masterKey);

	// get required information
	ULib.ConsoleLibrary.displayConsoleMessage("Key Information...");

	// account number
	ULib.ConsoleLibrary.displayConsoleMessage("+ Account Number   : ", false);
	char* parsedAccountNumber = getParsedParameterForFormattedString("AccountNumber");
	ULib.ConsoleLibrary.displayConsoleMessage("                                              [0 to 1023]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 57, false);
	unsigned char* masterKey_AccountNumber = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("1111", "    ", true, false, NULL, true, "0-1023", parsedAccountNumber);
	ULib.ConsoleLibrary.displayConsoleMessage(' ', 57 - ULib.StringLibrary.getStringLength((char*)masterKey_AccountNumber), true);
	mKey.setAccountNumber(ULib.StringLibrary.getWORDFromString((char*)masterKey_AccountNumber));
	ULib.HeapLibrary.freePrivateHeap(masterKey_AccountNumber);

	// application id
	ULib.ConsoleLibrary.displayConsoleMessage("+ Application Id   : ", false);
	char* parsedApplicationId = getParsedParameterForFormattedString("ApplicationId");
	ULib.ConsoleLibrary.displayConsoleMessage("                                                [0 to 63]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 57, false);
	unsigned char* masterKey_ApplicationId = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11", "  ", true, false, NULL, true, "0-63", parsedApplicationId);
	ULib.ConsoleLibrary.displayConsoleMessage(' ', 57 - ULib.StringLibrary.getStringLength((char*)masterKey_ApplicationId), true);
	mKey.setApplicationId(ULib.StringLibrary.getBYTEFromString((char*)masterKey_ApplicationId));
	ULib.HeapLibrary.freePrivateHeap(masterKey_ApplicationId);

	// product filename
	ULib.ConsoleLibrary.displayConsoleMessage("+ Product Filename : ", false);
	char* parsedFilename = getParsedParameter("Filename");
	unsigned char* masterKey_ProductFilename = NULL;
	if (parsedFilename != NULL)
	{
		// check for a valid filename length
		if (ULib.StringLibrary.getStringLength(parsedFilename) > 12) parsedFilename[12] = NULL;
		ULib.ConsoleLibrary.displayConsoleMessage(parsedFilename);
		masterKey_ProductFilename = (unsigned char*)parsedFilename;
	}
	else
	{
		masterKey_ProductFilename = ULib.ConsoleLibrary.getConsoleInput(12);
	}

	mKey.setProductFilename((char*)masterKey_ProductFilename);
	ULib.HeapLibrary.freePrivateHeap(masterKey_ProductFilename);

	// add crc value
	mKey.setCRCValue(getKeyCRC(masterKey, masterKeyLength, KEYTYPE_MASTERKEY));

	// encrypt master key (and check decryption succeeded)
	masterKeyEncrypted = encryptKey(masterKey, masterKeyLength, KEYTYPE_MASTERKEY);
	masterKeyDecrypted = decryptKey(masterKeyEncrypted, masterKeyLength, KEYTYPE_MASTERKEY);

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(masterKeyDecrypted, masterKeyLength, mKey.getCRCValue(), KEYTYPE_MASTERKEY)) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Creating Key   - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(masterKey, masterKeyLength, 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Encrypting Key - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(masterKeyEncrypted, masterKeyLength, 2, true);

	ULib.HeapLibrary.freePrivateHeap(masterKeyDecrypted);
	ULib.HeapLibrary.freePrivateHeap(masterKeyEncrypted);
	ULib.HeapLibrary.freePrivateHeap(masterKey);
}