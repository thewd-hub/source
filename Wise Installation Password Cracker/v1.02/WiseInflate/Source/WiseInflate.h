//////////////////////////////////////////////////
// WiseInflate                                  //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////

extern "C" int inflateWiseScript(char* sourceFilename,
								 unsigned long scriptOffset,
								 unsigned char* streamBuffer,
								 unsigned long* scriptCompressedSize,
								 unsigned long* scriptExpandedSize,
								 unsigned long* streamCRC);