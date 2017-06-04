#include "ckInfo.h"
#include "ckEncryption.h"
#include "ckEncryption6.h"
#include "ckSiteCode.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

///////////////////////////
// decrypt the site code //
///////////////////////////
bool decryptSiteCode(char* siteCodeString, unsigned char** siteCode, unsigned char** siteCodeDecrypted, bool useByCreateKey)
{
	// check site code length
	int siteCodeLength = ULib.StringLibrary.getStringLength(siteCodeString);
	if (siteCodeLength != 18)
	{
		if (useByCreateKey) ULib.ConsoleLibrary.displayConsoleMessage("");
		displayErrorMessage(ERROR_SITECODE_LENGTH);
	}

	// convert site code string into bytes
	if (*siteCode == NULL) *siteCode = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 0; i < siteCodeLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(*siteCode, siteCodeString[i], siteCodeString[i + 1], (i / 2));

	// decrypt the site code
	siteCodeLength = 9;
	*siteCodeDecrypted = decryptKey(*siteCode, siteCodeLength, KEYTYPE_SITECODE);

	// initialise site code class
	CCrypKey_SiteCode sCode(*siteCodeDecrypted);
	return (isKeyCRCCorrect(*siteCodeDecrypted, siteCodeLength, sCode.getCRCValue(), KEYTYPE_SITECODE));
}

///////////////////////////
// decrypt the site code //
///////////////////////////
bool decryptSiteCodeV6_Win32(char* siteCodeString, unsigned char** siteCode, unsigned char** siteCodeDecrypted, unsigned long* key1Id, unsigned long* key2Id, bool useByCreateKey)
{
	// check site code length
	int siteCodeLength = ULib.StringLibrary.getStringLength(siteCodeString);
	if (siteCodeLength != 18)
	{
		if (useByCreateKey) ULib.ConsoleLibrary.displayConsoleMessage("");
		displayErrorMessage(ERROR_SITECODE_LENGTH);
	}

	// convert site code string into bytes
	if (*siteCode == NULL) *siteCode = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 0; i < siteCodeLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(*siteCode, siteCodeString[i], siteCodeString[i + 1], (i / 2));

	// decrypt the site code
	siteCodeLength = 9;
	*siteCodeDecrypted = findDecryptionKeyIds(*siteCode, siteCodeLength, KEYTYPE_SITECODE6, key1Id, key2Id);
	return (*siteCodeDecrypted != NULL);
}

//////////////////////////////////////////////////////
// display information retrieved from the site code //
//////////////////////////////////////////////////////
void showSiteCodeInformation(char* siteCodeString)
{
	unsigned char* siteCode = NULL;
	unsigned char* siteCodeDecrypted = NULL;

	// decrypt site code
	unsigned long siteCodeLength = 9;
	bool decryptResult = decryptSiteCode(siteCodeString, &siteCode, &siteCodeDecrypted, false);

	ULib.ConsoleLibrary.displayConsoleMessage("Parsing Code    - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteCode, siteCodeLength, 2, true);
	bool isSiteCodeV6_Win32 = false;
	unsigned long key1Id = 0;
	unsigned long key2Id = 0;

	// try v6.00 decryption
	if (!decryptResult)
	{
		isSiteCodeV6_Win32 = true;
		ULib.ConsoleLibrary.displayConsoleMessage("Decrypt Failed  - Trying v6.00 Decryption...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 44, false);
		ULib.HeapLibrary.freePrivateHeap(siteCodeDecrypted);
		if (!decryptSiteCodeV6_Win32(siteCodeString, &siteCode, &siteCodeDecrypted, &key1Id, &key2Id, false)) displayErrorMessage(ERROR_SITECODE6_DECRYPTION_FAILED);
	}

	// initialise site code class
	CCrypKey_SiteCode sCode(siteCodeDecrypted);
	if (isSiteCodeV6_Win32) sCode.setIsSiteCodeV6_Win32();

	ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Code - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteCodeDecrypted, siteCodeLength, 2, false);
	if (!sCode.isSiteCodeV6_Win32()) ULib.ConsoleLibrary.displayConsoleMessage("");
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage(" [0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(key1Id, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		ULib.ConsoleLibrary.displayConsoleMessage(":0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(key2Id, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		ULib.ConsoleLibrary.displayConsoleMessage("]");
	}
	ULib.ConsoleLibrary.displayConsoleMessage("Code Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(siteCodeDecrypted, siteCodeLength, sCode.getCRCValue(), ((sCode.isSiteCodeV6_Win32()) ? KEYTYPE_SITECODE6 : KEYTYPE_SITECODE))) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Formatting Code :");

	// format site code
	char* sCodeFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	sCode.formatSiteCode(&sCodeFormatted);

	// generate output and display
	char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine6 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine7 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine8 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* companyName = sCode.getCompanyName(26);
	bool isCompanyNameAvailable = (ULib.StringLibrary.getStringLength(companyName) > 0);

	char* hsdnPaddedString = createPaddedStringFromValue(sCode.getHDSN(), 5, 'อ', false);
	wsprintf(outputLine1, "                            ศน ศน ฬออผ ฬออผ ฬออผ");
	if (sCode.isSiteCodeV6_Win32()) wsprintf(outputLine2, "                             บ  บ บ    บ    ศอออออออออออออออ Code CRC - 0x%04X", sCode.getCRCValue());
	else wsprintf(outputLine2, "  Site Code Id - 0x%02X อออออออน  บ บ    บ    ศอออออออออออออออ Code CRC - 0x%04X", sCode.getSiteCodeId(), sCode.getCRCValue());
	wsprintf(outputLine3, "  Allow Add Licence? - %sออ%c  บ บ    ฬออออออออ User Key Hash (Seed) - 0x%04X", ((sCode.isAddToExistingLicenceAllowed()) ? "Yes " : "No อ"), ((sCode.isSiteCodeV6_Win32()) ? 'น' : 'ผ'), sCode.getUserKeyHashSeed());
	if (sCode.isStaticSiteCode()) wsprintf(outputLine4, "  Allow Easy Licence? - Yes อ%s บ    ศอออออออออ Drive Serial Number - unused", ((sCode.isSiteCodeV6_Win32()) ? "ผ  บ" : "อออน"));
	else if (sCode.isEasyLicenceAllowed()) wsprintf(outputLine4, "  Allow Easy Licence? - Yes อ%s บ    ศออ%s%s Drive Serial Number%s - %d", ((sCode.isSiteCodeV6_Win32()) ? "ผ  บ" : "อออน"), hsdnPaddedString, ((sCode.isHDSNError()) ? "" : "ออออออออ"), ((sCode.isHDSNError()) ? " (Error)" : ""), sCode.getHDSN());
	else wsprintf(outputLine4, "  Allow Easy Licence? - No ออ%s บ    ศอออออออออ Drive Serial Number - unused", ((sCode.isSiteCodeV6_Win32()) ? "ผ  บ" : "อออน"));
	wsprintf(outputLine5, "  CrypKey Libraries - v%d.%d อออออผ ฬอ Account Number - %d", sCode.getCrypKeyVersionMajor(), sCode.getCrypKeyVersionMinor(), sCode.getAccountNumber());
	wsprintf(outputLine6, "                                  ฬอ Application Id - %d", sCode.getApplicationId());
	wsprintf(outputLine7, "                                  %cอ Company Number - 7956%d", ((isCompanyNameAvailable) ? 'ฬ' : 'ศ'), sCode.getAccountNumber());
	wsprintf(outputLine8, "                                  ศอ Company Name - %s", companyName);
	ULib.HeapLibrary.freePrivateHeap(hsdnPaddedString);

	ULib.ConsoleLibrary.displayConsoleMessage(sCodeFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine7);
	if (isCompanyNameAvailable) ULib.ConsoleLibrary.displayConsoleMessage(outputLine8);

	ULib.HeapLibrary.freePrivateHeap(sCodeFormatted);
	ULib.HeapLibrary.freePrivateHeap(companyName);
	ULib.HeapLibrary.freePrivateHeap(outputLine1);
	ULib.HeapLibrary.freePrivateHeap(outputLine2);
	ULib.HeapLibrary.freePrivateHeap(outputLine3);
	ULib.HeapLibrary.freePrivateHeap(outputLine4);
	ULib.HeapLibrary.freePrivateHeap(outputLine5);
	ULib.HeapLibrary.freePrivateHeap(outputLine6);
	ULib.HeapLibrary.freePrivateHeap(outputLine7);
	ULib.HeapLibrary.freePrivateHeap(outputLine8);

	ULib.HeapLibrary.freePrivateHeap(siteCode);
	ULib.HeapLibrary.freePrivateHeap(siteCodeDecrypted);
}

//////////////////////////////////////////////////////////
// create a new site code from the information supplied //
//////////////////////////////////////////////////////////
void createSiteCode(int keyType)
{
	unsigned char* siteCode = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* siteCodeEncrypted = NULL;
	unsigned char* siteCodeDecrypted = NULL;
	bool useParsedParameterValue = false;
	int siteCodeLength = 9;

	// initialise site code class
	CCrypKey_SiteCode sCode(siteCode);
	if (keyType == KEYTYPE_SITECODE6) sCode.setIsSiteCodeV6_Win32();

	// get required information
	ULib.ConsoleLibrary.displayConsoleMessage("Code Information...");

	// account number
	ULib.ConsoleLibrary.displayConsoleMessage("+ Account Number       : ", false);
	char* parsedAccountNumber = getParsedParameterForFormattedString("AccountNumber");
	ULib.ConsoleLibrary.displayConsoleMessage("                                          [0 to 1023]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
	unsigned char* siteCode_AccountNumber = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("1111", "    ", true, false, NULL, true, "0-1023", parsedAccountNumber);
	ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteCode_AccountNumber), true);
	sCode.setAccountNumber(ULib.StringLibrary.getWORDFromString((char*)siteCode_AccountNumber));
	ULib.HeapLibrary.freePrivateHeap(siteCode_AccountNumber);

	// application id
	ULib.ConsoleLibrary.displayConsoleMessage("+ Application Id       : ", false);
	char* parsedApplicationId = getParsedParameterForFormattedString("ApplicationId");
	ULib.ConsoleLibrary.displayConsoleMessage("                                            [0 to 63]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
	unsigned char* siteCode_ApplicationId = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11", "  ", true, false, NULL, true, "0-63", parsedApplicationId);
	ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteCode_ApplicationId), true);
	sCode.setApplicationId(ULib.StringLibrary.getBYTEFromString((char*)siteCode_ApplicationId));
	ULib.HeapLibrary.freePrivateHeap(siteCode_ApplicationId);

	// crypkey library version
	ULib.ConsoleLibrary.displayConsoleMessage("+ CrypKey Libraries    : v", false);
	char* parsedCrypKeyLibraries = getParsedParameterForFormattedString("CrypKeyLibraries");
	ULib.ConsoleLibrary.displayConsoleMessage("                                       [major.minor]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 52, false);
	unsigned char* siteCode_LibraryVersion = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("1~1", "\?.\?", true, false, NULL, false, ((sCode.isSiteCodeV6_Win32()) ? "6-9|0-9" : "1-9|0-9"), parsedCrypKeyLibraries);
	ULib.ConsoleLibrary.displayConsoleMessage(' ', 49, true);
	char* siteCode_LibraryMinorVersion = ULib.StringLibrary.getNextTokenString((char*)siteCode_LibraryVersion, '.');
	sCode.setCrypKeyVersion(ULib.StringLibrary.getBYTEFromString((char*)siteCode_LibraryVersion, false), ((siteCode_LibraryMinorVersion != NULL) ? ULib.StringLibrary.getBYTEFromString(siteCode_LibraryMinorVersion, false) : 0));
	ULib.HeapLibrary.freePrivateHeap(siteCode_LibraryVersion);

	// site code id
	if (!sCode.isSiteCodeV6_Win32())
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Site Code Id         : 0x", false);
		char* parsedSiteCodeId = getParsedParameterForFormattedString("SiteCodeId");
		if (parsedSiteCodeId == NULL) parsedSiteCodeId = getParsedParameterForFormattedString("SiteCodeCheckByte");
		ULib.ConsoleLibrary.displayConsoleMessage("                                     [0x00 to 0x7F]", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 51, false);
		unsigned char* siteCode_Id = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("HH", "\?\?", true, false, NULL, false, "0x00-0x7F", parsedSiteCodeId);
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 49, true);
		sCode.setSiteCodeId(ULib.StringLibrary.getBYTEFromString((char*)siteCode_Id, true));
		ULib.HeapLibrary.freePrivateHeap(siteCode_Id);
	}

	// allow add to licence?
	if (sCode.isAddToExistingLicenceSupported())
	{
		bool siteCode_AllowAddToExistingLicence = false;
		ULib.ConsoleLibrary.displayConsoleMessage("+ Allow Add Licence?   : ", false);
		char* parsedAllowAddToExistingLicence = getParsedParameter("AllowAddToExistingLicence");
		useParsedParameterValue = false;
		if (parsedAllowAddToExistingLicence != NULL)
		{
			if (ULib.StringLibrary.isStringMatch(parsedAllowAddToExistingLicence, "no"))
			{
				useParsedParameterValue = true;
				siteCode_AllowAddToExistingLicence = false;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedAllowAddToExistingLicence, "yes"))
			{
				useParsedParameterValue = true;
				siteCode_AllowAddToExistingLicence = true;
			}

			ULib.HeapLibrary.freePrivateHeap(parsedAllowAddToExistingLicence);
		}

		if (!useParsedParameterValue)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                                        [(Y)es, (N)o]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
			siteCode_AllowAddToExistingLicence = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		}

		if (siteCode_AllowAddToExistingLicence) sCode.setAddToExistingLicenceAllowed();
		ULib.ConsoleLibrary.displayConsoleMessage(((sCode.isAddToExistingLicenceAllowed()) ? "Yes                                                  " : "No                                                   "));
	}

	// allow easy licence?
	if (sCode.isSiteCodeV6())
	{
		bool siteCode_AllowEasyLicence = false;
		ULib.ConsoleLibrary.displayConsoleMessage("+ Allow Easy Licence?  : ", false);
		char* parsedAllowEasyLicence = getParsedParameter("AllowEasyLicence");
		if (parsedAllowEasyLicence == NULL) parsedAllowEasyLicence = getParsedParameter("UseDriveSerial");
		useParsedParameterValue = false;
		if (parsedAllowEasyLicence != NULL)
		{
			if (ULib.StringLibrary.isStringMatch(parsedAllowEasyLicence, "no"))
			{
				useParsedParameterValue = true;
				siteCode_AllowEasyLicence = false;
			}
			else if (ULib.StringLibrary.isStringMatch(parsedAllowEasyLicence, "yes"))
			{
				useParsedParameterValue = true;
				siteCode_AllowEasyLicence = true;
			}

			ULib.HeapLibrary.freePrivateHeap(parsedAllowEasyLicence);
		}

		if (!useParsedParameterValue)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                                        [(Y)es, (N)o]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
			siteCode_AllowEasyLicence = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		}

		if (siteCode_AllowEasyLicence) sCode.setEasyLicenceAllowed();
		ULib.ConsoleLibrary.displayConsoleMessage(((sCode.isEasyLicenceAllowed()) ? "Yes                                                  " : "No                                                   "));
	}

	// drive serial number (v6.00+)
	if (sCode.isEasyLicenceAllowed())
	{
		
		ULib.ConsoleLibrary.displayConsoleMessage("+ Drive Serial Number  : ", false);
		char* parsedDriveSerialNumber = getParsedParameterForFormattedString("DriveSerialNumber");
		ULib.ConsoleLibrary.displayConsoleMessage("                                         [0 to 32767]", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
		unsigned char* siteCode_DriveSerialNumber = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11111", "     ", true, false, NULL, true, "0-32767", parsedDriveSerialNumber);
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 53 - ULib.StringLibrary.getStringLength((char*)siteCode_DriveSerialNumber), true);
		sCode.setHDSN((unsigned short)ULib.StringLibrary.getWORDFromString((char*)siteCode_DriveSerialNumber), false);
		ULib.HeapLibrary.freePrivateHeap(siteCode_DriveSerialNumber);
	}
	// user key hash (seed)
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ User Key Hash (Seed) : 0x", false);
		char* parsedUserKeyHashSeed = getParsedParameterForFormattedString("UserKeyHashSeed");
		if (parsedUserKeyHashSeed == NULL) parsedUserKeyHashSeed = getParsedParameterForFormattedString("UserKeyValueSeed");
		ULib.ConsoleLibrary.displayConsoleMessage("                                 [0x0000 to 0xFFFF]", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 51, false);
		unsigned char* siteCode_UserKeyHashSeed = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("HHHH", "\?\?\?\?", true, false, NULL, false, NULL, parsedUserKeyHashSeed);
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 47, true);
		sCode.setUserKeyHashSeed((unsigned short)ULib.StringLibrary.getWORDFromString((char*)siteCode_UserKeyHashSeed, true));
		ULib.HeapLibrary.freePrivateHeap(siteCode_UserKeyHashSeed);
	}

	unsigned long key1Id = 0;
	unsigned long key2Id = 0;

	// encryption key ids (v6.00 - Win32)
	if (sCode.isSiteCodeV6_Win32())
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Encryption Key Ids   : ", false);
		char* parsedEncryptionKeyId1 = getParsedParameter("EncryptionKeyId1");
		if (parsedEncryptionKeyId1 == NULL) parsedEncryptionKeyId1 = getParsedParameter("EncryptionId1");
		char* parsedEncryptionKeyId2 = getParsedParameter("EncryptionKeyId2");
		if (parsedEncryptionKeyId2 == NULL) parsedEncryptionKeyId2 = getParsedParameter("EncryptionId2");
		bool usedParsedEncryptionKeyId1 = (parsedEncryptionKeyId1 != NULL);

		// check encryption key ids are valid
		if (usedParsedEncryptionKeyId1)
		{
			key1Id = ULib.StringLibrary.getBYTEFromString(parsedEncryptionKeyId1);
			if (key1Id > 0x1F) usedParsedEncryptionKeyId1 = false;
		}
		bool usedParsedEncryptionKeyId2 = (parsedEncryptionKeyId2 != NULL);
		if (usedParsedEncryptionKeyId2)
		{
			key2Id = ULib.StringLibrary.getBYTEFromString(parsedEncryptionKeyId2);
			if (key2Id > 0x1F) usedParsedEncryptionKeyId2 = false;
		}

		bool usedParsedEncryptionKeyIds = ((usedParsedEncryptionKeyId1) && (usedParsedEncryptionKeyId2));

		char* formattingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
		unsigned char* siteCode_EncryptionKeyIds = NULL;
		ULib.StringLibrary.copyString(formattingString, "\?\?:0x\?\?");

		if (!usedParsedEncryptionKeyIds)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("                                       [0x00 to 0x1F]", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 53, false);
		}

		ULib.ConsoleLibrary.displayConsoleMessage("0x", false);
		if (usedParsedEncryptionKeyId1)
		{
			ULib.ConsoleLibrary.displayConsoleMessage(key1Id, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.StringLibrary.copyString(formattingString, "\?\?");
		}
		else
		{
			ULib.ConsoleLibrary.displayConsoleMessage("\?\?", false);
		}

		ULib.ConsoleLibrary.displayConsoleMessage(":0x", false);
		if (usedParsedEncryptionKeyId2)
		{
			ULib.ConsoleLibrary.displayConsoleMessage(key2Id, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			wsprintf(formattingString, "\?\?:0x%02X", key2Id);
		}
		else
		{
			ULib.ConsoleLibrary.displayConsoleMessage("\?\?", false);
		}

		// get encryption key id 1
		if ((!usedParsedEncryptionKeyId1) && (usedParsedEncryptionKeyId2))
		{
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 7, false);
			siteCode_EncryptionKeyIds = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("HH~~~~~", formattingString, true, false, NULL, false, "0x00-0x1F");
			key1Id = ULib.StringLibrary.getBYTEFromString((char*)siteCode_EncryptionKeyIds, true);
		}
		// get encryption key id 2
		else if ((usedParsedEncryptionKeyId1) && (!usedParsedEncryptionKeyId2))
		{
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 2, false);
			siteCode_EncryptionKeyIds = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("HH", formattingString, true, false, NULL, false, "0x00-0x1F");
			key2Id = ULib.StringLibrary.getBYTEFromString((char*)siteCode_EncryptionKeyIds, true);
		}
		// get both encryption keys
		else if (!usedParsedEncryptionKeyIds)
		{
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 7, false);
			siteCode_EncryptionKeyIds = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("HH~~~HH", formattingString, true, false, NULL, false, "0x00-0x1F|0x00-0x1F");
			key1Id = ULib.StringLibrary.getBYTEFromString((char*)siteCode_EncryptionKeyIds, true);
			key2Id = ULib.StringLibrary.getBYTEFromString((char*)&siteCode_EncryptionKeyIds[5], true);
		}

		ULib.ConsoleLibrary.displayConsoleMessage("                                            ");
		ULib.HeapLibrary.freePrivateHeap(siteCode_EncryptionKeyIds);
		ULib.HeapLibrary.freePrivateHeap(formattingString);
		ULib.HeapLibrary.freePrivateHeap(parsedEncryptionKeyId1);
		ULib.HeapLibrary.freePrivateHeap(parsedEncryptionKeyId2);
	}

	// add crc value
	sCode.setCRCValue(getKeyCRC(siteCode, siteCodeLength, ((sCode.isSiteCodeV6_Win32()) ? KEYTYPE_SITECODE : KEYTYPE_SITECODE)));

	// encrypt site code (and check decryption succeeded)
	if (sCode.isSiteCodeV6_Win32())
	{
		if (key1Id >= 0x20) key1Id = 0;
		if (key2Id >= 0x20) key2Id = 0;
		siteCodeEncrypted = encryptKey6(siteCode, siteCodeLength, KEYTYPE_SITECODE6, key1Id, key2Id);
		siteCodeDecrypted = decryptKey6(siteCodeEncrypted, siteCodeLength, KEYTYPE_SITECODE6, key1Id, key2Id);
	}
	else
	{
		siteCodeEncrypted = encryptKey(siteCode, siteCodeLength, KEYTYPE_SITECODE);
		siteCodeDecrypted = decryptKey(siteCodeEncrypted, siteCodeLength, KEYTYPE_SITECODE);
	}

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("Code Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(siteCodeDecrypted, siteCodeLength, sCode.getCRCValue(), ((sCode.isSiteCodeV6_Win32()) ? KEYTYPE_SITECODE6 : KEYTYPE_SITECODE))) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Creating Code   - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteCode, siteCodeLength, 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Encrypting Code - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(siteCodeEncrypted, siteCodeLength, 2, true);

	ULib.HeapLibrary.freePrivateHeap(siteCodeDecrypted);
	ULib.HeapLibrary.freePrivateHeap(siteCodeEncrypted);
	ULib.HeapLibrary.freePrivateHeap(siteCode);
}