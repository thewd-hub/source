////////////////////////////////
// OpenSSL - Blowfish Library //
////////////////////////////////

// v0.9.8
#define BF_LONG			unsigned long
#define BF_ROUNDS		16
#define BF_ENCRYPT		1
#define BF_DECRYPT		0

typedef struct bf_key_st
{
	BF_LONG P[BF_ROUNDS + 2];
	BF_LONG S[4 * 256];
} BF_KEY;

extern "C" void BF_set_key(BF_KEY* key, int len, const unsigned char* data);
extern "C" void BF_encrypt(BF_LONG* data, const BF_KEY* key);
extern "C" void BF_decrypt(BF_LONG* data, const BF_KEY* key);