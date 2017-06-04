/////////////////////////////
// Dictionary Engine v1.12 //
/////////////////////////////

#define DICTIONARYENGINE_FILTER_DEFAULT			0x0001
#define DICTIONARYENGINE_FILTER_LOWERCASE		0x0002
#define DICTIONARYENGINE_FILTER_UPPERCASE		0x0004
#define DICTIONARYENGINE_FILTER_CAPITALISE		0x0008
#define DICTIONARYENGINE_FILTER_ALL				DICTIONARYENGINE_FILTER_DEFAULT | \
												DICTIONARYENGINE_FILTER_LOWERCASE | \
												DICTIONARYENGINE_FILTER_UPPERCASE | \
												DICTIONARYENGINE_FILTER_CAPITALISE

class DictionaryEngine
{
	public:
		bool initialiseDictionaryEngine(char* dictionaryFilename);
		bool initialiseDictionaryEngine(char* dictionaryFilename, unsigned long dictionaryFilter);
		bool initialiseDictionaryEngine(char* dictionaryFilename, unsigned long dictionaryFilter, int dictionaryWordLength);
		void destroyDictionaryEngine(void);

		void resetDictionaryEngine(void);

		char* getNextWord(unsigned long* dwNextWordLength);
};