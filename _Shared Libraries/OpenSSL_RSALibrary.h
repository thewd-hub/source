///////////////////////////
// OpenSSL - RSA Library //
///////////////////////////

// v0.9.8
typedef struct rsa_st RSA;
typedef struct rsa_meth_st RSA_METHOD;
typedef struct crypto_ex_data_st CRYPTO_EX_DATA;

#define RSA_3 0x3L
#define RSA_F4 0x10001L

#define RSA_PKCS1_PADDING 1

struct rsa_meth_st
{
	const char *name;
	int (*rsa_pub_enc)(int flen,const unsigned char *from, unsigned char *to, RSA *rsa,int padding);
	int (*rsa_pub_dec)(int flen,const unsigned char *from, unsigned char *to, RSA *rsa,int padding);
	int (*rsa_priv_enc)(int flen,const unsigned char *from, unsigned char *to, RSA *rsa,int padding);
	int (*rsa_priv_dec)(int flen,const unsigned char *from, unsigned char *to, RSA *rsa,int padding);
	int (*rsa_mod_exp)(BIGNUM *r0,const BIGNUM *I,RSA *rsa,BN_CTX *ctx);
	int (*bn_mod_exp)(BIGNUM *r, const BIGNUM *a, const BIGNUM *p, const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
	int (*init)(RSA *rsa);
	int (*finish)(RSA *rsa);
	int flags;
	char *app_data;
	int (*rsa_sign)(int type, const unsigned char *m, unsigned int m_length, unsigned char *sigret, unsigned int *siglen, const RSA *rsa);
	int (*rsa_verify)(int dtype, const unsigned char *m, unsigned int m_length, unsigned char *sigbuf, unsigned int siglen, const RSA *rsa);
	int (*rsa_keygen)(RSA *rsa, int bits, BIGNUM *e, VOID *cb);
};

struct crypto_ex_data_st
{
	VOID *sk;
	int dummy;
};

struct rsa_st
{
	int pad;
	long version;
	const RSA_METHOD *meth;
	VOID *engine;
	BIGNUM *n;
	BIGNUM *e;
	BIGNUM *d;
	BIGNUM *p;
	BIGNUM *q;
	BIGNUM *dmp1;
	BIGNUM *dmq1;
	BIGNUM *iqmp;
	CRYPTO_EX_DATA ex_data;
	int references;
	int flags;
	VOID *_method_mod_n;
	VOID *_method_mod_p;
	VOID *_method_mod_q;
	char *bignum_data;
	VOID *blinding;
	VOID *mt_blinding;
};

extern "C" RSA* RSA_new(void);
extern "C" void RSA_free(RSA *r);

extern "C" int RSA_public_decrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa,int padding);
extern "C" int RSA_private_encrypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa,int padding);