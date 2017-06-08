///////////////////////////////////////////////////////////
// Keyboard Input Simulator                              //
// (C)thewd, thewd@hotmail.com                           //
///////////////////////////////////////////////////////////

#define SCRIPTACTIONID_UNKNOWN			0
#define SCRIPTACTIONID_SEND				1
#define SCRIPTACTIONID_SENDWORD			2
#define SCRIPTACTIONID_SLEEP			3
#define SCRIPTACTIONID_BREAKIF			4
#define SCRIPTACTIONID_LABEL			5
#define SCRIPTACTIONID_INC				6
#define SCRIPTACTIONID_REPEATWHILE		7
#define SCRIPTACTIONID_REPEATUNTIL		8

#define SCRIPTACTIONID_SEND_CHARACTER		1
#define SCRIPTACTIONID_SEND_DICTIONARYWORD	2
#define SCRIPTACTIONID_SEND_TEMPLATEWORD	3

#define SCRIPTACTIONID_BREAKIF_WINDOWTITLE	1

#define SCRIPTACTIONID_INC_DICTIONARYWORD	1
#define SCRIPTACTIONID_INC_TEMPLATEWORD		2

#define SCRIPTACTIONID_REPEATWHILE_DICTIONARYWORD	1
#define SCRIPTACTIONID_REPEATWHILE_TEMPLATEWORD		2

#define SCRIPTACTIONID_REPEATUNTIL_COUNT			1

typedef struct _INPUTSCRIPTACTION
{
	int actionId;
	void* actionData;
} INPUTSCRIPTACTION;

typedef struct _INPUTSCRIPTACTION_SEND
{
	int actionId;
	char* strCode;
	short vkCode;
	short scanCode;
	unsigned long dwFlags;
} INPUTSCRIPTACTION_SEND;

typedef struct _INPUTSCRIPTACTION_SLEEP
{
	unsigned long sleepDuration;
} INPUTSCRIPTACTION_SLEEP;

typedef struct _INPUTSCRIPTACTION_BREAKIF
{
	int actionId;
	char* windowTitle;
} INPUTSCRIPTACTION_BREAKIF;

typedef struct _INPUTSCRIPTACTION_LABEL
{
	char* labelName;
	int inputScriptIndex;
} INPUTSCRIPTACTION_LABEL;

typedef struct _INPUTSCRIPTACTION_INC
{
	int actionId;
} INPUTSCRIPTACTION_INC;

typedef struct _INPUTSCRIPTACTION_REPEATWHILE
{
	int actionId;
	int inputScriptIndex;
} INPUTSCRIPTACTION_REPEATWHILE;

typedef struct _INPUTSCRIPTACTION_REPEATUNTIL
{
	int actionId;
	int inputScriptIndex;
	int countCurrentIndex;
	int countEndIndex;
} INPUTSCRIPTACTION_REPEATUNTIL;