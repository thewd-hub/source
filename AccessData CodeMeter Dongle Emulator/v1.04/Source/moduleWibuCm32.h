///////////////////////////////////////////////////////////
// AccessData CodeMeter Dongle Emulator                  //
// (C)thewd, thewd@hotmail.com                           //
///////////////////////////////////////////////////////////

#define MODULE_VERSION "AccessData CodeMeter Dongle Emulator v1.04.121122"

#define _CODEMETER_NODEFLIB
#define HCM_MAX 64

#define REGISTRYKEY_SETTINGS "Software\\AccessData\\CodeMeter Dongle Emulator"

#define DEBUG_NONE 0
#define DEBUG_BASIC 1
#define DEBUG_ADVANCED 2
#define DEBUG_CODEMETER 3

#define AD_RSAMODULESTR "Eric Young's PKCS#1 RSA\0"
#define AD_RSAMODULUSKEY_0 "\xBC\xB4\xF5\xC7\x9A\x63\x9D\xF8\x7B\x0D\x23\x03\x0C\xA2\xCB\xAB"
#define AD_AESKEY "accessdatadongle\0"

#define AD_FIRMCODE 100607
#define AD_PRODUCTCODE100 100
#define AD_PRODUCTCODE1000 1000
#define AD_DONGLEVERSION 1.00
#define AD_DONGLESERIALFIXED 1231460

#define CM_ACCESS_FLCTRL 1
#define CM_ACCESS_MFLCTRL 2
#define CM_CHECKEVENTS_FLEVENTS 3
#define CM_GETBOXES_IDPORT 4
#define CM_GETINFO_FLCTRL 5

// dongle header
#pragma pack(1)
typedef struct _AD_DONGLEHEADER
{
	USHORT dongleVersion;
	USHORT rsaKeyId;
	UCHAR aesKey[16];
	UCHAR aesKeyRSAEncrypted[108];
} AD_DONGLEHEADER;
#pragma pack()

// dongle packet
#pragma pack(1)
typedef struct _AD_DONGLEPACKET100
{
	AD_DONGLEHEADER dongleHeader;
	UCHAR* xmlLicenceEncrypted;
	ULONG dongleHeaderLength;
	ULONG xmlLicenceEncryptedLength;
	ULONG donglePacketLength;
	ULONG numberOfEntryDataBlocks;
} AD_DONGLEPACKET100;
#pragma pack()

// dongle packet
#pragma pack(1)
typedef struct _AD_DONGLEPACKET1000
{
	UCHAR licenceCount;
	UCHAR expiryYear;
	UCHAR expiryMonth;
	UCHAR expiryDay;
} AD_DONGLEPACKET1000;
#pragma pack()