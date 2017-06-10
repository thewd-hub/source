///////////////////////////////////////////////
// CentaMeter File Information               //
// (C)thewd, thewd@hotmail.com               //
///////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

// standard error messages
char* errorMessages[] =
{
	"Unable to open or read the licence file",
	"Unable to continue - no valid licences are found",
	"Unable to open or read the queue file",
	"Unable to continue - no valid queue items are found",
	"Unable to open or read the log file",
	"Unable to continue - invalid log file"
};

CUtilitiesLibrary ULib;

//////////////////////////////////
// decrypts the supplied string //
//////////////////////////////////
void decryptString(unsigned char* fileBuffer, int fileLength, unsigned long keyValue)
{
	for (int i = -1; i < fileLength; i++)
	{
		keyValue *= 0x15A4E35;
		keyValue++;
		if (i >= 0) fileBuffer[i] = (unsigned char)(fileBuffer[i] - (unsigned char)(((keyValue >> 16) & 0x7FFF) % 0xFF));
	}
}

////////////////////////////////////////////////////////
// decrypts all the events stored within the log file //
////////////////////////////////////////////////////////
void decryptLogFile(char* fileName, bool dumpRaw)
{
	// read in the log file
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize < 4)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	if (ULib.StringLibrary.getWORDFromBuffer(fileBuffer) != 0x0000) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);

	// display log information
	char* decryptedFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(decryptedFilename, "%s.txt", fileName);
	char* decryptedFilenameStripped = ULib.FileLibrary.stripPathFromFilename(decryptedFilename, true);
	ULib.ConsoleLibrary.displayConsoleMessage("Decrypted Filename : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(decryptedFilenameStripped);
	ULib.HeapLibrary.freePrivateHeap(decryptedFilenameStripped);

	// store the log header
	HANDLE fileHandle = NULL;
	unsigned long currentOffset = 0x02;
	unsigned long currentLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset);
	currentOffset += 2;
	if ((currentOffset >= fileSize) || (currentOffset + currentLength >= fileSize)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);
	ULib.FileLibrary.writeBufferToFile(&fileHandle, decryptedFilename, (unsigned char*)&fileBuffer[currentOffset], currentLength + 2);
	currentOffset += (currentLength + 2);

	int numberOfStringElements = 0;
	char* stringElements[10];
	char* outputBuffer = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// decrypt the event logs
	for (; currentOffset < (fileSize - 2);)
	{
		// check for correct event header
		if (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset) != 0x0118) break;
		// get event information
		currentOffset += 2;
		unsigned long currentKeyValue = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset);
		currentOffset += 2;
		currentLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset);
		currentOffset += 2;
		// decrypt the event
		decryptString((unsigned char*)&fileBuffer[currentOffset], currentLength, currentKeyValue);

		// get event elements
		if (dumpRaw) ULib.FileLibrary.appendBufferToFile(&fileHandle, NULL, (unsigned char*)&fileBuffer[currentOffset], (((currentOffset + currentLength) < (fileSize - 2)) ? currentLength + 2 : currentLength));
		else
		{
			ULib.StringLibrary.getStringElementsFromString((char*)&fileBuffer[currentOffset], stringElements, &numberOfStringElements, 10, ';');
			if (numberOfStringElements >= 9)
			{
				wsprintf(outputBuffer, "\r\n[%s %s]\r\n  Computer    : %s\r\n  User Name   : %s\r\n  Group Name  : %s\r\n  Application : %s\r\n  CTM Action  : %s\r\n  CTM Licence : %s\r\n", stringElements[0], stringElements[1], stringElements[2], stringElements[3], stringElements[4], stringElements[6], stringElements[7], stringElements[8]);
				ULib.FileLibrary.appendBufferToFile(&fileHandle, NULL, (unsigned char*)outputBuffer, ULib.StringLibrary.getStringLength(outputBuffer));
			}
			for (int i = (numberOfStringElements - 1); i >= 0; i--) ULib.HeapLibrary.freePrivateHeap(stringElements[i]);
		}

		currentOffset += (currentLength + 2);
	}

	ULib.HeapLibrary.freePrivateHeap(outputBuffer);
	ULib.HeapLibrary.freePrivateHeap(decryptedFilename);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
	CloseHandle(fileHandle);
}

////////////////////////////////////////////////////////
// decrypt all the licences within the licence file   //
// (%c%c%-8s%-6s%-8s%-4s %-14s%-64s%-64s%c%-8s%-128s) //
////////////////////////////////////////////////////////
void decryptLicenceFile(char* fileName)
{
	// read in the license file
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize < 6)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	// get number of licences and check validity
	int numberOfLicences = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, 2);
	if ((numberOfLicences <= 0) || (((numberOfLicences + 1) * 0x200) > (int)fileSize) || (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, 4) != 0x118)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);

	char* decryptedFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(decryptedFilename, "%s.decrypted", fileName);
	char* decryptedFilenameStripped = ULib.FileLibrary.stripPathFromFilename(decryptedFilename, true);

	// display licence information
	ULib.ConsoleLibrary.displayConsoleMessage("Number Of Licences : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((unsigned long)numberOfLicences);
	ULib.ConsoleLibrary.displayConsoleMessage("Decrypted Filename : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(decryptedFilenameStripped);
	ULib.HeapLibrary.freePrivateHeap(decryptedFilenameStripped);

	// decrypt the licences
	for (int i = 1; i <= numberOfLicences; i++)
	{
		int currentOffset = i << 9;
		unsigned long currentKeyValue = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, currentOffset + 0x34);
		decryptString((unsigned char*)&fileBuffer[currentOffset], 0x34, currentKeyValue);
		ULib.StringLibrary.addWORDToBuffer(fileBuffer, 0, currentOffset + 0x34);
		currentOffset += 0x36;
		decryptString((unsigned char*)&fileBuffer[currentOffset], 0x117, currentKeyValue);
	}

	// write decrypted buffer to file
	ULib.FileLibrary.writeBufferToFile(decryptedFilename, fileBuffer, fileSize);
	ULib.HeapLibrary.freePrivateHeap(decryptedFilename);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

/////////////////////////////////////////////////
// decrypt all the items within the queue file //
// (%c%c%-12s%-108s%c)                         //
/////////////////////////////////////////////////
void decryptQueueFile(char* fileName)
{
	// read in the queue file
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize < 6)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	// get number of queue items and check validity
	int numberOfQueueItems = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, 2);
	if ((numberOfQueueItems <= 0) || (((numberOfQueueItems + 1) * 0x80) > (int)fileSize) || (ULib.StringLibrary.getWORDFromBuffer(fileBuffer, 4) != 0x118)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	char* decryptedFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(decryptedFilename, "%s.decrypted", fileName);
	char* decryptedFilenameStripped = ULib.FileLibrary.stripPathFromFilename(decryptedFilename, true);

	// display licence information
	ULib.ConsoleLibrary.displayConsoleMessage("Number Of Queues   : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage((unsigned long)numberOfQueueItems);
	ULib.ConsoleLibrary.displayConsoleMessage("Decrypted Filename : ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(decryptedFilenameStripped);
	ULib.HeapLibrary.freePrivateHeap(decryptedFilenameStripped);

	// decrypt the queue items
	for (int i = 1; i <= numberOfQueueItems; i++)
	{
		int currentOffset = i << 7;
		unsigned long currentKeyValue = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, currentOffset);
		decryptString((unsigned char*)&fileBuffer[currentOffset + 4], 0x7B, currentKeyValue);
		ULib.StringLibrary.addWORDToBuffer(fileBuffer, 0, currentOffset);
	}

	// write decrypted buffer to file
	ULib.FileLibrary.writeBufferToFile(decryptedFilename, fileBuffer, fileSize);
	ULib.HeapLibrary.freePrivateHeap(decryptedFilename);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("CentaMeter File Information", "1.00", 2002);
	bool displayUsage = true;

	// if arguments are passed
	if (argc > 2)
	{
		if (ULib.StringLibrary.isStringMatch("/decryptLog", argv[1]))
		{
			displayUsage = false;
			decryptLogFile(argv[2], (((argc > 3) && (ULib.StringLibrary.isStringMatch("/dumpRaw", argv[3]))) ? true : false));
		}
		else if ((ULib.StringLibrary.isStringMatch("/decryptLicence", argv[1])) || (ULib.StringLibrary.isStringMatch("/decryptLicense", argv[1])))
		{
			displayUsage = false;
			decryptLicenceFile(argv[2]);
		}
		else if (ULib.StringLibrary.isStringMatch("/decryptQueue", argv[1]))
		{
			displayUsage = false;
			decryptQueueFile(argv[2]);
		}
	}

	// display usage information
	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : cmInfo [options]");
		ULib.ConsoleLibrary.displayConsoleMessage("Options:");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /decryptLog <log filename> [/dumpRaw]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /decryptLicence <licence filename>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /decryptQueue <queue filename>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}