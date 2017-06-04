//////////////////////////////////
// OpenSSL - Big Number Library //
//////////////////////////////////

// v0.9.8
#define BN_ULONG			unsigned long
#define BN_CTX_NUM			32
#define BN_CTX_NUM_POS		12
#define BN_CTX_POOL_SIZE	16
#define BN_num_bytes(a)		((BN_num_bits(a) + 7) / 8)

typedef struct bignum_st
{
	BN_ULONG* d;
	int top;
	int dmax;
	int neg;
	int flags;
} BIGNUM;

typedef struct bn_mont_ctx_st
{
	int ri;
	BIGNUM RR;
	BIGNUM N;
	BIGNUM Ni;
	BN_ULONG n0;
	int flags;
} BN_MONT_CTX;

// v0.9.8
typedef struct bignum_pool_item
{
	BIGNUM vals[BN_CTX_POOL_SIZE];
	struct bignum_pool_item *prev, *next;
} BN_POOL_ITEM;

typedef struct bignum_pool
{
	BN_POOL_ITEM *head, *current, *tail;
	unsigned used, size;
} BN_POOL;

typedef struct bignum_ctx_stack
{
	unsigned int* indexes;
	unsigned int depth, size;
} BN_STACK;

typedef struct bignum_ctx
{
	BN_POOL pool;
	BN_STACK stack;
	unsigned int used;
	int err_stack;
	int too_many;
} BN_CTX;

// v0.9.7
/*typedef struct bignum_ctx
{
	int tos;
	BIGNUM bn[BN_CTX_NUM];
	int flags;
	int depth;
	int pos[BN_CTX_NUM_POS];
	int too_many;
} BN_CTX;*/

extern "C" BN_CTX* BN_CTX_new(void);
extern "C" BIGNUM* BN_new(void);
extern "C" void BN_CTX_free(BN_CTX* c);
extern "C" void BN_free(BIGNUM* a);

extern "C" int BN_num_bits(const BIGNUM* a);
extern "C" BN_ULONG BN_get_word(const BIGNUM* a);
extern "C" int BN_set_word(BIGNUM* a, BN_ULONG w);
extern "C" int BN_bn2bin(const BIGNUM* a, unsigned char* to);
extern "C" BIGNUM* BN_bin2bn(const unsigned char* s, int len, BIGNUM* ret);
extern "C" int BN_hex2bn(BIGNUM** a, const char* str);

extern "C" int BN_mod_exp(BIGNUM* r, BIGNUM* a, const BIGNUM* p, const BIGNUM* m, BN_CTX* ctx);
extern "C" int BN_mod_exp_mont_word(BIGNUM* r, BN_ULONG a, const BIGNUM* p, const BIGNUM* m, BN_CTX* ctx, BN_MONT_CTX* m_ctx);
extern "C" int BN_add(BIGNUM* r, const BIGNUM* a, const BIGNUM* b);
extern "C" int BN_mod_sub(BIGNUM* r, const BIGNUM* a, const BIGNUM* b, const BIGNUM* m, BN_CTX* ctx);
extern "C" int BN_mul(BIGNUM* r, const BIGNUM* a, const BIGNUM* b, BN_CTX* ctx);
extern "C" int BN_mod_mul(BIGNUM* r, const BIGNUM* a, const BIGNUM* b, const BIGNUM* m, BN_CTX* ctx);
extern "C" int BN_div(BIGNUM* dv, BIGNUM* rem, const BIGNUM* m, const BIGNUM* d, BN_CTX* ctx);