#include "ckInfo.h"
#include "ckEncryption.h"
#include "ckUserKey.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;
extern char* errorMessages[];

// user key class
class CCrypKey_UserKey
{
private:
	#pragma pack(1)
	typedef struct _CrypKey_UserKey
	{
		unsigned char encryptionSeed;
		unsigned char encryptedPassword[12];
	} CrypKey_UserKey;
	#pragma pack()

	CrypKey_UserKey* uKey;
	unsigned long uKeyLength;
	unsigned long encryptedPasswordLength;

public:
	CCrypKey_UserKey(unsigned char* decryptedBuffer, unsigned long decryptedBufferLength)
	{
		uKey = (CrypKey_UserKey*)decryptedBuffer;
		uKeyLength = decryptedBufferLength;
		encryptedPasswordLength = uKeyLength - 1;
	}

	void formatUserKey(char** outputBuffer)
	{
		unsigned char* uKeyBuffer = (unsigned char*)uKey;
		ULib.StringLibrary.copyString(*outputBuffer, "                 ");
		for (unsigned long count = 0; count < encryptedPasswordLength; count++) wsprintf(*outputBuffer, "%s%02X", *outputBuffer, uKeyBuffer[count + 1]);
	}

	bool isUserKeyValid(void)
	{
		for (unsigned long i = 1; i < encryptedPasswordLength; i++) if (uKey->encryptedPassword[i] >= 0x20) return false;
		return true;
	}
	unsigned char* getEncryptedPassword(void) { return (unsigned char*)&uKey->encryptedPassword; }
	unsigned long getEncryptedPasswordLength(void) { return encryptedPasswordLength; }
	char* getPlaintextPassword(void) { return decryptPassword((unsigned char*)&uKey->encryptedPassword, encryptedPasswordLength); }
	unsigned long getPasswordNumber(char* userKeyString, unsigned long userKeyStringLength)
	{
		unsigned long passwordNumber = 1;
		for (unsigned long i = 0; i < userKeyStringLength; i++) passwordNumber = ((passwordNumber * userKeyString[i]) % 0x7FF0BDC9);
		return passwordNumber;
	}
	unsigned long getUserKeyHash(void)
	{
		int userKeyHash = 0;
		int userKeyHash6 = 0;
		getUserKeyHashes((unsigned char*)&uKey->encryptedPassword, encryptedPasswordLength, &userKeyHash, &userKeyHash6);
		return userKeyHash;
	}
	unsigned long getUserKeyHash6(void)
	{
		int userKeyHash = 0;
		int userKeyHash6 = 0;
		getUserKeyHashes((unsigned char*)&uKey->encryptedPassword, encryptedPasswordLength, &userKeyHash, &userKeyHash6);
		return userKeyHash6;
	}
	unsigned char* getUserKey(void) { return (unsigned char*)uKey; }
	unsigned long getUserKeyLength(void) { return uKeyLength; }

	void setPlaintextPassword(char* passwordString)
	{
		unsigned long passwordStringLength = ULib.StringLibrary.getStringLength(passwordString);
		if (passwordStringLength > 12) passwordStringLength = 12;
		if (passwordStringLength & 0x01) passwordString[passwordStringLength++] = 0x00;
		uKeyLength = passwordStringLength + 1;
		unsigned char* encryptedPassword = encryptPassword((unsigned char*)CharUpper(passwordString), passwordStringLength);
		memcpy((unsigned char*)&uKey->encryptedPassword, encryptedPassword, passwordStringLength);
		ULib.HeapLibrary.freePrivateHeap(encryptedPassword);
	}
};

///////////////////////////////////////////////////////////////
// retrieves the user key hashes from the encrypted password //
///////////////////////////////////////////////////////////////
void getUserKeyHashes(unsigned char* encryptedPasswordBytes, int encryptedPasswordLength, int* userKeyHash, int* userKeyHash6)
{
	*userKeyHash = 0;
	*userKeyHash6 = 0;

	// get the hashes from the encrypted password bytes
	char* encryptedPasswordString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 0; i < encryptedPasswordLength; i++)
	{
		// reached padding
		if (encryptedPasswordBytes[i] == 0)
		{
			encryptedPasswordLength = i;
			break;
		}

		*userKeyHash += encryptedPasswordBytes[i];
		char* byteString = ULib.StringLibrary.getStringFromBYTE(encryptedPasswordBytes[i], true);
		ULib.StringLibrary.appendString(encryptedPasswordString, byteString);
		ULib.HeapLibrary.freePrivateHeap(byteString);
	}

	unsigned char* userKeyHash6String = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(10);
	ULib.StringLibrary.addWORDToBuffer(userKeyHash6String, getKeyCRC((unsigned char*)encryptedPasswordString, ((encryptedPasswordLength - 0) * 2), KEYTYPE_USERKEY), 0, true);
	*userKeyHash6 = ULib.StringLibrary.getWORDFromBuffer(userKeyHash6String);

	ULib.HeapLibrary.freePrivateHeap(encryptedPasswordString);
	ULib.HeapLibrary.freePrivateHeap(userKeyHash6String);
}

////////////////////////////////////////////////////////////////////////////////////////////
// retrieves the user key hashes from the encrypted password within the supplied user key //
////////////////////////////////////////////////////////////////////////////////////////////
void getUserKeyHashesFromUserKey(char* userKeyString, int* userKeyHash, int* userKeyHash6)
{
	unsigned char* userKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* userKeyDecrypted = NULL;

	// check user key length
	int userKeyStringLength = ULib.StringLibrary.getStringLength(userKeyString);
	int userKeyLength = (userKeyStringLength / 2);
	if ((userKeyStringLength < 2) || (userKeyStringLength > 26))
	{
		ULib.ConsoleLibrary.displayConsoleMessage("");
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_USERKEY_LENGTH);
	}

	// convert user key string into bytes
	for (int i = 0; i < userKeyStringLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(userKey, userKeyString[i], userKeyString[i + 1], (i / 2));

	// decrypt user key
	userKeyDecrypted = decryptKey(userKey, userKeyLength, KEYTYPE_USERKEY);

	// initialise user key class
	CCrypKey_UserKey uKey(userKeyDecrypted, userKeyLength);
	if (!uKey.isUserKeyValid())
	{
		ULib.ConsoleLibrary.displayConsoleMessage("");
		displayErrorMessage(ERROR_USERKEY_LENGTH);
	}

	// get the user key hashes
	getUserKeyHashes(uKey.getEncryptedPassword(), uKey.getEncryptedPasswordLength(), userKeyHash, userKeyHash6);

	ULib.HeapLibrary.freePrivateHeap(userKey);
	ULib.HeapLibrary.freePrivateHeap(userKeyDecrypted);
}

///////////////////////////////////////////////////////////////
// retrieves the user key hashes from the encrypted password //
///////////////////////////////////////////////////////////////
void getUserKeyHashesFromPassword(char* passwordString, int* userKeyHash, int* userKeyHash6)
{
	int passwordStringLength = ULib.StringLibrary.getStringLength(passwordString);
	unsigned char* encryptedPasswordBytes = encryptPassword((unsigned char*)CharUpper(passwordString), passwordStringLength);
	// get the user key hashes
	getUserKeyHashes(encryptedPasswordBytes, passwordStringLength, userKeyHash, userKeyHash6);
	ULib.HeapLibrary.freePrivateHeap(encryptedPasswordBytes);
}

/////////////////////////////////////////////////////
// display information retrieved from the user key //
/////////////////////////////////////////////////////
void showUserKeyInformation(char* userKeyString)
{
	unsigned char* userKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* userKeyDecrypted = NULL;

	// check user key length
	int userKeyStringLength = ULib.StringLibrary.getStringLength(userKeyString);
	int userKeyLength = (userKeyStringLength / 2);
	if ((userKeyStringLength < 2) || (userKeyStringLength > 26)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_USERKEY_LENGTH);

	// convert user key string into bytes
	for (int i = 0; i < userKeyStringLength; i += 2) ULib.StringLibrary.addBYTEToBuffer(userKey, userKeyString[i], userKeyString[i + 1], (i / 2));

	// decrypt user key
	userKeyDecrypted = decryptKey(userKey, userKeyLength, KEYTYPE_USERKEY);

	ULib.ConsoleLibrary.displayConsoleMessage("Parsing Key    - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(userKey, userKeyLength, 2, true);

	// initialise user key class
	CCrypKey_UserKey uKey(userKeyDecrypted, userKeyLength);

	ULib.ConsoleLibrary.displayConsoleMessage("Decrypting Key - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(userKeyDecrypted, userKeyLength, 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((uKey.isUserKeyValid()) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Formatting Key :");

	// format user key
	char* uKeyFormatted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	uKey.formatUserKey(&uKeyFormatted);

	// generate output and display
	char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	bool isPasswordInformationAvailable = (uKey.getEncryptedPasswordLength() > 0);

	if (isPasswordInformationAvailable)
	{
		char* paddedString = ULib.StringLibrary.getString('อ', (uKey.getEncryptedPasswordLength() * 2) - 2);
		char* plaintextPassword = uKey.getPlaintextPassword();
		wsprintf(outputLine1, "                 ฬ%sผ", paddedString);
		wsprintf(outputLine2, "                 ฬอออ Password - %s", plaintextPassword);
		wsprintf(outputLine3, "                 ฬอออ Password Number - %d ", uKey.getPasswordNumber(userKeyString, userKeyStringLength));
		wsprintf(outputLine4, "                 ฬอออ User Key Hash (1) - 0x%02X ", uKey.getUserKeyHash());
		wsprintf(outputLine5, "                 ศอออ User Key Hash (2) - 0x%04X ", uKey.getUserKeyHash6());
		ULib.HeapLibrary.freePrivateHeap(paddedString);
		ULib.HeapLibrary.freePrivateHeap(plaintextPassword);
	}

	ULib.ConsoleLibrary.displayConsoleMessage(uKeyFormatted);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
	if (isPasswordInformationAvailable)
	{
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
	}

	ULib.HeapLibrary.freePrivateHeap(uKeyFormatted);
	ULib.HeapLibrary.freePrivateHeap(outputLine1);
	ULib.HeapLibrary.freePrivateHeap(outputLine2);
	ULib.HeapLibrary.freePrivateHeap(outputLine3);
	ULib.HeapLibrary.freePrivateHeap(outputLine4);
	ULib.HeapLibrary.freePrivateHeap(outputLine5);

	ULib.HeapLibrary.freePrivateHeap(userKey);
	ULib.HeapLibrary.freePrivateHeap(userKeyDecrypted);
}

/////////////////////////////////////////////////////////
// create a new user key from the information supplied //
/////////////////////////////////////////////////////////
void createUserKey(void)
{
	unsigned char* userKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* userKeyEncrypted = NULL;
	unsigned char* userKeyDecrypted = NULL;

	// initialise user key class
	CCrypKey_UserKey uKey(userKey, 1);

	// get required information
	ULib.ConsoleLibrary.displayConsoleMessage("Key Information...");

	// user password
	ULib.ConsoleLibrary.displayConsoleMessage("+ Plaintext Password : ", false);
	char* parsedPassword = getParsedParameter("Password");
	unsigned char* userKey_Password = NULL;
	if (parsedPassword != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage(parsedPassword);
		userKey_Password = (unsigned char*)parsedPassword;
	}
	else
	{
		userKey_Password = ULib.ConsoleLibrary.getConsoleInput(12);
	}

	uKey.setPlaintextPassword((char*)userKey_Password);
	ULib.HeapLibrary.freePrivateHeap(userKey_Password);

	// encrypt user key (and check decryption succeeded)
	userKeyEncrypted = encryptKey(uKey.getUserKey(), uKey.getUserKeyLength(), KEYTYPE_USERKEY);
	userKeyDecrypted = decryptKey(userKeyEncrypted, uKey.getUserKeyLength(), KEYTYPE_USERKEY);

	ULib.ConsoleLibrary.displayConsoleMessage("");
	ULib.ConsoleLibrary.displayConsoleMessage("Key Validation - ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((uKey.isUserKeyValid()) ? "OK" : "FAILED");
	ULib.ConsoleLibrary.displayConsoleMessage("Creating Key   - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(uKey.getUserKey(), uKey.getUserKeyLength(), 2, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Encrypting Key - ", false);
	ULib.ConsoleLibrary.displayHexadecimalString(userKeyEncrypted, uKey.getUserKeyLength(), 2, true);

	ULib.HeapLibrary.freePrivateHeap(userKeyDecrypted);
	ULib.HeapLibrary.freePrivateHeap(userKeyEncrypted);
	ULib.HeapLibrary.freePrivateHeap(userKey);
}