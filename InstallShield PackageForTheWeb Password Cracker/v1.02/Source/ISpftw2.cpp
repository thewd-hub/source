///////////////////////////////////////////////////////
// InstallShield PackageForTheWeb Password Cracker   //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// Support Routines for PFTW v2.00 - v2.02           //
///////////////////////////////////////////////////////
#include <windows.h>
#include "ISpftw2.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;
extern char* errorMessages[];

/////////////////////////////////////////////////////
// determines the CRC of the current password      //
// generated for comparison with the encrypted CRC //
/////////////////////////////////////////////////////
int getCRCCode(char* currentPassword, int currentPasswordLength)
{
	int baseCRC = 0xF143AC;
	for (int index = 0; index < currentPasswordLength; index++) baseCRC += (currentPassword[index] * 2);
	return baseCRC;
}

//////////////////////////////////////////////////
// brute-force approach to finding the password //
//////////////////////////////////////////////////
char* findPassword_v2(int passwordCRC)
{
	int searchValue = passwordCRC ^ 0x51993;
	char* possiblePassword = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
	possiblePassword[0] = 'z';
	int index = 1;
	int currentValue = searchValue - getCRCCode(possiblePassword, index);

	while ((currentValue > 0) && (index < 100))
	{
		currentValue = searchValue - getCRCCode(possiblePassword, index);
		if (currentValue > 0) possiblePassword[index++] = 'z';
	}

	possiblePassword[index] = NULL;
	int passwordLength = index;

	index = 0;
	while ((currentValue != 0) && (possiblePassword[0] >= 0x20))
	{
		possiblePassword[index] = (char)(possiblePassword[index] - 1);
		currentValue = searchValue - getCRCCode(possiblePassword, passwordLength);
		if (++index >= passwordLength) index = 0;
	}

	if (possiblePassword[0] < 0x20) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
	return possiblePassword;
}