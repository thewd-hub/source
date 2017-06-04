/////////////////////////////
// BruteForce Engine v1.46 //
/////////////////////////////

// engine class
class BruteForceEngine
{
	private:
		HANDLE BruteForceEngine_PrivateHeap;

		char* BruteForceEngine_ActualCharSet;
		int BruteForceEngine_ActualCharSetLength;

		int BruteForceEngine_CurrentWordLength;
		char* BruteForceEngine_CurrentWord;
		unsigned char* BruteForceEngine_IndexCounters;
		unsigned char BruteForceEngine_FirstCharacter;

	public:
		void initialiseCharacterSet(bool useLowercase, bool useUppercase, bool useNumbers, bool useSpecial);
		void initialiseCharacterSet(char* customCharacterSet);
		void destroyCharacterSet(void);
		int getCharacterSetLength(void);

		void setCurrentWordLength(int wordLength); // 1 to 16 characters
		int getCurrentWordLength(void);
		double getPercentageDone(void);

		char* getNextWord(void);
};