///////////////////////////////////////////////////////////////////////////
// Generate words of length 1 to INTELLIFORCEENGINE_MAX_WORD_LENGTH,     //
// based on tolerance levels, using lowercase & uppercase character sets //
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// Tolerance Levels 1 to 12                                              //
// 01 ... very restrictive                                               //
// 07 ... default tolerance                                              //
// 12 ... equivalent to brute force (but slower)                         //
///////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include "..\IntelliForceEngine.h"
#include "..\IntelliForceEngineSupport.h"

//////////////////////////////////////////////////////////////////////////
// creates the character set to be used in the word generation routines //
//////////////////////////////////////////////////////////////////////////
void IntelliForceEngine::initialiseCharacterSet(bool useLowercase, bool useUppercase)
{
	IntelliForceEngine_PrivateHeap = HeapCreate(NULL, 10000, 0);

	IntelliForceEngine_ActualCharSetLength = 0;
	if (useLowercase) IntelliForceEngine_ActualCharSetLength += LOWERCASE_CHARSET_LENGTH;
	if (useUppercase) IntelliForceEngine_ActualCharSetLength += UPPERCASE_CHARSET_LENGTH;

	IntelliForceEngine_ActualCharSet = (char*)HeapAlloc(IntelliForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, IntelliForceEngine_ActualCharSetLength + 1);
	IntelliForceEngine_CurrentWord = (char*)HeapAlloc(IntelliForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);
	IntelliForceEngine_IndexCounters = (unsigned char*)HeapAlloc(IntelliForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);

	int loopIndex = 0;
	int charSetIndex = 0;

	if (useLowercase) for (loopIndex = 0; loopIndex < LOWERCASE_CHARSET_LENGTH; loopIndex++) IntelliForceEngine_ActualCharSet[charSetIndex++] = LOWERCASE_CHARSET[loopIndex];
	if (useUppercase) for (loopIndex = 0; loopIndex < UPPERCASE_CHARSET_LENGTH; loopIndex++) IntelliForceEngine_ActualCharSet[charSetIndex++] = UPPERCASE_CHARSET[loopIndex];

	IntelliForceEngine_FirstCharacter = IntelliForceEngine_ActualCharSet[0];
}

//////////////////////////////////////////////////////////////////////////
// creates the character set to be used in the word generation routines //
//////////////////////////////////////////////////////////////////////////
void IntelliForceEngine::initialiseCharacterSet(char* customCharacterSet)
{
	IntelliForceEngine_PrivateHeap = HeapCreate(NULL, 10000, 0);

	IntelliForceEngine_ActualCharSetLength = (int)strlen(customCharacterSet);
	IntelliForceEngine_ActualCharSet = (char*)HeapAlloc(IntelliForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, IntelliForceEngine_ActualCharSetLength + 1);
	IntelliForceEngine_CurrentWord = (char*)HeapAlloc(IntelliForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);
	IntelliForceEngine_IndexCounters = (unsigned char*)HeapAlloc(IntelliForceEngine_PrivateHeap, HEAP_ZERO_MEMORY, 100);

	for (int loopIndex = 0; loopIndex < IntelliForceEngine_ActualCharSetLength; loopIndex++) IntelliForceEngine_ActualCharSet[loopIndex] = customCharacterSet[loopIndex];

	IntelliForceEngine_FirstCharacter = IntelliForceEngine_ActualCharSet[0];
}

///////////////////////////////////////////////////////////
// cleans up the variable state when completely finsihed //
///////////////////////////////////////////////////////////
void IntelliForceEngine::destroyCharacterSet(void)
{
	HeapFree(IntelliForceEngine_PrivateHeap, NULL, IntelliForceEngine_IndexCounters);
	HeapFree(IntelliForceEngine_PrivateHeap, NULL, IntelliForceEngine_CurrentWord);
	HeapFree(IntelliForceEngine_PrivateHeap, NULL, IntelliForceEngine_ActualCharSet);
	HeapDestroy(IntelliForceEngine_PrivateHeap);
}

/////////////////////////////////////////////////////
// returns the length of the current character set //
/////////////////////////////////////////////////////
int IntelliForceEngine::getCharacterSetLength(void)
{
	return IntelliForceEngine_ActualCharSetLength;
}

/////////////////////////////////////////////////////////
// sets the tolerance level in which the IntelliForce  //
// engine is to operate at. Default is: LEVEL 07       //
// The lower the tolerance level, the more restrictive //
// the selection process is. As this level increases   //
// more words are allowed by the selection process     //
/////////////////////////////////////////////////////////
void IntelliForceEngine::setToleranceLevel(int toleranceLevel)
{
	switch (toleranceLevel)
	{
		case 1: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_01; break;
		case 2: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_02; break;
		case 3: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_03; break;
		case 4: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_04; break;
		case 5: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_05; break;
		case 6: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_06; break;
		case 7: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_07; break;
		case 8: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_08; break;
		case 9: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_09; break;
		case 10: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_10; break;
		case 11: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_11; break;
		case 12: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_12; break;
		default: IntelliForceEngine_CurrentToleranceLevel = INTELLIFORCEENGINE_TOLERANCE_LEVEL_07; break;
	}
}

///////////////////////////////////////////////////////////
// set the new word length of the words to be generated  //
// Resets all index counters and starts generating words //
///////////////////////////////////////////////////////////
void IntelliForceEngine::setCurrentWordLength(int wordLength)
{
	ZeroMemory(IntelliForceEngine_IndexCounters, INTELLIFORCEENGINE_MAX_WORD_LENGTH);
	ZeroMemory(IntelliForceEngine_CurrentWord, INTELLIFORCEENGINE_MAX_WORD_LENGTH);

	IntelliForceEngine_CurrentWordLength = ((wordLength > INTELLIFORCEENGINE_MAX_WORD_LENGTH) ? INTELLIFORCEENGINE_MAX_WORD_LENGTH : wordLength);
	for (int i = 0; i < IntelliForceEngine_CurrentWordLength; i++) IntelliForceEngine_CurrentWord[i] = IntelliForceEngine_FirstCharacter;
}

/////////////////////////////////////////////////////////////
// returns the length of the current words to be generated //
/////////////////////////////////////////////////////////////
int IntelliForceEngine::getCurrentWordLength(void)
{
	return IntelliForceEngine_CurrentWordLength;
}

///////////////////////////////////////////////
// returns the approximate percentage of the //
// work done for the current word length     //
///////////////////////////////////////////////
double IntelliForceEngine::getPercentageDone(void)
{
	return (((double)IntelliForceEngine_IndexCounters[0] / (double)IntelliForceEngine_ActualCharSetLength) * 100);
}

/////////////////////////////////////////////////////
// returns the next word in the character set that //
// satisfies the tolerance level (a valid word)    //
/////////////////////////////////////////////////////
char* IntelliForceEngine::getNextWord(void)
{
	char* IntelliForceEngine_ActualCharSet = this->IntelliForceEngine_ActualCharSet;
	int IntelliForceEngine_ActualCharSetLength = this->IntelliForceEngine_ActualCharSetLength;

	int IntelliForceEngine_CurrentWordLength = this->IntelliForceEngine_CurrentWordLength;
	char* IntelliForceEngine_CurrentWord = this->IntelliForceEngine_CurrentWord;
	unsigned char* IntelliForceEngine_IndexCounters = this->IntelliForceEngine_IndexCounters;

	switch (IntelliForceEngine_CurrentWordLength)
	{
		// 1 character (no probability distribution checking)
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
					mov ebx, IntelliForceEngine_CurrentWord
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate01
					inc [esi + 0]
					jmp nextWordDone01

				nextWordTerminate01:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart02_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate02

				nextWordStart02_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck02_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart02_01

				nextWordCheck02_02:
					// check probability distribution (1 & 2)
					inc [esi + 1]
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone02
					jmp nextWordStart02_02

				nextWordTerminate02:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart03_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate03

				nextWordStart03_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck03_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart03_01

				nextWordCheck03_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart03_03
					inc [esi + 1]
					jmp nextWordStart03_02

				nextWordStart03_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck03_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart03_02

				nextWordCheck03_03:
					// check probability distribution (2 & 3)
					inc [esi + 2]
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone03
					jmp nextWordStart03_03

				nextWordTerminate03:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart04_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate04

				nextWordStart04_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck04_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart04_01

				nextWordCheck04_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart04_03
					inc [esi + 1]
					jmp nextWordStart04_02

				nextWordStart04_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck04_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart04_02

				nextWordCheck04_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart04_04
					inc [esi + 2]
					jmp nextWordStart04_03

				nextWordStart04_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck04_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart04_03

				nextWordCheck04_04:
					// check probability distribution (3 & 4)
					inc [esi + 3]
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone04
					jmp nextWordStart04_04

				nextWordTerminate04:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart05_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate05

				nextWordStart05_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck05_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart05_01

				nextWordCheck05_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart05_03
					inc [esi + 1]
					jmp nextWordStart05_02

				nextWordStart05_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck05_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart05_02

				nextWordCheck05_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart05_04
					inc [esi + 2]
					jmp nextWordStart05_03

				nextWordStart05_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck05_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart05_03

				nextWordCheck05_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart05_05
					inc [esi + 3]
					jmp nextWordStart05_04

				nextWordStart05_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck05_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart05_04

				nextWordCheck05_05:
					// check probability distribution (4 & 5)
					inc [esi + 4]
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone05
					jmp nextWordStart05_05

				nextWordTerminate05:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart06_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate06

				nextWordStart06_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck06_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart06_01

				nextWordCheck06_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart06_03
					inc [esi + 1]
					jmp nextWordStart06_02

				nextWordStart06_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck06_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart06_02

				nextWordCheck06_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart06_04
					inc [esi + 2]
					jmp nextWordStart06_03

				nextWordStart06_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck06_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart06_03

				nextWordCheck06_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart06_05
					inc [esi + 3]
					jmp nextWordStart06_04

				nextWordStart06_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck06_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart06_04

				nextWordCheck06_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart06_06
					inc [esi + 4]
					jmp nextWordStart06_05

				nextWordStart06_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck06_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart06_05

				nextWordCheck06_06:
					// check probability distribution (5 & 6)
					inc [esi + 5]
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone06
					jmp nextWordStart06_06

				nextWordTerminate06:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart07_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate07

				nextWordStart07_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck07_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart07_01

				nextWordCheck07_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart07_03
					inc [esi + 1]
					jmp nextWordStart07_02

				nextWordStart07_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck07_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart07_02

				nextWordCheck07_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart07_04
					inc [esi + 2]
					jmp nextWordStart07_03

				nextWordStart07_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck07_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart07_03

				nextWordCheck07_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart07_05
					inc [esi + 3]
					jmp nextWordStart07_04

				nextWordStart07_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck07_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart07_04

				nextWordCheck07_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart07_06
					inc [esi + 4]
					jmp nextWordStart07_05

				nextWordStart07_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck07_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart07_05

				nextWordCheck07_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart07_07
					inc [esi + 5]
					jmp nextWordStart07_06

				nextWordStart07_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck07_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart07_06

				nextWordCheck07_07:
					// check probability distribution (6 & 7)
					inc [esi + 6]
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone07
					jmp nextWordStart07_07

				nextWordTerminate07:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart08_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate08

				nextWordStart08_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck08_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart08_01

				nextWordCheck08_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart08_03
					inc [esi + 1]
					jmp nextWordStart08_02

				nextWordStart08_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck08_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart08_02

				nextWordCheck08_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart08_04
					inc [esi + 2]
					jmp nextWordStart08_03

				nextWordStart08_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck08_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart08_03

				nextWordCheck08_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart08_05
					inc [esi + 3]
					jmp nextWordStart08_04

				nextWordStart08_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck08_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart08_04

				nextWordCheck08_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart08_06
					inc [esi + 4]
					jmp nextWordStart08_05

				nextWordStart08_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck08_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart08_05

				nextWordCheck08_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart08_07
					inc [esi + 5]
					jmp nextWordStart08_06

				nextWordStart08_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck08_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart08_06

				nextWordCheck08_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart08_08
					inc [esi + 6]
					jmp nextWordStart08_07

				nextWordStart08_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck08_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart08_07

				nextWordCheck08_08:
					// check probability distribution (7 & 8)
					inc [esi + 7]
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone08
					jmp nextWordStart08_08

				nextWordTerminate08:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart09_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate09

				nextWordStart09_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck09_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart09_01

				nextWordCheck09_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_03
					inc [esi + 1]
					jmp nextWordStart09_02

				nextWordStart09_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck09_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart09_02

				nextWordCheck09_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_04
					inc [esi + 2]
					jmp nextWordStart09_03

				nextWordStart09_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck09_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart09_03

				nextWordCheck09_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_05
					inc [esi + 3]
					jmp nextWordStart09_04

				nextWordStart09_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck09_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart09_04

				nextWordCheck09_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_06
					inc [esi + 4]
					jmp nextWordStart09_05

				nextWordStart09_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck09_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart09_05

				nextWordCheck09_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_07
					inc [esi + 5]
					jmp nextWordStart09_06

				nextWordStart09_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck09_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart09_06

				nextWordCheck09_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_08
					inc [esi + 6]
					jmp nextWordStart09_07

				nextWordStart09_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck09_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart09_07

				nextWordCheck09_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart09_09
					inc [esi + 7]
					jmp nextWordStart09_08

				nextWordStart09_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck09_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart09_08

				nextWordCheck09_09:
					// check probability distribution (8 & 9)
					inc [esi + 8]
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone09
					jmp nextWordStart09_09

				nextWordTerminate09:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart10_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate10

				nextWordStart10_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck10_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart10_01

				nextWordCheck10_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_03
					inc [esi + 1]
					jmp nextWordStart10_02

				nextWordStart10_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck10_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart10_02

				nextWordCheck10_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_04
					inc [esi + 2]
					jmp nextWordStart10_03

				nextWordStart10_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck10_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart10_03

				nextWordCheck10_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_05
					inc [esi + 3]
					jmp nextWordStart10_04

				nextWordStart10_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck10_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart10_04

				nextWordCheck10_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_06
					inc [esi + 4]
					jmp nextWordStart10_05

				nextWordStart10_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck10_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart10_05

				nextWordCheck10_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_07
					inc [esi + 5]
					jmp nextWordStart10_06

				nextWordStart10_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck10_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart10_06

				nextWordCheck10_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_08
					inc [esi + 6]
					jmp nextWordStart10_07

				nextWordStart10_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck10_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart10_07

				nextWordCheck10_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_09
					inc [esi + 7]
					jmp nextWordStart10_08

				nextWordStart10_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck10_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart10_08

				nextWordCheck10_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart10_10
					inc [esi + 8]
					jmp nextWordStart10_09

				nextWordStart10_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck10_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart10_09

				nextWordCheck10_10:
					// check probability distribution (9 & 10)
					inc [esi + 9]
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone10
					jmp nextWordStart10_10

				nextWordTerminate10:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart11_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate11

				nextWordStart11_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck11_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart11_01

				nextWordCheck11_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_03
					inc [esi + 1]
					jmp nextWordStart11_02

				nextWordStart11_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck11_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart11_02

				nextWordCheck11_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_04
					inc [esi + 2]
					jmp nextWordStart11_03

				nextWordStart11_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck11_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart11_03

				nextWordCheck11_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_05
					inc [esi + 3]
					jmp nextWordStart11_04

				nextWordStart11_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck11_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart11_04

				nextWordCheck11_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_06
					inc [esi + 4]
					jmp nextWordStart11_05

				nextWordStart11_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck11_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart11_05

				nextWordCheck11_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_07
					inc [esi + 5]
					jmp nextWordStart11_06

				nextWordStart11_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck11_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart11_06

				nextWordCheck11_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_08
					inc [esi + 6]
					jmp nextWordStart11_07

				nextWordStart11_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck11_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart11_07

				nextWordCheck11_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_09
					inc [esi + 7]
					jmp nextWordStart11_08

				nextWordStart11_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck11_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart11_08

				nextWordCheck11_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_10
					inc [esi + 8]
					jmp nextWordStart11_09

				nextWordStart11_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck11_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart11_09

				nextWordCheck11_10:
					// check probability distribution (9 & 10)
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart11_11
					inc [esi + 9]
					jmp nextWordStart11_10

				nextWordStart11_11:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 11
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordCheck11_11
					mov [esi + 10], 0x00
					inc [esi + 9]
					jmp nextWordStart11_10

				nextWordCheck11_11:
					// check probability distribution (10 & 11)
					inc [esi + 10]
					mov al, [ebx + 9]
					mov cl, [ebx + 10]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone11
					jmp nextWordStart11_11

				nextWordTerminate11:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart12_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate12

				nextWordStart12_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck12_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart12_01

				nextWordCheck12_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_03
					inc [esi + 1]
					jmp nextWordStart12_02

				nextWordStart12_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck12_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart12_02

				nextWordCheck12_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_04
					inc [esi + 2]
					jmp nextWordStart12_03

				nextWordStart12_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck12_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart12_03

				nextWordCheck12_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_05
					inc [esi + 3]
					jmp nextWordStart12_04

				nextWordStart12_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck12_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart12_04

				nextWordCheck12_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_06
					inc [esi + 4]
					jmp nextWordStart12_05

				nextWordStart12_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck12_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart12_05

				nextWordCheck12_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_07
					inc [esi + 5]
					jmp nextWordStart12_06

				nextWordStart12_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck12_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart12_06

				nextWordCheck12_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_08
					inc [esi + 6]
					jmp nextWordStart12_07

				nextWordStart12_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck12_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart12_07

				nextWordCheck12_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_09
					inc [esi + 7]
					jmp nextWordStart12_08

				nextWordStart12_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck12_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart12_08

				nextWordCheck12_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_10
					inc [esi + 8]
					jmp nextWordStart12_09

				nextWordStart12_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck12_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart12_09

				nextWordCheck12_10:
					// check probability distribution (9 & 10)
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_11
					inc [esi + 9]
					jmp nextWordStart12_10

				nextWordStart12_11:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 11
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordCheck12_11
					mov [esi + 10], 0x00
					inc [esi + 9]
					jmp nextWordStart12_10

				nextWordCheck12_11:
					// check probability distribution (10 & 11)
					mov al, [ebx + 9]
					mov cl, [ebx + 10]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart12_12
					inc [esi + 10]
					jmp nextWordStart12_11

				nextWordStart12_12:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 12
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordCheck12_12
					mov [esi + 11], 0x00
					inc [esi + 10]
					jmp nextWordStart12_11

				nextWordCheck12_12:
					// check probability distribution (11 & 12)
					inc [esi + 11]
					mov al, [ebx + 10]
					mov cl, [ebx + 11]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone12
					jmp nextWordStart12_12

				nextWordTerminate12:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart13_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate13

				nextWordStart13_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck13_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart13_01

				nextWordCheck13_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_03
					inc [esi + 1]
					jmp nextWordStart13_02

				nextWordStart13_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck13_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart13_02

				nextWordCheck13_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_04
					inc [esi + 2]
					jmp nextWordStart13_03

				nextWordStart13_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck13_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart13_03

				nextWordCheck13_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_05
					inc [esi + 3]
					jmp nextWordStart13_04

				nextWordStart13_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck13_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart13_04

				nextWordCheck13_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_06
					inc [esi + 4]
					jmp nextWordStart13_05

				nextWordStart13_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck13_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart13_05

				nextWordCheck13_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_07
					inc [esi + 5]
					jmp nextWordStart13_06

				nextWordStart13_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck13_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart13_06

				nextWordCheck13_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_08
					inc [esi + 6]
					jmp nextWordStart13_07

				nextWordStart13_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck13_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart13_07

				nextWordCheck13_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_09
					inc [esi + 7]
					jmp nextWordStart13_08

				nextWordStart13_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck13_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart13_08

				nextWordCheck13_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_10
					inc [esi + 8]
					jmp nextWordStart13_09

				nextWordStart13_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck13_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart13_09

				nextWordCheck13_10:
					// check probability distribution (9 & 10)
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_11
					inc [esi + 9]
					jmp nextWordStart13_10

				nextWordStart13_11:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 11
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordCheck13_11
					mov [esi + 10], 0x00
					inc [esi + 9]
					jmp nextWordStart13_10

				nextWordCheck13_11:
					// check probability distribution (10 & 11)
					mov al, [ebx + 9]
					mov cl, [ebx + 10]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_12
					inc [esi + 10]
					jmp nextWordStart13_11

				nextWordStart13_12:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 12
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordCheck13_12
					mov [esi + 11], 0x00
					inc [esi + 10]
					jmp nextWordStart13_11

				nextWordCheck13_12:
					// check probability distribution (11 & 12)
					mov al, [ebx + 10]
					mov cl, [ebx + 11]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart13_13
					inc [esi + 11]
					jmp nextWordStart13_12

				nextWordStart13_13:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 13
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordCheck13_13
					mov [esi + 12], 0x00
					inc [esi + 11]
					jmp nextWordStart13_12

				nextWordCheck13_13:
					// check probability distribution (12 & 13)
					inc [esi + 12]
					mov al, [ebx + 11]
					mov cl, [ebx + 12]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone13
					jmp nextWordStart13_13

				nextWordTerminate13:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart14_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate14

				nextWordStart14_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck14_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart14_01

				nextWordCheck14_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_03
					inc [esi + 1]
					jmp nextWordStart14_02

				nextWordStart14_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck14_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart14_02

				nextWordCheck14_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_04
					inc [esi + 2]
					jmp nextWordStart14_03

				nextWordStart14_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck14_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart14_03

				nextWordCheck14_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_05
					inc [esi + 3]
					jmp nextWordStart14_04

				nextWordStart14_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck14_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart14_04

				nextWordCheck14_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_06
					inc [esi + 4]
					jmp nextWordStart14_05

				nextWordStart14_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck14_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart14_05

				nextWordCheck14_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_07
					inc [esi + 5]
					jmp nextWordStart14_06

				nextWordStart14_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck14_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart14_06

				nextWordCheck14_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_08
					inc [esi + 6]
					jmp nextWordStart14_07

				nextWordStart14_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck14_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart14_07

				nextWordCheck14_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_09
					inc [esi + 7]
					jmp nextWordStart14_08

				nextWordStart14_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck14_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart14_08

				nextWordCheck14_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_10
					inc [esi + 8]
					jmp nextWordStart14_09

				nextWordStart14_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck14_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart14_09

				nextWordCheck14_10:
					// check probability distribution (9 & 10)
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_11
					inc [esi + 9]
					jmp nextWordStart14_10

				nextWordStart14_11:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 11
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordCheck14_11
					mov [esi + 10], 0x00
					inc [esi + 9]
					jmp nextWordStart14_10

				nextWordCheck14_11:
					// check probability distribution (10 & 11)
					mov al, [ebx + 9]
					mov cl, [ebx + 10]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_12
					inc [esi + 10]
					jmp nextWordStart14_11

				nextWordStart14_12:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 12
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordCheck14_12
					mov [esi + 11], 0x00
					inc [esi + 10]
					jmp nextWordStart14_11

				nextWordCheck14_12:
					// check probability distribution (11 & 12)
					mov al, [ebx + 10]
					mov cl, [ebx + 11]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_13
					inc [esi + 11]
					jmp nextWordStart14_12

				nextWordStart14_13:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 13
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordCheck14_13
					mov [esi + 12], 0x00
					inc [esi + 11]
					jmp nextWordStart14_12

				nextWordCheck14_13:
					// check probability distribution (12 & 13)
					mov al, [ebx + 11]
					mov cl, [ebx + 12]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart14_14
					inc [esi + 12]
					jmp nextWordStart14_13

				nextWordStart14_14:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 14
					mov al, [esi + 13]
					mov dl, [edi + eax]
					mov [ebx + 13], dl
					cmp [esi + 13], cl
					jl nextWordCheck14_14
					mov [esi + 13], 0x00
					inc [esi + 12]
					jmp nextWordStart14_13

				nextWordCheck14_14:
					// check probability distribution (13 & 14)
					inc [esi + 13]
					mov al, [ebx + 12]
					mov cl, [ebx + 13]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone14
					jmp nextWordStart14_14

				nextWordTerminate14:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart15_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate15

				nextWordStart15_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck15_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart15_01

				nextWordCheck15_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_03
					inc [esi + 1]
					jmp nextWordStart15_02

				nextWordStart15_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck15_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart15_02

				nextWordCheck15_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_04
					inc [esi + 2]
					jmp nextWordStart15_03

				nextWordStart15_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck15_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart15_03

				nextWordCheck15_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_05
					inc [esi + 3]
					jmp nextWordStart15_04

				nextWordStart15_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck15_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart15_04

				nextWordCheck15_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_06
					inc [esi + 4]
					jmp nextWordStart15_05

				nextWordStart15_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck15_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart15_05

				nextWordCheck15_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_07
					inc [esi + 5]
					jmp nextWordStart15_06

				nextWordStart15_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck15_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart15_06

				nextWordCheck15_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_08
					inc [esi + 6]
					jmp nextWordStart15_07

				nextWordStart15_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck15_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart15_07

				nextWordCheck15_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_09
					inc [esi + 7]
					jmp nextWordStart15_08

				nextWordStart15_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck15_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart15_08

				nextWordCheck15_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_10
					inc [esi + 8]
					jmp nextWordStart15_09

				nextWordStart15_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck15_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart15_09

				nextWordCheck15_10:
					// check probability distribution (9 & 10)
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_11
					inc [esi + 9]
					jmp nextWordStart15_10

				nextWordStart15_11:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 11
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordCheck15_11
					mov [esi + 10], 0x00
					inc [esi + 9]
					jmp nextWordStart15_10

				nextWordCheck15_11:
					// check probability distribution (10 & 11)
					mov al, [ebx + 9]
					mov cl, [ebx + 10]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_12
					inc [esi + 10]
					jmp nextWordStart15_11

				nextWordStart15_12:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 12
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordCheck15_12
					mov [esi + 11], 0x00
					inc [esi + 10]
					jmp nextWordStart15_11

				nextWordCheck15_12:
					// check probability distribution (11 & 12)
					mov al, [ebx + 10]
					mov cl, [ebx + 11]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_13
					inc [esi + 11]
					jmp nextWordStart15_12

				nextWordStart15_13:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 13
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordCheck15_13
					mov [esi + 12], 0x00
					inc [esi + 11]
					jmp nextWordStart15_12

				nextWordCheck15_13:
					// check probability distribution (12 & 13)
					mov al, [ebx + 11]
					mov cl, [ebx + 12]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_14
					inc [esi + 12]
					jmp nextWordStart15_13

				nextWordStart15_14:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 14
					mov al, [esi + 13]
					mov dl, [edi + eax]
					mov [ebx + 13], dl
					cmp [esi + 13], cl
					jl nextWordCheck15_14
					mov [esi + 13], 0x00
					inc [esi + 12]
					jmp nextWordStart15_13

				nextWordCheck15_14:
					// check probability distribution (13 & 14)
					mov al, [ebx + 12]
					mov cl, [ebx + 13]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart15_15
					inc [esi + 13]
					jmp nextWordStart15_14

				nextWordStart15_15:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 15
					mov al, [esi + 14]
					mov dl, [edi + eax]
					mov [ebx + 14], dl
					cmp [esi + 14], cl
					jl nextWordCheck15_15
					mov [esi + 14], 0x00
					inc [esi + 13]
					jmp nextWordStart15_14

				nextWordCheck15_15:
					// check probability distribution (14 & 15)
					inc [esi + 14]
					mov al, [ebx + 13]
					mov cl, [ebx + 14]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone15
					jmp nextWordStart15_15

				nextWordTerminate15:
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

					mov ebx, IntelliForceEngine_CurrentWord
					mov esi, IntelliForceEngine_IndexCounters
					mov edi, IntelliForceEngine_ActualCharSet

				nextWordStart16_01:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 1
					mov al, [esi + 0]
					mov dl, [edi + eax]
					mov [ebx + 0], dl
					cmp [esi + 0], cl
					jge nextWordTerminate16

				nextWordStart16_02:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 2
					mov al, [esi + 1]
					mov dl, [edi + eax]
					mov [ebx + 1], dl
					cmp [esi + 1], cl
					jl nextWordCheck16_02
					mov [esi + 1], 0x00
					inc [esi + 0]
					jmp nextWordStart16_01

				nextWordCheck16_02:
					// check probability distribution (1 & 2)
					mov al, [ebx + 0]
					mov cl, [ebx + 1]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_03
					inc [esi + 1]
					jmp nextWordStart16_02

				nextWordStart16_03:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 3
					mov al, [esi + 2]
					mov dl, [edi + eax]
					mov [ebx + 2], dl
					cmp [esi + 2], cl
					jl nextWordCheck16_03
					mov [esi + 2], 0x00
					inc [esi + 1]
					jmp nextWordStart16_02

				nextWordCheck16_03:
					// check probability distribution (2 & 3)
					mov al, [ebx + 1]
					mov cl, [ebx + 2]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_04
					inc [esi + 2]
					jmp nextWordStart16_03

				nextWordStart16_04:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 4
					mov al, [esi + 3]
					mov dl, [edi + eax]
					mov [ebx + 3], dl
					cmp [esi + 3], cl
					jl nextWordCheck16_04
					mov [esi + 3], 0x00
					inc [esi + 2]
					jmp nextWordStart16_03

				nextWordCheck16_04:
					// check probability distribution (3 & 4)
					mov al, [ebx + 2]
					mov cl, [ebx + 3]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_05
					inc [esi + 3]
					jmp nextWordStart16_04

				nextWordStart16_05:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 5
					mov al, [esi + 4]
					mov dl, [edi + eax]
					mov [ebx + 4], dl
					cmp [esi + 4], cl
					jl nextWordCheck16_05
					mov [esi + 4], 0x00
					inc [esi + 3]
					jmp nextWordStart16_04

				nextWordCheck16_05:
					// check probability distribution (4 & 5)
					mov al, [ebx + 3]
					mov cl, [ebx + 4]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_06
					inc [esi + 4]
					jmp nextWordStart16_05

				nextWordStart16_06:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 6
					mov al, [esi + 5]
					mov dl, [edi + eax]
					mov [ebx + 5], dl
					cmp [esi + 5], cl
					jl nextWordCheck16_06
					mov [esi + 5], 0x00
					inc [esi + 4]
					jmp nextWordStart16_05

				nextWordCheck16_06:
					// check probability distribution (5 & 6)
					mov al, [ebx + 4]
					mov cl, [ebx + 5]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_07
					inc [esi + 5]
					jmp nextWordStart16_06

				nextWordStart16_07:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 7
					mov al, [esi + 6]
					mov dl, [edi + eax]
					mov [ebx + 6], dl
					cmp [esi + 6], cl
					jl nextWordCheck16_07
					mov [esi + 6], 0x00
					inc [esi + 5]
					jmp nextWordStart16_06

				nextWordCheck16_07:
					// check probability distribution (6 & 7)
					mov al, [ebx + 5]
					mov cl, [ebx + 6]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_08
					inc [esi + 6]
					jmp nextWordStart16_07

				nextWordStart16_08:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 8
					mov al, [esi + 7]
					mov dl, [edi + eax]
					mov [ebx + 7], dl
					cmp [esi + 7], cl
					jl nextWordCheck16_08
					mov [esi + 7], 0x00
					inc [esi + 6]
					jmp nextWordStart16_07

				nextWordCheck16_08:
					// check probability distribution (7 & 8)
					mov al, [ebx + 6]
					mov cl, [ebx + 7]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_09
					inc [esi + 7]
					jmp nextWordStart16_08

				nextWordStart16_09:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 9
					mov al, [esi + 8]
					mov dl, [edi + eax]
					mov [ebx + 8], dl
					cmp [esi + 8], cl
					jl nextWordCheck16_09
					mov [esi + 8], 0x00
					inc [esi + 7]
					jmp nextWordStart16_08

				nextWordCheck16_09:
					// check probability distribution (8 & 9)
					mov al, [ebx + 7]
					mov cl, [ebx + 8]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_10
					inc [esi + 8]
					jmp nextWordStart16_09

				nextWordStart16_10:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 10
					mov al, [esi + 9]
					mov dl, [edi + eax]
					mov [ebx + 9], dl
					cmp [esi + 9], cl
					jl nextWordCheck16_10
					mov [esi + 9], 0x00
					inc [esi + 8]
					jmp nextWordStart16_09

				nextWordCheck16_10:
					// check probability distribution (9 & 10)
					mov al, [ebx + 8]
					mov cl, [ebx + 9]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_11
					inc [esi + 9]
					jmp nextWordStart16_10

				nextWordStart16_11:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 11
					mov al, [esi + 10]
					mov dl, [edi + eax]
					mov [ebx + 10], dl
					cmp [esi + 10], cl
					jl nextWordCheck16_11
					mov [esi + 10], 0x00
					inc [esi + 9]
					jmp nextWordStart16_10

				nextWordCheck16_11:
					// check probability distribution (10 & 11)
					mov al, [ebx + 9]
					mov cl, [ebx + 10]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_12
					inc [esi + 10]
					jmp nextWordStart16_11

				nextWordStart16_12:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 12
					mov al, [esi + 11]
					mov dl, [edi + eax]
					mov [ebx + 11], dl
					cmp [esi + 11], cl
					jl nextWordCheck16_12
					mov [esi + 11], 0x00
					inc [esi + 10]
					jmp nextWordStart16_11

				nextWordCheck16_12:
					// check probability distribution (11 & 12)
					mov al, [ebx + 10]
					mov cl, [ebx + 11]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_13
					inc [esi + 11]
					jmp nextWordStart16_12

				nextWordStart16_13:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 13
					mov al, [esi + 12]
					mov dl, [edi + eax]
					mov [ebx + 12], dl
					cmp [esi + 12], cl
					jl nextWordCheck16_13
					mov [esi + 12], 0x00
					inc [esi + 11]
					jmp nextWordStart16_12

				nextWordCheck16_13:
					// check probability distribution (12 & 13)
					mov al, [ebx + 11]
					mov cl, [ebx + 12]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_14
					inc [esi + 12]
					jmp nextWordStart16_13

				nextWordStart16_14:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 14
					mov al, [esi + 13]
					mov dl, [edi + eax]
					mov [ebx + 13], dl
					cmp [esi + 13], cl
					jl nextWordCheck16_14
					mov [esi + 13], 0x00
					inc [esi + 12]
					jmp nextWordStart16_13

				nextWordCheck16_14:
					// check probability distribution (13 & 14)
					mov al, [ebx + 12]
					mov cl, [ebx + 13]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_15
					inc [esi + 13]
					jmp nextWordStart16_14

				nextWordStart16_15:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 15
					mov al, [esi + 14]
					mov dl, [edi + eax]
					mov [ebx + 14], dl
					cmp [esi + 14], cl
					jl nextWordCheck16_15
					mov [esi + 14], 0x00
					inc [esi + 13]
					jmp nextWordStart16_14

				nextWordCheck16_15:
					// check probability distribution (14 & 15)
					mov al, [ebx + 13]
					mov cl, [ebx + 14]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordStart16_16
					inc [esi + 14]
					jmp nextWordStart16_15

				nextWordStart16_16:
					xor eax, eax
					mov ecx, [IntelliForceEngine_ActualCharSetLength]
					// increment character 16
					mov al, [esi + 15]
					mov dl, [edi + eax]
					mov [ebx + 15], dl
					cmp [esi + 15], cl
					jl nextWordCheck16_16
					mov [esi + 15], 0x00
					inc [esi + 14]
					jmp nextWordStart16_15

				nextWordCheck16_16:
					// check probability distribution (15 & 16)
					inc [esi + 15]
					mov al, [ebx + 14]
					mov cl, [ebx + 15]
					sub al, 0x41
					mov edx, [ProbabilityDistributionTableDimensionSize]
					mul edx
					lea edx, [edx + eax]
					lea edx, [edx + ecx - 0x41]
					fld qword ptr [ProbabilityDistributionTable + edx * 8]
					fcomp qword ptr [IntelliForceEngine_CurrentToleranceLevel]
					fstsw ax
					test ah, 0x01
					je nextWordDone16
					jmp nextWordStart16_16

				nextWordTerminate16:
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
			IntelliForceEngine_CurrentWord[0] = NULL;
		}
	}

	return IntelliForceEngine_CurrentWord;
}