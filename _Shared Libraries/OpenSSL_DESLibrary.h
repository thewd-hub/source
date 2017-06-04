///////////////////////////
// OpenSSL - DES Library //
///////////////////////////

// v0.9.8
#define DES_LONG		unsigned long
#define DES_DECRYPT		0
#define DES_ENCRYPT		1

typedef unsigned char DES_cblock[8];
typedef unsigned char const_DES_cblock[8];

typedef struct DES_ks
{
	union
	{
		DES_cblock cblock;
		DES_LONG deslong[2];
	} ks[16];
} DES_key_schedule;

extern "C" void DES_set_key_unchecked(const_DES_cblock* key, DES_key_schedule* schedule);
extern "C" void DES_encrypt1(DES_LONG* data, DES_key_schedule* ks, int enc);
extern "C" void DES_decrypt3(DES_LONG* data, DES_key_schedule* ks1, DES_key_schedule* ks2, DES_key_schedule* ks3);