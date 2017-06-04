#include "ckInfo.h"
#include "ckEncryption.h"
#include "ckEncryption6.h"
#include "ckSiteCode.h"
#include "ckUserKey.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

// site key features - v6.00+ (32bit)
#define SITEKEY_FEATURES_ADDTOEXISTINGLICENCE	0x01

// site key class
class CCrypKey_SiteKey
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_SiteKey
	{
		union _U1
		{
			unsigned char encryptionSeed;
			unsigned char supportedFeatures; // v6.00+ (32bit)
		} U1;
		unsigned char siteCodeId;
		unsigned char licenceCount;
		unsigned short userKeyHash;
		union _U2
		{
			struct _keyOptions
			{
				unsigned long keyOptions;
			} keyOptions;
			struct _keyLevel
			{
				unsigned short keyLevel;
				unsigned short keyOptionsHigh;
			} keyLevel;
		} U2;
		unsigned short licenceDuration;
		unsigned short crcValue;
	} CrypKey_SiteKey;
	#pragma pack()

	CCrypKey_SiteCode* sCode;
	CrypKey_SiteKey* sKey;
	bool sKeyIsV6;
	bool sKeyIsV6_Win32;

public:
	CCrypKey_SiteKey(unsigned char* decryptedBuffer, CCrypKey_SiteCode* sCodePtr)
	{
		sKey = (CrypKey_SiteKey*)decryptedBuffer;
		sCode = sCodePtr;
		sKeyIsV6 = false;
		sKeyIsV6_Win32 = false;

		this->StaticKey.setSiteKey(sKey);
	}

	void formatSiteKey(char** outputBuffer)
	{
		unsigned char* sKeyBuffer = (unsigned char*)sKey;
		if (isSiteKeyV6_Static())
		{
			wsprintf(*outputBuffer, "                         %02X %02X%02X%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
				sKeyBuffer[0],
				sKeyBuffer[1], sKeyBuffer[2], sKeyBuffer[3], sKeyBuffer[4],
				sKeyBuffer[5], sKeyBuffer[6],
				sKeyBuffer[7], sKeyBuffer[8], sKeyBuffer[9], sKeyBuffer[10],
				sKeyBuffer[11], sKeyBuffer[12]);
		}
		else if (isSiteKeyV6_Win32())
		{
			wsprintf(*outputBuffer, "                      %02X %02X %02X %02X%02X %02X%02X%02X%02X %02X%02X %02X%02X",
				sKeyBuffer[0],
				sKeyBuffer[1],
				sKeyBuffer[2],
				sKeyBuffer[3], sKeyBuffer[4],
				sKeyBuffer[5], sKeyBuffer[6], sKeyBuffer[7], sKeyBuffer[8],
				sKeyBuffer[9], sKeyBuffer[10],
				sKeyBuffer[11], sKeyBuffer[12]);
		}
		else
		{
			wsprintf(*outputBuffer, "                         %02X %02X %02X%02X %02X%02X%02X%02X %02X%02X %02X%02X",
				sKeyBuffer[1],
				sKeyBuffer[2],
				sKeyBuffer[3], sKeyBuffer[4],
				sKeyBuffer[5], sKeyBuffer[6], sKeyBuffer[7], sKeyBuffer[8],
				sKeyBuffer[9], sKeyBuffer[10],
				sKeyBuffer[11], sKeyBuffer[12]);
		}
	}

	bool isSiteKeyV6(void) { return ((sKeyIsV6) || (sKeyIsV6_Win32)); }
	bool isSiteKeyV6_Win32(void) { return sKeyIsV6_Win32; }
	bool isSiteKeyV6_Static(void) { return ((isSiteKeyV6()) && (this->StaticKey.isStaticKey())); }
	void setIsSiteKeyV6(void) { sKeyIsV6 = true; }
	void setIsSiteKeyV6_Win32(void) { sKeyIsV6_Win32 = true; }

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(sKey->crcValue); }
	unsigned long getSiteCodeId(void) { return ((isSiteKeyV6_Win32()) ? 0 : (sKey->siteCodeId & 0x7F)); }
	bool isAddToExistingLicence(void) { return ((isSiteKeyV6_Win32()) ? ((sKey->U1.supportedFeatures & SITEKEY_FEATURES_ADDTOEXISTINGLICENCE) ? true : false) : (sKey->siteCodeId > 0x7F)); }
	bool isNetworkLicence(void) { return (sKey->licenceCount > 0x7F); }
	bool isEasyLicence(void) { return ((sKey->userKeyHash > 0x7FFF) && (!isNetworkLicence()) && (isLicenceDurationUnlimited()) && ((isAddToExistingLicence()) ? (getLicenceCount() == 0) : (getLicenceCount() == 1))); }
	unsigned long getLicenceCount(void) { return ((isNetworkLicence()) ? ((sKey->licenceCount * -1) & 0xFF) : sKey->licenceCount); }
	unsigned long getUserKeyHash(void) { return ((isEasyLicence()) ? (sKey->userKeyHash & 0x7FFF) : sKey->userKeyHash); }
	unsigned long getKeyOptions(void) { return sKey->U2.keyOptions.keyOptions; }
	unsigned long getKeyLevel(void) { return sKey->U2.keyLevel.keyLevel; }
	unsigned long getLicenceDuration(void) { return ((isLicenceDurationDayLimited()) ? (sKey->licenceDuration & 0x7FFF) : sKey->licenceDuration); }
	bool isLicenceDurationUnlimited(void) { return (sKey->licenceDuration == 0); }
	bool isLicenceDurationDayLimited(void) { return (sKey->licenceDuration > 0x7FFF); }
	bool isLicenceDurationRunLimited(void) { return ((!isLicenceDurationUnlimited()) && (sKey->licenceDuration <= 0x7FFF)); }

	void setCRCValue(unsigned short crcValue) { sKey->crcValue = ULib.StringLibrary.toBigEndian(crcValue); }
	void setSiteCodeId(unsigned char siteCodeId) { sKey->siteCodeId = (unsigned char)(siteCodeId & 0x7F); }
	void setUserKeyHash(unsigned long userKeyHashSeed, unsigned long userKeyHash, unsigned long userKeyHash6, unsigned long driveSerialNumber)
	{
		if (isSiteKeyV6())
		{
			if ((sCode->isEasyLicenceAllowed()) && (!sCode->isHDSNError())) sKey->userKeyHash = (unsigned short)(userKeyHash6 ^ driveSerialNumber);
			else sKey->userKeyHash = (unsigned short)userKeyHash6;
			if (sCode->isEasyLicenceAllowed()) sKey->userKeyHash &= 0x7FFF;
		}
		else
		{
			sKey->userKeyHash = (unsigned short)((unsigned char)((userKeyHashSeed & 0xFF) + userKeyHash) | (unsigned short)((((userKeyHashSeed >> 8) & 0xFF) + userKeyHash) << 8));
		}
	}
	void setLicenceCount(unsigned long licenceCount, bool setAsNetworkLicence) { ((setAsNetworkLicence) ? sKey->licenceCount = (unsigned char)((licenceCount * -1) & 0xFF) : sKey->licenceCount = (unsigned char)licenceCount); }
	void setAddToExistingLicence(void) { ((isSiteKeyV6_Win32()) ? (sKey->U1.supportedFeatures |= SITEKEY_FEATURES_ADDTOEXISTINGLICENCE) : (sKey->siteCodeId |= 0x80)); }
	void setEasyLicence(void) { if (isSiteKeyV6()) { sKey->userKeyHash |= 0x8000; setLicenceCount(((isAddToExistingLicence()) ? 0 : 1), false); } }
	void setKeyOptions(unsigned long keyOptions) { sKey->U2.keyOptions.keyOptions |= keyOptions; }
	void setKeyLevel(unsigned long keyLevel) { sKey->U2.keyLevel.keyLevel |= keyLevel; }
	void setLicenceDurationUnlimited(void) { sKey->licenceDuration = 0; }
	void setLicenceDurationDayLimited(unsigned long dayCount) { sKey->licenceDuration = (unsigned short)((dayCount & 0x7FFF) | 0x8000); }
	void setLicenceDurationRunLimited(unsigned long runCount) { sKey->licenceDuration = (unsigned short)(runCount & 0x7FFF); }

	// site key class (static key)
	class CCrypKey_SiteKey_StaticKey
	{
		private:
			CrypKey_SiteKey* sKey;

		public:
			void setSiteKey(CrypKey_SiteKey* sKey) { this->sKey = sKey; }

			bool isStaticKey(void) { return ((sKey->U1.supportedFeatures == 0) && (sKey->siteCodeId > 0) && (sKey->userKeyHash == 0) && (sKey->U2.keyOptions.keyOptions == 0)); }

			unsigned long getStaticKey(void) { return ((sKey->siteCodeId) | (sKey->licenceCount << 8) | (sKey->userKeyHash << 16)); }
			unsigned long getEncryptionKeyId1(void) { return (getStaticKey() & 0x1F); }
			unsigned long getEncryptionKeyId2(void) { return ((getStaticKey() >> 5) & 0x1F); }
			unsigned long getAccountInformation(void) { return sKey->licenceDuration; }
			unsigned long getAccountNumber(void) { return (getAccountInformation() & 0x03FF); }
            unsigned long getApplicationId(void) { return (getAccountInformation() >> 0x0A); }
            char* getCompanyName(int maximumLength) { return getCompanyNameFromAccountNumber(getAccountNumber(), maximumLength); }

			void setStaticKey(unsigned long staticKey) { sKey->siteCodeId = (unsigned char)(staticKey & 0xFF); sKey->licenceCount = (unsigned char)((staticKey >> 8) & 0xFF); sKey->userKeyHash = (unsigned short)((staticKey >> 16) & 0xFFFF); }
			void setAccountInformation(unsigned long accountInformation) { sKey->licenceDuration = (unsigned short)(accountInformation & 0xFFFF); }

	} StaticKey;
};

///////////////////////////////////////////
// checks whether an option is supported //
///////////////////////////////////////////
bool isKeyOptionSupported(unsigned long optionNumber, unsigned long keyOptions)
{
	switch (optionNumber)
	{
		case 1: return ((keyOptions & 0x80000000) ? true : false);
		case 2: return ((keyOptions & 0x40000000) ? true : false);
		case 3: return ((keyOptions & 0x20000000) ? true : false);
		case 4: return ((keyOptions & 0x10000000) ? true : false);
		case 5: return ((keyOptions & 0x08000000) ? true : false);
		case 6: return ((keyOptions & 0x04000000) ? true : false);
		case 7: return ((keyOptions & 0x02000000) ? true : false);
		case 8: return ((keyOptions & 0x01000000) ? true : false);
		case 9: return ((keyOptions & 0x00800000) ? true : false);
		case 10: return ((keyOptions & 0x00400000) ? true : false);
		case 11: return ((keyOptions & 0x00200000) ? true : false);
		case 12: return ((keyOptions & 0x00100000) ? true : false);
		case 13: return ((keyOptions & 0x00080000) ? true : false);
		case 14: return ((keyOptions & 0x00040000) ? true : false);
		case 15: return ((keyOptions & 0x00020000) ? true : false);
		case 16: return ((keyOptions & 0x00010000) ? true : false);
	}

	return false;
}

//////////////////////////////////////
// enables the specified key option //
//////////////////////////////////////
void enableKeyOption(unsigned long optionNumber, unsigned long* keyOptions)
{
	switch (optionNumber)
	{
		case 1: (*keyOptions |= 0x80000000); break;
		case 2: (*keyOptions |= 0x40000000); break;
		case 3: (*keyOptions |= 0x20000000); break;
		case 4: (*keyOptions |= 0x10000000); break;
		case 5: (*keyOptions |= 0x08000000); break;
		case 6: (*keyOptions |= 0x04000000); break;
		case 7: (*keyOptions |= 0x02000000); break;
		case 8: (*keyOptions |= 0x01000000); break;
		case 9: (*keyOptions |= 0x00800000); break;
		case 10: (*keyOptions |= 0x00400000); break;
		case 11: (*keyOptions |= 0x00200000); break;
		case 12: (*keyOptions |= 0x00100000); break;
		case 13: (*keyOptions |= 0x00080000); break;
		case 14: (*keyOptions |= 0x00040000); break;
		case 15: (*keyOptions |= 0x00020000); break;
		case 16: (*keyOptions |= 0x00010000); break;
	}
}

/////////////////////////////////////////////////////
// display information retrieved from the site key //
/////////////////////////////////////////////////////
void showSiteKeyInformation(char* siteKeyString)
{
	unsigned char* siteKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* siteKeyDecrypted = NULL;

	// check site key length
	int siteKeyLength = ULib.StringLibrary.getStringLength(siteKeyString);
	if (siteKeyLength != 26) displayErrorMessage(ERROR_SITEKEY_LENGTH);

	// convert site key string into bytes
	for (int i = 0; i < siteKeyLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(siteKey, siteKeyString[i], siteKeyString[i + 1], (i / 2));

	// decrypt site key
	siteKeyLength = 13;
	siteKeyDecrypted = decryptKey(siteKey, siteKeyLength, KEYTYPE_SITEKEY);

	ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteKey, siteKeyLength, 2, true);

	// test decryption succeeded (if not try v6.00 decryption)
	CCrypKey_SiteKey sKeyTest(siteKeyDecrypted, NULL);
	bool isSiteKey6 = false;
	unsigned long key1Id = 0;
	unsigned long key2Id = 0;

	if (!isKeyCRCCorrect(siteKeyDecrypted, siteKeyLength, sKeyTest.getCRCValue(), KEYTYPE_SITEKEY))
	{
		isSiteKey6 = true;
		ULib.ConsoleLibrary.displayConsoleMessage("Decrypt Failed - Trying v6.00 Decryption...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 44, false);

		ULib.HeapLibrary.freePrivateHeap(siteKeyDecrypted);
		siteKeyDecrypted = findDecryptionKeyIds(siteKey, siteKeyLength, KEYTYPE_SITEKEY6, &key1Id, &key2Id);
		if (siteKeyDecrypted == NULL) displayErrorMessage(ERROR_SITEKEY6_DECRYPTION_FAILED);
	}

	// initialise site key class
	CCrypKey_SiteKey sKey(siteKeyDecrypted, NULL);
	if (isSiteKey6) sKey.setIsSiteKeyV6_Win32();

	ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteKeyDecrypted, siteKeyLength, 2, false);
	if (!sKey.isSiteKeyV6_Win32()) ULib.ConsoleLibrary.displayConsoleMessage("");
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage(" [0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(key1Id, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		ULib.ConsoleLibrary.displayConsoleMessage(":0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(key2Id, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		ULib.ConsoleLibrary.displayConsoleMessage("]");
	}
	ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(siteKeyDecrypted, siteKeyLength, sKey.getCRCValue(), ((sKey.isSiteKeyV6_Win32()) ? KEYTYPE_SITEKEY6 : KEYTYPE_SITEKEY))) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

	// format site key
	char* sKeyFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	sKey.formatSiteKey(&sKeyFormatted);

	// licence duration
	char* outputLicenceDuration = (char*)ULib.HeapLibrary.allocPrivateHeap();
	if (sKey.isLicenceDurationUnlimited()) ULib.StringLibrary.copyString(outputLicenceDuration, "อออออออออออออ Unlimited Licence");
	else
	{
		char* licenceDurationPaddedString = createPaddedStringFromValue(sKey.getLicenceDuration(), 5, 'อ', false);
		wsprintf(outputLicenceDuration, "%s Limited Licence (%s) - %d", licenceDurationPaddedString, ((sKey.isLicenceDurationDayLimited()) ? "Days" : "Runs"), sKey.getLicenceDuration());
		ULib.HeapLibrary.freePrivateHeap(licenceDurationPaddedString);
	}

	// site key options (max 16 options)
	unsigned long keyOptions = sKey.getKeyOptions();
	char* keyOptionsString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 1; i <= 16;)
	{
		if (!isKeyOptionSupported(i, keyOptions))
		{
			i++;
			continue;
		}

		int startIndex = i;
		int endIndex = i;
		for (int j = (startIndex + 1); j <= 16; j++)
		{
			if (!isKeyOptionSupported(j, keyOptions)) break;
			endIndex = j;
		}

		if (endIndex > (startIndex + 1)) wsprintf(keyOptionsString, "%s%d-%d,", keyOptionsString, startIndex, endIndex);
		else if (endIndex > startIndex) wsprintf(keyOptionsString, "%s%d,%d,", keyOptionsString, startIndex, endIndex);
		else wsprintf(keyOptionsString, "%s%d,", keyOptionsString, startIndex);
		i = (endIndex + 1);
	}
	if (ULib.StringLibrary.getStringLength(keyOptionsString) > 0) keyOptionsString[ULib.StringLibrary.getStringLength(keyOptionsString) - 1] = NULL;
	else ULib.StringLibrary.copyString(keyOptionsString, "0");

	// generate output and display
	char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine6 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine7 = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// using a static key
	if (sKey.isSiteKeyV6_Static())
	{
		char* companyName = sKey.StaticKey.getCompanyName(16);
		bool isCompanyNameAvailable = (ULib.StringLibrary.getStringLength(companyName) > 0);

		ULib.StringLibrary.copyString(outputLine1, "                         ศผ ศออออออน ศออผ ศออผ ศออน ฬออผ");
		wsprintf(outputLine2, "  Static Key - 0x%08X อออออออออน        ษอออออผ ศออออออออ Key CRC - 0x%04X", sKey.StaticKey.getStaticKey(), sKey.getCRCValue());
		wsprintf(outputLine3, "  Encryption Key Ids - 0x%02X:0x%02X ออผ        ฬอ Account Number - %d", sKey.StaticKey.getEncryptionKeyId1(), sKey.StaticKey.getEncryptionKeyId2(), sKey.StaticKey.getAccountNumber());
		wsprintf(outputLine4, "                                            ฬอ Application Id - %d", sKey.StaticKey.getApplicationId());
		wsprintf(outputLine5, "                                            %cอ Company Number - 7956%d", ((isCompanyNameAvailable) ? 'ฬ' : 'ศ'), sKey.StaticKey.getAccountNumber());
		wsprintf(outputLine6, "	                                    ศอ Company Name - %s", companyName);
		ULib.StringLibrary.copyString(outputLine7, "\r\n  Note: Using static keys only authorises a single licence (non-networked)");

		ULib.ConsoleLibrary.displayConsoleMessage(sKeyFormatted);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
		if (isCompanyNameAvailable) ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine7);
		ULib.HeapLibrary.freePrivateHeap(companyName);
	}
	// normal site key
	else
	{
		char* licenceCountPaddedString = createPaddedStringFromValue(sKey.getLicenceCount(), 4, 'อ', false);
		wsprintf(outputLine1, "                      %s ศน ฬออผ ฬออออออผ ฬออผ ฬออผ", ((sKey.isSiteKeyV6_Win32()) ? "ฬผ ศผ" : "   ศน"));
		if (sKey.isSiteKeyV6_Win32()) wsprintf(outputLine2, "                      ศอออป  บ บ    บ        บ    ศออออออออออ Key CRC - 0x%04X", sKey.getCRCValue());
		else wsprintf(outputLine2, "  Site Code Id - 0x%02X ออออน  บ บ    บ        บ    ศออออออออออ Key CRC - 0x%04X", sKey.getSiteCodeId(), sKey.getCRCValue());
		wsprintf(outputLine3, "  Add To Licence? - %sออผ  บ บ    บ        ศอ%s", ((sKey.isAddToExistingLicence()) ? "Yes " : "No อ"), outputLicenceDuration);
		wsprintf(outputLine4, "  Number Of Licences - %d %sอน บ    ฬออ Key Level - %d", sKey.getLicenceCount(), licenceCountPaddedString, sKey.getKeyLevel());
		wsprintf(outputLine5, "  Network Licence? - %sออออผ บ    ศออ Key Options - %s", ((sKey.isNetworkLicence()) ? "Yes " : "No อ"), keyOptionsString);
		wsprintf(outputLine6, "  Easy Licence? - %sอออออออออสอออออออ User Key Hash - 0x%04X", ((sKey.isEasyLicence()) ? "Yes " : "No อ"), sKey.getUserKeyHash());
		ULib.HeapLibrary.freePrivateHeap(licenceCountPaddedString);

		ULib.ConsoleLibrary.displayConsoleMessage(sKeyFormatted);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
	}

	ULib.HeapLibrary.freePrivateHeap(sKeyFormatted);
	ULib.HeapLibrary.freePrivateHeap(outputLicenceDuration);
	ULib.HeapLibrary.freePrivateHeap(keyOptionsString);
	ULib.HeapLibrary.freePrivateHeap(outputLine1);
	ULib.HeapLibrary.freePrivateHeap(outputLine2);
	ULib.HeapLibrary.freePrivateHeap(outputLine3);
	ULib.HeapLibrary.freePrivateHeap(outputLine4);
	ULib.HeapLibrary.freePrivateHeap(outputLine5);
	ULib.HeapLibrary.freePrivateHeap(outputLine6);
	ULib.HeapLibrary.freePrivateHeap(outputLine7);

	ULib.HeapLibrary.freePrivateHeap(siteKeyDecrypted);
	ULib.HeapLibrary.freePrivateHeap(siteKey);
}

//////////////////////////////////////////////////////////////////////
// create a new site key from the information supplied (static key) //
//////////////////////////////////////////////////////////////////////
void createSiteKey(CCrypKey_SiteKey* sKey, CCrypKey_SiteCode* sCode, unsigned long key1Id, unsigned long key2Id)
{
	ULib.ConsoleLibrary.displayConsoleMessage("                                                                               ", false);

	sKey->StaticKey.setStaticKey(((key2Id << 5) | key1Id));
	sKey->StaticKey.setAccountInformation(sCode->getAccountInformation());
}

/////////////////////////////////////////////////////////
// create a new site key from the information supplied //
/////////////////////////////////////////////////////////
void createSiteKey(CCrypKey_SiteKey* sKey, CCrypKey_SiteCode* sCode)
{
	// copy site code id
	sKey->setSiteCodeId((unsigned char)(sCode->getSiteCodeId()));

	// check for required value
	bool useUserKey = false;
	bool useUserKeyHash = false;
	bool usePassword = false;
	bool useParsedParameterValue = false;

	// check for parsed value
	char* parsedUserKey = getParsedParameter("UserKey");
	char* parsedUserKeyHash = getParsedParameterForFormattedString("UserKeyHash");
	if (parsedUserKeyHash == NULL) parsedUserKeyHash = getParsedParameter("UserKeyValue");
	char* parsedPassword = getParsedParameter("Password");
	if (parsedUserKey != NULL)
	{
		useParsedParameterValue = true;
		useUserKey = true;
	}
	else if (parsedUserKeyHash != NULL)
	{
		useParsedParameterValue = true;
		useUserKeyHash = true;
	}
	else if (parsedPassword != NULL)
	{
		useParsedParameterValue = true;
		usePassword = true;
	}

	// check whether we use the user key, the user key hash or the password
	if ((!useUserKey) && (!useUserKeyHash) && (!usePassword))
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Required Information :             [(U)ser Key, User (K)ey Hash, (P)assword]", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 58, false);
		unsigned char resultRequiredInformation = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "UuKkPp");
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 58, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 85, false);
		useUserKey = ((resultRequiredInformation == 'U') || (resultRequiredInformation == 'u'));
		useUserKeyHash = ((resultRequiredInformation == 'K') || (resultRequiredInformation == 'k'));
		usePassword = ((resultRequiredInformation == 'P') || (resultRequiredInformation == 'p'));
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 58, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 58, false);
	}

	// get required value
	if (useUserKey) ULib.ConsoleLibrary.displayConsoleMessage("+ User Key             : ", false);
	else if (useUserKeyHash)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ User Key Hash (", false);
		ULib.ConsoleLibrary.displayConsoleMessage(((sKey->isSiteKeyV6()) ? "2" : "1"), false);
		ULib.ConsoleLibrary.displayConsoleMessage(")    : 0x", false);
	}
	else if (usePassword) ULib.ConsoleLibrary.displayConsoleMessage("+ Plaintext Password   : ", false);

	unsigned char* requiredInformationString = NULL;
	// user key hash (parsed or required input)
	if (useUserKeyHash)
	{
		ULib.ConsoleLibrary.displayConsoleMessage(((sKey->isSiteKeyV6()) ? "                                 [0x0000 to 0xFFFF]" : "                                     [0x00 to 0xFF]"), false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 51, false);
		requiredInformationString = ULib.ConsoleLibrary.getConsoleInputAsFormattedString(((sKey->isSiteKeyV6()) ? "HHHH" : "HH"), ((sKey->isSiteKeyV6()) ? "\?\?\?\?" : "\?\?"), true, false, NULL, false, NULL, parsedUserKeyHash);
		ULib.ConsoleLibrary.displayConsoleMessage(' ', ((sKey->isSiteKeyV6()) ? 47 : 49), true);
	}
	// parsed user key or password
	else if (useParsedParameterValue)
	{
		if (useUserKey) requiredInformationString = (unsigned char*)parsedUserKey;
		else if (usePassword) requiredInformationString = (unsigned char*)parsedPassword;
		ULib.ConsoleLibrary.displayConsoleMessage((char*)requiredInformationString);
	}
	// required input - user key or password
	else requiredInformationString = ULib.ConsoleLibrary.getConsoleInput(((usePassword) ? 12 : 32));

	int userKeyHash = 0;
	int userKeyHash6 = 0;

	// use user key
	if (useUserKey)
	{
		char* requiredInformationStringNoSpaces = ULib.StringLibrary.removeSpacesFromString((char*)requiredInformationString);
		getUserKeyHashesFromUserKey(requiredInformationStringNoSpaces, &userKeyHash, &userKeyHash6);
		ULib.HeapLibrary.freePrivateHeap(requiredInformationStringNoSpaces);
	}
	// use user key hash
	else if (useUserKeyHash)
	{
		userKeyHash = ULib.StringLibrary.getWORDFromString((char*)requiredInformationString, true);
		userKeyHash6 = ULib.StringLibrary.getWORDFromString((char*)requiredInformationString, true);
	}
	// use password
	else if (usePassword) getUserKeyHashesFromPassword((char*)requiredInformationString, &userKeyHash, &userKeyHash6);
	ULib.HeapLibrary.freePrivateHeap(requiredInformationString);

	// store user key hash
	sKey->setUserKeyHash(sCode->getUserKeyHashSeed(), userKeyHash, userKeyHash6, sCode->getHDSN());

	// add to existing licence (v3.40+)
	if (sCode->isAddToExistingLicenceAllowed())
	{
		bool siteKey_AddToExistingLicence = false;
		ULib.ConsoleLibrary.displayConsoleMessage("+ Add To Licence?      : ", false);
		char* parsedAddToExistingLicence = getParsedParameter("AddToExistingLicence");
		useParsedParameterValue = false;
		if (parsedAddToExistingLicence != NULL)
		{
			if (ULib.StringLibrary.isStringMatch(parsedAddToExistingLicence, "no"))
			{
				useParsedParameterValue = true;
				siteKey_AddToExistingLicence = false;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedAddToExistingLicence, "yes"))
			{
				useParsedParameterValue = true;
				siteKey_AddToExistingLicence = true;
			}

			ULib.HeapLibrary.freePrivateHeap(parsedAddToExistingLicence);
		}

		if (!useParsedParameterValue)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                                        [(Y)es, (N)o]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
			siteKey_AddToExistingLicence = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		}

		if (siteKey_AddToExistingLicence) sKey->setAddToExistingLicence();
		ULib.ConsoleLibrary.displayConsoleMessage(((sKey->isAddToExistingLicence()) ? "Yes                                                  " : "No                                                   "));
	}

	// easy licence (v6.00+)
	bool siteKey_IsEasyLicence = false;
	if ((sCode->isEasyLicenceAllowed()) && (!sCode->isHDSNError()))
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Easy Licence?        : ", false);
		char* parsedEasyLicence = getParsedParameter("EasyLicence");
		useParsedParameterValue = false;
		if (parsedEasyLicence != NULL)
		{
			if (ULib.StringLibrary.isStringMatch(parsedEasyLicence, "no"))
			{
				useParsedParameterValue = true;
				siteKey_IsEasyLicence = false;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedEasyLicence, "yes"))
			{
				useParsedParameterValue = true;
				siteKey_IsEasyLicence = true;
			}

			ULib.HeapLibrary.freePrivateHeap(parsedEasyLicence);
		}

		if (!useParsedParameterValue)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                                        [(Y)es, (N)o]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
			siteKey_IsEasyLicence = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		}

		if (siteKey_IsEasyLicence) sKey->setEasyLicence();
		ULib.ConsoleLibrary.displayConsoleMessage(((siteKey_IsEasyLicence) ? "Yes                                                  " : "No                                                   "));
	}

	// if using easy licence then ignore licence count and duration
	if (!siteKey_IsEasyLicence)
	{
		// number of licences (maximum 127)
		ULib.ConsoleLibrary.displayConsoleMessage("+ Number Of Licences   : ", false);
		char* parsedNumberOfLicences = getParsedParameterForFormattedString("NumberOfLicences");
		ULib.ConsoleLibrary.displayConsoleMessage("                                           [0 to 127]", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
		unsigned char* siteKey_NumberOfLicences = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("111", "   ", true, false, NULL, true, "0-127", parsedNumberOfLicences);
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteKey_NumberOfLicences), true);

		unsigned long numberOfLicences = ULib.StringLibrary.getDWORDFromString((char*)siteKey_NumberOfLicences);
		ULib.HeapLibrary.freePrivateHeap(siteKey_NumberOfLicences);
		if (numberOfLicences > 127) numberOfLicences = 127;

		// is network licence?
		bool siteKey_IsNetworkLicence = false;
		ULib.ConsoleLibrary.displayConsoleMessage("+ Network Licence?     : ", false);
		char* parsedNetworkLicence = getParsedParameter("NetworkLicence");
		useParsedParameterValue = false;
		if (parsedNetworkLicence != NULL)
		{
			if (ULib.StringLibrary.isStringMatch(parsedNetworkLicence, "no"))
			{
				useParsedParameterValue = true;
				siteKey_IsNetworkLicence = false;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedNetworkLicence, "yes"))
			{
				useParsedParameterValue = true;
				siteKey_IsNetworkLicence = true;
			}

			ULib.HeapLibrary.freePrivateHeap(parsedNetworkLicence);
		}

		if (!useParsedParameterValue)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                                        [(Y)es, (N)o]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
			siteKey_IsNetworkLicence = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		}

		// store number of licences (and network licence property)
		ULib.ConsoleLibrary.displayConsoleMessage(((siteKey_IsNetworkLicence) ? "Yes                                                  " : "No                                                   "));
		sKey->setLicenceCount(numberOfLicences, siteKey_IsNetworkLicence);

		// licence duration
		bool isUnlimited = false;
		bool isLimitedDays = false;
		bool isLimitedRuns = false;

		ULib.ConsoleLibrary.displayConsoleMessage("+ Licence Duration     : ", false);
		char* parsedLicenceDuration = getParsedParameter("LicenceDuration");
		useParsedParameterValue = false;
		if (parsedLicenceDuration != NULL)
		{
			if (ULib.StringLibrary.isStringMatch(parsedLicenceDuration, "runs"))
			{
				useParsedParameterValue = true;
				isLimitedRuns = true;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedLicenceDuration, "days"))
			{
				useParsedParameterValue = true;
				isLimitedDays = true;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedLicenceDuration, "unlimited"))
			{
				useParsedParameterValue = true;
				isUnlimited = true;
			}

			ULib.HeapLibrary.freePrivateHeap(parsedLicenceDuration);
		}

		if ((!isUnlimited) && (!isLimitedDays) && (!isLimitedRuns))
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                        [(U)nlimited, (D)ays, (R)uns]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
			unsigned char siteKey_LicenceDuration = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "UuDdRr");
			isUnlimited = ((siteKey_LicenceDuration == 'U') || (siteKey_LicenceDuration == 'u'));
			isLimitedDays = ((siteKey_LicenceDuration == 'D') || (siteKey_LicenceDuration == 'd'));
			isLimitedRuns = ((siteKey_LicenceDuration == 'R') || (siteKey_LicenceDuration == 'r'));
		}

		if (isUnlimited) ULib.ConsoleLibrary.displayConsoleMessage("Unlimited                                            ");
		else if (isLimitedDays) ULib.ConsoleLibrary.displayConsoleMessage("Limited (Days)                                       ");
		else if (isLimitedRuns) ULib.ConsoleLibrary.displayConsoleMessage("Limited (Runs)                                       ");
		else ULib.ConsoleLibrary.displayConsoleMessage(' ', 53, true);

		// get number of days/runs
		if (isUnlimited)
		{
			sKey->setLicenceDurationUnlimited();
		}
		else
		{
			// days
			if (isLimitedDays)
			{
				ULib.ConsoleLibrary.displayConsoleMessage("+ Number Of Days       : ", false);
				char* parsedNumberOfDays = getParsedParameterForFormattedString("NumberOfDays");
				ULib.ConsoleLibrary.displayConsoleMessage("                                         [0 to 32767]", false);
				ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
				unsigned char* siteKey_NumberOfDays = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11111", "     ", true, false, NULL, true, "0-32767", parsedNumberOfDays);
				ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteKey_NumberOfDays), true);
				sKey->setLicenceDurationDayLimited(ULib.StringLibrary.getDWORDFromString((char*)siteKey_NumberOfDays));
				ULib.HeapLibrary.freePrivateHeap(siteKey_NumberOfDays);
			}
			// runs
			else if (isLimitedRuns)
			{
				ULib.ConsoleLibrary.displayConsoleMessage("+ Number Of Runs       : ", false);
				char* parsedNumberOfRuns = getParsedParameterForFormattedString("NumberOfRuns");
				ULib.ConsoleLibrary.displayConsoleMessage("                                         [0 to 32767]", false);
				ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
				unsigned char* siteKey_NumberOfRuns = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11111", "     ", true, false, NULL, true, "0-32767", parsedNumberOfRuns);
				ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteKey_NumberOfRuns), true);
				sKey->setLicenceDurationRunLimited(ULib.StringLibrary.getDWORDFromString((char*)siteKey_NumberOfRuns));
				ULib.HeapLibrary.freePrivateHeap(siteKey_NumberOfRuns);
			}
		}
	}

	// key level
	unsigned long maxKeyLevel = 65535;
	ULib.ConsoleLibrary.displayConsoleMessage("+ Key Level            : ", false);
	// check for parsed value
	char* parsedKeyLevel = getParsedParameterForFormattedString("KeyLevel");
	ULib.ConsoleLibrary.displayConsoleMessage("                                         [0 to 65535]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
	unsigned char* siteKey_KeyLevel = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11111", "     ", true, false, NULL, true, "0-65535", parsedKeyLevel);
	ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteKey_KeyLevel), true);
	unsigned long keyLevel = ULib.StringLibrary.getDWORDFromString((char*)siteKey_KeyLevel);
	ULib.HeapLibrary.freePrivateHeap(siteKey_KeyLevel);

	// check maximum key level
	if (keyLevel > maxKeyLevel) keyLevel = maxKeyLevel;
	sKey->setKeyLevel(keyLevel);

	// key options
	ULib.ConsoleLibrary.displayConsoleMessage("+ Key Options          : ", false);
	char* parsedKeyOptions = getParsedParameter("KeyOptions");
	unsigned char* siteKey_KeyOptions = NULL;
	if (parsedKeyOptions != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage(parsedKeyOptions);
		siteKey_KeyOptions = (unsigned char*)parsedKeyOptions;
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("                                            [0 to 16]", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
		siteKey_KeyOptions = ULib.ConsoleLibrary.getConsoleInput(40, true, false);
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteKey_KeyOptions), true);
	}

	// parse the supplied key options string
	char* dashPtr = NULL;
	unsigned long keyOptions = 0;
	int numberOfKeyOptionElements;
	char* keyOptionElements[32];
	char* siteKey_KeyOptionsNoSpaces = ULib.StringLibrary.removeSpacesFromString((char*)siteKey_KeyOptions);
	ULib.StringLibrary.getStringElementsFromString(siteKey_KeyOptionsNoSpaces, keyOptionElements, &numberOfKeyOptionElements, 32, ',');
	ULib.HeapLibrary.freePrivateHeap(siteKey_KeyOptionsNoSpaces);

	for (int i = 0; i < numberOfKeyOptionElements; i++)
	{
		// range specified (x-y)
		dashPtr = (strstr(keyOptionElements[i], "-"));
		if (dashPtr != NULL)
		{
			int startIndex = ULib.StringLibrary.getDWORDFromString(keyOptionElements[i]);
			int endIndex = ULib.StringLibrary.getDWORDFromString(++dashPtr);
			if (startIndex < 0) startIndex = 0;
			if (endIndex > 16) endIndex = 16;
			for (int j = startIndex; j <= endIndex; j++) enableKeyOption(j, &keyOptions);
		}
		// otherwise, individual number
		else
		{
			enableKeyOption(ULib.StringLibrary.getDWORDFromString(keyOptionElements[i]), &keyOptions);
		}

		ULib.HeapLibrary.freePrivateHeap(keyOptionElements[i]);
	}

	sKey->setKeyOptions(keyOptions);
	ULib.HeapLibrary.freePrivateHeap(siteKey_KeyOptions);

	// checked for banned key options and levels (generated by CORE keygens)
	if ((!sKey->isNetworkLicence()) && (sKey->getLicenceCount() == 100) && ((sKey->getKeyOptions() == 0xFFFCFFFF) || (sKey->getKeyOptions() == 0x80000002)))
	{
		// increment licence count to beat detection
		sKey->setLicenceCount(101, false);
	}
}

/////////////////////////////////////////////////////////
// create a new site key from the information supplied //
/////////////////////////////////////////////////////////
void createSiteKey(void)
{
	unsigned char* siteKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* siteKeyEncrypted = NULL;
	unsigned char* siteKeyDecrypted = NULL;
	int siteKeyLength = 13;

	// get required information
	ULib.ConsoleLibrary.displayConsoleMessage("Key Information...");

	// site code
	ULib.ConsoleLibrary.displayConsoleMessage("+ Site Code            : ", false);
	char* parsedSiteCode = getParsedParameter("SiteCode");
	unsigned char* siteCodeString = NULL;
	if (parsedSiteCode != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage(parsedSiteCode);
		siteCodeString = (unsigned char*)parsedSiteCode;
	}
	else
	{
		siteCodeString = ULib.ConsoleLibrary.getConsoleInput(22);
	}

	// decrypt site code
	unsigned char* siteCode = NULL;
	unsigned char* siteCodeDecrypted = NULL;
	unsigned long key1Id = 0;
	unsigned long key2Id = 0;
	bool isSiteCodeV6_Win32 = false;
	char* siteCodeNoSpaces = ULib.StringLibrary.removeSpacesFromString((char*)siteCodeString);
	if (!decryptSiteCode(siteCodeNoSpaces, &siteCode, &siteCodeDecrypted, true))
	{
		isSiteCodeV6_Win32 = true;
		ULib.ConsoleLibrary.displayConsoleMessage("  Decrypt Failed - Trying v6.00 Decryption...", false);
		ULib.HeapLibrary.freePrivateHeap(siteCodeDecrypted);
		if (!decryptSiteCodeV6_Win32(siteCodeNoSpaces, &siteCode, &siteCodeDecrypted, &key1Id, &key2Id, false)) displayErrorMessage(ERROR_SITECODE6_DECRYPTION_FAILED);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 45, false);
	}

	// initialise site code & site key class
	CCrypKey_SiteCode sCode(siteCodeDecrypted);
	CCrypKey_SiteKey sKey(siteKey, &sCode);
	if (isSiteCodeV6_Win32) sCode.setIsSiteCodeV6_Win32();
	if (sCode.isSiteCodeV6()) sKey.setIsSiteKeyV6();
	if (sCode.isSiteCodeV6_Win32()) sKey.setIsSiteKeyV6_Win32();
	ULib.HeapLibrary.freePrivateHeap(siteCodeString);
	ULib.HeapLibrary.freePrivateHeap(siteCodeNoSpaces);

	// create site key (using static key)
	if ((sCode.isSiteCodeV6_Win32()) &&
		(sCode.isAddToExistingLicenceAllowed()) &&
		(sCode.isEasyLicenceAllowed()) &&
		(sCode.getCrypKeyVersion() >= 0x3D) &&
		(sCode.getHDSN() == 0x3355))
	{
		createSiteKey(&sKey, &sCode, key1Id, key2Id);
	}
	// create site key (normal key)
	else
	{
		createSiteKey(&sKey, &sCode);
	}

	// set crc value
	sKey.setCRCValue(getKeyCRC(siteKey, siteKeyLength, ((sKey.isSiteKeyV6_Win32()) ? KEYTYPE_SITEKEY6 : KEYTYPE_SITEKEY)));

	// encrypt site key (and check decryption succeeded)
	if (sCode.isSiteCodeV6_Win32())
	{
		if (key1Id >= 0x20) key1Id = 0;
		if (key2Id >= 0x20) key2Id = 0;
		siteKeyEncrypted = encryptKey6(siteKey, siteKeyLength, KEYTYPE_SITEKEY6, key1Id, key2Id);
		siteKeyDecrypted = decryptKey6(siteKeyEncrypted, siteKeyLength, KEYTYPE_SITEKEY6, key1Id, key2Id);
	}
	else
	{
		siteKeyEncrypted = encryptKey(siteKey, siteKeyLength, KEYTYPE_SITEKEY);
		siteKeyDecrypted = decryptKey(siteKeyEncrypted, siteKeyLength, KEYTYPE_SITEKEY);
	}

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(siteKeyDecrypted, siteKeyLength, sKey.getCRCValue(), ((sKey.isSiteKeyV6_Win32()) ? KEYTYPE_SITEKEY6 : KEYTYPE_SITEKEY))) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Creating Key   - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteKey, siteKeyLength, 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Encrypting Key - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteKeyEncrypted, siteKeyLength, 2, true);

	ULib.HeapLibrary.freePrivateHeap(siteCodeDecrypted);
	ULib.HeapLibrary.freePrivateHeap(siteCode);
	ULib.HeapLibrary.freePrivateHeap(siteKeyDecrypted);
	ULib.HeapLibrary.freePrivateHeap(siteKeyEncrypted);
	ULib.HeapLibrary.freePrivateHeap(siteKey);
}