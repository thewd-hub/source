CrypKey Copy Protection Information
------------------------------------------------------------------------------

1.  Introduction
2.  Levels and Options
3.  Usage
4.  Release History


1.  Introduction
==============================================================================

A command-line utility that displays information about the keys used by the
CrypKey copy protection system. Also supports the creation of certain keys

- Master Key & User Key
  Hexadecimal strings issued by CrypKey, based on the company number, product
  name and product password. These strings are generally embedded within the
  application files and are used in the InitCrypkey function.

  int InitCrypkey(char* filePath, char* masterKey, char* userKey,
                  int allowFloppy, unsigned long networkCheckTime);

- Site Code
  A site-specific hexadecimal string issued by the protected product when
  installed on a client's computer. This site code is used by the site key
  generator to create a key that licences the application

- Site Key
  A hexadecimal string based on the client's site code and created by the
  site key generator. This site key unlocks software features and licence
  restrictions (stored within the file - <product name>.key)

- Restriction Key
  A hexadecimal string used internally to record the licence restrictions
  (stored within the file - <product name>.rst)

- Entry Key
  A hexadecimal string used internally when a site code has been issued
  (stored within the file - <product name>.ent)

- Confirmation Code
  A hexadecimal string issued by the protected product when the KillLicence
  function has been executed. Can be entered into the site key generator to
  prove that the licence has been terminated


2.  Levels and Options
==============================================================================

Levels and options allow different features of the product to be authorised
and can be used to distinguish between product versions or editions. There are
a number of ways to obtain the correct levels and options, but they may not
work will all protected applications

- examine functions calls to GetAuthorization, GetLevel, GetOption and see
  what return values are expected. Use debug breakpoints or examine
  disassembled code

  int GetAuthorization(unsigned long* optionLevel, int decrementCount);
  unsigned long GetLevel(int numberOfDefinedOptions);
  int GetOption(int numberOfDefinedOptions, int optionNumber);

  Note: these functions could be in a separate module, or embedded within the
        application, depending on whether it uses static or dynamic libraries

- examine the trial function calls ReadyToTry, ReadyToTryDays, ReadyToTryRuns
  and see what values are passed. You should be able to get these values
  without using a debugger (examine disassembled code)

  int ReadyToTry(unsigned long optionLevel, int numberOfDays);
  int ReadyToTryDays(unsigned long optionLevel, int numberOfDays,
                     int latestVersion, int numberOfCopies);
  int ReadyToTryRuns(unsigned long optionLevel, int numberOfRuns,
                     int latestVersion, int numberOfCopies);

- providing the protected product is licenced fully (i.e. the trial licence
  has all features enabled) you should be able to obtain the level and options
  for the site key contained within the <product name>.key file (using ckInfo)


3.  Usage
==============================================================================

Usage  : ckInfo [options]
Options:

/createKey [master | user | sitecode | sitecode6 | site]

   used to create the main keys used within the protection system
   Supports Master Keys (pre v6), User Keys, Site Codes and Site Keys
   (see script parameters.txt for further information)

/masterKey [<master key> | <filename>]

   displays information contained within the master key. To workaround the
   limitations of the Windows 9x command interpreter the master key can be
   stored in a text file (v6 keys exceed the maximum allowed length of 128)
   e.g. /masterKey 2A5D 57C4 1B4C 135B F09E 17F7 600B 2D70 79E8 F275 C36A
        /masterKey masterkey.txt

/userKey <user key>

   displays information contained within the user key
   e.g. /userKey D050 815C D1A2 A79D B1

/siteCode <site code>

   displays information contained within the site code
   e.g. /siteCode D3D1 9008 2C97 5C1D 7C

/siteKey <site key>

   displays information contained within the site key
   e.g. /siteKey C139 C392 C58E 3928 1865 B112 43

/restrictionKey <restriction key>

   displays information contained within the restriction key
   e.g. /restrictionKey F319 8987 D64A B9AC 6F6E 6E49 59A4 43

/entryKey <entry key>

   displays information contained within the entry key
   e.g. /entryKey D0B1 7362 EA4D 12

/confirmationCode <confirmation code>

   displays information contained within the confirmation code
   e.g. /confirmationCode DC81 6687 FA10 9C69 9285 E10D 2B

/customInfoBytes <filename> <output filename>
   decrypts the custom information bytes stored in the encrypted file
   and saves the decrypted bytes to the specified output file
   e.g. /customInformation example.csb example-decrypted.csb

/diskInformation

   displays the model number, firmware revision, serial number and
   HDSN (used by crypkey) for each supported IDE/EIDE disk that is found
   Note: requires Administrator permissions under Windows Server 2003

/executeScript <script filename>

   executes the script filename which can automate a number of options
   (see script parameters.txt for further information)
   e.g. /executeScript example.ckInfo

/registerExtension

   registers the file type extension .ckInfo
   (so scripts can be easily executed from Windows Explorer)

/unregisterExtension

   unregisters the file type extension .ckInfo


4.  Release History
==============================================================================

v1.14 (24 January 2010)
- updated master key support for vm protection, modules and crypkey licence file
- fixed createsitekey script action when using UserKeyHash
- other minor fixes

v1.13 (18 March 2007)
- updated master key information to support v7 keys
- other minor fixes

v1.12 (05 September 2006)
- updated master key support for static and usb keys
- fixed issue with v6 master keys that are not of standard length
- fixed error in encryption routine for v6 keys
- fixed issue with static key detection
- support for detecting static site codes

v1.11 (05 November 2004)
- added additional support for static keys. Now able to view and create site
  keys based on static keys
- updated the scripting engine so that further validity checks are made on
  parsed values when creating codes & keys

v1.10 (21 July 2004)
- support for files that contain custom information bytes (*.csb)
- show whether static keys are used (by examining the master key) and if true
  then display the encryption key ids used to protect the site code & site key
- fix bug in the site key timestamp property within the restriction keys
- renamed script parameters EncryptionId1 & EncryptionId2 to
  EncryptionKeyId1 & EncryptionKeyId2. Still supports the old parameter names
  but they are depreciated (see script parameters.txt - v1.03)
- minor display changes

v1.09 (10 October 2003)
- utility will now prompt for a script file if run from windows explorer
  and no arguments are specified
- minor bug fixes

v1.08 (22 April 2003)
- updated the v6 decryption routines. In some cases the decrypt function would
  fail and decrypt the site code or site key using the wrong keys
- updated the scripting engine to support viewing codes and keys
  (see script parameters.txt - v1.02)

v1.07 (10 December 2002)
- support added for v6.00 keys and codes
- added /diskInformation argument, which replicates the information obtained
  and used in the HDSN/Easy Licence process
- display licenced modules (stored within master key)
- support 'allow add to existing licence' when creating site codes
- support for ranges when specifing key options
  (i.e. 1,2,3,4,5,8,9,10,14,16 ==> 1-5,8-10,14,16)
- updated scripting engine to support v6 keys and codes
  (see script parameters.txt - v1.01)
- updated code to use class structures
- enhanced method used to display information

v1.06 (04 April 2002)
- minor updates to the scripting engine

v1.05 (01 April 2002)
- scripting engine added for key creation
  (see script parameters.txt - v1.00)

v1.04 (26 January 2002)
- support for confirmation codes
- minor updates

v1.03 (07 December 2001)
- add to existing licence (for site keys)
- can create site codes
- support for v6.00 keys (early beta version)
- support for v6.00 options (early beta version)
- expanded the information displayed
  - master key: company number
  - user key: hash for v6, encrypted password & number
  - site key: licence properties & key hashes
  - site code: company number, drive serial number and crypkey version
  - entry key: expanded information titles
  - restriction key: expanded information titles
- removed the company names and account numbers to
  an external file (ckInfo.ini)
- fixes a couple of issues

v1.02 (01 April 2001)
- displays information in a clearer way
- displays further information about restriction keys
- displays information for the entry key
- can create master keys, user keys & site keys

v1.01 (22 March 2001)
- shows restriction key information

v1.00 (14 March 2001)
- original version