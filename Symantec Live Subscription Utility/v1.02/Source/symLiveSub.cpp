///////////////////////////////////////////////////////////////////
// Symantec Live Subscription Utility                            //
// (C)thewd, thewd@hotmail.com                                   //
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// Display the subscription information for registered products. //
// Can also update this information to extend the subscription   //
//                                                               //
// v2.02.1147+ modules use LiveReg instead of LiveAdvisor and    //
// when first run will convert the registry based subscription   //
// services to the file based catalog (Catalog.LiveSubscribe)    //
//                                                               //
// Tested:                                                       //
// - v1.00.0564, v1.00.0579, v1.00.0621, v1.00.0706, v1.00.0777  //
// - v2.00.0840, v2.00.0963, v2.02.1147, v2.06.1314              //
// - v2.10.1419, v2.15.1502                                      //
// - v2.20.1621                                                  //
// - v2.40.1902, v2.40.1995, v2.40.2003, v2.40.2044, v2.41.2056  //
// - v3.00.5                                                     //
///////////////////////////////////////////////////////////////////
#include <windows.h>
#include <time.h>
#include "..\..\..\_Shared Libraries\OpenSSL_BlowfishLibrary.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"{CRLF}Unable to open the Live Subscription Services registry key",
	"{CRLF}Unable to find any Live Subscription Services within the registry",
	"{CRLF}Unable to update the Live Subscription Service information",
	"",
	"",
	"{CRLF}Unable to open or read the Live Subscription Services catalog",
	"{CRLF}Unable to find any Live Subscription Services within the catalog",
	"{CRLF}An error occurred while attemping to decrypt the catalog",
	"{CRLF}An error occurred while attemping to encrypt the catalog",
	"{CRLF}An error occurred while attemping to backup and update the catalog",
	"{CRLF}An error occurred while enumerating the Live Subscription Services"
};

/////////////////////////////////////////////////////////////
// returns the checksum value for the supplied data stream //
/////////////////////////////////////////////////////////////
unsigned long getServiceDataChecksum(unsigned char* dataStream)
{
	unsigned long checksumValue = 0x12A512D;
	checksumValue += ULib.StringLibrary.getDWORDFromBuffer(dataStream, 0x00);
	checksumValue += ULib.StringLibrary.getDWORDFromBuffer(dataStream, 0x04);
	checksumValue += ULib.StringLibrary.getDWORDFromBuffer(dataStream, 0x08);
	checksumValue += ULib.StringLibrary.getDWORDFromBuffer(dataStream, 0x0C);
	return checksumValue;
}

////////////////////////////////////////////////////////////////////////
// display the key registry entries and allow the dates to be changed //
// (i.e. Norton AntiVirus 2000/2001 & Norton Internet Security 2000)  //
////////////////////////////////////////////////////////////////////////
bool handleRegistryService(char* serviceName)
{
	// open service key
	char* serviceKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(serviceKey, "Software\\Symantec\\LiveSubscribe\\Services\\%s", serviceName);
	HKEY hKey = ULib.RegistryLibrary.openRegistrySubKey(HKEY_LOCAL_MACHINE, serviceKey);
	ULib.HeapLibrary.freePrivateHeap(serviceKey);
	if (hKey == NULL) return false;

	// get sku
	char* serviceSKU = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "Subscription");
	RegCloseKey(hKey);
	if (serviceSKU == NULL) return false;

	// open service-sku key
	char* serviceSKUKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(serviceSKUKey, "Software\\Symantec\\LiveSubscribe\\Services\\%s\\%s", serviceName, serviceSKU);
	hKey = ULib.RegistryLibrary.openRegistrySubKey(HKEY_LOCAL_MACHINE, serviceSKUKey);
	ULib.HeapLibrary.freePrivateHeap(serviceSKUKey);
	if (hKey == NULL) return false;

	// get service information
	unsigned long serviceDataLength = 0;
	unsigned char* serviceData = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "DATA", &serviceDataLength, NULL);
	char* productName = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "PRODUCT_NAME");
	char* productVersion = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "PRODUCT_VERSION");
	char* serviceDescription = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "SERVICE_NAME");
	char* subscriptionLength = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "SUBSCRIPTION_LENGTH");
	char* productSKU = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "PRODUCT_SKU");
	char* partNumber = ULib.RegistryLibrary.getRegistryKeyValueString(hKey, "PART_NUMBER");
	if ((productName == NULL) || (productVersion == NULL) || (serviceDescription == NULL) || (subscriptionLength == NULL) || (serviceData == NULL) || (serviceDataLength != 0x14)) return false;
	if (getServiceDataChecksum(serviceData) != ULib.StringLibrary.getDWORDFromBuffer(serviceData, 0x10)) return false;

	// get subscription information
	char* beginDate = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* endDate = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* serviceDateFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(beginDate, "%lu", ULib.StringLibrary.getDWORDFromBuffer(serviceData));
	wsprintf(endDate, "%lu", ULib.StringLibrary.getDWORDFromBuffer(serviceData, 0x08));
	wsprintf(serviceDateFormatted, "%02d/%02d/%04d to %02d/%02d/%04d", ULib.StringLibrary.getBYTEFromString((char*)&beginDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&beginDate[4], false, 2), ULib.StringLibrary.getWORDFromString(beginDate, false, 4), ULib.StringLibrary.getBYTEFromString((char*)&endDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&endDate[4], false, 2), ULib.StringLibrary.getWORDFromString(endDate, false, 4));

	// display information
	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("[", false);
	ULib.ConsoleLibrary.displayConsoleMessage(productName, false);
	ULib.ConsoleLibrary.displayConsoleMessage(" ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(productVersion, false);
	ULib.ConsoleLibrary.displayConsoleMessage("]");
	ULib.ConsoleLibrary.displayConsoleMessage("+ Description   : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(serviceDescription, true);
	if (productSKU != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Product SKU   : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(productSKU, true);
	}
	if (partNumber != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Part Number   : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(partNumber, true);
	}
	ULib.ConsoleLibrary.displayConsoleMessage("+ Subscription  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(serviceDateFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage("- Update Subscription Information?", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);

	// update subscription information
	if (ULib.ConsoleLibrary.getConsoleInputAsYesNo(false))
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Update Dates  : ", false);
		unsigned char* updatedServiceDate = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11~11~1111~~~~11~11~1111", "dd/mm/yyyy to dd/mm/yyyy", true, true, NULL, false, "1-31|1-12|1900-2100|1-31|1-12|1900-2100");

		// copy entered information to date strings
		memcpy((char*)&beginDate[0], (char*)&updatedServiceDate[6], 4);
		memcpy((char*)&beginDate[4], (char*)&updatedServiceDate[3], 2);
		memcpy((char*)&beginDate[6], (char*)&updatedServiceDate[0], 2);
		memcpy((char*)&endDate[0], (char*)&updatedServiceDate[20], 4);
		memcpy((char*)&endDate[4], (char*)&updatedServiceDate[17], 2);
		memcpy((char*)&endDate[6], (char*)&updatedServiceDate[14], 2);

		// create time structures based on entered information
		struct tm* installTM = (tm*)ULib.HeapLibrary.allocPrivateHeap(sizeof(tm));
		installTM->tm_year = ULib.StringLibrary.getWORDFromString((char*)&updatedServiceDate[6], false, 4) - 1900;
		installTM->tm_mon = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[3], false, 2) - 1;
		installTM->tm_mday = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[0], false, 2);
		struct tm* expiryTM = (tm*)ULib.HeapLibrary.allocPrivateHeap(sizeof(tm));
		expiryTM->tm_year = ULib.StringLibrary.getWORDFromString((char*)&updatedServiceDate[20], false, 4) - 1900;
		expiryTM->tm_mon = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[17], false, 2) - 1;
		expiryTM->tm_mday = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[14], false, 2);
		ULib.HeapLibrary.freePrivateHeap(updatedServiceDate);

		// backup existing entries
		char* backupData = ULib.FileLibrary.getBackupFilename("DATA", "");
		char* backupSubscriptionLength = ULib.FileLibrary.getBackupFilename("SUBSCRIPTION_LENGTH", "");
		RegSetValueEx(hKey, backupData, 0, REG_BINARY, serviceData, 0x14);
		RegSetValueEx(hKey, backupSubscriptionLength, 0, REG_SZ, (unsigned char*)subscriptionLength, ULib.StringLibrary.getStringLength(subscriptionLength));
		ULib.HeapLibrary.freePrivateHeap(backupData);
		ULib.HeapLibrary.freePrivateHeap(backupSubscriptionLength);

		// update data stream
		ULib.StringLibrary.addDWORDToBuffer(serviceData, ULib.StringLibrary.getDWORDFromString(beginDate), 0x00);
		ULib.StringLibrary.addDWORDToBuffer(serviceData, ULib.StringLibrary.getDWORDFromString(beginDate), 0x04);
		ULib.StringLibrary.addDWORDToBuffer(serviceData, ULib.StringLibrary.getDWORDFromString(endDate), 0x08);
		ULib.StringLibrary.addDWORDToBuffer(serviceData, ULib.StringLibrary.getDWORDFromString(beginDate), 0x0C);
		ULib.StringLibrary.addDWORDToBuffer(serviceData, getServiceDataChecksum(serviceData), 0x10);
		// get number of days
		ULib.StringLibrary.copyString(subscriptionLength, ULib.StringLibrary.getStringFromNumber((DWORD)(difftime(mktime(expiryTM), mktime(installTM)) / 86400), false));
		// update subscription information
		if (RegSetValueEx(hKey, "DATA", 0, REG_BINARY, serviceData, 0x14) != ERROR_SUCCESS) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		if (RegSetValueEx(hKey, "SUBSCRIPTION_LENGTH", 0, REG_SZ, (unsigned char*)subscriptionLength, ULib.StringLibrary.getStringLength(subscriptionLength)) != ERROR_SUCCESS) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		ULib.HeapLibrary.freePrivateHeap(installTM);
		ULib.HeapLibrary.freePrivateHeap(expiryTM);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 34, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);
	}

	RegCloseKey(hKey);
	ULib.HeapLibrary.freePrivateHeap(serviceData);
	ULib.HeapLibrary.freePrivateHeap(subscriptionLength);
	ULib.HeapLibrary.freePrivateHeap(productName);
	ULib.HeapLibrary.freePrivateHeap(productVersion);
	ULib.HeapLibrary.freePrivateHeap(serviceDescription);
	ULib.HeapLibrary.freePrivateHeap(beginDate);
	ULib.HeapLibrary.freePrivateHeap(endDate);
	ULib.HeapLibrary.freePrivateHeap(serviceDateFormatted);
	return true;
}

////////////////////////////////////////////////////////////////////////////
// check the registry for the old antivirus subscription services (v5.00) //
////////////////////////////////////////////////////////////////////////////
bool handleRegistryOldAntiVirusService(void)
{
	// open antvirus key
	HKEY hKey = ULib.RegistryLibrary.openRegistrySubKey(HKEY_LOCAL_MACHINE, "Software\\Symantec\\Norton AntiVirus\\DefAnnuity");
	if (hKey == NULL) return false;

	// get free start date and antivirus version
	unsigned char* serviceFreeStartString = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "FreeStart", NULL, NULL);
	unsigned char* serviceVersionString = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "Version", NULL, NULL);
	if ((serviceFreeStartString == NULL) || (serviceVersionString == NULL)) return false;

	unsigned long serviceFreeStart = ULib.StringLibrary.getDWORDFromBuffer(serviceFreeStartString);
	unsigned long serviceVersion = ((ULib.StringLibrary.getDWORDFromBuffer(serviceVersionString) >> 24) & 0xFF);
	if ((serviceFreeStart <= 0) || (serviceVersion <= 0)) return false;

	char* beginDate = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* serviceDateFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(beginDate, "%lu", serviceFreeStart);
	wsprintf(serviceDateFormatted, "%02d/%02d/%04d to %02d/%02d/%04d", ULib.StringLibrary.getBYTEFromString((char*)&beginDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&beginDate[4], false, 2), ULib.StringLibrary.getWORDFromString(beginDate, false, 4), ULib.StringLibrary.getBYTEFromString((char*)&beginDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&beginDate[4], false, 2), ULib.StringLibrary.getWORDFromString(beginDate, false, 4) + 1);

	// display information
	ULib.ConsoleLibrary.displayConsoleMessage("Found Old AntiVirus Subscription Service...");
	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("[Norton AntiVirus ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(serviceVersion, true, false);
	ULib.ConsoleLibrary.displayConsoleMessage(".00]");
	ULib.ConsoleLibrary.displayConsoleMessage("+ Subscription  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(serviceDateFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage("- Update Subscription Information?", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);

	// update subscription information
	if (ULib.ConsoleLibrary.getConsoleInputAsYesNo(false))
	{
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 34, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);
		ULib.ConsoleLibrary.displayConsoleMessage("+ Start Date    : ", false);
		unsigned char* updatedServiceDate = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11~11~1111~~~~~~~~~~~~~~~~~~~~~~", "dd/mm/yyyy (1 year subscription)", true, true, NULL, false, "1-31|1-12|1900-2100");

		// copy entered information to begin date
		memcpy((char*)&beginDate[0], (char*)&updatedServiceDate[6], 4);
		memcpy((char*)&beginDate[4], (char*)&updatedServiceDate[3], 2);
		memcpy((char*)&beginDate[6], (char*)&updatedServiceDate[0], 2);
		ULib.HeapLibrary.freePrivateHeap(updatedServiceDate);

		// backup existing entries
		char* backupFreeStart = ULib.FileLibrary.getBackupFilename("FreeStart", "");
		RegSetValueEx(hKey, backupFreeStart, 0, REG_DWORD, serviceFreeStartString, 0x04);
		ULib.HeapLibrary.freePrivateHeap(backupFreeStart);

		// update subscription information
		serviceFreeStart = ULib.StringLibrary.getDWORDFromString(beginDate);
		if (RegSetValueEx(hKey, "FreeStart", 0, REG_DWORD, (LPBYTE)&serviceFreeStart, 0x04) != ERROR_SUCCESS) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 34, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);
	}

	RegCloseKey(hKey);
	ULib.HeapLibrary.freePrivateHeap(serviceDateFormatted);
	ULib.HeapLibrary.freePrivateHeap(beginDate);
	ULib.HeapLibrary.freePrivateHeap(serviceFreeStartString);
	ULib.HeapLibrary.freePrivateHeap(serviceVersionString);
	return true;
}

///////////////////////////////////////////////////////////////////////
// parse the registry, looking for live subscription service entries //
///////////////////////////////////////////////////////////////////////
bool parseRegistry(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Opening Live Subscription Services Key...");

	// check for old antivirus subscription services
	bool handledOldAntiVirusService = handleRegistryOldAntiVirusService();

	// open main services key
	HKEY hKey = ULib.RegistryLibrary.openRegistrySubKey(HKEY_LOCAL_MACHINE, "Software\\Symantec\\LiveSubscribe\\Services");
	if (hKey == NULL)
	{
		if (handledOldAntiVirusService) return true;
		else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	}

	bool foundService = false;
	int dwIndex = 0;
	char* keyName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned long keyNameLength = ULib.HeapLibrary.DEFAULT_ALLOC_SIZE;
	FILETIME ftLastWriteTime;

	if (handledOldAntiVirusService) ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("Enumerating Live Subscription Services...");

	// enumerate the service entries
	while (RegEnumKeyEx(hKey, dwIndex, keyName, &keyNameLength, NULL, NULL, NULL, &ftLastWriteTime) == ERROR_SUCCESS)
	{
		if (handleRegistryService(keyName)) foundService = true;
		keyNameLength = ULib.HeapLibrary.DEFAULT_ALLOC_SIZE;
		dwIndex++;
	}

	RegCloseKey(hKey);
	if (!foundService)
	{
		if (handledOldAntiVirusService) return true;
		else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	return true;
}

/////////////////////////////////////////////////////////
// handles the blowfish decryption of the catalog file //
/////////////////////////////////////////////////////////
bool handleFileDecryption(unsigned char* fileBuffer, unsigned long fileSize, unsigned char* pBox, unsigned char* sBox)
{
	BF_KEY* bfKey = (BF_KEY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(bf_key_st));
	memcpy(bfKey->P, pBox, 0x48);
	memcpy(bfKey->S, sBox, 0x1000);
	for (unsigned long i = 0x04; i < fileSize; i += 8) BF_decrypt((BF_LONG*)&fileBuffer[i], bfKey);
	ULib.HeapLibrary.freePrivateHeap(bfKey);

	// check decryption succeeded
	unsigned long crcValue = 0;
	unsigned long catalogSize = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4);
	if (catalogSize > fileSize - 12) return false;
	for (unsigned long i = 4; i < catalogSize; i += 4) crcValue += ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, i);
	return (crcValue == ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, catalogSize + 4));
}

/////////////////////////////////////////////////////////
// handles the blowfish encryption of the catalog file //
/////////////////////////////////////////////////////////
bool handleFileEncryption(unsigned char* fileBuffer, unsigned long fileSize, unsigned char* pBox, unsigned char* sBox)
{
	BF_KEY* bfKey = (BF_KEY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(bf_key_st));
	memcpy(bfKey->P, pBox, 0x48);
	memcpy(bfKey->S, sBox, 0x1000);
	for (unsigned long i = 0x04; i < fileSize; i += 8) BF_encrypt((BF_LONG*)&fileBuffer[i], bfKey);
	ULib.HeapLibrary.freePrivateHeap(bfKey);

	// check encryption succeeded
	unsigned char* fileBufferCopy = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(fileSize);
	memcpy(fileBufferCopy, fileBuffer, fileSize);
	bool decryptionSucceeded = handleFileDecryption(fileBufferCopy, fileSize, pBox, sBox);
	ULib.HeapLibrary.freePrivateHeap(fileBufferCopy);
	return decryptionSucceeded;
}

// catalog file buffer
unsigned char* fileBuffer = NULL;

///////////////////////////////////////////////////////////////////////
// display the key service entries and allow the dates to be changed //
// (i.e. Norton AntiVirus 2002+ & Norton Internet Security 2002+)    //
///////////////////////////////////////////////////////////////////////
bool handleFileService(unsigned long* fileSize, unsigned long* fileOffset, bool* fileUpdated)
{
	char* productName = NULL;
	char* productVersion = NULL;
	char* serviceDescription = NULL;
	char* subscriptionLength = NULL;
	char* beginDate = NULL;
	char* endDate = NULL;
	char* activationDate = NULL;
	char* isActivated = NULL;
	char* productSKU = NULL;
	char* partNumber = NULL;
	char* productType = NULL;
	int productNameLength = 0;
	int productVersionLength = 0;
	int serviceDescriptionLength = 0;
	int subscriptionLengthLength = 0;
	int beginDateLength = 0;
	int endDateLength = 0;
	int activationDateLength = 0;
	int isActivatedLength = 0;
	int productSKULength = 0;
	int partNumberLength = 0;
	int productTypeLength = 0;

	// skip service id
	*fileOffset += 0x26;
	unsigned long numberOfEntries = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, *fileOffset);
	*fileOffset += 0x04;

	__try
	{
		// enumerate the parameters, looking for the required ones
		for (unsigned long i = 0; i < numberOfEntries; i++)
		{
			int nameLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, *fileOffset);
			int valueLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, *fileOffset + nameLength + 2);
			*fileOffset += 2;
			if ((*fileOffset + nameLength) >= *fileSize) return false;

			if ((nameLength == 12) && (memcmp((char*)&fileBuffer[*fileOffset], "PRODUCT_NAME", nameLength) == 0))
			{
				productName = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				productNameLength = valueLength;
			}
			else if ((nameLength == 15) && (memcmp((char*)&fileBuffer[*fileOffset], "PRODUCT_VERSION", nameLength) == 0))
			{
				productVersion = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				productVersionLength = valueLength;
			}
			else if ((nameLength == 12) && (memcmp((char*)&fileBuffer[*fileOffset], "SERVICE_NAME", nameLength) == 0))
			{
				serviceDescription = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				serviceDescriptionLength = valueLength;
			}
			else if ((nameLength == 19) && (memcmp((char*)&fileBuffer[*fileOffset], "SUBSCRIPTION_LENGTH", nameLength) == 0))
			{
				subscriptionLength = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				subscriptionLengthLength = valueLength;
			}
			else if ((nameLength == 10) && (memcmp((char*)&fileBuffer[*fileOffset], "BEGIN_DATE", nameLength) == 0))
			{
				beginDate = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				beginDateLength = valueLength;
			}
			else if ((nameLength == 8) && (memcmp((char*)&fileBuffer[*fileOffset], "END_DATE", nameLength) == 0))
			{
				endDate = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				endDateLength = valueLength;
			}
			else if ((nameLength == 15) && (memcmp((char*)&fileBuffer[*fileOffset], "ACTIVATION_DATE", nameLength) == 0))
			{
				activationDate = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				activationDateLength = valueLength;
			}
			else if ((nameLength == 9) && (memcmp((char*)&fileBuffer[*fileOffset], "ACTIVATED", nameLength) == 0))
			{
				isActivated = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				isActivatedLength = valueLength;
			}
			else if ((nameLength == 11) && (memcmp((char*)&fileBuffer[*fileOffset], "PRODUCT_SKU", nameLength) == 0))
			{
				productSKU = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				productSKULength = valueLength;
			}
			else if ((nameLength == 11) && (memcmp((char*)&fileBuffer[*fileOffset], "PART_NUMBER", nameLength) == 0))
			{
				partNumber = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				partNumberLength = valueLength;
			}
			else if ((nameLength == 12) && (memcmp((char*)&fileBuffer[*fileOffset], "PRODUCT_TYPE", nameLength) == 0))
			{
				productType = (char*)&fileBuffer[*fileOffset + nameLength + 2];
				productTypeLength = valueLength;
			}

			*fileOffset += nameLength;
			*fileOffset += 2;
			*fileOffset += valueLength;
			if (*fileOffset >= *fileSize) return false;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		productName = NULL;
	}

	// validate the parsed items
	if ((productName == NULL) || (productVersion == NULL) || (serviceDescription == NULL) || (subscriptionLength == NULL) || (beginDate == NULL) || (endDate == NULL) || (activationDate == NULL)) return false;
	if ((beginDateLength != 8) || (endDateLength != 8) || (activationDateLength != 8)) return false;

	// get subscription dates
	char* serviceDateFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* activationDateFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(serviceDateFormatted, "%02d/%02d/%04d to %02d/%02d/%04d", ULib.StringLibrary.getBYTEFromString((char*)&beginDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&beginDate[4], false, 2), ULib.StringLibrary.getWORDFromString(beginDate, false, 4), ULib.StringLibrary.getBYTEFromString((char*)&endDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&endDate[4], false, 2), ULib.StringLibrary.getWORDFromString(endDate, false, 4));
	wsprintf(activationDateFormatted, " (%02d/%02d/%04d)", ULib.StringLibrary.getBYTEFromString((char*)&activationDate[6], false, 2), ULib.StringLibrary.getBYTEFromString((char*)&activationDate[4], false, 2), ULib.StringLibrary.getWORDFromString(activationDate, false, 4));

	// display information
	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("[", false);
	ULib.ConsoleLibrary.displayConsoleMessage(productName, productNameLength, false);
	ULib.ConsoleLibrary.displayConsoleMessage(" ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(productVersion, productVersionLength, false);
	ULib.ConsoleLibrary.displayConsoleMessage("]");
	ULib.ConsoleLibrary.displayConsoleMessage("+ Description   : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(serviceDescription, serviceDescriptionLength, true);
	if (productSKU != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Product SKU   : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(productSKU, productSKULength, true);
	}
	if (partNumber != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Part Number   : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(partNumber, partNumberLength, true);
	}
	if (productType != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("+ Product Type  : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(productType, productTypeLength, true);
	}
	ULib.ConsoleLibrary.displayConsoleMessage("+ Is Activated  : ", false);
	if (isActivated == NULL) ULib.ConsoleLibrary.displayConsoleMessage("Yes", false);
	else ULib.ConsoleLibrary.displayConsoleMessage(isActivated, isActivatedLength, false);
	ULib.ConsoleLibrary.displayConsoleMessage(activationDateFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage("+ Subscription  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(serviceDateFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage("- Update Subscription Information?", false);
	ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);

	// update subscription information
	if (ULib.ConsoleLibrary.getConsoleInputAsYesNo(false))
	{
		*fileUpdated = true;
		ULib.ConsoleLibrary.displayConsoleMessage("+ Update Dates  : ", false);
		unsigned char* updatedServiceDate = ULib.ConsoleLibrary.getConsoleInputAsFormattedString("11~11~1111~~~~11~11~1111", "dd/mm/yyyy to dd/mm/yyyy", true, true, NULL, false, "1-31|1-12|1900-2100|1-31|1-12|1900-2100");

		// copy entered information to date strings
		memcpy((char*)&beginDate[0], (char*)&updatedServiceDate[6], 4);
		memcpy((char*)&beginDate[4], (char*)&updatedServiceDate[3], 2);
		memcpy((char*)&beginDate[6], (char*)&updatedServiceDate[0], 2);
		memcpy((char*)&activationDate[0], (char*)&updatedServiceDate[6], 4);
		memcpy((char*)&activationDate[4], (char*)&updatedServiceDate[3], 2);
		memcpy((char*)&activationDate[6], (char*)&updatedServiceDate[0], 2);
		memcpy((char*)&endDate[0], (char*)&updatedServiceDate[20], 4);
		memcpy((char*)&endDate[4], (char*)&updatedServiceDate[17], 2);
		memcpy((char*)&endDate[6], (char*)&updatedServiceDate[14], 2);

		// create time structures based on entered information
		struct tm* installTM = (tm*)ULib.HeapLibrary.allocPrivateHeap(sizeof(tm));
		installTM->tm_year = ULib.StringLibrary.getWORDFromString((char*)&updatedServiceDate[6], false, 4) - 1900;
		installTM->tm_mon = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[3], false, 2) - 1;
		installTM->tm_mday = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[0], false, 2);
		struct tm* expiryTM = (tm*)ULib.HeapLibrary.allocPrivateHeap(sizeof(tm));
		expiryTM->tm_year = ULib.StringLibrary.getWORDFromString((char*)&updatedServiceDate[20], false, 4) - 1900;
		expiryTM->tm_mon = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[17], false, 2) - 1;
		expiryTM->tm_mday = ULib.StringLibrary.getBYTEFromString((char*)&updatedServiceDate[14], false, 2);
		ULib.HeapLibrary.freePrivateHeap(updatedServiceDate);

		// change product type from Trial to OEM
		if ((productType != NULL) && (ULib.StringLibrary.isStringMatch(productType, "Trial", false, productTypeLength))) ULib.StringLibrary.copyString(productType, "OEM\0\0", 5);

		// get number of days
		char* newSubscriptionLength = ULib.StringLibrary.getStringFromNumber((DWORD)(difftime(mktime(expiryTM), mktime(installTM)) / 86400), false);
		int newSubscriptionLengthLength = lstrlen(newSubscriptionLength);
		if (subscriptionLengthLength == newSubscriptionLengthLength) memcpy(subscriptionLength, newSubscriptionLength, newSubscriptionLengthLength);
		else
		{
			// adjust catalog to incorporate the new subscription length
			unsigned char* newFileBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(*fileSize + 0x10);
			unsigned long partLength = PtrToUlong(subscriptionLength) - PtrToUlong(fileBuffer);
			ULib.StringLibrary.addWORDToBuffer(fileBuffer, (WORD)newSubscriptionLengthLength, partLength - 2);
			memcpy(newFileBuffer, fileBuffer, partLength);
			memcpy((char*)&newFileBuffer[partLength], newSubscriptionLength, newSubscriptionLengthLength);
			memcpy((char*)&newFileBuffer[partLength + newSubscriptionLengthLength], (char*)&fileBuffer[partLength + subscriptionLengthLength], *fileSize - partLength - subscriptionLengthLength);

			// update file size, file offset
			unsigned long fileDifference = subscriptionLengthLength - newSubscriptionLengthLength;
			*fileSize -= fileDifference;
			*fileSize += 8 - ((*fileSize - 4) % 8);
			*fileOffset -= fileDifference;
			ULib.StringLibrary.addDWORDToBuffer(newFileBuffer, ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4) - fileDifference, 4);

			// realloc the fileBuffer and copy the new bytes
			fileBuffer = (unsigned char*)ULib.HeapLibrary.reallocPrivateHeap(fileBuffer, *fileSize);
			memcpy(fileBuffer, newFileBuffer, *fileSize);
			ULib.HeapLibrary.freePrivateHeap(newFileBuffer);
		}

		ULib.HeapLibrary.freePrivateHeap(installTM);
		ULib.HeapLibrary.freePrivateHeap(expiryTM);
		ULib.HeapLibrary.freePrivateHeap(newSubscriptionLength);

		// adjust catalog for changed subscription length
		/*unsigned char* fileBufferCopy = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(*fileSize);
		ULib.StringLibrary.copyBuffer(fileBufferCopy, fileBuffer, *fileSize);
		ZeroMemory(fileBuffer, *fileSize);

		// get length and adjustment values
		char* newSubscriptionLength = ULib.StringLibrary.getStringFromNumber((DWORD)(difftime(mktime(expiryTM), mktime(installTM)) / 86400), false);
		int newSubscriptionLengthLength = ULib.StringLibrary.getStringLength(newSubscriptionLength);
		unsigned long fileSizeDiff = newSubscriptionLengthLength - subscriptionLengthLength;
		unsigned long partLength = PtrToUlong(subscriptionLength) - PtrToUlong(fileBuffer);

		*fileSize += fileSizeDiff;
		// realloc buffer based on new size
		fileBuffer = (unsigned char*)ULib.HeapLibrary.reallocPrivateHeap(fileBuffer, *fileSize);
		// copy buffer before subscription length
		ULib.StringLibrary.copyBuffer(fileBuffer, fileBufferCopy, partLength - 2);
		// change subscription string length
		ULib.StringLibrary.addWORDToBuffer(fileBuffer, (WORD)newSubscriptionLengthLength, partLength - 2);
		// add subscription length
		ULib.StringLibrary.copyBuffer((unsigned char*)&fileBuffer[partLength], (unsigned char*)newSubscriptionLength, newSubscriptionLengthLength);
		// append the rest of the buffer
		ULib.StringLibrary.copyBuffer((unsigned char*)&fileBuffer[partLength + newSubscriptionLengthLength], (unsigned char*)&fileBufferCopy[partLength + subscriptionLengthLength], *fileSize - partLength);
		ULib.HeapLibrary.freePrivateHeap(newSubscriptionLength);
		ULib.HeapLibrary.freePrivateHeap(fileBufferCopy);

		ULib.HeapLibrary.freePrivateHeap(installTM);
		ULib.HeapLibrary.freePrivateHeap(expiryTM);*/
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 34, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 34, false);
	}

	ULib.HeapLibrary.freePrivateHeap(serviceDateFormatted);
	ULib.HeapLibrary.freePrivateHeap(activationDateFormatted);
	return true;
}

///////////////////////////////////////////////////////////////////
// parse the file, looking for live subscription service entries //
///////////////////////////////////////////////////////////////////
void parseFile(char* fileName, bool dumpFile)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Opening Live Subscription Catalog...");

	// open catalog file
	unsigned long fileSize = 0;
	fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize < 0x20)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);

	// read encryption boxes
	unsigned long pboxResourceSize = 0;
	unsigned long sboxResourceSize = 0;
	unsigned char* pboxResourceBuffer = ULib.FileLibrary.getResourceFromFile(NULL, "PBOX", RT_RCDATA, &pboxResourceSize);
	unsigned char* sboxResourceBuffer = ULib.FileLibrary.getResourceFromFile(NULL, "SBOX", RT_RCDATA, &sboxResourceSize);
	if ((pboxResourceBuffer == NULL) || (sboxResourceBuffer == NULL) || (pboxResourceSize != 72) || (sboxResourceSize != 4096)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 7);

	// do file decryption
	ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Live Subscription Catalog...");
	if (!handleFileDecryption(fileBuffer, fileSize, pboxResourceBuffer, sboxResourceBuffer)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 7);

	// dump file
	if (dumpFile)
	{
		char* dumpFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(dumpFilename, "%s.txt", fileName);
		char* dumpFilenameStripped = ULib.FileLibrary.stripPathFromFilename(dumpFilename, true);
		ULib.ConsoleLibrary.displayConsoleMessage("Dumping Live Subscription Catalog (", false);
		ULib.ConsoleLibrary.displayConsoleMessage(dumpFilenameStripped, false);
		ULib.ConsoleLibrary.displayConsoleMessage(")...");
		ULib.FileLibrary.writeBufferToFile(dumpFilename, fileBuffer, fileSize);
		ULib.HeapLibrary.freePrivateHeap(dumpFilename);
		ULib.HeapLibrary.freePrivateHeap(dumpFilenameStripped);
	}

	bool foundService = false;
	bool fileUpdated = false;
	unsigned long fileOffset = 0x20;
	unsigned long numberOfHeaderEntries = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x1C);

	ULib.ConsoleLibrary.displayConsoleMessage("Enumerating Live Subscription Services...");

	char* userId = NULL;
	char* userIdHeader = NULL;
	char* userPassword = NULL;
	char* userPasswordHeader = NULL;
	char* veniceId = NULL;
	char* veniceIdHeader = NULL;
	int userIdLength = 0;
	int userPasswordLength = 0;
	int veniceIdLength = 0;

	// skip catalog header
	__try
	{
		for (unsigned long i = 0; i < numberOfHeaderEntries; i++)
		{
			int nameLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, fileOffset);
			int valueLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, fileOffset + nameLength + 2);
			fileOffset += 2;
			if ((fileOffset + nameLength) >= fileSize) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 10);

			if ((nameLength == 7) && (memcmp((char*)&fileBuffer[fileOffset], "USER_ID", nameLength) == 0))
			{
				userIdHeader = (char*)&fileBuffer[fileOffset];
				userId = (char*)&fileBuffer[fileOffset + nameLength + 2];
				userIdLength = valueLength;
			}
			else if ((nameLength == 13) && (memcmp((char*)&fileBuffer[fileOffset], "USER_PASSWORD", nameLength) == 0))
			{
				userPasswordHeader = (char*)&fileBuffer[fileOffset];
				userPassword = (char*)&fileBuffer[fileOffset + nameLength + 2];
				userPasswordLength = valueLength;
			}
			else if ((nameLength == 9) && (memcmp((char*)&fileBuffer[fileOffset], "VENICE_ID", nameLength) == 0))
			{
				veniceIdHeader = (char*)&fileBuffer[fileOffset];
				veniceId = (char*)&fileBuffer[fileOffset + nameLength + 2];
				veniceIdLength = valueLength;
			}

			fileOffset += nameLength;
			fileOffset += 2;
			fileOffset += valueLength;
			if (fileOffset >= fileSize) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 10);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 10);
	}

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("[Account Information]");
	ULib.ConsoleLibrary.displayConsoleMessage("+ Account Id    : ", false);
	if (veniceId != NULL) ULib.ConsoleLibrary.displayConsoleMessage(veniceId, veniceIdLength, true);
	else ULib.ConsoleLibrary.displayConsoleMessage("Not Found", true);
	ULib.ConsoleLibrary.displayConsoleMessage("+ User Id       : ", false);
	if (userId != NULL) ULib.ConsoleLibrary.displayConsoleMessage(userId, userIdLength, true);
	else ULib.ConsoleLibrary.displayConsoleMessage("Not Found", true);
	ULib.ConsoleLibrary.displayConsoleMessage("+ User Password : ", false);
	if (userPassword != NULL) ULib.ConsoleLibrary.displayConsoleMessage(userPassword, userPasswordLength, true);
	else ULib.ConsoleLibrary.displayConsoleMessage("Not Found", true);

	//unsigned long fileSizeOrg = fileSize;

	// enumerate the service entries
	while (fileOffset < fileSize)
	{
		// skip service flags
		fileOffset += 6;
		if (fileOffset >= fileSize) break;

		// check we are starting at the service id
		if (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, fileOffset) != 0x24) break;
		if (handleFileService(&fileSize, &fileOffset, &fileUpdated)) foundService = true;
	}

	// update the live subscription file
	if ((foundService) && (fileUpdated))
	{
		ULib.ConsoleLibrary.displayConsoleMessage("\r\n- Reset Account Information?", false);/*
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 28, false);

		// update subscription information
		if (ULib.ConsoleLibrary.getConsoleInputAsYesNo(false))
		{
			char* userIdNew = ULib.StringLibrary.getString('0', userIdLength);
			char* userPasswordNew = ULib.StringLibrary.getString(' ', userPasswordLength);
			char* veniceIdNew = ULib.StringLibrary.getString('0', veniceIdLength);
			memcpy((char*)&userId[0], userIdNew, userIdLength);
			memcpy((char*)&userPassword[0], userPasswordNew, userPasswordLength);
			memcpy((char*)&veniceId[0], veniceIdNew, veniceIdLength);
			ULib.HeapLibrary.freePrivateHeap(userIdNew);
			ULib.HeapLibrary.freePrivateHeap(userPasswordNew);
			ULib.HeapLibrary.freePrivateHeap(veniceIdNew);
		}*/

		// update the timestamp
		time_t tm;
		time(&tm);
		ULib.StringLibrary.addDWORDToBuffer(fileBuffer, (((DWORD)tm) * 0x1D9C6EB) + 0x88A467);

		/*unsigned long fileSizeDiff = fileSize - fileSizeOrg;
		// update the location of the crc value
		ULib.StringLibrary.addDWORDToBuffer(fileBuffer, ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4) + fileSizeDiff, 4);
		// update the crc value
		unsigned long crcValue = 0;
		for (unsigned long i = 4; i < ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4); i += 4) crcValue += ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, i);
		ULib.StringLibrary.addDWORDToBuffer(fileBuffer, crcValue, ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4) + 4);*/

		// update the crc value
		unsigned long crcValue = 0;
		for (unsigned long i = 4; i < ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4); i += 4) crcValue += ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, i);
		ULib.StringLibrary.addDWORDToBuffer(fileBuffer, crcValue, ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 4) + 4);

		// encrypt the catalog
		ULib.ConsoleLibrary.displayConsoleMessage("Encrypting Live Subscription Catalog...");
	 	if (!handleFileEncryption(fileBuffer, fileSize, pboxResourceBuffer, sboxResourceBuffer)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 8);

		// create backup and update file
		ULib.ConsoleLibrary.displayConsoleMessage("Creating Backup and Updating the Live Subscription Catalog...");
		char* backupFilename = ULib.FileLibrary.getBackupFilename(fileName);
		CopyFile(fileName, backupFilename, TRUE);
		ULib.HeapLibrary.freePrivateHeap(backupFilename);
		if (!ULib.FileLibrary.writeBufferToFile(fileName, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 9);
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	if (!foundService) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Symantec Live Subscription Utility", "1.02", 2003);

	if ((argc > 1) && (ULib.StringLibrary.isStringMatch(argv[1], "/registry"))) parseRegistry();
	else if (argc > 1) parseFile(argv[1], ((argc > 2) && (ULib.StringLibrary.isStringMatch(argv[2], "/dumpFile"))));
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage  : symLiveSub [<live registration catalog> | /registry] [/dumpFile]");

	ULib.ProcessLibrary.exitProcessHandler();
}