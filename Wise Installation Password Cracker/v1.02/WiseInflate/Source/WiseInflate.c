//////////////////////////////////////////////////
// WiseInflate                                  //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Decompresses the Wise installation script    //
//                                              //
// based on exwise v0.5: (c) Andrew de Quincey  //
// portions based on funzip.c by M. Adler et al //
// portions based on Info-ZIP v5.31             //
//////////////////////////////////////////////////
#define FUNZIP
#define UNZIP_INTERNAL
#include "unzip.h"
#include <windows.h>

unsigned char* outputBufferPtr;
unsigned long outputBufferPtrPosition = 0;

//////////////////////////////////////////////////////
// stores the inflated stream to the desired string //
//////////////////////////////////////////////////////
void storeInflatedStream(unsigned long streamLength)
{
	unsigned long i = 0;
	G.crc32val = crc32(G.crc32val, slide, (extent)streamLength);
	for (; i < streamLength; i++) outputBufferPtr[outputBufferPtrPosition + i] = slide[i];
	outputBufferPtrPosition += streamLength;
}

//////////////////////////////////////////////////
// used by inflate to write the inflated stream //
//////////////////////////////////////////////////
int flush(unsigned long w)
{
	storeInflatedStream(w);
	return 0;
}

////////////////////////////////////////////////////////////////
// inflates the compressed stream and writes to memory buffer //
////////////////////////////////////////////////////////////////
int inflateWiseScript(char* sourceFilename,
					  unsigned long scriptOffset,
					  unsigned char* streamBuffer,
					  unsigned long* scriptCompressedSize,
					  unsigned long* scriptExpandedSize,
					  unsigned long* streamCRC)
{
	int result;
	outputBufferPtr = streamBuffer;

	// open file
	if ((G.in = fopen(sourceFilename, "rb")) == NULL) return 0;
	// jump to correct offset
	fseek(G.in, scriptOffset, SEEK_SET);

	// prepare output buffer and crc
	G.outptr = slide;
	G.outcnt = 0L;
	G.crc32val = CRCVAL_INITIAL;

	// inflate stream
	if ((result = inflate(__G)) != 0) return 4;
	inflate_free(__G);

	// flush one last time; no need to reset G.outptr/outcnt
	if (G.outcnt) storeInflatedStream(G.outcnt);

	*scriptCompressedSize = (ftell(G.in) - scriptOffset);
	*scriptExpandedSize = outputBufferPtrPosition;
	*streamCRC = G.crc32val;
	fclose(G.in);
	return -1;
}