/////////////////////////////////////////////
// Peachtree Accounting Access Key Finder  //
// (C)thewd, thewd@hotmail.com             //
/////////////////////////////////////////////
/////////////////////////////////////////////
// Determines the Peachtree Access Key for //
// a particular security code              //
//                                         //
// Tested with...                          //
// - v7.00                                 //
// - v8.00                                 //
// - v9.00 (2002)                          //
// - v10.00 (2003)                         //
// - v11.00 (2004)                         //
// - v12.00 (2005)                         //
// - v13.00 (2006)                         //
// - v14.00 (2007)                         //
/////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

CUtilitiesLibrary ULib;

char* errorMessages[] =
{
	"Error occurred while converting the Security Code",
	"Error occurred while generating the correct Access Key"
};

//////////////////////////////////////////////////////
// uses the internal Peachtree algorithm to encrypt //
// the security code, which gives the Access Key    //
//////////////////////////////////////////////////////
unsigned long getAccessKey(unsigned long securityCode)
{
	return ((securityCode % 0xEA62) * ((securityCode + 1) % 0x1179A));
}

//////////////////////////////////////////////
// finds the access key for a corresponding //
// security code and displays the results   //
//////////////////////////////////////////////
void recoverAccessKey(char* securityCode)
{
	unsigned long securityCode_UL = 0;
	unsigned long accessKey_UL = 0;
	char* accessKey = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* accessKeyConverted = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* endCharacter;

	securityCode_UL = strtoul(securityCode, &endCharacter, 0x24);
	if ((securityCode_UL == NULL) || (securityCode_UL == -1)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	accessKey_UL = getAccessKey(securityCode_UL);
	_ultoa(accessKey_UL, accessKey, 0x24);

	if ((accessKey_UL == NULL) ||
		(accessKey_UL == -1) ||
		(accessKey == NULL))
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
	}

	accessKeyConverted[0] = accessKey[ULib.StringLibrary.getStringLength(accessKey) - 1];
	ULib.StringLibrary.copyString(&accessKeyConverted[1], accessKey, ULib.StringLibrary.getStringLength(accessKey) - 1);

	ULib.ConsoleLibrary.displayConsoleMessage("Security Code\t: ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(securityCode);
	ULib.ConsoleLibrary.displayConsoleMessage("Access Key\t: ", false);
	ULib.ConsoleLibrary.displayConsoleMessage(accessKeyConverted);
	ULib.HeapLibrary.freePrivateHeap(accessKey);
	ULib.HeapLibrary.freePrivateHeap(accessKeyConverted);
	MessageBeep(MB_ICONEXCLAMATION);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Peachtree Accounting Access Key Finder", "1.00", 2000);

	// if arguments are passed
	if (argc > 1) recoverAccessKey(argv[1]);
	else ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tPTreeCrk <peachtree security code>");

	ULib.ProcessLibrary.exitProcessHandler();
}