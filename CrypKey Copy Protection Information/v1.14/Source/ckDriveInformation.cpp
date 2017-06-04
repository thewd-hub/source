#include "ckInfo.h"
#include "ckDriveInformation.h"
#include "ckEncryption.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;
bool displayedOneDisk = false;

// keep a record of found disks (windows nt only)
char* foundDisksNT[32];
int foundDisksNTCount = 0;

///////////////////////////////////////////////////////////////
// removes the leading invalid characters (including spaces) //
///////////////////////////////////////////////////////////////
char* removeInvalidLeavingCharacters(char* buffer, unsigned long bufferLength)
{
	unsigned long startIndex = 0;
	for (unsigned long i = 0; i < bufferLength; i++)
	{
		if (buffer[i] > 0x20)
		{
			startIndex = i;
			break;
		}
	}

	return (char*)&buffer[startIndex];
}

////////////////////////////////////////////////////
// returns a string for the specified word buffer //
////////////////////////////////////////////////////
void getStringFromWordBuffer(unsigned char* sourceBuffer, unsigned long bufferLength, char* outBuffer)
{
	for (unsigned long i = 0; i < bufferLength; i += 2) ULib.StringLibrary.addWORDToBuffer((unsigned char*)outBuffer, ULib.StringLibrary.getWORDFromBuffer(sourceBuffer, i), i, true);
	// remove trailing spaces
	for (int i = (bufferLength - 1); i >= 0; i--)
	{
		if (outBuffer[i] > 0x20) break;
		outBuffer[i] = 0x00;
	}
}

////////////////////////////////////////////
// display the information about the disk //
////////////////////////////////////////////
bool displayDiskInformation(IDENTIFY_DATA* pId, unsigned long controllerType, unsigned long driveType, bool usingSCSIBackdoor)
{
	// determine disk type
	unsigned long diskType = DISKTYPE_UNKNOWN;
	if (pId->GeneralConfiguration & 0x80) diskType = DISKTYPE_REMOVABLE;
	else if (pId->GeneralConfiguration & 0x40) diskType = DISKTYPE_FIXED;

	char* modelNumberHDSN = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* firmwareRevisionHDSN = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* serialNumberHDSN = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// get disk information
	getStringFromWordBuffer((unsigned char*)pId->ModelNumber, min(ULib.StringLibrary.getStringLength((char*)pId->ModelNumber), 40), modelNumberHDSN);
	getStringFromWordBuffer((unsigned char*)pId->FirmwareRevision, min(ULib.StringLibrary.getStringLength((char*)pId->FirmwareRevision), 4), firmwareRevisionHDSN);
	getStringFromWordBuffer((unsigned char*)pId->SerialNumber, min(ULib.StringLibrary.getStringLength((char*)pId->SerialNumber), 20), serialNumberHDSN);

	char* modelNumber = removeInvalidLeavingCharacters(modelNumberHDSN, ULib.StringLibrary.getStringLength(modelNumberHDSN));
	char* firmwareRevision = removeInvalidLeavingCharacters(firmwareRevisionHDSN, ULib.StringLibrary.getStringLength(firmwareRevisionHDSN));
	char* serialNumber = removeInvalidLeavingCharacters(serialNumberHDSN, ULib.StringLibrary.getStringLength(serialNumberHDSN));

	// validate information
	if (!isDebugMode())
	{
		bool skipCurrentDisk = false;
		// missing information
		if ((ULib.StringLibrary.getStringLength(modelNumber) <= 0) && (ULib.StringLibrary.getStringLength(firmwareRevision) <= 0) && (ULib.StringLibrary.getStringLength(serialNumber) <= 0)) skipCurrentDisk = true;
		// check for existing match
		if (!skipCurrentDisk) for (int i = 0; i < foundDisksNTCount; i++) if (ULib.StringLibrary.isStringMatch(foundDisksNT[i], serialNumber, true)) skipCurrentDisk = true;
		// skip current disk
		if (skipCurrentDisk)
		{
			ULib.HeapLibrary.freePrivateHeap(modelNumberHDSN);
			ULib.HeapLibrary.freePrivateHeap(firmwareRevisionHDSN);
			ULib.HeapLibrary.freePrivateHeap(serialNumberHDSN);
			return false;
		}
	}

	// store current serial number found
	if (foundDisksNTCount >= 32) return false;
	foundDisksNT[foundDisksNTCount] = (char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.copyString(foundDisksNT[foundDisksNTCount], serialNumber);
	foundDisksNTCount++;

	// get hdsn
	char* hdsnString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	if ((serialNumber != NULL) && (serialNumber[0] != NULL)) wsprintf(hdsnString, "%s%s%s", serialNumberHDSN, modelNumberHDSN, firmwareRevisionHDSN);
	unsigned long hdsnCRC = getKeyCRC((unsigned char*)hdsnString, ULib.StringLibrary.getStringLength(hdsnString), KEYTYPE_HDSN);
	if (hdsnCRC != 0xFFFF) hdsnCRC = (((hdsnCRC >> 8) | (hdsnCRC << 8)) & 0x7FFF);

	// display information
	char* outputLine1 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine3 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine4 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine5 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* outputLine6 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(outputLine1, "%s%s Disk Controller (%s)...", ((displayedOneDisk) ? "\r\n" : ""), ((controllerType == CONTROLLERTYPE_PRIMARY) ? "Primary" : ((controllerType == CONTROLLERTYPE_SECONDARY) ? "Secondary" : ((controllerType == CONTROLLERTYPE_TERTIARY) ? "Tertiary" : ((controllerType == CONTROLLERTYPE_QUATERNARY) ? "Quaternary" : "Unknown")))), ((driveType == DRIVETYPE_MASTER) ? "Master" : ((driveType == DRIVETYPE_SLAVE) ? "Slave" : "Unknown")));
	wsprintf(outputLine2, "+ Disk Type     : %s%s", ((diskType == DISKTYPE_FIXED) ? "Fixed" : ((diskType == DISKTYPE_REMOVABLE) ? "Removable" : "Unknown")), (((isDebugMode()) && (usingSCSIBackdoor)) ? " (SCSI)" : ""));
	wsprintf(outputLine3, "+ Model Number  : %s", modelNumber);
	wsprintf(outputLine4, "+ Firmware Rev. : %s", firmwareRevision);
	wsprintf(outputLine5, "+ Serial Number : %s", serialNumber);
	wsprintf(outputLine6, "+ CrypKey HDSN  : %d", ((hdsnCRC != 0xFFFF) ? hdsnCRC : -1));

	displayedOneDisk = true;
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine1);
	ULib.ConsoleLibrary.displayConsoleMessage(outputLine2);
	if ((isDebugMode()) || ((diskType != DISKTYPE_UNKNOWN) && (modelNumber != NULL) && (modelNumber[0] != NULL)))
	{
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine3);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine4);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine5);
		ULib.ConsoleLibrary.displayConsoleMessage(outputLine6);
	}

	ULib.HeapLibrary.freePrivateHeap(hdsnString);
	ULib.HeapLibrary.freePrivateHeap(outputLine1);
	ULib.HeapLibrary.freePrivateHeap(outputLine2);
	ULib.HeapLibrary.freePrivateHeap(outputLine3);
	ULib.HeapLibrary.freePrivateHeap(outputLine4);
	ULib.HeapLibrary.freePrivateHeap(outputLine5);
	ULib.HeapLibrary.freePrivateHeap(outputLine6);

	ULib.HeapLibrary.freePrivateHeap(modelNumberHDSN);
	ULib.HeapLibrary.freePrivateHeap(firmwareRevisionHDSN);
	ULib.HeapLibrary.freePrivateHeap(serialNumberHDSN);
	return true;
}

///////////////////////////////////////////////////////
// display the information used by the HDSN facility //
///////////////////////////////////////////////////////
void showHDSNInformation_NTx(void)
{
	// initialise device structures
	char* bufferIn = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* bufferOut = (char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned long bufferInSize = (sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE - 1);
	unsigned long bufferOutSize = (sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1);
	unsigned long bufferCmdInSize = (sizeof(SENDCMDINPARAMS) - 1);
	unsigned long bufferCmdOutSize = (sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1);
	SRB_IO_CONTROL* pSRB_IN = (SRB_IO_CONTROL*)bufferIn;
	SRB_IO_CONTROL* pSRB_OUT = (SRB_IO_CONTROL*)bufferOut;
	SENDCMDINPARAMS* pCMD_IN = (SENDCMDINPARAMS*)(bufferIn + sizeof(SRB_IO_CONTROL));
	SENDCMDOUTPARAMS* pCMD_OUT = (SENDCMDOUTPARAMS*)(bufferOut + sizeof(SRB_IO_CONTROL));

	HANDLE deviceHandle = NULL;
	char* deviceFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// cycle through all the disk controllers & drives - SCSI DRIVES (BACKDOOR)
	for (int deviceId = 0; deviceId < MAX_SCSI_DRIVES; deviceId++)
	{
		// get device name & handle
		wsprintf(deviceFilename, "\\\\.\\SCSI%d:", deviceId);
		deviceHandle = CreateFile(deviceFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE) continue;

		// try master/slave drives
		for (int driveType = 0; driveType < 2; driveType++)
		{
			__try
			{
				ZeroMemory(bufferIn, bufferInSize);
				ZeroMemory(bufferOut, bufferOutSize);

				// query device - get identify data
				pSRB_IN->HeaderLength = sizeof(SRB_IO_CONTROL);
				pSRB_IN->Length = bufferInSize - (pSRB_IN->HeaderLength);
				pSRB_IN->Timeout = 10000;
				pSRB_IN->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
				CopyMemory(pSRB_IN->Signature, "SCSIDISK", 8);

				pCMD_IN->bDriveNumber = (unsigned char)driveType;
				pCMD_IN->cBufferSize = IDENTIFY_BUFFER_SIZE;
				pCMD_IN->irDriveRegs.bFeaturesReg = 0;
				pCMD_IN->irDriveRegs.bSectorCountReg = 1;
				pCMD_IN->irDriveRegs.bSectorNumberReg = 1;
				pCMD_IN->irDriveRegs.bCylLowReg = 0;
				pCMD_IN->irDriveRegs.bCylHighReg = 0;
				pCMD_IN->irDriveRegs.bDriveHeadReg = (unsigned char)(0xA0 | ((driveType & 1) << 4));
				pCMD_IN->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;

				unsigned long bytesReturned = 0;
				if (DeviceIoControl(deviceHandle, IOCTL_SCSI_MINIPORT,
					pSRB_IN, bufferInSize, pSRB_OUT, bufferOutSize, &bytesReturned, NULL))
				{
					IDENTIFY_DATA* pId = (IDENTIFY_DATA*)(pCMD_OUT->bBuffer);
					displayDiskInformation(pId, deviceId + 1, driveType + 1, true);
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}

		CloseHandle(deviceHandle);
	}

	// cycle through all the disk controllers & drives - PHYSICAL DRIVES
	for (int deviceId = 0; deviceId < MAX_IDE_DRIVES; deviceId++)
	{
		// get device name & handle
		wsprintf(deviceFilename, "\\\\.\\PhysicalDrive%d", deviceId);
		deviceHandle = CreateFile(deviceFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (deviceHandle == INVALID_HANDLE_VALUE) continue;

		__try
		{
			ZeroMemory(bufferIn, bufferInSize);
			ZeroMemory(bufferOut, bufferOutSize);

			pCMD_IN->bDriveNumber = (unsigned char)deviceId;
			pCMD_IN->cBufferSize = IDENTIFY_BUFFER_SIZE;
			pCMD_IN->irDriveRegs.bFeaturesReg = 0;
			pCMD_IN->irDriveRegs.bSectorCountReg = 1;
			pCMD_IN->irDriveRegs.bSectorNumberReg = 1;
			pCMD_IN->irDriveRegs.bCylLowReg = 0;
			pCMD_IN->irDriveRegs.bCylHighReg = 0;
			pCMD_IN->irDriveRegs.bDriveHeadReg = (unsigned char)(0xA0 | ((deviceId & 1) << 4));
			pCMD_IN->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;

			unsigned long bytesReturned = 0;
			if (DeviceIoControl(deviceHandle, SMART_RCV_DRIVE_DATA,
				pCMD_IN, bufferCmdInSize, pCMD_OUT, bufferCmdOutSize, &bytesReturned, NULL))
			{
				IDENTIFY_DATA* pId = (IDENTIFY_DATA*)(pCMD_OUT->bBuffer);
				displayDiskInformation(pId, (deviceId / 2) + 1, (deviceId % 2) + 1, false);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}

		CloseHandle(deviceHandle);
	}

	for (int i = 0; i < foundDisksNTCount; i++) ULib.HeapLibrary.freePrivateHeap(foundDisksNT[i]);
	ULib.HeapLibrary.freePrivateHeap(deviceFilename);
	ULib.HeapLibrary.freePrivateHeap(bufferIn);
	ULib.HeapLibrary.freePrivateHeap(bufferOut);
}

///////////////////////////////////////////////////////
// display the information used by the HDSN facility //
///////////////////////////////////////////////////////
void showHDSNInformation_9x(void)
{
	// get ideinfo.vxd resource
	unsigned long deviceResourceSize = 0;
	unsigned char* deviceResourceBuffer = ULib.FileLibrary.getResourceFromFile(NULL, "IDEINFO", RT_RCDATA, &deviceResourceSize);
	if ((deviceResourceBuffer == NULL) || (deviceResourceSize <= 0)) displayErrorMessage(ERROR_DISKINFORMATION_VXDEXTRACT);

	// extract resource to temporary file
	char* tempFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	if (!ULib.FileLibrary.extractResourceToTemporaryFile(deviceResourceBuffer, deviceResourceSize, tempFilename, NULL, "vxd")) displayErrorMessage(ERROR_DISKINFORMATION_VXDEXTRACT);

	// get device name & handle
	char* deviceFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(deviceFilename, "\\\\.\\%s", tempFilename);
	HANDLE deviceHandle = CreateFile(deviceFilename, NULL, NULL, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (deviceHandle == INVALID_HANDLE_VALUE) displayErrorMessage(ERROR_DISKINFORMATION_VXDFAILED);

	// execute vxd function call
	unsigned long bytesReturned = 0;
	IDEINFO* pIdeInfo = (IDEINFO*)ULib.HeapLibrary.allocPrivateHeap(sizeof(IDEINFO));
	__try
	{
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		DeviceIoControl(deviceHandle, IDEINFO_IDENTIFY, NULL, NULL, pIdeInfo, sizeof(DWORD), &bytesReturned, NULL);
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		CloseHandle(deviceHandle);
		DeleteFile(tempFilename);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		CloseHandle(deviceHandle);
		DeleteFile(tempFilename);
		displayErrorMessage(ERROR_DISKINFORMATION_VXDFAILED);
	}

	// cycle through the returned information
	for (unsigned long i = 0; i < MAX_IDE_DRIVES; i++)
	{
		if ((pIdeInfo->DiskExists[i]) && (pIdeInfo->IDEExists[i / 2]))
		{
			// display disk information
			IDENTIFY_DATA* pId = (IDENTIFY_DATA*)&pIdeInfo->DisksRawInfo[i * IDENTIFY_BUFFER_SIZE];
			displayDiskInformation(pId, ((i / 2) + 1), ((i % 2) + 1), false);
		}
	}

	ULib.HeapLibrary.freePrivateHeap(pIdeInfo);
	ULib.HeapLibrary.freePrivateHeap(tempFilename);
	ULib.HeapLibrary.freePrivateHeap(deviceFilename);
}

///////////////////////////////////////////////////////
// display the information used by the HDSN facility //
///////////////////////////////////////////////////////
void showHDSNInformation(void)
{
	if (ULib.OperatingSystemLibrary.isWindowsNTx()) showHDSNInformation_NTx();
	else showHDSNInformation_9x();

	// error displaying disk information
	if (!displayedOneDisk) displayErrorMessage(ERROR_DISKINFORMATION_FAILED);
}