////////////////////////////////////////////////////////////
// CrypKey Copy Protection Information                    //
////////////////////////////////////////////////////////////
// Displays information contained within the encryption   //
// keys used in the CrypKey copy protection system        //
//                                                        //
// Should work with v3.40+ (and maybe earlier)            //
//                                                        //
// Tested Versions                                        //
// - v4.3.134                                             //
// - v5.0.161, v5.0.170, v5.0.174                         //
// - v5.3.218                                             //
// - v5.4.219                                             //
// - v5.5.220                                             //
// - v5.6.222                                             //
// - v5.7.229, v5.7.232, v5.7.234 (build 1021,1030,1034)  //
// - v6.0.311 (build 2002a,2002b,6000)                    //
// - v6.0.316 (build 6004)                                //
// - v6.0.326 (build 6007)                                //
// - v6.1.332 (build 6107)                                //
// - v6.1.345 (build 6120)                                //
// - v6.1.349 (build 6134)                                //
// - v6.5.505 (build 6505)                                //
// - v6.5.507 (build 6507)                                //
// - v7.0 (build 7027)                                    //
// - v7.2 (build 7250)                                    //
//                                                        //
// v3.40 - Add To Existing Licence                        //
// v6.00 - Hard Drive Serial Number / New Key Formats     //
// v6.10 - Custom Information Bytes (build 6109)          //
//       - Static Keys (build 6111)                       //
//                                                        //
// History...                                             //
//                                                        //
// v1.14 (06 March 2010)                                  //
// - support for new entry keys used in v7+               //
// - fixed create site key script using UserKeyHash       //
// - compiled with Visual Studio 2008                     //
//                                                        //
// v1.13 (17 March 2007)                                  //
// - updated master key to support v7+ keys               //
// - updated master key to support network seats          //
//                                                        //
// v1.12 (05 September 2006)                              //
// - updated master key support for static and usb keys   //
// - fixed seed values for v6 encryption keys             //
// - fixed issue with v6 master keys that are not of      //
//   standard length (255 rather than 256 characters)     //
// - fixed issue with static key detection                //
// - support for detecting static site codes              //
//                                                        //
// v1.11 (05 November 2004)                               //
// - added additional support for static keys. Now able   //
//   to view and create site keys based on static keys    //
// - updated the scripting engine so further validity     //
//   checks are made on parsed values when creating codes //
//   and keys (master keys, site codes & site keys)       //
// - bug fixes                                            //
//                                                        //
// v1.10 (21 July 2004)                                   //
// - added support for files that hold custom information //
//   bytes (*.csb files). Argument: /customInfoBytes      //
// - show whether static keys are used (by examining the  //
//   master key) and if true then display the encryption  //
//   key ids used to protect the site code & site key     //
// - fix bug in the Site Key Timestamp property within    //
//   the restriction keys (wrong date was displayed)      //
// - v6 restriction keys use a DWORD for licence counts   //
//   and not a WORD like v5 keys. Updated key display to  //
//   reflect this                                         //
// - now recognises banned site keys (based on key level, //
//   key options and licence count). Increments the       //
//   licence count to defeat detection (CORE keygens)     //
// - renamed EncryptionId1 & EncryptionId2 parameters to  //
//   EncryptionKeyId1 & EncryptionKeyId2. Still supports  //
//   the old parameter names but they are depreciated     //
//   (see script parameters.txt - v1.03)                  //
// - minor display changes                                //
//                                                        //
// v1.09 (10 October 2003)                                //
// - utility will now prompt for a script file if run     //
//   from windows explorer and no arguments are specified //
// - minor bug fixes                                      //
//                                                        //
// v1.08 (22 April 2003)                                  //
// - updated the v6 decryption routines. In some cases    //
//   the decrypt function would fail and decrypt the site //
//   code or site key using the wrong keys                //
// - updated the scripting engine to support viewing      //
//   codes and keys (see script parameters.txt - v1.02)   //
//                                                        //
// v1.07 (10 December 2002)                               //
// - support added for v6.00 keys and codes               //
// - added /diskInformation argument, which replicates    //
//   the information obtained in the Easy Licence process //
// - display licenced modules (stored within master key)  //
// - support 'allow add to existing licence' when         //
//   creating site codes                                  //
// - support for ranges when specifing key options        //
//   (i.e. 1,2,3,4,5,8,9,10,14,16 ==> 1-5,8-10,14,16)     //
// - updated scripting engine to support v6 keys and      //
//   codes (see script parameters.txt - v1.01)            //
// - updated code to use class structures                 //
// - enhanced method used to display information          //
//                                                        //
// v1.06 (04 April 2002)                                  //
// - minor updates to the scripting engine                //
//                                                        //
// v1.05 (01 April 2002)                                  //
// - scripting engine added for key creation              //
//   (see script parameters.txt - v1.00)                  //
//                                                        //
// v1.04 (26 January 2002)                                //
// - support for confirmation codes                       //
// - minor updates                                        //
//                                                        //
// v1.03 (07 December 2001)                               //
// - add to existing licence (for site keys)              //
// - can create site codes                                //
// - support for v6.00 keys (early beta version)          //
// - support for v6.00 options (early beta version)       //
// - expanded the information displayed                   //
//   - master key: company number                         //
//   - user key: hash for v6, encrypted password & number //
//   - site key: licence properties & key hashes          //
//   - site code: company number, drive serial number and //
//                crypkey library version                 //
//   - entry key: expanded information titles             //
//   - restriction key: expanded information titles       //
// - removed the company names and account numbers to     //
//   an external file (ckInfo.ini)                        //
// - fixes a couple of issues                             //
//                                                        //
// v1.02 (01 April 2001)                                  //
// - displays information in a clearer way                //
// - displays further information about restriction keys  //
// - displays information for the entry key               //
// - can create master keys, user keys & site keys        //
//                                                        //
// v1.01 (22 March 2001)                                  //
// - shows restriction key information                    //
//                                                        //
// v1.00 (14 March 2001)                                  //
// - original version                                     //
////////////////////////////////////////////////////////////
#include "ckInfo.h"
#include "ckMasterKey.h"
#include "ckUserKey.h"
#include "ckSiteCode.h"
#include "ckSiteKey.h"
#include "ckRestrictionKey.h"
#include "ckEntryKey.h"
#include "ckConfirmationCode.h"
#include "ckCustomInfoBytes.h"
#include "ckDriveInformation.h"
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"

char* errorMessages[] =
{
	"Error occurred parsing the Master Key - Check Key Length",
	"Error occurred parsing the User Key - Check Key Length",
	"Error occurred parsing the Site Code - Check Code Length",
	"Error occurred parsing the Site Code - CRC Check Failed",
	"Error occurred parsing the Site Key - Check Key Length",
	"Error occurred parsing the Restriction Key - Check Key Length",
	"Error occurred parsing the Entry Key - Check Key Length",
	"Error occurred parsing the Confirmation Code - Check Code Length",

	"Error occurred while registering the file extension .ckInfo",
	"Error occurred while unregistering the file extension .ckInfo",
	"Error occurred while parsing the specified script file",
	"Error occurred parsing the script - Unable to open script file",
	"Error occurred parsing the script - Unsupported file format",
	"Error occurred parsing the script - Unrecognised Action parameter",
	"Error occurred parsing the script - Required parameter not found",

	"{CRLF}{CRLF}Error occurred decrypting the Site Code - Encryption Keys Not Found",
	"{CRLF}{CRLF}Error occurred decrypting the Site Key - Encryption Keys Not Found",

	"Unable to retrieve disk information - no supported ide drives found",
	"Unable to retrieve disk information - ideinfo.vxd failed to extract",
	"Unable to retrieve disk information - ideinfo.vxd failed to load",

	"{CRLF}Unable to open or read the Custom Information Bytes file (*.csb)",
	"{CRLF}Error occurred reading the Custom Info Bytes file - Incorrect Size",

	"{CRLF}Unable to open or read the specified Entry Key file"
};

CUtilitiesLibrary ULib;
bool bIsDebugMode = false;

///////////////////////////////////
// returns the debug mode status //
///////////////////////////////////
bool isDebugMode(void)
{
	return bIsDebugMode;
}

/////////////////////////////////////////
// display the specified error message //
/////////////////////////////////////////
void displayErrorMessage(int errorNumber)
{
	ULib.ConsoleLibrary.displayErrorMessage(errorMessages, errorNumber);
}

///////////////////////////////////////////////////////
// creates a padded string based on the value passed //
///////////////////////////////////////////////////////
char* createPaddedStringFromValue(unsigned long dwValue, int maximumLength, char paddingCharacter, bool treatAsHexadecimal)
{
	char* strValue = ULib.StringLibrary.getStringFromNumber(dwValue, treatAsHexadecimal);
	unsigned long strValueLength = ULib.StringLibrary.getStringLength(strValue);
	ULib.HeapLibrary.freePrivateHeap(strValue);
	return ULib.StringLibrary.getString(paddingCharacter, maximumLength - strValueLength);
}

//////////////////////////////////////////////////
// returns the parsed parameter value, if found //
//////////////////////////////////////////////////
char* getParsedParameter(char* parameterName)
{
	char* parsedParameter = (char*)ULib.HeapLibrary.allocPrivateHeap();
	bool parsedParameterResult = ULib.ParsingEngine.getParsedParameter(parameterName, parsedParameter);
	if (ULib.StringLibrary.getStringLength(parsedParameter) <= 0) parsedParameterResult = false;
	if (parsedParameterResult) return parsedParameter;
	ULib.HeapLibrary.freePrivateHeap(parsedParameter);
	return NULL;
}

//////////////////////////////////////////////////
// returns the parsed parameter value, if found //
//////////////////////////////////////////////////
char* getParsedParameterForFormattedString(char* parameterName)
{
	char* parsedParameter = getParsedParameter(parameterName);
	if (parsedParameter == NULL) return NULL;
	char* parsedParameter2 = (char*)ULib.HeapLibrary.allocPrivateHeap();
	// add new line character (for terminate) and remove invalid characters
	wsprintf(parsedParameter2, "%s\n", ((ULib.StringLibrary.hasStringHexadecimalPrefix(parsedParameter)) ? (char*)&parsedParameter[2] : parsedParameter));
	char* parsedParameter3 = ULib.StringLibrary.removeCharacterFromString(parsedParameter2, '.');
	ULib.HeapLibrary.freePrivateHeap(parsedParameter);
	ULib.HeapLibrary.freePrivateHeap(parsedParameter2);
	return parsedParameter3;
}

///////////////////////////////////////////////////////////
// gets the company name that matches the account number //
///////////////////////////////////////////////////////////
char* getCompanyNameFromAccountNumber(int accountNumber, int maximumLength)
{
	char* companyName = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* fileNameWithPath = (char*)ULib.HeapLibrary.allocPrivateHeap();
	if (SearchPath(NULL, "ckInfo.ini", NULL, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fileNameWithPath, NULL) != 0)
	{
		char* accountNameString = ULib.StringLibrary.getStringFromNumber(accountNumber, false);
		GetPrivateProfileString("Company Names", accountNameString, "", companyName, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, fileNameWithPath);
		ULib.HeapLibrary.freePrivateHeap(accountNameString);
	}

	// check maximum length
	if ((maximumLength >= 3) && (ULib.StringLibrary.getStringLength(companyName) > maximumLength))
	{
		companyName[maximumLength - 3] = '.';
		companyName[maximumLength - 2] = '.';
		companyName[maximumLength - 1] = '.';
		companyName[maximumLength] = NULL;
	}

	ULib.HeapLibrary.freePrivateHeap(fileNameWithPath);
	return companyName;
}

////////////////////////////////////////////////////////////////////////
// used to concatenate command-line arguments into the crypkey format //
////////////////////////////////////////////////////////////////////////
char* joinKeyArguments(char* argv[], int startIndex, int numberOfArguments)
{
	char* keyString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	for (int i = startIndex; i < numberOfArguments; i++) ULib.StringLibrary.appendString(keyString, argv[i]);
	char* keyStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(keyString);
	ULib.HeapLibrary.freePrivateHeap(keyString);
	return keyStringNoSpaces;
}

/////////////////////////////////////////////////////////////////
// attempts to open the master key file and parse the contents //
/////////////////////////////////////////////////////////////////
bool readMasterKeyFile(char* fileName, char** outputBuffer)
{
	if (outputBuffer == NULL) return false;
	*outputBuffer = NULL;
	unsigned long keyBufferLength = 0;
	unsigned char* keyBuffer = ULib.FileLibrary.readBufferFromFile(fileName, &keyBufferLength);
	if ((keyBuffer == NULL) || (keyBufferLength <= 0)) return false;
	for (unsigned long i = 0; i < keyBufferLength; i++) if (keyBuffer[i] < 0x20) keyBuffer[i] = 0x20;
	*outputBuffer = ULib.StringLibrary.removeSpacesFromString((char*)keyBuffer);
	ULib.HeapLibrary.freePrivateHeap(keyBuffer);
	return true;
}

////////////////////////
// execute the script //
////////////////////////
void executeScript(char* scriptFilename)
{
	// initialise parsing engine
	ULib.ParsingEngine.initialiseParsingEngine(scriptFilename, "#Default Script (ckInfo)", "AccountNumber,Action,AddToExistingLicence,AllowAddToExistingLicence,AllowEasyLicence,ApplicationId,ConfirmationCode,CrypKeyLibraries,DriveSerialNumber,EasyLicence,EncryptionKeyId1,EncryptionId1,EncryptionKeyId2,EncryptionId2,EntryKey,Filename,KeyLevel,KeyOptions,LicenceDuration,MasterKey,NumberOfLicences,NetworkLicence,NumberOfDays,NumberOfRuns,Password,RestrictionKey,ScriptName,SiteCode,SiteCodeId,SiteKey,UserKey,UserKeyHash,UserKeyHashSeed,SiteCodeCheckByte,UseDriveSerial,UserKeyValue,UserKeyValueSeed");
	// parse the script, handling any errors that occur
	if (!ULib.ParsingEngine.parseFile())
	{
		int lastErrorNumber = ULib.ParsingEngine.getLastErrorNumber();
		if (lastErrorNumber == ULib.ParsingEngine.ERROR_PARSINGENGINE_FILENOTFOUND) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_SCRIPTFILE_OPEN);
		else if (lastErrorNumber == ULib.ParsingEngine.ERROR_PARSINGENGINE_UNSUPPORTEDFILEFORMAT) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_SCRIPTFILE_UNSUPPORTED);
		else ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_SCRIPTFILE_PARSING);
	}

	// display script name (or filename)
	ULib.ConsoleLibrary.displayConsoleMessage("Executing Script File (", false);
	char* scriptNameParameter = getParsedParameter("ScriptName");
	if (scriptNameParameter != NULL)
	{
		ULib.ConsoleLibrary.displayConsoleMessage(scriptNameParameter, false);
		ULib.HeapLibrary.freePrivateHeap(scriptNameParameter);
	}
	else
	{
		char* fileNameLongPath = ULib.FileLibrary.convertShortPathToLongPath(scriptFilename);
		char* fileNameLowercase = ULib.FileLibrary.stripPathFromFilename(fileNameLongPath, true);
		ULib.ConsoleLibrary.displayConsoleMessage(fileNameLowercase, false);
		ULib.HeapLibrary.freePrivateHeap(fileNameLowercase);
		ULib.HeapLibrary.freePrivateHeap(fileNameLongPath);
	}

	ULib.ConsoleLibrary.displayConsoleMessage(")");
	ULib.ConsoleLibrary.displayConsoleMessage("");

	// get action parameter
	bool displayActionError = false;
	bool displayViewError = false;
	char* actionParameter = getParsedParameter("Action");
	if (actionParameter == NULL) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_SCRIPTFILE_UNRECOGNISED);
	// check for recognised action values
	if (ULib.StringLibrary.isStringMatch(actionParameter, "CreateMasterKey")) createMasterKey();
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "CreateUserKey")) createUserKey();
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "CreateSiteCode")) createSiteCode(KEYTYPE_SITECODE);
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "CreateSiteCode6")) createSiteCode(KEYTYPE_SITECODE6);
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "CreateSiteKey")) createSiteKey();
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewMasterKey"))
	{
		char* masterKeyString = getParsedParameter("MasterKey");
		if (masterKeyString == NULL) displayViewError = true;
		else
		{
			char* masterKeyStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(masterKeyString);
			showMasterKeyInformation(masterKeyStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(masterKeyStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(masterKeyString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewUserKey"))
	{
		char* userKeyString = getParsedParameter("UserKey");
		if (userKeyString == NULL) displayViewError = true;
		else
		{
			char* userKeyStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(userKeyString);
			showUserKeyInformation(userKeyStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(userKeyStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(userKeyString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewSiteCode"))
	{
		char* siteCodeString = getParsedParameter("SiteCode");
		if (siteCodeString == NULL) displayViewError = true;
		else
		{
			char* siteCodeStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(siteCodeString);
			showSiteCodeInformation(siteCodeStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(siteCodeStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(siteCodeString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewSiteKey"))
	{
		char* siteKeyString = getParsedParameter("SiteKey");
		if (siteKeyString == NULL) displayViewError = true;
		else
		{
			char* siteKeyStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(siteKeyString);
			showSiteKeyInformation(siteKeyStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(siteKeyStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(siteKeyString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewRestrictionKey"))
	{
		char* restrictionKeyString = getParsedParameter("RestrictionKey");
		if (restrictionKeyString == NULL) displayViewError = true;
		else
		{
			char* restrictionKeyStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(restrictionKeyString);
			showRestrictionKeyInformation(restrictionKeyStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(restrictionKeyStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(restrictionKeyString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewEntryKey"))
	{
		char* entryKeyString = getParsedParameter("EntryKey");
		if (entryKeyString == NULL) displayViewError = true;
		else
		{
			char* entryKeyStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(entryKeyString);
			showEntryKeyInformation(entryKeyStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(entryKeyStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(entryKeyString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewConfirmationCode"))
	{
		char* confirmationCodeString = getParsedParameter("ConfirmationCode");
		if (confirmationCodeString == NULL) displayViewError = true;
		else
		{
			char* confirmationCodeStringNoSpaces = ULib.StringLibrary.removeSpacesFromString(confirmationCodeString);
			showConfirmationCodeInformation(confirmationCodeStringNoSpaces);
			ULib.HeapLibrary.freePrivateHeap(confirmationCodeStringNoSpaces);
		}
		ULib.HeapLibrary.freePrivateHeap(confirmationCodeString);
	}
	else if (ULib.StringLibrary.isStringMatch(actionParameter, "ViewDiskInformation"))
	{
		showHDSNInformation();
	}
	else
	{
		displayActionError = true;
	}

	ULib.HeapLibrary.freePrivateHeap(actionParameter);
	ULib.ParsingEngine.destroyParsingEngine();

	// display error message
	if (displayActionError) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_SCRIPTFILE_UNRECOGNISED);
	else if (displayViewError) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_SCRIPTFILE_REQUIREDNOTFOUND);
}

// register the .ckInfo shell extension
void registerShellExtension(bool ignoreConsoleOutput)
{
	ULib.RegistryLibrary.unregisterFileExtension("ckInfo", 2);
	char* ckInfoModuleFilename = ULib.FileLibrary.getModuleFilename(NULL);
	char* defaultIconString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	char* shellCommandsString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	wsprintf(defaultIconString, "%s,1", ckInfoModuleFilename);
	wsprintf(shellCommandsString, "Execute Script|%s /executeScript \"%%1\",Edit with Notepad|notepad.exe \"%%1\"", ckInfoModuleFilename);
	if ((!ULib.RegistryLibrary.registerFileExtension("ckInfo", "CrypKey Copy Protection Information Script File", defaultIconString, shellCommandsString)) && (!ignoreConsoleOutput)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_REGISTEREXTENSION);
	else if (!ignoreConsoleOutput) ULib.ConsoleLibrary.displayConsoleMessage("Successfully registered the file extension .ckInfo");
	ULib.HeapLibrary.freePrivateHeap(ckInfoModuleFilename);
	ULib.HeapLibrary.freePrivateHeap(defaultIconString);
	ULib.HeapLibrary.freePrivateHeap(shellCommandsString);
}

////////////////////////////
// main program execution //
////////////////////////////
void main(int argc, char* argv[])
{
	ULib.ConsoleLibrary.displayApplicationTitle("CrypKey Copy Protection Information", "1.14", 2001);
	bIsDebugMode = ULib.ProcessLibrary.isDebugModeEnabled();
	bool isParentProcessWindowsExplorer = ULib.ProcessLibrary.isParentProcessWindowsExplorer();
	bool displayUsage = false;

	// automatically register file extension (if debug mode is enabled and run from window explorer without any parameters)
	if ((argc <= 1) && (bIsDebugMode) && (isParentProcessWindowsExplorer)) registerShellExtension(true);

	// if arguments are passed
	if (argc > 1)
	{
		// disk information
		if ((ULib.StringLibrary.isStringMatch("/diskInformation", argv[1])) ||
			(ULib.StringLibrary.isStringMatch("/di", argv[1])))
		{
			showHDSNInformation();
		}
		// register file extension
		else if (ULib.StringLibrary.isStringMatch("/reg", argv[1], false, 4))
		{
			registerShellExtension(false);
		}
		// unregister file extension
		else if (ULib.StringLibrary.isStringMatch("/unreg", argv[1], false, 6))
		{
			if (!ULib.RegistryLibrary.unregisterFileExtension("ckInfo", 2)) ULib.ConsoleLibrary.displayErrorMessage(errorMessages, ERROR_UNREGISTEREXTENSION);
			else ULib.ConsoleLibrary.displayConsoleMessage("Successfully unregistered the file extension .ckInfo");
		}
		// script filename (without /executeScript option)
		else if (ULib.FileLibrary.getFileSize(argv[1]) > 0)
		{
			executeScript(argv[1]);
		}
		else if (argc > 2)
		{
			char* keyString = joinKeyArguments(argv, 2, argc);

			// create new key
			if ((ULib.StringLibrary.isStringMatch("/createkey", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/ck", argv[1])))
			{
				// master key
				if ((ULib.StringLibrary.isStringMatch("master", argv[2])) || (ULib.StringLibrary.isStringMatch("masterKey", argv[2]))) createMasterKey();
				// user key
				else if ((ULib.StringLibrary.isStringMatch("user", argv[2])) || (ULib.StringLibrary.isStringMatch("userKey", argv[2]))) createUserKey();
				// site key
				else if ((ULib.StringLibrary.isStringMatch("site", argv[2])) || (ULib.StringLibrary.isStringMatch("siteKey", argv[2]))) createSiteKey();
				// site code
				else if (ULib.StringLibrary.isStringMatch("siteCode", argv[2])) createSiteCode(KEYTYPE_SITECODE);
				else if (ULib.StringLibrary.isStringMatch("siteCode6", argv[2])) createSiteCode(KEYTYPE_SITECODE6);
				else displayUsage = true;
			}
			// master key information
			else if ((ULib.StringLibrary.isStringMatch("/masterKey", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/mk", argv[1])))
			{
				char* keyBuffer = NULL;
				if (!readMasterKeyFile(argv[2], &keyBuffer)) showMasterKeyInformation(keyString);
				else if (keyBuffer != NULL)
				{
					showMasterKeyInformation(keyBuffer);
					ULib.HeapLibrary.freePrivateHeap(keyBuffer);
				}
				else displayUsage = true;
			}
			// user key information
			else if ((ULib.StringLibrary.isStringMatch("/userKey", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/uk", argv[1])))
			{
				showUserKeyInformation(keyString);
			}
			// site code information
			else if ((ULib.StringLibrary.isStringMatch("/siteCode", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/sc", argv[1])))
			{
				showSiteCodeInformation(keyString);
			}
			// site key information
			else if ((ULib.StringLibrary.isStringMatch("/siteKey", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/sk", argv[1])))
			{
				showSiteKeyInformation(keyString);
			}
			// restriction key information
			else if ((ULib.StringLibrary.isStringMatch("/restrictionKey", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/rk", argv[1])))
			{
				showRestrictionKeyInformation(keyString);
			}
			// entry key information
			else if ((ULib.StringLibrary.isStringMatch("/entryKey", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/ek", argv[1])))
			{
				showEntryKeyInformation(keyString);
			}
			// confirmation code information
			else if ((ULib.StringLibrary.isStringMatch("/confirmationCode", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/cc", argv[1])))
			{
				showConfirmationCodeInformation(keyString);
			}
			// execute script
			else if ((ULib.StringLibrary.isStringMatch("/executeScript", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/es", argv[1])))
			{
				executeScript(argv[2]);
			}
			// custom information bytes
			else if ((argc > 3) && ((ULib.StringLibrary.isStringMatch("/customInfoBytes", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/cib", argv[1]))))
			{
				decryptCustomInfoBytes(argv[2], argv[3]);
			}
			// ced licence
			else if ((argc > 3) && ((ULib.StringLibrary.isStringMatch("/cedLicence", argv[1])) ||
				(ULib.StringLibrary.isStringMatch("/ced", argv[1]))))
			{
				decryptCEDLicence(argv[2], argv[3]);
			}
			else displayUsage = true;

			ULib.HeapLibrary.freePrivateHeap(keyString);
		}
		else
		{
			displayUsage = true;
		}
	}
	// request filename
	else if (isParentProcessWindowsExplorer)
	{
		char* scriptFilename = (char*)ULib.HeapLibrary.allocPrivateHeap();
		if (ULib.FileLibrary.getOpenFileName("Select Script File", "Script Files (*.ckInfo)\0*.ckInfo\0\0", "ckInfo", NULL, scriptFilename)) executeScript(scriptFilename);
		else displayUsage = true;
		ULib.HeapLibrary.freePrivateHeap(scriptFilename);
	}
	else
	{
		displayUsage = true;
	}

	if (displayUsage)
	{
		ULib.ConsoleLibrary.displayConsoleMessage("Usage  : ckInfo [options]");
		ULib.ConsoleLibrary.displayConsoleMessage("Options:");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /createKey [master|user|sitecode|sitecode6|site]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /masterKey [<master key>|<filename>]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /userKey <user key>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /siteCode <site code>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /siteKey <site key>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /restrictionKey <restriction key>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /entryKey [<entry key>|<filename>]");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /confirmationCode <confirmation code>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /customInfoBytes <filename> <output filename>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /cedLicence <filename> <output filename>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /diskInformation\r\n");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /executeScript <script filename>");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /registerExtension");
		ULib.ConsoleLibrary.displayConsoleMessage("\t /unregisterExtension\r\n");
		ULib.ConsoleLibrary.displayConsoleMessage("\t (see readme.txt for further usage information)");
	}

	ULib.ProcessLibrary.exitProcessHandler();
}