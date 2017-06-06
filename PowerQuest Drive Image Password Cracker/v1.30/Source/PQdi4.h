//////////////////////////////////////////////////
// PowerQuest Drive Image Password Cracker      //
// (C)thewd, thewd@hotmail.com                  //
//////////////////////////////////////////////////

#define SEARCH_UNKNOWN			0
#define SEARCH_BRUTEFORCE		1
#define SEARCH_INTELLIFORCE		2
#define SEARCH_DICTIONARY		3

void setSearchProperties(int searchMethod, int startLength, int endLength, int toleranceLevel, bool useLowercase,
						 bool useUppercase, bool useNumbers, bool useSpecial, char* dictionaryName);
char* findPasswordv4(unsigned char* passwordCRC);