//////////////////////////////////////////////////////////////////////
// Generate words of length 1 to BRUTEFORCEENGINE_MAX_WORD_LENGTH,  //
// using the lowercase, uppercase, number or special character sets //
//////////////////////////////////////////////////////////////////////
#include <windows.h>
#include "..\BruteForceEngine.h"

const int BRUTEFORCEENGINE_MAX_WORD_LENGTH = 16;

const int LOWERCASE_CHARSET_LENGTH = 26;
const int UPPERCASE_CHARSET_LENGTH = 26;
const int NUMBER_CHARSET_LENGTH = 10;
const int SPECIAL_CHARSET_LENGTH = 32;

const char LOWERCASE_CHARSET[LOWERCASE_CHARSET_LENGTH] =
{
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};
const char UPPERCASE_CHARSET[UPPERCASE_CHARSET_LENGTH] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};
const char NUMBER_CHARSET[NUMBER_CHARSET_LENGTH] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
};
const char SPECIAL_CHARSET[SPECIAL_CHARSET_LENGTH] =
{
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+',
	',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[',
	']', '\\', '^', '_', '{',  '}', '|', '~'
};

//////////////////////////////////////////////////////////////////////////
// creates the character set to be used in the word generation routines //
//////////////////////////////////////////////////////////////////////////
void BruteForceEngine::initialiseCharacterSet(bool useLowercase, bool useUppercase, bool useNumbers, bool useSpecial)
{
	BruteForceEngine_PrivateHeap = HeapCreate(NULL, 10000, 0);

	BruteForceEngine_ActualCharSetLength = 0;
	if (useLowercase) BruteForceEngine_ActualCharSetLength += LOWERCASE_CHARSET_LENGTH;
	if (useUppercase) BruteForceEngine_ActualCharSetLength += UPPERCASE_CHARSET_LENGTH;
	if (useNumbers) BruteForceEngine_ActualCharSetLength += NUMBER_CHARSET_LENGTH;
	if (useSpecial) BruteForceEngine_ActualCharSetLength += SPECIAL_CHARSET_LENGTH;

	BruteForceEngine_ActualCharSet = (char*)HeapAlloc(BruteForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, BruteForceEngine_ActualCharSetLength + 1);
	BruteForceEngine_CurrentWord = (char*)HeapAlloc(BruteForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);
	BruteForceEngine_IndexCounters = (unsigned char*)HeapAlloc(BruteForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);

	int loopIndex = 0;
	int charSetIndex = 0;

	if (useLowercase) for (loopIndex = 0; loopIndex < LOWERCASE_CHARSET_LENGTH; loopIndex++) BruteForceEngine_ActualCharSet[charSetIndex++] = LOWERCASE_CHARSET[loopIndex];
	if (useUppercase) for (loopIndex = 0; loopIndex < UPPERCASE_CHARSET_LENGTH; loopIndex++) BruteForceEngine_ActualCharSet[charSetIndex++] = UPPERCASE_CHARSET[loopIndex];
	if (useNumbers) for (loopIndex = 0; loopIndex < NUMBER_CHARSET_LENGTH; loopIndex++) BruteForceEngine_ActualCharSet[charSetIndex++] = NUMBER_CHARSET[loopIndex];
	if (useSpecial) for (loopIndex = 0; loopIndex < SPECIAL_CHARSET_LENGTH; loopIndex++) BruteForceEngine_ActualCharSet[charSetIndex++] = SPECIAL_CHARSET[loopIndex];

	BruteForceEngine_FirstCharacter = BruteForceEngine_ActualCharSet[0];
}

//////////////////////////////////////////////////////////////////////////
// creates the character set to be used in the word generation routines //
//////////////////////////////////////////////////////////////////////////
void BruteForceEngine::initialiseCharacterSet(char* customCharacterSet)
{
	BruteForceEngine_PrivateHeap = HeapCreate(NULL, 10000, 0);

	BruteForceEngine_ActualCharSetLength = (int)strlen(customCharacterSet);
	BruteForceEngine_ActualCharSet = (char*)HeapAlloc(BruteForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, BruteForceEngine_ActualCharSetLength + 1);
	BruteForceEngine_CurrentWord = (char*)HeapAlloc(BruteForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);
	BruteForceEngine_IndexCounters = (unsigned char*)HeapAlloc(BruteForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);

	for (int loopIndex = 0; loopIndex < BruteForceEngine_ActualCharSetLength; loopIndex++) BruteForceEngine_ActualCharSet[loopIndex] = customCharacterSet[loopIndex];

	BruteForceEngine_FirstCharacter = BruteForceEngine_ActualCharSet[0];
}

///////////////////////////////////////////////////////////
// cleans up the variable state when completely finsihed //
///////////////////////////////////////////////////////////
void BruteForceEngine::destroyCharacterSet(void)
{
	HeapFree(BruteForceEngine_PrivateHeap, NULL, BruteForceEngine_IndexCounters);
	HeapFree(BruteForceEngine_PrivateHeap, NULL, BruteForceEngine_CurrentWord);
	HeapFree(BruteForceEngine_PrivateHeap, NULL, BruteForceEngine_ActualCharSet);
	HeapDestroy(BruteForceEngine_PrivateHeap);
}

/////////////////////////////////////////////////////
// returns the length of the current character set //
/////////////////////////////////////////////////////
int BruteForceEngine::getCharacterSetLength(void)
{
	return BruteForceEngine_ActualCharSetLength;
}

///////////////////////////////////////////////////////////
// set the new word length of the words to be generated  //
// Resets all index counters and starts generating words //
///////////////////////////////////////////////////////////
void BruteForceEngine::setCurrentWordLength(int wordLength)
{
	ZeroMemory(BruteForceEngine_IndexCounters, BRUTEFORCEENGINE_MAX_WORD_LENGTH);
	ZeroMemory(BruteForceEngine_CurrentWord, BRUTEFORCEENGINE_MAX_WORD_LENGTH);

	BruteForceEngine_IndexCounters[0] = 0xFF;
	BruteForceEngine_CurrentWordLength = ((wordLength > BRUTEFORCEENGINE_MAX_WORD_LENGTH) ? BRUTEFORCEENGINE_MAX_WORD_LENGTH : wordLength);
	for (int i = 0; i < BruteForceEngine_CurrentWordLength; i++) BruteForceEngine_CurrentWord[i] = BruteForceEngine_FirstCharacter;
}

////////////////////////////////////////////////////////////
// returns the length of the current word to be generated //
////////////////////////////////////////////////////////////
int BruteForceEngine::getCurrentWordLength(void)
{
	return BruteForceEngine_CurrentWordLength;
}

///////////////////////////////////////////////
// returns the approximate percentage of the //
// work done for the current word length     //
///////////////////////////////////////////////
double BruteForceEngine::getPercentageDone(void)
{
	return (((double)BruteForceEngine_IndexCounters[(BruteForceEngine_CurrentWordLength - 1)] / (double)BruteForceEngine_ActualCharSetLength) * 100);
}

////////////////////////////////////////////////
// returns the next word in the character set //
////////////////////////////////////////////////
char* BruteForceEngine::getNextWord(void)
{
	char* BruteForceEngine_ActualCharSet = this->BruteForceEngine_ActualCharSet;
	int BruteForceEngine_ActualCharSetLength = this->BruteForceEngine_ActualCharSetLength;

	int BruteForceEngine_CurrentWordLength = this->BruteForceEngine_CurrentWordLength;
	char* BruteForceEngine_CurrentWord = this->BruteForceEngine_CurrentWord;
	unsigned char* BruteForceEngine_IndexCounters = this->BruteForceEngine_IndexCounters;
	unsigned char BruteForceEngine_FirstCharacter = this->BruteForceEngine_FirstCharacter;

	switch (BruteForceEngine_CurrentWordLength)
	{
		// 1 character
		case 1:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone01
					mov [ebx], 0x00

				nextWordDone01:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 2 characters
		case 2:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone02
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone02
					mov [ebx], 0x00

				nextWordDone02:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 3 characters
		case 3:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone03
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone03
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone03
					mov [ebx], 0x00

				nextWordDone03:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 4 characters
		case 4:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone04
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone04
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone04
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone04
					mov [ebx], 0x00

				nextWordDone04:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 5 characters
		case 5:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone05
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone05
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone05
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone05
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone05
					mov [ebx], 0x00

				nextWordDone05:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 6 characters
		case 6:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone06
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone06
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone06
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone06
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone06
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone06
					mov [ebx], 0x00

				nextWordDone06:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 7 characters
		case 7:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone07
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone07
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone07
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone07
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone07
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone07
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone07
					mov [ebx], 0x00

				nextWordDone07:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 8 characters
		case 8:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone08
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone08
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone08
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone08
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone08
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone08
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone08
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone08
					mov [ebx], 0x00

				nextWordDone08:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 9 characters
		case 9:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone09
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone09
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone09
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone09
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone09
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone09
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone09
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone09
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone09
					mov [ebx], 0x00

				nextWordDone09:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 10 characters
		case 10:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone10
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone10
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone10
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone10
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone10
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone10
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone10
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone10
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone10
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone10
					mov [ebx], 0x00

				nextWordDone10:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 11 characters
		case 11:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone11
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone11
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone11
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone11
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone11
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone11
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone11
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone11
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone11
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone11
					// increment character 11
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 9], al
					mov [esi + 9], 0x00
					inc [esi + 10]
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordDone11
					mov [ebx], 0x00

				nextWordDone11:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 12 characters
		case 12:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone12
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone12
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone12
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone12
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone12
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone12
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone12
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone12
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone12
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone12
					// increment character 11
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 9], al
					mov [esi + 9], 0x00
					inc [esi + 10]
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordDone12
					// increment character 12
					mov al, [BruteForceEngine_FirstCharacter]
					mov [esi + 10], 0x00
					mov [ebx + 10], al
					inc [esi + 11]
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordDone12
					mov [ebx], 0x00

				nextWordDone12:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 13 characters
		case 13:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone13
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone13
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone13
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone13
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone13
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone13
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone13
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone13
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone13
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone13
					// increment character 11
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 9], al
					mov [esi + 9], 0x00
					inc [esi + 10]
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordDone13
					// increment character 12
					mov al, [BruteForceEngine_FirstCharacter]
					mov [esi + 10], 0x00
					mov [ebx + 10], al
					inc [esi + 11]
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordDone13
					// increment character 13
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 11], al
					mov [esi + 11], 0x00
					inc [esi + 12]
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordDone13
					mov [ebx], 0x00

				nextWordDone13:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 14 characters
		case 14:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone14
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone14
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone14
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone14
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone14
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone14
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone14
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone14
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone14
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone14
					// increment character 11
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 9], al
					mov [esi + 9], 0x00
					inc [esi + 10]
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordDone14
					// increment character 12
					mov al, [BruteForceEngine_FirstCharacter]
					mov [esi + 10], 0x00
					mov [ebx + 10], al
					inc [esi + 11]
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordDone14
					// increment character 13
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 11], al
					mov [esi + 11], 0x00
					inc [esi + 12]
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordDone14
					// increment character 14
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 12], al
					mov [esi + 12], 0x00
					inc [esi + 13]
					mov al, [esi + 13]
					mov dl, [edi + eax]
					mov [ebx + 13], dl
					cmp [esi + 13], cl
					jl nextWordDone14
					mov [ebx], 0x00

				nextWordDone14:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 15 characters
		case 15:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone15
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone15
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone15
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone15
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone15
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone15
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone15
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone15
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone15
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone15
					// increment character 11
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 9], al
					mov [esi + 9], 0x00
					inc [esi + 10]
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordDone15
					// increment character 12
					mov al, [BruteForceEngine_FirstCharacter]
					mov [esi + 10], 0x00
					mov [ebx + 10], al
					inc [esi + 11]
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordDone15
					// increment character 13
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 11], al
					mov [esi + 11], 0x00
					inc [esi + 12]
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordDone15
					// increment character 14
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 12], al
					mov [esi + 12], 0x00
					inc [esi + 13]
					mov al, [esi + 13]
					mov dl, [edi + eax]
					mov [ebx + 13], dl
					cmp [esi + 13], cl
					jl nextWordDone15
					// increment character 15
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 13], al
					mov [esi + 13], 0x00
					inc [esi + 14]
					mov al, [esi + 14]
					mov dl, [edi + eax]
					mov [ebx + 14], dl
					cmp [esi + 14], cl
					jl nextWordDone15
					mov [ebx], 0x00

				nextWordDone15:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		// 16 characters
		case 16:
		{
			__asm
			{
					push eax
					push ebx
					push ecx
					push esi
					push edi

					xor eax, eax
					mov ebx, BruteForceEngine_CurrentWord
					mov ecx, [BruteForceEngine_ActualCharSetLength]
					mov esi, BruteForceEngine_IndexCounters
					mov edi, BruteForceEngine_ActualCharSet

					// increment character 1
					inc [esi + 0]
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jl nextWordDone16
					// increment character 2
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 0], al
					mov [esi + 0], 0x00
					inc [esi + 1]
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordDone16
					// increment character 3
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 1], al
					mov [esi + 1], 0x00
					inc [esi + 2]
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordDone16
					// increment character 4
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 2], al
					mov [esi + 2], 0x00
					inc [esi + 3]
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordDone16
					// increment character 5
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 3], al
					mov [esi + 3], 0x00
					inc [esi + 4]
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordDone16
					// increment character 6
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 4], al
					mov [esi + 4], 0x00
					inc [esi + 5]
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordDone16
					// increment character 7
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 5], al
					mov [esi + 5], 0x00
					inc [esi + 6]
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordDone16
					// increment character 8
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 6], al
					mov [esi + 6], 0x00
					inc [esi + 7]
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordDone16
					// increment character 9
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 7], al
					mov [esi + 7], 0x00
					inc [esi + 8]
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordDone16
					// increment character 10
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 8], al
					mov [esi + 8], 0x00
					inc [esi + 9]
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordDone16
					// increment character 11
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 9], al
					mov [esi + 9], 0x00
					inc [esi + 10]
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordDone16
					// increment character 12
					mov al, [BruteForceEngine_FirstCharacter]
					mov [esi + 10], 0x00
					mov [ebx + 10], al
					inc [esi + 11]
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordDone16
					// increment character 13
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 11], al
					mov [esi + 11], 0x00
					inc [esi + 12]
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordDone16
					// increment character 14
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 12], al
					mov [esi + 12], 0x00
					inc [esi + 13]
					mov al, [esi + 13]
					mov dl, [edi + eax]
					mov [ebx + 13], dl
					cmp [esi + 13], cl
					jl nextWordDone16
					// increment character 15
					mov al, [BruteForceEngine_FirstCharacter]
					mov [ebx + 13], al
					mov [esi + 13], 0x00
					inc [esi + 14]
					mov al, [esi + 14]
					mov dl, [edi + eax]
					mov [ebx + 14], dl
					cmp [esi + 14], cl
					jl nextWordDone16
					// increment character 16
					mov al, [BruteForceEngine_FirstCharacter]
					mov [esi + 14], 0x00
					mov [ebx + 14], al
					inc [esi + 15]
					mov al, [esi + 15]
					mov dl, [edi + eax]
					mov [ebx + 15], dl
					cmp [esi + 15], cl
					jl nextWordDone16
					mov [ebx], 0x00

				nextWordDone16:
					pop edi
					pop esi
					pop ecx
					pop ebx
					pop eax
			}

			break;
		}
		default:
		{
			BruteForceEngine_CurrentWord[0] = NULL;
		}
	}

	return BruteForceEngine_CurrentWord;
}