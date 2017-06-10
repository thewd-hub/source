// RC4 Stream Cipher Library
#include "rc4.h"

#define swap_byte(x, y) t = *(x); *(x) = *(y); *(y) = t;

void RC4::prepareKey(unsigned char* keyData, int keyDataLength, RC4_KEY* key)
{
	unsigned char t;
	unsigned char index1;
	unsigned char index2;
	unsigned char* state;
	int counter;

	state = &key->state[0];
	for (counter = 0; counter < 256; counter++)
	{
		state[counter] = (unsigned char)counter;
	}

	key->x = 0;
	key->y = 0;
	index1 = 0;
	index2 = 0;

	for (counter = 0; counter < 256; counter++)
	{
		index2 = (unsigned char)((keyData[index1] + state[counter] + index2) % 256);
		swap_byte(&state[counter], &state[index2]);
		index1 = (unsigned char)((index1 + 1) % keyDataLength);
	}
}

void RC4::performRC4(unsigned char* buffer, int bufferOffset, int bufferLength, RC4_KEY* key)
{
	unsigned char t;
	unsigned char x;
	unsigned char y;
	unsigned char* state;
	unsigned char xorIndex;
	int counter;

	x = key->x;
	y = key->y;
	state = &key->state[0];

	for (counter = 0; counter < bufferLength; counter++)
	{
		x = (unsigned char)((x + 1) % 256);
		y = (unsigned char)((state[x] + y) % 256);
		swap_byte(&state[x], &state[y]);
		xorIndex = (unsigned char)((state[x] + state[y]) % 256);
		buffer[bufferOffset + counter] ^= state[xorIndex];
	}

	key->x = x;
	key->y = y;
}