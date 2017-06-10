///////////////////////////////////////////////////
// Generates words from the specified dictionary //
///////////////////////////////////////////////////
#include <windows.h>
#include "DictionaryEngine.h"

const unsigned long DICTIONARYENGINE_BLOCK_SIZE = 5000000;
const unsigned long DICTIONARYENGINE_MAX_WORD_LENGTH = 128;
const char DictionaryEngine_LineSeparator[] = "\r\n";

typedef struct _DICTIONARYENGINE_WORD
{
	unsigned long dwLength;
	char* lpszWord;
	_DICTIONARYENGINE_WORD* lpNextDictionaryWord;
} DICTIONARYENGINE_WORD;

HANDLE DictionaryEngine_FileHandle = NULL;
HANDLE DictionaryEngine_PrivateHeap = NULL;

DICTIONARYENGINE_WORD* DictionaryEngine_RootDictionaryWord = NULL;
DICTIONARYENGINE_WORD* DictionaryEngine_CurrentDictionaryWord = NULL;
char* DictionaryEngine_CurrentWord = NULL;
unsigned long DictionaryEngine_WordLength = 0;

unsigned long DictionaryEngine_DictioneryFilter = DICTIONARYENGINE_FILTER_DEFAULT;

unsigned long DictionaryEngine_CurrentFileBlock = 0;
unsigned long DictionaryEngine_NumberOfFileBlocks = 0;

////////////////////////////////////////////////////////////////////////
// opens the dictionary file name and initialises the data structures //
////////////////////////////////////////////////////////////////////////
bool DictionaryEngine::initialiseDictionaryEngine(char* dictionaryFilename)
{
	return initialiseDictionaryEngine(dictionaryFilename, DICTIONARYENGINE_FILTER_DEFAULT, 0);
}

////////////////////////////////////////////////////////////////////////
// opens the dictionary file name and initialises the data structures //
////////////////////////////////////////////////////////////////////////
bool DictionaryEngine::initialiseDictionaryEngine(char* dictionaryFilename, unsigned long dictionaryFilter)
{
	return initialiseDictionaryEngine(dictionaryFilename, dictionaryFilter, 0);
}

////////////////////////////////////////////////////////////////////////
// opens the dictionary file name and initialises the data structures //
////////////////////////////////////////////////////////////////////////
bool DictionaryEngine::initialiseDictionaryEngine(char* dictionaryFilename, unsigned long dictionaryFilter, int dictionaryWordLength)
{
	// open handle to file
	DictionaryEngine_FileHandle = CreateFile(dictionaryFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (DictionaryEngine_FileHandle == INVALID_HANDLE_VALUE) return false;

	// get file size
	unsigned long DictionaryEngine_FileSize = GetFileSize(DictionaryEngine_FileHandle, NULL);
	if (DictionaryEngine_FileSize <= 0) return false;

	DictionaryEngine_RootDictionaryWord = NULL;
	DictionaryEngine_CurrentDictionaryWord = NULL;
	DictionaryEngine_CurrentWord = NULL;
	DictionaryEngine_WordLength = ((dictionaryWordLength > DICTIONARYENGINE_MAX_WORD_LENGTH) ? 0 : dictionaryWordLength);

	DictionaryEngine_DictioneryFilter = dictionaryFilter;

	DictionaryEngine_CurrentFileBlock = 0;
	DictionaryEngine_NumberOfFileBlocks = ((DictionaryEngine_FileSize / DICTIONARYENGINE_BLOCK_SIZE) + 1);

	return true;
}

//////////////////////////////////////////////////////////////////
// close handle to the dictionary and tidy up memory structures //
//////////////////////////////////////////////////////////////////
void DictionaryEngine::destroyDictionaryEngine(void)
{
	__try
	{
		CloseHandle(DictionaryEngine_FileHandle);
		HeapDestroy(DictionaryEngine_PrivateHeap);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

//////////////////////////////////////////////////////////////////////////
// reads the next file block and creates the dictionary word structures //
//////////////////////////////////////////////////////////////////////////
bool readDictionaryBlock(void)
{
	// finished reading all memory blocks
	if (DictionaryEngine_CurrentFileBlock >= DictionaryEngine_NumberOfFileBlocks) return false;

	// destroy previous memory heap
	if (DictionaryEngine_CurrentFileBlock > 0)
	{
		HeapDestroy(DictionaryEngine_PrivateHeap);

		// move file pointer back to read split words
		SetFilePointer(DictionaryEngine_FileHandle, ~DICTIONARYENGINE_MAX_WORD_LENGTH, NULL, FILE_CURRENT);
	}

	DictionaryEngine_CurrentFileBlock++;

	DictionaryEngine_PrivateHeap = HeapCreate(NULL, DICTIONARYENGINE_BLOCK_SIZE * 2, 0);
	unsigned char* DictionaryEngine_FileBuffer = (unsigned char*)HeapAlloc(DictionaryEngine_PrivateHeap, NULL, DICTIONARYENGINE_BLOCK_SIZE + 1);
	char* DictionaryEngine_TempBuffer = (char*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, DICTIONARYENGINE_MAX_WORD_LENGTH + 1);
	DictionaryEngine_CurrentWord = (char*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, DICTIONARYENGINE_MAX_WORD_LENGTH + 1);

	// read in dictionary block
	unsigned long bytesRead;
	if (ReadFile(DictionaryEngine_FileHandle, DictionaryEngine_FileBuffer, DICTIONARYENGINE_BLOCK_SIZE, &bytesRead, NULL) == 0) return false;
	if (bytesRead == 0) return false;

	DictionaryEngine_RootDictionaryWord = (DICTIONARYENGINE_WORD*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, sizeof(DICTIONARYENGINE_WORD));
	DictionaryEngine_RootDictionaryWord->dwLength = 0;
	DictionaryEngine_RootDictionaryWord->lpszWord = NULL;
	DictionaryEngine_RootDictionaryWord->lpNextDictionaryWord = NULL;

	DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_RootDictionaryWord;

	// get the first line from the dictionary
	char* DictionaryEngine_CurrentBuffer = strtok((char*)DictionaryEngine_FileBuffer, DictionaryEngine_LineSeparator);
	unsigned long DictionaryEngine_CurrentBufferLength = 0;

	// store the dictionary words (based on the filter)
	while ((DictionaryEngine_CurrentBuffer != NULL) && (DictionaryEngine_CurrentBuffer[0] != NULL))
	{
		DictionaryEngine_CurrentBufferLength = (int)strlen(DictionaryEngine_CurrentBuffer);
		if (DictionaryEngine_CurrentBufferLength > DICTIONARYENGINE_MAX_WORD_LENGTH) DictionaryEngine_CurrentBufferLength = DICTIONARYENGINE_MAX_WORD_LENGTH;

		// check for correct length (if required)
		if ((DictionaryEngine_WordLength > 0) && (DictionaryEngine_CurrentBufferLength != DictionaryEngine_WordLength))
		{
			// get the next line from the dictionary
			DictionaryEngine_CurrentBuffer = strtok(NULL, DictionaryEngine_LineSeparator);
			if (DictionaryEngine_CurrentBuffer == NULL) break;
			continue;
		}

		// store default word
		if (DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_DEFAULT)
		{
			DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = (DICTIONARYENGINE_WORD*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, sizeof(DICTIONARYENGINE_WORD));
			DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord;
			DictionaryEngine_CurrentDictionaryWord->dwLength = DictionaryEngine_CurrentBufferLength;
			DictionaryEngine_CurrentDictionaryWord->lpszWord = (char*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, DictionaryEngine_CurrentBufferLength + 1);
			strncpy(DictionaryEngine_CurrentDictionaryWord->lpszWord, DictionaryEngine_CurrentBuffer, DictionaryEngine_CurrentBufferLength);
			DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = NULL;
		}
		// store lowercase word (if required)
		if (DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_LOWERCASE)
		{
			strncpy(DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBuffer, DictionaryEngine_CurrentBufferLength);
			CharLowerBuff(DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBufferLength);
			// check if there's a difference
			if ((!(DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_DEFAULT)) || (strcmp(DictionaryEngine_CurrentBuffer, DictionaryEngine_TempBuffer) != 0))
			{
				DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = (DICTIONARYENGINE_WORD*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, sizeof(DICTIONARYENGINE_WORD));
				DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord;
				DictionaryEngine_CurrentDictionaryWord->dwLength = DictionaryEngine_CurrentBufferLength;
				DictionaryEngine_CurrentDictionaryWord->lpszWord = (char*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, DictionaryEngine_CurrentBufferLength + 1);
				strncpy(DictionaryEngine_CurrentDictionaryWord->lpszWord, DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBufferLength);
				DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = NULL;
			}
		}
		// store uppercase word (if required)
		if (DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_UPPERCASE)
		{
			strncpy(DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBuffer, DictionaryEngine_CurrentBufferLength);
			CharUpperBuff(DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBufferLength);
			// check if there's a difference
			if ((!(DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_DEFAULT)) || (strcmp(DictionaryEngine_CurrentBuffer, DictionaryEngine_TempBuffer) != 0))
			{
				DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = (DICTIONARYENGINE_WORD*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, sizeof(DICTIONARYENGINE_WORD));
				DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord;
				DictionaryEngine_CurrentDictionaryWord->dwLength = DictionaryEngine_CurrentBufferLength;
				DictionaryEngine_CurrentDictionaryWord->lpszWord = (char*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, DictionaryEngine_CurrentBufferLength + 1);
				strncpy(DictionaryEngine_CurrentDictionaryWord->lpszWord, DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBufferLength);
				DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = NULL;
			}
		}
		// store capitalised word (if required)
		if (DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_CAPITALISE)
		{
			strncpy(DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBuffer, DictionaryEngine_CurrentBufferLength);
			CharLowerBuff(DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBufferLength);
			// uppercase first letter
			if (IsCharLower(DictionaryEngine_TempBuffer[0])) DictionaryEngine_TempBuffer[0] -= 0x20;
			// check if there's a difference
			if ((!(DictionaryEngine_DictioneryFilter & DICTIONARYENGINE_FILTER_DEFAULT)) || (strcmp(DictionaryEngine_CurrentBuffer, DictionaryEngine_TempBuffer) != 0))
			{
				DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = (DICTIONARYENGINE_WORD*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, sizeof(DICTIONARYENGINE_WORD));
				DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord;
				DictionaryEngine_CurrentDictionaryWord->dwLength = DictionaryEngine_CurrentBufferLength;
				DictionaryEngine_CurrentDictionaryWord->lpszWord = (char*)HeapAlloc(DictionaryEngine_PrivateHeap, HEAP_ZERO_MEMORY, DictionaryEngine_CurrentBufferLength + 1);
				strncpy(DictionaryEngine_CurrentDictionaryWord->lpszWord, DictionaryEngine_TempBuffer, DictionaryEngine_CurrentBufferLength);
				DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord = NULL;
			}
		}

		// get the next line from the dictionary
		DictionaryEngine_CurrentBuffer = strtok(NULL, DictionaryEngine_LineSeparator);
		if (DictionaryEngine_CurrentBuffer == NULL) break;
	}

	DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_RootDictionaryWord->lpNextDictionaryWord;
	HeapFree(DictionaryEngine_PrivateHeap, NULL, DictionaryEngine_TempBuffer);
	HeapFree(DictionaryEngine_PrivateHeap, NULL, DictionaryEngine_FileBuffer);
	return true;
}

////////////////////////////////////////////
// reset the dictionary engine structures //
////////////////////////////////////////////
void DictionaryEngine::resetDictionaryEngine(void)
{
	__try
	{
		HeapDestroy(DictionaryEngine_PrivateHeap);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	DictionaryEngine_CurrentFileBlock = 0;
	SetFilePointer(DictionaryEngine_FileHandle, 0, NULL, FILE_BEGIN);

	DictionaryEngine_RootDictionaryWord = NULL;
	DictionaryEngine_CurrentDictionaryWord = NULL;
	DictionaryEngine_CurrentWord = NULL;
}

//////////////////////////////////////////////////////////////
// retrieve the next word from the dictionary (with length) //
//////////////////////////////////////////////////////////////
char* DictionaryEngine::getNextWord(unsigned long* dwNextWordLength)
{
	if (DictionaryEngine_CurrentDictionaryWord != NULL)
	{
		DictionaryEngine_CurrentWord = DictionaryEngine_CurrentDictionaryWord->lpszWord;
		*dwNextWordLength = DictionaryEngine_CurrentDictionaryWord->dwLength;
		DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord;
		return DictionaryEngine_CurrentWord;
	}
	else
	{
		// try to read next dictionary block
		if ((readDictionaryBlock()) && (DictionaryEngine_CurrentDictionaryWord != NULL))
		{
			DictionaryEngine_CurrentWord = DictionaryEngine_CurrentDictionaryWord->lpszWord;
			*dwNextWordLength = DictionaryEngine_CurrentDictionaryWord->dwLength;
			DictionaryEngine_CurrentDictionaryWord = DictionaryEngine_CurrentDictionaryWord->lpNextDictionaryWord;
		}
		else
		{
			*dwNextWordLength = 0;
			DictionaryEngine_CurrentWord[0] = NULL;
		}

		return DictionaryEngine_CurrentWord;
	}
}