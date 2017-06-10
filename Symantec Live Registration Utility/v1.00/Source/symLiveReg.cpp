/////////////////////////////////////////////////////
// Symantec Live Registration Utility              //
// (C)thewd, thewd@hotmail.com                     //
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Used to register products contained within the  //
// live registration files. Can also dump the file //
// contents. Supports Norton SystemWorks 2000+     //
/////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the Live Registration file",
	"Doesn't appear to be a supported Live Registration file",
	"Error occurred creating a backup and updating the file"
};

// supported value types
#define VALUETYPE_STRING	1
#define VALUETYPE_BINARY	3
#define VALUETYPE_DWORD		4

// product information structure
typedef struct _PRODUCT_REGISTRATION_INFORMATION
{
	bool isComplete;

	char* keyName;
	char* productName;
	char* productVersion;
	char* serialNumber;
	unsigned long serialNumberIndex;
	bool isRegistered;
	unsigned long isRegisteredIndex;

} PRODUCT_REGISTRATION_INFORMATION;

PRODUCT_REGISTRATION_INFORMATION* productRegistrationInformation[64];
int currentProductRegistrationInformationIndex = 0;

///////////////////////////////////////////////////////////////////
// cycles through the keys and values, dumping them if necessary //
///////////////////////////////////////////////////////////////////
void parseKeys(char* currentKeyName, unsigned char* fileBuffer, int* currentIndex, int numberOfPaddingSpaces, bool dumpFile)
{
	if (currentKeyName != NULL)
	{
		// store key name
		if (dumpFile)
		{
			for (int j = 0; j < numberOfPaddingSpaces; j++) ULib.ConsoleLibrary.displayConsoleMessage(" ", false);
			ULib.ConsoleLibrary.displayConsoleMessage("[", false);
			ULib.ConsoleLibrary.displayConsoleMessage(currentKeyName, false);
			ULib.ConsoleLibrary.displayConsoleMessage("]");
		}

		numberOfPaddingSpaces += 2;
	}

	// check for any values
	int numberOfValues = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, *currentIndex);
	*currentIndex += 4;

	// check after parsing the values whether we have collected all the required information
	bool checkProductRegistrationInformation = false;

	// parse values
	for (int i = 0; i < numberOfValues; i++)
	{
		// get value name length
		int valueNameLength = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, *currentIndex);
		*currentIndex += 4;

		// get value name
		char* currentValueName = (char*)&fileBuffer[*currentIndex];

		// store value name
		if (dumpFile)
		{
			for (int j = 0; j < numberOfPaddingSpaces; j++) ULib.ConsoleLibrary.displayConsoleMessage(" ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(currentValueName, valueNameLength, false);
			ULib.ConsoleLibrary.displayConsoleMessage("=", false);
		}

		// skip value name
		*currentIndex += valueNameLength;

		// get the value type
		int valueType = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, *currentIndex);
		*currentIndex += 4;

		// get value data length
		int valueDataLength = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, *currentIndex);
		*currentIndex += 4;

		// get value data
		char* currentValueData = (char*)&fileBuffer[*currentIndex];

		// store value data
		if (dumpFile)
		{
			if (valueType == VALUETYPE_STRING) ULib.ConsoleLibrary.displayConsoleMessage(currentValueData, valueDataLength, true);
			else if (valueType == VALUETYPE_BINARY) ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)currentValueData, valueDataLength, 1, true, NULL, ",");
			else if (valueType == VALUETYPE_DWORD) ULib.ConsoleLibrary.displayConsoleMessage(ULib.StringLibrary.getDWORDFromBuffer((unsigned char*)currentValueData), true, true);
			else ULib.ConsoleLibrary.displayConsoleMessage("<unknown value data type>");
		}

		// check for a possible product name match (and store information)
		if (ULib.StringLibrary.isStringMatch(currentValueName, "Name", false, 4))
		{
			checkProductRegistrationInformation = true;
			productRegistrationInformation[currentProductRegistrationInformationIndex] = (PRODUCT_REGISTRATION_INFORMATION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(PRODUCT_REGISTRATION_INFORMATION));
			productRegistrationInformation[currentProductRegistrationInformationIndex]->keyName = (char*)ULib.HeapLibrary.allocPrivateHeap();
			productRegistrationInformation[currentProductRegistrationInformationIndex]->productName = (char*)ULib.HeapLibrary.allocPrivateHeap();
			productRegistrationInformation[currentProductRegistrationInformationIndex]->productVersion = (char*)ULib.HeapLibrary.allocPrivateHeap();
			productRegistrationInformation[currentProductRegistrationInformationIndex]->serialNumber = (char*)ULib.HeapLibrary.allocPrivateHeap();
			productRegistrationInformation[currentProductRegistrationInformationIndex]->serialNumberIndex = 0;
			productRegistrationInformation[currentProductRegistrationInformationIndex]->isRegistered = false;
			productRegistrationInformation[currentProductRegistrationInformationIndex]->isRegisteredIndex = 0;
			productRegistrationInformation[currentProductRegistrationInformationIndex]->isComplete = false;

			ULib.StringLibrary.copyString(productRegistrationInformation[currentProductRegistrationInformationIndex]->keyName, currentKeyName);
			ULib.StringLibrary.copyString(productRegistrationInformation[currentProductRegistrationInformationIndex]->productName, currentValueData);
		}
		// collect further product information
		else if (checkProductRegistrationInformation)
		{
			if (ULib.StringLibrary.isStringMatch(currentValueName, "Version", false, 7)) ULib.StringLibrary.copyString(productRegistrationInformation[currentProductRegistrationInformationIndex]->productVersion, currentValueData);
			else if (ULib.StringLibrary.isStringMatch(currentValueName, "SerialNum", false, 9))
			{
				ULib.StringLibrary.copyString(productRegistrationInformation[currentProductRegistrationInformationIndex]->serialNumber, currentValueData);
				productRegistrationInformation[currentProductRegistrationInformationIndex]->serialNumberIndex = *currentIndex;
			}
			else if (ULib.StringLibrary.isStringMatch(currentValueName, "IsRegistered", false, 12))
			{
				if ((currentValueData != NULL) && (currentValueData[0] == 'Y')) productRegistrationInformation[currentProductRegistrationInformationIndex]->isRegistered = true;
				else productRegistrationInformation[currentProductRegistrationInformationIndex]->isRegistered = false;
				productRegistrationInformation[currentProductRegistrationInformationIndex]->isRegisteredIndex = *currentIndex;

				// flag product parsing has finished
				productRegistrationInformation[currentProductRegistrationInformationIndex]->isComplete = true;
			}
		}

		// skip value data
		*currentIndex += valueDataLength;
	}

	// check for a completed product registration information structure
	if (checkProductRegistrationInformation)
	{
		if (productRegistrationInformation[currentProductRegistrationInformationIndex]->isComplete)
		{
			currentProductRegistrationInformationIndex++;
			if (currentProductRegistrationInformationIndex >= 64) return;
		}
		else
		{
			ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[currentProductRegistrationInformationIndex]->keyName);
			ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[currentProductRegistrationInformationIndex]->productName);
			ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[currentProductRegistrationInformationIndex]->productVersion);
			ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[currentProductRegistrationInformationIndex]->serialNumber);
			ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[currentProductRegistrationInformationIndex]);
		}
	}

	// check for any keys
	int numberOfKeys = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, *currentIndex);
	*currentIndex += 4;

	// parse keys
	for (int i = 0; i < numberOfKeys; i++)
	{
		// get key name length
		int keyNameLength = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, *currentIndex);
		*currentIndex += 4;

		// get key name
		char* keyName = (char*)ULib.HeapLibrary.allocPrivateHeap();
		memcpy(keyName, (char*)&fileBuffer[*currentIndex], keyNameLength);

		// skip key name
		*currentIndex += keyNameLength;

		// parse sub keys and values
		parseKeys(keyName, fileBuffer, currentIndex, numberOfPaddingSpaces, dumpFile);
		ULib.HeapLibrary.freePrivateHeap(keyName);
	}
}

///////////////////////////////////////////////////////////
// registers the product with the supplied serial number //
///////////////////////////////////////////////////////////
unsigned char* registerProductInformation(unsigned char* fileBuffer, unsigned long* fileSize, PRODUCT_REGISTRATION_INFORMATION* productRegistrationInformation, unsigned long* baseAdjustment)
{
	unsigned char* newFileBuffer = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(*fileSize + 0x20);

	// get offset values
	unsigned long serialNumberIndex = productRegistrationInformation->serialNumberIndex + *baseAdjustment;
	unsigned long isRegisteredIndex = productRegistrationInformation->isRegisteredIndex + *baseAdjustment;

	// adjust serial number size and the is registered flag
	fileBuffer[serialNumberIndex - 4] = 0x12;
	fileBuffer[isRegisteredIndex] = 'Y';

	// update buffer
	memcpy(newFileBuffer, fileBuffer, serialNumberIndex);
	memcpy((unsigned char*)&newFileBuffer[serialNumberIndex], productRegistrationInformation->serialNumber, 0x12);
	memcpy((unsigned char*)&newFileBuffer[serialNumberIndex + 0x12], (unsigned char*)&fileBuffer[serialNumberIndex + 1], *fileSize - serialNumberIndex - 1);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);

	// adjust values
	*fileSize += 0x11;
	*baseAdjustment += 0x11;

	return newFileBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// opens the live registration file and parses the keys. dumps if required //
/////////////////////////////////////////////////////////////////////////////
void openFileAndParse(char* fileName, bool dumpFile)
{
	// open and read file
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize < 4)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// check for a supported file format
	int currentIndex = 0;
	if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, currentIndex) != 0x031DB358) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	HANDLE dumpFileHandle = NULL;
	char* dumpFilename = NULL;

	// create dump file and handles
	if (dumpFile)
	{
		dumpFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(dumpFilename, "%s.txt", fileName);
		char* dumpFilenameStripped = ULib.FileLibrary.stripPathFromFilename(dumpFilename, true);
		dumpFileHandle = CreateFile(dumpFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		ULib.ConsoleLibrary.displayConsoleMessage("Dumping Live Registration File (", false);
		ULib.ConsoleLibrary.displayConsoleMessage(dumpFilenameStripped, false);
		ULib.ConsoleLibrary.displayConsoleMessage(")...\r\n");
		ULib.ConsoleLibrary.setConsoleOutputHandle(dumpFileHandle);
		ULib.HeapLibrary.freePrivateHeap(dumpFilenameStripped);
	}

	// parse all the keys and values
	currentIndex += 4;
	parseKeys(NULL, fileBuffer, &currentIndex, 0, dumpFile);

	// close dump file handles
	if (dumpFile)
	{
		ULib.HeapLibrary.freePrivateHeap(dumpFilename);
		CloseHandle(dumpFileHandle);
	}

	// display found information
	ULib.ConsoleLibrary.setConsoleOutputHandle(NULL);

	char* outputBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int numberOfRegisteredProducts = 0;
	int numberOfUnregisteredProducts = 0;

	// display any found registered product information
	ULib.ConsoleLibrary.displayConsoleMessage("[Registered Products]");

	for (int i = 0; i < currentProductRegistrationInformationIndex; i++)
	{
		if (productRegistrationInformation[i]->isRegistered)
		{
			numberOfRegisteredProducts++;
			wsprintf(outputBuffer, "  %s %s (%s)", productRegistrationInformation[i]->productName, productRegistrationInformation[i]->productVersion, productRegistrationInformation[i]->serialNumber);
			ULib.ConsoleLibrary.displayConsoleMessage(outputBuffer);
		}
	}

	if (numberOfRegisteredProducts == 0) ULib.ConsoleLibrary.displayConsoleMessage("  None");

	// display any found unregistered product information
	unsigned long baseAdjustment = 0;
	bool fileUpdated = false;
	ULib.ConsoleLibrary.displayConsoleMessage("\r\n[Unregistered Products]");

	for (int i = 0; i < currentProductRegistrationInformationIndex; i++)
	{
		if (!productRegistrationInformation[i]->isRegistered)
		{
			numberOfUnregisteredProducts++;
			wsprintf(outputBuffer, "  %s %s", productRegistrationInformation[i]->productName, productRegistrationInformation[i]->productVersion);
			ULib.ConsoleLibrary.displayConsoleMessage(outputBuffer, false);

			// register product?
			ULib.ConsoleLibrary.displayConsoleMessage(" - [Register Product?]\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", false);
			bool registerProduct = ULib.ConsoleLibrary.getConsoleInputAsYesNo(false);
			if (!registerProduct) ULib.ConsoleLibrary.displayConsoleMessage("\b\b\b\b                      ");
			else
			{
				char* consoleInput = (char*)ULib.ConsoleLibrary.getConsoleInputAsFormattedString("111~1111~111~1111", "xxx-xxxx-xxx-xxxx", false, true);
				ULib.StringLibrary.copyString(productRegistrationInformation[i]->serialNumber, consoleInput);
				ULib.HeapLibrary.freePrivateHeap(consoleInput);
				fileBuffer = registerProductInformation(fileBuffer, &fileSize, productRegistrationInformation[i], &baseAdjustment);
				fileUpdated = true;
			}
		}
	}

	if (numberOfUnregisteredProducts == 0) ULib.ConsoleLibrary.displayConsoleMessage("  None");

	for (int i = 0; i < currentProductRegistrationInformationIndex; i++)
	{
		ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[i]->keyName);
		ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[i]->productName);
		ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[i]->productVersion);
		ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[i]->serialNumber);
		ULib.HeapLibrary.freePrivateHeap(productRegistrationInformation[i]);
	}

	// update the live registration file
	if (fileUpdated)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("\r\nCreating Backup and Updating the Live Registration File...");
		char* backupFilename = ULib.FileLibrary.getBackupFilename(fileName);
		CopyFile(fileName, backupFilename, TRUE);
		ULib.HeapLibrary.freePrivateHeap(backupFilename);
		if (!ULib.FileLibrary.writeBufferToFile(fileName, fileBuffer, fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	}

	ULib.HeapLibrary.freePrivateHeap(outputBuffer);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Symantec Live Registration Utility", "1.00", 2002);

	if (argc > 1) openFileAndParse(argv[1], ((argc > 2) && (ULib.StringLibrary.isStringMatch(argv[2], "/dumpFile"))));
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage  : symLiveReg <live registration filename> [/dumpFile]");

	ULib.ProcessLibrary.exitProcessHandler();
}