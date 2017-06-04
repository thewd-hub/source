#include "ckInfo.h"
#include "ckEncryption.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include <time.h>

extern CUtilitiesLibrary ULib;

// confirmation code class
class CCrypKey_ConfirmationCode
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_ConfirmationCode
	{
		unsigned char encryptionSeed;
		unsigned char _reserved1; // code version?
		unsigned char _reserved2;
		unsigned char crypkeyVersion;
		unsigned char licenceCount;
		unsigned short accountInformation;
		unsigned long codeTimestamp;
		unsigned short crcValue;
	} CrypKey_ConfirmationCode;
	#pragma pack()

	CrypKey_ConfirmationCode* cCode;

public:
	CCrypKey_ConfirmationCode(unsigned char* decryptedBuffer) { cCode = (CrypKey_ConfirmationCode*)decryptedBuffer; }

	void formatConfirmationCode(char** outputBuffer)
	{
		unsigned char* cCodeBuffer = (unsigned char*)cCode;
		wsprintf(*outputBuffer, "                       %02X%02X %02X %02X %02X%02X %02X%02X%02X%02X %02X%02X",
			cCodeBuffer[1], cCodeBuffer[2],
			cCodeBuffer[3],
			cCodeBuffer[4],
			cCodeBuffer[5], cCodeBuffer[6],
			cCodeBuffer[7], cCodeBuffer[8], cCodeBuffer[9], cCodeBuffer[10],
			cCodeBuffer[11], cCodeBuffer[12]);
	}

	unsigned short getCRCValue(void) { return ULib.StringLibrary.toBigEndian(cCode->crcValue); }
	unsigned long getCrypKeyVersion(void) { return (cCode->crypkeyVersion & 0x7F); }
	unsigned long getCrypKeyVersionMajor(void) { return (getCrypKeyVersion() / 10); }
	unsigned long getCrypKeyVersionMinor(void) { return (getCrypKeyVersion() % 10); }
	bool isNetworkLicence(void) { return (cCode->licenceCount > 0x7F); }
	unsigned long getLicenceCount(void) { return ((isNetworkLicence()) ? ((cCode->licenceCount * -1) & 0xFF) : cCode->licenceCount); }
	unsigned long getAccountNumber(void) { return (cCode->accountInformation & 0x03FF); }
	unsigned long getApplicationId(void) { return (cCode->accountInformation >> 0x0A); }
	char* getCompanyName(void) { return getCompanyNameFromAccountNumber(getAccountNumber(), 26); }
	long getTimestamp(void) { return (cCode->codeTimestamp + 0x7C543000); }
};

//////////////////////////////////////////////////////////////
// display information retrieved from the confirmation code //
//////////////////////////////////////////////////////////////
void showConfirmationCodeInformation(char* confirmationCodeString)
{
	unsigned char* confirmationCode = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* confirmationCodeDecrypted = NULL;

	// check confirmation code length
	int confirmationCodeLength = ULib.StringLibrary.getStringLength(confirmationCodeString);
	if (confirmationCodeLength != 26) displayErrorMessage(ERROR_CONFIRMATIONCODE_LENGTH);

	// convert confirmation code string into bytes
	for (int i = 0; i < confirmationCodeLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(confirmationCode, confirmationCodeString[i], confirmationCodeString[i + 1], (i / 2));

	// decrypt confirmation code
	confirmationCodeLength = 13;
	confirmationCodeDecrypted = decryptKey(confirmationCode, confirmationCodeLength, KEYTYPE_CONFIRMATIONCODE);

	ULib.ConsoleLibrary.displayConsoleMessage("Parsing Code    - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(confirmationCode, confirmationCodeLength, 2, true);

	// initialise confirmation code class
	CCrypKey_ConfirmationCode cCode(confirmationCodeDecrypted);

	ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Code - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(confirmationCodeDecrypted, confirmationCodeLength, 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Code Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((isKeyCRCCorrect(confirmationCodeDecrypted, confirmationCodeLength, cCode.getCRCValue(), KEYTYPE_CONFIRMATIONCODE)) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Formatting Code :");

	// format confirmation code
	char* cCodeFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	cCode.formatConfirmationCode(&cCodeFormatted);

	// get code timestamp
	struct tm tmTime;
	time_t tTime = cCode.getTimestamp();
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
	char* companyName = cCode.getCompanyName();
	bool isCompanyNameAvailable = (ULib.StringLibrary.getStringLength(companyName) > 0);

	char* licenceCodePaddedString = createPaddedStringFromValue(cCode.getLicenceCount(), 4, 'อ', false);
	wsprintf(outputLine1, "                       ศออผ ศน ศน ฬออผ ฬออออออผ ฬออผ");
	wsprintf(outputLine2, "  CrypKey Libraries - v%d.%d ออผ  บ บ    บ        ศอออออออออออ Code CRC - 0x%04X", cCode.getCrypKeyVersionMajor(), cCode.getCrypKeyVersionMinor(), cCode.getCRCValue());
	wsprintf(outputLine3, "  Number Of Licences - %d %sออออน บ    ศอ Code Timestamp - %s", cCode.getLicenceCount(), licenceCodePaddedString, dateString);
	wsprintf(outputLine4, "  Network Licence? - %sอออออออผ ฬอ Account Number - %d", ((cCode.isNetworkLicence()) ? "Yes " : "No อ"), cCode.getAccountNumber());
	wsprintf(outputLine5, "                                  ฬอ Application Id - %d", cCode.getApplicationId());
	wsprintf(outputLine6, "                                  %cอ Company Number - 7956%d", ((isCompanyNameAvailable) ? 'ฬ' : 'ศ'), cCode.getAccountNumber());
	wsprintf(outputLine7, "                                  ศอ Company Name - %s", companyName);
	ULib.HeapLibrary.freePrivateHeap(licenceCodePaddedString);

	ULib.ConsoleLibrary.displayConsoleMessage(cCodeFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
	if (isCompanyNameAvailable) ULib.ConsoleLibrary.displayConsoleMessage(outputLine7);

	ULib.HeapLibrary.freePrivateHeap(cCodeFormatted);
	ULib.HeapLibrary.freePrivateHeap(dateString);
	ULib.HeapLibrary.freePrivateHeap(companyName);
	ULib.HeapLibrary.freePrivateHeap(outputLine1);
	ULib.HeapLibrary.freePrivateHeap(outputLine2);
	ULib.HeapLibrary.freePrivateHeap(outputLine3);
	ULib.HeapLibrary.freePrivateHeap(outputLine4);
	ULib.HeapLibrary.freePrivateHeap(outputLine5);
	ULib.HeapLibrary.freePrivateHeap(outputLine6);
	ULib.HeapLibrary.freePrivateHeap(outputLine7);

	ULib.HeapLibrary.freePrivateHeap(confirmationCode);
	ULib.HeapLibrary.freePrivateHeap(confirmationCodeDecrypted);
}