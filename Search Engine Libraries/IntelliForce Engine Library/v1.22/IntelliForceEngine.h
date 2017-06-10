///////////////////////////////
// IntelliForce Engine v1.22 //
///////////////////////////////

class IntelliForceEngine
{
	private:
		HANDLE IntelliForceEngine_PrivateHeap;

		char* IntelliForceEngine_ActualCharSet;
		int IntelliForceEngine_ActualCharSetLength;

		int IntelliForceEngine_CurrentWordLength;
		char* IntelliForceEngine_CurrentWord;
		unsigned char* IntelliForceEngine_IndexCounters;
		unsigned char IntelliForceEngine_FirstCharacter;

	public:
		void initialiseCharacterSet(bool useLowercase, bool useUppercase);
		void initialiseCharacterSet(char* customCharacterSet);
		void destroyCharacterSet(void);
		int getCharacterSetLength(void);

		void setToleranceLevel(int toleranceLevel); // 1 to 12 levels (7 - default level)
		void setCurrentWordLength(int wordLength); // 1 to 16 characters
		int getCurrentWordLength(void);
		double getPercentageDone(void);

		char* getNextWord(void);
};