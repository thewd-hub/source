//////////////////////////////////////////////////
// PowerQuest Drive Image Password Cracker      //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////
#include <windows.h>
#include "PQdi2.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;
const int XORvalues[4] = {0xD2, 0x84, 0x35, 0x7C};

///////////////////////////////////////////////////
// Determines the file system and partition type //
// of the partition compressed within the image  //
///////////////////////////////////////////////////
void determineImageType(int pType, char* filesystemType, int fsType, char* partitionType)
{
	switch (fsType)
	{
		case 0x01:
		{
			if ((pType == 0x0B) || (pType == 0x0C) || (pType == 0x1B) || (pType == 0x1C)) ULib.StringLibrary.copyString(filesystemType, "FAT32");
			else ULib.StringLibrary.copyString(filesystemType, "FAT");
			break;
		}
		case 0x02: ULib.StringLibrary.copyString(filesystemType, "Xenix"); break;
		case 0x03: ULib.StringLibrary.copyString(filesystemType, "Free"); break;
		case 0x04: ULib.StringLibrary.copyString(filesystemType, "NTFS"); break;
		case 0x05: ULib.StringLibrary.copyString(filesystemType, "Extended"); break;
		case 0x06: ULib.StringLibrary.copyString(filesystemType, "EPBR"); break;
		case 0x07: ULib.StringLibrary.copyString(filesystemType, "HPFS"); break;
		case 0x08: ULib.StringLibrary.copyString(filesystemType, "BAD MBR"); break;
		case 0x09: ULib.StringLibrary.copyString(filesystemType, "Linux"); break;
		case 0x0A: ULib.StringLibrary.copyString(filesystemType, "OS/2 Boot"); break;
		case 0x0B: ULib.StringLibrary.copyString(filesystemType, "Linux / Minix"); break;
		case 0x0E: ULib.StringLibrary.copyString(filesystemType, "Flex"); break;
		case 0x0F: ULib.StringLibrary.copyString(filesystemType, "Netware"); break;
		case 0x10: ULib.StringLibrary.copyString(filesystemType, "Unformatted"); break;
		default: ULib.StringLibrary.copyString(filesystemType, "Unknown");
	}

	switch (pType)
	{
		case 0x01: ULib.StringLibrary.copyString(partitionType, "FAT 12-bit"); break;
		case 0x02: ULib.StringLibrary.copyString(partitionType, "Xenix"); break;
		case 0x04: ULib.StringLibrary.copyString(partitionType, "FAT 16-bit (<= 32MB)"); break;
		case 0x05: ULib.StringLibrary.copyString(partitionType, "Extended"); break;
		case 0x06: ULib.StringLibrary.copyString(partitionType, "FAT 16-bit (>= 32MB)"); break;
		case 0x07: ULib.StringLibrary.copyString(partitionType, "IFS"); break;
		case 0x08: ULib.StringLibrary.copyString(partitionType, "AIX Bootable"); break;
		case 0x09: ULib.StringLibrary.copyString(partitionType, "AIX Data"); break;
		case 0x0A: ULib.StringLibrary.copyString(partitionType, "OS/2 Boot Manager"); break;
		case 0x0B: ULib.StringLibrary.copyString(partitionType, "FAT 32-bit"); break;
		case 0x0C: ULib.StringLibrary.copyString(partitionType, "FAT 32-bit (Ext INT 13)"); break;
		case 0x0E: ULib.StringLibrary.copyString(partitionType, "FAT 16-bit (>= 32MB, Ext INT 13)"); break;
		case 0x0F: ULib.StringLibrary.copyString(partitionType, "Extended (Ext INT 13)"); break;
		case 0x10: ULib.StringLibrary.copyString(partitionType, "OPUS"); break;
		case 0x11: ULib.StringLibrary.copyString(partitionType, "FAT 12-bit (Hidden)"); break;
		case 0x14: ULib.StringLibrary.copyString(partitionType, "FAT 16-bit (Hidden, <= 32MB)"); break;
		case 0x16: ULib.StringLibrary.copyString(partitionType, "FAT 16-bit (Hidden, >= 32MB)"); break;
		case 0x17: ULib.StringLibrary.copyString(partitionType, "Hidden Installable File System"); break;
		case 0x18: ULib.StringLibrary.copyString(partitionType, "AST Windows Swap File"); break;
		case 0x1B: ULib.StringLibrary.copyString(partitionType, "FAT 32-bit (Hidden)"); break;
		case 0x1C: ULib.StringLibrary.copyString(partitionType, "FAT 32-bit (Ext INT 13)"); break;
		case 0x1E: ULib.StringLibrary.copyString(partitionType, "FAT 16-bit (Hidden, Ext INT 13)"); break;
		case 0x24: ULib.StringLibrary.copyString(partitionType, "NEC MS-DOS 3.x"); break;
		case 0x3C: ULib.StringLibrary.copyString(partitionType, "PowerQuest Flex"); break;
		case 0x40: ULib.StringLibrary.copyString(partitionType, "Venix 80286"); break;
		case 0x50: ULib.StringLibrary.copyString(partitionType, "Ontrack Disk Manager"); break;
		case 0x51: ULib.StringLibrary.copyString(partitionType, "Novell"); break;
		case 0x52: ULib.StringLibrary.copyString(partitionType, "CP/M"); break;
		case 0x54: ULib.StringLibrary.copyString(partitionType, "Ontrack Disk Manager 6.0 (DDO)"); break;
		case 0x55: ULib.StringLibrary.copyString(partitionType, "EZ-Drive 3.05"); break;
		case 0x56: ULib.StringLibrary.copyString(partitionType, "GoldenBow VFeature"); break;
		case 0x5C: ULib.StringLibrary.copyString(partitionType, "Priam EDISK"); break;
		case 0x61: ULib.StringLibrary.copyString(partitionType, "SpeedStor"); break;
		case 0x63: ULib.StringLibrary.copyString(partitionType, "GNU HURD"); break;
		case 0x64: ULib.StringLibrary.copyString(partitionType, "Novell Netware 286"); break;
		case 0x65: ULib.StringLibrary.copyString(partitionType, "Novell Netware (3.11 and 4.1)"); break;
		case 0x70: ULib.StringLibrary.copyString(partitionType, "DiskSecure Multi-Boot"); break;
		case 0x75: ULib.StringLibrary.copyString(partitionType, "PC/IX"); break;
		case 0x80: ULib.StringLibrary.copyString(partitionType, "Minix v1.1 - 1.4a"); break;
		case 0x81: ULib.StringLibrary.copyString(partitionType, "Linux/Minux v1.4b+"); break;
		case 0x82: ULib.StringLibrary.copyString(partitionType, "Linux Swap Partition"); break;
		case 0x83: ULib.StringLibrary.copyString(partitionType, "Linux Native File System"); break;
		case 0x84: ULib.StringLibrary.copyString(partitionType, "OS/2 Hiding Type 04h Partition"); break;
		case 0x86: ULib.StringLibrary.copyString(partitionType, "NT FAT Volume Set"); break;
		case 0x87: ULib.StringLibrary.copyString(partitionType, "NT IFS Volume Set"); break;
		case 0x93: ULib.StringLibrary.copyString(partitionType, "Amoeba File System"); break;
		case 0x94: ULib.StringLibrary.copyString(partitionType, "Amoeba Bad Block Table"); break;
		case 0xA0: ULib.StringLibrary.copyString(partitionType, "Save To Disk Partition"); break;
		case 0xA5: ULib.StringLibrary.copyString(partitionType, "FreeBSD/386"); break;
		case 0xA7: ULib.StringLibrary.copyString(partitionType, "NextStep"); break;
		case 0xB7: ULib.StringLibrary.copyString(partitionType, "BSDI File System"); break;
		case 0xB8: ULib.StringLibrary.copyString(partitionType, "BSDI Swap Partition"); break;
		case 0xC0: ULib.StringLibrary.copyString(partitionType, "CTOS"); break;
		case 0xC1: ULib.StringLibrary.copyString(partitionType, "DR-DOS (12-bit FAT)"); break;
		case 0xC4: ULib.StringLibrary.copyString(partitionType, "DR-DOS (16-bit FAT)"); break;
		case 0xC6: ULib.StringLibrary.copyString(partitionType, "Disabled NT FAT Volume Set"); break;
		case 0xC7: ULib.StringLibrary.copyString(partitionType, "Disabled NT IFS Volume Set"); break;
		case 0xDB: ULib.StringLibrary.copyString(partitionType, "CP/M"); break;
		case 0xE1: ULib.StringLibrary.copyString(partitionType, "SpeedStor (12-bit FAT Extended)"); break;
		case 0xE3: ULib.StringLibrary.copyString(partitionType, "SpeedStor"); break;
		case 0xE4: ULib.StringLibrary.copyString(partitionType, "SpeedStor (16-bit FAT Extended)"); break;
		case 0xF1: ULib.StringLibrary.copyString(partitionType, "SpeedStor Dimensions"); break;
		case 0xF2: ULib.StringLibrary.copyString(partitionType, "DOS 3.3+"); break;
		case 0xF4: ULib.StringLibrary.copyString(partitionType, "SpeedStor Storage Dimensions"); break;
		case 0xFE: ULib.StringLibrary.copyString(partitionType, "LANStep"); break;
		case 0xFF: ULib.StringLibrary.copyString(partitionType, "Xenix Bad Block Table"); break;
		default: ULib.StringLibrary.copyString(partitionType, "Unknown");
	}
}

///////////////////////////////////////////////////
// this method may not recover the password that //
// was selected at creation time. What this does //
// is to generate a valid password that matches  //
// the CRC stored in the encrypted header        //
///////////////////////////////////////////////////
char* findPasswordv2(unsigned char* passwordCRC)
{
	unsigned char* currentWord;
	char* recoveredPassword = (char*)ULib.HeapLibrary.allocPrivateHeap(100);
	bool foundPassword = false;

	passwordCRC[0] ^= XORvalues[0];
	passwordCRC[1] ^= XORvalues[1];
	passwordCRC[2] ^= XORvalues[2];
	passwordCRC[3] ^= XORvalues[3];

	BruteForceEngine BFengine;
	BFengine.initialiseCharacterSet(false, true, true, false);

	bool redoFindPassword = true;
	int startInnerIndex = 1;

	while ((startInnerIndex <= 4) && (redoFindPassword))
	{
		int previousInnerIndex = startInnerIndex;
		redoFindPassword = false;

		// find the password characters in reverse
		for (int index = 3; index >= 0; index--)
		{
			foundPassword = false;
			bool extendedCRC = false;

			// can a character be generated to match the CRC
			// of the password and of the correct length.
			// If not then we have to loop around past 0xFF
			if (passwordCRC[index] < (previousInnerIndex * 0x20)) extendedCRC = true;

			// try passwords upto length 4 for each CRC
			for (int innerIndex = previousInnerIndex; innerIndex <= 4; innerIndex++)
			{
				BFengine.setCurrentWordLength(innerIndex);

				do
				{
					currentWord = (unsigned char*)BFengine.getNextWord();
					if (extendedCRC)
					{
						if (currentWord[0] + currentWord[1] + currentWord[2] + currentWord[3] == (passwordCRC[index] + 0xFF + 0x01))
						{
							foundPassword = true;
							break;
						}
					}
					else
					{
						if (currentWord[0] + currentWord[1] + currentWord[2] + currentWord[3] == passwordCRC[index])
						{
							foundPassword = true;
							break;
						}
					}

				} while (currentWord[0] != NULL);

				if (foundPassword)
				{
					// if we loop around, decrement
					// the value of the next password
					// character to consider the carry value
					if (extendedCRC) recoveredPassword[index + 1] = (char)(recoveredPassword[index + 1] - 1);

					if (innerIndex == 1)
					{
						recoveredPassword[0 + index] = currentWord[0];
					}

					if (innerIndex == 2)
					{
						recoveredPassword[0 + index] = currentWord[0];
						recoveredPassword[0 + 4 + index] = currentWord[1];
					}

					if (innerIndex == 3)
					{
						recoveredPassword[0 + index] = currentWord[0];
						recoveredPassword[0 + 4 + index] = currentWord[1];
						recoveredPassword[0 + 4 + 4 + index] = currentWord[2];
					}

					if (innerIndex == 4)
					{
						recoveredPassword[0 + index] = currentWord[0];
						recoveredPassword[0 + 4 + index] = currentWord[1];
						recoveredPassword[0 + 4 + 4 + index] = currentWord[2];
						recoveredPassword[0 + 4 + 4 + 4 + index] = currentWord[3];
					}

					// if the password length for a
					// particular CRC is to small then
					// start password recovery again with
					// an increased starting value
					if (innerIndex > (previousInnerIndex + 1))
					{
						redoFindPassword = true;
						startInnerIndex++;
					}

					previousInnerIndex = innerIndex;
					break;
				}
			}

			// unable to recover a suitable password
			if ((!foundPassword) && (!redoFindPassword))
			{
				ULib.ConsoleLibrary.displayConsoleMessage("\r\n");
				return NULL;
			}

			if (redoFindPassword) break;
		}
	}

	BFengine.destroyCharacterSet();
	return recoveredPassword;
}