///////////////////////////////////////////////////////////
// Keyboard Input Simulator                              //
// (C)thewd, thewd@hotmail.com                           //
///////////////////////////////////////////////////////////
// Release History                                       //
//                                                       //
// 06 September 2004 (v1.01)                             //
// - added parameter SendWord which repeatedly sends a   //
//   word rather than a character                        //
// 29 June 2004                                          //
// - updated to support TemplateOrder parameter          //
// 26 June 2004                                          //
// - updated to support templates and character sets     //
// - using multi-threaded version of BruteForce engine   //
// 24 April 2004                                         //
// - initial version                                     //
///////////////////////////////////////////////////////////

#include <windows.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "..\..\..\_Shared Libraries\DictionaryEngine.h"
#include "..\..\..\_Shared Libraries\BruteForceEngine.h"
#include "kbInput.h"
#include "kbInputWinAPI.h"

// unreferenced formal parameter
#pragma warning(disable:4100)

CUtilitiesLibrary ULib;

char* errorMessages_General[] =
{
	"Unable to find SendInput or GetGUIThreadInfo export in USER32.DLL{CRLF}(Requires Microsoft Windows 98+)",
	"Unable to find SendInput or GetGUIThreadInfo export in USER32.DLL{CRLF}(Requires Microsoft Windows NT4 SP3+)",
	"Unable to find SendInput or GetGUIThreadInfo export in USER32.DLL",
	"{CRLF}Error occurred while creating the script log file"
};

char* errorMessages_ScriptFile[] =
{
	"Error occurred while registering the file extension .kbInput",
	"Error occurred while unregistering the file extension .kbInput",
	"{CRLF}Error occurred while parsing the specified script file",
	"{CRLF}Error occurred parsing the script - Unable to open script file",
	"{CRLF}Error occurred parsing the script - Unsupported file format",
	"{CRLF}Error occurred parsing the script - Required parameter not found",
	"{CRLF}Unable to find any [Input Script] section within the script file"
};

char* errorMessages_ParsingEntries[] =
{
	"{CRLF}Error occurred parsing the Input Script section",
	"{CRLF}Error occurred parsing the Input Script section (Send Action)",
	"{CRLF}Error occurred parsing the Input Script section (SendWord Action)",
	"{CRLF}Error occurred parsing the Input Script section (Sleep Action)",
	"{CRLF}Error occurred parsing the Input Script section (BreakIf Action)",
	"{CRLF}Error occurred parsing the Input Script section (Label Action)",
	"{CRLF}Error occurred parsing the Input Script section (Inc Action)",
	"{CRLF}Error occurred parsing the Input Script section (RepeatWhile Action)",
	"{CRLF}Error occurred parsing the Input Script section (RepeatUntil Action)"
};

char* errorMessages_ExecutingEntries[] =
{
	"{CRLF}Error occurred executing the Input Script section",
	"{CRLF}Error occurred executing the Input Script section (Send Action)",
	"{CRLF}Error occurred executing the Input Script section (SendWord Action)",
	"{CRLF}Error occurred executing the Input Script section (Sleep Action)",
	"{CRLF}Error occurred executing the Input Script section (BreakIf Action)",
	"{CRLF}Error occurred executing the Input Script section (Label Action)",
	"{CRLF}Error occurred executing the Input Script section (Inc Action)",
	"{CRLF}Error occurred executing the Input Script section (RepeatWhile Action)",
	"{CRLF}Error occurred executing the Input Script section (RepeatUntil Action)"
};

#define MAXIMUM_INPUTSCRIPTACTIONS				4096
#define MAXIMUM_NUMBERINSENDGROUP				1024
#define MAXIMUM_TEMPLATECHARACTERSETSTRINGS		128

// input script class
class InputScript
{
	public:
		char* scriptName;
		unsigned long initialDelay;
		char* dictionaryFilename;
		bool dictionaryWordContainsCharacterStrings;
		char* templateWord;
		bool templateWordContainsCharacterStrings;

		int numberOfInputScriptActions;
		INPUTSCRIPTACTION* inputScriptActions[MAXIMUM_INPUTSCRIPTACTIONS];
};

// template class
class TemplateCharacterSetString
{
	public:
		BruteForceEngine BFEngine;
		int wordOffset;
};

INPUT sendInput[MAXIMUM_NUMBERINSENDGROUP * 7];
SENDINPUT SendInputAPI = NULL;
GETGUITHREADINFO GetGUIThreadInfoAPI = NULL;

bool logFileEnabled = false;
HANDLE logFileHandle = NULL;
char* logOutputMessage = NULL;

InputScript* pInputScript = NULL;
int currentActionId = SCRIPTACTIONID_UNKNOWN;
bool breakIfWindowTitle = false;

DictionaryEngine DictEngine;
char* currentDictionaryWord = NULL;
unsigned long currentDictionaryWordLength = 0;

TemplateCharacterSetString* pTemplateCharacterSetString[MAXIMUM_TEMPLATECHARACTERSETSTRINGS];
int numberOfTemplateCharacterSetStrings;
char* currentTemplateWord = NULL;

// open a new log file with the next index number
bool openLogFile(char* scriptFilename)
{
	if ((!logFileEnabled) || (scriptFilename == NULL)) return false;

	// get filename with no extension
	int numberOfElements = 0;
	char* stringElements[1];
	ULib.StringLibrary.getStringElementsFromString(scriptFilename, stringElements, &numberOfElements, 1, '.');
	if (numberOfElements <= 0) return false;

	// get unique file name
	char* logFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int logIndex = 1;
	do
	{
		wsprintf(logFilename, "%s.%d.log", stringElements[0], logIndex++);
	} while (ULib.FileLibrary.doesFileExist(logFilename, true));
	ULib.HeapLibrary.freePrivateHeap(stringElements[0]);

	// test we can write to the new log file
	if (!ULib.FileLibrary.writeBufferToFile(&logFileHandle, logFilename, (unsigned char*)logFilename, 0))
	{
		ULib.HeapLibrary.freePrivateHeap(logFilename);
		return false;
	}

	ULib.HeapLibrary.freePrivateHeap(logFilename);
	return true;
}

// add message to the log file
void addToLog(char* outputMessage)
{
	if ((logFileEnabled) && (logFileHandle != NULL))
	{
		ULib.FileLibrary.appendBufferToFile(&logFileHandle, NULL, (unsigned char*)outputMessage, ULib.StringLibrary.getStringLength(outputMessage));
		ULib.FileLibrary.appendBufferToFile(&logFileHandle, NULL, (unsigned char*)"\r\n", 2);
	}
}

// cleanup when terminating script execution
void cleanupExecution(void)
{
	if (pInputScript != NULL)
	{
		if (pInputScript->scriptName != NULL) ULib.HeapLibrary.freePrivateHeap(pInputScript->scriptName);
		if (pInputScript->dictionaryFilename != NULL)
		{
			DictEngine.destroyDictionaryEngine();
			ULib.HeapLibrary.freePrivateHeap(pInputScript->dictionaryFilename);
		}
		if (pInputScript->templateWord != NULL) ULib.HeapLibrary.freePrivateHeap(pInputScript->templateWord);

		for (int i = 0; i < pInputScript->numberOfInputScriptActions; i++)
		{
			if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_SEND) ULib.HeapLibrary.freePrivateHeap(((INPUTSCRIPTACTION_SEND*)pInputScript->inputScriptActions[i]->actionData)->strCode);
			if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_BREAKIF) ULib.HeapLibrary.freePrivateHeap(((INPUTSCRIPTACTION_BREAKIF*)pInputScript->inputScriptActions[i]->actionData)->windowTitle);
			if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_LABEL) ULib.HeapLibrary.freePrivateHeap(((INPUTSCRIPTACTION_LABEL*)pInputScript->inputScriptActions[i]->actionData)->labelName);
			ULib.HeapLibrary.freePrivateHeap(pInputScript->inputScriptActions[i]->actionData);
		}

		ULib.HeapLibrary.freePrivateHeap(pInputScript);
	}

	for (int i = 0; i < numberOfTemplateCharacterSetStrings; i++)
	{
		pTemplateCharacterSetString[i]->BFEngine.destroyCharacterSet();
		ULib.HeapLibrary.freePrivateHeap(pTemplateCharacterSetString[i]);
	}

	if (logFileEnabled)
	{
		ULib.HeapLibrary.freePrivateHeap(logOutputMessage);
		if (logFileHandle != NULL) CloseHandle(logFileHandle);
	}
}

// show the error message from parsing or executing the input script
void showInputScriptError(char* errorMessages[], char* additionalMessage)
{
	// show error occurred in log file
	if (logFileEnabled)
	{
		addToLog("\r\n==============================================================================\r\nTerminating Script (Error Occurred)");
		cleanupExecution();
	}

	// no additional message
	if (!ULib.StringLibrary.isString(additionalMessage, 1))
	{
		if (currentActionId == SCRIPTACTIONID_SEND) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 1);
		else if (currentActionId == SCRIPTACTIONID_SENDWORD) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 2);
		else if (currentActionId == SCRIPTACTIONID_SLEEP) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 3);
		else if (currentActionId == SCRIPTACTIONID_BREAKIF) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 4);
		else if (currentActionId == SCRIPTACTIONID_LABEL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 5);
		else if (currentActionId == SCRIPTACTIONID_INC) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 6);
		else if (currentActionId == SCRIPTACTIONID_REPEATWHILE) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 7);
		else if (currentActionId == SCRIPTACTIONID_REPEATUNTIL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 8);
		else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0);
	}
	else
	{
		ULib.ConsoleLibrary.displayErrorMessage(errorMessages, 0, false);
		char* outputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(outputMessage, "           (");
		if (currentActionId == SCRIPTACTIONID_SEND) ULib.StringLibrary.appendString(outputMessage, "Send");
		else if (currentActionId == SCRIPTACTIONID_SENDWORD) ULib.StringLibrary.appendString(outputMessage, "SendWord");
		else if (currentActionId == SCRIPTACTIONID_SLEEP) ULib.StringLibrary.appendString(outputMessage, "Sleep");
		else if (currentActionId == SCRIPTACTIONID_BREAKIF) ULib.StringLibrary.appendString(outputMessage, "BreakIf");
		else if (currentActionId == SCRIPTACTIONID_LABEL) ULib.StringLibrary.appendString(outputMessage, "Label");
		else if (currentActionId == SCRIPTACTIONID_INC) ULib.StringLibrary.appendString(outputMessage, "Inc");
		else if (currentActionId == SCRIPTACTIONID_REPEATWHILE) ULib.StringLibrary.appendString(outputMessage, "RepeatWhile");
		else if (currentActionId == SCRIPTACTIONID_REPEATUNTIL) ULib.StringLibrary.appendString(outputMessage, "RepeatUntil");
		if (currentActionId != SCRIPTACTIONID_UNKNOWN) ULib.StringLibrary.appendString(outputMessage, " Action - ");
		ULib.StringLibrary.appendString(outputMessage, additionalMessage);
		ULib.StringLibrary.appendString(outputMessage, ")");
		ULib.ConsoleLibrary.displayConsoleMessage(outputMessage);
		ULib.HeapLibrary.freePrivateHeap(outputMessage);
		ULib.ProcessLibrary.exitProcessHandler();
	}
}

// error occurred parsing the entries within the input script
void parseInputScriptError(char* additionalMessage)
{
	showInputScriptError(errorMessages_ParsingEntries, additionalMessage);
}

// error occurred executing the entries within the input script
void executeInputScriptError(char* additionalMessage)
{
	showInputScriptError(errorMessages_ExecutingEntries, additionalMessage);
}

// add action to the input script
void addInputScriptAction(int actionId, void* actionData)
{
	if (actionData == NULL) parseInputScriptError(NULL);

	if (pInputScript->numberOfInputScriptActions < MAXIMUM_INPUTSCRIPTACTIONS)
	{
		pInputScript->inputScriptActions[pInputScript->numberOfInputScriptActions] = (INPUTSCRIPTACTION*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION));
		pInputScript->inputScriptActions[pInputScript->numberOfInputScriptActions]->actionId = actionId;
		pInputScript->inputScriptActions[pInputScript->numberOfInputScriptActions]->actionData = actionData;
		pInputScript->numberOfInputScriptActions++;
	}
}

// returns whether the string contains the dictionary word character string
bool isDictionaryWordCharacterString(char* keyCodeString)
{
	return (ULib.StringLibrary.isStringMatch(keyCodeString, "{DICTIONARY_WORD}"));
}

// returns whether the string contains the template word character string
bool isTemplateWordCharacterString(char* keyCodeString)
{
	return (ULib.StringLibrary.isStringMatch(keyCodeString, "{TEMPLATE_WORD}"));
}

// populate the properties for the send action
INPUTSCRIPTACTION_SEND* getInputScriptActionSend(char* keyCodeString, bool keyUp, bool* shiftKey, bool* ctrlKey, bool* altKey)
{
	if (keyCodeString == NULL) return NULL;

	INPUTSCRIPTACTION_SEND* actionSend = (INPUTSCRIPTACTION_SEND*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_SEND));
	actionSend->actionId = SCRIPTACTIONID_SEND_CHARACTER;
	actionSend->dwFlags = ((keyUp) ? KEYEVENTF_KEYUP : 0);

	// standard single ascii character (or escaped ascii character)
	if ((ULib.StringLibrary.getStringLength(keyCodeString) == 1) || ((ULib.StringLibrary.getStringLength(keyCodeString) == 3) && (keyCodeString[0] == '{') && (keyCodeString[2] == '}')))
	{
		char charCode = keyCodeString[0];
		if ((charCode >= '0') && (charCode <= '9'))
		{
			actionSend->vkCode = charCode;
			actionSend->scanCode = (WORD)MapVirtualKey(charCode, 0);
		}
		else if ((charCode >= 'A') && (charCode <= 'Z'))
		{
			if (shiftKey != NULL) *shiftKey = true;
			actionSend->vkCode = charCode;
			actionSend->scanCode = (WORD)MapVirtualKey(charCode, 0);
		}
		else
		{
			// escaped character
			if (ULib.StringLibrary.getStringLength(keyCodeString) == 3) charCode = keyCodeString[1];
			WORD vkKey = VkKeyScan(charCode);
			if ((shiftKey != NULL) && ((HIBYTE(vkKey) & 0x01) == 0x01)) *shiftKey = true;
			if ((ctrlKey != NULL) && ((HIBYTE(vkKey) & 0x02) == 0x02)) *ctrlKey = true;
			if ((altKey != NULL) && ((HIBYTE(vkKey) & 0x04) == 0x04)) *altKey = true;
			actionSend->vkCode = LOBYTE(vkKey);
			actionSend->scanCode = (WORD)MapVirtualKey(charCode, 0);
			if ((actionSend->vkCode == 0) && (actionSend->scanCode == 0)) parseInputScriptError("Unrecognised Character");
		}
	}
	// character strings
	else
	{
		short charCode = 0;
		if (ULib.StringLibrary.isStringMatch(keyCodeString, "{APPS}")) charCode = VK_APPS;
		else if ((ULib.StringLibrary.isStringMatch(keyCodeString, "{BACKSPACE}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{BS}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{BKSP}"))) charCode = VK_BACK;
		else if ((ULib.StringLibrary.isStringMatch(keyCodeString, "{BREAK}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{PAUSE}"))) charCode = VK_PAUSE;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{CAPSLOCK}")) charCode = VK_CAPITAL;
		else if ((ULib.StringLibrary.isStringMatch(keyCodeString, "{DELETE}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{DEL}"))) charCode = VK_DELETE;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{DOWN}")) charCode = VK_DOWN;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{END}")) charCode = VK_END;
		else if ((ULib.StringLibrary.isStringMatch(keyCodeString, "{ENTER}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{RETURN}"))) charCode = VK_RETURN;
		else if ((ULib.StringLibrary.isStringMatch(keyCodeString, "{ESCAPE}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{ESC}"))) charCode = VK_ESCAPE;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{EXECUTE}")) charCode = VK_EXECUTE;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{HELP}")) charCode = VK_HELP;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{HOME}")) charCode = VK_HOME;
		else if ((ULib.StringLibrary.isStringMatch(keyCodeString, "{INSERT}")) || (ULib.StringLibrary.isStringMatch(keyCodeString, "{INS}"))) charCode = VK_INSERT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{LEFT}")) charCode = VK_LEFT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{LWIN}")) charCode = VK_LWIN;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{NUMLOCK}")) charCode = VK_NUMLOCK;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{PGDN}")) charCode = VK_NEXT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{PGUP}")) charCode = VK_PRIOR;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{PRINT}")) charCode = VK_PRINT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{PRTSC}")) charCode = VK_SNAPSHOT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{RIGHT}")) charCode = VK_RIGHT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{RWIN}")) charCode = VK_RWIN;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{SCROLLLOCK}")) charCode = VK_SCROLL;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{SELECT}")) charCode = VK_SELECT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{SLEEP}")) charCode = VK_SLEEP;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{SPACE}")) charCode = VK_SPACE;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{TAB}")) charCode = VK_TAB;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{UP}")) charCode = VK_UP;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F1}")) charCode = VK_F1;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F2}")) charCode = VK_F2;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F3}")) charCode = VK_F3;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F4}")) charCode = VK_F4;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F5}")) charCode = VK_F5;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F6}")) charCode = VK_F6;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F7}")) charCode = VK_F7;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F8}")) charCode = VK_F8;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F9}")) charCode = VK_F9;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F10}")) charCode = VK_F10;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F11}")) charCode = VK_F11;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F12}")) charCode = VK_F12;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F13}")) charCode = VK_F13;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F14}")) charCode = VK_F14;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F15}")) charCode = VK_F15;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{F16}")) charCode = VK_F16;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{SHIFT}")) charCode = VK_SHIFT;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{CTRL}")) charCode = VK_CONTROL;
		else if (ULib.StringLibrary.isStringMatch(keyCodeString, "{ALT}")) charCode = VK_MENU;
		else parseInputScriptError("Unrecognised Character String");
		actionSend->vkCode = charCode;
		actionSend->scanCode = (WORD)MapVirtualKey(charCode, 0);
	}

	actionSend->strCode = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(keyCodeString) + 1);
	ULib.StringLibrary.copyString(actionSend->strCode, keyCodeString);
	return actionSend;
}

// parse the entries within the input script
void parseInputScript(void)
{
	char* parsedParameterName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* parsedParameterValue = (char*)ULib.HeapLibrary.allocPrivateHeap();
	currentActionId = SCRIPTACTIONID_UNKNOWN;

	// enable logging
	if (ULib.ParsingEngine.getParsedParameter("EnableLogging", parsedParameterValue))
	{
		logFileEnabled = (ULib.StringLibrary.isStringMatch(parsedParameterValue, "true"));
	}

	// script name
	if (ULib.ParsingEngine.getParsedParameter("ScriptName", parsedParameterValue))
	{
		pInputScript->scriptName = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(parsedParameterValue) + 1);
		ULib.StringLibrary.copyString(pInputScript->scriptName, parsedParameterValue);
	}

	// initial delay
	if (ULib.ParsingEngine.getParsedParameter("InitialDelay", parsedParameterValue))
	{
		pInputScript->initialDelay = ULib.StringLibrary.getDWORDFromString(parsedParameterValue);
	}

	// dictionary filename
	if (ULib.ParsingEngine.getParsedParameter("DictionaryFilename", parsedParameterValue))
	{
		pInputScript->dictionaryFilename = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(parsedParameterValue) + 1);
		ULib.StringLibrary.copyString(pInputScript->dictionaryFilename, parsedParameterValue);
	}

	// dictionary word contains character strings
	if (ULib.ParsingEngine.getParsedParameter("DictionaryWordContainsCharacterStrings", parsedParameterValue))
	{
		pInputScript->dictionaryWordContainsCharacterStrings = (ULib.StringLibrary.isStringMatch(parsedParameterValue, "true"));
	}

	// template
	if (ULib.ParsingEngine.getParsedParameter("Template", parsedParameterValue))
	{
		numberOfTemplateCharacterSetStrings = 0;
		pInputScript->templateWord = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(parsedParameterValue) + 10);
		ULib.StringLibrary.copyString(pInputScript->dictionaryFilename, parsedParameterValue);

		// parse the character sets strings
		char* chSet = strstr(parsedParameterValue, "{CS-");
		if (chSet == NULL)
		{
			ULib.StringLibrary.copyString(pInputScript->templateWord, parsedParameterValue);
		}
		else
		{
			int twIndex = 0;
			char* chSetCurrent = parsedParameterValue;

			do
			{
				// get number of characters between character set strings and append normal characters
				twIndex = (int)(chSet - chSetCurrent);
				ULib.StringLibrary.appendString(pInputScript->templateWord, chSetCurrent, twIndex);

				// find end of character set string
				char* endIndex = strstr(chSet, "}");
				if (endIndex == NULL) parseInputScriptError("Unable to parse the Character Set string");

				// update pointers
				chSetCurrent += (int)(endIndex - chSetCurrent) + 1;
				chSet += 4; // skip "{CS-"

				// get character set number
				int chSetNumber = ULib.StringLibrary.getDWORDFromString(chSet);
				if (chSetNumber <= 0) parseInputScriptError("Unable to determine the Character Set number");
				if (numberOfTemplateCharacterSetStrings >= MAXIMUM_TEMPLATECHARACTERSETSTRINGS) parseInputScriptError("Maximum number of Character Set Strings have been reached");

				// get character set
				char* chSetCharacterSet = (char*)ULib.HeapLibrary.allocPrivateHeap();
				if (!ULib.ParsingEngine.getParsedParameter("CharacterSet", chSetCharacterSet, chSetNumber - 1)) parseInputScriptError("Unable to find the specified Character Set");

				// initialise class
				pTemplateCharacterSetString[numberOfTemplateCharacterSetStrings] = (TemplateCharacterSetString*)ULib.HeapLibrary.allocPrivateHeap(sizeof(TemplateCharacterSetString));
				pTemplateCharacterSetString[numberOfTemplateCharacterSetStrings]->BFEngine.initialiseCharacterSet(chSetCharacterSet);
				pTemplateCharacterSetString[numberOfTemplateCharacterSetStrings]->BFEngine.setCurrentWordLength(1);
				pTemplateCharacterSetString[numberOfTemplateCharacterSetStrings]->wordOffset = ULib.StringLibrary.getStringLength(pInputScript->templateWord);
				// append first character
				ULib.StringLibrary.appendString(pInputScript->templateWord, pTemplateCharacterSetString[numberOfTemplateCharacterSetStrings]->BFEngine.getNextWord());
				ULib.HeapLibrary.freePrivateHeap(chSetCharacterSet);
				numberOfTemplateCharacterSetStrings++;

				// update pointer
				chSet = strstr(chSetCurrent, "{CS-");

				// append final characters
				if ((chSet == NULL) && (chSetCurrent != NULL)) ULib.StringLibrary.appendString(pInputScript->templateWord, chSetCurrent);

			} while ((chSet != NULL) && (chSetCurrent != NULL));
		}
	}

	// template order - order the character strings are incremented
	if (ULib.ParsingEngine.getParsedParameter("TemplateOrder", parsedParameterValue))
	{
		// Left-To-Right order
		if (ULib.StringLibrary.isStringMatch(parsedParameterValue, "LTR"))
		{
			// already done
		}
		else
		{
			// make copy of template character set strings
			TemplateCharacterSetString* pTemplateCharacterSetStringCopy[MAXIMUM_TEMPLATECHARACTERSETSTRINGS];
			for (int i = 0; i < numberOfTemplateCharacterSetStrings; i++) pTemplateCharacterSetStringCopy[i] = pTemplateCharacterSetString[i];

			// Right-To-Left order
			if (ULib.StringLibrary.isStringMatch(parsedParameterValue, "RTL"))
			{
				// reverse
				for (int i = 0; i < numberOfTemplateCharacterSetStrings; i++) pTemplateCharacterSetString[i] = pTemplateCharacterSetStringCopy[numberOfTemplateCharacterSetStrings - 1 - i];
			}
			// numerical order
			else
			{
				// get numbers
				int numberOfTemplateOrderElements = 0;
				char* templateOrderElements[MAXIMUM_TEMPLATECHARACTERSETSTRINGS];
				ULib.StringLibrary.getStringElementsFromString(ULib.StringLibrary.removeSpacesFromString(parsedParameterValue), templateOrderElements, &numberOfTemplateOrderElements, MAXIMUM_TEMPLATECHARACTERSETSTRINGS, ',');
				if (numberOfTemplateOrderElements != numberOfTemplateCharacterSetStrings) parseInputScriptError("Unable to parse TemplateOrder - does not match Character Set count");

				// parse numbers and check for duplicates and invalid indexes
				int templateOrderValues[MAXIMUM_TEMPLATECHARACTERSETSTRINGS];
				for (int i = 0; i < numberOfTemplateCharacterSetStrings; i++)
				{
					templateOrderValues[i] = (int)ULib.StringLibrary.getDWORDFromString(templateOrderElements[i]);
					if (templateOrderValues[i] <= 0) parseInputScriptError("Unable to parse TemplateOrder - index must be greater than zero");
					if (templateOrderValues[i] > numberOfTemplateOrderElements) parseInputScriptError("Unable to parse TemplateOrder - index exceeds Character Set count");
					for (int j = (i - 1); j >= 0; j--) if (templateOrderValues[i] == templateOrderValues[j]) parseInputScriptError("Unable to parse TemplateOrder - duplicated indexes found");
				}

				// reorder
				for (int i = 0; i < numberOfTemplateCharacterSetStrings; i++) pTemplateCharacterSetString[i] = pTemplateCharacterSetStringCopy[templateOrderValues[i] - 1];

				// cleanup
				for (int i = 0; i < numberOfTemplateOrderElements; i++) ULib.HeapLibrary.freePrivateHeap(templateOrderElements[i]);
			}
		}
	}

	// template word contains character strings
	if (ULib.ParsingEngine.getParsedParameter("TemplateWordContainsCharacterStrings", parsedParameterValue))
	{
		pInputScript->templateWordContainsCharacterStrings = (ULib.StringLibrary.isStringMatch(parsedParameterValue, "true"));
	}

	// cycle through the input script
	while (ULib.ParsingEngine.getNextParsedParameter("Input Script", parsedParameterName, parsedParameterValue))
	{
		// Send - send keyboard input (repeatedly if necessary)
		if (ULib.StringLibrary.isStringMatch(parsedParameterName, "Send"))
		{
			currentActionId = SCRIPTACTIONID_SEND;

			// get send character (and optionally repeat count)
			int repeatCount = 1;
			int numberOfElements = 0;
			char* sendElements[2];
			ULib.StringLibrary.getStringElementsFromString(parsedParameterValue, sendElements, &numberOfElements, 2, ' ');

			// repeat count
			if (numberOfElements >= 2) repeatCount = ULib.StringLibrary.getDWORDFromString(sendElements[1]);
			if (repeatCount <= 0) parseInputScriptError("Unable to determine the repeat count value");
			// character
			if ((numberOfElements >= 1) && (ULib.StringLibrary.getStringLength(sendElements[0]) > 0))
			{
				char* keyCodeString = sendElements[0];

				// for dictionary word & template word character strings
				if ((isDictionaryWordCharacterString(keyCodeString)) || (isTemplateWordCharacterString(keyCodeString)))
				{
					INPUTSCRIPTACTION_SEND* actionSend = getInputScriptActionSend("0", false, NULL, NULL, NULL);
					if (actionSend == NULL) parseInputScriptError(NULL);
					if (isDictionaryWordCharacterString(keyCodeString)) actionSend->actionId = SCRIPTACTIONID_SEND_DICTIONARYWORD;
					else if (isTemplateWordCharacterString(keyCodeString)) actionSend->actionId = SCRIPTACTIONID_SEND_TEMPLATEWORD;

					// add key to input script (repeatedly if necessary)
					for (int i = 0; i < repeatCount; i++) addInputScriptAction(SCRIPTACTIONID_SEND, actionSend);
				}
				// all other characters & character strings
				else
				{
					bool shiftKey = false;
					bool ctrlKey = false;
					bool altKey = false;

					// check for ctrl, shift & alt control characters
					while ((keyCodeString != NULL) && ((keyCodeString[0] == '+') || (keyCodeString[0] == '^') || (keyCodeString[0] == '%')))
					{
						if (keyCodeString[0] == '+') shiftKey = true;
						else if (keyCodeString[0] == '^') ctrlKey = true;
						else if (keyCodeString[0] == '%') altKey = true;
						keyCodeString++;
					}

					INPUTSCRIPTACTION_SEND* actionSend = getInputScriptActionSend(keyCodeString, false, &shiftKey, &ctrlKey, &altKey);
					if (actionSend == NULL) parseInputScriptError(NULL);

					// shift/ctrl/alt keydown
					if (shiftKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{SHIFT}", false, NULL, NULL, NULL));
					if (ctrlKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{CTRL}", false, NULL, NULL, NULL));
					if (altKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{ALT}", false, NULL, NULL, NULL));

					// key up
					INPUTSCRIPTACTION_SEND* actionSendUp = (INPUTSCRIPTACTION_SEND*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_SEND));
					memcpy(actionSendUp, actionSend, sizeof(INPUTSCRIPTACTION_SEND));
					actionSendUp->dwFlags = KEYEVENTF_KEYUP;

					// add key to input script (repeatedly if necessary)
					for (int i = 0; i < repeatCount; i++)
					{
						addInputScriptAction(SCRIPTACTIONID_SEND, actionSend);
						addInputScriptAction(SCRIPTACTIONID_SEND, actionSendUp);
					}

					if (shiftKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{SHIFT}", true, NULL, NULL, NULL));
					if (ctrlKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{CTRL}", true, NULL, NULL, NULL));
					if (altKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{ALT}", true, NULL, NULL, NULL));
				}
			}
			else
			{
				parseInputScriptError("Incorrect number of parameter values");
			}

			for (int i = 0; i < numberOfElements; i++) ULib.HeapLibrary.freePrivateHeap(sendElements[i]);
		}
		// SendWord - send keyboard input (repeatedly if necessary)
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "SendWord"))
		{
			currentActionId = SCRIPTACTIONID_SENDWORD;

			// get send character (and optionally repeat count)
			int repeatCount = 1;
			int numberOfElements = 0;
			char* sendElements[2];
			ULib.StringLibrary.getStringElementsFromString(parsedParameterValue, sendElements, &numberOfElements, 2, ' ');

			// repeat count
			if (numberOfElements >= 2) repeatCount = ULib.StringLibrary.getDWORDFromString(sendElements[1]);
			if (repeatCount <= 0) parseInputScriptError("Unable to determine the repeat count value");
			// word
			if ((numberOfElements >= 1) && (ULib.StringLibrary.getStringLength(sendElements[0]) > 0))
			{
				char* keyCodeString = sendElements[0];
				int keyCodeStringLength = ULib.StringLibrary.getStringLength(keyCodeString);
				char* keyCodeStringChar = (char*)ULib.HeapLibrary.allocPrivateHeap(10);

				// add word to input script (repeatedly if necessary)
				for (int i = 0; i < repeatCount; i++)
				{
					for (int j = 0; j < keyCodeStringLength; j++)
					{
						bool shiftKey = false;
						bool ctrlKey = false;
						bool altKey = false;

						keyCodeStringChar[0] = keyCodeString[j];
						INPUTSCRIPTACTION_SEND* actionSend = getInputScriptActionSend(keyCodeStringChar, false, &shiftKey, &ctrlKey, &altKey);
						if (actionSend == NULL) parseInputScriptError(NULL);

						// shift/ctrl/alt keydown
						if (shiftKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{SHIFT}", false, NULL, NULL, NULL));
						if (ctrlKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{CTRL}", false, NULL, NULL, NULL));
						if (altKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{ALT}", false, NULL, NULL, NULL));

						// key up
						INPUTSCRIPTACTION_SEND* actionSendUp = (INPUTSCRIPTACTION_SEND*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_SEND));
						memcpy(actionSendUp, actionSend, sizeof(INPUTSCRIPTACTION_SEND));
						actionSendUp->dwFlags = KEYEVENTF_KEYUP;

						// add word to input script
						addInputScriptAction(SCRIPTACTIONID_SEND, actionSend);
						addInputScriptAction(SCRIPTACTIONID_SEND, actionSendUp);

						if (shiftKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{SHIFT}", true, NULL, NULL, NULL));
						if (ctrlKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{CTRL}", true, NULL, NULL, NULL));
						if (altKey) addInputScriptAction(SCRIPTACTIONID_SEND, getInputScriptActionSend("{ALT}", true, NULL, NULL, NULL));
					}
				}
			}
			else
			{
				parseInputScriptError("Incorrect number of parameter values");
			}

			for (int i = 0; i < numberOfElements; i++) ULib.HeapLibrary.freePrivateHeap(sendElements[i]);
		}
		// Sleep - pause execution by specified milliseconds
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "Sleep"))
		{
			currentActionId = SCRIPTACTIONID_SLEEP;

			// get sleep duration
			INPUTSCRIPTACTION_SLEEP* actionSleep = (INPUTSCRIPTACTION_SLEEP*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_SLEEP));
			actionSleep->sleepDuration = ULib.StringLibrary.getDWORDFromString(parsedParameterValue);
			if (actionSleep->sleepDuration <= 0) parseInputScriptError("Unable to determine sleep duration value");
			addInputScriptAction(SCRIPTACTIONID_SLEEP, actionSleep);
		}
		// BreakIf - stop execution if true
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "BreakIf"))
		{
			currentActionId = SCRIPTACTIONID_BREAKIF;

			// get breakif parameters
			int numberOfElements = 0;
			char* breakIfElements[2];
			ULib.StringLibrary.getStringElementsFromString(parsedParameterValue, breakIfElements, &numberOfElements, 2, ' ');
			if (numberOfElements < 2) parseInputScriptError("Incorrect number of parameter values");

			INPUTSCRIPTACTION_BREAKIF* actionBreakIf = (INPUTSCRIPTACTION_BREAKIF*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_BREAKIF));

			// break on window title match
			if (ULib.StringLibrary.isStringMatch(breakIfElements[0], "WindowTitle"))
			{
				actionBreakIf->actionId = SCRIPTACTIONID_BREAKIF_WINDOWTITLE;
				actionBreakIf->windowTitle = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(breakIfElements[1]) + 1);
				ULib.StringLibrary.copyString(actionBreakIf->windowTitle, breakIfElements[1]);
				addInputScriptAction(SCRIPTACTIONID_BREAKIF, actionBreakIf);
			}
			else
			{
				parseInputScriptError("Unrecognised BreakIf parameter value");
			}

			// cleanup
			for (int i = 0; i < numberOfElements; i++) ULib.HeapLibrary.freePrivateHeap(breakIfElements[i]);
		}
		// Label - used to repeat actions
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "Label"))
		{
			currentActionId = SCRIPTACTIONID_LABEL;

			// get label name and input script position
			if (!ULib.StringLibrary.isString(parsedParameterValue, 1)) parseInputScriptError("Incorrect number of parameter values");
			INPUTSCRIPTACTION_LABEL* actionLabel = (INPUTSCRIPTACTION_LABEL*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_LABEL));
			actionLabel->labelName = (char*)ULib.HeapLibrary.allocPrivateHeap(ULib.StringLibrary.getStringLength(parsedParameterValue) + 1);
			ULib.StringLibrary.copyString(actionLabel->labelName, parsedParameterValue);
			actionLabel->inputScriptIndex = pInputScript->numberOfInputScriptActions;
			addInputScriptAction(SCRIPTACTIONID_LABEL, actionLabel);
		}
		// Inc - increment (i.e. next dictionary or template word)
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "Inc"))
		{
			currentActionId = SCRIPTACTIONID_INC;

			// get increment character string
			if (!ULib.StringLibrary.isString(parsedParameterValue, 1)) parseInputScriptError("Incorrect number of parameter values");

			// increment dictionary or template word
			if ((isDictionaryWordCharacterString(parsedParameterValue)) || (isTemplateWordCharacterString(parsedParameterValue)))
			{
				INPUTSCRIPTACTION_INC* actionInc = (INPUTSCRIPTACTION_INC*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_INC));
				if (isDictionaryWordCharacterString(parsedParameterValue)) actionInc->actionId = SCRIPTACTIONID_INC_DICTIONARYWORD;
				else if (isTemplateWordCharacterString(parsedParameterValue)) actionInc->actionId = SCRIPTACTIONID_INC_TEMPLATEWORD;
				addInputScriptAction(SCRIPTACTIONID_INC, actionInc);
			}
			else
			{
				parseInputScriptError("Unrecognised Inc parameter value");
			}
		}
		// RepeatWhile - repeat actions while condition is true (i.e. dictionary or template word available)
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "RepeatWhile"))
		{
			currentActionId = SCRIPTACTIONID_REPEATWHILE;

			// get repeatwhile parameters
			int numberOfElements = 0;
			char* repeatWhileElements[2];
			ULib.StringLibrary.getStringElementsFromString(parsedParameterValue, repeatWhileElements, &numberOfElements, 2, ' ');
			if (numberOfElements < 2) parseInputScriptError("Incorrect number of parameter values");

			// repeat on dictionary or template word
			if ((isDictionaryWordCharacterString(repeatWhileElements[0])) || (isTemplateWordCharacterString(repeatWhileElements[0])))
			{
				// find label and retrieve index
				int labelIndex = -1;
				for (int i = 0; i < pInputScript->numberOfInputScriptActions; i++) if ((pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_LABEL) && (ULib.StringLibrary.isStringMatch(((INPUTSCRIPTACTION_LABEL*)pInputScript->inputScriptActions[i]->actionData)->labelName, repeatWhileElements[1]))) labelIndex = ((INPUTSCRIPTACTION_LABEL*)pInputScript->inputScriptActions[i]->actionData)->inputScriptIndex;
				if (labelIndex < 0) parseInputScriptError("Unable to find a defined Label that matches");

				INPUTSCRIPTACTION_REPEATWHILE* actionRepeatWhile = (INPUTSCRIPTACTION_REPEATWHILE*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_REPEATWHILE));
				if (isDictionaryWordCharacterString(repeatWhileElements[0])) actionRepeatWhile->actionId = SCRIPTACTIONID_REPEATWHILE_DICTIONARYWORD;
				else if (isTemplateWordCharacterString(repeatWhileElements[0])) actionRepeatWhile->actionId = SCRIPTACTIONID_REPEATWHILE_TEMPLATEWORD;
				actionRepeatWhile->inputScriptIndex = labelIndex;
				addInputScriptAction(SCRIPTACTIONID_REPEATWHILE, actionRepeatWhile);
			}
			else
			{
				parseInputScriptError("Unrecognised RepeatWhile parameter value");
			}
		}
		// RepeatUntil - repeat actions until condition is true
		else if (ULib.StringLibrary.isStringMatch(parsedParameterName, "RepeatUntil"))
		{
			currentActionId = SCRIPTACTIONID_REPEATUNTIL;

			// get repeatuntil parameters
			int numberOfElements = 0;
			char* repeatUntilElements[3];
			ULib.StringLibrary.getStringElementsFromString(parsedParameterValue, repeatUntilElements, &numberOfElements, 3, ' ');
			if (numberOfElements < 3) parseInputScriptError("Incorrect number of parameter values");

			if (ULib.StringLibrary.isStringMatch(repeatUntilElements[0], "Count"))
			{
				// find label and retrieve index
				int labelIndex = -1;
				for (int i = 0; i < pInputScript->numberOfInputScriptActions; i++) if ((pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_LABEL) && (ULib.StringLibrary.isStringMatch(((INPUTSCRIPTACTION_LABEL*)pInputScript->inputScriptActions[i]->actionData)->labelName, repeatUntilElements[2]))) labelIndex = ((INPUTSCRIPTACTION_LABEL*)pInputScript->inputScriptActions[i]->actionData)->inputScriptIndex;
				if (labelIndex < 0) parseInputScriptError("Unable to find a defined Label that matches");

				INPUTSCRIPTACTION_REPEATUNTIL* actionRepeatUntil = (INPUTSCRIPTACTION_REPEATUNTIL*)ULib.HeapLibrary.allocPrivateHeap(sizeof(INPUTSCRIPTACTION_REPEATUNTIL));
				actionRepeatUntil->actionId = SCRIPTACTIONID_REPEATUNTIL_COUNT;
				actionRepeatUntil->inputScriptIndex = labelIndex;
				actionRepeatUntil->countCurrentIndex = 0;
				actionRepeatUntil->countEndIndex = ULib.StringLibrary.getDWORDFromString(repeatUntilElements[1]);
				if (actionRepeatUntil->countEndIndex <= actionRepeatUntil->countCurrentIndex) parseInputScriptError("Unable to determine the count ending value");
				addInputScriptAction(SCRIPTACTIONID_REPEATUNTIL, actionRepeatUntil);
			}
			else
			{
				parseInputScriptError("Unrecognised RepeatUntil parameter value");
			}
		}

		currentActionId = SCRIPTACTIONID_UNKNOWN;
		if (pInputScript->numberOfInputScriptActions >= MAXIMUM_INPUTSCRIPTACTIONS) break;
	}

	ULib.HeapLibrary.freePrivateHeap(parsedParameterName);
	ULib.HeapLibrary.freePrivateHeap(parsedParameterValue);
}

// execute the input script
void executeInputScript(void)
{
	currentActionId = SCRIPTACTIONID_UNKNOWN;

	// initialise dictionary engine (dictionary word)
	if (pInputScript->dictionaryFilename != NULL)
	{
		if (!DictEngine.initialiseDictionaryEngine(pInputScript->dictionaryFilename)) executeInputScriptError("Unable to open the dictionary filename");
		currentDictionaryWord = DictEngine.getNextWord(&currentDictionaryWordLength);
	}

	// initialise bruteforce engine (template word)
	if (pInputScript->templateWord != NULL)
	{
		currentTemplateWord = pInputScript->templateWord;
	}

	// initial delay
	if (pInputScript->initialDelay > 0)
	{
		if (logFileEnabled)
		{
			wsprintf(logOutputMessage, "» Sleeping for Initial Delay (%d ms)", pInputScript->initialDelay);
			addToLog(logOutputMessage);
		}

		char* initialDelayString = ULib.StringLibrary.getStringFromDWORD((pInputScript->initialDelay / 1000), false);
		ULib.ConsoleLibrary.displayConsoleMessage("Delaying Script Execution (for ", false);
		ULib.ConsoleLibrary.displayConsoleMessage(initialDelayString, false);
		ULib.ConsoleLibrary.displayConsoleMessage(" seconds)...");
		ULib.HeapLibrary.freePrivateHeap(initialDelayString);

		Sleep(pInputScript->initialDelay);
	}

	// cycle through the input script
	for (int i = 0; i < pInputScript->numberOfInputScriptActions; i++)
	{
		// Send
		if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_SEND)
		{
			currentActionId = SCRIPTACTIONID_SEND;

			INPUTSCRIPTACTION_SEND* actionSend = (INPUTSCRIPTACTION_SEND*)pInputScript->inputScriptActions[i]->actionData;
			if (actionSend == NULL) executeInputScriptError(NULL);

			// dictionary/template word
			if ((actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD) || (actionSend->actionId == SCRIPTACTIONID_SEND_TEMPLATEWORD))
			{
				char* currentWord = NULL;
				bool currentWordContainsCharacterStrings = false;
				if (actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD)
				{
					currentWord = currentDictionaryWord;
					currentWordContainsCharacterStrings = pInputScript->dictionaryWordContainsCharacterStrings;
				}
				else if (actionSend->actionId == SCRIPTACTIONID_SEND_TEMPLATEWORD)
				{
					currentWord = currentTemplateWord;
					currentWordContainsCharacterStrings = pInputScript->templateWordContainsCharacterStrings;
				}

				// reached end of available words
				if (currentWord == NULL) executeInputScriptError(((actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD) ? "Reached end of Dictionary words" : "Reached end of Template words"));

				INPUTSCRIPTACTION_SEND* actionSendDW = NULL;
				int numberOfInputs = 0;
				bool shiftKey = false;
				bool ctrlKey = false;
				bool altKey = false;
				int numberOfCharacters = ULib.StringLibrary.getStringLength(currentWord);
				if (numberOfCharacters >= MAXIMUM_NUMBERINSENDGROUP) numberOfCharacters = (MAXIMUM_NUMBERINSENDGROUP - 1);
				char* currentCharacter = (char*)ULib.HeapLibrary.allocPrivateHeap(numberOfCharacters + 1);

				if (logFileEnabled)
				{
					if (actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD) wsprintf(logOutputMessage, "» Processing Dictionary Word (%s)", currentWord);
					else if (actionSend->actionId == SCRIPTACTIONID_SEND_TEMPLATEWORD) wsprintf(logOutputMessage, "» Processing Template Word (%s)", currentWord);
					addToLog(logOutputMessage);
				}

				// cycle through each character adding it to the keyboard input buffer
				for (int j = 0; j < numberOfCharacters; j++)
				{
					ZeroMemory(currentCharacter, numberOfCharacters + 1);
					shiftKey = false;
					ctrlKey = false;
					altKey = false;

					currentCharacter[0] = currentWord[j];

					// check for character strings in word
					if ((currentWordContainsCharacterStrings) && (currentCharacter[0] == '{'))
					{
						if (j >= (numberOfCharacters - 2)) executeInputScriptError(((actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD) ? "Invalid Character String In Dictionary Word" : "Invalid Character String In Template Word"));
						char* endBrace = strstr((char*)&currentWord[j + 1], "}");
						if (endBrace == NULL) executeInputScriptError(((actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD) ? "Invalid Character String In Dictionary Word" : "Invalid Character String In Template Word"));
						int braceLength = (int)(endBrace - ((char*)&currentWord[j + 1]));
						if (braceLength <= 0) executeInputScriptError(((actionSend->actionId == SCRIPTACTIONID_SEND_DICTIONARYWORD) ? "Invalid Character String In Dictionary Word" : "Invalid Character String In Template Word"));
						ULib.StringLibrary.copyString(currentCharacter, (char*)&currentWord[j], braceLength + 2);
						j += (braceLength + 1);
					}

					// get character
					actionSendDW = getInputScriptActionSend(currentCharacter, false, &shiftKey, &ctrlKey, &altKey);
					if (actionSendDW == NULL) executeInputScriptError(NULL);

					if (shiftKey)
					{
						sendInput[numberOfInputs].type = INPUT_KEYBOARD;
						sendInput[numberOfInputs].ki.wVk = VK_SHIFT;
						sendInput[numberOfInputs].ki.wScan = (WORD)MapVirtualKey(VK_SHIFT, 0);
						sendInput[numberOfInputs].ki.dwFlags = 0;
						sendInput[numberOfInputs].ki.time = 0;
						sendInput[numberOfInputs].ki.dwExtraInfo = 0;
						numberOfInputs++;
						if (logFileEnabled) addToLog("» Sending SHIFT key (down)");
					}
					if (ctrlKey)
					{
						sendInput[numberOfInputs].type = INPUT_KEYBOARD;
						sendInput[numberOfInputs].ki.wVk = VK_CONTROL;
						sendInput[numberOfInputs].ki.wScan = (WORD)MapVirtualKey(VK_CONTROL, 0);
						sendInput[numberOfInputs].ki.dwFlags = 0;
						sendInput[numberOfInputs].ki.time = 0;
						sendInput[numberOfInputs].ki.dwExtraInfo = 0;
						numberOfInputs++;
						if (logFileEnabled) addToLog("» Sending CTRL key (down)");
					}
					if (altKey)
					{
						sendInput[numberOfInputs].type = INPUT_KEYBOARD;
						sendInput[numberOfInputs].ki.wVk = VK_MENU;
						sendInput[numberOfInputs].ki.wScan = (WORD)MapVirtualKey(VK_MENU, 0);
						sendInput[numberOfInputs].ki.dwFlags = 0;
						sendInput[numberOfInputs].ki.time = 0;
						sendInput[numberOfInputs].ki.dwExtraInfo = 0;
						numberOfInputs++;
						if (logFileEnabled) addToLog("» Sending ALT key (down)");
					}

					sendInput[numberOfInputs].type = INPUT_KEYBOARD;
					sendInput[numberOfInputs].ki.wVk = actionSendDW->vkCode;
					sendInput[numberOfInputs].ki.wScan = actionSendDW->scanCode;
					sendInput[numberOfInputs].ki.dwFlags = 0;
					sendInput[numberOfInputs].ki.time = 0;
					sendInput[numberOfInputs].ki.dwExtraInfo = 0;
					numberOfInputs++;
					if (logFileEnabled)
					{
						wsprintf(logOutputMessage, "» Sending %s key (down)", actionSendDW->strCode);
						addToLog(logOutputMessage);
					}

					sendInput[numberOfInputs].type = INPUT_KEYBOARD;
					sendInput[numberOfInputs].ki.wVk = actionSendDW->vkCode;
					sendInput[numberOfInputs].ki.wScan = actionSendDW->scanCode;
					sendInput[numberOfInputs].ki.dwFlags = KEYEVENTF_KEYUP;
					sendInput[numberOfInputs].ki.time = 0;
					sendInput[numberOfInputs].ki.dwExtraInfo = 0;
					numberOfInputs++;
					if (logFileEnabled)
					{
						wsprintf(logOutputMessage, "» Sending %s key (up)", actionSendDW->strCode);
						addToLog(logOutputMessage);
					}

					if (shiftKey)
					{
						sendInput[numberOfInputs].type = INPUT_KEYBOARD;
						sendInput[numberOfInputs].ki.wVk = VK_SHIFT;
						sendInput[numberOfInputs].ki.wScan = (WORD)MapVirtualKey(VK_SHIFT, 0);
						sendInput[numberOfInputs].ki.dwFlags = KEYEVENTF_KEYUP;
						sendInput[numberOfInputs].ki.time = 0;
						sendInput[numberOfInputs].ki.dwExtraInfo = 0;
						numberOfInputs++;
						if (logFileEnabled) addToLog("» Sending SHIFT key (up)");
					}
					if (ctrlKey)
					{
						sendInput[numberOfInputs].type = INPUT_KEYBOARD;
						sendInput[numberOfInputs].ki.wVk = VK_CONTROL;
						sendInput[numberOfInputs].ki.wScan = (WORD)MapVirtualKey(VK_CONTROL, 0);
						sendInput[numberOfInputs].ki.dwFlags = KEYEVENTF_KEYUP;
						sendInput[numberOfInputs].ki.time = 0;
						sendInput[numberOfInputs].ki.dwExtraInfo = 0;
						numberOfInputs++;
						if (logFileEnabled) addToLog("» Sending CTRL key (up)");
					}
					if (altKey)
					{
						sendInput[numberOfInputs].type = INPUT_KEYBOARD;
						sendInput[numberOfInputs].ki.wVk = VK_MENU;
						sendInput[numberOfInputs].ki.wScan = (WORD)MapVirtualKey(VK_MENU, 0);
						sendInput[numberOfInputs].ki.dwFlags = KEYEVENTF_KEYUP;
						sendInput[numberOfInputs].ki.time = 0;
						sendInput[numberOfInputs].ki.dwExtraInfo = 0;
						numberOfInputs++;
						if (logFileEnabled) addToLog("» Sending ALT key (up)");
					}

					ULib.HeapLibrary.freePrivateHeap(actionSendDW->strCode);
					ULib.HeapLibrary.freePrivateHeap(actionSendDW);
				}

				ULib.HeapLibrary.freePrivateHeap(currentCharacter);
				SendInputAPI(numberOfInputs, sendInput, sizeof(INPUT));
			}
			// standard character
			else
			{
				int numberInSendGroup = 1;
				while ((numberInSendGroup < MAXIMUM_NUMBERINSENDGROUP) && ((i + numberInSendGroup) < pInputScript->numberOfInputScriptActions) && (pInputScript->inputScriptActions[i + numberInSendGroup]->actionId == SCRIPTACTIONID_SEND) && (((INPUTSCRIPTACTION_SEND*)pInputScript->inputScriptActions[i + numberInSendGroup]->actionData)->actionId == SCRIPTACTIONID_SEND_CHARACTER)) numberInSendGroup++;

				// group send actions together (but not dictionary or template words)
				for (int j = 0; j < numberInSendGroup; j++)
				{
					actionSend = (INPUTSCRIPTACTION_SEND*)pInputScript->inputScriptActions[i + j]->actionData;
					sendInput[j].type = INPUT_KEYBOARD;
					sendInput[j].ki.wVk = actionSend->vkCode;
					sendInput[j].ki.wScan = actionSend->scanCode;
					sendInput[j].ki.dwFlags = actionSend->dwFlags;
					sendInput[j].ki.time = 0;
					sendInput[j].ki.dwExtraInfo = 0;
					if (logFileEnabled)
					{
						wsprintf(logOutputMessage, "» Sending %s key (%s)", actionSend->strCode, ((actionSend->dwFlags == KEYEVENTF_KEYUP) ? "up" : "down"));
						addToLog(logOutputMessage);
					}
				}

				i += (numberInSendGroup - 1);
				SendInputAPI(numberInSendGroup, sendInput, sizeof(INPUT));
			}
		}
		// Sleep
		else if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_SLEEP)
		{
			currentActionId = SCRIPTACTIONID_SLEEP;

			INPUTSCRIPTACTION_SLEEP* actionSleep = (INPUTSCRIPTACTION_SLEEP*)pInputScript->inputScriptActions[i]->actionData;
			if (actionSleep == NULL) executeInputScriptError(NULL);

			if (logFileEnabled)
			{
				wsprintf(logOutputMessage, "» Sleeping (%d ms)", actionSleep->sleepDuration);
				addToLog(logOutputMessage);
			}

			Sleep(actionSleep->sleepDuration);
		}
		// BreakIf
		else if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_BREAKIF)
		{
			currentActionId = SCRIPTACTIONID_BREAKIF;

			INPUTSCRIPTACTION_BREAKIF* actionBreakIf = (INPUTSCRIPTACTION_BREAKIF*)pInputScript->inputScriptActions[i]->actionData;
			if (actionBreakIf == NULL) executeInputScriptError(NULL);

			// Window Title
			if (actionBreakIf->actionId == SCRIPTACTIONID_BREAKIF_WINDOWTITLE)
			{
				if (logFileEnabled)
				{
					wsprintf(logOutputMessage, "» BreakIf Window Title match (%s)", actionBreakIf->windowTitle);
					addToLog(logOutputMessage);
				}

				// get thread info
				GUITHREADINFO* pThreadInfo = (GUITHREADINFO*)ULib.HeapLibrary.allocPrivateHeap(sizeof(GUITHREADINFO));
				pThreadInfo->cbSize = sizeof(GUITHREADINFO);
				if (GetGUIThreadInfoAPI(NULL, pThreadInfo) == 0) executeInputScriptError("Error occurred getting GUI Thread Info (GetGUIThreadInfo)");
				// get window title
				char* windowTitle = (char*)ULib.HeapLibrary.allocPrivateHeap();
				if (GetWindowText(pThreadInfo->hwndActive, windowTitle, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE) == 0) executeInputScriptError("Error occurred getting Window Title (GetWindowText)");
				// check for match
				if (ULib.StringLibrary.isStringPatternMatch(windowTitle, actionBreakIf->windowTitle)) breakIfWindowTitle = true;
				ULib.HeapLibrary.freePrivateHeap(windowTitle);
				ULib.HeapLibrary.freePrivateHeap(pThreadInfo);
				// found match
				if (breakIfWindowTitle) break;
			}
			else
			{
				executeInputScriptError("Unrecognised BreakIf action");
			}
		}
		// Label
		else if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_LABEL)
		{
			currentActionId = SCRIPTACTIONID_LABEL;

			if (logFileEnabled)
			{
				INPUTSCRIPTACTION_LABEL* actionLabel = (INPUTSCRIPTACTION_LABEL*)pInputScript->inputScriptActions[i]->actionData;
				if (actionLabel == NULL) executeInputScriptError(NULL);
				wsprintf(logOutputMessage, "» Label (Name: %s, Index: %d)", actionLabel->labelName, actionLabel->inputScriptIndex);
				addToLog(logOutputMessage);
			}
		}
		// Inc
		else if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_INC)
		{
			currentActionId = SCRIPTACTIONID_INC;

			INPUTSCRIPTACTION_INC* actionInc = (INPUTSCRIPTACTION_INC*)pInputScript->inputScriptActions[i]->actionData;
			if (actionInc == NULL) executeInputScriptError(NULL);

			// increment dictionary word
			if (actionInc->actionId == SCRIPTACTIONID_INC_DICTIONARYWORD)
			{
				if (logFileEnabled) addToLog("» Incrementing Dictionary Word");
				if (currentDictionaryWord == NULL) executeInputScriptError("Reached end of Dictionary words");
				currentDictionaryWord = DictEngine.getNextWord(&currentDictionaryWordLength);
				if (currentDictionaryWord[0] == NULL) currentDictionaryWord = NULL;
				else currentDictionaryWord[currentDictionaryWordLength] = NULL;
			}
			// increment template word
			else if (actionInc->actionId == SCRIPTACTIONID_INC_TEMPLATEWORD)
			{
				if (logFileEnabled) addToLog("» Incrementing Template Word");
				if (currentTemplateWord == NULL) executeInputScriptError("Reached end of Template words");

				// increment characters
				char* nextCharacter = NULL;
				int currentTemplateCharactetSetString = 0;
				for (int i = 0; i < numberOfTemplateCharacterSetStrings; i++)
				{
					// reached current final index - break
					if (i > currentTemplateCharactetSetString) break;
					nextCharacter = pTemplateCharacterSetString[i]->BFEngine.getNextWord();
					if (nextCharacter == NULL) break;

					// reached last character on last character set string
					if ((nextCharacter[0] == NULL) && (currentTemplateCharactetSetString == (numberOfTemplateCharacterSetStrings - 1)))
					{
						currentTemplateWord = NULL;
						break;
					}
					// reached last character on current character set string - reset & increase
					else if (nextCharacter[0] == NULL)
					{
						// reset character set
						pTemplateCharacterSetString[i]->BFEngine.setCurrentWordLength(1);
						nextCharacter = pTemplateCharacterSetString[i]->BFEngine.getNextWord();
						currentTemplateWord[pTemplateCharacterSetString[i]->wordOffset] = nextCharacter[0];
						// increase current final index
						currentTemplateCharactetSetString++;
					}
					else
					{
						currentTemplateWord[pTemplateCharacterSetString[i]->wordOffset] = nextCharacter[0];
						currentTemplateCharactetSetString = 0;
					}
				}
			}
			else
			{
				executeInputScriptError("Unrecognised Inc action");
			}
		}
		// RepeatWhile
		else if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_REPEATWHILE)
		{
			currentActionId = SCRIPTACTIONID_REPEATWHILE;

			INPUTSCRIPTACTION_REPEATWHILE* actionRepeatWhile = (INPUTSCRIPTACTION_REPEATWHILE*)pInputScript->inputScriptActions[i]->actionData;
			if (actionRepeatWhile == NULL) executeInputScriptError(NULL);

			// repeat while dictionary word isn't null
			if (actionRepeatWhile->actionId == SCRIPTACTIONID_REPEATWHILE_DICTIONARYWORD)
			{
				if (currentDictionaryWord != NULL)
				{
					if (logFileEnabled) addToLog("» RepeatWhile Dictionary Word (true)");
					i = (actionRepeatWhile->inputScriptIndex - 1);
				}
				else
				{
					if (logFileEnabled) addToLog("» RepeatWhile Dictionary Word (false)");
					DictEngine.resetDictionaryEngine();
				}
			}
			// repeat while template word isn't null
			else if (actionRepeatWhile->actionId == SCRIPTACTIONID_REPEATWHILE_TEMPLATEWORD)
			{
				if ((currentTemplateWord != NULL) && (numberOfTemplateCharacterSetStrings > 0))
				{
					if (logFileEnabled) addToLog("» RepeatWhile Template Word (true)");
					i = (actionRepeatWhile->inputScriptIndex - 1);
				}
				else
				{
					if (logFileEnabled) addToLog("» RepeatWhile Template Word (false)");
				}
			}
			else
			{
				executeInputScriptError("Unrecognised RepeatWhile action");
			}
		}
		// RepeatUntil
		else if (pInputScript->inputScriptActions[i]->actionId == SCRIPTACTIONID_REPEATUNTIL)
		{
			currentActionId = SCRIPTACTIONID_REPEATUNTIL;

			INPUTSCRIPTACTION_REPEATUNTIL* actionRepeatUntil = (INPUTSCRIPTACTION_REPEATUNTIL*)pInputScript->inputScriptActions[i]->actionData;
			if (actionRepeatUntil == NULL) executeInputScriptError(NULL);

			// repeat until count is reached
			if (actionRepeatUntil->actionId == SCRIPTACTIONID_REPEATUNTIL_COUNT)
			{
				actionRepeatUntil->countCurrentIndex++;

				if (logFileEnabled)
				{
					wsprintf(logOutputMessage, "» RepeatUntil Count (Current: %d, End: %d)", actionRepeatUntil->countCurrentIndex, actionRepeatUntil->countEndIndex);
					addToLog(logOutputMessage);
				}

				if (actionRepeatUntil->countCurrentIndex < actionRepeatUntil->countEndIndex) i = (actionRepeatUntil->inputScriptIndex - 1);
				else actionRepeatUntil->countCurrentIndex = 0;
			}
			else
			{
				executeInputScriptError("Unrecognised RepeatUntil action");
			}
		}

		currentActionId = SCRIPTACTIONID_UNKNOWN;
	}
}

// register the .kbInput shell extension
void registerShellExtension(bool ignoreConsoleOutput)
{
	ULib.RegistryLibrary.unregisterFileExtension("kbInput", 2);
	char* appFilename = ULib.FileLibrary.getModuleFilename(NULL);
	char* defaultIconString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* shellCommandsString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(defaultIconString, "%s,1", appFilename);
	wsprintf(shellCommandsString, "Execute Script|%s /executeScript \"%%1\",Edit with Notepad|notepad.exe \"%%1\"", appFilename);
	if ((!ULib.RegistryLibrary.registerFileExtension("kbInput", "Keyboard Input Simulator Script File", defaultIconString, shellCommandsString)) && (!ignoreConsoleOutput)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_ScriptFile, 0);
	else if (!ignoreConsoleOutput) ULib.ConsoleLibrary.displayConsoleMessage("Successfully registered the file extension .kbInput");
	ULib.HeapLibrary.freePrivateHeap(appFilename);
	ULib.HeapLibrary.freePrivateHeap(defaultIconString);
	ULib.HeapLibrary.freePrivateHeap(shellCommandsString);
}

// main program execution
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("Keyboard Input Simulator", "1.01", 2004);

	// get SendInput & GetGUIThreadInfo exports from USER32.DLL (requires Windows 98+ & Windows NT4 SP3+)
	HMODULE hUser32 = ULib.FileLibrary.moduleLoadLibrary("user32.dll");
	SendInputAPI = (SENDINPUT)GetProcAddress(hUser32, "SendInput");
	GetGUIThreadInfoAPI = (GETGUITHREADINFO)GetProcAddress(hUser32, "GetGUIThreadInfo");
	if ((SendInputAPI == NULL) || (GetGUIThreadInfoAPI == NULL))
	{
		if (ULib.OperatingSystemLibrary.isWindows9x()) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_General, 0);
		else if (ULib.OperatingSystemLibrary.isWindowsNTx()) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_General, 1);
		else ULib.ConsoleLibrary.displayErrorMessage(errorMessages_General, 2);
	}

	bool displayUsage = false;
	bool useScriptFilename = false;
	char* scriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();

	// use open dialog to select script filename
	if ((argc <= 1) && (ULib.ProcessLibrary.isParentProcessWindowsExplorer()))
	{
		// automatically register file extension (if debug mode is enabled and run from window explorer without any parameters)
		if (ULib.ProcessLibrary.isDebugModeEnabled()) registerShellExtension(true);
		if (ULib.FileLibrary.getOpenFileName("Select Script File", "Script Files (*.kbInput)\0*.kbInput\0\0", "kbInput", NULL, scriptFilename)) useScriptFilename = true;
		else displayUsage = true;
	}
	// script filename (without /executeScript option)
	else if ((argc > 1) && (ULib.FileLibrary.getFileSize(argv[1]) > 0))
	{
		ULib.StringLibrary.copyString(scriptFilename, argv[1]);
		useScriptFilename = true;
	}

	// if arguments are passed
	if (((argc > 2) && (ULib.StringLibrary.isStringMatch("/executeScript", argv[1]))) || (useScriptFilename))
	{
		// display script filename
		if (!useScriptFilename) ULib.StringLibrary.copyString(scriptFilename, argv[2]);
		ULib.ConsoleLibrary.displayConsoleMessage("Parsing Script File (", false);
		char* fileNameLong = ULib.FileLibrary.convertShortPathToLongPath(scriptFilename);
		char* fileNameStripped = ULib.FileLibrary.stripPathFromFilename(fileNameLong, true);
		ULib.ConsoleLibrary.displayConsoleMessage(fileNameStripped, false);
		ULib.ConsoleLibrary.displayConsoleMessage(")...");
		ULib.HeapLibrary.freePrivateHeap(fileNameLong);

		// initialise parsing engine
		ULib.ParsingEngine.initialiseParsingEngine(scriptFilename, "#kbInput Script", "EnableLogging,ScriptName,InitialDelay,DictionaryFilename,DictionaryWordContainsCharacterStrings,Template,TemplateOrder,TemplateWordContainsCharacterStrings,CharacterSet,Send,SendWord,Sleep,BreakIf,Label,Inc,RepeatWhile,RepeatUntil");
		// parse the script, handling any errors that occur
		if (!ULib.ParsingEngine.parseFile())
		{
			int lastErrorNumber = ULib.ParsingEngine.getLastErrorNumber();
			if (lastErrorNumber == ULib.ParsingEngine.ERROR_PARSINGENGINE_FILENOTFOUND) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_ScriptFile, 3);
			else if (lastErrorNumber == ULib.ParsingEngine.ERROR_PARSINGENGINE_UNSUPPORTEDFILEFORMAT) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_ScriptFile, 4);
			else ULib.ConsoleLibrary.displayErrorMessage(errorMessages_ScriptFile, 2);
		}

		// set the current directory to the directory that contains the script
		char* fileNamePath = ULib.FileLibrary.getPathFromFilename(scriptFilename);
		if (ULib.StringLibrary.isString(fileNamePath, 1)) SetCurrentDirectory(fileNamePath);
		ULib.HeapLibrary.freePrivateHeap(fileNamePath);

		// check for an input section
		if ((!ULib.ParsingEngine.isParsedSectionName("Input Script")) || (ULib.ParsingEngine.getNumberOfSectionParameters("Input Script") <= 0)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_ScriptFile, 6);

		// initialise input script
		pInputScript = (InputScript*)ULib.HeapLibrary.allocPrivateHeap(sizeof(InputScript));
		pInputScript->scriptName = NULL;
		pInputScript->initialDelay = 5000;
		pInputScript->dictionaryFilename = NULL;
		pInputScript->dictionaryWordContainsCharacterStrings = false;
		pInputScript->templateWord = NULL;
		pInputScript->templateWordContainsCharacterStrings = false;
		pInputScript->numberOfInputScriptActions = 0;

		// parse input script
		parseInputScript();

		// open log file
		if (logFileEnabled)
		{
			if (!openLogFile(fileNameStripped)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_General, 3);
			logOutputMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
			wsprintf(logOutputMessage, "Starting Script (%s)\r\n==============================================================================\r\n", fileNameStripped);
			addToLog(logOutputMessage);
		}

		// show script name (if available)
		ULib.ConsoleLibrary.displayConsoleMessage("Executing Script (", false);
		ULib.ConsoleLibrary.displayConsoleMessage(((pInputScript->scriptName != NULL) ? pInputScript->scriptName : fileNameStripped), false);
		ULib.ConsoleLibrary.displayConsoleMessage(")...");
		ULib.HeapLibrary.freePrivateHeap(fileNameStripped);

		// execute input script
		executeInputScript();

		char* terminationString = (char*)ULib.HeapLibrary.allocPrivateHeap();
		wsprintf(terminationString, "Teminating Script (%s)...", ((breakIfWindowTitle) ? "BreakIf - WindowTitle" : "Reached End"));
		ULib.ConsoleLibrary.displayConsoleMessage(terminationString);
		ULib.HeapLibrary.freePrivateHeap(terminationString);

		// close log file
		if (logFileEnabled)
		{
			wsprintf(logOutputMessage, "\r\n==============================================================================\r\nTerminating Script (%s)", ((breakIfWindowTitle) ? "BreakIf - WindowTitle" : "Reached End"));
			addToLog(logOutputMessage);
		}

		// cleanup memory
		cleanupExecution();
		ULib.ParsingEngine.destroyParsingEngine();
	}
	else if (argc > 1)
	{
		// register file extension
		if (ULib.StringLibrary.isStringMatch("/reg", argv[1], false, 4))
		{
			registerShellExtension(false);
		}
		// unregister file extension
		else if (ULib.StringLibrary.isStringMatch("/unreg", argv[1], false, 6))
		{
			if (!ULib.RegistryLibrary.unregisterFileExtension("kbInput", 2)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages_ScriptFile, 1);
			else ULib.ConsoleLibrary.displayConsoleMessage("Successfully unregistered the file extension .kbInput");
		}
		else
		{
			displayUsage = true;
		}
	}
	else
	{
		displayUsage = true;
	}

	// display usage information
	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : kbInput [options]");
		ULib.ConsoleLibrary.displayConsoleMessage("Options:");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /executeScript <script filename>\t- execute specified script file");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /registerExtension\t\t\t- register extension .kbInput");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /unregisterExtension\t\t\t- unregister extension .kbInput");
	}

	ULib.HeapLibrary.freePrivateHeap(scriptFilename);
	ULib.ProcessLibrary.exitProcessHandler();
}