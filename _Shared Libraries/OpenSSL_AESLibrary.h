///////////////////////////
// OpenSSL - AES Library //
///////////////////////////

// v0.9.8
#define AES_ENCRYPT		1
#define AES_DECRYPT		0

#define AES_MAXNR		14
#define AES_BLOCK_SIZE	16

struct aes_key_st
{
	unsigned long rd_key[4 * (AES_MAXNR + 1)];
	int rounds;
};

typedef struct aes_key_st AES_KEY;

extern "C" int AES_set_encrypt_key(const unsigned char* userKey, const int bits, AES_KEY* key);
extern "C" int AES_set_decrypt_key(const unsigned char* userKey, const int bits, AES_KEY* key);

extern "C" void AES_encrypt(const unsigned char* in, unsigned char* out, const AES_KEY* key);
extern "C" void AES_decrypt(const unsigned char* in, unsigned char* out, const AES_KEY* key);

extern "C" void AES_ctr128_encrypt(const unsigned char* in, unsigned char* out, const unsigned long length, const AES_KEY* key, unsigned char ivec[AES_BLOCK_SIZE], unsigned char ecount_buf[AES_BLOCK_SIZE], unsigned int* num);