// RC4 Stream Cipher Library
typedef struct
{
	unsigned char state[256];
	unsigned char x;
	unsigned char y;
} RC4_KEY;

class RC4
{
	public:
		void prepareKey(unsigned char* keyData, int keyDataLength, RC4_KEY* key);
		void performRC4(unsigned char* buffer, int bufferOffset, int bufferLength, RC4_KEY* key);
};