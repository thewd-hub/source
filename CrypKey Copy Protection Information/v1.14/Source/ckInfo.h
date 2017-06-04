#include <windows.h>

#define KEYTYPE_UNKNOWNKEY					0
#define KEYTYPE_MASTERKEY					1
#define KEYTYPE_MASTERKEY6					2
#define KEYTYPE_USERKEY						3
#define KEYTYPE_SITECODE					4
#define KEYTYPE_SITECODE6					5
#define KEYTYPE_SITEKEY						6
#define KEYTYPE_SITEKEY6					7
#define KEYTYPE_RESTRICTIONKEY				8
#define KEYTYPE_RESTRICTIONKEY6				9
#define KEYTYPE_ENTRYKEY					10
#define KEYTYPE_ENTRYKEY7					11
#define KEYTYPE_CONFIRMATIONCODE			12
#define KEYTYPE_HDSN						13
#define KEYTYPE_CUSTOMINFOBYTES				14
#define KEYTYPE_CEDLICENCE					15

#define ERROR_MASTERKEY_LENGTH				0
#define ERROR_USERKEY_LENGTH				1
#define ERROR_SITECODE_LENGTH				2
#define ERROR_SITECODE_CRCFAILED			3
#define ERROR_SITEKEY_LENGTH				4
#define ERROR_RESTRICTIONKEY_LENGTH			5
#define ERROR_ENTRYKEY_LENGTH				6
#define ERROR_CONFIRMATIONCODE_LENGTH		7
#define ERROR_REGISTEREXTENSION				8
#define ERROR_UNREGISTEREXTENSION			9
#define ERROR_SCRIPTFILE_PARSING			10
#define ERROR_SCRIPTFILE_OPEN				11
#define ERROR_SCRIPTFILE_UNSUPPORTED		12
#define ERROR_SCRIPTFILE_UNRECOGNISED		13
#define ERROR_SCRIPTFILE_REQUIREDNOTFOUND	14

#define ERROR_SITECODE6_DECRYPTION_FAILED	15
#define ERROR_SITEKEY6_DECRYPTION_FAILED	16

#define ERROR_DISKINFORMATION_FAILED		17
#define ERROR_DISKINFORMATION_VXDEXTRACT	18
#define ERROR_DISKINFORMATION_VXDFAILED		19

#define ERROR_CUSTOMINFOBYTES_FILENAME		20
#define ERROR_CUSTOMINFOBYTES_FILESIZE		21

#define ERROR_GENERALFILE_FILENAME			22

bool isDebugMode(void);
void displayErrorMessage(int errorNumber);
char* createPaddedStringFromValue(unsigned long dwValue, int maximumLength, char paddingCharacter, bool treatAsHexadecimal);
char* getParsedParameter(char* parameterName);
char* getParsedParameterForFormattedString(char* parameterName);
char* getCompanyNameFromAccountNumber(int accountNumber, int maximumLength);