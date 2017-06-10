//////////////////////////////////////////////////////////
// Norton Internet Security URL List Decryption Utility //
// (C)thewd, thewd@hotmail.com                          //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Decrypts the banned urls within the Norton Internet  //
// Security URL Lists database files (v1.xx+)           //
//////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the encrypted URL List database",
	"{CRLF}Error reading data from the encrypted URL List database",
	"Doesn't appear to be a supported URL List database",
	"Error occurred saving the urls into categories"
};

typedef struct _URLCATEGORY
{
	char* categoryName;
	HANDLE fileHandle;

} URLCATEGORY;

#define URL_LIST_CATEGORIES_DIRECTORY	"URL List Categories"
const int urlCategoriesMaximumIndex = 100;
int urlCategoriesEndIndex = 0;
URLCATEGORY urlCategories[urlCategoriesMaximumIndex + 1];

/////////////////////////////////////////////////////////////
// splits the url and store it in the appropriate category //
/////////////////////////////////////////////////////////////
bool storeUrlInCategory(char* fullUrl, char* categoryName)
{
	// replace illegal characters in url category name
	for (int i = 0; i < ULib.StringLibrary.getStringLength(categoryName); i++)
	{
		if (categoryName[i] == '/')
		{
			categoryName[i] = '.';
		}
		else
		{
			if ((categoryName[i] == '<') ||
				(categoryName[i] == '>') ||
				(categoryName[i] == ':') ||
				(categoryName[i] == '"') ||
				(categoryName[i] == '\\') ||
				(categoryName[i] == '|'))
			{
				categoryName[i] = ' ';
			}
		}
	}

	// check to see if the category already exists
	int categoryIndex = -1;
	for (int i = 0; i < urlCategoriesEndIndex; i++)
	{
		if (ULib.StringLibrary.isStringMatch(urlCategories[i].categoryName, categoryName))
		{
			categoryIndex = i;
			break;
		}
	}

	// if category doesn't exist, create file and open handle
	if (categoryIndex == -1)
	{
		categoryIndex = urlCategoriesEndIndex++;

		// create category filename
		char* categoryFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(categoryFilename, URL_LIST_CATEGORIES_DIRECTORY);
		ULib.StringLibrary.appendString(categoryFilename, "\\");
		ULib.StringLibrary.appendString(categoryFilename, categoryName);
		ULib.StringLibrary.appendString(categoryFilename, ".txt");

		// reached category limit
		if (urlCategoriesEndIndex > urlCategoriesMaximumIndex) return false;

		// store category name
		urlCategories[categoryIndex].categoryName = (char*)ULib.HeapLibrary.allocPrivateHeap((ULib.StringLibrary.getStringLength(categoryName) + 1));
		ULib.StringLibrary.copyString(urlCategories[categoryIndex].categoryName, categoryName);

		// store file handle
		urlCategories[categoryIndex].fileHandle = CreateFile(categoryFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		ULib.HeapLibrary.freePrivateHeap(categoryFilename);
		if (urlCategories[categoryIndex].fileHandle == INVALID_HANDLE_VALUE) return false;
	}

	// store url into category file
	unsigned long actuallyWritten = 0;
	if (WriteFile(urlCategories[categoryIndex].fileHandle, fullUrl, ULib.StringLibrary.getStringLength(fullUrl), &actuallyWritten, NULL) == 0) return false;
	if (WriteFile(urlCategories[categoryIndex].fileHandle, "\r\n", 2, &actuallyWritten, NULL) == 0) return false;
	return true;
}

////////////////////////////////////////////////////////////////////
// performs cleanup after categorisation, i.e. close file handles //
////////////////////////////////////////////////////////////////////
void storeUrlInCategoryCleanup(void)
{
	for (int i = 0; i < urlCategoriesEndIndex; i++)
	{
		if (urlCategories[i].fileHandle != INVALID_HANDLE_VALUE) CloseHandle(urlCategories[i].fileHandle);
		ULib.HeapLibrary.freePrivateHeap(urlCategories[i].categoryName);
	}
}

//////////////////////////////////////////////////////////////////
// opens the database file, decrypts and stores url information //
//////////////////////////////////////////////////////////////////
void openFileAndDecrypt(char* fileName, bool decryptURLs)
{
	// get the file header
	unsigned char* fileHeader = ULib.FileLibrary.readBufferFromFile(fileName, 0x40);
	if (fileHeader == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	char* fileHeaderInformation = ULib.StringLibrary.replaceCharacterInString((char*)fileHeader, 0x0A, 0x00);
	int fileHeaderInformationLength = ULib.StringLibrary.getStringLength(fileHeaderInformation);

	// get the string elements from file header
	char* stringElements[16];
	int numberOfStringElements = 0;
	ULib.StringLibrary.getStringElementsFromString(fileHeaderInformation, stringElements, &numberOfStringElements, 16, ' ');
	if (numberOfStringElements <= 5) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// get header information
	int headerFormatVersion = ULib.StringLibrary.getDWORDFromString(stringElements[1]);
	char* headerRevisionVersion = stringElements[2];
	char* headerRevisionDate = (char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 3; i < numberOfStringElements; i++)
	{
		ULib.StringLibrary.appendString(headerRevisionDate, stringElements[i]);
		ULib.StringLibrary.appendString(headerRevisionDate, " ");
	}
	// check for starting # value
	if (fileHeader[0] != '#') ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	// invalid format version
	if ((headerFormatVersion != 2) && (headerFormatVersion != 21)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// display file header information
	ULib.ConsoleLibrary.displayConsoleMessage("[Header Information]");
	if (headerFormatVersion == 2) ULib.ConsoleLibrary.displayConsoleMessage("  Format Version    : 2.0");
	else if (headerFormatVersion == 21) ULib.ConsoleLibrary.displayConsoleMessage("  Format Version    : 2.1");
	ULib.ConsoleLibrary.displayConsoleMessage("  Revision Version  : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(headerRevisionVersion);
	ULib.ConsoleLibrary.displayConsoleMessage("  Revision Date     : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(headerRevisionDate);
	ULib.HeapLibrary.freePrivateHeap(headerRevisionDate);
	ULib.HeapLibrary.freePrivateHeap(fileHeaderInformation);
	ULib.HeapLibrary.freePrivateHeap(fileHeader);
	for (int i = 0; i < numberOfStringElements; i++) ULib.HeapLibrary.freePrivateHeap(stringElements[i]);

	// decrypt urls, if required
	if (decryptURLs)
	{
		ULib.ConsoleLibrary.displayConsoleMessage((DWORD)fileHeaderInformationLength, true);

		// read in url list database
		ULib.ConsoleLibrary.displayConsoleMessage("\r\n  Reading URL List  : ....", false);
		unsigned long dbListSize = 0;
		unsigned long dbIndex = 0;
		unsigned char* dbListBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &dbListSize);
		ULib.ConsoleLibrary.displayConsoleMessage("\b\b\b\bdone");
		if ((dbListBuffer == NULL) || (dbListSize <= 0x1000)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

		// decrypt database
		ULib.ConsoleLibrary.displayConsoleMessage("  Decrypting List   : ....", false);
		// find starting point
		unsigned char bufferMatch[5] = {0x0A,0x03,0xD2,0xD2,0x03};
		for (dbIndex = fileHeaderInformationLength; dbIndex <= 0x500; dbIndex++)
		{
			if (ULib.StringLibrary.isBufferMatch((unsigned char*)&dbListBuffer[dbIndex], (unsigned char*)&bufferMatch[0], 5)) break;
		}
		// not able to find the starting point
		if (dbIndex >= 0x500)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("\r\n");
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		}
		ULib.ConsoleLibrary.displayConsoleMessage((DWORD)dbIndex, true);
		// decrypt the rest of the database
		for (--dbIndex; dbIndex < dbListSize; dbIndex++)
		{
			// end of line (encrypted)
			if (dbListBuffer[dbIndex] == 0xA0) dbListBuffer[dbIndex] = 0x00;
			else dbListBuffer[dbIndex] = (unsigned char)(((dbListBuffer[dbIndex] >> 4) | (dbListBuffer[dbIndex] << 4)) & 0xFF);
		}
		ULib.FileLibrary.writeBufferToFile("out.dat", dbListBuffer, dbListSize);
		ULib.ConsoleLibrary.displayConsoleMessage("\b\b\b\bdone");

		// create url list categories directory
		CreateDirectory(URL_LIST_CATEGORIES_DIRECTORY, NULL);

		// store each url into different categories
		ULib.ConsoleLibrary.displayConsoleMessage("  Categorising List : ....", false);
		char* currentBufferPosition = (char*)&dbListBuffer[fileHeaderInformationLength];
		char* currentUrl = NULL;
		char* currentCategory = NULL;
		char* currentCategoryStart = NULL;

		do
		{
			currentUrl = ULib.StringLibrary.replaceCharacterInString(currentBufferPosition, 0x1C, 0x00);
			currentCategory = ULib.StringLibrary.replaceCharacterInString(currentBufferPosition, 0x1C, 0x00);
			currentCategoryStart = currentCategory;

			// store url into all categories
			do
			{
				currentCategory = ULib.StringLibrary.getNextTokenString(currentCategory, 0x00);
				if ((currentCategory == NULL) || (currentCategory[0] < 0x20)) break;
				if (!storeUrlInCategory(currentUrl, currentCategory))
				{
					storeUrlInCategoryCleanup();
					ULib.ConsoleLibrary.displayConsoleMessage("\r\n");
					ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
				}

			} while (currentCategory != NULL);

			if (currentUrl != NULL) ULib.HeapLibrary.freePrivateHeap(currentUrl);
			if (currentCategoryStart != NULL) ULib.HeapLibrary.freePrivateHeap(currentCategoryStart);

			currentBufferPosition = ULib.StringLibrary.getNextTokenString(currentBufferPosition, 0x00);

		} while (currentBufferPosition != NULL);

		ULib.ConsoleLibrary.displayConsoleMessage("\b\b\b\bdone");
		ULib.HeapLibrary.freePrivateHeap(dbListBuffer);
		storeUrlInCategoryCleanup();
	}
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Norton Internet Security URL List Decryption Utility", "2.01", 2001);

	// if arguments are passed
	if (argc > 1)
	{
		if ((argc > 2) && (ULib.StringLibrary.isStringMatch(argv[2], "/decryptURLs"))) openFileAndDecrypt(argv[1], true);
		else openFileAndDecrypt(argv[1], false);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tnisUrlList <encrypted url list database file> [/decryptURLs]");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}