#include "ckInfo.h"
#include "..\..\..\_Shared Libraries\OpenSSL_BigNumberLibrary.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

extern CUtilitiesLibrary ULib;

///////////////////////////////////////////////////
// try to retrieve password from encrypted bytes //
///////////////////////////////////////////////////
char* decryptPassword(unsigned char* encryptedPassword, int encryptedPasswordLength)
{
	char* plaintextPassword = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// attempt to find an acceptable password (probably different from original)
	for (int i = 0; i < encryptedPasswordLength; i++)
	{
		// premature end of encrypted password
		if (encryptedPassword[i] == 0x00) break;

		for (int j = 0x5A; j >= 0x20; j--)
		{
			// skip question mark (for characters not found) and other conflict characters
			if ((j == '?') || (j == 'O') || (j == 'o') || (j == '0') || (j == 'l')) continue;
			if (((j >> 3) & 0xFF) == encryptedPassword[i])
			{
				plaintextPassword[i] = (unsigned char)j;
				break;
			}
		}

		// unable to find the password character
		if (plaintextPassword[i] == 0x00) plaintextPassword[i] = '?';
	}

	return plaintextPassword;
}

/////////////////////////////////////////////
// encrypt the supplied plaintext password //
/////////////////////////////////////////////
unsigned char* encryptPassword(unsigned char* plaintextPassword, int plaintextPasswordLength)
{
	// encrypt password
	unsigned char* encryptedPassword = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = 0; i < plaintextPasswordLength; i++) encryptedPassword[i] = (unsigned char)((plaintextPassword[i] >> 3) & 0xFF);
	return encryptedPassword;
}

//////////////////////////////////////////////////
// reverse the bit order of the supplied number //
//////////////////////////////////////////////////
unsigned long reverseBits(unsigned long orgBits)
{
	int newBits = 0;
	if (orgBits & 0x01) newBits += 0x80;
	if (orgBits & 0x02) newBits += 0x40;
	if (orgBits & 0x04) newBits += 0x20;
	if (orgBits & 0x08) newBits += 0x10;
	if (orgBits & 0x10) newBits += 0x08;
	if (orgBits & 0x20) newBits += 0x04;
	if (orgBits & 0x40) newBits += 0x02;
	if (orgBits & 0x80) newBits += 0x01;
	return (newBits & 0xFFFF);
}

//////////////////////////////////////////
// returns the crc for the supplied key //
//////////////////////////////////////////
unsigned short getKeyCRC(unsigned char* key, int keyLength, int keyType)
{
	unsigned short keyCRC = 0xFFFF;
	int startIndex = 1;
	int endIndex = (keyLength - 2);

	if (keyType == KEYTYPE_MASTERKEY)
	{
		startIndex = 2;
	}
	else if ((keyType == KEYTYPE_MASTERKEY6) ||
		(keyType == KEYTYPE_SITEKEY6) ||
		(keyType == KEYTYPE_CUSTOMINFOBYTES))
	{
		startIndex = 0;
	}
	else if ((keyType == KEYTYPE_USERKEY) ||
		(keyType == KEYTYPE_HDSN))
	{
		startIndex = 0;
		endIndex = keyLength;
	}
	else if ((keyType == KEYTYPE_SITECODE) ||
		(keyType == KEYTYPE_SITEKEY) ||
		(keyType == KEYTYPE_RESTRICTIONKEY) ||
		(keyType == KEYTYPE_ENTRYKEY) ||
		(keyType == KEYTYPE_CONFIRMATIONCODE) ||
		(keyType == KEYTYPE_SITECODE6))
	{
	}
	else if (keyType == KEYTYPE_RESTRICTIONKEY6)
	{
		startIndex = 3;
	}
	else
	{
		return keyCRC;
	}

	// get crc value
	for (int i = startIndex; i < endIndex; i++)
	{
		keyCRC ^= (reverseBits(key[i]) << 8);

		for (int j = 0; j < 8; j++)
		{
			if (keyCRC & 0x8000)
			{
				keyCRC *= 2;
				keyCRC ^= 0x8005;
			}
			else
			{
				keyCRC *= 2;
			}
		}
	}

	return (unsigned short)((reverseBits((keyCRC >> 8) & 0xFF) << 8) | (reverseBits(keyCRC & 0xFF)));
}

/////////////////////////////////////////////
// checks to see if the key crc is correct //
/////////////////////////////////////////////
bool isKeyCRCCorrect(unsigned char* key, int keyLength, unsigned short keyCRC, int keyType)
{
	return (keyCRC == getKeyCRC(key, keyLength, keyType));
}

//////////////////////////////
// decrypt the supplied key //
//////////////////////////////
unsigned char* decryptKey(unsigned char* key, int keyLength, int keyType)
{
	unsigned char* keyDecrypted1 = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* keyDecrypted2 = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();

	// stage 1 decryption, simple xor subsitution
	unsigned char* currentMagicKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.addDWORDToBuffer(currentMagicKey, 0xA3F667E4);

	for (int i = 0; i < keyLength; i++)
	{
		keyDecrypted1[i] = (unsigned char)((((key[i] ^ currentMagicKey[0]) ^ currentMagicKey[1]) ^ currentMagicKey[2]) ^ currentMagicKey[3]);
		currentMagicKey[3] ^= currentMagicKey[2];
		currentMagicKey[2] ^= currentMagicKey[1];
		currentMagicKey[1] ^= currentMagicKey[0];
		currentMagicKey[0] ^= keyDecrypted1[i];
	}

	ULib.HeapLibrary.freePrivateHeap(currentMagicKey);

	// stage 2 decryption, big number mathematics (BN_mod_exp)
	BN_CTX* bnCtx = BN_CTX_new();
	BIGNUM* bnResult = BN_new();
	BIGNUM* bnP = BN_new();
	BIGNUM* bnM = BN_new();

	int startIndex = 1;
	int indexCount = 1;
	int keySeed = keyDecrypted1[0];
	keyDecrypted2[0] = keyDecrypted1[0];

	if (keyType == KEYTYPE_MASTERKEY)
	{
		BN_set_word(bnP, 0x000329); // 0x40894800
		BN_set_word(bnM, 0x01FED3); // 0x40FFED30

		keyDecrypted2[1] = keyDecrypted1[1];
		keySeed = ULib.StringLibrary.getWORDFromBuffer(keyDecrypted1);
		startIndex = 2;
	}
	else if ((keyType == KEYTYPE_USERKEY) ||
		(keyType == KEYTYPE_SITEKEY) ||
		(keyType == KEYTYPE_RESTRICTIONKEY) ||
		(keyType == KEYTYPE_ENTRYKEY))
	{
		BN_set_word(bnP, 0x0000E5); // 0x406CA000
		BN_set_word(bnM, 0x01E0E3); // 0x40FE0E30
	}
	else if ((keyType == KEYTYPE_SITECODE) ||
		(keyType == KEYTYPE_CONFIRMATIONCODE))
	{
		BN_set_word(bnP, 0x00042D); // 0x4090B400
		BN_set_word(bnM, 0x01E0E3); // 0x40FE0E30
	}
	else if (keyType == KEYTYPE_RESTRICTIONKEY6)
	{
		BN_set_word(bnP, 0x0000E5); // 0x406CA000
		BN_set_word(bnM, 0x01E0E3); // 0x40FE0E30

		keyDecrypted2[1] = keyDecrypted1[1];
		keyDecrypted2[2] = keyDecrypted1[2];
		keySeed = ULib.StringLibrary.getDWORDFromBuffer(keyDecrypted1);
		keySeed &= 0x00FFFFFF;
		startIndex = 3;
	}

	for (int i = startIndex; i < keyLength; i += 2)
	{
		unsigned long currentValue = ULib.StringLibrary.getWORDFromBuffer(keyDecrypted1, i, true);
		if (indexCount & keySeed) currentValue += 0x10000;
		BN_mod_exp_mont_word(bnResult, currentValue, bnP, bnM, bnCtx, NULL);
		ULib.StringLibrary.addWORDToBuffer(keyDecrypted2, (unsigned short)BN_get_word(bnResult), i, true);
		indexCount *= 2;
	}

	BN_CTX_free(bnCtx);
	BN_free(bnResult);
	BN_free(bnP);
	BN_free(bnM);
	ULib.HeapLibrary.freePrivateHeap(keyDecrypted1);
	return keyDecrypted2;
}

//////////////////////////////
// encrypt the supplied key //
//////////////////////////////
unsigned char* encryptKey(unsigned char* key, int keyLength, int keyType)
{
	unsigned char* keyEncrypted1 = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* keyEncrypted2 = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();

	// stage 1 encryption, big number mathematics (BN_mod_exp)
	BN_CTX* bnCtx = BN_CTX_new();
	BIGNUM* bnResult = BN_new();
	BIGNUM* bnP = BN_new();
	BIGNUM* bnM = BN_new();

	int startIndex = 1;
	int indexCount = 1;
	int keySeed = 0;

	if (keyType == KEYTYPE_MASTERKEY)
	{
		BN_set_word(bnP, 0x000829); // 0x40A05200
		BN_set_word(bnM, 0x01FED3); // 0x40FFED30
		startIndex = 2;
	}
	else if ((keyType == KEYTYPE_USERKEY) ||
		(keyType == KEYTYPE_SITEKEY) ||
		(keyType == KEYTYPE_RESTRICTIONKEY) ||
		(keyType == KEYTYPE_ENTRYKEY))
	{
		BN_set_word(bnP, 0x00042D); // 0x4090B400
		BN_set_word(bnM, 0x01E0E3); // 0x40FE0E30
	}
	else if ((keyType == KEYTYPE_SITECODE) ||
		(keyType == KEYTYPE_CONFIRMATIONCODE))
	{
		BN_set_word(bnP, 0x0000E5); // 0x406CA000
		BN_set_word(bnM, 0x01E0E3); // 0x40FE0E30
	}

	for (int i = startIndex; i < keyLength; i += 2)
	{
		unsigned long currentValue = ULib.StringLibrary.getWORDFromBuffer(key, i, true);
		BN_mod_exp_mont_word(bnResult, currentValue, bnP, bnM, bnCtx, NULL);
		if (BN_get_word(bnResult) >= 0x10000) keySeed += indexCount;
		ULib.StringLibrary.addWORDToBuffer(keyEncrypted1, (unsigned short)BN_get_word(bnResult), i, true);
		indexCount *= 2;
	}

	// store encryption seed
	if (keyType == KEYTYPE_MASTERKEY)
	{
		ULib.StringLibrary.addWORDToBuffer(key, (unsigned short)keySeed);
		ULib.StringLibrary.addWORDToBuffer(keyEncrypted1, (unsigned short)keySeed);
	}
	else
	{
		key[0] = (unsigned char)(keySeed & 0xFF);
		keyEncrypted1[0] = (unsigned char)(keySeed & 0xFF);
	}

	// stage 2 encryption, simple xor subsitution
	unsigned char* currentMagicKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.addDWORDToBuffer(currentMagicKey, 0xA3F667E4);

	for (int i = 0; i < keyLength; i++)
	{
		keyEncrypted2[i] = (unsigned char)((((keyEncrypted1[i] ^ currentMagicKey[0]) ^ currentMagicKey[1]) ^ currentMagicKey[2]) ^ currentMagicKey[3]);
		currentMagicKey[3] ^= currentMagicKey[2];
		currentMagicKey[2] ^= currentMagicKey[1];
		currentMagicKey[1] ^= currentMagicKey[0];
		currentMagicKey[0] ^= keyEncrypted1[i];
	}

	BN_CTX_free(bnCtx);
	BN_free(bnResult);
	BN_free(bnP);
	BN_free(bnM);
	ULib.HeapLibrary.freePrivateHeap(keyEncrypted1);
	ULib.HeapLibrary.freePrivateHeap(currentMagicKey);
	return keyEncrypted2;
}

/////////////////////////////////////////////////////////////////////////////
// decrypts the licence file buffer stored in the windows system directory //
/////////////////////////////////////////////////////////////////////////////
unsigned char* decryptSystemLicenceFile(unsigned char* fileBuffer, unsigned long fileLength)
{
	unsigned char* bufferDecrypted = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	unsigned char* currentMagicKey = (unsigned char*)ULib.HeapLibrary.allocPrivateHeap();
	ULib.StringLibrary.addDWORDToBuffer(currentMagicKey, 0xAFE764BE);

	for (unsigned long i = 0; i < fileLength; i++)
	{
		bufferDecrypted[i] = (unsigned char)((((fileBuffer[i] ^ currentMagicKey[0]) ^ currentMagicKey[1]) ^ currentMagicKey[2]) ^ currentMagicKey[3]);
		currentMagicKey[3] ^= currentMagicKey[2];
		currentMagicKey[2] ^= currentMagicKey[1];
		currentMagicKey[1] ^= currentMagicKey[0];
		currentMagicKey[0] ^= bufferDecrypted[i];
	}

	ULib.HeapLibrary.freePrivateHeap(currentMagicKey);
	return bufferDecrypted;
}