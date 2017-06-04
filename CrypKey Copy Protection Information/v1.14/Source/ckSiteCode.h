// site code features - v6.00+ (32bit)
#define SITECODE_FEATURES_ADDTOEXISTINGLICENCE		0x01
#define SITECODE_FEATURES_EASYLICENCE				0x02

// site code class
class CCrypKey_SiteCode
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_SiteCode
	{
		unsigned char encryptionSeed;
		union _U1
		{
			unsigned char siteCodeId;
			unsigned char supportedFeatures; // v6.00+ (32bit)
		} U1;
		unsigned char crypkeyVersion;
		unsigned short accountInformation;
		union _U2
		{
			unsigned short userKeyHashSeed;
			unsigned short driveSerialNumber; // v6.00+ (Easy Licence)
		} U2;
		unsigned short crcValue;
	} CrypKey_SiteCode;
	#pragma pack()

	CrypKey_SiteCode* sCode;
	bool sCodeIsV6_Win32;

public:
	CCrypKey_SiteCode(unsigned char* decryptedBuffer)
	{
		sCode = (CrypKey_SiteCode*)decryptedBuffer;
		sCodeIsV6_Win32 = false;
	}

	void formatSiteCode(char** outputBuffer)
	{
		unsigned char* sCodeBuffer = (unsigned char*)sCode;
		wsprintf(*outputBuffer, "                            %02X %02X %02X%02X %02X%02X %02X%02X",
			sCodeBuffer[1],
			sCodeBuffer[2],
			sCodeBuffer[3],	sCodeBuffer[4],
			sCodeBuffer[5], sCodeBuffer[6],
			sCodeBuffer[7], sCodeBuffer[8]);
	}

	bool isSiteCodeV6(void) { return ((getCrypKeyVersionMajor() >= 6) || (isSiteCodeV6_Win32())); }
	bool isSiteCodeV6_Win32(void) { return (sCodeIsV6_Win32); }
	void setIsSiteCodeV6_Win32(void) { sCodeIsV6_Win32 = true; }

	unsigned short getCRCValue(void) { return (unsigned short)((sCode->crcValue >> 8) | (sCode->crcValue << 8)); }
	unsigned long getSiteCodeId(void) { return ((isSiteCodeV6_Win32()) ? 0 : (sCode->U1.siteCodeId & 0x7F)); }
	unsigned long getSupportedFeatures(void) { return ((isSiteCodeV6_Win32()) ? (sCode->U1.supportedFeatures & 0x7F) : 0); } 
	unsigned long getCrypKeyVersion(void) { return (sCode->crypkeyVersion & 0x7F); }
	unsigned long getCrypKeyVersionMajor(void) { return (getCrypKeyVersion() / 10); }
	unsigned long getCrypKeyVersionMinor(void) { return (getCrypKeyVersion() % 10); }
	unsigned long getAccountInformation(void) { return sCode->accountInformation; }
	unsigned long getAccountNumber(void) { return (sCode->accountInformation & 0x03FF); }
	unsigned long getApplicationId(void) { return (sCode->accountInformation >> 0x0A); }
	char* getCompanyName(int maximumLength) { return getCompanyNameFromAccountNumber(getAccountNumber(), maximumLength); }
	unsigned long getUserKeyHashSeed(void) { return sCode->U2.userKeyHashSeed; }
	//bool isPhysicalHDSN(void) { return ((sCode->U2.driveSerialNumber >> 8) > 0x7F); } // not supported
	bool isHDSNError(void) { return ((sCode->U2.driveSerialNumber >> 8) > 0x7F); }
	unsigned long getHDSN(void) { return ((isHDSNError()) ? ((sCode->U2.driveSerialNumber * -1) & 0xFFFF) : sCode->U2.driveSerialNumber); }
	bool isAddToExistingLicenceSupported(void) { return ((sCode->crypkeyVersion & 0x7F) >= 0x22); } // v3.40+
	bool isAddToExistingLicenceAllowed(void) { return ((isAddToExistingLicenceSupported()) ? ((isSiteCodeV6_Win32()) ? ((getSupportedFeatures() & SITECODE_FEATURES_ADDTOEXISTINGLICENCE) ? true : false) : (sCode->U1.siteCodeId > 0x7F)) : false); }
	bool isEasyLicenceSupported(void) { return (isSiteCodeV6()); } // v6.00+
	bool isEasyLicenceAllowed(void) { return ((isEasyLicenceSupported()) ? ((isSiteCodeV6_Win32()) ? ((getSupportedFeatures() & SITECODE_FEATURES_EASYLICENCE) ? true : false) : (sCode->crypkeyVersion > 0x7F)) : false); }
	bool isStaticSiteCode(void) { return ((isEasyLicenceAllowed()) && (getHDSN() == 0x3355)); } // v6.00+

	void setCRCValue(unsigned short crcValue) { sCode->crcValue = (unsigned short)((crcValue >> 8) | (crcValue << 8)); }
	void setSiteCodeId(unsigned char siteCodeId) { if (!isSiteCodeV6_Win32()) { sCode->U1.siteCodeId = (unsigned char)(siteCodeId & 0x7F); } }
	void setCrypKeyVersion(unsigned char majorVersion, unsigned char minorVersion) { if ((isSiteCodeV6_Win32()) && (majorVersion < 6)) { majorVersion = 6; minorVersion = 0; } sCode->crypkeyVersion = (unsigned char)(((majorVersion * 10) + (minorVersion % 10)) & 0x7F); }
	void setAccountNumber(unsigned short accountNumber) { sCode->accountInformation |= (accountNumber & 0x03FF); }
	void setApplicationId(unsigned char applicationId) { sCode->accountInformation |= ((applicationId * 4) << 8); }
	void setUserKeyHashSeed(unsigned short userKeyHashSeed) { sCode->U2.userKeyHashSeed = userKeyHashSeed; }
	void setHDSN(unsigned short driveSerialNumber, bool isPhysicalDrive) { sCode->U2.driveSerialNumber = (unsigned short)((isPhysicalDrive) ? (((driveSerialNumber * -1) & 0xFFFF) | 0x8000) : (driveSerialNumber & 0x7FFF)); }
	void setAddToExistingLicenceAllowed(void) { if (isAddToExistingLicenceSupported()) { ((isSiteCodeV6_Win32()) ? (sCode->U1.supportedFeatures |= SITECODE_FEATURES_ADDTOEXISTINGLICENCE) : (sCode->U1.siteCodeId |= 0x80)); } }
	void setEasyLicenceAllowed(void) { if (isEasyLicenceSupported()) { ((isSiteCodeV6_Win32()) ? (sCode->U1.supportedFeatures |= SITECODE_FEATURES_EASYLICENCE) : (sCode->crypkeyVersion |= 0x80)); } }
};

bool decryptSiteCode(char* siteCodeString, unsigned char** siteCode, unsigned char** siteCodeDecrypted, bool useByCreateKey);
bool decryptSiteCodeV6_Win32(char* siteCodeString, unsigned char** siteCode, unsigned char** siteCodeDecrypted, unsigned long* key1Id, unsigned long* key2Id, bool useByCreateKey);

void showSiteCodeInformation(char* siteCodeString);
void createSiteCode(int keyType);