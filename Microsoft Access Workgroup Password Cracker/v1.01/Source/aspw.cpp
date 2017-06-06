////////////////////////////////////////////////////////////
// Microsoft Access Workgroup Password Cracker            //
// (C)thewd, thewd@hotmail.com                            //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Recovers account names, passwords, pids and sids from  //
// protected Access Workgroup Information Files (*.mdw)   //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Tested with Jet Database Engines...                    //
// [Access 95]                                            //
// v3.00.2120.0, v3.00.2809.0, v3.00.4513.0               //
// [Access 97]                                            //
// v3.50.3428.0, v3.50.3907.5, v3.51.0623.4, v3.51.2026.0 //
// v3.51.2723.0, v3.51.3203.0, v3.51.3328.0               //
// [Access 2000/2002/2003]                                //
// v4.00.2521.8, v4.00.2927.4, v4.00.3714.7, v4.00.4431.4 //
// v4.00.6218.0, v4.00.7328.0                             //
////////////////////////////////////////////////////////////
#include "aspw.h"
#include "aspwsupport.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;
bool DEBUG_MODE = false;

char* errorMessages[] =
{
	"Unable to open or read the Access Workgroup file",
	"Error reading data from the Access Workgroup file",
	"Doesn't appear to be a valid Access Workgroup file",
};

//////////////////////////////////////////////////////
// recovers the password from the database using    //
// the decryption table. Also recovers Account Name //
//////////////////////////////////////////////////////
void displayAccessSecurity(unsigned char* fileBuffer, int fileSize, int databaseFormat)
{
	int baseOffset = ((databaseFormat == Access200XFormat) ? 0x15000 : 0xE000);
	int fileOffset = ((databaseFormat == Access200XFormat) ? 0x0C : 0x08);

	// check filesize
	if (fileSize < (baseOffset + 0x1500)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	// decrypt account section
	doRC4(fileBuffer, baseOffset, true);

	// number of accounts and determine first account length
	int firstAccountLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, baseOffset + 4);
	if (databaseFormat == Access200XFormat) firstAccountLength *= 2;
	int accountLength = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, baseOffset + fileOffset);
	fileOffset += 2;

	int accountOffsets[1024][2];
	int previousAccountOffset;
	int index = 0;

	// special case for first account (account that contains owner information)
	if (accountLength > 0)
	{
		accountOffsets[index][0] = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, baseOffset + fileOffset) & 0x3FFF;
		accountOffsets[index][1] = firstAccountLength;
		index++;
	}

	// determine file offsets to the accounts
	for (int loopCount = 1; loopCount < accountLength; loopCount++)
	{
		previousAccountOffset = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, baseOffset + fileOffset) & 0x3FFF;
		fileOffset += 2;

		accountOffsets[index][0] = 0;
		accountOffsets[index][1] = 0;

		if (fileBuffer[baseOffset + fileOffset + 1] <= 0x0F)
		{
			accountOffsets[index][0] = ULib.StringLibrary.getWORDFromBuffer(fileBuffer, baseOffset + fileOffset) & 0x3FFF;
			accountOffsets[index][1] = (previousAccountOffset - accountOffsets[index][0] - 1) & 0x3FFF;
			if ((accountOffsets[index][0] == 0x00) || (accountOffsets[index][1] == 0x00)) break;
			index++;
		}

		// maximum 1024 accounts
		if (index >= 1024) break;
	}

	int finalAccountIndex = index;

	// display workgroup owner information
	if (accountLength > 0)
	{
		char* workgroupName = NULL;
		char* workgroupOrganisation = NULL;
		char* workgroupId = NULL;
		int workgroupInformationOffset = accountOffsets[0][0] + firstAccountLength + 1;

		recoverWorkgroupInformation(fileBuffer, baseOffset + workgroupInformationOffset,
			databaseFormat, &workgroupName, &workgroupOrganisation, &workgroupId);

		ULib.ConsoleLibrary.displayConsoleMessage("");
		ULib.ConsoleLibrary.displayConsoleMessage("[Workgroup Information]");
		if (DEBUG_MODE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Offsets      : 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(workgroupInformationOffset, true, true, ULib.ConsoleLibrary.DISPLAY_AS_WORD);
		}
		ULib.ConsoleLibrary.displayConsoleMessage("Name         : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(workgroupName);
		ULib.ConsoleLibrary.displayConsoleMessage("Organisation : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(workgroupOrganisation);
		ULib.ConsoleLibrary.displayConsoleMessage("Workgroup Id : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(workgroupId);

		ULib.ConsoleLibrary.displayConsoleMessage("");
		ULib.ConsoleLibrary.displayConsoleMessage("(C)ontinue or (E)xit...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 23, false);
		unsigned char consoleInput = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "CcEe");
		ULib.ConsoleLibrary.displayConsoleMessage(' ', 23, false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 23, false);
		if ((consoleInput == 'E') || (consoleInput == 'e')) finalAccountIndex = -1;
	}

	// loop through each account
	for (index = 0; index < finalAccountIndex; index++)
	{
		int accountNameOffset = 0;
		int accountNameLength = 0;
		int passwordOffset = 0;
		int passwordLength = 0;
		int pidOffset = 0;
		int pidLength = 0;
		bool isUserAccount = true;
		bool isBuiltInUserAccount = false;
		bool isBuiltInGroupAccount = false;
		bool isBuiltInAdminAccount = false;

		if (databaseFormat == Access200XFormat)
		{
			accountNameOffset = 2;
			accountNameLength = fileBuffer[baseOffset + accountOffsets[index][0] + accountOffsets[index][1] - 6] - 2;
			passwordOffset = fileBuffer[baseOffset + accountOffsets[index][0] + accountOffsets[index][1] - 8];
			passwordLength = 0x28;
			pidOffset = fileBuffer[baseOffset + accountOffsets[index][0] + accountOffsets[index][1] - 6];
			pidLength = passwordOffset - pidOffset;
		}
		else
		{
			accountNameOffset = 1;
			accountNameLength = fileBuffer[baseOffset + accountOffsets[index][0] + accountOffsets[index][1] - 3] - 1;
			passwordOffset = fileBuffer[baseOffset + accountOffsets[index][0] + accountOffsets[index][1] - 4];
			passwordLength = 0x10;
			pidOffset = fileBuffer[baseOffset + accountOffsets[index][0] + accountOffsets[index][1] - 3];
			pidLength = passwordOffset - pidOffset;
		}

		// special case for first account (account that contains owner information)
		if (index == 0)
		{
			accountNameLength = firstAccountLength;
			passwordOffset = firstAccountLength;
			passwordLength = 1;
			pidOffset = 0;
			pidLength = 0;
		}

		// no password length
		if (passwordLength <= 0) continue;
		// is group account?
		if ((passwordOffset + passwordLength) > accountOffsets[index][1]) isUserAccount = false;

		//ULib.ConsoleLibrary.displayConsoleMessage("");
		if (index == 0) ULib.ConsoleLibrary.displayConsoleMessage("[Account Information]");

		// account offsets
		if (DEBUG_MODE)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Offsets      : 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(accountOffsets[index][0], true, false, ULib.ConsoleLibrary.DISPLAY_AS_WORD);
			ULib.ConsoleLibrary.displayConsoleMessage(".0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(accountOffsets[index][1], true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(" 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(accountNameOffset, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(".0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(accountNameLength, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			if (isUserAccount)
			{
				ULib.ConsoleLibrary.displayConsoleMessage(" 0x", false);
				ULib.ConsoleLibrary.displayConsoleMessage(passwordOffset, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
				ULib.ConsoleLibrary.displayConsoleMessage(".0x", false);
				ULib.ConsoleLibrary.displayConsoleMessage(passwordLength, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			}
			ULib.ConsoleLibrary.displayConsoleMessage(" 0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(pidOffset, true, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(".0x", false);
			ULib.ConsoleLibrary.displayConsoleMessage(pidLength, true, true, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
		}

		// get account name (check for recognised users and groups)
		char* accountName = recoverAccountName(fileBuffer, (baseOffset + accountOffsets[index][0]), accountNameLength, databaseFormat);
		if (ULib.StringLibrary.isStringMatch(accountName, "admin")) isBuiltInAdminAccount = true;
		else if ((ULib.StringLibrary.isStringMatch(accountName, "engine")) || (ULib.StringLibrary.isStringMatch(accountName, "creator"))) isBuiltInUserAccount = true;
		else if ((ULib.StringLibrary.isStringMatch(accountName, "admins")) || (ULib.StringLibrary.isStringMatch(accountName, "users"))) isBuiltInGroupAccount = true;

		// account type
		ULib.ConsoleLibrary.displayConsoleMessage("Account Type : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(((isBuiltInUserAccount) ? "User (Internal)" : ((isBuiltInAdminAccount) ? "User (Built-in)" : ((isUserAccount) ? "User" : ((isBuiltInGroupAccount) ? "Group (Built-in)" : "Group")))));

		// account name
		ULib.ConsoleLibrary.displayConsoleMessage((((isUserAccount) || (isBuiltInUserAccount)) ? "User Name    : " : "Group Name   : "), false);
		ULib.ConsoleLibrary.displayConsoleMessage(accountName);
		if (accountName != NULL) ULib.HeapLibrary.freePrivateHeap(accountName);

		// account password
		if (isUserAccount)
		{
			char* recoveredPassword = recoverPassword(fileBuffer, (baseOffset + accountOffsets[index][0] + passwordOffset), databaseFormat);
			ULib.ConsoleLibrary.displayConsoleMessage("Password     : ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(((!ULib.StringLibrary.isString(recoveredPassword, 1)) ? "<blank>" : recoveredPassword));
			if ((recoveredPassword != NULL) && (databaseFormat == Access200XFormat)) ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
		}

		// personal id
		char* recoveredPID = ((pidLength <= 2) ? NULL : recoverPID(fileBuffer, (baseOffset + accountOffsets[index][0] + pidOffset), pidLength));
		ULib.ConsoleLibrary.displayConsoleMessage("Personal Id  : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(((!ULib.StringLibrary.isString(recoveredPID, 1)) ? "<blank>" : recoveredPID));
		if (recoveredPID != NULL) ULib.HeapLibrary.freePrivateHeap(recoveredPID);

		// security id
		/*char* recoveredSID = ((pidLength <= 2) ? NULL : recoverPID(fileBuffer, (baseOffset + accountOffsets[index][0] + pidOffset), pidLength));
		ULib.ConsoleLibrary.displayConsoleMessage("Security Id  : ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(((!ULib.StringLibrary.isString(recoveredSID, 1)) ? "<blank>" : recoveredSID));
		if (recoveredSID != NULL) ULib.HeapLibrary.freePrivateHeap(recoveredSID);*/

		if (index < (finalAccountIndex - 1))
		{
			ULib.ConsoleLibrary.displayConsoleMessage("");
			ULib.ConsoleLibrary.displayConsoleMessage("(C)ontinue or (E)xit...", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 23, false);
			unsigned char consoleInput = ULib.ConsoleLibrary.getConsoleInputAsKeyPress(false, "CcEe");
			ULib.ConsoleLibrary.displayConsoleMessage(' ', 23, false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 23, false);
			if ((consoleInput == 'E') || (consoleInput == 'e')) break;
		}
	}
}

////////////////////////////////////////////
// load this file, determine Jet version, //
// and decrypt the account sections       //
////////////////////////////////////////////
void parseFileAndDecrypt(char* fileName)
{
	unsigned long fileSize = 0;
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &fileSize);
	if ((fileBuffer == NULL) || (fileSize <= 0x100)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	if (!isJetWorkgroup(fileBuffer)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);

	ULib.ConsoleLibrary.displayConsoleMessage("Parsing Workgroup Information File (Access ", false);

	// determine Jet Engine version
	int databaseFormat;
	// v4.00 (Access 2000/2002/2003)
	if ((fileBuffer[0x9C + 0] == 0x34) &&
		(fileBuffer[0x9C + 1] == 0x2E) &&
		(fileBuffer[0x9C + 2] == 0x30))
	{
		databaseFormat = Access200XFormat;
		ULib.ConsoleLibrary.displayConsoleMessage("2000/2002/2003)...");
	}
	// v3.xx (Access 95/97)
	else
	{
		databaseFormat = Access9597Format;
		ULib.ConsoleLibrary.displayConsoleMessage("95/97)...");
	}

	// recover and display User-Level accounts
	displayAccessSecurity(fileBuffer, fileSize, databaseFormat);
	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Microsoft Access Workgroup Password Cracker", "1.01", 1999);

	// if arguments are passed
	if (argc > 1)
	{
		// check whether the shift key is depressed (enable debug mode)
		DEBUG_MODE = ULib.ProcessLibrary.isDebugModeEnabled();
		parseFileAndDecrypt(argv[1]);
	}
	else
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage:\taspw <workgroup information file>");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}