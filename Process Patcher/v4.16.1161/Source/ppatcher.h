///////////////////////////////////////////////////////
// Process Patcher                                   //
// (C)thewd, thewd@hotmail.com                       //
///////////////////////////////////////////////////////

// unreferenced formal parameter
#pragma warning(disable:4100)
// conditional expression is constant
#pragma warning(disable:4127)

#include <stdio.h>
#include "..\..\..\_Shared Libraries\UtilitiesLibrary.h"
#include "apiWin32Support.h"

extern CUtilitiesLibrary ULib;

// process patcher information
#define ppApplicationVersion				"4.16"
#define ppFullApplicationVersion			"4.16.1161"

#ifdef _M_X64
#define ppShellExtension					"ppc64"
#else
#define ppShellExtension					"ppc"
#endif

#define RELEASEBUILD_PRIVATE				true
#define ALLOW_DEBUG_INFORMATION				true
#define ALLOW_SCRIPT_DECRYPTION				RELEASEBUILD_PRIVATE
#define ALLOW_MEMORY_INFORMATION			RELEASEBUILD_PRIVATE
#define ALLOW_DEBUG_SUPPORT					RELEASEBUILD_PRIVATE

CRITICAL_SECTION* getMemoryAccessLock(void);
CRITICAL_SECTION* getModuleBaseAddressLock(void);
CRITICAL_SECTION* getDisplayConsoleMessageLock(void);
CRITICAL_SECTION* getDisplayErrorMessageLock(void);

bool isDebugInformationEnabled(void);
bool isDisplayScriptInformationEnabled(void);
bool isProcessPatcherRunningAsImageDebugger(void);

char* getForwardedCommandLine(void);
char* getCurrentDirectory(void);