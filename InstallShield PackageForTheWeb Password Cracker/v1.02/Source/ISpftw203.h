///////////////////////////////////////////////////////
// InstallShield PackageForTheWeb Password Cracker   //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// Support Routines for PFTW v2.03+                  //
///////////////////////////////////////////////////////

typedef struct tagPFTWCAB
{
	DWORD CFHEADER_Signature;
	DWORD CFHEADER_Reserved1;
	DWORD CFHEADER_CabLength;
	DWORD CFHEADER_Reserved2;
	DWORD CFHEADER_CFFILEOffset;
	DWORD CFHEADER_Reserved3;
	BYTE CFHEADER_CabVersionMinor;
	BYTE CFHEADER_CabVersionMajor;
	WORD CFHEADER_NumberOfCFFOLDERS; // 28 known characters
	WORD CFHEADER_NumberOfCFFILES;
	WORD CFHEADER_Flags;
	WORD CFHEADER_SetId;
	WORD CFHEADER_ICabinet;
	DWORD CFFOLDER_CFDATAOffset;
	WORD CFFOLDER_NumberOfCFDATAS;
	WORD CFFOLDER_CompressionType;
	DWORD CFFILE_UncompressedSize;
	DWORD CFFILE_UncompressedOffset;
	WORD CFFILE_IFolder;
} PFTWCAB;

void headerPreProcessing(unsigned char* fileBuffer, int headerSize);
char* findPassword_v203(unsigned long Decrypted_CFHEADER_CabLength, unsigned char* encryptedData, bool getLongPassword, bool ignorePasswordLength, bool dumpHeaders);