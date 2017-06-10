// MD5 Message-Digest Library
typedef unsigned char* POINTER;
typedef unsigned short UINT2;
typedef unsigned long UINT4;

typedef struct
{
	UINT4 state[4];
	UINT4 count[2];
	unsigned char buffer[64];
} MD5_CONTEXT;

class MD5
{
	public:
		void MD5Init(MD5_CONTEXT* context);
		void MD5Update(MD5_CONTEXT* context, unsigned char* inputBuffer, unsigned int inputBufferLength);
		void MD5Final(unsigned char digest[16], MD5_CONTEXT* context);
};