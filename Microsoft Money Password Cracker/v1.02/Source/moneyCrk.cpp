//////////////////////////////////////////////////////
// Microsoft Money Password Cracker                 //
// (C)thewd, thewd@hotmail.com                      //
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
// Recovers passwords up to a maximum length of 14  //
// characters for Microsoft Money account databases //
//////////////////////////////////////////////////////
#include <windows.h>
#include "..\..\..\_Shared Libraries\md5.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "..\..\..\_Shared Libraries\IntelliForceEngine.h"
#include "..\..\..\_Shared Libraries\DictionaryEngine.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

// frame pointer register
#pragma warning(disable:4731)

char* errorMessages[] =
{
	"Unable to open or read the Microsoft Money file",
	"Doesn't appear to be a supported Money file",
	"Unable to find any encrypted password",
	"Unable to recover a suitable password",
	"A search method hasn't been specified (see options)\r\n",
	"Unable to open or read the dictionary wordlist"
};

CUtilitiesLibrary ULib;
bool DEBUG_MODE = false;
unsigned char* recoveredPassword;

#define SEARCH_UNKNOWN			0
#define SEARCH_BRUTEFORCE		1
#define SEARCH_INTELLIFORCE		2
#define SEARCH_DICTIONARY		3
int selectedSearchMethod = SEARCH_UNKNOWN;

unsigned long startLength = 0;
unsigned long endLength = 0;
unsigned long toleranceLevel = 0;
bool useUppercase = false;
bool useNumbers = false;
bool useSpecial = false;
char* dictionaryName = NULL;
char* userName;

MD5_CONTEXT md5context;
CUtilitiesLibrary::CEncryptionLibrary::CRC4Library::RC4_KEY rc4_key;
unsigned char* rc4key = (unsigned char*)&md5context.state;
unsigned char* rc4keyState = (unsigned char*)&rc4_key.state;
bool foundCorrectWord = false;

void displayOptions(void);

////////////////////////////////////
// is the file format Money 2000+ //
////////////////////////////////////
bool isMoney200X(unsigned char* fileBuffer)
{
	if ((fileBuffer[0x00] == 0x00) && (fileBuffer[0x01] == 0x01) &&
		(ULib.StringLibrary.isStringMatch((char*)&fileBuffer[0x04], "MSISAM Database")))
	{
		return true;
	}

	return false;
}

///////////////////////////////////
// is the file format Money 2000 //
///////////////////////////////////
bool isMoney2000(unsigned char* fileBuffer)
{
	return ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x298) & 0x06) ? false : true);
}

///////////////////////////////////
// is the file format Money 2001 //
///////////////////////////////////
bool isMoney2001(unsigned char* fileBuffer)
{
	return ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x298) & 0x06) ? false : true);
}

///////////////////////////////////
// is the file format Money 2002 //
///////////////////////////////////
bool isMoney2002(unsigned char* fileBuffer)
{
	return ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x298) & 0x06) ? true : false);
}

///////////////////////////////////
// is the file format Money 2003 //
///////////////////////////////////
bool isMoney2003(unsigned char* fileBuffer)
{
	return ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x298) & 0x08) ? true : false);
}

///////////////////////////////////
// is the file format Money 2004 //
///////////////////////////////////
bool isMoney2004(unsigned char* fileBuffer)
{
	return ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x298) & 0x08) ? true : false);
}

///////////////////////////////////
// is the file format Money 2005 //
///////////////////////////////////
bool isMoney2005(unsigned char* fileBuffer)
{
	return ((ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x298) & 0x08) ? true : false);
}

//////////////////////////////////////////////////////
// decrypts a money section using the RC4 algorithm //
//////////////////////////////////////////////////////
void decryptMoneySection(unsigned char* fileBuffer, int fileOffset, int fileLength)
{
	unsigned char keyData[4] = {0xC7, 0xDA, 0x39, 0x6B};
	ULib.EncryptionLibrary.RC4Library.encryptBuffer(keyData, 4, fileBuffer, fileOffset, fileLength);
}

///////////////////////////////////////////////////////////
// gets the password from the database (Money 2000/2001) //
///////////////////////////////////////////////////////////
void getMoney2000Password(unsigned char* fileBuffer)
{
	// decrypt file header
	decryptMoneySection(fileBuffer, 0x18, 0x80);

	// retrieve XOR value
	int fileOffset = 0x72;
	_int64 getXOR = ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset + 4);
	getXOR = getXOR << 32;
	getXOR += ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset);

	double tempXOR;
	__asm
	{
			fld getXOR
			fstp tempXOR
	}

	unsigned long xorValue = (unsigned long)tempXOR;
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Password XOR Value\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(xorValue, true, true, ULib.ConsoleLibrary.DISPLAY_AS_WORD);
	}

	int index = 0;
	fileOffset = 0x42;

	// is the database password-protected?
	unsigned long currentValue = (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset) ^ xorValue);

	if (currentValue != 0)
	{
		// decrypt rest of password
		int currentPasswordLength = 0;
		while (index < 40)
		{
			currentValue = (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, fileOffset + index) ^ xorValue);
			ULib.StringLibrary.addDWORDToBuffer(fileBuffer, currentValue, fileOffset + index);
			recoveredPassword[currentPasswordLength++] = fileBuffer[fileOffset + index + 0];
			recoveredPassword[currentPasswordLength++] = fileBuffer[fileOffset + index + 2];
			index += 4;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// generates the password crc for the current word and check its validity //
////////////////////////////////////////////////////////////////////////////
bool __inline checkCurrentWord(unsigned char* currentWord, unsigned char* keySalt,
							   unsigned char* baseCRC, unsigned char* requiredCRC)
{
	__asm
	{
			push eax
			push ebx
			push ecx
			push edx
			push ebp
			push esi
			push edi

			lea edi, [md5context]
			mov esi, [currentWord]
			mov [esi + 40], 0x80
			mov [esi + 56], 0x40
			mov [esi + 57], 0x01

			mov eax, 0x67452301
			mov ebx, 0xEFCDAB89
			mov ecx, 0x98BADCFE
			mov edx, 0x10325476
			mov [edi + 0x00], eax
			mov [edi + 0x04], ebx
			mov [edi + 0x08], ecx
			mov [edi + 0x0C], edx
			push edi

			// R0 section
			mov edi, ecx
			mov ebp, [esi]
			// R0 0
			xor edi, edx
			and edi, ebx
			lea	eax, [3614090360 + ebp + eax]
			xor edi, edx
			add eax, edi
			mov edi, ebx
			rol	eax, 7
			mov ebp, [esi + 4]
			add	eax, ebx
			// R0 1
			xor edi, ecx
			and edi, eax
			lea edx, [3905402710 + ebp + edx]
			xor edi, ecx
			add edx, edi
			mov edi, eax
			rol edx, 12
			mov ebp, [esi + 8]
			add edx, eax
			// R0 2
			xor edi, ebx
			and edi, edx
			lea ecx, [606105819 + ebp + ecx]
			xor edi, ebx
			add ecx, edi
			mov edi, edx
			rol ecx, 17
			mov ebp, [esi + 12]
			add ecx, edx
			// R0 3
			xor edi, eax
			and edi, ecx
			lea ebx, [3250441966 + ebp + ebx]
			xor edi, eax
			add ebx, edi
			mov edi, ecx
			rol ebx, 22
			mov ebp, [esi + 16]
			add ebx, ecx
			// R0 4
			xor edi, edx
			and edi, ebx
			lea eax, [4118548399 + ebp + eax]
			xor edi, edx
			add eax, edi
			mov edi, ebx
			rol eax, 7
			mov ebp, [esi + 20]
			add eax, ebx
			// R0 5
			xor edi, ecx
			and edi, eax
			lea edx, [1200080426 + ebp + edx]
			xor edi, ecx
			add edx, edi
			mov edi, eax
			rol edx, 12
			mov ebp, [esi + 24]
			add edx, eax
			// R0 6
			xor edi, ebx
			and edi, edx
			lea ecx, [2821735955 + ebp + ecx]
			xor edi, ebx
			add ecx, edi
			mov edi, edx
			rol ecx, 17
			mov ebp, [esi + 28]
			add ecx, edx
			// R0 7
			xor edi, eax
			and edi, ecx
			lea ebx, [4249261313 + ebp + ebx]
			xor edi, eax
			add ebx, edi
			mov edi, ecx
			rol ebx, 22
			mov ebp, [esi + 32]
			add ebx, ecx
			// R0 8
			xor edi, edx
			and edi, ebx
			lea eax, [1770035416 + ebp + eax]
			xor edi, edx
			add eax, edi
			mov edi, ebx
			rol eax, 7
			mov ebp, [esi + 36]
			add eax, ebx
			// R0 9
			xor edi, ecx
			and edi, eax
			lea edx, [2336552879 + ebp + edx]
			xor edi, ecx
			add edx, edi
			mov edi, eax
			rol edx, 12
			mov ebp, [esi + 40]
			add edx, eax
			// R0 10
			xor edi, ebx
			and edi, edx
			lea ecx, [4294925233 + ebp + ecx]
			xor edi, ebx
			add ecx, edi
			mov edi, edx
			rol ecx, 17
			mov ebp, [esi + 44]
			add ecx, edx
			// R0 11
			xor edi, eax
			and edi, ecx
			lea ebx, [2304563134 + ebp + ebx]
			xor edi, eax
			add ebx, edi
			mov edi, ecx
			rol ebx, 22
			mov ebp, [esi + 48]
			add ebx, ecx
			// R0 12
			xor edi, edx
			and edi, ebx
			lea eax, [1804603682 + ebp + eax]
			xor edi, edx
			add eax, edi
			mov edi, ebx
			rol eax, 7
			mov ebp, [esi + 52]
			add eax, ebx
			// R0 13
			xor edi, ecx
			and edi, eax
			lea edx, [4254626195 + ebp + edx]
			xor edi, ecx
			add edx, edi
			mov edi, eax
			rol edx, 12
			mov ebp, [esi + 56]
			add edx, eax
			// R0 14
			xor edi, ebx
			and edi, edx
			lea ecx, [2792965006 + ebp + ecx]
			xor edi, ebx
			add ecx, edi
			mov edi, edx
			rol ecx, 17
			mov ebp, [esi + 60]
			add ecx, edx
			// R0 15
			xor edi, eax
			and edi, ecx
			lea ebx, [1236535329 + ebp + ebx]
			xor edi, eax
			add ebx, edi
			mov edi, ecx
			rol ebx, 22
			mov ebp, [esi + 4]
			add ebx, ecx
			// R1 section
			// R1 16
			lea eax, [4129170786 + ebp + eax]
			xor edi, ebx
			and edi, edx
			mov ebp, [esi + 24]
			xor edi, ecx
			add eax, edi
			mov edi, ebx
			rol eax, 5
			add eax, ebx
			// R1 17
			lea edx, [3225465664 + ebp + edx]
			xor edi, eax
			and edi, ecx
			mov ebp, [esi + 44]
			xor edi, ebx
			add edx, edi
			mov edi, eax
			rol edx, 9
			add edx, eax
			// R1 18
			lea ecx, [643717713 + ebp + ecx]
			xor edi, edx
			and edi, ebx
			mov ebp, [esi]
			xor edi, eax
			add ecx, edi
			mov edi, edx
			rol ecx, 14
			add ecx, edx
			// R1 19
			lea ebx, [3921069994 + ebp + ebx]
			xor edi, ecx
			and edi, eax
			mov ebp, [esi + 20]
			xor edi, edx
			add ebx, edi
			mov edi, ecx
			rol ebx, 20
			add ebx, ecx
			// R1 20
			lea eax, [3593408605 + ebp + eax]
			xor edi, ebx
			and edi, edx
			mov ebp, [esi + 40]
			xor edi, ecx
			add eax, edi
			mov edi, ebx
			rol eax, 5
			add eax, ebx
			// R1 21
			lea edx, [38016083 + ebp + edx]
			xor edi, eax
			and edi, ecx
			mov ebp, [esi + 60]
			xor edi, ebx
			add edx, edi
			mov edi, eax
			rol edx, 9
			add edx, eax
			// R1 22
			lea ecx, [3634488961 + ebp + ecx]
			xor edi, edx
			and edi, ebx
			mov ebp, [esi + 16]
			xor edi, eax
			add ecx, edi
			mov edi, edx
			rol ecx, 14
			add ecx, edx
			// R1 23
			lea ebx, [3889429448 + ebp + ebx]
			xor edi, ecx
			and edi, eax
			mov ebp, [esi + 36]
			xor edi, edx
			add ebx, edi
			mov edi, ecx
			rol ebx, 20
			add ebx, ecx
			// R1 24
			lea eax, [568446438 + ebp + eax]
			xor edi, ebx
			and edi, edx
			mov ebp, [esi + 56]
			xor edi, ecx
			add eax, edi
			mov edi, ebx
			rol eax, 5
			add eax, ebx
			// R1 25
			lea edx, [3275163606 + ebp + edx]
			xor edi, eax
			and edi, ecx
			mov ebp, [esi + 12]
			xor edi, ebx
			add edx, edi
			mov edi, eax
			rol edx, 9
			add edx, eax
			// R1 26
			lea ecx, [4107603335 + ebp + ecx]
			xor edi, edx
			and edi, ebx
			mov ebp, [esi + 32]
			xor edi, eax
			add ecx, edi
			mov edi, edx
			rol ecx, 14
			add ecx, edx
			// R1 27
			lea ebx, [1163531501 + ebp + ebx]
			xor edi, ecx
			and edi, eax
			mov ebp, [esi + 52]
			xor edi, edx
			add ebx, edi
			mov edi, ecx
			rol ebx, 20
			add ebx, ecx
			// R1 28
			lea eax, [2850285829 + ebp + eax]
			xor edi, ebx
			and edi, edx
			mov ebp, [esi + 8]
			xor edi, ecx
			add eax, edi
			mov edi, ebx
			rol eax, 5
			add eax, ebx
			// R1 29
			lea edx, [4243563512 + ebp + edx]
			xor edi, eax
			and edi, ecx
			mov ebp, [esi + 28]
			xor edi, ebx
			add edx, edi
			mov edi, eax
			rol edx, 9
			add edx, eax
			// R1 30
			lea ecx, [1735328473 + ebp + ecx]
			xor edi, edx
			and edi, ebx
			mov ebp, [esi + 48]
			xor edi, eax
			add ecx, edi
			mov edi, edx
			rol ecx, 14
			add ecx, edx
			// R1 31
			lea ebx, [2368359562 + ebp + ebx]
			xor edi, ecx
			and edi, eax
			mov ebp, [esi + 20]
			xor edi, edx
			add ebx, edi
			mov edi, ecx
			rol ebx, 20
			add ebx, ecx
			// R2 section
			// R2 32
			xor edi, edx
			xor edi, ebx
			lea eax, [4294588738 + ebp + eax]
			add eax, edi
			rol eax, 4
			mov ebp, [esi + 32]
			mov edi, ebx
			// R2 33
			lea edx, [2272392833 + ebp + edx]
			add eax, ebx
			xor edi, ecx
			xor edi, eax
			mov ebp, [esi + 44]
			add edx, edi
			mov edi, eax
			rol edx, 11
			add edx, eax
			// R2 34
			xor edi, ebx
			xor edi, edx
			lea ecx, [1839030562 + ebp + ecx]
			add ecx, edi
			rol ecx, 16
			mov ebp, [esi + 56]
			mov edi, edx
			// R2 35
			lea ebx, [4259657740 + ebp + ebx]
			add ecx, edx
			xor edi, eax
			xor edi, ecx
			mov ebp, [esi + 4]
			add ebx, edi
			mov edi, ecx
			rol ebx, 23
			add ebx, ecx
			// R2 36
			xor edi, edx
			xor edi, ebx
			lea eax, [2763975236 + ebp + eax]
			add eax, edi
			rol eax, 4
			mov ebp, [esi + 16]
			mov edi, ebx
			// R2 37
			lea edx, [1272893353 + ebp + edx]
			add eax, ebx
			xor edi, ecx
			xor edi, eax
			mov ebp, [esi + 28]
			add edx, edi
			mov edi, eax
			rol edx, 11
			add edx, eax
			// R2 38
			xor edi, ebx
			xor edi, edx
			lea ecx, [4139469664 + ebp + ecx]
			add ecx, edi
			rol ecx, 16
			mov ebp, [esi + 40]
			mov edi, edx
			// R2 39
			lea ebx, [3200236656 + ebp + ebx]
			add ecx, edx
			xor edi, eax
			xor edi, ecx
			mov ebp, [esi + 52]
			add ebx, edi
			mov edi, ecx
			rol ebx, 23
			add ebx, ecx
			// R2 40
			xor edi, edx
			xor edi, ebx
			lea eax, [681279174 + ebp + eax]
			add eax, edi
			rol eax, 4
			mov ebp, [esi]
			mov edi, ebx
			// R2 41
			lea edx, [3936430074 + ebp + edx]
			add eax, ebx
			xor edi, ecx
			xor edi, eax
			mov ebp, [esi + 12]
			add edx, edi
			mov edi, eax
			rol edx, 11
			add edx, eax
			// R2 42
			xor edi, ebx
			xor edi, edx
			lea ecx, [3572445317 + ebp + ecx]
			add ecx, edi
			rol ecx, 16
			mov ebp, [esi + 24]
			mov edi, edx
			// R2 43
			lea ebx, [76029189 + ebp + ebx]
			add ecx, edx
			xor edi, eax
			xor edi, ecx
			mov ebp, [esi + 36]
			add ebx, edi
			mov edi, ecx
			rol ebx, 23
			add ebx, ecx
			// R2 44
			xor edi, edx
			xor edi, ebx
			lea eax, [3654602809 + ebp + eax]
			add eax, edi
			rol eax, 4
			mov ebp, [esi + 48]
			mov edi, ebx
			// R2 45
			lea edx, [3873151461 + ebp + edx]
			add eax, ebx
			xor edi, ecx
			xor edi, eax
			mov ebp, [esi + 60]
			add edx, edi
			mov edi, eax
			rol edx, 11
			add edx, eax
			// R2 46
			xor edi, ebx
			xor edi, edx
			lea ecx, [530742520 + ebp + ecx]
			add ecx, edi
			rol ecx, 16
			mov ebp, [esi + 8]
			mov edi, edx
			// R2 47
			lea ebx, [3299628645 + ebp + ebx]
			add ecx, edx
			xor edi, eax
			xor edi, ecx
			mov ebp, [esi]
			add ebx, edi
			mov edi, -1
			rol ebx, 23
			add ebx, ecx
			// R3 section
			// R3 48
			xor edi, edx
			or edi, ebx
			lea eax, [4096336452 + ebp + eax]
			xor edi, ecx
			mov ebp, [esi + 28]
			add eax, edi
			mov edi, -1
			rol eax, 6
			xor edi, ecx
			add eax, ebx
			// R3 49
			or edi, eax
			lea edx, [1126891415 + ebp + edx]
			xor edi, ebx
			mov ebp, [esi + 56]
			add edx, edi
			mov edi, -1
			rol edx, 10
			xor edi, ebx
			add edx, eax
			// R3 50
			or edi, edx
			lea ecx, [2878612391 + ebp + ecx]
			xor edi, eax
			mov ebp, [esi + 20]
			add ecx, edi
			mov edi, -1
			rol ecx, 15
			xor edi, eax
			add ecx, edx
			// R3 51
			or edi, ecx
			lea ebx, [4237533241 + ebp + ebx]
			xor edi, edx
			mov ebp, [esi + 48]
			add ebx, edi
			mov edi, -1
			rol ebx, 21
			xor edi, edx
			add ebx, ecx
			// R3 52
			or edi, ebx
			lea eax, [1700485571 + ebp + eax]
			xor edi, ecx
			mov ebp, [esi + 12]
			add eax, edi
			mov edi, -1
			rol eax, 6
			xor edi, ecx
			add eax, ebx
			// R3 53
			or edi, eax
			lea edx, [2399980690 + ebp + edx]
			xor edi, ebx
			mov ebp, [esi + 40]
			add edx, edi
			mov edi, -1
			rol edx, 10
			xor edi, ebx
			add edx, eax
			// R3 54
			or edi, edx
			lea ecx, [4293915773 + ebp + ecx]
			xor edi, eax
			mov ebp, [esi + 4]
			add ecx, edi
			mov edi, -1
			rol ecx, 15
			xor edi, eax
			add ecx, edx
			// R3 55
			or edi, ecx
			lea ebx, [2240044497 + ebp + ebx]
			xor edi, edx
			mov ebp, [esi + 32]
			add ebx, edi
			mov edi, -1
			rol ebx, 21
			xor edi, edx
			add ebx, ecx
			// R3 56
			or edi, ebx
			lea eax, [1873313359 + ebp + eax]
			xor edi, ecx
			mov ebp, [esi + 60]
			add eax, edi
			mov edi, -1
			rol eax, 6
			xor edi, ecx
			add eax, ebx
			// R3 57
			or edi, eax
			lea edx, [4264355552 + ebp + edx]
			xor edi, ebx
			mov ebp, [esi + 24]
			add edx, edi
			mov edi, -1
			rol edx, 10
			xor edi, ebx
			add edx, eax
			// R3 58
			or edi, edx
			lea ecx, [2734768916 + ebp + ecx]
			xor edi, eax
			mov ebp, [esi + 52]
			add ecx, edi
			mov edi, -1
			rol ecx, 15
			xor edi, eax
			add ecx, edx
			// R3 59
			or edi, ecx
			lea ebx, [1309151649 + ebp + ebx]
			xor edi, edx
			mov ebp, [esi + 16]
			add ebx, edi
			mov edi, -1
			rol ebx, 21
			xor edi, edx
			add ebx, ecx
			// R3 60
			or edi, ebx
			lea eax, [4149444226 + ebp + eax]
			xor edi, ecx
			mov ebp, [esi + 44]
			add eax, edi
			mov edi, -1
			rol eax, 6
			xor edi, ecx
			add eax, ebx
			// R3 61
			or edi, eax
			lea edx, [3174756917 + ebp + edx]
			xor edi, ebx
			mov ebp, [esi + 8]
			add edx, edi
			mov edi, -1
			rol edx, 10
			xor edi, ebx
			add edx, eax
			// R3 62
			or edi, edx
			lea ecx, [718787259 + ebp + ecx]
			xor edi, eax
			mov ebp, [esi + 36]
			add ecx, edi
			mov edi, -1
			rol ecx, 15
			xor edi, eax
			add ecx, edx
			// R3 63
			or edi, ecx
			lea ebx, [3951481745 + ebp + ebx]
			xor edi, edx
			add ebx, edi
			add esi, 64
			rol ebx, 21
			add ebx, ecx

			pop edi
			add eax, [edi + 0x00]
			add ebx, [edi + 0x04]
			add ecx, [edi + 0x08]
			add edx, [edi + 0x0C]
			mov [edi + 0x00], eax
			mov [edi + 0x04], ebx
			mov [edi + 0x08], ecx
			mov [edi + 0x0C], edx

			pop edi
			pop esi
			pop ebp
			pop edx
			pop ecx
			pop ebx
			pop eax

			////////////////////
			// RC4 ENCRYPTION //
			////////////////////

			push eax
			push ecx
			push edx
			push esi
			push edi

			// setup RC4 table
			mov ecx, [rc4keyState]

			mov dword ptr [ecx + 0x00], 0x03020100
			mov dword ptr [ecx + 0x04], 0x07060504
			mov dword ptr [ecx + 0x08], 0x0B0A0908
			mov dword ptr [ecx + 0x0C], 0x0F0E0D0C

			mov dword ptr [ecx + 0x10], 0x13121110
			mov dword ptr [ecx + 0x14], 0x17161514
			mov dword ptr [ecx + 0x18], 0x1B1A1918
			mov dword ptr [ecx + 0x1C], 0x1F1E1D1C

			mov dword ptr [ecx + 0x20], 0x23222120
			mov dword ptr [ecx + 0x24], 0x27262524
			mov dword ptr [ecx + 0x28], 0x2B2A2928
			mov dword ptr [ecx + 0x2C], 0x2F2E2D2C

			mov dword ptr [ecx + 0x30], 0x33323130
			mov dword ptr [ecx + 0x34], 0x37363534
			mov dword ptr [ecx + 0x38], 0x3B3A3938
			mov dword ptr [ecx + 0x3C], 0x3F3E3D3C

			mov dword ptr [ecx + 0x40], 0x43424140
			mov dword ptr [ecx + 0x44], 0x47464544
			mov dword ptr [ecx + 0x48], 0x4B4A4948
			mov dword ptr [ecx + 0x4C], 0x4F4E4D4C

			mov dword ptr [ecx + 0x50], 0x53525150
			mov dword ptr [ecx + 0x54], 0x57565554
			mov dword ptr [ecx + 0x58], 0x5B5A5958
			mov dword ptr [ecx + 0x5C], 0x5F5E5D5C

			mov dword ptr [ecx + 0x60], 0x63626160
			mov dword ptr [ecx + 0x64], 0x67666564
			mov dword ptr [ecx + 0x68], 0x6B6A6968
			mov dword ptr [ecx + 0x6C], 0x6F6E6D6C

			mov dword ptr [ecx + 0x70], 0x73727170
			mov dword ptr [ecx + 0x74], 0x77767574
			mov dword ptr [ecx + 0x78], 0x7B7A7978
			mov dword ptr [ecx + 0x7C], 0x7F7E7D7C

			mov dword ptr [ecx + 0x80], 0x83828180
			mov dword ptr [ecx + 0x84], 0x87868584
			mov dword ptr [ecx + 0x88], 0x8B8A8988
			mov dword ptr [ecx + 0x8C], 0x8F8E8D8C

			mov dword ptr [ecx + 0x90], 0x93929190
			mov dword ptr [ecx + 0x94], 0x97969594
			mov dword ptr [ecx + 0x98], 0x9B9A9998
			mov dword ptr [ecx + 0x9C], 0x9F9E9D9C

			mov dword ptr [ecx + 0xA0], 0xA3A2A1A0
			mov dword ptr [ecx + 0xA4], 0xA7A6A5A4
			mov dword ptr [ecx + 0xA8], 0xABAAA9A8
			mov dword ptr [ecx + 0xAC], 0xAFAEADAC

			mov dword ptr [ecx + 0xB0], 0xB3B2B1B0
			mov dword ptr [ecx + 0xB4], 0xB7B6B5B4
			mov dword ptr [ecx + 0xB8], 0xBBBAB9B8
			mov dword ptr [ecx + 0xBC], 0xBFBEBDBC

			mov dword ptr [ecx + 0xC0], 0xC3C2C1C0
			mov dword ptr [ecx + 0xC4], 0xC7C6C5C4
			mov dword ptr [ecx + 0xC8], 0xCBCAC9C8
			mov dword ptr [ecx + 0xCC], 0xCFCECDCC

			mov dword ptr [ecx + 0xD0], 0xD3D2D1D0
			mov dword ptr [ecx + 0xD4], 0xD7D6D5D4
			mov dword ptr [ecx + 0xD8], 0xDBDAD9D8
			mov dword ptr [ecx + 0xDC], 0xDFDEDDDC

			mov dword ptr [ecx + 0xE0], 0xE3E2E1E0
			mov dword ptr [ecx + 0xE4], 0xE7E6E5E4
			mov dword ptr [ecx + 0xE8], 0xEBEAE9E8
			mov dword ptr [ecx + 0xEC], 0xEFEEEDEC

			mov dword ptr [ecx + 0xF0], 0xF3F2F1F0
			mov dword ptr [ecx + 0xF4], 0xF7F6F5F4
			mov dword ptr [ecx + 0xF8], 0xFBFAF9F8
			mov dword ptr [ecx + 0xFC], 0xFFFEFDFC

			/////////////////////////////
			// reorder RC4 table based //
			// on the encryption key   //
			/////////////////////////////

			xor eax, eax
			xor edx, edx
			mov esi, [keySalt]
			mov edi, [rc4key]

			// cycle 0
			add dl, [edi + 0x00]
			mov al, [ecx + 0x00]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x00], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x01]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x01], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x02]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x02], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x03]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x03], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x04]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x04], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x05]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x05], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x06]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x06], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x07]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x07], ah

			// cycle 01
			add dl, [edi + 0x08]
			mov al, [ecx + 0x08]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x08], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x09]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x09], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x0A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x0A], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x0B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x0B], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x0C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x0C], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x0D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x0D], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x0E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x0E], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x0F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x0F], ah

			// cycle 02
			add dl, [esi + 0x00]
			mov al, [ecx + 0x10]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x10], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0x11]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x11], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0x12]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x12], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0x13]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x13], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0x14]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x14], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0x15]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x15], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0x16]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x16], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0x17]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x17], ah

			// cycle 03
			add dl, [edi + 0x00]
			mov al, [ecx + 0x18]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x18], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x19]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x19], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x1A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x1A], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x1B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x1B], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x1C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x1C], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x1D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x1D], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x1E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x1E], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x1F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x1F], ah

			// cycle 04
			add dl, [edi + 0x08]
			mov al, [ecx + 0x20]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x20], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x21]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x21], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x22]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x22], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x23]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x23], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x24]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x24], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x25]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x25], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x26]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x26], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x27]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x20], ah

			// cycle 05
			add dl, [esi + 0x00]
			mov al, [ecx + 0x28]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x28], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0x29]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x29], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0x2A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x2A], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0x2B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x2B], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0x2C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x2C], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0x2D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x2D], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0x2E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x2E], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0x2F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x2F], ah

			// cycle 06
			add dl, [edi + 0x00]
			mov al, [ecx + 0x30]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x30], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x31]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x31], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x32]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x32], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x33]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x33], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x34]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x34], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x35]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x35], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x36]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x36], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x37]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x37], ah

			// cycle 07
			add dl, [edi + 0x08]
			mov al, [ecx + 0x38]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x38], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x39]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x39], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x3A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x3A], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x3B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x3B], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x3C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x3C], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x3D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x3D], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x3E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x3E], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x3F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x3F], ah

			// cycle 08
			add dl, [esi + 0x00]
			mov al, [ecx + 0x40]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x40], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0x41]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x41], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0x42]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x42], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0x43]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x43], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0x44]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x44], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0x45]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x45], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0x46]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x46], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0x47]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x47], ah

			// cycle 09
			add dl, [edi + 0x00]
			mov al, [ecx + 0x48]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x48], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x49]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x49], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x4A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x4A], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x4B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x4B], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x4C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x4C], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x4D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x4D], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x4E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x4E], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x4F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x4F], ah

			// cycle 0A
			add dl, [edi + 0x08]
			mov al, [ecx + 0x50]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x50], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x51]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x51], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x52]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x52], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x53]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x53], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x54]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x54], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x55]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x55], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x56]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x56], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x57]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x57], ah

			// cycle 0B
			add dl, [esi + 0x00]
			mov al, [ecx + 0x58]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x58], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0x59]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x59], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0x5A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x5A], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0x5B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x5B], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0x5C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x5C], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0x5D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x5D], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0x5E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x5E], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0x5F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x5F], ah

			// cycle 0C
			add dl, [edi + 0x00]
			mov al, [ecx + 0x60]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x60], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x61]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x61], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x62]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x62], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x63]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x63], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x64]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x64], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x65]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x65], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x66]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x66], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x67]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x67], ah

			// cycle 0D
			add dl, [edi + 0x08]
			mov al, [ecx + 0x68]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x68], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x69]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x69], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x6A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x6A], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x6B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x6B], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x6C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x6C], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x6D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x6D], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x6E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x6E], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x6F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x6F], ah

			// cycle 0E
			add dl, [esi + 0x00]
			mov al, [ecx + 0x70]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x70], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0x71]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x71], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0x72]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x72], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0x73]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x73], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0x74]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x74], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0x75]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x75], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0x76]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x76], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0x77]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x77], ah

			// cycle 0F
			add dl, [edi + 0x00]
			mov al, [ecx + 0x78]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x78], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x79]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x79], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x7A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x7A], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x7B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x7B], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x7C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x7C], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x7D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x7D], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x7E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x7E], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x7F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x7F], ah

			// cycle 10
			add dl, [edi + 0x08]
			mov al, [ecx + 0x80]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x80], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x81]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x81], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x82]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x82], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x83]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x83], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x84]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x84], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x85]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x85], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x86]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x86], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x87]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x87], ah

			// cycle 11
			add dl, [esi + 0x00]
			mov al, [ecx + 0x88]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x88], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0x89]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x89], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0x8A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x8A], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0x8B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x8B], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0x8C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x8C], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0x8D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x8D], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0x8E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x8E], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0x8F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x8F], ah

			// cycle 12
			add dl, [edi + 0x00]
			mov al, [ecx + 0x90]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x90], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0x91]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x91], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0x92]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x92], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0x93]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x93], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0x94]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x94], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0x95]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x95], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0x96]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x96], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0x97]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x97], ah

			// cycle 13
			add dl, [edi + 0x08]
			mov al, [ecx + 0x98]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x98], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0x99]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x99], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0x9A]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x9A], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0x9B]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x9B], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0x9C]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x9C], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0x9D]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x9D], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0x9E]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x9E], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0x9F]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0x9F], ah

			// cycle 14
			add dl, [esi + 0x00]
			mov al, [ecx + 0xA0]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA0], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0xA1]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA1], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0xA2]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA2], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0xA3]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA3], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0xA4]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA4], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0xA5]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA5], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0xA6]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA6], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0xA7]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA7], ah

			// cycle 15
			add dl, [edi + 0x00]
			mov al, [ecx + 0xA8]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA8], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0xA9]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xA9], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0xAA]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xAA], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0xAB]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xAB], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0xAC]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xAC], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0xAD]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xAD], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0xAE]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xAE], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0xAF]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xAF], ah

			// cycle 16
			add dl, [edi + 0x08]
			mov al, [ecx + 0xB0]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB0], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0xB1]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB1], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0xB2]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB2], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0xB3]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB3], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0xB4]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB4], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0xB5]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB5], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0xB6]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB6], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0xB7]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB7], ah

			// cycle 17
			add dl, [esi + 0x00]
			mov al, [ecx + 0xB8]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB8], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0xB9]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xB9], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0xBA]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xBA], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0xBB]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xBB], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0xBC]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xBC], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0xBD]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xBD], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0xBE]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xBE], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0xBF]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xBF], ah

			// cycle 18
			add dl, [edi + 0x00]
			mov al, [ecx + 0xC0]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC0], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0xC1]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC1], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0xC2]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC2], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0xC3]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC3], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0xC4]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC4], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0xC5]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC5], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0xC6]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC6], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0xC7]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC7], ah

			// cycle 19
			add dl, [edi + 0x08]
			mov al, [ecx + 0xC8]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC8], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0xC9]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xC9], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0xCA]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xCA], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0xCB]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xCB], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0xCC]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xCC], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0xCD]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xCD], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0xCE]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xCE], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0xCF]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xCF], ah

			// cycle 1A
			add dl, [esi + 0x00]
			mov al, [ecx + 0xD0]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD0], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0xD1]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD1], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0xD2]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD2], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0xD3]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD3], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0xD4]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD4], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0xD5]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD5], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0xD6]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD6], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0xD7]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD7], ah

			// cycle 1B
			add dl, [edi + 0x00]
			mov al, [ecx + 0xD8]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD8], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0xD9]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xD9], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0xDA]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xDA], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0xDB]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xDB], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0xDC]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xDC], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0xDD]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xDD], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0xDE]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xDE], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0xDF]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xDF], ah

			// cycle 1C
			add dl, [edi + 0x08]
			mov al, [ecx + 0xE0]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE0], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0xE1]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE1], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0xE2]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE2], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0xE3]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE3], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0xE4]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE4], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0xE5]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE5], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0xE6]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE6], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0xE7]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE7], ah

			// cycle 1D
			add dl, [esi + 0x00]
			mov al, [ecx + 0xE8]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE8], ah

			add dl, [esi + 0x01]
			mov al, [ecx + 0xE9]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xE9], ah

			add dl, [esi + 0x02]
			mov al, [ecx + 0xEA]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xEA], ah

			add dl, [esi + 0x03]
			mov al, [ecx + 0xEB]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xEB], ah

			add dl, [esi + 0x04]
			mov al, [ecx + 0xEC]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xEC], ah

			add dl, [esi + 0x05]
			mov al, [ecx + 0xED]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xED], ah

			add dl, [esi + 0x06]
			mov al, [ecx + 0xEE]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xEE], ah

			add dl, [esi + 0x07]
			mov al, [ecx + 0xEF]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xEF], ah

			// cycle 1E
			add dl, [edi + 0x00]
			mov al, [ecx + 0xF0]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF0], ah

			add dl, [edi + 0x01]
			mov al, [ecx + 0xF1]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF1], ah

			add dl, [edi + 0x02]
			mov al, [ecx + 0xF2]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF2], ah

			add dl, [edi + 0x03]
			mov al, [ecx + 0xF3]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF3], ah

			add dl, [edi + 0x04]
			mov al, [ecx + 0xF4]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF4], ah

			add dl, [edi + 0x05]
			mov al, [ecx + 0xF5]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF5], ah

			add dl, [edi + 0x06]
			mov al, [ecx + 0xF6]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF6], ah

			add dl, [edi + 0x07]
			mov al, [ecx + 0xF7]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF7], ah

			// cycle 1F
			add dl, [edi + 0x08]
			mov al, [ecx + 0xF8]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF8], ah

			add dl, [edi + 0x09]
			mov al, [ecx + 0xF9]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xF9], ah

			add dl, [edi + 0x0A]
			mov al, [ecx + 0xFA]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xFA], ah

			add dl, [edi + 0x0B]
			mov al, [ecx + 0xFB]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xFB], ah

			add dl, [edi + 0x0C]
			mov al, [ecx + 0xFC]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xFC], ah

			add dl, [edi + 0x0D]
			mov al, [ecx + 0xFD]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xFD], ah

			add dl, [edi + 0x0E]
			mov al, [ecx + 0xFE]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xFE], ah

			add dl, [edi + 0x0F]
			mov al, [ecx + 0xFF]
			add dl, al
			mov ah, [ecx + edx]
			mov [ecx + edx], al
			mov [ecx + 0xFF], ah

			/////////////////////////////////
			// do RC4 encryption on buffer //
			/////////////////////////////////

			mov edi, [rc4keyState]
			mov esi, [requiredCRC]
			mov edx, [baseCRC]
			xor eax, eax
			xor ecx, ecx

			// byte 1
			mov cl, [edi + 0x01]
			add al, cl
			push eax
			mov ch, [edi + eax]
			mov [edi + eax], cl
			mov [edi + 0x01], ch
			mov al, cl
			add al, ch
			mov al, [edi + eax]
			xor al, [edx + 0x00]
			cmp al, [esi + 0x00]
			pop eax
			jne endCheck

			// byte 2
			mov cl, [edi + 0x02]
			add al, cl
			push eax
			mov ch, [edi + eax]
			mov [edi + eax], cl
			mov [edi + 0x02], ch
			mov al, cl
			add al, ch
			mov al, [edi + eax]
			xor al, [edx + 0x01]
			cmp al, [esi + 0x01]
			pop eax
			jne endCheck

			// byte 3
			mov cl, [edi + 0x03]
			add al, cl
			push eax
			mov ch, [edi + eax]
			mov [edi + eax], cl
			mov [edi + 0x03], ch
			mov al, cl
			add al, ch
			mov al, [edi + eax]
			xor al, [edx + 0x02]
			cmp al, [esi + 0x02]
			pop eax
			jne endCheck

			// byte 4
			mov cl, [edi + 0x04]
			add al, cl
			push eax
			mov ch, [edi + eax]
			mov [edi + eax], cl
			mov [edi + 0x04], ch
			mov al, cl
			add al, ch
			mov al, [edi + eax]
			xor al, [edx + 0x03]
			cmp al, [esi + 0x03]
			pop eax
			jne endCheck

			// found correct word
			mov [foundCorrectWord], 0x01

		endCheck:
			pop edi
			pop esi
			pop edx
			pop ecx
			pop eax
	}

	return foundCorrectWord;
}

///////////////////////////////////////////////////////////
// gets the password from the database (Money 2002-2005) //
///////////////////////////////////////////////////////////
void getMoney2002Password(unsigned char* fileBuffer)
{
	// decrypt file header
	decryptMoneySection(fileBuffer, 0x18, 0x80);

	int offsetAdjustment = ((isMoney2003(fileBuffer)) ? fileBuffer[0x72] : 0);
	int offsetUsername = 0x2F1 + offsetAdjustment;
	int offsetGUID = 0x373 + offsetAdjustment;
	int offsetRC4EncryptionBuffer = 0x2E9 + offsetAdjustment;

	// decrypt user name
	if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, offsetUsername) != 0) decryptMoneySection(fileBuffer, offsetUsername, 0x82);
	// decrypt guid
	if (ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, offsetGUID) != 0) decryptMoneySection(fileBuffer, offsetGUID, 0x4E);

	if (DEBUG_MODE)
	{
		char* guid = ULib.StringLibrary.convertUnicodeToAnsi((PWSTR)&fileBuffer[offsetGUID], 0x4E);
		ULib.ConsoleLibrary.displayConsoleMessage("Password CRC\t\t: 0x", false);
		ULib.ConsoleLibrary.displayConsoleMessage(ULib.StringLibrary.getDWORDFromBuffer(fileBuffer, 0x72), true, true, ULib.ConsoleLibrary.DISPLAY_AS_DWORD);
		ULib.ConsoleLibrary.displayConsoleMessage("User Name\t\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage((char*)&fileBuffer[offsetUsername]);
		ULib.ConsoleLibrary.displayConsoleMessage("GUID\t\t\t: ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(guid);
		ULib.ConsoleLibrary.displayConsoleMessage("RC4 Encryption Salt\t: ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&fileBuffer[0x72], 8, 1, true);
		ULib.ConsoleLibrary.displayConsoleMessage("RC4 Encryption Buffer\t: ", false);
		ULib.ConsoleLibrary.displayHexadecimalString((unsigned char*)&fileBuffer[offsetRC4EncryptionBuffer], 4, 1, true);
		ULib.ConsoleLibrary.displayConsoleMessage("");
		ULib.HeapLibrary.freePrivateHeap(guid);
	}

	unsigned char* passwordData = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(100);
	// test for a blank password
	if (checkCurrentWord(passwordData, (unsigned char*)&fileBuffer[0x72],
		(unsigned char*)&fileBuffer[offsetRC4EncryptionBuffer], (unsigned char*)&fileBuffer[0x72]))
	{
		return;
	}

	if (selectedSearchMethod != SEARCH_UNKNOWN)
	{
		if (endLength < startLength) endLength = startLength;

		ULib.ConsoleLibrary.displayConsoleMessage("Search Engine\t\t: ", false);
		if (selectedSearchMethod == SEARCH_BRUTEFORCE) ULib.ConsoleLibrary.displayConsoleMessage("BruteForce Engine");
		else if (selectedSearchMethod == SEARCH_INTELLIFORCE) ULib.ConsoleLibrary.displayConsoleMessage("IntelliForce Engine");
		else if (selectedSearchMethod == SEARCH_DICTIONARY) ULib.ConsoleLibrary.displayConsoleMessage("Dictionary Engine");
		else ULib.ConsoleLibrary.displayConsoleMessage("");
		if (selectedSearchMethod == SEARCH_DICTIONARY)
		{
			char* dictionaryNameStripped = ULib.FileLibrary.stripPathFromFilename(dictionaryName, true);
			ULib.ConsoleLibrary.displayConsoleMessage("Dictionary Filename\t: ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(dictionaryNameStripped);
			ULib.ConsoleLibrary.displayConsoleMessage("");
			ULib.HeapLibrary.freePrivateHeap(dictionaryNameStripped);
		}
		else
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Search Length\t\t: ", false);
			if (startLength == endLength)
			{
				ULib.ConsoleLibrary.displayConsoleMessage(startLength, false, false);
				ULib.ConsoleLibrary.displayConsoleMessage(" characters");
			}
			else
			{
				ULib.ConsoleLibrary.displayConsoleMessage(startLength, false, false);
				ULib.ConsoleLibrary.displayConsoleMessage(" to ", false);
				ULib.ConsoleLibrary.displayConsoleMessage(endLength, false, false);
				ULib.ConsoleLibrary.displayConsoleMessage(" character(s)");
			}
			ULib.ConsoleLibrary.displayConsoleMessage("Character Set\t\t: ", false);
			if (useUppercase) ULib.ConsoleLibrary.displayConsoleMessage("Uppercase ", false);
			if (useNumbers) ULib.ConsoleLibrary.displayConsoleMessage("Numbers ", false);
			if (useSpecial) ULib.ConsoleLibrary.displayConsoleMessage("Special ", false);
			if (selectedSearchMethod == SEARCH_INTELLIFORCE)
			{
				ULib.ConsoleLibrary.displayConsoleMessage("");
				ULib.ConsoleLibrary.displayConsoleMessage("Tolerance Level\t\t: ", false);
				ULib.ConsoleLibrary.displayConsoleMessage(toleranceLevel, false, false);
			}
			ULib.ConsoleLibrary.displayConsoleMessage("\r\n");
		}
	}

	char* currentWord = NULL;
	bool foundPassword = false;

	// do bruteforce search
	if (selectedSearchMethod == SEARCH_BRUTEFORCE)
	{
		BruteForceEngine BFengine;
		BFengine.initialiseCharacterSet(false, useUppercase, useNumbers, useSpecial);
		ULib.ConsoleLibrary.hideConsoleCursor();

		for (unsigned long currentWordLength = startLength; currentWordLength <= endLength; currentWordLength++)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (Length - ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(currentWordLength, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(")...", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 39, false);
			BFengine.setCurrentWordLength(currentWordLength);

			do
			{
				currentWord = BFengine.getNextWord();
				if (checkCurrentWord((unsigned char*)currentWord, (unsigned char*)&fileBuffer[0x72],
					(unsigned char*)&fileBuffer[offsetRC4EncryptionBuffer], (unsigned char*)&fileBuffer[0x72]))
				{
					ULib.StringLibrary.copyString(userName, (char*)&fileBuffer[offsetUsername]);
					char* ansiPassword = ULib.StringLibrary.convertUnicodeToAnsi((PWSTR)currentWord, currentWordLength);
					ULib.StringLibrary.copyString((char*)recoveredPassword, ansiPassword);
					ULib.HeapLibrary.freePrivateHeap(ansiPassword);
					foundPassword = true;
					break;
				}

			} while (currentWord[0] != NULL);

			if (foundPassword) break;
		}

		BFengine.destroyCharacterSet();
	}
	// do intelliforce search
	else if (selectedSearchMethod == SEARCH_INTELLIFORCE)
	{
		IntelliForceEngine IFengine;
		IFengine.initialiseCharacterSet(false, useUppercase);
		IFengine.setToleranceLevel(toleranceLevel);
		ULib.ConsoleLibrary.hideConsoleCursor();

		for (unsigned long currentWordLength = startLength; currentWordLength <= endLength; currentWordLength++)
		{
			ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (Length - ", false);
			ULib.ConsoleLibrary.displayConsoleMessage(currentWordLength, false, false, ULib.ConsoleLibrary.DISPLAY_AS_BYTE);
			ULib.ConsoleLibrary.displayConsoleMessage(")...", false);
			ULib.ConsoleLibrary.displayConsoleMessage('\b', 39, false);
			IFengine.setCurrentWordLength(currentWordLength);

			do
			{
				currentWord = IFengine.getNextWord();
				if (checkCurrentWord((unsigned char*)currentWord, (unsigned char*)&fileBuffer[0x72],
					(unsigned char*)&fileBuffer[offsetRC4EncryptionBuffer], (unsigned char*)&fileBuffer[0x72]))
				{
					ULib.StringLibrary.copyString(userName, (char*)&fileBuffer[offsetUsername]);
					char* ansiPassword = ULib.StringLibrary.convertUnicodeToAnsi((PWSTR)currentWord, currentWordLength);
					ULib.StringLibrary.copyString((char*)recoveredPassword, ansiPassword);
					ULib.HeapLibrary.freePrivateHeap(ansiPassword);
					foundPassword = true;
					break;
				}

			} while (currentWord[0] != NULL);

			if (foundPassword) break;
		}

		IFengine.destroyCharacterSet();
	}
	else if (selectedSearchMethod == SEARCH_DICTIONARY)
	{
		DictionaryEngine DictEngine;
		// try to open dictionary
		if (!DictEngine.initialiseDictionaryEngine(dictionaryName, DICTIONARYENGINE_FILTER_UPPERCASE)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);
		ULib.ConsoleLibrary.hideConsoleCursor();

		ULib.ConsoleLibrary.displayConsoleMessage("Searching for Password (Dictionary)...", false);
		ULib.ConsoleLibrary.displayConsoleMessage('\b', 38, false);

		unsigned long currentWordLength = 0;

		do
		{
			// empty previous word
			ZeroMemory(passwordData, currentWordLength * 2);
			currentWord = DictEngine.getNextWord(&currentWordLength);
			// convert dictionary word to unicode
			for (unsigned long i = 0; i < currentWordLength; i++) passwordData[i * 2] = (((currentWord[i] >= 0x61) && (currentWord[i] <= 0x7A)) ? (unsigned char)(currentWord[i] - 0x20) : currentWord[i]);
			if (checkCurrentWord(passwordData, (unsigned char*)&fileBuffer[0x72],
				(unsigned char*)&fileBuffer[offsetRC4EncryptionBuffer], (unsigned char*)&fileBuffer[0x72]))
			{
				ULib.StringLibrary.copyString(userName, (char*)&fileBuffer[offsetUsername]);
				ULib.StringLibrary.copyString((char*)recoveredPassword, currentWord);
				foundPassword = true;
				break;
			}

		} while (currentWord[0] != NULL);

		DictEngine.destroyDictionaryEngine();
	}
	// no search method specified
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4, false);
		displayOptions();
		ULib.ProcessLibrary.exitProcessHandler();
	}

	ULib.ConsoleLibrary.showConsoleCursor();
	ULib.HeapLibrary.freePrivateHeap(passwordData);

	// no password found
	if (!foundPassword) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
}

//////////////////////////////////////////////////
// Load this file, determine the format, and    //
// decrypt the password at the specified offset //
//////////////////////////////////////////////////
void parseFileAndDecrypt(char* fileName)
{
	unsigned char* fileBuffer = ULib.FileLibrary.readBufferFromFile(fileName, 0x500);
	if (fileBuffer == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);

	bool bIsMoney2000 = isMoney2000(fileBuffer);
	bool bIsMoney2001 = isMoney2001(fileBuffer);
	bool bIsMoney2002 = isMoney2002(fileBuffer);
	bool bIsMoney2003 = isMoney2003(fileBuffer);
	bool bIsMoney2004 = isMoney2004(fileBuffer);
	bool bIsMoney2005 = isMoney2005(fileBuffer);

	// display file format
	if (DEBUG_MODE)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Recognised File Format\t: ", false);
		if ((bIsMoney2002) || (bIsMoney2003) || (bIsMoney2004) || (bIsMoney2005)) ULib.ConsoleLibrary.displayConsoleMessage("Money 2002-2005");
		else if ((bIsMoney2001) || (bIsMoney2000)) ULib.ConsoleLibrary.displayConsoleMessage("Money 2000/2001");
		else ULib.ConsoleLibrary.displayConsoleMessage("");
	}

	if ((bIsMoney2000) || (bIsMoney2001)) getMoney2000Password(fileBuffer);
	else if ((bIsMoney2002) || (bIsMoney2003) || (bIsMoney2004) || (bIsMoney2005)) getMoney2002Password(fileBuffer);
	else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);

	ULib.HeapLibrary.freePrivateHeap(fileBuffer);
}

////////////////////////////////////////////////////////
// display program options is no arguments are passed //
////////////////////////////////////////////////////////
void displayOptions(void)
{
	ULib.ConsoleLibrary.displayConsoleMessage("Usage:\tmoneyCrk <microsoft money database>");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t         [-bruteforce <start length> <end length> <charset - A1!>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t         [-intelliforce <start length> <end length> <tolerance level>]");
	ULib.ConsoleLibrary.displayConsoleMessage("      \t         [-dictionary <dictionary filename>]");
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Microsoft Money Password Cracker", "1.02", 1999);

	// check whether the shift key is depressed (enable debug mode)
	DEBUG_MODE = ULib.ProcessLibrary.isDebugModeEnabled();

	// if arguments are passed
	if (argc > 1)
	{
		if (argc > 2)
		{
			if ((ULib.StringLibrary.isStringMatch("-bruteforce", argv[2])) && (argc > 5))
			{
				selectedSearchMethod = SEARCH_BRUTEFORCE;
				startLength = ULib.StringLibrary.getDWORDFromString(argv[3]);
				endLength = ULib.StringLibrary.getDWORDFromString(argv[4]);
				for (int index = 0; index < ULib.StringLibrary.getStringLength(argv[5]); index++)
				{
					if (((argv[5])[index]) == 'A') useUppercase = true;
					if (((argv[5])[index]) == '1') useNumbers = true;
					if (((argv[5])[index]) == '!') useSpecial = true;
				}
			}
			else if ((ULib.StringLibrary.isStringMatch("-intelliforce", argv[2])) && (argc > 5))
			{
				selectedSearchMethod = SEARCH_INTELLIFORCE;
				startLength = ULib.StringLibrary.getDWORDFromString(argv[3]);
				endLength = ULib.StringLibrary.getDWORDFromString(argv[4]);
				toleranceLevel = ULib.StringLibrary.getDWORDFromString(argv[5]);
				useUppercase = true;
			}
			else if ((ULib.StringLibrary.isStringMatch("-dictionary", argv[2])) && (argc > 3))
			{
				selectedSearchMethod = SEARCH_DICTIONARY;
				dictionaryName = argv[3];
			}
		}

		userName = (char*)ULib.HeapLibrary.allocPrivateHeap(50);
		recoveredPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap(50);
		parseFileAndDecrypt(argv[1]);

		if (recoveredPassword[0] != NULL)
		{
			// display user name (2002+)
			if (userName[0] != NULL)
			{
				ULib.ConsoleLibrary.displayConsoleMessage("User Name\t\t: ", false);
				ULib.ConsoleLibrary.displayConsoleMessage(userName);
			}
			// display password
			ULib.ConsoleLibrary.displayConsoleMessage("Recovered Password\t: '", false);
			ULib.ConsoleLibrary.displayConsoleMessage((char*)recoveredPassword, false);
			ULib.ConsoleLibrary.displayConsoleMessage("'                ");
			MessageBeep(MB_ICONEXCLAMATION);
		}
		else
		{
			ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2, false);
		}

		ULib.HeapLibrary.freePrivateHeap(userName);
		ULib.HeapLibrary.freePrivateHeap(recoveredPassword);
	}
	else
	{
		displayOptions();
	}

	ULib.ProcessLibrary.exitProcessHandler();
}