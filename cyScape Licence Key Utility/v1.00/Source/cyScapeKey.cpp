///////////////////////////////////////////////////////
// cyScape Licence Key Utility                       //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// Used to display the information about cyScape     //
// licence files and also create new licence files   //
//                                                   //
// Example - DENT036111901S50X                       //
//                                                   //
// D       - is trial version? (D - yes, P - no)     //
// ENT     - product edition                         //
//           - GUI (Standard)                        //
//           - SRV (Professional)                    //
//           - ENT (Enterprise)                      //
// 036     - *registered property (>= 700 && <= 900) //
// 111901  - expiry date (trial or BDF) (19/11/2001) //
// S       - *BDF subscription? (S - yes, ~S - no)   //
// 50      - product version (v5.00)                 //
// X       - product type                            //
//           - X (BrowserHawk - ActiveX/COM/.NET)    //
//           - J (BrowserHawk - Java)                //
//           - C (CountryHawk - ActiveX/COM/.NET)    //
//           - D (CountryHawk - Java)                //
//                                                   //
// (* BrowserHawk Editor - ActiveX/COM/.NET)         //
///////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the cyScape licence file",
	"Doesn't appear to be a supported cyScape licence file",
	"{CRLF}Error occurred decoding the licence key - invalid crc value",
	"{CRLF}Error occurred decoding the licence key - unsupported key format",
	"Error occurred encoding the licence key - unrecognised key format",
	"{CRLF}Error occurred while creating the new licence file"
};

/////////////////////////////////////////////////////////////
// used to generate the crc value from the supplied string //
/////////////////////////////////////////////////////////////
short generateLicenceCRCValue(char* licenceString)
{
	int crcValues1[16] = {0, 4225, 8450, 12675, 16900, 21125, 25350, 29575, 33800, 38025, 42250, 46475, 50700, 54925, 59150, 63375};
	int crcValues2[16] = {0, 4489, 8978, 12955, 17956, 22445, 25910, 29887, 35912, 40385, 44890, 48851, 51820, 56293, 59774, 63735};
	short currentCRCValue = 1234;
	int currentCharValue = 0;

	// get crc value
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceString); i++)
	{
		currentCharValue = (licenceString[i] ^ currentCRCValue);
		currentCRCValue = (short)((currentCRCValue / 256) ^ (crcValues1[((currentCharValue & 0xFF) / 16)]) ^ (crcValues2[(currentCharValue & 0xF)]));
	}

	return (short)(currentCRCValue & 0xFFFF);
}

////////////////////////////////////
// used to decode the licence key //
////////////////////////////////////
char* decodeLicenceKey(char* licenceKey, char* licenceUserName, char* licenceCompanyName, int keyXORValue, int keyUserValue)
{
	char* decodedlicenceKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* userInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int currentIndex = 0;

	// trim the user information
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceUserName); i++)
	{
		if ((licenceUserName[i] >= 'A') && (licenceUserName[i] <= 'Z')) userInformation[currentIndex++] = (char)(licenceUserName[i] + 0x20);
		else if ((licenceUserName[i] >= 'a') && (licenceUserName[i] <= 'z')) userInformation[currentIndex++] = licenceUserName[i];
	}
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceCompanyName); i++)
	{
		if ((licenceCompanyName[i] >= 'A') && (licenceCompanyName[i] <= 'Z')) userInformation[currentIndex++] = (char)(licenceCompanyName[i] + 0x20);
		else if ((licenceCompanyName[i] >= 'a') && (licenceCompanyName[i] <= 'z')) userInformation[currentIndex++] = licenceCompanyName[i];
	}

	int userInformationLength = ULib.StringLibrary.getStringLength(userInformation);

	// decode the key
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceKey) / 2; i++)
	{
		if (++keyUserValue > userInformationLength) keyUserValue = 1;
		decodedlicenceKey[i] = (char)((ULib.StringLibrary.getBYTEFromString((char*)&licenceKey[(i * 2)], true) ^ keyXORValue) - userInformation[keyUserValue - 1] + 79);
	}

	ULib.HeapLibrary.freePrivateHeap(userInformation);
	return decodedlicenceKey;
}

////////////////////////////////////
// used to encode the licence key //
////////////////////////////////////
char* encodeLicenceKey(char* licenceKey, char* licenceUserName, char* licenceCompanyName, int keyXORValue, int keyUserValue)
{
	char* encodedlicenceKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* userInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int currentIndex = 0;

	// trim the user information
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceUserName); i++)
	{
		if ((licenceUserName[i] >= 'A') && (licenceUserName[i] <= 'Z')) userInformation[currentIndex++] = (char)(licenceUserName[i] + 0x20);
		else if ((licenceUserName[i] >= 'a') && (licenceUserName[i] <= 'z')) userInformation[currentIndex++] = licenceUserName[i];
	}
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceCompanyName); i++)
	{
		if ((licenceCompanyName[i] >= 'A') && (licenceCompanyName[i] <= 'Z')) userInformation[currentIndex++] = (char)(licenceCompanyName[i] + 0x20);
		else if ((licenceCompanyName[i] >= 'a') && (licenceCompanyName[i] <= 'z')) userInformation[currentIndex++] = licenceCompanyName[i];
	}

	int userInformationLength = ULib.StringLibrary.getStringLength(userInformation);

	// encode the key
	for (int i = 0; i < ULib.StringLibrary.getStringLength(licenceKey); i++)
	{
		if (++keyUserValue > userInformationLength) keyUserValue = 1;
		char* byteString = ULib.StringLibrary.getStringFromBYTE((char)(keyXORValue ^ (licenceKey[i] + userInformation[keyUserValue - 1] - 79)), true);
		ULib.StringLibrary.appendString(encodedlicenceKey, byteString);
		ULib.HeapLibrary.freePrivateHeap(byteString);
	}

	ULib.HeapLibrary.freePrivateHeap(userInformation);
	return encodedlicenceKey;
}

////////////////////////////////////////////////////////////////
// displays the information contained within the licence file //
////////////////////////////////////////////////////////////////
void displayLicenceInformation(char* licenceFilename)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(licenceFilename);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	char* stringBufferContext;
	char* licenceUserName = NULL;
	char* licenceCompanyName = NULL;
	char* licenceKey = NULL;

	// get licence properties
	__try
	{
		licenceUserName = ULib.StringLibrary.removeCharacterFromString(strtok_s((char*)fileBuffer, "\r\n", &stringBufferContext), 0x22);
		licenceCompanyName = ULib.StringLibrary.removeCharacterFromString(strtok_s(NULL, "\r\n", &stringBufferContext), 0x22);
		licenceKey = ULib.StringLibrary.removeCharacterFromString(strtok_s(NULL, "\r\n", &stringBufferContext), 0x22);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	// check for required licence properties
	if ((licenceUserName == NULL) || (licenceCompanyName == NULL) || (licenceKey == NULL)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	if (ULib.StringLibrary.getStringLength(licenceKey) != 42) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// display licence properties
	ULib.ConsoleLibrary.displayConsoleMessage("[Encoded Licence Information]");

	char* licenceFilenameStripped = ULib.FileLibrary.stripPathFromFilename(licenceFilename, true);
	ULib.ConsoleLibrary.displayConsoleMessage("  Licence Filename  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceFilenameStripped);
	ULib.HeapLibrary.freePrivateHeap(licenceFilenameStripped);

	ULib.ConsoleLibrary.displayConsoleMessage("  User Name         : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceUserName);

	ULib.ConsoleLibrary.displayConsoleMessage("  Company Name      : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceCompanyName);

	ULib.ConsoleLibrary.displayConsoleMessage("  Licence Key       : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceKey);

	// remove the crc value from the licence key
	char* strippedLicenceKey = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
	char* strippedLicenceKeyCRC = (char*)ULib.HeapLibrary.allocPrivateHeap(10);
	memcpy((void*)&strippedLicenceKey[0], (void*)&licenceKey[0], 5);
	memcpy((void*)&strippedLicenceKey[5], (void*)&licenceKey[9], 33);
	memcpy(strippedLicenceKeyCRC, (void*)&licenceKey[5], 4);

	// check for a valid crc value
	char* licenceString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(licenceString, "%s%s%s", strippedLicenceKey, licenceUserName, licenceCompanyName);
	short licenceCRCValue = generateLicenceCRCValue(licenceString);
	if (licenceCRCValue != (short)ULib.StringLibrary.getDWORDFromString(strippedLicenceKeyCRC, true)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// remove the crc value & key values from the licence key
	ZeroMemory(strippedLicenceKey, 100);
	memcpy((void*)&strippedLicenceKey[0], (void*)&licenceKey[0], 5);
	memcpy((void*)&strippedLicenceKey[5], (void*)&licenceKey[9], 5);
	memcpy((void*)&strippedLicenceKey[10], (void*)&licenceKey[16], 5);
	memcpy((void*)&strippedLicenceKey[15], (void*)&licenceKey[23], 19);

	// get the decoding values
	char* keyXORValue = (char*)ULib.HeapLibrary.allocPrivateHeap(5);
	char* keyUserValue = (char*)ULib.HeapLibrary.allocPrivateHeap(5);
	memcpy((void*)&keyXORValue[0], (void*)&licenceKey[14], 2);
	memcpy((void*)&keyUserValue[0], (void*)&licenceKey[21], 2);

	ULib.ConsoleLibrary.displayConsoleMessage("                    - 0x", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceCRCValue & 0xFFFF, true, false, ULib.ConsoleLibrary.DISPLAY_AS_WORD);
	ULib.ConsoleLibrary.displayConsoleMessage(", ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(keyXORValue, false);
	ULib.ConsoleLibrary.displayConsoleMessage(", ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(keyUserValue);

	// decode the licence key
	char* decodedLicenceKey = decodeLicenceKey(strippedLicenceKey, licenceUserName, licenceCompanyName, ULib.StringLibrary.getDWORDFromString(keyXORValue, false), ULib.StringLibrary.getDWORDFromString(keyUserValue, false));

	// display decoded licence key
	ULib.ConsoleLibrary.displayConsoleMessage("                    - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(decodedLicenceKey);

	// check decoded licence key
	if ((ULib.StringLibrary.getStringLength(decodedLicenceKey) != 17) ||
		((decodedLicenceKey[16] != 'X') && (decodedLicenceKey[16] != 'J') &&
		(decodedLicenceKey[16] != 'C') && (decodedLicenceKey[16] != 'D')))
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	}

	// display decoded licence key information
	ULib.ConsoleLibrary.displayConsoleMessage("\r\n[Licence Key Information]");

	bool isBrowserHawk = (((decodedLicenceKey[16] == 'X') || (decodedLicenceKey[16] == 'J')) ? true : false);
	bool isCountryHawk = (((decodedLicenceKey[16] == 'C') || (decodedLicenceKey[16] == 'D')) ? true : false);
	bool isActiveX = (((decodedLicenceKey[16] == 'X') || (decodedLicenceKey[16] == 'C')) ? true : false);
	bool isJava = (((decodedLicenceKey[16] == 'J') || (decodedLicenceKey[16] == 'D')) ? true : false);
	bool isStandardVersion = (ULib.StringLibrary.isStringMatch((char*)&decodedLicenceKey[1], "GUI", true, 3));
	bool isProfessionalVersion = (ULib.StringLibrary.isStringMatch((char*)&decodedLicenceKey[1], "SRV", true, 3));
	bool isEnterpriseVersion = (ULib.StringLibrary.isStringMatch((char*)&decodedLicenceKey[1], "ENT", true, 3));
	bool isTrialVersion = ((decodedLicenceKey[0] == 'D') ? true : false);
	bool isBDFSubscription = (((isBrowserHawk) && (decodedLicenceKey[13] == 'S')) ? true : false);
	unsigned char* tempBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();

	// product name
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Name      : ", false);
	if (isBrowserHawk) ULib.ConsoleLibrary.displayConsoleMessage("BrowserHawk");
	else if (isCountryHawk) ULib.ConsoleLibrary.displayConsoleMessage("CountryHawk");
	else ULib.ConsoleLibrary.displayConsoleMessage("");

	// product type
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Type      : ", false);
	if (isActiveX) ULib.ConsoleLibrary.displayConsoleMessage("ActiveX/COM/.NET");
	else if (isJava) ULib.ConsoleLibrary.displayConsoleMessage("Java");
	else ULib.ConsoleLibrary.displayConsoleMessage("");

	// product edition
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Edition   : ", false);
	if (isStandardVersion) ULib.ConsoleLibrary.displayConsoleMessage("Standard");
	else if (isProfessionalVersion) ULib.ConsoleLibrary.displayConsoleMessage("Professional");
	else if (isEnterpriseVersion) ULib.ConsoleLibrary.displayConsoleMessage("Enterprise");
	else ULib.ConsoleLibrary.displayConsoleMessage("");

	// product version
	char* productVersion = (char*)ULib.HeapLibrary.allocPrivateHeap(10);
	int productVersionValue = ULib.StringLibrary.getBYTEFromString((char*)&decodedLicenceKey[14], true, 2);
	wsprintf(productVersion, "v%d.%02d", productVersionValue / 0x10, productVersionValue % 0x10);
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Version   : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(productVersion);
	ULib.HeapLibrary.freePrivateHeap(productVersion);

	// expiry date
	char* expiryDate = (char*)ULib.HeapLibrary.allocPrivateHeap(20);
	int expiryDateDay = ULib.StringLibrary.getBYTEFromString((char*)&decodedLicenceKey[9], false, 2);
	int expiryDateMonth = ULib.StringLibrary.getBYTEFromString((char*)&decodedLicenceKey[7], false, 2);
	int expiryDateYear = ULib.StringLibrary.getBYTEFromString((char*)&decodedLicenceKey[11], false, 2);

	// trial version?
	if (isTrialVersion)
	{
		wsprintf(expiryDate, "%02d/%02d/%d%02d", expiryDateDay, expiryDateMonth, ((expiryDateYear < 90) ? 20 : 19), expiryDateYear);
		ULib.ConsoleLibrary.displayConsoleMessage("  Trial Expiry Date : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(expiryDate);
	}
	// BDF subscription?
	if (isBDFSubscription)
	{
		if (!isTrialVersion)
		{
			expiryDateYear++;
			expiryDateYear %= 100;
		}

		wsprintf(expiryDate, "%02d/%02d/%d%02d", expiryDateDay, expiryDateMonth, ((expiryDateYear < 90) ? 20 : 19), expiryDateYear);
		ULib.ConsoleLibrary.displayConsoleMessage("  BDF Expiry Date   : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(expiryDate);
	}

	ULib.HeapLibrary.freePrivateHeap(decodedLicenceKey);
	ULib.HeapLibrary.freePrivateHeap(expiryDate);
	ULib.HeapLibrary.freePrivateHeap(tempBuffer);
	ULib.HeapLibrary.freePrivateHeap(keyUserValue);
	ULib.HeapLibrary.freePrivateHeap(keyXORValue);
	ULib.HeapLibrary.freePrivateHeap(licenceString);
	ULib.HeapLibrary.freePrivateHeap(strippedLicenceKey);
	ULib.HeapLibrary.freePrivateHeap(strippedLicenceKeyCRC);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	ULib.HeapLibrary.freePrivateHeap(licenceUserName);
	ULib.HeapLibrary.freePrivateHeap(licenceCompanyName);
	ULib.HeapLibrary.freePrivateHeap(licenceKey);
}

/////////////////////////////////////////////////////////////
// creates a new licence based on the supplied information //
/////////////////////////////////////////////////////////////
void createLicence(char* licenceFilename)
{
	char* licenceUserName = NULL;
	char* licenceCompanyName = NULL;
	char* licenceProductVersion = NULL;
	char* licenceExpiryDate = (char*)ULib.HeapLibrary.allocPrivateHeap();

	bool isBrowserHawk = false;
	bool isCountryHawk = false;
	bool isActiveX = false;
	bool isJava = false;
	bool isStandardVersion = false;
	bool isProfessionalVersion = false;
	bool isEnterpriseVersion = false;
	bool isTrialVersion = false;
	bool isBDFSubscription = false;

	// get the required information
	ULib.ConsoleLibrary.displayConsoleMessage("[Enter Required Information]");

	// user name
	ULib.ConsoleLibrary.displayConsoleMessage("  User Name         : ", false);
	licenceUserName = (char*)ULib.ConsoleLibrary.getConsoleInput(30);

	// company name
	ULib.ConsoleLibrary.displayConsoleMessage("  Company Name      : ", false);
	licenceCompanyName = (char*)ULib.ConsoleLibrary.getConsoleInput(30);

	// product name
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Name      :                           [(B)rowserHawk, (C)ountryHawk]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 56, false);
	if ((!isBrowserHawk) && (!isCountryHawk))
	{
		unsigned char licenceProductName = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "BbCc");
		isBrowserHawk = ((licenceProductName == 'B') || (licenceProductName == 'b'));
		isCountryHawk = ((licenceProductName == 'C') || (licenceProductName == 'c'));
	}
	// display product name
	if (isBrowserHawk) ULib.ConsoleLibrary.displayConsoleMessage("BrowserHawk                                             ");
	else if (isCountryHawk) ULib.ConsoleLibrary.displayConsoleMessage("CountryHawk                                             ");
	else ULib.ConsoleLibrary.displayConsoleMessage(' ', 56, true);

	// product type
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Type      :                             [(A)ctiveX/COM/.NET, (J)ava]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 56, false);
	if ((!isActiveX) && (!isJava))
	{
		unsigned char licenceProductType = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "AaJj");
		isActiveX = ((licenceProductType == 'A') || (licenceProductType == 'a'));
		isJava = ((licenceProductType == 'J') || (licenceProductType == 'j'));
	}
	// display product type
	if (isActiveX) ULib.ConsoleLibrary.displayConsoleMessage("ActiveX/COM/.NET                                        ");
	else if (isJava) ULib.ConsoleLibrary.displayConsoleMessage("Java                                                    ");
	else ULib.ConsoleLibrary.displayConsoleMessage(' ', 56, true);

	// product edition
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Edition   :               [(S)tandard, (P)rofessional, (E)nterprise]", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 56, false);
	if ((!isStandardVersion) && (!isProfessionalVersion) && (!isEnterpriseVersion))
	{
		unsigned char licenceProductEdition = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "SsPpEe");
		isStandardVersion = ((licenceProductEdition == 'S') || (licenceProductEdition == 's'));
		isProfessionalVersion = ((licenceProductEdition == 'P') || (licenceProductEdition == 'p'));
		isEnterpriseVersion = ((licenceProductEdition == 'E') || (licenceProductEdition == 'e'));
	}
	// display product edition
	if (isStandardVersion) ULib.ConsoleLibrary.displayConsoleMessage("Standard                                                ");
	else if (isProfessionalVersion) ULib.ConsoleLibrary.displayConsoleMessage("Professional                                            ");
	else if (isEnterpriseVersion) ULib.ConsoleLibrary.displayConsoleMessage("Enterprise                                              ");
	else ULib.ConsoleLibrary.displayConsoleMessage(' ', 56, true);

	// product version
	ULib.ConsoleLibrary.displayConsoleMessage("  Product Version   : v", false);
	licenceProductVersion = (char*)ULib.ConsoleLibrary.getConsoleInputAsFormattedString("HH~~~", "\?\?.00", false, true);

	// is trial version?
	ULib.ConsoleLibrary.displayConsoleMessage("  Trial Version?    : ", false);
	isTrialVersion = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
	ULib.ConsoleLibrary.displayConsoleMessage(((isTrialVersion) ? "Yes" : "No"));

	// trial expiry date
	if (isTrialVersion)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("  Trial Expiry Date : ", false);
		unsigned char* trialVersionBufferString = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11~11~11", "dd/mm/yy", true, true, NULL, false, "1-31|1-12|0-99");

		// month
		licenceExpiryDate[2] = trialVersionBufferString[0];
		licenceExpiryDate[3] = trialVersionBufferString[1];
		// day
		licenceExpiryDate[0] = trialVersionBufferString[3];
		licenceExpiryDate[1] = trialVersionBufferString[4];
		// year
		licenceExpiryDate[4] = trialVersionBufferString[6];
		licenceExpiryDate[5] = trialVersionBufferString[7];
		ULib.HeapLibrary.freePrivateHeap(trialVersionBufferString);
	}

	// for browserhawk editor only
	if (isBrowserHawk)
	{
		// bdf subscription?
		ULib.ConsoleLibrary.displayConsoleMessage("  BDF Subscription? : ", false);
		isBDFSubscription = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
		ULib.ConsoleLibrary.displayConsoleMessage(((isBDFSubscription) ? "Yes" : "No"));

		// bdf subscription expiry date (if not trial version)
		if ((isBDFSubscription) && (!isTrialVersion))
		{
			ULib.ConsoleLibrary.displayConsoleMessage("  BDF Expiry Date   : ", false);
			unsigned char* bdfSubscriptionBufferString = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11~11~11", "dd/mm/yy", true, true, NULL, false, "1-31|1-12|0-99");

			// month
			licenceExpiryDate[2] = bdfSubscriptionBufferString[0];
			licenceExpiryDate[3] = bdfSubscriptionBufferString[1];
			// day
			licenceExpiryDate[0] = bdfSubscriptionBufferString[3];
			licenceExpiryDate[1] = bdfSubscriptionBufferString[4];
			// year
			licenceExpiryDate[4] = bdfSubscriptionBufferString[6];
			licenceExpiryDate[5] = bdfSubscriptionBufferString[7];
			ULib.HeapLibrary.freePrivateHeap(bdfSubscriptionBufferString);

			// decrement the expiry date by one year
			if (licenceExpiryDate[5] >= '1')
			{
				licenceExpiryDate[5]--;
			}
			else
			{
				if (licenceExpiryDate[4] >= '1') licenceExpiryDate[4]--;
				else licenceExpiryDate[4] = '9';
				licenceExpiryDate[5] = '9';
			}
		}
	}

	// if no expiry date has been set
	if ((!isBDFSubscription) && (!isTrialVersion)) ULib.StringLibrary.copyString(licenceExpiryDate, "010101");
	ULib.ConsoleLibrary.displayConsoleMessage("");

	// create the decoded licence key
	char* decodedLicenceKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	// trial or full key
	decodedLicenceKey[0] = ((isTrialVersion) ? 'D' : 'P');
	// product edition
	if (isStandardVersion) ULib.StringLibrary.copyString((char*)&decodedLicenceKey[1], "GUI");
	else if (isProfessionalVersion) ULib.StringLibrary.copyString((char*)&decodedLicenceKey[1], "SRV");
	else if (isEnterpriseVersion) ULib.StringLibrary.copyString((char*)&decodedLicenceKey[1], "ENT");
	// trial or full key
	ULib.StringLibrary.copyString((char*)&decodedLicenceKey[4], ((isTrialVersion) ? "036" : "864"));
	// expiry date
	ULib.StringLibrary.copyString((char*)&decodedLicenceKey[7], licenceExpiryDate);
	// bdf subscription?
	decodedLicenceKey[13] = ((isBDFSubscription) ? 'S' : 'N');
	// product version
	wsprintf((char*)&decodedLicenceKey[14], "%X", ULib.StringLibrary.getDWORDFromString(licenceProductVersion, false));
	decodedLicenceKey[15] = '0';
	// product type
	if (isBrowserHawk)
	{
		if (isActiveX) decodedLicenceKey[16] = 'X';
		else if (isJava) decodedLicenceKey[16] = 'J';
	}
	else if (isCountryHawk)
	{
		if (isActiveX) decodedLicenceKey[16] = 'C';
		else if (isJava) decodedLicenceKey[16] = 'D';
	}

	// check licence properties
	int licenceUserNameLength = ULib.StringLibrary.getStringLength(licenceUserName);
	int licenceCompanyNameLength = ULib.StringLibrary.getStringLength(licenceCompanyName);
	if ((licenceUserNameLength <= 1) || (licenceCompanyNameLength <= 1) || (ULib.StringLibrary.getStringLength(decodedLicenceKey) != 17)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);

	int keyXORValue = licenceUserNameLength + licenceCompanyNameLength;
	int keyUserValue = licenceUserNameLength;

	// encode licence key
	char* encodedLicenceKey = encodeLicenceKey(decodedLicenceKey, licenceUserName, licenceCompanyName, keyXORValue, keyUserValue);
	if (ULib.StringLibrary.getStringLength(encodedLicenceKey) != 34) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);

	// add key values to encoded key
	char* encodedLicenceKeyWithKeyValues = (char*)ULib.HeapLibrary.allocPrivateHeap();
	memcpy((void*)&encodedLicenceKeyWithKeyValues[0], (void*)&encodedLicenceKey[0], 10);
	wsprintf((char*)&encodedLicenceKeyWithKeyValues[10], "%02d", keyXORValue);
	memcpy((void*)&encodedLicenceKeyWithKeyValues[12], (void*)&encodedLicenceKey[10], 5);
	wsprintf((char*)&encodedLicenceKeyWithKeyValues[17], "%02d", keyUserValue);
	memcpy((void*)&encodedLicenceKeyWithKeyValues[19], (void*)&encodedLicenceKey[15], 19);

	// generate the crc value
	char* licenceString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(licenceString, "%s%s%s", encodedLicenceKeyWithKeyValues, licenceUserName, licenceCompanyName);
	short licenceCRCValue = generateLicenceCRCValue(licenceString);
	ULib.HeapLibrary.freePrivateHeap(licenceString);

	// add crc value to encoded key
	char* encodedLicenceKeyComplete = (char*)ULib.HeapLibrary.allocPrivateHeap();
	memcpy((void*)&encodedLicenceKeyComplete[0], (void*)&encodedLicenceKeyWithKeyValues[0], 5);
	wsprintf((char*)&encodedLicenceKeyComplete[5], "%04X", licenceCRCValue & 0xFFFF);
	memcpy((void*)&encodedLicenceKeyComplete[9], (void*)&encodedLicenceKeyWithKeyValues[5], 33);

	// display licence properties
	ULib.ConsoleLibrary.displayConsoleMessage("[Encoded Licence Information]");

	char* licenceFilenameStripped = ULib.FileLibrary.stripPathFromFilename(licenceFilename, true);
	ULib.ConsoleLibrary.displayConsoleMessage("  Licence Filename  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceFilenameStripped);
	ULib.HeapLibrary.freePrivateHeap(licenceFilenameStripped);

	ULib.ConsoleLibrary.displayConsoleMessage("  User Name         : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceUserName);

	ULib.ConsoleLibrary.displayConsoleMessage("  Company Name      : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceCompanyName);

	ULib.ConsoleLibrary.displayConsoleMessage("  Licence Key       : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(decodedLicenceKey);

	ULib.ConsoleLibrary.displayConsoleMessage("                    - 0x", false);
	ULib.ConsoleLibrary.displayConsoleMessage(licenceCRCValue & 0xFFFF, true, false, ULib.ConsoleLibrary.DISPLAY_AS_WORD);
	ULib.ConsoleLibrary.displayConsoleMessage(", ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(keyXORValue, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
	ULib.ConsoleLibrary.displayConsoleMessage(", ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(keyUserValue, false, true, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);

	ULib.ConsoleLibrary.displayConsoleMessage("                    - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(encodedLicenceKeyComplete);

	// create serial number
	char* licenceSerialNumber = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned long tickCount = GetTickCount();
	if (tickCount > 999999) tickCount /= 10000;
	wsprintf(licenceSerialNumber, "%si%c0-%06d", ((isBrowserHawk) ? ((isActiveX) ? "bhax" : "bh4j") : ((isCountryHawk) ? ((isActiveX) ? "chax" : "ch4j") : "xx")), ((licenceProductVersion != NULL) ? licenceProductVersion[0] : '0'), tickCount);

	// create the new licence file
	char* licenceFileBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(licenceFileBuffer, "\"%s\"\r\n\"%s\"\r\n\"%s\"\r\n\"%s\"\r\n", licenceUserName, licenceCompanyName, encodedLicenceKeyComplete, licenceSerialNumber);
	if (!ULib.FileLibrary.writeBufferToFile(licenceFilename, (unsigned char*)licenceFileBuffer, ULib.StringLibrary.getStringLength(licenceFileBuffer))) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);

	ULib.HeapLibrary.freePrivateHeap(licenceUserName);
	ULib.HeapLibrary.freePrivateHeap(licenceCompanyName);
	ULib.HeapLibrary.freePrivateHeap(licenceProductVersion);
	ULib.HeapLibrary.freePrivateHeap(licenceFileBuffer);
	ULib.HeapLibrary.freePrivateHeap(licenceSerialNumber);
	ULib.HeapLibrary.freePrivateHeap(encodedLicenceKeyComplete);
	ULib.HeapLibrary.freePrivateHeap(encodedLicenceKeyWithKeyValues);
	ULib.HeapLibrary.freePrivateHeap(licenceExpiryDate);
	ULib.HeapLibrary.freePrivateHeap(decodedLicenceKey);
	ULib.HeapLibrary.freePrivateHeap(encodedLicenceKey);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("cyScape Licence Key Utility", "1.00", 2002);
	bool displayUsage = true;

	if (argc > 1)
	{
		displayUsage = false;
		// create new licence key
		if ((argc > 2) && ((ULib.StringLibrary.isStringMatch(argv[2], "/createLicence")) || (ULib.StringLibrary.isStringMatch(argv[2], "/createLicense")))) createLicence(argv[1]);
		// display licence information
		else displayLicenceInformation(argv[1]);
	}

	// display usage
	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : cyScapeKey <licence filename> [/createLicence]");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}