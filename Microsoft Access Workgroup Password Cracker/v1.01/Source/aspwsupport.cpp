////////////////////////////////////////////////////////////
// Microsoft Access Workgroup Password Cracker            //
// (C)thewd, thewd@hotmail.com                            //
////////////////////////////////////////////////////////////
#include "aspw.h"
#include "..\..\..\_Shared Libraries\OpenSSL_DESLibrary.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;
extern bool DEBUG_MODE;
extern char* errorMessages[];

unsigned char XORBaseValues[4];

/////////////////////////////////////////////////////////////////////
// checks whether the file header conforms to the workgroup format //
/////////////////////////////////////////////////////////////////////
bool isJetWorkgroup(unsigned char* fileHeader)
{
	if (fileHeader == NULL) return false;
	return (ULib.StringLibrary.isStringMatch((char*)&fileHeader[0x04], "Jet System DB", false, 13));
}

///////////////////////////////////////////////////////////////////
// Decrypt the workgroup file header to determine the starting   //
// XORBaseValues for decrypting the accounts section (using RC4) //
///////////////////////////////////////////////////////////////////
void decryptJetHeader(unsigned char* fileBuffer)
{
	unsigned char* fileHeader = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	CopyMemory(fileHeader, fileBuffer, 0x100);

	// check for 'Jet System DB'
	if (!isJetWorkgroup(fileHeader)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	unsigned char baseValues[4] = {0xC7, 0xDA, 0x39, 0x6B};
	ULib.EncryptionLibrary.RC4Library.encryptBuffer(baseValues, 4, fileHeader, 0x18, 90);

	// retrieve XOR values
	XORBaseValues[0] = fileHeader[0x3E + 0];
	XORBaseValues[1] = fileHeader[0x3E + 1];
	XORBaseValues[2] = fileHeader[0x3E + 2];
	XORBaseValues[3] = fileHeader[0x3E + 3];

	ULib.HeapLibrary.freePrivateHeap(fileHeader);
}

///////////////////////////////////////////////////////////////
// used to determine the starting value for XORBaseValues[0] //
// which is used to decrypt the accounts section             //
///////////////////////////////////////////////////////////////
bool determineStartingXORBaseValues(unsigned char* fileBuffer, int baseOffset)
{
	unsigned char tempBuffer[2];

	// determine the correct starting value for XORBaseValues[0]
	for (int index = 0; index <= 0xFF; index++)
	{
		XORBaseValues[0] = (unsigned char)index;
		tempBuffer[0] = fileBuffer[baseOffset + 0];
		tempBuffer[1] = fileBuffer[baseOffset + 1];
		ULib.EncryptionLibrary.RC4Library.encryptBuffer(XORBaseValues, 4, tempBuffer, 0, 2);

		// check CRC value (0x0101)
		if ((tempBuffer[0] == 0x01) && (tempBuffer[1] == 0x01)) return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////
// Use the RC4 alogrithm to decrypt or encrypt the Access account section //
////////////////////////////////////////////////////////////////////////////
void doRC4(unsigned char* fileBuffer, int baseOffset, bool determineXORBaseValues)
{
	if (determineXORBaseValues)
	{
		decryptJetHeader(fileBuffer);

		if (DEBUG_MODE)
		{
			char* startingXORString = (char*)ULib.HeapLibrary.allocPrivateHeap();
			wsprintf(startingXORString, "RC4 Key (1)  : 0x%02X%02X%02X%02X", XORBaseValues[3], XORBaseValues[2], XORBaseValues[1], XORBaseValues[0]);
			ULib.ConsoleLibrary.displayConsoleMessage(startingXORString);
			ULib.HeapLibrary.freePrivateHeap(startingXORString);
		}

		// unable to determine appropriate starting XOR values
		if (!determineStartingXORBaseValues(fileBuffer, baseOffset)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

		// decrypt whole account section
		if (DEBUG_MODE)
		{
			char* foundXORString = (char*)ULib.HeapLibrary.allocPrivateHeap();
			wsprintf(foundXORString, "RC4 Key (2)  : 0x%02X%02X%02X%02X", XORBaseValues[3], XORBaseValues[2], XORBaseValues[1], XORBaseValues[0]);
			ULib.ConsoleLibrary.displayConsoleMessage(foundXORString);
			ULib.HeapLibrary.freePrivateHeap(foundXORString);
		}
	}

	ULib.EncryptionLibrary.RC4Library.encryptBuffer(XORBaseValues, 4, fileBuffer, baseOffset, 0x1500);
}

///////////////////////////////////////
// returns the workgroup information //
///////////////////////////////////////
void recoverWorkgroupInformation(unsigned char* fileBuffer, int fileOffset, int databaseFormat,
								 char** workgroupName, char** workgroupOrganisation, char** workgroupId)
{
	char* encryptionKey = "rkatpvjgjshcsabn";
	int numberOfGroups = 12;
	fileOffset += ((databaseFormat == Access200XFormat) ? 2 : 1);

	DES_key_schedule desScheduleA;
	DES_key_schedule desScheduleB;
	DES_cblock desKeyA;
	DES_cblock desKeyB;
	memcpy((void*)&desKeyA[0], (void*)&encryptionKey[0], 4);
	memcpy((void*)&desKeyA[4], (void*)&encryptionKey[4], 4);
	memcpy((void*)&desKeyB[0], (void*)&encryptionKey[8], 4);
	memcpy((void*)&desKeyB[4], (void*)&encryptionKey[12], 4);
	DES_set_key_unchecked(&desKeyA, &desScheduleA);
	DES_set_key_unchecked(&desKeyB, &desScheduleB);

	for (int i = 0; i < numberOfGroups; i++)
	{
		DES_decrypt3((DES_LONG*)&fileBuffer[(fileOffset + (8 * i))], &desScheduleA, &desScheduleB, &desScheduleA);
	}

	// long organisation
	if (fileBuffer[fileOffset + 0x60] != 0xDF)
	{
		fileBuffer[fileOffset + 0x60] = '?';
		fileBuffer[fileOffset + 0x61] = '?';
		fileBuffer[fileOffset + 0x62] = '?';
		fileBuffer[fileOffset + 0x63] = NULL;
	}

	if (workgroupId != NULL) *workgroupId = (char*)&fileBuffer[fileOffset];
	// not the default workgroup file
	if (ULib.StringLibrary.getStringLength(*workgroupId) != 1)
	{
		if (workgroupName != NULL) *workgroupName = (char*)&fileBuffer[fileOffset + 0x15];
		if (workgroupOrganisation != NULL) *workgroupOrganisation = (char*)&fileBuffer[fileOffset + 0x15 + 0x28];
	}
}

////////////////////////////////////////////////////////////////////
// returns the Account Name associated with an User-Level account //
////////////////////////////////////////////////////////////////////
char* recoverAccountName(unsigned char* fileBuffer, int fileOffset, int accountNameLength, int databaseFormat)
{
	if (databaseFormat == Access200XFormat) return ULib.StringLibrary.convertUnicodeToAnsi((PWSTR)&fileBuffer[fileOffset + 2], accountNameLength / 2);
	char* recoveredAccountName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	memcpy(recoveredAccountName, (char*)&fileBuffer[fileOffset + 1], accountNameLength);
	return recoveredAccountName;
}

////////////////////////////////////
// returns the decrypted password //
////////////////////////////////////
char* recoverPassword(unsigned char* fileBuffer, int fileOffset, int databaseFormat)
{
	char* encryptionKey = "kjhggautnitremvk";
	int numberOfGroups = ((databaseFormat == Access200XFormat) ? 5 : 2);

	DES_key_schedule desScheduleA;
	DES_key_schedule desScheduleB;
	DES_cblock desKeyA;
	DES_cblock desKeyB;
	memcpy((void*)&desKeyA[0], (void*)&encryptionKey[0], 4);
	memcpy((void*)&desKeyA[4], (void*)&encryptionKey[4], 4);
	memcpy((void*)&desKeyB[0], (void*)&encryptionKey[8], 4);
	memcpy((void*)&desKeyB[4], (void*)&encryptionKey[12], 4);
	DES_set_key_unchecked(&desKeyA, &desScheduleA);
	DES_set_key_unchecked(&desKeyB, &desScheduleB);

	for (int i = 0; i < numberOfGroups; i++)
	{
		DES_decrypt3((DES_LONG*)&fileBuffer[(fileOffset + (8 * i))], &desScheduleA, &desScheduleB, &desScheduleA);
	}

	if (databaseFormat == Access200XFormat) return ULib.StringLibrary.convertUnicodeToAnsi((PWSTR)&fileBuffer[fileOffset], 0x28);
	else return (char*)&fileBuffer[fileOffset];
}

///////////////////////////////////////
// returns the decrypted account pid //
///////////////////////////////////////
char* recoverPID(unsigned char* fileBuffer, int fileOffset, int pidLength)
{
	char* recoveredPID = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* recoveredPIDReversed = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* recoveredPIDPointer = NULL;
	unsigned char baseValue = 0;

	if (fileBuffer[fileOffset] == 0xFF)
	{
		recoveredPIDPointer = (char*)&fileBuffer[fileOffset + 1];
		pidLength--;
		baseValue = 4;
	}
	else
	{
		recoveredPIDPointer = (char*)&fileBuffer[fileOffset + 2];
		pidLength -= 2;
		baseValue = fileBuffer[fileOffset + 1];
	}

	unsigned char subValue = (unsigned char)(pidLength - baseValue);
	unsigned char xorValue = (unsigned char)((0x7FA52EBD / subValue) & 0xFF);
	for (int i = 0; i < pidLength; i++) recoveredPIDPointer[i] ^= (xorValue * i);

	unsigned char shiftRightValue = (unsigned char)((subValue % 7) + 1);
	unsigned char shiftLeftValue = (unsigned char)(8 - shiftRightValue);
	unsigned char lastValue = (unsigned char)(recoveredPIDPointer[pidLength - 1] << shiftLeftValue);
	unsigned char currentValue = 0;
	for (int i = 0; i < pidLength; i++)
	{
		currentValue = recoveredPIDPointer[i];
		recoveredPIDPointer[i] = (char)(lastValue | (currentValue >> shiftRightValue));
		lastValue = (unsigned char)(currentValue << shiftLeftValue);
	}

	unsigned char splitValue1 = (unsigned char)(pidLength - (pidLength >> 1));
	unsigned char splitValue2 = (unsigned char)(pidLength - splitValue1);
	memcpy(recoveredPIDReversed, (LPVOID)&recoveredPIDPointer[splitValue1], splitValue2);
	memcpy((LPVOID)&recoveredPIDReversed[(splitValue2)], recoveredPIDPointer, splitValue1);

	int index = 0;
	int incrementValue = ((baseValue <= 1) ? 1 : (subValue / (baseValue - 1)));
	for (unsigned char i = 0; i < baseValue;)
	{
		recoveredPID[baseValue - (++i)] = recoveredPIDReversed[index++];
		index += incrementValue;
	}

	ULib.HeapLibrary.freePrivateHeap(recoveredPIDReversed);
	return recoveredPID;
}