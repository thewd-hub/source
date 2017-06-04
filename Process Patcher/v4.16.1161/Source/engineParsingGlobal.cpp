///////////////////////////////////////////////////////////////////////////////////////
// Parses the parameters that are not associated with a particular section but are   //
// Global parameters. Stores the information in the PATCHINGDETAILS_GLOBAL structure //
///////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "ppatcher.h"
#include "engineParsing.h"
#include "engineParsingGlobal.h"
#include "errorMessageHandler.h"

// attempt to parse all the Global parameters
bool parseGlobalParameters(char* stringBuffer, PATCHINGDETAILS_GLOBAL* pGlobalDetails)
{
	char* parsingString = (char*)ULib.HeapLibrary.allocPrivateHeap();
	int parsingResult = PARSINGENGINE_PARSINGERROR;

	// minimum process patcher version required for this patch
	if ((parsingResult = parseStringParameter("Version", (unsigned long)pGlobalDetails->minimumProcessPatcherVersionRequired, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pGlobalDetails->minimumProcessPatcherVersionRequired = ULib.StringLibrary.getFloatFromString(parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// a comma separated list declaring the operating system
	// platforms supported by this patch. Supported Platforms...
	//
	//                      -+
	//    Win95(+)           |
	//    Win98(+)           +- Win9x    -+
	//    WinME(+)           |            |
	//                      -+            |
	//                                    +- All
	//                      -+            |
	//    WinNT4(+)          |            |
	//    Win2000(+)         |            |
	//    WinXP(+)           |            |
	//    WinServer2003(+)   |            |
	//    WinVista(+)        +- WinNTx   -+
	//    WinServer2008(+)   |
	//    Win7(+)            |
	//    WinServer2008R2(+) |
	//                      -+
	//
	if ((parsingResult = parseStringParameter("SupportedPlatforms", pGlobalDetails->supportedOperatingSystemPlatforms, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		unsigned long supportedOperatingSystemPlatforms = OS_UNDEFINED;

		// get the string elements, and parse
		int numberOfStringElements = 0;
		char* stringElements[32];
		ULib.StringLibrary.getStringElementsFromString(parsingString, stringElements, &numberOfStringElements, 32, ',');

		// cycle throught the elements looking for any matches
		for (int i = 0; i < numberOfStringElements; i++)
		{
			if (stringElements[i] == NULL) continue;
			ULib.StringLibrary.trimStringSpaces(stringElements[i], true, true, true);

			// Supports all Microsoft Win32 Operating Systems
			if (ULib.StringLibrary.isStringMatch(stringElements[i], "All"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_95;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_98;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_ME;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_NT4;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_2000;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_XP;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
				// no need to check for any other parameters
				break;
			}
			// Windows 95/98/ME
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win9x"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_95;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_98;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_ME;
			}
			// Windows 95+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win95+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_95;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_98;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_ME;
			}
			// Windows 95
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win95"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_95;
			}
			// Windows 98+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win98+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_98;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_ME;
			}
			// Windows 98
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win98"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_98;
			}
			// Windows ME+
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "WinME+")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "WinMill+")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_ME;
			}
			// Windows ME (or Windows Millennium)
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "WinME")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "WinMill")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_ME;
			}
			// Windows NT4/2000/XP/Server 2003/Vista/Server 2008/7/Server 2008 R2
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNTx"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_NT4;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_2000;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_XP;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows NT4+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNT4+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_NT4;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_2000;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_XP;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows NT4
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNT4"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_NT4;
			}
			// Windows 2000+
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "Win2000+")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNT5+")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_2000;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_XP;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows 2000 (or Windows NT5)
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "Win2000")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNT5")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_2000;
			}
			// Windows XP+
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "WinXP+")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "Win2002")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_XP;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows XP (or Windows 2002)
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "WinXP")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "Win2002")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_XP;
			}
			// Windows Server 2003+
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "WinServer2003+")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNETServer+")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows Server 2003
			else if ((ULib.StringLibrary.isStringMatch(stringElements[i], "WinServer2003")) || (ULib.StringLibrary.isStringMatch(stringElements[i], "WinNETServer")))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2003;
			}
			// Windows Vista+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinVista+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows Vista
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinVista"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_VISTA;
			}
			// Windows Server 2008+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinServer2008+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows Server 2008
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinServer2008"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008;
			}
			// Windows 7+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win7+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows 7
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "Win7"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
			}
			// Windows Server 2008 R2+
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinServer2008R2+"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_7;
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}
			// Windows Server 2008 R2
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WinServer2008R2"))
			{
				supportedOperatingSystemPlatforms |= OS_WINDOWS_SERVER2008R2;
			}

			ULib.HeapLibrary.freePrivateHeap(stringElements[i]);
		}

		pGlobalDetails->supportedOperatingSystemPlatforms = supportedOperatingSystemPlatforms;
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// attempts to hide the process patcher from the target process
	// so to avoid standard detection attempts (Windows 9x Only)
	if ((parsingResult = parseStringParameter("StealthMode", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pGlobalDetails->enableStealthMode = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// general information about the patch
	if ((parsingResult = parseStringParameter("PatchInformation", pGlobalDetails->patchInformation, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pGlobalDetails->patchInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pGlobalDetails->patchInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// information about the author of the patch
	if ((parsingResult = parseStringParameter("PatchAuthor", pGlobalDetails->patchAuthor, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pGlobalDetails->patchAuthor = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pGlobalDetails->patchAuthor, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// contact information for the author of the patch
	if ((parsingResult = parseStringParameter("PatchContactInformation", pGlobalDetails->patchContactInformation, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pGlobalDetails->patchContactInformation = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(pGlobalDetails->patchContactInformation, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, parsingString);
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// ignores the session id when checking for a process match (for Windows 2000+)
	if ((parsingResult = parseStringParameter("IgnoreSessionId", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		pGlobalDetails->ignoreSessionId = (ULib.StringLibrary.isStringMatch(parsingString, "true"));
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	// A comma separated list declaring block execution options. Supported Options...
	//
	//    64Bit --- Windows 64-bit operating system (64-bit process)
	//    WOW64 --- Windows WOW64 (32-bit processes running on 64-bit operating system)
	//
	if ((parsingResult = parseStringParameter("BlockExecution", (char*)NULL, parsingString, stringBuffer)) == PARSINGENGINE_PARSINGERROR) return false;
	if (parsingResult == PARSINGENGINE_PARSINGOK)
	{
		unsigned long blockExecution =  BLOCKEXECUTION_UNDEFINED;

		// get the string elements, and parse
		int numberOfStringElements = 0;
		char* stringElements[32];
		ULib.StringLibrary.getStringElementsFromString(parsingString, stringElements, &numberOfStringElements, 32, ',');

		// cycle throught the elements looking for any matches
		for (int i = 0; i < numberOfStringElements; i++)
		{
			if (stringElements[i] == NULL) continue;
			ULib.StringLibrary.trimStringSpaces(stringElements[i], true, true, true);

			// Block 64Bit
			if (ULib.StringLibrary.isStringMatch(stringElements[i], "64Bit"))
			{
				blockExecution |= BLOCKEXECUTION_64BIT;
			}
			// Block WOW64
			else if (ULib.StringLibrary.isStringMatch(stringElements[i], "WOW64"))
			{
				blockExecution |= BLOCKEXECUTION_WOW64;
			}

			ULib.HeapLibrary.freePrivateHeap(stringElements[i]);
		}

		pGlobalDetails->blockExecution = blockExecution;
		ULib.HeapLibrary.freePrivateHeap(parsingString);
		return true;
	}

	ULib.HeapLibrary.freePrivateHeap(parsingString);
	return true;
}

// check that the global parameters have been defined and are correct
bool checkGlobalParameters(PATCHINGDETAILS_GLOBAL* pGlobalDetails)
{
	// check process patcher version, if required
	if (pGlobalDetails->minimumProcessPatcherVersionRequired > 0)
	{
		// the process patcher is not the required minimum version
		if (ULib.StringLibrary.getFloatFromString(ppApplicationVersion) < pGlobalDetails->minimumProcessPatcherVersionRequired)
		{
			char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
			sprintf_s(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Process Patcher v%.2f+ is required for execution of this script", pGlobalDetails->minimumProcessPatcherVersionRequired);
			displayCustomErrorMessage(errorMessage, true);
			ULib.HeapLibrary.freePrivateHeap(errorMessage);
		}
	}

	// check block execution flags
	if (pGlobalDetails->blockExecution > 0)
	{
		bool blockExecution = false;
		char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Does Not Support ");

		// check for 64-bit block
		if (pGlobalDetails->blockExecution & BLOCKEXECUTION_64BIT)
		{
			if ((ULib.OperatingSystemLibrary.isWindows64Bit()) && (!ULib.ProcessLibrary.isWOW64Process()))
			{
				ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "64-Bit OS, ");
				blockExecution = true;
			}
		}
		// check for WOW64 block
		if (pGlobalDetails->blockExecution & BLOCKEXECUTION_WOW64)
		{
			if ((ULib.OperatingSystemLibrary.isWindows64Bit()) && (ULib.ProcessLibrary.isWOW64Process()))
			{
				ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "WOW64 Environment, ");
				blockExecution = true;
			}
		}

		// display error message if execution is to be blocked
		if (blockExecution)
		{
			ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "\b\b");
			displayErrorMessage(ERROR_PARSING_BLOCKEXECUTION, errorMessage, true);
		}
	}

	return true;
}

// check that the current operating system platform is compatible with this script
bool checkOperatingSystemPlatform(PATCHINGDETAILS_GLOBAL* pGlobalDetails)
{
	// check operating system platform, if required
	if (pGlobalDetails->supportedOperatingSystemPlatforms == OS_UNDEFINED) return true;

	bool isWindows95 = false;
	bool isWindows98 = false;
	bool isWindowsME = false;
	bool isWindowsNT4 = false;
	bool isWindows2000 = false;
	bool isWindowsXP = false;
	bool isWindowsServer2003 = false;
	bool isWindowsVista = false;
	bool isWindowsServer2008 = false;
	bool isWindows7 = false;
	bool isWindowsServer2008R2 = false;

	// check for supported platforms
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_95) isWindows95 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_98) isWindows98 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_ME) isWindowsME = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_NT4) isWindowsNT4 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_2000) isWindows2000 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_XP) isWindowsXP = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_SERVER2003) isWindowsServer2003 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_VISTA) isWindowsVista = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_SERVER2008) isWindowsServer2008 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_7) isWindows7 = true;
	if (pGlobalDetails->supportedOperatingSystemPlatforms & OS_WINDOWS_SERVER2008R2) isWindowsServer2008R2 = true;

	bool foundSupportedPlatform = false;

	// Windows 95
	if ((isWindows95) && (ULib.OperatingSystemLibrary.isWindows95())) foundSupportedPlatform = true;
	// Windows 98
	if ((isWindows98) && (ULib.OperatingSystemLibrary.isWindows98())) foundSupportedPlatform = true;
	// Windows ME
	if ((isWindowsME) && (ULib.OperatingSystemLibrary.isWindowsME())) foundSupportedPlatform = true;
	// Windows NT4
	if ((isWindowsNT4) && (ULib.OperatingSystemLibrary.isWindowsNT4())) foundSupportedPlatform = true;
	// Windows 2000
	if ((isWindows2000) && (ULib.OperatingSystemLibrary.isWindows2000())) foundSupportedPlatform = true;
	// Windows XP
	if ((isWindowsXP) && (ULib.OperatingSystemLibrary.isWindowsXP())) foundSupportedPlatform = true;
	// Windows Server 2003
	if ((isWindowsServer2003) && (ULib.OperatingSystemLibrary.isWindowsServer2003())) foundSupportedPlatform = true;
	// Windows Vista
	if ((isWindowsVista) && (ULib.OperatingSystemLibrary.isWindowsVista())) foundSupportedPlatform = true;
	// Windows Server 2008
	if ((isWindowsServer2008) && (ULib.OperatingSystemLibrary.isWindowsServer2008())) foundSupportedPlatform = true;
	// Windows 7
	if ((isWindows7) && (ULib.OperatingSystemLibrary.isWindows7())) foundSupportedPlatform = true;
	// Windows Server 2008 R2
	if ((isWindowsServer2008R2) && (ULib.OperatingSystemLibrary.isWindowsServer2008R2())) foundSupportedPlatform = true;

	// not running on a supported platform
	if (!foundSupportedPlatform)
	{
		char* errorMessage = (char*)ULib.HeapLibrary.allocPrivateHeap();
		ULib.StringLibrary.copyString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Supports Windows ");
		if (isWindows95) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "95, ");
		if (isWindows98) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "98, ");
		if (isWindowsME) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "ME, ");
		if (isWindowsNT4) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "NT4, ");
		if (isWindows2000) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "2000, ");
		if (isWindowsXP) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "XP, ");
		if (isWindowsServer2003) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Server 2003, ");
		if (isWindowsVista) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Vista, ");
		if (isWindowsServer2008) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Server 2008, ");
		if (isWindows7) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "7, ");
		if (isWindowsServer2008R2) ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "Server 2008 R2, ");
		ULib.StringLibrary.appendString(errorMessage, ULib.HeapLibrary.DEFAULT_ALLOC_SIZE, "\b\b Only");
		displayErrorMessage(ERROR_PARSING_OS_UNSUPPORTED, errorMessage, true);
	}

	return true;
}