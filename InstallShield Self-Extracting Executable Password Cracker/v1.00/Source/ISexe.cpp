////////////////////////////////////////////////////////////////
// InstallShield Self-Extracting Executable Password Cracker  //
// (C)thewd, thewd@hotmail.com                                //
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// Recovers passwords for self-extracting executables         //
// created with InstallShield EXE Builder or                  //
// InstallShield Express (Both 16-bit and 32-bit versions)    //
//                                                            //
// Tested with...                                             //
// - InstallShield EXE Builder v1.60                          //
// - InstallShield Express v1.11                              //
// - InstallShield Express v2.04, v2.11, v2.12, v2.13         //
////////////////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Unable to open or read the InstallShield file executable",
	"Unknown file format (Only PE and NE formats are supported)",
	"Doesn't appear to be a supported InstallShield Self-Extracting File",
	"The installation is not password protected"
};

#define MAX_PASSWORD_LENGTH		50

unsigned char encryptedPassword[MAX_PASSWORD_LENGTH];
char decryptedPassword[MAX_PASSWORD_LENGTH];
unsigned char encryptedProgramTitle[MAX_PASSWORD_LENGTH];
char decryptedProgramTitle[MAX_PASSWORD_LENGTH];

//////////////////////////////////////////////////////
// if an encrypted password is found, then copy the //
// bytes into the encryptedPassword integer array   //
//////////////////////////////////////////////////////
void storeEncryptedPasswordBytes(unsigned char* pMem)
{
	int baseOffset = 0x1C;
	if (pMem[baseOffset] == 0xCA) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
	for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) encryptedPassword[i] = pMem[baseOffset + i];
}

///////////////////////////////////////////////////////////
// if an encrypted program title is found, then copy the //
// bytes into the encryptedProgramTitle integer array    //
///////////////////////////////////////////////////////////
void storeEncryptedProgramTitleBytes(unsigned char* pMem)
{
	const int baseOffset = 0xC4;
	if (pMem[baseOffset] == 0xCA) encryptedProgramTitle[0] = NULL;
	else for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) encryptedProgramTitle[i] = pMem[baseOffset + i];
}

///////////////////////////////////////////////////
// if the installation executable is a PE format //
// file then retrieve the MYRESOURCE resource    //
///////////////////////////////////////////////////
void retrievePasswordResourceFromPEFileFormat(char* fileName)
{
	unsigned char* resourceStream = ULib.FileLibrary.getResourceFromFile(fileName, "MYRESOURCE", "#1024");
	if (resourceStream == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
	storeEncryptedPasswordBytes(resourceStream);
	storeEncryptedProgramTitleBytes(resourceStream);
}

///////////////////////////////////////////////////
// if the installation executable is a NE format //
// file then retrieve the MYRESOURCE resource    //
///////////////////////////////////////////////////
void retrievePasswordResourceFromNEFileFormat(char* fileName)
{
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize <= 0)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	unsigned long index = 0;
	bool isInstallShieldExecutable = false;

	while ((isInstallShieldExecutable == false) && (index < 0x400))
	{
		if ((fileBuffer[index + 0] == 'M') &&
			(fileBuffer[index + 1] == 'Y') &&
			(fileBuffer[index + 2] == 'R') &&
			(fileBuffer[index + 3] == 'E') &&
			(fileBuffer[index + 4] == 'S') &&
			(fileBuffer[index + 5] == 'O') &&
			(fileBuffer[index + 6] == 'U') &&
			(fileBuffer[index + 7] == 'R') &&
			(fileBuffer[index + 8] == 'C') &&
			(fileBuffer[index + 9] == 'E'))
		{
			isInstallShieldExecutable = true;
		}

		index++;
	}

	if (isInstallShieldExecutable == false) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	bool foundPasswordResource = false;
	index = 0;

	while ((foundPasswordResource == false) && (index < (int)(fileSize - 1024)))
	{
		if ((fileBuffer[index] == 0x94) &&
			(fileBuffer[index + 1] == 0x01) &&
			(fileBuffer[index + 2] == 0x00) &&
			(fileBuffer[index + 3] == 0x00) &&
			(fileBuffer[index + 120] == 0x4A) &&
			(fileBuffer[index + 120 + 1] == 0x76) &&
			(fileBuffer[index + 120 + 2] == 0x3E) &&
			(fileBuffer[index + 120 + 3] == 0xA0))
		{
			storeEncryptedPasswordBytes(&fileBuffer[index]);
			storeEncryptedProgramTitleBytes(&fileBuffer[index]);
			foundPasswordResource = true;
		}

		index++;
	}

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////////////////////////////
// calls the appropriate method for getting the   //
// MYRESOURCE resource from either PE or NE files //
////////////////////////////////////////////////////
void retrievePasswordResource(char* fileName, int fileFormat)
{
	if (fileFormat == ULib.FileLibrary.FILEFORMAT_PE) retrievePasswordResourceFromPEFileFormat(fileName);
	else if (fileFormat == ULib.FileLibrary.FILEFORMAT_NE) retrievePasswordResourceFromNEFileFormat(fileName);
	else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
}

////////////////////////////////////////////////////////
// this method is used to decrypt the encrypted bytes //
// within the MYRESOURCE resource. This applies to    //
// both password and program title encrypted bytes    //
////////////////////////////////////////////////////////
void decryptBytes(bool recoverPassword)
{
	const int XORValues[8] = {0xB3, 0xF2, 0xEA, 0x1F, 0xAA, 0x27, 0x66, 0x13};
	const int XORValuesSize = 7;

	int currentXORValuesPosition = 0;
	int currentPosition = 0;
	bool finished = false;
	int currentValue;
	int x1, x2;

	while ((currentPosition < MAX_PASSWORD_LENGTH) && (finished == false))
	{
		currentValue = XORValuesSize - currentXORValuesPosition;
		x1 = currentValue;

		currentValue = XORValues[x1];
		if (recoverPassword) x2 = currentValue ^ encryptedPassword[currentPosition];
		else  x2 = currentValue ^ encryptedProgramTitle[currentPosition];

		__asm
		{
				push ecx
				mov ecx, x1
				ror byte ptr [x2], cl
				pop ecx
		}

		currentValue = XORValues[currentXORValuesPosition];

		if (recoverPassword)
		{
			decryptedPassword[currentPosition] = (char)(x2 ^ currentValue);
			if (decryptedPassword[currentPosition] == 0x00) finished = true;
		}
		else
		{
			decryptedProgramTitle[currentPosition] = (char)(x2 ^ currentValue);
			if (decryptedProgramTitle[currentPosition] == 0x00) finished = true;
		}

		currentPosition++;
		currentXORValuesPosition++;

		if (currentXORValuesPosition > XORValuesSize) currentXORValuesPosition = 0;
	}
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("InstallShield Self-Extracting Executable Password Cracker", "1.00", 1999);

	if (argc > 1)
	{
		int fileFormat = ULib.FileLibrary.determineFileFormat(argv[1]);
		if (fileFormat == ULib.FileLibrary.FILEFORMAT_UNKNOWN) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
		retrievePasswordResource(argv[1], fileFormat);

		// decrypt password
		decryptBytes(true);

		// decrypt program title if available
		if (encryptedProgramTitle[0] != NULL) decryptBytes(false);

		char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(argv[1], true);
		ULib.ConsoleLibrary.displayConsoleMessage("Application Filename\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(fileNameStripped);
		ULib.ConsoleLibrary.displayConsoleMessage("Program Title\t\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(decryptedProgramTitle);
		ULib.ConsoleLibrary.displayConsoleMessage("");
		ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password\t: '", false);
		ULib.ConsoleLibrary.displayConsoleMessage(decryptedPassword, false);
		ULib.ConsoleLibrary.displayConsoleMessage("'");
		MessageBeep(MB_ICONEXCLAMATION);
		ULib.HeapLibrary.freePrivateHeap(fileNameStripped);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tISexe <self-extracting filename>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}