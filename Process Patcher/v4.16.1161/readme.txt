Process Patcher
(C)thewd@hotmail.com
------------------------------------------------------------------------------

1.  Introduction
2.  Usage
3.  Configuration File Format
    - Example
    - Notes
    - Memory Addresses
4.  Configuration File Parameters
    - List
    - Description & Example
5.  Release History


1.  Introduction
==============================================================================

This is a command-line script based tool that allows a Win32 application to be
patched during its execution cycle. This method might be required because the
executable may be compressed or encrypted and known unpackers fail to
decompress or decrypt the executable to a runnable form. Also, as this tool
doesn't alter the executable, all CRC checks performed should succeed.

To add limited protection to the configuration file and stop unauthorised
persons using the script, you can encrypt it using the RC4 algorithm
(see /encryptScript argument)


2.  Usage
==============================================================================

To run the process patcher, type 'ppatcher.exe' from the directory containing
'ppatcher.ppc', or by executing the configuration file (.ppc) from Explorer

Note: if the configuration file name is not 'ppatcher.ppc', then use the
      /executeScript parameter to specify the actual name (command prompt only)

All the patching details needed are taken from the configuration file.

- '/executeScript <script name>' specifies the file name of the configuration
  file, if different from 'ppatcher.ppc'

- '/displayScriptInfo' will display patch information which includes author
   and contact information (if available)

- '/reg' will register the extension .ppc as a file type.
  Clicking on the configuration file will execute this tool and run the script.
  You can also edit the file, by selecting 'Edit...' from the context-menu

- '/unreg' will unregister the file type extension .ppc

- '/encryptScript' encrypts the configuration file using a key
   Note: The encryption key can be up to 16 characters (case-sensitive)
         Using the /defaultKey argument encrypts the configuration file
         using the default key and while providing limited protection it
         also allows the script to be executed without user intervention


3a. Configuration File Format - Example
==============================================================================

#Process Patcher Configuration File
Version=4.16
SupportedPlatforms=Win98+,Win2000+

PatchInformation=Example Script
PatchAuthor=thewd
PatchContactInformation=thewd@hotmail.com

DisplayName=Test Program
Filename=test.exe
Filesize=363818-364000
Arguments=/quiet
WaitForWindowName=Test*

Address=0x402368:0x55,0x8B,0xEC:0xB0,0x01,0xC3 // patch 0x402368 to 0x40236A

[Module]
Filename=testModule.dll
Filesize=61400
Address=0x004322:0x12:0x00

[Plugin]
Filename=testPlugin.dll
PluginVersion=1.05
Arguments=VERBOSE


3b. Configuration File Format - Notes
==============================================================================

The target process that will be executed is included first, followed by
additional modules, plugins, or registry entries.
DisplayName and Filename are compulsory for all scripts.

[Module] section contains the module to be patched. Filename is compulsory.
The memory addresses listed here are not the absolute address but relative.
Supports up to 5 modules

[Plugin] section contains the plugins to execute. Filename is compulsory.
Supports up to 5 plugins (plugins are executed before any patching begins)

[Registry] section contains information about actions to perform on the
registry. Only Action is compulsory. Supports up to 5 registry entries

Version control has been introduced since v2.50 and determines the minimum
version the process patcher must be to execute the script (optional)


3c. Configuration File Format - Memory Addresses
==============================================================================

The basic format of the memory address parameter is...
Address=<memory address>:<expected byte>:<patched byte>

The bytes can be either decimal or hexadecimal.
Hexadecimal bytes must be preceeded with the prefix "0x"

Since v4.15, the memory address parameter has been expanded to support...
- consecutive memory address patches in one parameter (maximum 128 bytes)
  e.g. Address=0x401000:0x74,0x04:0xEB,0x00 // patch 0x401000 & 0x401001 bytes

- . character as the <expected byte> to signal that you don't care what byte
  is found at the specific memory address
  e.g. Address=0x401000:.:0x43 // patch 0x401000 with 0x43

- . character as the <patched byte> to signal that you don't want to patch
  the specific memory address (you only what to search for the expected byte)
  e.g. Address=0x401000:0x43:. // skip memory address patch at 0x401000

- an additional element that specifies the variation number. Should the byte
  not be found at the specific address, how many memory addresses around this
  address should be searched for a match with the expected byte
  (best used with the conditional operators to reduce false positives)
  e.g. Address=0x401034:0x74:0xEB:0x34 // search from 0x401000 to 0x401068

- conditional operators before the <expected byte> value. This allows for an
  improved memory search and means that memory patches can be more generic

   Operator | Description
  ----------------------------------------------------------------
      =     | found byte equals <expected byte>
      ==    | found byte equals <expected byte>
      !=    | found byte does not equal <expected byte>
      <>    | found byte does not equal <expected byte>
      >=    | found byte is greater than or equal <expected byte>
      <=    | found byte is less than or equal <expected byte>
      >     | found byte is greater than <expected byte>
      <     | found byte is less than <expected byte>
      ><    | found byte is between <expected bytes> (inclusive)

   e.g.
   Address=0x401034:>0x73:0xEB          // found byte is greater than 0x73
   Address=0x401034:>0x73,<0x23:0xEB,.  // found byte @ 0x401034 is greater
                                        // than 0x73. found byte @ 0x401035
                                        // is less than 0x23
   Address=0x401034:><0x73-0xAF:0xEB    // found byte @ 0x401034 is between
                                        // 0x73 and 0xAF (inclusive)


4a. Configuration File Parameters - List
==============================================================================

Able to parse the following entries (since v3.00)...
(*) denotes a compulsory parameter for that section

[Global Parameters]
   Version
   SupportedPlatforms
   StealthMode
   PatchInformation
   PatchAuthor
   PatchContactInformation
   IgnoreSessionId
   BlockExecution

[Common Parameters]
   IgnoreParsingErrors
   IsDisabled

[Target Process Parameters]
   DisplayName (*)
   Filename (*)
   Filesize
   Arguments
   Address
   RetryCount
   OpenExistingProcess
   CreateSuspendedProcess
   CreatesChildProcess
   UserNotify
   UserNotify-Message
   WaitForWindowName
   WaitForGlobalWindowName
   WaitForTimeDelay

[Child Process Parameters]
   Filename (*)
   Filesize

[Module Parameters]
   Filename (*)
   Filesize
   Address
   RetryCount

[Plugin Parameters]
   Filename (*)
   PluginVersion
   Arguments

[Registry Parameters]
   Action (*)
   Use64BitView


4a. Configuration File Parameters - Description
==============================================================================

Lists all the parameters supported within the configuration file, along with
a description and an example


Global Parameters
------------------------------------------------------------------------------

- Version, the minimum version the Process Patcher needs to be
           e.g. Version=4.10

- SupportedPlatforms, comma separated list that declares which operating system
                      platforms are supported by this script. Possible values...

                                          -+
                      Win95(+)             |
                      Win98(+)             +- Win9x    -+
                      WinME(+)             |            |
                                          -+            |
                                                        +- All
                                          -+            |
                      WinNT4(+)            |            |
                      Win2000(+)           |            |
                      WinXP(+)             |            |
                      WinServer2003(+)     |            |
                      WinVista(+)          +- WinNTx   -+
                      WinServer2008(+)     |
                      Win7(+)              |
                      WinServer20008R2(+)  |
                                          -+

                      e.g. SupportedPlatforms=Win98+,WinXP

- StealthMode, attempts to hide this process patcher from the target process
               (Windows 9x only)
               e.g. StealthMode=true

- PatchInformation, basic information about the patch/script
                    e.g. PatchInformation=Accepts any serial number

- PatchAuthor, name of the script author
               e.g. PatchAuthor=thewd

- PatchContactInformation, information on how to contact the author
                           e.g. PatchContactInformation=thewd@hotmail.com

- IgnoreSessionId, ignores the session id of the process when looking for a
                   process match (emulates the behaviour of previous versions)
                   e.g. IgnoreSessionId=true

- BlockExecution, do not execute script under certain conditions
                  e.g. BlockExecution=WOW64,64Bit


Common Parameters
------------------------------------------------------------------------------

- IgnoreParsingErrors, ignores any parsing errors that are found
                       within the current section
                       e.g. IgnoreParsingErrors=true

- IsDisabled, disables the current section
              IsDisabled=true


Target Process Parameters
------------------------------------------------------------------------------

- DisplayName, a name that represents the target process to be patched
               e.g. DisplayName=Test Application

- Filename, the name of the target process to be executed
            (or use <request> for the filename/process to be selected at runtime)
            e.g. Filename=test.exe
                 Filename=<request>

- Filesize, the file size of the target process. Could be used to check that
            the patch is applied to the correct version
            e.g. Filesize=542331
            e.g. Filesize=542331-543331

- Arguments, arguments to be forwarded to the target process
             e.g. Arguments=/silent /nowarn

- Address, there can be zero or more of these parameters and contain details
           of the patch to be performed. The format of the parameter is...
           <memory address>:<expected byte>:<patched byte> (see section 3c)
           e.g. Address=0x402000:0x74:0xEB
                Address=0x402000:0x74,0x04:0xEB,0x00

- RetryCount, the number of patching attempts to try on the memory locations
              e.g. RetryCount=5

- OpenExistingProcess, attempts to open a process that is already executing, providing
                       it matches the filename (and filesize) specified in the script.
                       If you specifiy <request> as the Filename parameter you can select
                       the process at runtime, but OpenExistingProcess must be declared
                       before the Filename parameter
                       e.g. OpenExistingProcess=true

- CreateSuspendedProcess, starts the target process in a suspended state. It's
                          up to a plugin to control the execution and send a
                          notification to the engine when to resume execution
                          e.g. CreateSuspendedProcess=true

                          Note: this parameter has been depreciated, since plugins using
                                the API v1.01 can replicate this using the ResetEvent
                                support function (see ResumeProcessPatching plugin example)

- CreatesChildProcess, supports applications that creates a child process which
                       is actually the main application
                       e.g. CreatesChildProcess=true

                       Note: The CreatesChildProcess parameter can either be
                             defined as above, in which case the first child
                             process found is patched, or it can be define in
                             its own section, i.e.

                             [Child Process]
                             Filename=childprocess.exe

- UserNotify, displays a message box which the user closes for the patch to continue
              e.g. UserNotify=true

- UserNotify-Message, specifies an additional message that is displayed when
                      the UserNotify parameter is used
                      e.g. UserNotify-Message=Press OK when 'Welcome' window is shown

- WaitForWindowName, specifies the name of the target process window. The patching
                     engine waits until this window exists before attempting to
                     patch the process (window owned by the process)
                     e.g. WaitForWindowName=Target Process Name

- WaitForGlobalWindowName, specifies the name of the target process window. The
                           patching engine waits until this window exists before
                           attempting to patch the process (any window - global)
                           e.g. WaitForGlobalWindowName=Target Process Name

- WaitForTimeDelay, specifies the time delay (in seconds) to wait before attempting
                    to patch the process
                    e.g. WaitForTimeDelay=5

Note: Parameters that can pause the running of the patching engine are executed
      in the following order...
      - WaitForTimeDelay
      - WaitForGlobalWindowName/WaitForWindowName
      - UserNotify


Child Process Parameters
------------------------------------------------------------------------------

- Filename, the name of the child process application
            (or use <request> for the filename to be selected at runtime)
            e.g. Filename=child_test.exe
                 Filename=<request>

- Filesize, the file size of the child process application
            e.g. Filesize=542331
            e.g. Filesize=542331-543331


Module Parameters
------------------------------------------------------------------------------

- Filename, the name of the module loaded by the application
            (or use <request> for the filename to be selected at runtime)
            e.g. Filename=test.dll
                 Filename=<request>

- Filesize, the file size of the module. Could be used to check that
            the patch is applied to the correct version
            e.g. Filesize=542331
            e.g. Filesize=542331-543331

- Address, there can be zero or more of these parameters and contain details of
           the patch to be performed. The format of the parameter is...
           <memory address>:<expected byte>:<patched byte> (see section 3c)
           e.g. Address=0x2000:0x74:0xEB // relative memory address
                Address=0x2000:0x74,0x04:0xEB,0x00

- RetryCount, the number of patching attempts to try on the memory locations
              e.g. RetryCount=5


Plugin Parameters
------------------------------------------------------------------------------

- Filename, the name of the plugin module
            (or use <request> for the filename to be selected at runtime)
            e.g. Filename=test.dll
                 Filename=<request>

- PluginVersion, the minimum version the plugin needs to be for execution
                 e.g. PluginVersion=1.01

- Arguments, arguments to be forwarded to the plugin
             e.g. Arguments=/silent


Registry Parameters
------------------------------------------------------------------------------

- Action, the action to perform within the windows registry
          (can be multiple actions within one registry section)
          Number of arguments for required action:

             Action   | Key/Value Name | Value Data | Value Type
          -------------------------------------------------------
          CreateKey   |      Yes       |     No     |     No
          DeleteKey   |      Yes       |     No     |     No
          DeleteValue |      Yes       |     No     |     No
          SetValue    |      Yes       |     Yes    |     Yes

          Value Types: REG_SZ, REG_EXPAND_SZ, REG_MULTI_SZ, REG_BINARY
                       REG_DWORD, REG_DWORD_LITTLE_ENDIAN, REG_DWORD_BIG_ENDIAN

          e.g.
          Action=CreateKey, HKEY_CURRENT_USER\PROCESSPATCHERTEST\KEY1
          Action=DeleteKey, HKEY_CURRENT_USER\PROCESSPATCHERTEST\KEY1
          Action=DeleteValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\TESTVALUE
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\, "Default Value - Test String", REG_SZ
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\STRING, "Test String", REG_SZ
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\STRING_EXPAND, %WINDIR%, REG_EXPAND_SZ
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\STRING_MULTI, "Test String", REG_MULTI_SZ
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\DWORD, 12345678, REG_DWORD
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\DWORD_HEX, 0x12345678, REG_DWORD
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\DWORD_LITTLE_ENDIAN, 0x12345678, REG_DWORD_LITTLE_ENDIAN
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\DWORD_BIG_ENDIAN, 0x12345678, REG_DWORD_BIG_ENDIAN
          Action=SetValue, HKEY_CURRENT_USER\PROCESSPATCHERTEST\BINARY, "0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08", REG_BINARY

- Use32BitView, when accessing registry use 32-bit alternative view (when running under 64-bit process)
                (this value is ignored if Use64BitView parameter is set to true)
                e.g. Use32BitView=true

- Use64BitView, when accessing registry use 64-bit alternative view (when running under WOW64 process)
                e.g. Use64BitView=true


5.  Release History
==============================================================================

v4.16.1161 (17 May 2011)
- better support for Vista+ operating systems
- fixed process handling functions in 64-bit operating systems
- fixed windows handling functions in 64-bit operating systems
- added Use32BitView to registry parameters (when running as a 64-bit process)
- added Use64BitView to registry parameters
- added BlockExecution to global parameters

v4.15.1159 (19 November 2005)
- fixed bug with the OpenExistingProcess parameter and <request> value when
  running on Windows XP (problems with visual styles after installing KB884883)
- the FileSize parameter is now supported on Module patches
- the FileSize parameter now supports a range value (i.e. 1,240,000-1,250,000)
- added WinVista & WinVista+ supported operating system parameters

v4.15.1158 (01 January 2005)
- expanded the Address parameter to support additional functions
  (see section 3c for a complete list)

v4.15.1157 (03 September 2004)
- redesigned process & module enumeration routines for NT operating systems
  (due to changes introduced in Windows XP SP2 & Windows Server 2003 SP1)
- support added for terminal server sessions (see IgnoreSessionId parameter)
- removed dependency on the psapi.dll module
- improved memory patching routines
- improved parsing engine
- fixed bug in parsing engine (when handling script comments)

v4.14.1140 (04 November 2003)
- the Filename parameter now supports the <request> string when using the
  OpenExistingProcess parameter. A list will be shown of all the processes
  running and the required process may be selected from the list
  (OpenExistingProcess must be declared before the Filename parameter)
- bug fixes

v4.13.1139 (22 April 2003)
- added WaitForTimeDelay parameter, which specifies the number of seconds to
  wait before continuing the patching engine
- plugin api v1.03
- minor updates

v4.12 (23 October 2002)
- added OpenExistingProcess parameter, which searches for a process that is
  already executing and matching the filename specified in the script
- fixed bug in getting the child process thread handle under Windows 95 OSR2.x
- depreciated CreateSuspendedProcess parameter (replaced by plugin api)

v4.11 (02 September 2002)
- plugin api v1.02 (can get the thread id and thread handle of child processes)
- the target process plugin function GetProcessFilename now returns the full
  path and file name, when under v4.10 it only returned the file name
- documented the plugin api functions (for v1.01+)
- fixed access violation exception under Windows NT4
- fixed registry patching issue under Windows 95

v4.10 (21 June 2002)
- plugin api v1.01 (api v1.00 no longer supported)
  - support functions added
  - more events added
  - dynamic waiting event names (using support functions)
  - child process information
- supports comment blocks (/* ... */)
- fixed a module patching issue on winxp sp1 & win2000 sp3
- added WaitForGlobalWindowName parameter, which searches the
  whole desktop for a match. WaitForWindowName only matches
  window names that are created by the target process
- added WinServer2003(+) parameters (for Windows Server 2003)
- removed ImpersonateUser and associated parameters
- removed FurtherDetails parameter
- other minor updates

v4.00 (21 May 2001)
- support for registry patching
- added IsDisabled & IgnoreParsingErrors parameters
- redefined the plugin architecture (v1.00)
- added CreateSuspendedProcess parameter so that plugins can
  control the execution flow
- plugins can control the patching engine execution by using
  defined waiting events (see examples)
- minor bug fixes
- included a selection of examples

v3.93 (22 January 2001)
- added Win95+, Win98+, WinME+, WinNT4+, Win2000+, WinXP
  & WinXP+ supported operating system parameters
- support for Windows XP (Beta 2)
- experimental support for Windows 64-bit (on IA-64 architecture)

v3.92 (01 January 2001)
- added ImpersonateUser-UserName & ImpersonateUser-Password
  parameters, which specifies the account details which
  the target process will be executed (Windows 2000+ only)
- minor updates

v3.91 (22 September 2000)
- added FurtherDetails parameter, which displays further
  details about the patch during runtime
- fixed bug in the parsing of the minimum version number

v3.90 (02 September 2000)
- added WaitForWindowName parameter, where the patching engine
  waits until a window exists that matches the defined name.
  Supports pattern matching for window names.
- fixed bug in the pattern matching functions, where strings
  which clearly didn't match was deemed to match

v3.80 (30 May 2000)
- introduced experimental patching routines
- other internal enhancements

v3.71 (20 February 2000)
- the WaitInfinte parameter is no longer required or supported
- the IsDynamic parameter is no longer needed when defining
  dynamically linked modules

v3.70 (16 February 2000)
- module and Child Process filenames support pattern matching
  using asterisks and/or question marks
  i.e. register.exe ==> regist??.ex?
                    ==> reg*.exe
                    ==> regis*.??e
- introduced UserNotifyMessage parameter
- fixed a major memory leak

v3.60 (29 January 2000)
- full Windows 2000 support
- increased Parsing & Patching Engine robustness
- added patch information parameters - PatchInformation,
  PatchAuthor and PatchContactInformation
  Information is accessed by using the /displayScriptInfo argument
  (this is an attempt to stop people bugging me about patches
  that don't work and were not written by me)
- added RetryCount parameter which determines the number of
  times the memory patch is attempted
- introduced StealthMode parameter, which attempts to hide
  this process patcher from the target process (Windows 9x Only)
- so many other internal enhancements

v3.30 (01 November 1999)
- supports a plugin architecture (beta)
- updated parsing engine to support SupportedPlatforms and
  ImpersonateUser parameters
- faster parsing and patching engine with better error handling
- introduced /defaultKey argument for use with the /encryptScript option.
  Creates an encrypted configuration file with the default
  key. This allows the patch to be run without user intervention
- no longer need psapi.dll module installed when executing on Windows NT4
- many other updates and enhancements
- better Windows 2000 (RC3) support

v3.20 (04 October 1999)
- provided greater support for Windows NT4
- fixed a number of new bugs introduced since v3.xx

v3.10 (16 September 1999)
- added UserNotify and CreatesChildProcess parameters to parsing engine
- updated documentation to give details of every parameter supported

v3.00 (01 September 1999)
- configuration file has a new format (no longer supports the old format)
- new faster parsing engine
- new faster patching engine
- can patch statically & dynamically linked libraries loaded
  or used by the process (Beta Engine)
- no longer need add-on utility to encrypt configuration
  files, as it's now built-in (using /encryptScript parameter)
- blowfish encrypted configuration files are no longer supported
- added Encrypt option to shell extension .ppc

v2.50 (26 June 1999)
- version control introduced
- fixed small bugs
- updated the parsing engine
- updated the registered shell extension .ppc
  (run ppatcher /shell to update registry)

v2.40 (24 April 1999)
- added support for configuration files with a name other than
  'ppatcher.ppc' (/executeScript <filename>)
- the configuration file now supports hexadecimal numbers as
  well as decimal numbers, i.e.  0x402368 (in hexadecimal)
                                 4203368  (in decimal)
- minor bug fixes
- renamed /idle option to /wait

v2.30 (15 March 1999)
- small bug fixes

v2.20 (02 March 1999)
- automatically asks for decryption key if the configuration
  file is encrypted. No need to use /decrypt
- removed /decrypt option
- updated Blowfish Component DLL

v2.10 (22 January 1999)
- rewrote the process timing routine due to problems with
  patching when anti-virus software is running

v2.00 (18 January 1999)
- added support for encrypted configuration files
- created an add-on utility that can encrypt the configuration file

v1.60 (17 January 1999)
- changed the default configuration file extension from .cfg to .ppc
- can register (/shell) and unregister (/noshell) a file type
  association to the extension .ppc

v1.50 (09 January 1999)
- added version and icon resources

v1.40 (05 January 1999)
- fixed timing bug under Windows NT 4.0
- updated the whole timing routine

v1.30 (04 January 1999)
- added command-line option which waits until the process is
  idle before patching bytes (/idle)
- updated documentation

v1.20 (03 January 1999)
- first public version
- changed the layout of the configuration file
- now supports patching to more than one memory location
- updated documentation (although it's still crap)
- fixed known bugs

v1.10 (03 January 1999)
- added support for command-line arguments
- fixed known bugs

v1.00 (02 January 1999)
- initial release