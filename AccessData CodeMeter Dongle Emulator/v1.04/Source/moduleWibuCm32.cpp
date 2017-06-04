///////////////////////////////////////////////////////////
// AccessData CodeMeter Dongle Emulator                  //
// (C)thewd, thewd@hotmail.com                           //
///////////////////////////////////////////////////////////
// History                                               //
//                                                       //
// v1.04                                                 //
// - updated code to fully support 64-bit os & patching  //
// - fixed a couple of patching bugs                     //
// v1.03                                                 //
// - support for disabling dongle creation via registry  //
// - support for accessdata product code 1000 dongle     //
// - support CmGetInfo option CM_GEI_BOXINFO. copy the   //
//   fixed dongle serial number to the structure         //
// - support for fixed dongle serial numbers (AD Triage) //
// - support for 64-bit module                           //
// v1.02                                                 //
// - support for other patching rsa modulus key arrays   //
// - changed memory range validation checks to support   //
//   aslr on vista+ operating systems                    //
// - support for additional codemeter exports            //
//   CmAccess2 and CmActLicenseControl                   //
// v1.01                                                 //
// - support for patching rsa modulus key array          //
// - support for scanning recovery_jni.dll module memory //
//   which is used by prtk and dna products              //
// v1.00                                                 //
// - initial release                                     //
///////////////////////////////////////////////////////////

#include <windows.h>
#include <tlhelp32.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "..\..\..\_Shared Libraries\OpenSSL_BigNumberLibrary.h"
#include "..\..\..\_Shared Libraries\OpenSSL_AESLibrary.h"
#include "..\..\..\_Shared Libraries\OpenSSL_RSALibrary.h"
#include "moduleWibuCm32.h"
#include "CodeMeter.450.h"

DWORD dwDebugLevel = DEBUG_BASIC;
BOOL bNetworkDevice = FALSE;
BOOL bIsDisabled = FALSE;

CHAR* strDebugOutput = NULL;

DWORD hcmHandles[HCM_MAX + 1];
DWORD hcmProductCode[HCM_MAX + 1];
DWORD cmLastErrorCode = 0;

AD_DONGLEPACKET100* pDonglePacket100 = NULL;
AD_DONGLEPACKET1000* pDonglePacket1000 = NULL;
DWORD dongleSerialNumber = 0;
DWORD dongleSerialNumberFixed = 0;
CHAR* donglePacketVersionXML = NULL;
CHAR* dongleSerialNumberXML = NULL;
CHAR* dongleNetworkDeviceXML = NULL;
CHAR* dongleExpirationDate = NULL;
CHAR* dongleExpirationDateXML = NULL;
CHAR* dongleExpirationDateDemoXML = NULL;
CHAR* dongleExpirationDateBetaXML = NULL;
SYSTEMTIME lpSystemTimeExpiry;
UCHAR RSAModulusKey_0[108];

CUtilitiesLibrary ULib;

// display debug message
BOOL DisplayDebugString(CHAR* debugStr, DWORD debugLevel = DEBUG_BASIC, BOOL retValue = TRUE, BOOL displayPrefix = TRUE)
{
	// check current debug level
	if ((debugStr != NULL) && (dwDebugLevel >= debugLevel))
	{
		if (displayPrefix)
		{
			CHAR* debugStrFull = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(debugStr) + 128);
			if (ULib.ProcessLibrary.is64BitProcess()) wsprintf(debugStrFull, "[WibuCm64-AccessData] %s", debugStr);
			else wsprintf(debugStrFull, "[WibuCm32-AccessData] %s", debugStr);
			OutputDebugString(debugStrFull);
			ULib.HeapLibrary.freePrivateHeap(debugStrFull);
		}
		else
		{
			OutputDebugString(debugStr);
		}
	}

	return retValue;
}

// display debug message (multi-lined)
BOOL DisplayDebugStringMultilined(CHAR* debugStr, DWORD debugLevel = DEBUG_BASIC, BOOL retValue = TRUE, BOOL displayPrefix = TRUE)
{
	// check current debug level
	if ((debugStr != NULL) && (dwDebugLevel >= debugLevel))
	{
		int numberOfElements;
		char* strElements[128];
		ULib.StringLibrary.getStringElementsFromString(debugStr, strElements, &numberOfElements, 128, NULL, "\r\n");
		for (int i = 0; i < numberOfElements; i++)
		{
			DisplayDebugString(strElements[i], debugLevel, retValue, displayPrefix);
			ULib.HeapLibrary.freePrivateHeap(strElements[i]);
		}
	}

	return retValue;
}

// load the rsa modulus key from resources and copy to memory buffer
VOID LoadAndStoreRSAModulusKey_0()
{
	DWORD keyn_len = 0;
	UCHAR* keyn = ULib.FileLibrary.getResourceFromFile(((ULib.ProcessLibrary.is64BitProcess()) ? "WibuCm64.dll" : "WibuCm32.dll"), "N", "RSAKEYS", &keyn_len);
	// if parameters ok, copy rsa modulus key to memory buffer
	if ((keyn != NULL) && (keyn_len == 108)) ULib.StringLibrary.copyBuffer(RSAModulusKey_0, 108, keyn, 108);
	else DisplayDebugString("LoadAndStoreRSAModulusKey_0:Error - RSA Key Resource Not Found");
}

// get registry settings (initialise default values if not found)
VOID GetRegistrySettings()
{
	DWORD tmpSettingValue = 0;
	HKEY hKey = ULib.RegistryLibrary.createRegistrySubKey(HKEY_LOCAL_MACHINE, REGISTRYKEY_SETTINGS, KEY_ALL_ACCESS | ULib.RegistryLibrary.ACCESSMASK_WOW64_ACCESS64BITKEY);
	if (hKey != NULL)
	{
		UCHAR* currentSetting = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "DebugLevel", NULL, NULL);
		if (currentSetting != NULL)
		{
			tmpSettingValue = ULib.StringLibrary.getDWORDFromBuffer(currentSetting);
			if ((tmpSettingValue >= DEBUG_NONE) && (tmpSettingValue <= DEBUG_CODEMETER)) dwDebugLevel = tmpSettingValue;
			ULib.HeapLibrary.freePrivateHeap(currentSetting);
		}
		else
		{
			tmpSettingValue = dwDebugLevel;
			ULib.RegistryLibrary.setRegistryKeyValue(hKey, "DebugLevel", (UCHAR*)&tmpSettingValue, REG_DWORD);
		}

		currentSetting = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "IsDisabled", NULL, NULL);
		if (currentSetting != NULL)
		{
			tmpSettingValue = ULib.StringLibrary.getDWORDFromBuffer(currentSetting);
			bIsDisabled = ((tmpSettingValue == 1) ? TRUE : FALSE);
			ULib.HeapLibrary.freePrivateHeap(currentSetting);
		}
		else
		{
			tmpSettingValue = ((bIsDisabled) ? 1 : 0);
			ULib.RegistryLibrary.setRegistryKeyValue(hKey, "IsDisabled", (UCHAR*)&tmpSettingValue, REG_DWORD);
		}

		currentSetting = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "IsNetworkDevice", NULL, NULL);
		if (currentSetting != NULL)
		{
			tmpSettingValue = ULib.StringLibrary.getDWORDFromBuffer(currentSetting);
			bNetworkDevice = ((tmpSettingValue == 1) ? TRUE : FALSE);
			ULib.HeapLibrary.freePrivateHeap(currentSetting);
		}
		else
		{
			tmpSettingValue = ((bNetworkDevice) ? 1 : 0);
			ULib.RegistryLibrary.setRegistryKeyValue(hKey, "IsNetworkDevice", (UCHAR*)&tmpSettingValue, REG_DWORD);
		}

		currentSetting = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "SerialNumber", NULL, NULL);
		if (currentSetting != NULL)
		{
			tmpSettingValue = ULib.StringLibrary.getDWORDFromBuffer(currentSetting);
			if (tmpSettingValue > 0) dongleSerialNumber = tmpSettingValue;
			ULib.HeapLibrary.freePrivateHeap(currentSetting);
		}
		else
		{
			tmpSettingValue = 0;
			ULib.RegistryLibrary.setRegistryKeyValue(hKey, "SerialNumber", (UCHAR*)&tmpSettingValue, REG_DWORD);
		}

		currentSetting = ULib.RegistryLibrary.getRegistryKeyValue(hKey, "SerialNumberFixed", NULL, NULL);
		if (currentSetting != NULL)
		{
			tmpSettingValue = ULib.StringLibrary.getDWORDFromBuffer(currentSetting);
			if (tmpSettingValue > 0) dongleSerialNumberFixed = tmpSettingValue;
			ULib.HeapLibrary.freePrivateHeap(currentSetting);
		}
		else
		{
			tmpSettingValue = AD_DONGLESERIALFIXED;
			ULib.RegistryLibrary.setRegistryKeyValue(hKey, "SerialNumberFixed", (UCHAR*)&tmpSettingValue, REG_DWORD);
		}

		RegCloseKey(hKey);
	}
}

// generate dynamic values used by dongle licence
VOID GenerateDynamicDongleValues()
{
	DWORD dwMajorVersion = 0x10;
	DWORD dwMinorVersion = 0x01;

	dongleExpirationDate = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);
	dongleExpirationDateXML = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);
	dongleExpirationDateDemoXML = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);
	dongleExpirationDateBetaXML = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);
	donglePacketVersionXML = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);
	dongleSerialNumberXML = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);
	dongleNetworkDeviceXML = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(128);

	GetLocalTime(&lpSystemTimeExpiry);
	lpSystemTimeExpiry.wMonth = ((lpSystemTimeExpiry.wMilliseconds % 12) + 1);
	if (lpSystemTimeExpiry.wMonth == 2) lpSystemTimeExpiry.wDay = ((lpSystemTimeExpiry.wMilliseconds % 28) + 1);
	else lpSystemTimeExpiry.wDay = ((lpSystemTimeExpiry.wMilliseconds % 30) + 1);
	lpSystemTimeExpiry.wYear += 2;
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, &lpSystemTimeExpiry, "dd-MMM-yyyy\"", dongleExpirationDate, 128);
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, &lpSystemTimeExpiry, " 'expiration='\"dd-MMM-yyyy\"", dongleExpirationDateXML, 128);
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, &lpSystemTimeExpiry, " 'demo-expiration='\"dd-MMM-yyyy\"", dongleExpirationDateDemoXML, 128);
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, &lpSystemTimeExpiry, " 'beta-expiration='\"dd-MMM-yyyy\"", dongleExpirationDateBetaXML, 128);

	// dongle packet version and serial number
	wsprintf(donglePacketVersionXML, " packet-version=\"%d\"", ((dwMajorVersion - dwMinorVersion) << 8) + dwMinorVersion);
	if (dongleSerialNumber == 0) dongleSerialNumber = (1000000 + ((lpSystemTimeExpiry.wMilliseconds % 5) * 10101) + ((lpSystemTimeExpiry.wMilliseconds + 1) * (lpSystemTimeExpiry.wSecond + 1)));
	wsprintf(dongleSerialNumberXML, " num=\"%d\"", dongleSerialNumber);

	// is network device licence (always true if queried by AD NLS)
	CHAR* tmpStr = (CHAR*)ULib.HeapLibrary.allocPrivateHeap();
	GetModuleFileName(NULL, tmpStr, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE);
	if (ULib.StringLibrary.isStringPatternMatch(tmpStr, "*\\adNetworkLicenseService.exe")) bNetworkDevice = TRUE;
	wsprintf(dongleNetworkDeviceXML, " network_device=\"%d\"", ((bNetworkDevice) ? 1 : 0));
	ULib.HeapLibrary.freePrivateHeap(tmpStr);
}

// display module information and parent process information
VOID DisplayVersionInformation()
{
	// display version
	DisplayDebugString(" ");
	DisplayDebugString(MODULE_VERSION);

	// process information
	CHAR* tmpStr = (CHAR*)ULib.HeapLibrary.allocPrivateHeap();
	GetModuleFileName(NULL, tmpStr, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE);
	wsprintf(strDebugOutput, "Process (%d): %s", GetCurrentProcessId(), tmpStr);
	DisplayDebugString(strDebugOutput);
	ULib.HeapLibrary.freePrivateHeap(tmpStr);
}

// create the dongle packet retrieved using CMGetInfo (using CM_GEI_ENTRYDATA)
BOOL CreateDonglePacket100(AD_DONGLEPACKET100* pDonglePacket100)
{
	DisplayDebugString(" ");
	DisplayDebugString("Create Dongle Packet (100)");

	// check parameters, return error if failed
	if (pDonglePacket100 == NULL) return DisplayDebugString("Create Dongle Packet:Error - Dongle Packet Not Defined", DEBUG_BASIC, FALSE);

	// read licence file
	DWORD xmlLicenceLength = 0;
	CHAR* xmlLicence = (CHAR*)ULib.FileLibrary.readBufferFromFile("ad-licence.xml", &xmlLicenceLength);
	// not found in local folder, try windows system32 folder (or syswow64 folder under wow64)
	if (xmlLicence == NULL)
	{
		CHAR* strFilenameSYS32 = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(1024);
		GetSystemDirectory(strFilenameSYS32, 1024);
		ULib.StringLibrary.appendString(strFilenameSYS32, "\\ad-licence.xml");
		xmlLicence = (CHAR*)ULib.FileLibrary.readBufferFromFile(strFilenameSYS32, &xmlLicenceLength);
		ULib.HeapLibrary.freePrivateHeap(strFilenameSYS32);
	}
	// licence not found, return error
	if ((xmlLicence == NULL) || (xmlLicenceLength <= 0)) return DisplayDebugString("Create Dongle Packet:Error - Licence File Not Found", DEBUG_BASIC, FALSE);

	xmlLicence = ULib.StringLibrary.replaceSubString(xmlLicence, " packet-version=\"?\"", donglePacketVersionXML);
	xmlLicence = ULib.StringLibrary.replaceSubString(xmlLicence, " num=\"?\"", dongleSerialNumberXML);
	xmlLicence = ULib.StringLibrary.replaceSubString(xmlLicence, " network_device=\"?\"", dongleNetworkDeviceXML);
	xmlLicence = ULib.StringLibrary.replaceSubString(xmlLicence, " expiration=\"?\"", dongleExpirationDateXML);
	xmlLicence = ULib.StringLibrary.replaceSubString(xmlLicence, " demo-expiration=\"?\"", dongleExpirationDateDemoXML);
	xmlLicence = ULib.StringLibrary.replaceSubString(xmlLicence, " beta-expiration=\"?\"", dongleExpirationDateBetaXML);
	xmlLicenceLength = ULib.StringLibrary.getStringLength(xmlLicence);

	// display licence
	DisplayDebugString(" ");
	DisplayDebugString("XML Dongle Licence");
	DisplayDebugString("***********************************************************************************************");
	DisplayDebugStringMultilined(xmlLicence);
	DisplayDebugString("***********************************************************************************************");

	// encrypt AES key using RSA (108 bytes)
	RSA* rsaTmp = RSA_new();
	BIGNUM* bnN = BN_new();
	BIGNUM* bnE = BN_new();
	BIGNUM* bnD = BN_new();
	DWORD keyd_len = 0;
	DWORD keye_len = 0;
	DWORD keyn_len = 0;
	UCHAR* keyd = ULib.FileLibrary.getResourceFromFile(((ULib.ProcessLibrary.is64BitProcess()) ? "WibuCm64.dll" : "WibuCm32.dll"), "D", "RSAKEYS", &keyd_len);
	UCHAR* keye = ULib.FileLibrary.getResourceFromFile(((ULib.ProcessLibrary.is64BitProcess()) ? "WibuCm64.dll" : "WibuCm32.dll"), "E", "RSAKEYS", &keye_len);
	UCHAR* keyn = ULib.FileLibrary.getResourceFromFile(((ULib.ProcessLibrary.is64BitProcess()) ? "WibuCm64.dll" : "WibuCm32.dll"), "N", "RSAKEYS", &keyn_len);

	// check for errors, return error if found
	if ((rsaTmp == NULL) || (keyn == NULL) || (keyn_len != 108) || (keyd == NULL) || (keyd_len != 108) || (keye == NULL) || (keye_len != 3)) return DisplayDebugString("Create Dongle Packet:Error - RSA Key Resources Not Found", DEBUG_BASIC, FALSE);

	// set big numbers and rsa keys
	BN_bin2bn(keyd, keyd_len, bnD);
	BN_bin2bn(keye, keye_len, bnE);
	BN_bin2bn(keyn, keyn_len, bnN);
	rsaTmp->n = bnN;
	rsaTmp->e = bnE;
	rsaTmp->d = bnD;

	// encrypt rsa key, return error if failed
	if (RSA_private_encrypt(ULib.StringLibrary.getStringLength(AD_AESKEY), (UCHAR*)AD_AESKEY, pDonglePacket100->dongleHeader.aesKeyRSAEncrypted, rsaTmp, RSA_PKCS1_PADDING) != 108) return DisplayDebugString("Create Dongle Packet:Error - RSA_private_encrypt Failed", DEBUG_BASIC, FALSE);

	// encrypt xml licence using AES key
	AES_KEY* aesKey = (AES_KEY*)ULib.HeapLibrary.allocPrivateHeap(sizeof(AES_KEY));
	UCHAR aes_ecount_buf[AES_BLOCK_SIZE];
	UCHAR aes_ivec[AES_BLOCK_SIZE];
	unsigned int aes_num = 0;
	ULib.StringLibrary.copyBuffer((UCHAR*)pDonglePacket100->dongleHeader.aesKey, (UCHAR*)AD_AESKEY, 16);
	ULib.StringLibrary.copyBuffer((UCHAR*)&aes_ivec[0], (UCHAR*)AD_AESKEY, 16);
	memset(aes_ecount_buf, 0, AES_BLOCK_SIZE);
	AES_set_encrypt_key((UCHAR*)AD_AESKEY, 128, aesKey);
	pDonglePacket100->xmlLicenceEncrypted = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap(xmlLicenceLength);
	AES_ctr128_encrypt((UCHAR*)xmlLicence, pDonglePacket100->xmlLicenceEncrypted, xmlLicenceLength, aesKey, aes_ivec, aes_ecount_buf, &aes_num);
	ULib.HeapLibrary.freePrivateHeap(aesKey);

	// set values, lengths and number of entry data blocks
	pDonglePacket100->dongleHeader.dongleVersion = 0;
	pDonglePacket100->dongleHeader.rsaKeyId = 0;
	pDonglePacket100->dongleHeaderLength = sizeof(AD_DONGLEHEADER);
	pDonglePacket100->xmlLicenceEncryptedLength = xmlLicenceLength;
	pDonglePacket100->donglePacketLength = pDonglePacket100->dongleHeaderLength + pDonglePacket100->xmlLicenceEncryptedLength;
	if (pDonglePacket100->donglePacketLength <= CM_MAX_STRING_LEN) pDonglePacket100->numberOfEntryDataBlocks = 1;
	else pDonglePacket100->numberOfEntryDataBlocks = (pDonglePacket100->donglePacketLength / CM_MAX_STRING_LEN) + 1;

	RSA_free(rsaTmp);
	ULib.HeapLibrary.freePrivateHeap(xmlLicence);
	return TRUE;
}

// create the dongle packet retrieved using CMGetInfo (using CM_GEI_ENTRYDATA)
BOOL CreateDonglePacket1000(AD_DONGLEPACKET1000* pDonglePacket1000)
{
	DisplayDebugString(" ");
	DisplayDebugString("Create Dongle Packet (1000)");

	// check parameters, return error if failed
	if (pDonglePacket1000 == NULL) return DisplayDebugString("Create Dongle Packet:Error - Dongle Packet Not Defined", DEBUG_BASIC, FALSE);

	pDonglePacket1000->expiryYear = (UCHAR)(lpSystemTimeExpiry.wYear - 2000);
	pDonglePacket1000->expiryDay = (UCHAR)(lpSystemTimeExpiry.wDay);
	pDonglePacket1000->expiryMonth = (UCHAR)(lpSystemTimeExpiry.wMonth);
	pDonglePacket1000->licenceCount = 15;

	// display licence
	CHAR* strLicence = (CHAR*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(strLicence, "Expiry Date: Day=\"%d\", Month=\"%d\", Year=\"%d\"\r\nLicence Count: \"%d\"", pDonglePacket1000->expiryDay, pDonglePacket1000->expiryMonth, pDonglePacket1000->expiryYear + 2000, pDonglePacket1000->licenceCount);
	DisplayDebugString(" ");
	DisplayDebugString("Dongle Licence");
	DisplayDebugString("***********************************************************************************************");
	DisplayDebugStringMultilined(strLicence);
	DisplayDebugString("***********************************************************************************************");

	return TRUE;
}

// RSA_public_decrypt function called by target application - changes rsa modulus
int _RSA_public_decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
{
	DisplayDebugString(" ");
	DisplayDebugString("RSA_public_decrypt");

	DWORD keyn_len = 0;
	UCHAR* keyn = ULib.FileLibrary.getResourceFromFile(((ULib.ProcessLibrary.is64BitProcess()) ? "WibuCm64.dll" : "WibuCm32.dll"), "N", "RSAKEYS", &keyn_len);
	int returnValue = -1;

	// check initial parameters and resource is loaded
	if ((rsa != NULL) && (rsa->meth != NULL) && ((DWORD)(rsa->meth->rsa_pub_dec) == (DWORD)&_RSA_public_decrypt) && (keyn != NULL) && (keyn_len == 108))
	{
		// check for already patched modulus key
		if ((rsa->n->top == 0x1B) && (rsa->n->d[0] == ULib.StringLibrary.getDWORDFromBuffer(keyn, 108 - 4, true)) && (rsa->n->d[1] == ULib.StringLibrary.getDWORDFromBuffer(keyn, 108 - 8, true)))
		{
			DisplayDebugString("RSA_public_decrypt:Action - Already Patched RSA Modulus Key");
		}
		// otherwise, patch key
		else
		{
			DisplayDebugString("RSA_public_decrypt:Action - Patch RSA Modulus Key");
			BN_bin2bn(keyn, keyn_len, rsa->n);
		}

		RSA* rsaTmp = RSA_new();
		returnValue = rsaTmp->meth->rsa_pub_dec(flen, from, to, rsa, padding);
		RSA_free(rsaTmp);
	}
	else
	{
		if ((keyn == NULL) || (keyn_len != 108)) DisplayDebugString("RSA_public_decrypt:Error - RSA Key Resource Not Found");
		else if ((rsa == NULL) || (rsa->meth == NULL) || ((DWORD)(rsa->meth->rsa_pub_dec) != (DWORD)&_RSA_public_decrypt)) DisplayDebugString("RSA_public_decrypt:Error - RSA Structure Invalid");
		else DisplayDebugString("RSA_public_decrypt:Error - Parameter Checks Failed");
	}

	return returnValue;
}

// changes the memory protection back to its original settings
BOOL RevertMemoryProtection(PVOID memoryAddress, SIZE_T numberOfBytes, DWORD newProtectionFlags)
{
	DWORD oldProtectionFlags;
	return (VirtualProtectEx(GetCurrentProcess(), memoryAddress, numberOfBytes, newProtectionFlags, &oldProtectionFlags) != 0);
}

// alter the memory protection, call RevertMemoryProtection to undo change
DWORD AlterMemoryProtection(MEMORY_BASIC_INFORMATION* mbi, PVOID memoryAddress, SIZE_T numberOfBytes)
{
	// query the memory to see if it's protected
	BOOL isPageNoAccess = FALSE;
	BOOL isPageGuard = FALSE;
	BOOL alterProtection = FALSE;
	BOOL alterProtectionExecute = FALSE;
	BOOL alterProtectionExecuteRead = FALSE;
	BOOL alterProtectionReadOnly = FALSE;

	// check for no access protection
	isPageNoAccess = ((mbi->Protect & PAGE_NOACCESS) ? TRUE : FALSE);
	// check for guard protection
	isPageGuard = ((mbi->Protect & PAGE_GUARD) ? TRUE : FALSE);
	// check for execute protection
	alterProtectionExecute = ((mbi->Protect & PAGE_EXECUTE) ? TRUE : FALSE);
	// check for execute-read protection
	alterProtectionExecuteRead = ((mbi->Protect & PAGE_EXECUTE_READ) ? TRUE : FALSE);
	// check for read-only protection
	alterProtectionReadOnly = ((mbi->Protect & PAGE_READONLY) ? TRUE : FALSE);

	DWORD oldProtectionFlags = mbi->Protect;
	DWORD newProtectionFlags = mbi->Protect;

	// remove page no access protection
	if (isPageNoAccess) newProtectionFlags ^= PAGE_NOACCESS;
	// remove page guard protection
	if (isPageGuard) newProtectionFlags ^= PAGE_GUARD;
	// add execute read-write protection
	if (alterProtectionExecute)
	{
		newProtectionFlags ^= PAGE_EXECUTE;
		newProtectionFlags |= PAGE_EXECUTE_READWRITE;
		alterProtection = TRUE;
	}
	// add execute read-write protection
	else if (alterProtectionExecuteRead)
	{
		newProtectionFlags ^= PAGE_EXECUTE_READ;
		newProtectionFlags |= PAGE_EXECUTE_READWRITE;
		alterProtection = TRUE;
	}
	// add read-write protection
	else if (alterProtectionReadOnly)
	{
		newProtectionFlags ^= PAGE_READONLY;
		newProtectionFlags |= PAGE_READWRITE;
		alterProtection = TRUE;
	}

	// alter the memory protection for the required memory addresses
	if ((alterProtection) && (newProtectionFlags != oldProtectionFlags)) return ((VirtualProtectEx(GetCurrentProcess(), memoryAddress, numberOfBytes, newProtectionFlags, &oldProtectionFlags) == 0) ? NULL : oldProtectionFlags);
	return NULL;
}

// read the process memory
BOOL ReadMemoryAddress(PVOID memoryAddress, SIZE_T numberOfBytes, PVOID memoryBuffer, BOOL alterMemoryAccessProtection, MEMORY_BASIC_INFORMATION* mbi = NULL)
{
	__try
	{
		DWORD oldProtectionFlags = NULL;
		SIZE_T bytesRead = 0;
		if ((alterMemoryAccessProtection) && (mbi != NULL)) oldProtectionFlags = AlterMemoryProtection(mbi, memoryAddress, numberOfBytes);
		BOOL resultReadMemory = ((ReadProcessMemory(GetCurrentProcess(), memoryAddress, memoryBuffer, numberOfBytes, &bytesRead) == 0) ? FALSE : TRUE);
		if ((alterMemoryAccessProtection) && (oldProtectionFlags != NULL)) RevertMemoryProtection(memoryAddress, numberOfBytes, oldProtectionFlags);
		return ((resultReadMemory) && (bytesRead == numberOfBytes));
	}
	__finally
	{
	}
}

// write to the process memory
BOOL WriteMemoryAddress(PVOID memoryAddress, SIZE_T numberOfBytes, PVOID memoryBuffer, BOOL alterMemoryAccessProtection, MEMORY_BASIC_INFORMATION* mbi = NULL)
{
	__try
	{
		DWORD oldProtectionFlags = NULL;
		SIZE_T bytesWritten = 0;
		if ((alterMemoryAccessProtection) && (mbi != NULL)) oldProtectionFlags = AlterMemoryProtection(mbi, memoryAddress, numberOfBytes);
		BOOL resultWriteMemory = ((WriteProcessMemory(GetCurrentProcess(), memoryAddress, memoryBuffer, numberOfBytes, &bytesWritten) == 0) ? FALSE : TRUE);
		if ((alterMemoryAccessProtection) && (oldProtectionFlags != NULL)) RevertMemoryProtection(memoryAddress, numberOfBytes, oldProtectionFlags);
		return ((resultWriteMemory) && (bytesWritten == numberOfBytes));
	}
	__finally
	{
	}
}

// search the specified memory for the first RSA modulus key and patch if found
BOOL SearchMemoryAndPatchRSAModulusKey_0(PVOID startAddress, PVOID endAddress)
{
	__try
	{
		MEMORY_BASIC_INFORMATION mbi;
		LPVOID currentAddressPtr = (LPVOID)startAddress;
		LPVOID endAddressPtr = (LPVOID)endAddress;
		ULONG_PTR foundRSAKey[32];
		DWORD foundRSAKeyIndex = 0;

		// find rsa module string
		while (currentAddressPtr < endAddressPtr)
		{
			// query virutal memory
			if (VirtualQueryEx(GetCurrentProcess(), currentAddressPtr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
			{
				// only process specific memory ranges with certain characteristics
				if ((mbi.State == MEM_COMMIT) && (mbi.Type == MEM_IMAGE))
				{
					wsprintf(strDebugOutput, "Patch RSA:Action - Check Memory For RSA Modulus Key 0 (0x%012IX to 0x%012IX)", (ULONG_PTR)mbi.BaseAddress, (ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize - 1);
					DisplayDebugString(strDebugOutput, DEBUG_ADVANCED);

					// read memory region into buffer
					UCHAR* memoryBuffer = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap((ULONG_PTR)mbi.RegionSize);
					if (ReadMemoryAddress(currentAddressPtr, (ULONG_PTR)mbi.RegionSize, memoryBuffer, FALSE))
					{
						DWORD keyStrLength = 112;
						DWORD rsaModulusKeyDWORD = ULib.StringLibrary.getDWORDFromBuffer((UCHAR*)&AD_RSAMODULUSKEY_0, 0, false);

						// scan memory for match
						for (ULONG_PTR i = 0; i < ((ULONG_PTR)mbi.RegionSize - keyStrLength); i++)
						{
							// check memory for zero dword followed by first part of accessdata modulus key
							if ((ULib.StringLibrary.getDWORDFromBuffer(memoryBuffer, (INT)i, false) == 0) &&
								(ULib.StringLibrary.getDWORDFromBuffer(memoryBuffer, (INT)i + 4, false) == rsaModulusKeyDWORD))
							{
								// do full check for modulus key
								if (ULib.StringLibrary.isBufferMatch((UCHAR*)&memoryBuffer[i + 4], (UCHAR*)&AD_RSAMODULUSKEY_0, 16))
								{
									// store memory address where key was found
									if (foundRSAKeyIndex < 32)
									{
										foundRSAKey[foundRSAKeyIndex] = (ULONG_PTR)currentAddressPtr + i + 4;
										foundRSAKeyIndex++;

										wsprintf(strDebugOutput, "Patch RSA:Action - Found RSA Modulus Key 0 (0x%012IX)", (ULONG_PTR)currentAddressPtr + i + 4);
										DisplayDebugString(strDebugOutput);
									}
								}
							}
						}
					}

					ULib.HeapLibrary.freePrivateHeap(memoryBuffer);
				}

				currentAddressPtr = (LPVOID)((ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize);
			}
			else
			{
				currentAddressPtr = endAddressPtr;
			}
		}

		// found rsa key - now try to find references to this (i.e. rsa key array)
		if (foundRSAKeyIndex > 0)
		{
			currentAddressPtr = (LPVOID)startAddress;
			endAddressPtr = (LPVOID)endAddress;

			while (currentAddressPtr < endAddressPtr)
			{
				// query virtual memory
				if (VirtualQueryEx(GetCurrentProcess(), currentAddressPtr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
				{
					// only process specific memory ranges with certain characteristics
					if ((mbi.State == MEM_COMMIT) && (mbi.Type == MEM_IMAGE))
					{
						wsprintf(strDebugOutput, "Patch RSA:Action - Check Memory Region For RSA Modulus Key Array (0x%012IX to 0x%012IX)", (ULONG_PTR)mbi.BaseAddress, (ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize - 1);
						DisplayDebugString(strDebugOutput, DEBUG_ADVANCED);

						// read memory region into buffer
						UCHAR* memoryBuffer = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap((ULONG_PTR)mbi.RegionSize);
						if (ReadMemoryAddress(currentAddressPtr, (ULONG_PTR)mbi.RegionSize, memoryBuffer, FALSE))
						{
							// cycle through each rsa key found previously
							for (DWORD j = 0; j < foundRSAKeyIndex; j++)
							{
								// scan memory for match
								for (ULONG_PTR i = 0; i < ((ULONG_PTR)mbi.RegionSize - sizeof(rsa_meth_st)); i++)
								{
									BOOL bPatch = FALSE;
									INT memoryKeyOffset = 112;

									// found memory location pointing to rsa modulus key
									if (ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i, false) == foundRSAKey[j])
									{
										// check the memory locations following above for additional pointers to rsa keys
										if ((abs((INT)(ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + (sizeof(ULONG_PTR) * 1), false) - foundRSAKey[j])) == (memoryKeyOffset * 1)) &&
											(abs((INT)(ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + (sizeof(ULONG_PTR) * 2), false) - foundRSAKey[j])) == (memoryKeyOffset * 2)) &&
											(abs((INT)(ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + (sizeof(ULONG_PTR) * 3), false) - foundRSAKey[j])) == (memoryKeyOffset * 3)))
										{
											bPatch = TRUE;
											wsprintf(strDebugOutput, "Patch RSA:Action - Found RSA Modulus Key Array (0x%012IX)", (ULONG_PTR)currentAddressPtr + i);
											DisplayDebugString(strDebugOutput);
										}
									}

									// found memory location pointing to rsa modulus key - already patched
									if (ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i, false) == (ULONG_PTR)&RSAModulusKey_0)
									{
										// check the memory locations following above for additional pointers to rsa keys
										if ((abs((INT)(ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + (sizeof(ULONG_PTR) * 1), false) - foundRSAKey[j])) == (memoryKeyOffset * 1)) &&
											(abs((INT)(ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + (sizeof(ULONG_PTR) * 2), false) - foundRSAKey[j])) == (memoryKeyOffset * 2)) &&
											(abs((INT)(ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + (sizeof(ULONG_PTR) * 3), false) - foundRSAKey[j])) == (memoryKeyOffset * 3)))
										{
											bPatch = FALSE;
											wsprintf(strDebugOutput, "Patch RSA:Action - Already Patched RSA Modulus Key Array 0 Location (0x%012IX)", (ULONG_PTR)&RSAModulusKey_0);
											DisplayDebugString(strDebugOutput);
										}
									}

									// found valid location - patch memory
									if (bPatch)
									{
										UCHAR* tmpBuffer = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap(12);
										ULib.StringLibrary.addULONGToBuffer(tmpBuffer, (ULONG_PTR)&RSAModulusKey_0, 0, false);
										if (WriteMemoryAddress((LPVOID)((ULONG_PTR)currentAddressPtr + i), sizeof(ULONG_PTR), tmpBuffer, FALSE)) wsprintf(strDebugOutput, "Patch RSA:Action - Patch RSA Modulus Key Array 0 Location (0x%012IX to 0x%012IX)", ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i, false), (ULONG_PTR)&RSAModulusKey_0);
										else wsprintf(strDebugOutput, "Patch RSA:Error - Patch RSA Modulus Key Array 0 Location Failed (0x%012IX to 0x%012IX)", ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i, false), (ULONG_PTR)&RSAModulusKey_0);
										DisplayDebugString(strDebugOutput);
										ULib.HeapLibrary.freePrivateHeap(tmpBuffer);
									}
								}
							}
						}

						ULib.HeapLibrary.freePrivateHeap(memoryBuffer);
					}

					currentAddressPtr = (LPVOID)((ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize);
				}
				else
				{
					currentAddressPtr = endAddressPtr;
				}
			}
		}

	}
	__finally
	{
	}

	return TRUE;
}

// search the specified memory for the key RSA encryption module string and patch if found
BOOL SearchMemoryAndPatchRSA(PVOID startAddress, PVOID endAddress)
{
	__try
	{
		MEMORY_BASIC_INFORMATION mbi;
		LPVOID currentAddressPtr = (LPVOID)startAddress;
		LPVOID endAddressPtr = (LPVOID)endAddress;
		ULONG_PTR foundRSAModuleString[32];
		DWORD foundRSAModuleStringIndex = 0;

		// find rsa module string
		while (currentAddressPtr < endAddressPtr)
		{
			// query virutal memory
			if (VirtualQueryEx(GetCurrentProcess(), currentAddressPtr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
			{
				// only process specific memory ranges with certain characteristics
				if ((mbi.State == MEM_COMMIT) && (mbi.Type == MEM_IMAGE))
				{
					wsprintf(strDebugOutput, "Patch RSA:Action - Check Memory For RSA Module String (0x%012IX to 0x%012IX)", (ULONG_PTR)mbi.BaseAddress, (ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize - 1);
					DisplayDebugString(strDebugOutput, DEBUG_ADVANCED);

					// read memory region into buffer
					UCHAR* memoryBuffer = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap((ULONG_PTR)mbi.RegionSize);
					if (ReadMemoryAddress(currentAddressPtr, (ULONG_PTR)mbi.RegionSize, memoryBuffer, FALSE))
					{
						DWORD moduleStrLength = 23;
						DWORD rsaModuleStrDWORD = ULib.StringLibrary.getDWORDFromBuffer((UCHAR*)&AD_RSAMODULESTR, 0, false);

						// scan memory for match
						for (ULONG_PTR i = 0; i < ((ULONG_PTR)mbi.RegionSize - moduleStrLength); i++)
						{
							// check memory for first part of rsa module string
							if (ULib.StringLibrary.getDWORDFromBuffer(memoryBuffer, (INT)i, false) == rsaModuleStrDWORD)
							{
								// do full check for module string
								if (ULib.StringLibrary.isBufferMatch((UCHAR*)&memoryBuffer[i], (UCHAR*)&AD_RSAMODULESTR, moduleStrLength))
								{
									// store memory address where string was found
									if (foundRSAModuleStringIndex < 32)
									{
										foundRSAModuleString[foundRSAModuleStringIndex] = (ULONG_PTR)currentAddressPtr + i;
										foundRSAModuleStringIndex++;

										wsprintf(strDebugOutput, "Patch RSA:Action - Found RSA Module String (0x%012IX)", (ULONG_PTR)currentAddressPtr + i);
										DisplayDebugString(strDebugOutput);
									}
								}
							}
						}
					}

					ULib.HeapLibrary.freePrivateHeap(memoryBuffer);
				}

				currentAddressPtr = (LPVOID)((ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize);
			}
			else
			{
				currentAddressPtr = endAddressPtr;
			}
		}

		// found rsa module string - now try to find references to this (i.e. rsa structure)
		if (foundRSAModuleStringIndex > 0)
		{
			currentAddressPtr = (LPVOID)startAddress;
			endAddressPtr = (LPVOID)endAddress;

			while (currentAddressPtr < endAddressPtr)
			{
				// query virtual memory
				if (VirtualQueryEx(GetCurrentProcess(), currentAddressPtr, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION))
				{
					// only process specific memory ranges with certain characteristics
					if ((mbi.State == MEM_COMMIT) && (mbi.Type == MEM_IMAGE))
					{
						wsprintf(strDebugOutput, "Patch RSA:Action - Check Memory Region For RSA Method Structure (0x%012IX to 0x%012IX)", (ULONG_PTR)mbi.BaseAddress, (ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize - 1);
						DisplayDebugString(strDebugOutput, DEBUG_ADVANCED);

						// read memory region into buffer
						UCHAR* memoryBuffer = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap((ULONG_PTR)mbi.RegionSize);
						UCHAR* tmpBuffer = (UCHAR*)ULib.HeapLibrary.allocPrivateHeap(12);
						if (ReadMemoryAddress(currentAddressPtr, (ULONG_PTR)mbi.RegionSize, memoryBuffer, FALSE))
						{
							// cycle through each rsa module string found previously
							for (DWORD j = 0; j < foundRSAModuleStringIndex; j++)
							{
								// scan memory for match
								for (ULONG_PTR i = 0; i < ((ULONG_PTR)mbi.RegionSize - sizeof(rsa_meth_st)); i++)
								{
									// found possible match
									if (ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i, false) == foundRSAModuleString[j])
									{
										// get function address for rsa_pub_dec function
										if (ReadMemoryAddress((LPVOID)ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + 8, false), 6, tmpBuffer, FALSE))
										{
											DWORD functionBytes1a = 0x000008B8;
											DWORD functionBytes1b = 0x00000CB8;
											WORD functionBytes2 = 0xE800;
											DWORD functionBytesPatched = ULib.StringLibrary.getDWORDFromBuffer((UCHAR*)&_RSA_public_decrypt, 0, false);

											// check original rsa_pub_dec function matches expected bytes
											if (((ULib.StringLibrary.getDWORDFromBuffer(tmpBuffer, 0, false) == functionBytes1a) ||
												(ULib.StringLibrary.getDWORDFromBuffer(tmpBuffer, 0, false) == functionBytes1b)) &&
												(ULib.StringLibrary.getWORDFromBuffer(tmpBuffer, 0, false) != functionBytesPatched) &&
												(ULib.StringLibrary.getWORDFromBuffer(tmpBuffer, 4, false) == functionBytes2))
											{
												wsprintf(strDebugOutput, "Patch RSA:Action - Found RSA Method Structure (0x%012IX)", (ULONG_PTR)currentAddressPtr + i);
												DisplayDebugString(strDebugOutput);

												ULib.StringLibrary.addULONGToBuffer(tmpBuffer, (ULONG_PTR)&_RSA_public_decrypt, 0, false);
												if (WriteMemoryAddress((LPVOID)((ULONG_PTR)currentAddressPtr + i + 8), sizeof(ULONG_PTR), tmpBuffer, FALSE)) wsprintf(strDebugOutput, "Patch RSA:Action - Patch RSA Method Structure Function Call (0x%012IX to 0x%012IX)", ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + 8, false), (ULONG_PTR)&_RSA_public_decrypt);
												else wsprintf(strDebugOutput, "Patch RSA:Error - Patch RSA Method Structure Function Call Failed (0x%012IX to 0x%012IX)", ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + 8, false), (ULONG_PTR)&_RSA_public_decrypt);
												DisplayDebugString(strDebugOutput);
											}

											// check whether we've already patched the memory
											if (ULib.StringLibrary.getDWORDFromBuffer(tmpBuffer, 0, false) == functionBytesPatched)
											{
												wsprintf(strDebugOutput, "Patch RSA:Action - Found RSA Method Structure (0x%012IX)", (ULONG_PTR)currentAddressPtr + i);
												DisplayDebugString(strDebugOutput);

												if (ULib.StringLibrary.getULONGFromBuffer(memoryBuffer, (INT)i + 8, false) == (ULONG_PTR)&_RSA_public_decrypt)
												{
													wsprintf(strDebugOutput, "Patch RSA:Action - Already Patched RSA Method Structure Function Call (0x%012IX)", (ULONG_PTR)&_RSA_public_decrypt);
													DisplayDebugString(strDebugOutput);
												}
											}
										}
									}
								}
							}
						}

						ULib.HeapLibrary.freePrivateHeap(memoryBuffer);
						ULib.HeapLibrary.freePrivateHeap(tmpBuffer);
					}

					currentAddressPtr = (LPVOID)((ULONG_PTR)mbi.BaseAddress + (ULONG_PTR)mbi.RegionSize);
				}
				else
				{
					currentAddressPtr = endAddressPtr;
				}
			}
		}
	}
	__finally
	{
	}

	return TRUE;
}

// attempt to find rsa string and method structure in memory
// if found, patch to run custom rsa decryption function (above)
BOOL FindRSAMethodStructureAndPatch()
{
	DisplayDebugString(" ");
	DisplayDebugString("Patch RSA");

	// get list of modules running in current process
	DisplayDebugString("Patch RSA:Action - Parse Modules", DEBUG_ADVANCED);
	HANDLE moduleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
	if (moduleSnapshot == INVALID_HANDLE_VALUE) return DisplayDebugString("Patch RSA:Error - CreateToolhelp32Snapshot Failed", DEBUG_BASIC, FALSE);

	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(MODULEENTRY32);
	if (Module32First(moduleSnapshot, &mEntry) == FALSE) return DisplayDebugString("Patch RSA:Error - Module32First Failed", DEBUG_BASIC, FALSE);

	// try to discover the starting memory address of the process and
	// the absolute maximum end address before the module images
	PVOID dwProcessBaseAddress = 0;
	PVOID dwProcessEndAddress = ((ULib.ProcessLibrary.is64BitProcess()) ? (PVOID)MAXLONGLONG : (PVOID)MAXLONG);

	// additional modules to search within
	PVOID dwModuleD57BaseAddress = 0;
	PVOID dwModuleD57EndAddress = 0;
	PVOID dwModuleR_JNIBaseAddress = 0;
	PVOID dwModuleR_JNIEndAddress = 0;
	PVOID dwModuleADTriageBaseAddress = 0;
	PVOID dwModuleADTriageEndAddress = 0;

	do
	{
		wsprintf(strDebugOutput, "Patch RSA:Module (Name: %s, Base Address: 0x%012IX, Size: 0x%012IX)", mEntry.szModule, mEntry.modBaseAddr, mEntry.modBaseSize);
		DisplayDebugString(strDebugOutput, DEBUG_ADVANCED);

		// record memory address information
		if (dwProcessBaseAddress == 0) dwProcessBaseAddress = mEntry.modBaseAddr;
		else if ((dwProcessBaseAddress < mEntry.modBaseAddr) && (mEntry.modBaseAddr < dwProcessEndAddress)) dwProcessEndAddress = mEntry.modBaseAddr;

		// module - d57.dll (accessdata dongle module)
		if (ULib.StringLibrary.isStringMatch(mEntry.szModule, "d57.dll"))
		{
			dwModuleD57BaseAddress = mEntry.modBaseAddr;
			dwModuleD57EndAddress = mEntry.modBaseAddr + mEntry.modBaseSize;
		}
		// module - recovery_jni.dll (accessdata prtk/dna java module)
		if (ULib.StringLibrary.isStringMatch(mEntry.szModule, "recovery_jni.dll"))
		{
			dwModuleR_JNIBaseAddress = mEntry.modBaseAddr;
			dwModuleR_JNIEndAddress = mEntry.modBaseAddr + mEntry.modBaseSize;
		}
		// module - adtriagelicensing.dll (accessdata ad triage module)
		if (ULib.StringLibrary.isStringMatch(mEntry.szModule, "adtriagelicensing.dll"))
		{
			dwModuleADTriageBaseAddress = mEntry.modBaseAddr;
			dwModuleADTriageEndAddress = mEntry.modBaseAddr + mEntry.modBaseSize;
		}

	} while (Module32Next(moduleSnapshot, &mEntry));

	CloseHandle(moduleSnapshot);

	// display process search space
	wsprintf(strDebugOutput, "Patch RSA:Action - Search Process Memory Space (0x%012IX to 0x%012IX)", dwProcessBaseAddress, dwProcessEndAddress);
	DisplayDebugString(strDebugOutput);

	// validate process search space
	if ((dwProcessBaseAddress < (PVOID)(0x100000)) || (dwProcessBaseAddress > dwProcessEndAddress)) return DisplayDebugString("Patch RSA:Error - Process Memory Space Search Invalid", DEBUG_BASIC, FALSE);

	// scan process memory
	SearchMemoryAndPatchRSA(dwProcessBaseAddress, dwProcessEndAddress);
	SearchMemoryAndPatchRSAModulusKey_0(dwProcessBaseAddress, dwProcessEndAddress);

	// if module found, scan module memory
	if ((dwModuleD57BaseAddress > 0) && (dwModuleD57EndAddress) && (dwModuleD57BaseAddress < dwModuleD57EndAddress))
	{
		// display module search space
		wsprintf(strDebugOutput, "Patch RSA:Action - Search Module (d57.dll) Memory Space (0x%012IX to 0x%012IX)", dwModuleD57BaseAddress, dwModuleD57EndAddress);
		DisplayDebugString(strDebugOutput);

		SearchMemoryAndPatchRSA(dwModuleD57BaseAddress, dwModuleD57EndAddress);
		SearchMemoryAndPatchRSAModulusKey_0(dwModuleD57BaseAddress, dwModuleD57EndAddress);
	}

	// if module found, scan module memory
	if ((dwModuleR_JNIBaseAddress > 0) && (dwModuleR_JNIEndAddress) && (dwModuleR_JNIBaseAddress < dwModuleR_JNIEndAddress))
	{
		// display module search space
		wsprintf(strDebugOutput, "Patch RSA:Action - Search Module (recovery_jni.dll) Memory Space (0x%012IX to 0x%012IX)", dwModuleR_JNIBaseAddress, dwModuleR_JNIEndAddress);
		DisplayDebugString(strDebugOutput);

		SearchMemoryAndPatchRSA(dwModuleR_JNIBaseAddress, dwModuleR_JNIEndAddress);
		SearchMemoryAndPatchRSAModulusKey_0(dwModuleR_JNIBaseAddress, dwModuleR_JNIEndAddress);
	}

	// if module found, scan module memory
	if ((dwModuleADTriageBaseAddress > 0) && (dwModuleADTriageEndAddress) && (dwModuleADTriageBaseAddress < dwModuleADTriageEndAddress))
	{
		// display module search space
		wsprintf(strDebugOutput, "Patch RSA:Action - Search Module (adtriagelicensing.dll) Memory Space (0x%012IX to 0x%012IX)", dwModuleADTriageBaseAddress, dwModuleADTriageEndAddress);
		DisplayDebugString(strDebugOutput);

		SearchMemoryAndPatchRSA(dwModuleADTriageBaseAddress, dwModuleADTriageEndAddress);
		SearchMemoryAndPatchRSAModulusKey_0(dwModuleADTriageBaseAddress, dwModuleADTriageEndAddress);
	}

	return TRUE;
}

// get the next available handle (1 to HCM_MAX)
DWORD GetNextHandleIndex()
{
	for (DWORD i = 1; i <= HCM_MAX; i++)
	{
		if (hcmHandles[i] == 0)
		{
			return i;
		}
	}

	return 0;
}

// display found parameter options
VOID DisplayFoundParamterOptions(CHAR* strPrefix, CHAR* strOption)
{
	CHAR* debugStrFull = (CHAR*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(debugStrFull, "%s: - %s", strPrefix, strOption);
	DisplayDebugString(debugStrFull, DEBUG_CODEMETER);
	ULib.HeapLibrary.freePrivateHeap(debugStrFull);
}

// check parameters for recognised options
VOID DisplayParameterOptions(ULONG optionCategory, ULONG optionMask, CHAR* strPrefix)
{
	if (optionCategory == CM_ACCESS_FLCTRL)
	{
		if ((optionMask & CM_ACCESS_CMDMASK) == CM_ACCESS_LOCAL) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_LOCAL");
		if ((optionMask & CM_ACCESS_CMDMASK) == CM_ACCESS_LAN) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_LAN");
		if ((optionMask & CM_ACCESS_CMDMASK) == CM_ACCESS_LOCAL_LAN) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_LOCAL_LAN");
		if ((optionMask & CM_ACCESS_CMDMASK) == CM_ACCESS_LAN_LOCAL) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_LAN_LOCAL");
	}

	if (optionCategory == CM_ACCESS_MFLCTRL)
	{
		if ((optionMask & CM_ACCESS_NOUSERLIMIT) == CM_ACCESS_NOUSERLIMIT) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_NOUSERLIMIT");
		else if ((optionMask & CM_ACCESS_USERLIMIT) == CM_ACCESS_USERLIMIT) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_USERLIMIT");
		if ((optionMask & CM_ACCESS_EXCLUSIVE) == CM_ACCESS_EXCLUSIVE) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_EXCLUSIVE");
		if ((optionMask & CM_ACCESS_STATIONSHARE) == CM_ACCESS_STATIONSHARE) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_STATIONSHARE");
		if ((optionMask & CM_ACCESS_CONVENIENT) == CM_ACCESS_CONVENIENT) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_CONVENIENT");
		if ((optionMask & CM_ACCESS_STRUCTMASK) == CM_ACCESS_STRUCTMASK) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_STRUCTMASK");
		if ((optionMask & CM_ACCESS_FORCE) == CM_ACCESS_FORCE) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_FORCE");
		if ((optionMask & CM_ACCESS_CHECK_FSB) == CM_ACCESS_CHECK_FSB) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_CHECK_FSB");
		if ((optionMask & CM_ACCESS_CHECK_CTSB) == CM_ACCESS_CHECK_CTSB) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_CHECK_CTSB");
		if ((optionMask & CM_ACCESS_SUBSYSTEM) == CM_ACCESS_SUBSYSTEM) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_SUBSYSTEM");
		if ((optionMask & CM_ACCESS_FIRMITEM) == CM_ACCESS_FIRMITEM) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_FIRMITEM");
		if ((optionMask & CM_ACCESS_BORROW_ACCESS) == CM_ACCESS_BORROW_ACCESS) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_BORROW_ACCESS");
		if ((optionMask & CM_ACCESS_BORROW_RELEASE) == CM_ACCESS_BORROW_RELEASE) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_BORROW_RELEASE");
		if ((optionMask & CM_ACCESS_BORROW_VALIDATE) == CM_ACCESS_BORROW_VALIDATE) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_BORROW_VALIDATE");
		if ((optionMask & CM_ACCESS_BORROW_IGNORESTATE) == CM_ACCESS_BORROW_IGNORESTATE) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_BORROW_IGNORESTATE");
		if ((optionMask & CM_ACCESS_IGNORE_LINGERTIME) == CM_ACCESS_IGNORE_LINGERTIME) DisplayFoundParamterOptions(strPrefix, "CM_ACCESS_IGNORE_LINGERTIME");
	}

	if (optionCategory == CM_CHECKEVENTS_FLEVENTS)
	{
		if ((optionMask & CM_CE_BOXREMOVED) == CM_CE_BOXREMOVED) DisplayFoundParamterOptions(strPrefix, "CM_CE_BOXREMOVED");
		if ((optionMask & CM_CE_BOXREPLACED) == CM_CE_BOXREPLACED) DisplayFoundParamterOptions(strPrefix, "CM_CE_BOXREPLACED");
		if ((optionMask & CM_CE_BOXADDED) == CM_CE_BOXADDED) DisplayFoundParamterOptions(strPrefix, "CM_CE_BOXADDED");
		if ((optionMask & CM_CE_NETWORKLOST) == CM_CE_NETWORKLOST) DisplayFoundParamterOptions(strPrefix, "CM_CE_NETWORKLOST");
		if ((optionMask & CM_CE_NETWORKREPLACED) == CM_CE_NETWORKREPLACED) DisplayFoundParamterOptions(strPrefix, "CM_CE_NETWORKREPLACED");
		if ((optionMask & CM_CE_ENTRYMODIFIED) == CM_CE_ENTRYMODIFIED) DisplayFoundParamterOptions(strPrefix, "CM_CE_ENTRYMODIFIED");
		if ((optionMask & CM_CE_THRESHOLD_UNITCOUNTER) == CM_CE_THRESHOLD_UNITCOUNTER) DisplayFoundParamterOptions(strPrefix, "CM_CE_THRESHOLD_UNITCOUNTER");
		if ((optionMask & CM_CE_THRESHOLD_EXPDATE) == CM_CE_THRESHOLD_EXPDATE) DisplayFoundParamterOptions(strPrefix, "CM_CE_THRESHOLD_EXPDATE");
		if ((optionMask & CM_CE_SERVER_TERMINATED) == CM_CE_SERVER_TERMINATED) DisplayFoundParamterOptions(strPrefix, "CM_CE_SERVER_TERMINATED");
		if ((optionMask & CM_CE_BOXENABLED) == CM_CE_BOXENABLED) DisplayFoundParamterOptions(strPrefix, "CM_CE_BOXENABLED");
		if ((optionMask & CM_CE_BOXDISABLED) == CM_CE_BOXDISABLED) DisplayFoundParamterOptions(strPrefix, "CM_CE_BOXDISABLED");
		if ((optionMask & CM_CE_ENTRYALTERED) == CM_CE_ENTRYALTERED) DisplayFoundParamterOptions(strPrefix, "CM_CE_ENTRYALTERED");
		if ((optionMask & CM_CE_RELEASE_EVENT) == CM_CE_RELEASE_EVENT) DisplayFoundParamterOptions(strPrefix, "CM_CE_RELEASE_EVENT");
	}

	if (optionCategory == CM_GETBOXES_IDPORT)
	{
		if ((optionMask & CM_GB_ALLPORTS) == CM_GB_ALLPORTS) DisplayFoundParamterOptions(strPrefix, "CM_GB_ALLPORTS");
		if ((optionMask & CM_GB_DONGLE) == CM_GB_DONGLE) DisplayFoundParamterOptions(strPrefix, "CM_GB_DONGLE");
		if ((optionMask & CM_GB_SIM) == CM_GB_SIM) DisplayFoundParamterOptions(strPrefix, "CM_GB_SIM");
		if ((optionMask & CM_GB_ACT) == CM_GB_ACT) DisplayFoundParamterOptions(strPrefix, "CM_GB_ACT");
	}

	if (optionCategory == CM_GETINFO_FLCTRL)
	{
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BOXCONTROL) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BOXCONTROL");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BOXINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BOXINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BOXSECURITY) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BOXSECURITY");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BOXTIME) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BOXTIME");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENTRYDATA) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENTRYDATA");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENTRYINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENTRYINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_INTERNALENTRYINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_INTERNALENTRYINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_MEMINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_MEMINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_SIGNEDLIST) DisplayFoundParamterOptions(strPrefix, "CM_GEI_SIGNEDLIST");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_SIGNEDTIME) DisplayFoundParamterOptions(strPrefix, "CM_GEI_SIGNEDTIME");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_SYSTEM) DisplayFoundParamterOptions(strPrefix, "CM_GEI_SYSTEM");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_VERSION) DisplayFoundParamterOptions(strPrefix, "CM_GEI_VERSION");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENABLEBLOCKITEMS) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENABLEBLOCKITEMS");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENABLELOOKUPITEMS_FI) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENABLELOOKUPITEMS_FI");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENABLELOOKUPITEMS_PI) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENABLELOOKUPITEMS_PI");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_CHIPINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_CHIPINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BOXSTATUS) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BOXSTATUS");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_USBCHIPINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_USBCHIPINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_NETINFO_CLUSTER) DisplayFoundParamterOptions(strPrefix, "CM_GEI_NETINFO_CLUSTER");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_NETINFO_USER) DisplayFoundParamterOptions(strPrefix, "CM_GEI_NETINFO_USER");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_CREDENTIAL) DisplayFoundParamterOptions(strPrefix, "CM_GEI_CREDENTIAL");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_SECURITYVERSION) DisplayFoundParamterOptions(strPrefix, "CM_GEI_SECURITYVERSION");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_NETINFO_USER_EXT) DisplayFoundParamterOptions(strPrefix, "CM_GEI_NETINFO_USER_EXT");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_MEMINFO2) DisplayFoundParamterOptions(strPrefix, "CM_GEI_MEMINFO2");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ACT_LICENSE_INFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ACT_LICENSE_INFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ACT_ERROR_INFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ACT_ERROR_INFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_CMACTVERSION) DisplayFoundParamterOptions(strPrefix, "CM_GEI_CMACTVERSION");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BORROWCLIENT) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BORROWCLIENT");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BORROWDATA) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BORROWDATA");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_BORROWITEMS) DisplayFoundParamterOptions(strPrefix, "CM_GEI_BORROWITEMS");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENTRYINFO2) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENTRYINFO2");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ENABLEINFO) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ENABLEINFO");
		if ((optionMask & CM_GEI_CMDMASK) == CM_GEI_ACT_LICENSE_INFO2) DisplayFoundParamterOptions(strPrefix, "CM_GEI_ACT_LICENSE_INFO2");
		if ((optionMask & CM_GEI_USELOCALTIME) == CM_GEI_USELOCALTIME) DisplayFoundParamterOptions(strPrefix, "CM_GEI_USELOCALTIME");
	}
}

// CmGetLastErrorCode returns the last set error code
int CMAPIENTRY _CmGetLastErrorCode()
{
	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmGetLastErrorCode", DEBUG_CODEMETER);

	// display error code being returned
	wsprintf(strDebugOutput, "CmGetLastErrorCode:Action - Return Error Code: %d", cmLastErrorCode);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	return cmLastErrorCode;
}

// CmGetLastErrorText returns the last set error text
int CMAPIENTRY _CmGetLastErrorText(CMULONG flCtrl, char* pszErrorText, CMUINT cbErrorText)
{
	flCtrl; pszErrorText; cbErrorText;

	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmGetLastErrorText", DEBUG_CODEMETER);

	ULib.StringLibrary.copyString(pszErrorText, cbErrorText, "CM EMULATOR ERROR");
	// display error code being returned
	//wsprintf(strDebugOutput, "CmGetLastErrorText:Action - Return Error Code: %d", cmLastErrorCode);
	//DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	return 1;
}

// CmSetLastErrorCode sets an error code into the internal global error code variable
void CMAPIENTRY _CmSetLastErrorCode(int idError)
{
	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmSetLastErrorCode", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmSetLastErrorCode:Param - idError: %d", idError);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	//MessageBox(GetDesktopWindow(), "OK To Continue", "", MB_OK);
	//__asm int 3

	cmLastErrorCode = idError;
}

// CmSetLastErrorCode sets an error code into the internal global error code variable
void CMAPIENTRY _CmSetLastErrorCodeInternal(int idError)
{
	cmLastErrorCode = idError;
}

// CmAccess() opens the access to a SubSystem (SubSystem Access),
// to a CM-Stick (Box Access), to a single Firm Item (FC, Firm Item Access)
// or a CM-Stick entry (FC:PC, Entry Access) in a given subsystem
HCMSysEntry CMAPIENTRY _CmAccess(CMULONG flCtrl, CMACCESS *pcmAcc)
{
	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmAccess", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmAccess:Param - flCtrl: 0x%08X", flCtrl);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	DisplayParameterOptions(CM_ACCESS_FLCTRL, flCtrl, "CmAccess:Param - flCtrl");

	// check for missing parameters
	if (pcmAcc == NULL)
	{
		DisplayDebugString("CmAccess:Error - CMERROR_INVALID_PARAMETER", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_INVALID_PARAMETER);
		return NULL;
	}

	// check for disabled dongle
	if (bIsDisabled)
	{
		_CmSetLastErrorCodeInternal(CMERROR_FIRMCODEWRONG);
		return NULL;
	}

	// display function parameters
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc: 0x%08X", (ULONG)pcmAcc);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc: defined (size: 0x%04X)", sizeof(CMACCESS));
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mflCtrl: 0x%08X", pcmAcc->mflCtrl);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	DisplayParameterOptions(CM_ACCESS_MFLCTRL, pcmAcc->mflCtrl, "CmAccess:Param - pcmAcc->mflCtrl");
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mulFirmCode: %d", pcmAcc->mulFirmCode);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mulProductCode: %d", pcmAcc->mulProductCode);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mulFeatureCode: %d", pcmAcc->mulFeatureCode);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mulUsedRuntimeVersion: %d", pcmAcc->mulUsedRuntimeVersion);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->midProcess: %d", pcmAcc->midProcess);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->musProductItemReference: %d", pcmAcc->musProductItemReference);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->musSession: %d", pcmAcc->musSession);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mabIPv4Address: %d.%d.%d.%d", (unsigned int)pcmAcc->mabIPv4Address[0], (unsigned int)pcmAcc->mabIPv4Address[1], (unsigned int)pcmAcc->mabIPv4Address[2], (unsigned int)pcmAcc->mabIPv4Address[3]);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	if (((void*)&pcmAcc->mcmBoxInfo) == NULL)
	{
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo: not defined");
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	}
	else
	{
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo: defined (size: 0x%04X)", sizeof(CMBOXINFO));
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.mbMajorVersion: %d", pcmAcc->mcmBoxInfo.mbMajorVersion);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.mbMinorVersion: %d", pcmAcc->mcmBoxInfo.mbMinorVersion);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.musBoxMask: %d", pcmAcc->mcmBoxInfo.musBoxMask);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.mulSerialNumber: %d", pcmAcc->mcmBoxInfo.mulSerialNumber);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.musBoxKeyId: %d", pcmAcc->mcmBoxInfo.musBoxKeyId);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.musUserKeyId: %d", pcmAcc->mcmBoxInfo.musUserKeyId);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.mabBoxPublicKey: 0x%02X 0x%02X 0x%02X 0x%02X ...", pcmAcc->mcmBoxInfo.mabBoxPublicKey[0], pcmAcc->mcmBoxInfo.mabBoxPublicKey[1], pcmAcc->mcmBoxInfo.mabBoxPublicKey[2], pcmAcc->mcmBoxInfo.mabBoxPublicKey[3]);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.mabSerialPublicKey: 0x%02X 0x%02X 0x%02X 0x%02X ...", pcmAcc->mcmBoxInfo.mabSerialPublicKey[0], pcmAcc->mcmBoxInfo.mabSerialPublicKey[1], pcmAcc->mcmBoxInfo.mabSerialPublicKey[2], pcmAcc->mcmBoxInfo.mabSerialPublicKey[3]);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmAccess:Param - pcmAcc->mcmBoxInfo.mulReserve: %d", pcmAcc->mcmBoxInfo.mulReserve);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	}

	// if firm code and product code are set then check they are set to accessdata values
	if ((pcmAcc->mulFirmCode > 0) && (pcmAcc->mulProductCode > 0))
	{
		if (pcmAcc->mulFirmCode != AD_FIRMCODE)
		{
			DisplayDebugString("CmAccess:Error - CMERROR_FIRMCODEWRONG", DEBUG_CODEMETER);
			_CmSetLastErrorCodeInternal(CMERROR_FIRMCODEWRONG);
			return NULL;
		}
		else
		{
			if ((pcmAcc->mulProductCode != AD_PRODUCTCODE100) && (pcmAcc->mulProductCode != AD_PRODUCTCODE1000))
			{
				DisplayDebugString("CmAccess:Error - CMERROR_PRODUCTCODEWRONG", DEBUG_CODEMETER);
				_CmSetLastErrorCodeInternal(CMERROR_PRODUCTCODEWRONG);
				return NULL;
			}
		}
	}

	// get next handle
	DWORD hcmIndex = GetNextHandleIndex();
	if ((hcmIndex <= 0) || (hcmIndex > HCM_MAX))
	{
		DisplayDebugString("CmAccess:Error - CMERROR_NO_MORE_HANDLES", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_NO_MORE_HANDLES);
		return NULL;
	}

	// assign handle
	hcmHandles[hcmIndex] = 1;
	hcmProductCode[hcmIndex] = pcmAcc->mulProductCode;
	pcmAcc->midProcess = GetCurrentProcessId();
	pcmAcc->musSession = (CMUSHORT)hcmIndex;

	// set values
	wsprintf(strDebugOutput, "CmAccess:Action - Assigned Handle: %d", hcmIndex);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Action - Set Process (pcmAcc->midProcess): %d", pcmAcc->midProcess);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmAccess:Action - Set Session (pcmAcc->musSession): %d", pcmAcc->musSession);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	pcmAcc->mcmBoxInfo.mabBoxPublicKey[0] = 0xAA;
	pcmAcc->mcmBoxInfo.mabSerialPublicKey[0] = 0xBB;

	_CmSetLastErrorCodeInternal(CMERROR_NO_ERROR);
	return (HCMSysEntry)hcmIndex;
}

// CmAccess2() opens the access to a SubSystem (SubSystem Access),
// to a CM-Stick (Box Access), to a single Firm Item (FC, Firm Item Access)
// or a CM-Stick entry (FC:PC, Entry Access) in a given subsystem
HCMSysEntry CMAPIENTRY _CmAccess2(CMULONG flCtrl, CMACCESS *pcmAcc)
{
	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmAccess2", DEBUG_CODEMETER);
	DisplayDebugString("CmAccess2:Action - Calling Function CmAccess", DEBUG_CODEMETER);

	return _CmAccess(flCtrl, pcmAcc);
}

// CmRelease closes a handle opened by CmAccess and all related subsystem accesses
int CMAPIENTRY _CmRelease(HCMSysEntry hcmse)
{
	DWORD hcmIndex = (DWORD)hcmse;

	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmRelease", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmRelease:Param - hcmse: %d", hcmse);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	// check for valid handles
	if ((hcmIndex <= 0) || (hcmIndex > HCM_MAX))
	{
		DisplayDebugString("CmRelease:Error - CMERROR_BAD_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_BAD_HANDLE);
		return FALSE;
	}

	// check for valid handles
	if (hcmHandles[hcmIndex] != 1)
	{
		DisplayDebugString("CmRelease:Error - CMERROR_INVALID_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_INVALID_HANDLE);
		return FALSE;
	}

	// release handle
	hcmHandles[hcmIndex] = 0;
	wsprintf(strDebugOutput, "CmRelease:Action - Released Handle: %d", hcmIndex);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	_CmSetLastErrorCodeInternal(CMERROR_NO_ERROR);
	return TRUE;
}

unsigned long CMAPIENTRY _CmGetVersion(HCMSysEntry hcmse)
{
	hcmse;
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmGetVersion", DEBUG_CODEMETER);

	return 0;
}

unsigned long CMAPIENTRY _CmCheckEvents(CMULONG flEvents)
{
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmCheckEvents", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmCheckEvents:Param - flEvents: 0x%08X", flEvents);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	DisplayParameterOptions(CM_CHECKEVENTS_FLEVENTS, flEvents, "CmCheckEvents:Param - flEvents");

	return flEvents;
}

// CmGetBoxes returns all connected CM-Sticks of the given port
int CMAPIENTRY _CmGetBoxes(HCMSysEntry hcmse, CMULONG idPort, CMBOXINFO *pcmBoxInfo, CMUINT cbBoxInfo)
{
	DWORD hcmIndex = (DWORD)hcmse;

	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmGetBoxes", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmGetBoxes:Param - hcmse: %d", hcmse);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmGetBoxes:Param - idPort: 0x%08X", idPort);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	DisplayParameterOptions(CM_GETBOXES_IDPORT, idPort, "CmGetBoxes:Param - idPort");
	if (pcmBoxInfo == NULL)
	{
		wsprintf(strDebugOutput, "CmGetBoxes:Param - pcmBoxInfo: not defined");
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	}
	else
	{
		wsprintf(strDebugOutput, "CmGetBoxes:Param - pcmBoxInfo: 0x%08X", (ULONG)pcmBoxInfo);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmGetBoxes:Param - pcmBoxInfo: defined (size: 0x%04X)", cbBoxInfo * sizeof(CMBOXINFO));
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	}
	wsprintf(strDebugOutput, "CmGetBoxes:Param - cbBoxInfo: %d", cbBoxInfo);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	// check for valid handles
	if ((hcmIndex <= 0) || (hcmIndex > HCM_MAX))
	{
		DisplayDebugString("CmGetBoxes:Error - CMERROR_BAD_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_BAD_HANDLE);
		return 0;
	}

	// check for valid handles
	if (hcmHandles[hcmIndex] != 1)
	{
		DisplayDebugString("CmGetBoxes:Error - CMERROR_INVALID_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_INVALID_HANDLE);
		return 0;
	}

	// buffer overflow - checking size for follow up call
	if ((pcmBoxInfo == NULL) || (cbBoxInfo <= 0))
	{
		DisplayDebugString("CmGetBoxes:Error - CMERROR_BUFFER_OVERFLOW", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
		return 1;
	}

	// populate box info
	if ((pcmBoxInfo != NULL) && (cbBoxInfo > 0))
	{
		pcmBoxInfo->musBoxMask = 1;
		pcmBoxInfo->mulSerialNumber = dongleSerialNumber;
		pcmBoxInfo->mbMajorVersion = 1;
		pcmBoxInfo->mbMinorVersion = 0;

		pcmBoxInfo->musBoxKeyId = 1;
		pcmBoxInfo->musUserKeyId = 1;
		UCHAR* keyString = (UCHAR*)ULib.StringLibrary.getString('*', 64);
		ULib.StringLibrary.copyBuffer(pcmBoxInfo->mabBoxPublicKey, 64, keyString, 64);
		ULib.StringLibrary.copyBuffer(pcmBoxInfo->mabSerialPublicKey, 64, keyString, 64);
		ULib.HeapLibrary.freePrivateHeap(keyString);

		_CmSetLastErrorCodeInternal(CMERROR_NO_ERROR);

		wsprintf(strDebugOutput, "CmGetBoxes:Action - Set Major Version (pcmBoxInfo->mbMajorVersion): %d", pcmBoxInfo->mbMajorVersion);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmGetBoxes:Action - Set Minor Version (pcmBoxInfo->mbMajorVersion): %d", pcmBoxInfo->mbMinorVersion);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmGetBoxes:Action - Set Box Mask (pcmBoxInfo->musBoxMask): %d", pcmBoxInfo->musBoxMask);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		wsprintf(strDebugOutput, "CmGetBoxes:Action - Set Serial Number (pcmBoxInfo->mulSerialNumber): %d", pcmBoxInfo->mulSerialNumber);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

		return 1;
	}

	_CmSetLastErrorCodeInternal(CMERROR_BOX_NOT_FOUND);
	return 0;
}

// CmGetInfo returns data related to the opened entry
int CMAPIENTRY _CmGetInfo(HCMSysEntry hcmse, CMULONG flCtrl, void *pvDest, CMUINT cbDest)
{
	DWORD hcmIndex = (DWORD)hcmse;

	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmGetInfo", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmGetInfo:Param - hcmse: %d", hcmse);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmGetInfo:Param - flCtrl: 0x%08X", flCtrl);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	DisplayParameterOptions(CM_GETINFO_FLCTRL, flCtrl, "CmGetInfo:Param - flCtrl");
	if (pvDest == NULL)
	{
		wsprintf(strDebugOutput, "CmGetInfo:Param - pvDest: not defined");
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	}
	else
	{
		wsprintf(strDebugOutput, "CmGetInfo:Param - pvDest: defined (size: 0x%04X)", cbDest);
		DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	}
	wsprintf(strDebugOutput, "CmGetInfo:Param - cbDest: %d", cbDest);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	// check for valid handles
	if ((hcmIndex <= 0) || (hcmIndex > HCM_MAX))
	{
		DisplayDebugString("CmGetInfo:Error - CMERROR_BAD_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_BAD_HANDLE);
		return 0;
	}

	// check for valid handles
	if (hcmHandles[hcmIndex] != 1)
	{
		DisplayDebugString("CmGetInfo:Error - CMERROR_INVALID_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_INVALID_HANDLE);
		return 0;
	}

	// which info to return
	switch (flCtrl)
	{
		case CM_GEI_BOXINFO:
		{
			DisplayDebugString("CmGetInfo:Action - CM_GEI_BOXINFO", DEBUG_CODEMETER);

			// buffer overflow - checking size for follow up call
			if ((pvDest == NULL) || (cbDest < sizeof(CMBOXINFO)))
			{
				DisplayDebugString("CmGetInfo:Error - CMERROR_BUFFER_OVERFLOW", DEBUG_CODEMETER);
				_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
				return sizeof(CMBOXINFO);
			}

			CMBOXINFO* pCmBoxInfo = ((CMBOXINFO*)pvDest);

			// set dongle serial number (fixed)
			pCmBoxInfo->mulSerialNumber = dongleSerialNumberFixed;

			wsprintf(strDebugOutput, "CmGetInfo:Action - Set Serial Number (pCmBoxInfo->mulSerialNumber): %d", pCmBoxInfo->mulSerialNumber);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

			break;
		}

		case CM_GEI_ENTRYINFO:
		{
			DisplayDebugString("CmGetInfo:Action - CM_GEI_ENTRYINFO", DEBUG_CODEMETER);

			// buffer overflow - checking size for follow up call
			if ((pvDest == NULL) || (cbDest < sizeof(CMBOXENTRY)))
			{
				DisplayDebugString("CmGetInfo:Error - CMERROR_BUFFER_OVERFLOW", DEBUG_CODEMETER);
				_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
				return sizeof(CMBOXENTRY);
			}

			CMBOXENTRY* pCmBoxEntry = ((CMBOXENTRY*)pvDest);

			// display function parameters
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mflFiCtrl: %d", pCmBoxEntry->mflFiCtrl);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mulFirmCode: %d", pCmBoxEntry->mulFirmCode);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->musFirmAccessCounter: %d", pCmBoxEntry->musFirmAccessCounter);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->musReserve1: %d", pCmBoxEntry->musReserve1);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mulFirmUpdateCounter: %d", pCmBoxEntry->mulFirmUpdateCounter);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mulFirmPreciseTime: %d", pCmBoxEntry->mulFirmPreciseTime);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mausFirmItemText: %s", ULib.StringLibrary.convertUnicodeToAnsi((wchar_t*)pCmBoxEntry->mausFirmItemText));
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mflSetPios: %d", pCmBoxEntry->mflSetPios);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mulProductCode: %d", pCmBoxEntry->mulProductCode);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mulFeatureMap: %d", pCmBoxEntry->mulFeatureMap);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Param - pCmBoxEntry->mulUnitCounter: %d", pCmBoxEntry->mulUnitCounter);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

			pCmBoxEntry->mulFirmCode = AD_FIRMCODE;
			pCmBoxEntry->mulProductCode = hcmProductCode[hcmIndex];
			pCmBoxEntry->mflSetPios = CM_GF_EXTPROTDATA;

			wsprintf(strDebugOutput, "CmGetInfo:Action - Set Firm Code (pCmBoxEntry->mulFirmCode): %d", pCmBoxEntry->mulFirmCode);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Action - Set Product Code (pCmBoxEntry->mulProductCode): %d", pCmBoxEntry->mulProductCode);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			wsprintf(strDebugOutput, "CmGetInfo:Action - Set ProductItem Option: CM_GF_EXTPROTDATA (pCmBoxEntry->mflSetPios): 0x%04X", pCmBoxEntry->mflSetPios);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

			// attempt to find rsa string and method structure in memory
			// if found, patch to run custom rsa decryption function
			FindRSAMethodStructureAndPatch();

			// create dongle packet
			pDonglePacket100 = (AD_DONGLEPACKET100*)ULib.HeapLibrary.allocPrivateHeap(sizeof(AD_DONGLEPACKET100));
			CreateDonglePacket100(pDonglePacket100);

			break;
		}

		case CM_GEI_ENTRYDATA:
		{
			DisplayDebugString("CmGetInfo:Action - CM_GEI_ENTRYDATA", DEBUG_CODEMETER);

			// product code 100 dongle
			if (hcmProductCode[hcmIndex] == AD_PRODUCTCODE100)
			{
				if (pDonglePacket100 == NULL)
				{
					DisplayDebugString("CmGetInfo:Error - Dongle Packet Not Created");
					_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
					return 0;
				}

				// buffer overflow - checking size for follow up call
				if ((pvDest == NULL) || (cbDest < sizeof(CMENTRYDATA)))
				{
					DisplayDebugString("CmGetInfo:Error - CMERROR_BUFFER_OVERFLOW", DEBUG_CODEMETER);
					_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
					return (sizeof(CMENTRYDATA) * pDonglePacket100->numberOfEntryDataBlocks);
				}

				DisplayDebugString("CmGetInfo:Action - Setting CmEntryData", DEBUG_CODEMETER);
				CMENTRYDATA* pCmEntryData = ((CMENTRYDATA*)pvDest);
				memset((void*)pCmEntryData, 0, (sizeof(CMENTRYDATA) * pDonglePacket100->numberOfEntryDataBlocks));

				// only one block required
				if (pDonglePacket100->numberOfEntryDataBlocks == 1)
				{
					pCmEntryData->mflCtrl = CM_GF_EXTPROTDATA;
					ULib.StringLibrary.copyBuffer((UCHAR*)&pCmEntryData->mabData, (UCHAR*)&pDonglePacket100->dongleHeader, pDonglePacket100->dongleHeaderLength);
					ULib.StringLibrary.copyBuffer((UCHAR*)&pCmEntryData->mabData[pDonglePacket100->dongleHeaderLength], (UCHAR*)pDonglePacket100->xmlLicenceEncrypted, pDonglePacket100->xmlLicenceEncryptedLength);
					pCmEntryData->mcbData = pDonglePacket100->donglePacketLength;
				}
				else
				{
					unsigned long xmlLicenceEncryptedLengthRemaining = pDonglePacket100->xmlLicenceEncryptedLength;
					unsigned long xmlLicenceEncryptedLengthToCopy = 0;

					// create all the required data entries
					for (unsigned long i = 0; i < pDonglePacket100->numberOfEntryDataBlocks; i++)
					{
						// set index (and buffer offset)
						pCmEntryData->mflCtrl = ((i << 16) | CM_GF_EXTPROTDATA);

						// include dongle header
						if (i == 0)
						{
							xmlLicenceEncryptedLengthToCopy = CM_MAX_STRING_LEN - pDonglePacket100->dongleHeaderLength;
							ULib.StringLibrary.copyBuffer((UCHAR*)&pCmEntryData->mabData, (UCHAR*)&pDonglePacket100->dongleHeader, pDonglePacket100->dongleHeaderLength);
							ULib.StringLibrary.copyBuffer((UCHAR*)&pCmEntryData->mabData[pDonglePacket100->dongleHeaderLength], (UCHAR*)pDonglePacket100->xmlLicenceEncrypted, xmlLicenceEncryptedLengthToCopy);
							pCmEntryData->mcbData = CM_MAX_STRING_LEN;
						}
						else
						{
							xmlLicenceEncryptedLengthToCopy = ((xmlLicenceEncryptedLengthRemaining > CM_MAX_STRING_LEN) ? CM_MAX_STRING_LEN : xmlLicenceEncryptedLengthRemaining);
							ULib.StringLibrary.copyBuffer((UCHAR*)&pCmEntryData->mabData, (UCHAR*)&pDonglePacket100->xmlLicenceEncrypted[pDonglePacket100->xmlLicenceEncryptedLength - xmlLicenceEncryptedLengthRemaining], xmlLicenceEncryptedLengthToCopy);
							pCmEntryData->mcbData = xmlLicenceEncryptedLengthToCopy;
						}

						xmlLicenceEncryptedLengthRemaining -= xmlLicenceEncryptedLengthToCopy;
						pCmEntryData++;
					}
				}
			}

			// product code 1000 dongle
			if (hcmProductCode[hcmIndex] == AD_PRODUCTCODE1000)
			{
				if (pDonglePacket1000 == NULL)
				{
					// create dongle packet
					pDonglePacket1000 = (AD_DONGLEPACKET1000*)ULib.HeapLibrary.allocPrivateHeap(sizeof(AD_DONGLEPACKET1000));
					CreateDonglePacket1000(pDonglePacket1000);
				}

				// buffer overflow - checking size for follow up call
				if ((pvDest == NULL) || (cbDest < sizeof(CMENTRYDATA)))
				{
					DisplayDebugString("CmGetInfo:Error - CMERROR_BUFFER_OVERFLOW", DEBUG_CODEMETER);
					_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
					return (sizeof(CMENTRYDATA) * 1);
				}

				if (pDonglePacket1000 == NULL)
				{
					DisplayDebugString("CmGetInfo:Error - Dongle Packet Not Created");
					_CmSetLastErrorCodeInternal(CMERROR_BUFFER_OVERFLOW);
					return 0;
				}

				DisplayDebugString("CmGetInfo:Action - Setting CmEntryData", DEBUG_CODEMETER);
				CMENTRYDATA* pCmEntryData = ((CMENTRYDATA*)pvDest);
				memset((void*)pCmEntryData, 0, (sizeof(CMENTRYDATA) * 1));

				// only one block required
				pCmEntryData->mflCtrl = CM_GF_PROTDATA;
				ULib.StringLibrary.copyBuffer((UCHAR*)pCmEntryData->mabData, (UCHAR*)pDonglePacket1000, sizeof(AD_DONGLEPACKET1000));
				pCmEntryData->mcbData = sizeof(AD_DONGLEPACKET1000);
			}

			break;
		}

		default:
		{
			wsprintf(strDebugOutput, "CmGetInfo:Error - Unknown Action (%d)", flCtrl);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
		}
	}

	return cbDest;
}

// CmGetRemoteContext stores the content of a box into an
// encrypted and compressed Remote Context file
int CMAPIENTRY _CmGetRemoteContext(const char *pszRacFile, CMUINT flCtrl, HCMSysEntry hcmBox, CMUINT *pulFirmCodes, CMUINT cbFirmCodes)
{
	DWORD hcmIndex = (DWORD)hcmBox;
	CHAR* fCodesStr = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(1024);
	DWORD fCodesWritten = 0;

	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmGetRemoteContext", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmGetRemoteContext:Param - pszRacFile: %s", ((pszRacFile == NULL) ? " " : pszRacFile));
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmGetRemoteContext:Param - flCtrl: 0x%08X", flCtrl);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmGetRemoteContext:Param - hcmBox: %d", hcmIndex);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmGetRemoteContext:Param - pulFirmCodes: %s", ((pulFirmCodes == NULL) ? "not defined" : "defined"));
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmGetRemoteContext:Param - cbFirmCodes: %d", cbFirmCodes);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	if ((cbFirmCodes > 0) && (pulFirmCodes != NULL))
	{
		CMUINT* pulFirmCodesCopy = pulFirmCodes;
		for (DWORD i = 0; i < cbFirmCodes; i++)
		{
			ULib.StringLibrary.appendString(fCodesStr, (CHAR*)pulFirmCodesCopy);
			ULib.StringLibrary.appendString(fCodesStr, "\r\n");
			wsprintf(strDebugOutput, "CmGetRemoteContext:Param - pulFirmCodes.#%02d: %d", (i + 1), *pulFirmCodesCopy);
			DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
			if (i < (cbFirmCodes - 1)) pulFirmCodesCopy++;
		}
	}

	// check for valid handles
	if ((hcmIndex <= 0) || (hcmIndex > HCM_MAX))
	{
		DisplayDebugString("CmGetRemoteContext:Error - CMERROR_BAD_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_BAD_HANDLE);
		return fCodesWritten;
	}

	// check for valid handles
	if (hcmHandles[hcmIndex] != 1)
	{
		DisplayDebugString("CmGetRemoteContext:Error - CMERROR_INVALID_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_INVALID_HANDLE);
		return fCodesWritten;
	}

	/*if (ULib.FileLibrary.writeBufferToFile((CHAR*)pszRacFile, (UCHAR*)fCodesStr, ULib.StringLibrary.getStringLength(fCodesStr)))
	{
		DisplayDebugString("CmGetBoxes:Action - Create Remote Context File", DEBUG_CODEMETER);
		fCodesWritten = cbFirmCodes;
	}
	else
	{
		DisplayDebugString("CmGetBoxes:Error - Create Remote Context File Failed", DEBUG_CODEMETER);
	}*/

	ULib.HeapLibrary.freePrivateHeap(fCodesStr);
	return fCodesWritten;
}

// CmSetRemoteUpdate programs a CM-Stick with a given
// Remote Activation Update file (WibuCmRaU file)
int CMAPIENTRY _CmSetRemoteUpdate(const char *pszRauFile, HCMSysEntry hcmBox)
{
	DWORD hcmIndex = (DWORD)hcmBox;

	// display function called
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmSetRemoteUpdate", DEBUG_CODEMETER);

	// display function parameters
	wsprintf(strDebugOutput, "CmSetRemoteUpdate:Param - pszRauFile: %s", ((pszRauFile == NULL) ? " " : pszRauFile));
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmSetRemoteUpdate:Param - hcmBox: %d", hcmIndex);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	// check for valid handles
	if ((hcmIndex <= 0) || (hcmIndex > HCM_MAX))
	{
		DisplayDebugString("CmSetRemoteUpdate:Error - CMERROR_BAD_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_BAD_HANDLE);
		return FALSE;
	}

	// check for valid handles
	if (hcmHandles[hcmIndex] != 1)
	{
		DisplayDebugString("CmSetRemoteUpdate:Error - CMERROR_INVALID_HANDLE", DEBUG_CODEMETER);
		_CmSetLastErrorCodeInternal(CMERROR_INVALID_HANDLE);
		return FALSE;
	}

	return TRUE;
}

int CMAPIENTRY _CmActLicenseControl(const char *pszRacFile, CMUINT flCtrl, HCMSysEntry hcmBox, CMUINT *pulFirmCodes, CMUINT cbFirmCodes, CMUINT cbFirmCodes2)
{
	pszRacFile; flCtrl; hcmBox; pulFirmCodes; cbFirmCodes; cbFirmCodes2;
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmActLicenseControl", DEBUG_CODEMETER);

	return TRUE;
}

int CMAPIENTRY _CmSetCertifiedTimeUpdate(HCMSysEntry hcmse, CHAR* pszCtcsServer)
{
	hcmse; pszCtcsServer;
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmSetCertifiedTimeUpdate", DEBUG_CODEMETER);

	return TRUE;
}

int CMAPIENTRY _CmCrypt(HCMSysEntry hcmse, CMULONG flCtrl, CMCRYPT* pcmCrypt, void* pvDest, CMUINT cbDest)
{
	hcmse; flCtrl; pcmCrypt; pvDest; cbDest;
	DisplayDebugString(" ", DEBUG_CODEMETER);
	DisplayDebugString("CmCrypt", DEBUG_CODEMETER);

	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mcmBaseCrypt.mflCtrl: 0x%08X", pcmCrypt->mcmBaseCrypt.mflCtrl);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mcmBaseCrypt.mulKeyExtType: %d", pcmCrypt->mcmBaseCrypt.mulKeyExtType);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mcmBaseCrypt.mulEncryptionCode: 0x%08X (%d)", pcmCrypt->mcmBaseCrypt.mulEncryptionCode, pcmCrypt->mcmBaseCrypt.mulEncryptionCode);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mcmBaseCrypt.mulEncryptionCodeOptions: 0x%08X (%d)", pcmCrypt->mcmBaseCrypt.mulEncryptionCodeOptions, pcmCrypt->mcmBaseCrypt.mulEncryptionCodeOptions);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mcmBaseCrypt.mulFeatureCode: %d", pcmCrypt->mcmBaseCrypt.mulFeatureCode);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mcmBaseCrypt.mulCrc: 0x%08X (%d)", pcmCrypt->mcmBaseCrypt.mulCrc, pcmCrypt->mcmBaseCrypt.mulCrc);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pcmCrypt->mabInitKey: 0x%02X 0x%02X 0x%02X 0x%02X", pcmCrypt->mabInitKey[0], pcmCrypt->mabInitKey[1], pcmCrypt->mabInitKey[2], pcmCrypt->mabInitKey[3]);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);
	wsprintf(strDebugOutput, "CmCrypt:Param - pvDest: 0x%02X 0x%02X 0x%02X 0x%02X", ((CHAR*)pvDest)[0], ((CHAR*)pvDest)[1], ((CHAR*)pvDest)[2], ((CHAR*)pvDest)[3]);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	wsprintf(strDebugOutput, "CmCrypt:Param - flCtrl: 0x%08X", flCtrl);
	DisplayDebugString(strDebugOutput, DEBUG_CODEMETER);

	/*memset(pvDest, 1, 16);
	pcmCrypt->mcmBaseCrypt.mulCrc = 0xEBDC6395;
	MessageBox(GetDesktopWindow(), "OK To Continue", "", MB_OK);
	__asm int 3*/

	return 16;
}

// entry point
BOOL WINAPI DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID dwReserved)
{
	hDll; dwReserved;

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			ULib.ProcessLibrary.disableUnhandledExceptionFilter();
			ULib.ConsoleLibrary.disableConsoleControlHandler();
			ULib.ProcessLibrary.setDefaultExitProcessFlags(ULib.ProcessLibrary.EXITPROCESS_DISPLAYMESSAGE_NEVER);

			strDebugOutput = (CHAR*)ULib.HeapLibrary.allocPrivateHeap(1024);
			GetRegistrySettings();
			GenerateDynamicDongleValues();
			LoadAndStoreRSAModulusKey_0();
			DisplayVersionInformation();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
	}

	return TRUE;
}