/* ---- CodeMeter.h ----------------------------------------------------------
 * CodeMeter Development-Kit ANSI/Microsoft C header file.
 * (C) Copyright 2002-2007 by WIBU-SYSTEMS AG
 * D-76137 Karlsruhe, Germany
 * Version 3.20 of 2007Mar01
 * ---------------------------------------------------------------------------
 */

#if !defined(_CODEMETER_INCLUDED)
#define _CODEMETER_INCLUDED

#if defined(__cplusplus)
/* CodeMeter.h contains only C declarations */
extern "C" {
#endif

#if !defined(CMAPIENTRY)
  #if defined(_WIN32)
    /* Win32 API */
    #define CMAPIENTRY __cdecl
    #define CMSTDCALL __stdcall
  #else
    /* Standard */
    #define CMAPIENTRY
    #define CMSTDCALL
  #endif
#endif /* #if !defined(CMAPIENTRY) */

#if !defined(CODEMETER_API)
  #if defined(_WIN32) && !defined(_CODEMETER_API_STATIC_LIB)
    /* Win32 API */
    #if defined(_CODEMETER_API_EXPORTS)
      /* for WibuCm32/64.dll source code */
      #define CODEMETER_API __declspec(dllexport)
    #else
      /* for source code including WibuCm32/64.dll */
     #define CODEMETER_API __declspec(dllimport)
    #endif
  #else
    /* Standard */
    #define CODEMETER_API
  #endif
#endif /* #if !defined(CODEMETER_API) */

#if !defined(_CODEMETER_NODEFLIB)
  #if defined(_WIN64)
    #if defined(_CODEMETER_API_STATIC_LIB) && !defined(_DEBUG)
      /* link static lib, only possible when release is wanted */
      #if defined(_MFC_VER)
        /* MFC variant */
        #pragma comment(lib, "WibuCm64StaticV8MD")
      #else
        /* Standard C variant */
        #pragma comment(lib, "WibuCm64StaticV8MT")
      #endif
    #else
      /* link dynamic lib/dll */
      #pragma comment(lib, "WibuCm64")
    #endif
  #elif defined(_WIN32)
    #if defined(_CODEMETER_API_STATIC_LIB) && !defined(_DEBUG)
      /* link static lib, only possible when release is wanted */
      #if defined(_MSC_VER)
        #if _MSC_VER >= 1400
          #if defined(_MFC_VER)
            /* link MFC based variant */ 
            #pragma comment(lib, "WibuCm32StaticV8MD")
          #else
            /* link Standard C based variant */ 
            #pragma comment(lib, "WibuCm32StaticV8MT")
          #endif
        #elif _MSC_VER >= 1300
          #if defined(_MFC_VER)
            /* link MFC based variant */ 
            #pragma comment(lib, "WibuCm32StaticV7MD")
          #else
            /* link Standard C based variant */ 
            #pragma comment(lib, "WibuCm32StaticV7MT")
          #endif
        #elif _MSC_VER >= 1200
          #if defined(_MFC_VER)
            /* link MFC based variant */ 
            #pragma comment(lib, "WibuCm32StaticV6MD")
          #else
            /* link Standard C based variant */ 
            #pragma comment(lib, "WibuCm32StaticV6MT")
          #endif
        #else
          /* no fitting VC compiler -> take dll */
          #pragma comment(lib, "WibuCm32")
        #endif
      #else
        /* no fitting compiler -> take dll */
        #pragma comment(lib, "WibuCm32")
      #endif
    #else
      /* link dynamic lib/dll */
      #pragma comment(lib, "WibuCm32")
    #endif
  #endif
#endif /* #if !defined(_CODEMETER_NODEFLIB) */

#if defined(_WIN32)
  #define CMUINT     unsigned int
  #define CMULONG    unsigned long
  #define CMUSHORT   unsigned short
  #define CMBYTE     unsigned char
#elif defined(_WIN64)
  #define CMUINT     unsigned int
  #define CMULONG    unsigned long
  #define CMUSHORT   unsigned short
  #define CMBYTE     unsigned char
#elif defined(__APPLE__)
  #define CMUINT     unsigned int
  #define CMULONG    unsigned int
  #define CMUSHORT   unsigned short
  #define CMBYTE     unsigned char
#elif defined(__linux__)
  #define CMUINT     unsigned int
  #define CMULONG    unsigned int
  #define CMUSHORT   unsigned short
  #define CMBYTE     unsigned char
#elif defined(__sun) || defined(__sun__) || defined(sun)
  #define CMUINT     unsigned int
  #define CMULONG    unsigned int
  #define CMUSHORT   unsigned short
  #define CMBYTE     unsigned char
#else
  #define CMUINT     unsigned int
  #define CMULONG    unsigned long
  #define CMUSHORT   unsigned short
  #define CMBYTE     unsigned char
#endif

/*****************************************************************************
 ************************* CodeMeter error codes *****************************
******************************************************************************/

/* common errors of the CodeMeter driver components */
/* common no error code is 0 */
#define CMERROR_NO_ERROR                        0
/* box specific error codes */
#define CMERROR_COMMANDUNDEFINED                1
#define CMERROR_COMMANDINVALID                  2
#define CMERROR_COMMANDTOOLARGE                 3
#define CMERROR_COMMANDWRONG                    4
#define CMERROR_COMMANDTVBWRONG                 5
#define CMERROR_COMMANDINCOMPLETE               6
#define CMERROR_AWAITENCRYPTIONRESTART          7
#define CMERROR_ANSWERBUFFERFULL                8
#define CMERROR_VALUETOOLARGE                   9
#define CMERROR_OPTIONUNDEFINED                 10
#define CMERROR_OPTIONINVALID                   11
#define CMERROR_TYPEUNDEFINED                   12
#define CMERROR_TYPEWRONG                       13
#define CMERROR_KEYSOURCEUNDEFINED              14
#define CMERROR_KEYSOURCEINVALID                15
#define CMERROR_KEYSOURCEMISSED                 16
#define CMERROR_KEYSOURCEWRONG                  17
#define CMERROR_DATATOOSHORT                    18
#define CMERROR_DATATOOLONG                     19
#define CMERROR_DATAWRONG                       20
#define CMERROR_FIRMITEMINVALID                 21
#define CMERROR_FIRMCODEEXISTS                  22
#define CMERROR_PRODUCTITEMINVALID              23
#define CMERROR_PRODUCTITEMOPTIONUNDEFINED      24
#define CMERROR_PRODUCTITEMOPTIONINVALID        25
#define CMERROR_PRODUCTITEMOPTIONMISSED         26
#define CMERROR_FIRMCODEWRONG                   27
#define CMERROR_PRODUCTCODEWRONG                28
#define CMERROR_FEATUREMAPMISSED                29
#define CMERROR_FEATUREMAPINCOMPATIBLE          30
#define CMERROR_UNITCOUNTERMISSED               31
#define CMERROR_UNITCOUNTERZERO                 32
#define CMERROR_UNITCOUNTERUNDERRUN             33
#define CMERROR_EXPIRATIONTIMEMISSED            34
#define CMERROR_EXPIRATIONTIMEOVERRUN           35
#define CMERROR_ACTIVATIONTIMEMISSED            36
#define CMERROR_ACTIVATIONTIMEUNDERRUN          37
#define CMERROR_FIRMACCESSCOUNTERZERO           38
#define CMERROR_CERTIFIEDTIMEOBSOLETE           39
#define CMERROR_ENCRYPTIONINVALID               40
#define CMERROR_DIFFIEHELLMANFAILED             41
#define CMERROR_ECIESFAILED                     42
#define CMERROR_SUBCOMMANDUNDEFINED             43
#define CMERROR_SUBCOMMANDINVALID               44
#define CMERROR_SUBCOMMANDINCOMPLETE            45
#define CMERROR_SUBCOMMANDSTRUCTUREWRONG        46
#define CMERROR_TVBWRONG                        47
#define CMERROR_FIRMUPDATECOUNTERWRONG          48
#define CMERROR_PITVBFLAGINVALID                49
#define CMERROR_PITVBFLAGINCOMPATIBLE           50
#define CMERROR_ENABLEBLOCKUSED                 51
#define CMERROR_BOXDISABLED                     52
#define CMERROR_FIRMITEMDISABLED                53
#define CMERROR_PRODUCTITEMDISABLED             54
#define CMERROR_INDEXINVALID                    55
#define CMERROR_LEVELINVALID                    56
#define CMERROR_TABLEFULL                       57
#define CMERROR_MEMORYFULL                      58
#define CMERROR_MEMORYDAMAGED                   59
#define CMERROR_QUEENMEMORYDAMAGED              60
#define CMERROR_FIRMITEMDAMAGED                 61
#define CMERROR_CRTOKENOBSOLETE                 62
#define CMERROR_MEMORYWRITEFAILED               63
#define CMERROR_BOXBUSY                         64
#define CMERROR_FEATURECODEINVALID              65
#define CMERROR_COMMANDUNSUPPORTED              66
#define CMERROR_FIELDUPDATELIMITEXCEEDED        67
#define CMERROR_FLASHWRITEFAILED                68
#define CMERROR_ACCESSMODEWRONG                 69
#define CMERROR_BOXLOST                         70
#define CMERROR_USAGEPERIODOVERRUN              71
#define CMERROR_USAGEPERIODUNDERRUN             72

#define CMERROR_BOXMEMORYDAMAGED                80
#define CMERROR_BOSSMEMORY_DAMAGED              81
#define CMERROR_FIRMWAREMEMORY_DAMAGED          82
#define CMERROR_BOSS_FIRMWAREMEMORY_DAMAGED     83
#define CMERROR_FUMMEMORY_DAMAGED               84
#define CMERROR_BOSS_FUMMEMORY_DAMAGED          85
#define CMERROR_FIRMWARE_FUMMEMORY_DAMAGED      86
#define CMERROR_BOSS_FIRMWARE_FUMMEMORY_DAMAGED 87
#define CMERROR_SYSTIMEOBSOLETE                 88
#define CMERROR_BAD_IO_MODE                     89
#define CMERROR_FUNCTIONLIMITREACHED            90

/* I2C error codes */
#define CMERROR_I2C_TIMEOUT_REQUEST             192
#define CMERROR_I2C_TIMEOUT_ANSWER              193
#define CMERROR_I2C_ANSWER_CRC                  194
#define CMERROR_I2C_TWCOMMAND_INVALID           195
#define CMERROR_I2C_PROTOCOL_VERSION            196 
#define CMERROR_I2C_COMMUNICATION               197 
#define CMERROR_I2C_REQUEST_CRC                 198 
#define CMERROR_I2C_ANSWER_BEFORE_REQUEST       199 

#define CMERROR_INTERNAL                        254
#define CMERROR_UNKNOWN                         255

/* management related error codes */
#define CMERROR_NETWORK_FAULT                   100
#define CMERROR_SERVER_NOT_FOUND                101
#define CMERROR_SEND_FAULT                      102
#define CMERROR_RECEIVE_FAULT                   103
#define CMERROR_INTERNAL_ORGANISATION           104
#define CMERROR_INVALID_PARAMETER               105
#define CMERROR_INVALID_HANDLE                  106
#define CMERROR_NO_MORE_HANDLES                 107
#define CMERROR_NO_MORE_MEMORY                  108
#define CMERROR_SECURITY_PROBLEM                109
#define CMERROR_NO_LOCAL_SERVER_STARTED         110
#define CMERROR_NO_NETWORK_SERVER               111
#define CMERROR_BUFFER_OVERFLOW                 112
#define CMERROR_BAD_ADDRESS                     113
#define CMERROR_BAD_HANDLE                      114
#define CMERROR_WRONG_HANDLE_TYPE               115
#define CMERROR_UNDEFINED_SYSTEM_HANDLE         116
#define CMERROR_NO_EVENT_OCCURRED               117
#define CMERROR_CTCS_FAULT                      118
#define CMERROR_UNKNOWN_OS                      119
#define CMERROR_NO_SECURITY_OBJECT              120
#define CMERROR_WRONG_SECURITY_OBJECT           121
#define CMERROR_NO_FSB_FUNCTIONALITY            122
#define CMERROR_NO_CTSB_FUNCTIONALITY           123
#define CMERROR_WRONG_OEMID                     124
#define CMERROR_SERVER_VERSION_TOO_OLD          125

/* box command execution related error codes */
#define CMERROR_ENTRY_NOT_FOUND                 200
#define CMERROR_BOX_NOT_FOUND                   201
#define CMERROR_CRYPTION_FAILED                 202
#define CMERROR_CRC_VERIFY_FAILED               203
#define CMERROR_NO_ENABLING_EXISTS              204
#define CMERROR_NO_PIO_SET                      205
#define CMERROR_FIRMKEY_TOO_SHORT               206
#define CMERROR_NO_BOXLOCK_NEEDED               207
#define CMERROR_BOXLOCK_NOT_CHANGED             208
#define CMERROR_NO_DATA_AVAILABLE               209
#define CMERROR_SIGNEDLIST_FAILED               210
#define CMERROR_VALIDATION_FAILED               211
#define CMERROR_NO_MORE_LICENSES                212
#define CMERROR_EXCLUSIVE_MODE_CONFLICT         213
#define CMERROR_RESERVEDFI_MISSED               214
#define CMERROR_WRONG_CONTENT                   215
#define CMERROR_FIELDUPDATE_FAILED              216
#define CMERROR_LICENSEFILE_CREATION_FAILED     217
#define CMERROR_NO_LICENSE_AVAILABLE            218
#define CMERROR_CONTEXT_FILE_WRITING_FAILED     219
#define CMERROR_UPDATE_FILE_READING_FAILED      220
#define CMERROR_NETINFO_FAILED                  221
#define CMERROR_FIELDUPDATE_RESET_FAILED        222
#define CMERROR_WRONG_ACCESS_MODE               223

/* basic error codes */
#define CMERROR_ERRORTEXT_NOT_FOUND             300
#define CMERROR_SEND_PROTECTION_FAILED          301
#define CMERROR_ANALYSING_DETECTED              302
#define CMERROR_COMMAND_EXECUTION_FAILED        303
#define CMERROR_FUNCTION_NOT_IMPLEMENTED        304


/*****************************************************************************
 **************************** Global definitions *****************************
******************************************************************************/

#define CM_CHALLENGE_LEN              16
#define CM_BLOCK_SIZE                 16
#define CM_BLOCKCIPHER_KEY_LEN        32
#define CM_DIGEST_LEN                 32
#define CM_IP_ADDRESS_LEN             8
#define CM_PRIVATE_KEY_LEN            32
#define CM_PUBLIC_KEY_LEN             64
#define CM_SESSIONID_LEN              8
#define CM_TALKKEYINPUT_LEN           80
#define CM_TVB_LEN                    16
#define CM_RSA1024_KEY_LEN            128

#define CM_MAX_COMPANY_LEN            32
#define CM_MAX_STRING_LEN             256
#define CM_MAX_PASSWORD_LEN           64

/*****************************************************************************
 **************************** Macro definitions ******************************
******************************************************************************/

/* macro setting the complete version in a CMULONG value */
#define CM_SET_VERSION(a,b,c,d) (((a) & 0xff) << 24) | (((b) & 0xff) << 16) \
    | (((c) & 0x0f) << 12) | ((d) & 0x0fff)
/* macro getting the major version from a complete version value (see CM_SET_VERSION) */
#define CM_GET_MAJOR_VERSION(v) (((v) >> 24) & 0xff)
/* macro getting the minor version from a complete version value (see CM_SET_VERSION) */
#define CM_GET_MINOR_VERSION(v) (((v) >> 16) & 0xff)
/* macro getting the build version from a complete version value (see CM_SET_VERSION) */
#define CM_GET_BUILD_VERSION(v) (((v) >> 12) & 0x0f)
/* macro getting the count version from a complete version value (see CM_SET_VERSION) */
#define CM_GET_COUNT_VERSION(v) ((v) & 0x0fff)

/*****************************************************************************
 ***************************** Type definitions ******************************
******************************************************************************/

typedef void* HCMSysEntry;

/***** 
 definition of gobal flags
*****/
/* 
  * used in mflFiCtrl of CMBOXENTRY
  * used in mflCtrl of CMCRYPTSIM
  * used in mflCtrl of CMPIOCK
  * used in mflCtrl of CMTALKKEYINPUT_USER
  * used in musFirmItemTypePlain & musFirmItemTypeEncrypted of CMPROGRAM_ADD_FIRMITEM
  * used in mulFirmItemType of CMINTERNALENTRYINFO
  * used in mulFirmItemType of CMCREATEITEM
*/
#define CM_GF_FILEBASED               0x0000
#define CM_GF_BOXBASED                0x0001
#define CM_GF_SEC_MASK                0x0001
#define CM_GF_FI_RESERVED             0x8000

/* 
  * used in flCtrl for CmCreateSequence()
  * used in flCtrl for CmProgram()
*/
#define CM_GF_ADD_FIRMITEM            0x0000
#define CM_GF_UPDATE_FIRMITEM         0x0001
#define CM_GF_DELETE_FIRMITEM         0x0002
#define CM_GF_ADD_PRODUCTITEM         0x0003
#define CM_GF_UPDATE_PRODUCTITEM      0x0004
#define CM_GF_DELETE_PRODUCTITEM      0x0005
#define CM_GF_SET_BOXCONTROL          0x0006
#define CM_GF_SET_BOXLOCK             0x0007
#define CM_GF_SET_FIRMKEY             0x0008
#define CM_GF_SET_USERKEY             0x0009
#define CM_GF_ADD_ENABLEBLOCK         0x000a
#define CM_GF_UPDATE_ENABLEBLOCK      0x000b
#define CM_GF_DELETE_ENABLEBLOCK      0x000c
#define CM_GF_ATTACH_ENABLEBLOCK      0x000d
#define CM_GF_DETACH_ENABLEBLOCK      0x000e
#define CM_GF_ACTIVATE_FIELDUPDATE    0x000f
#define CM_GF_ADD_FIELDUPDATE         0x0010
#define CM_GF_EXECUTE_FIELDUPDATE     0x001f
#define CM_GF_SET_BOXPASSWORD         0x002f
#define CM_GF_CHANGE_BOXPASSWORD      0x003f
#define CM_GF_RESET_BOXPASSWORD       0x004f
#define CM_GF_CHANGE_BOXPASSWORD_M    0x005f
#define CM_GF_ITEM_MASK               0x007f


/* 
  * used in mflSetPios of CMBOXENTRY
  * used in mflCtrl of CMENTRYDATA
  * used in musPioType of CMSECUREDATA
*/
#define CM_GF_PRODUCTCODE             0x0001
#define CM_GF_FEATUREMAP              0x0002
#define CM_GF_EXPTIME                 0x0004
#define CM_GF_ACTTIME                 0x0008
#define CM_GF_UNITCOUNTER             0x0010
#define CM_GF_PROTDATA                0x0020
#define CM_GF_EXTPROTDATA             0x0040
#define CM_GF_HIDDENDATA              0x0080
#define CM_GF_SECRETDATA              0x0100
#define CM_GF_USERDATA                0x0200
#define CM_GF_TEXT                    0x0400
#define CM_GF_USAGEPERIOD             0x0800
#define CM_GF_PIO_MASK                0x0fff

/*
  * used in mflTvbCtrl of CMCPIO_PRODUCTCODE
  * used in mflDependency of CMENTRYDATA
  * used in mfbDependencyXX of CMBOXENTRY
*/
#define CM_GF_DATA                    0x0001
#define CM_GF_FUC                     0x0002
#define CM_GF_SERIAL                  0x0004

/*****************************************************************************
 ************************** Structures declarations **************************
******************************************************************************/

#if defined(__sun__) || defined(__sun) || defined(sun)
  #pragma pack(8)
#elif defined(_WIN32) || defined(_WIN64) || (defined(__GNUC__) && __GNUC__ >= 3)
  /* set 64 Bit = 8 Byte alignment */
  #pragma pack(push, 8)
#elif defined(__GNUC__)
  #pragma pack(8)
#else
  #error "Undefined platform / Compiler. Packing not enabled!"
#endif

/*******************
 1.) Main structures
********************/

/***** CMBOXCONTROL *****/
typedef struct __CMBOXCONTROL {
  CMUSHORT musIndicatorFlags;
  CMUSHORT musSwitchFlags;
  CMULONG mulReserve;
} CMBOXCONTROL;

/***** CMBOXINFO *****/
typedef struct __CMBOXINFO {
  CMBYTE mbMajorVersion;
  CMBYTE mbMinorVersion;
  CMUSHORT musBoxMask;
  CMULONG mulSerialNumber;
  CMUSHORT musBoxKeyId;
  CMUSHORT musUserKeyId;
  CMBYTE mabBoxPublicKey[CM_PUBLIC_KEY_LEN];
  CMBYTE mabSerialPublicKey[CM_PUBLIC_KEY_LEN];
  CMULONG mulReserve;
} CMBOXINFO;

/***** CMACCESS *****/
/*
  flags for mflCtrl in CMACCESS
*/
/* algorithm flag of the CM-Box */
#define CM_VERSION1                   0x00000000
/* flags for kind of access */
#define CM_ACCESS_USERLIMIT           0x00000000
#define CM_ACCESS_NOUSERLIMIT         0x00000100
#define CM_ACCESS_EXCLUSIVE           0x00000200
#define CM_ACCESS_STATIONSHARE        0x00000300
#define CM_ACCESS_CONVENIENT          0x00000400

#define CM_ACCESS_STRUCTMASK          0x00000700
/* no validation check of the entry data */
#define CM_ACCESS_FORCE               0x00010000
/* constant for searching a fitting FSB entry */
#define CM_ACCESS_CHECK_FSB           0x00020000
/* constant for searching a fitting CTSB entry */
#define CM_ACCESS_CHECK_CTSB          0x00040000
/* allow normal subsystem access if no CM-Box is found */
#define CM_ACCESS_SUBSYSTEM           0x00080000
/* force FI access to prevent access to a FC:PC=x:0 */
#define CM_ACCESS_FIRMITEM            0x00100000

/* These option constants are used in Feature Codes which are FSB-internally
   created for a specific FSB operation (mulFeatureCode). */

/* Feature Code for the <FsbEncryptAddFi> FSB operation */
#define CM_FSBFEATURECODE_ADDFI           0x00000001
/* Feature Code for the <FsbHashBoxAlg::DeleteFi> FSB operation */
#define CM_FSBFEATURECODE_DELETEFI        0x00000002
/* Feature Code for the <FsbHashBoxAlg::UpdateFi> FSB operation */
#define CM_FSBFEATURECODE_UPDATEFI        0x00000004
/* Feature Code for the <FsbHashBoxAlg::AddPi> FSB operation */
#define CM_FSBFEATURECODE_ADDPI           0x00000008
/* Feature Code for the <FsbHashBoxAlg::DeletePi> FSB operation */
#define CM_FSBFEATURECODE_DELETEPI        0x00000010
/* Feature Code for the <FsbHashBoxAlg::UpdatePi> FSB operation */
#define CM_FSBFEATURECODE_UPDATEPI        0x00000020
/* Feature Code for the <FsbHashBoxAlg::AddEb> FSB operation */
#define CM_FSBFEATURECODE_ADDEB           0x00000040
/* Feature Code for the <FsbHashBoxAlg::DeleteEb> FSB operation */
#define CM_FSBFEATURECODE_DELETEEB        0x00000080
/* Feature Code for the <FsbHashBoxAlg::UpdateEb> FSB operation */
#define CM_FSBFEATURECODE_UPDATEEB        0x00000100
/* Feature Code for the <FsbHashBoxAlg::AttachEb> FSB operation */
#define CM_FSBFEATURECODE_ATTACHEB        0x00000200
/* Feature Code for the <FsbHashBoxAlg::GetTalkKeyInput> FSB operation */
#define CM_FSBFEATURECODE_GETTALKKEYINPUT 0x00000400
/* Feature Code for the <FsbHashBoxAlg::GetBoxTimeSign> FSB operation */
#define CM_FSBFEATURECODE_GETBOXTIMESIGN  0x00002000
/* Feature Code for the <FsbHashBoxAlg::DeleteFiSign> FSB operation */
#define CM_FSBFEATURECODE_DELETEFISIGN    0x00004000
/* Feature Code for the <FsbHashBoxAlg::ListSign> FSB operation */
#define CM_FSBFEATURECODE_LISTSIGN        0x00008000
/* Feature Code for the <FsbHashBoxAlg::Encrypt> FSB operation */
#define CM_FSBFEATURECODE_ENCRYPT         0x00010000
/* Feature Code for the <FsbPriceDeduct> FSB operation */
#define CM_FSBFEATURECODE_PRICEDEDUCT     0x00020000

typedef struct __CMACCESS {
  CMULONG mflCtrl;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulFeatureCode;
  CMULONG mulUsedRuntimeVersion;
  CMULONG midProcess;
  CMUSHORT musProductItemReference;
  CMUSHORT musSession;
  unsigned char    mabIPv4Address[4];
  CMBOXINFO mcmBoxInfo;
} CMACCESS;

/***** CMAUTHENTICATE *****/
/*
  flags for mflCtrl in CMAUTHENTICATE
*/
/* Key Source specification */
#define CM_AUTH_FIRMKEY               0x00000000
#define CM_AUTH_HIDDENDATA            0x00000001
#define CM_AUTH_SECRETDATA            0x00000002
#define CM_AUTH_SERIALKEY             0x00000003
#define CM_AUTH_BOXKEY                0x00000004
#define CM_AUTH_KSMASK                0x00000007
/* Authentication mode specification */
#define CM_AUTH_DIRECT                0x00000000
#define CM_AUTH_EXTENDED              0x00010000
#define CM_AUTH_SERIAL                0x00020000
#define CM_AUTH_AMMASK                0x00030000

typedef struct __CMAUTHENTICATE {
  CMULONG mflCtrl;
  CMULONG mulKeyExtType;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulEncryptionCodeOptions;
  CMULONG mulFeatureCode;
  CMBOXINFO mcmBoxInfo;
  CMBYTE mabDigest[CM_DIGEST_LEN];
} CMAUTHENTICATE;

/***** CMTIME *****/
typedef struct __CMTIME {
  CMUSHORT musYear;
  CMUSHORT musMonth;
  CMUSHORT musDay;
  CMUSHORT musHours;
  CMUSHORT musMinutes;
  CMUSHORT musSeconds;
  CMULONG mulSecondsSince01_01_2000;
} CMTIME;

/***** CMBOXENTRY *****/
/*
 global flags used for mflFiCtrl, mflSetPios & mfbDependencyXX of CMBOXENTRY
*/
typedef struct __CMBOXENTRY {
  /* Firm Item data */
  CMULONG mflFiCtrl;
  CMULONG mulFirmCode;
  CMUSHORT musFirmAccessCounter;
  CMUSHORT musReserve1;
  CMULONG mulFirmUpdateCounter;
  CMULONG mulFirmPreciseTime;
  CMUSHORT mausFirmItemText[CM_MAX_STRING_LEN];
  /* flag specified the set ProductItem Option */
  CMULONG mflSetPios;
  /* Product Item data */
  CMULONG mulProductCode;
  CMULONG mulFeatureMap;
  CMULONG mulUnitCounter;
  CMTIME mcmExpirationTime;
  CMTIME mcmActivationTime;
  CMBYTE mfbDependencyPC;
  CMBYTE mfbDependencyFM;
  CMBYTE mfbDependencyUC;
  CMBYTE mfbDependencyET;
  CMBYTE mfbDependencyAT;
  CMBYTE mabReserve2[3];
} CMBOXENTRY;

/***** CMBOXSECURITY *****/
typedef struct __CMBOXSECURITY {
  CMULONG midOem;
  CMULONG mulFsbFirmCode;
  CMULONG mulCtsbFirmCode;
  CMULONG mulReserve;
} CMBOXSECURITY;

/***** CMBOXTIME *****/
typedef struct __CMBOXTIME {
  CMTIME mcmCertifiedTime;
  CMTIME mcmBoxTime;
  CMTIME mcmSystemTime;
} CMBOXTIME;

/***** CMUSAGEPERIOD *****/
typedef struct __CMUSAGEPERIOD {
  CMULONG mulPeriod;
  CMTIME mcmStartTime;
} CMUSAGEPERIOD;

/***** CMBASECRYPT *****/

/*
  flags for mflCtrl in CMBASECRYPT
*/
/* Key Source specification */
#define CM_CRYPT_FIRMKEY              0x00000000
#define CM_CRYPT_HIDDENDATA           0x00000001
#define CM_CRYPT_SECRETDATA           0x00000002
#define CM_CRYPT_KSMASK               0x00000003
/* CM-Stick encryption algorithms */
#define CM_CRYPT_AES                  0x00000000
#define CM_CRYPT_ECIES                0x01000000
#define CM_CRYPT_ECIES_STD            0x02000000
#define CM_CRYPT_AES_DIRECT           0x03000000
#define CM_CRYPT_RSA                  0x05000000
#define CM_CRYPT_ALGMASK              0xff000000
/* flags for CRC calculation and checking */
#define CM_CRYPT_CHKCRC               0x00010000
#define CM_CRYPT_CALCCRC              0x00020000

/*
  flags for mulEncryptionCodeOptions in CMBASECRYPT
*/
#define CM_CRYPT_RES1MASK             0xC0000000
#define CM_CRYPT_UCMASK               0x30000000
#define CM_CRYPT_UCCHECK              0x00000000
#define CM_CRYPT_UCIGNORE             0x20000000
#define CM_CRYPT_UCREQUIRED           0x10000000
#define CM_CRYPT_ATMASK               0x0C000000
#define CM_CRYPT_ATCHECK              0x00000000
#define CM_CRYPT_ATIGNORE             0x08000000
#define CM_CRYPT_ATREQUIRED           0x04000000
#define CM_CRYPT_ETMASK               0x03000000
#define CM_CRYPT_ETCHECK              0x00000000
#define CM_CRYPT_ETIGNORE             0x02000000
#define CM_CRYPT_ETREQUIRED           0x01000000
#define CM_CRYPT_CERTTIME             0x00800000
#define CM_CRYPT_FACDECREMENT         0x00400000
#define CM_CRYPT_TOPLAINONLY          0x00200000
#define CM_CRYPT_RES2MASK             0x00300000
#define CM_CRYPT_SAMASK               0x000F0000
#define CM_CRYPT_SAEXCLUSIVE          0x00080000
#define CM_CRYPT_SAUNLIMITED          0x00000000
#define CM_CRYPT_SAUSERLIMIT          0x00040000
#define CM_CRYPT_SASTATIONSHARE       0x00020000
#define CM_CRYPT_RES3MASK             0x0000C000
#define CM_CRYPT_UCDELTAMASK          0x00003FFF
#define CM_CRYPT_MAX                  0xFFFFFFFF

typedef struct __CMBASECRYPT {
  CMULONG mflCtrl;
  CMULONG mulKeyExtType;
  CMULONG mulEncryptionCode;
  CMULONG mulEncryptionCodeOptions;
  CMULONG mulFeatureCode;
  CMULONG mulCrc;
} CMBASECRYPT;

/***** CMCRYPT *****/
typedef struct __CMCRYPT {
  CMBASECRYPT mcmBaseCrypt;
  CMBYTE mabInitKey[CM_BLOCK_SIZE];
} CMCRYPT;

/***** CMCRYPTSIM *****/
/*
  global flags used in mflCtrl of CMCRYPTSIM
*/
typedef struct __CMCRYPTSIM {
  CMULONG mflCtrl;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMBASECRYPT mcmBaseCrypt;
  CMBYTE mabInitKey[CM_BLOCK_SIZE];
  CMULONG mcbExtFirmKey;
  CMBYTE mabExtFirmKey[CM_BLOCKCIPHER_KEY_LEN];
} CMCRYPTSIM;

/***** CMSECUREDATA *****/
/*
 global flags also used for musPioType and musKeyExtType of CMSECUREDATA
*/
typedef struct __CMSECUREDATA {
  CMBASECRYPT mcmBaseCrypt;
  CMUSHORT musPioType;
  CMUSHORT musExtType;
  CMBYTE mabPioEncryptionKey[CM_BLOCK_SIZE];
  CMULONG mulReserve;
} CMSECUREDATA;

/***** CMPIOCK *****/
/*
  global flags used in mflCtrl of CMPIOCK
*/
typedef struct __CMPIOCK {
  CMSECUREDATA mcmSecureData;
  CMULONG mflCtrl;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulHiddenDataAccessCode;
  CMULONG mcbExtFirmKey;
  CMBYTE mabExtFirmKey[CM_BLOCKCIPHER_KEY_LEN];
} CMPIOCK;

/***** CMCHIPINFO *****/
typedef struct __CMCHIPINFO {
  CMULONG mulFirmwareBuild;
  CMULONG mulFirmwareBuildTop;
  CMULONG mulMdfaLba;
  CMULONG mulReserved;
  CMUSHORT musChipType;
  CMUSHORT musSiliconRevision;
  CMUSHORT midFactory;
  CMUSHORT musProductYear;
  CMUSHORT musLotNumber;
  CMUSHORT midWafer;
  CMUSHORT midChipOnWafer;
  CMUSHORT musDowngradeCount;
} CMCHIPINFO;

/***** CMUSBCHIPINFO *****/
typedef struct __CMUSBCHIPINFO {
  CMUSHORT musFirmwareMajor;
  CMUSHORT musFirmwareMinor;
  CMULONG  mulFlashSize;
  char mszNodeDescription[CM_MAX_STRING_LEN];
  CMBYTE mszNodes[CM_MAX_STRING_LEN];
} CMUSBCHIPINFO;

/***** CMNETINFOCLUSTER *****/
typedef struct __CMNETINFOCLUSTER {
  CMUINT mulSerial;
  CMUSHORT musMask;
  CMUSHORT musProductItemRef;
  CMUINT mulFirmCode;
  CMUINT mulProductCode;
  CMUINT mulFeatureMap;
  CMUINT mulUserLimitLicenses;
  CMUINT mulNoUserLimitLicenses;
  CMUINT mulExclusiveLicenses;
  CMUINT mulStationShareLicenses;
  CMUINT mulUsedLicenses;
  CMUINT mulFreeLicenses;
  CMUINT mulTotalLicenses;
  CMUINT mulLicenseQuantity;
  CMUINT mulReserved1;
} CMNETINFOCLUSTER;

/***** CMNETINFOUSER *****/
typedef struct __CMNETINFOUSER {
  CMUINT mulSerial;
  CMUSHORT musMask;
  CMUSHORT musProductItemRef;
  CMUINT mulFirmCode;
  CMUINT mulProductCode;
  CMUINT mulFeatureMap;
  CMUINT mulID;
  CMUINT mulAccessMode;
  CMUINT mulReserved;
  char mszClientIP[32];
} CMNETINFOUSER;

/***** CMINTERNALENTRYINFO *****/
typedef struct __CMINTERNALENTRYINFO {
  CMUSHORT musFirmItemReference;
  CMUSHORT musProductItemReference;
  CMULONG mulFirmItemType;
  CMULONG mulFirmUpdateCounter;
  CMULONG mulReserve;
} CMINTERNALENTRYINFO;

/***** CMENTRYDATA *****/
/*
 global flags used for mflCtrl &  mflDependency of CMENTRYDATA
*/

typedef struct __CMENTRYDATA {
  CMULONG mflCtrl;
  CMULONG mflDependency;
  CMULONG mulReserve;
  CMUINT mcbData;
  CMBYTE mabData[2 * CM_MAX_STRING_LEN];
} CMENTRYDATA;

/***** CMMEMINFO *****/
typedef struct __CMMEMINFO {
  CMUSHORT mcFree4ByteBlock;
  CMUSHORT mcFree8ByteBlock;
  CMUSHORT mcFree16ByteBlock;
  CMUSHORT mcFree32ByteBlock;
  CMUSHORT mcFree64ByteBlock;
  CMUSHORT mcFree128ByteBlock;
  CMUSHORT mcFree256ByteBlock;
  CMUSHORT mcFree512ByteBlock;
} CMMEMINFO;

/***** CMLICENSEINFO *****/
/*
  flags for mflCtrl in CMLICENSEINFO
*/
#define CM_LICENSE_FILEBASED          0x00000001
#define CM_LICENSE_BOXBASED           0x00000002
#define CM_LICENSE_FILEBASEDCREATE    0x00010000
#define CM_LICENSE_FIRMKEY            0x00020000
#define CM_LICENSE_PUBLICFIRMKEY      0x00040000
#define CM_LICENSE_FSBUPDATEKEY       0x00080000
#define CM_LICENSE_ENCRYPTED          0x00100000
#define CM_LICENSE_BOXBASEDFSB        0x00200000

typedef struct __CMLICENSEINFO {
  CMULONG mflCtrl;
  CMULONG mulFirmCode;
  CMULONG midOem;
  CMULONG mulFsbFirmCode;
  CMULONG mulFsbProductCode;
  CMULONG mulReserve;
  char mszDescription[CM_MAX_STRING_LEN];
  char mszFirmItemText[CM_MAX_STRING_LEN];
} CMLICENSEINFO;

/***** CMRESERVEFI *****/
typedef struct __CMRESERVEFI {
  CMUSHORT musFirmItemRef;
  CMBYTE mabSessionId[2 * CM_SESSIONID_LEN];
  CMBYTE mabReserve[6];
} CMRESERVEFI;

/***** CMSIGNEDTIME *****/
typedef struct __CMSIGNEDTIME {
  CMBOXTIME mcmBoxTime;
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
} CMSIGNEDTIME;

/***** CMSYSTEM *****/
/*
  flags for midPlatform in CMVERSION
*/
#define CM_SYSTEM_W95                 0x00000000
#define CM_SYSTEM_W98                 0x00000001
#define CM_SYSTEM_WME                 0x00000002
#define CM_SYSTEM_NT4                 0x00000003
#define CM_SYSTEM_W2K                 0x00000004
#define CM_SYSTEM_WXP                 0x00000005
#define CM_SYSTEM_W2003               0x00000006
#define CM_SYSTEM_VISTA               0x00000007

#define CM_SYSTEM_WINDOWS             0x00000000
#define CM_SYSTEM_MACOSX              0x00000100
#define CM_SYSTEM_SOLARIS             0x00000200
#define CM_SYSTEM_WIN_CE              0x00001000
#define CM_SYSTEM_LINUX               0x00010000

#define CM_SYSTEM_PLATFORM_MASK       0x000FFF00
#define CM_SYSTEM_VERSION_MASK        0x000000FF

typedef struct __CMSYSTEM {
  CMULONG midPlatform;
  CMULONG mulSystemKernelVersion;
  CMUSHORT mausIpAddress[CM_IP_ADDRESS_LEN];
  char mszComputerName[CM_MAX_STRING_LEN];
} CMSYSTEM;

/***** CMTALKKEYINPUT_USER *****/
/*
 global flags also used for mflCtrl of CMTALKKEYINPUT_USER
*/
#define CM_TKI_FUC                    0x00010000
#define CM_TKI_SERIAL                 0x00020000

#define CM_TKI_DATAENCRYPT            0x10000000

typedef struct __CMTALKKEYINPUT_USER {
  /* input parameters */
  CMULONG mflCtrl;
  CMULONG mulToken;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulFirmUpdateCounter;
  CMULONG mulReserve;
  CMBOXINFO mcmBoxInfo;
  /* in-/output parameters */
  CMBYTE mabTalkKeyInput[CM_TALKKEYINPUT_LEN];
} CMTALKKEYINPUT_USER;

/***** CMTALKKEYINPUT_LICENSOR *****/
typedef struct __CMTALKKEYINPUT_LICENSOR {
  CMBYTE mabPrivateLicensorKey[CM_PRIVATE_KEY_LEN];
  CMBYTE mabPublicLicensorKey[CM_PUBLIC_KEY_LEN];
  CMBYTE mabSessionId[CM_SESSIONID_LEN];
} CMTALKKEYINPUT_LICENSOR;

/***** CMTALKKEY *****/
typedef struct __CMTALKKEY {
  CMBYTE mabTalkKey[CM_BLOCK_SIZE];
  CMBYTE mabSessionId[CM_SESSIONID_LEN];
} CMTALKKEY;

/***** CMVERSION *****/
typedef struct __CMVERSION {
  CMUSHORT musVersion;
  CMUSHORT musSubVersion;
  CMUSHORT musBuild;
  CMUSHORT musCount;
  CMUSHORT musYear;
  CMUSHORT musMonth;
  CMUSHORT musDay;
  CMUSHORT musReserve;
} CMVERSION;

/**********************
 2.) Special structures
***********************/

/****************************
 2.1.) Programming Structures
*****************************/

/***** CMCREATEITEM *****/
typedef struct __CMCREATEITEM {
  /* must be set for all program commands */
  CMULONG mulFirmCode;
  CMULONG mulFirmItemType;
  CMBOXINFO mcmBoxInfoUser;
  /* must be set for all program commands except AddFi */
  CMULONG mulFirmUpdateCounter;
  /* must be set for UpdatePi & DeletePi */
  CMULONG mulProductCode;
  /* must be set for BoxLock */
  CMULONG midOem;
  /* must be set for ActFieldUpd */
  CMULONG mulFirmwareBuild;
} CMCREATEITEM;

/***** CMCPIO_PRODUCTCODE *****/
/*
 global flags also used for mflTvbCtrl of CMCPIO_PRODUCTCODE
 additional flags for mflTvbCtrl of CMCPIO_PRODUCTCODE
*/
#define CM_PIO_CHANGE_PC              0x00010000

typedef struct __CMCPIO_PRODUCTCODE {
  CMULONG mflTvbCtrl;
  CMUSHORT musFirmItemReference;
  CMUSHORT musProductItemReference;
  CMULONG mulProductCode;
  CMULONG mulReserve;
} CMCPIO_PRODUCTCODE;

/***** CMCPIO_EXTPROTDATA *****/
typedef struct __CMCPIO_EXTPROTDATA {
  CMUSHORT musExtType;
  CMUSHORT mcbData;
  CMULONG mulReserve;
  /* variable length, max = 256 */
  CMBYTE mabData[CM_MAX_STRING_LEN];
} CMCPIO_EXTPROTDATA;

/***** CMCPIO_HIDDENDATA *****/
typedef struct __CMCPIO_HIDDENDATA {
  CMUSHORT musExtType;
  CMUSHORT mcbTotal;
  CMUSHORT mcbData;
  CMBYTE mabReserve[6];
  CMULONG mulHiddenDataAccessCode;
  /* variable length, max = 256 */
  CMBYTE mabData[CM_MAX_STRING_LEN];
} CMCPIO_HIDDENDATA;

/***** CMCPIO_PROTDATA *****/
typedef struct __CMCPIO_PROTDATA {
  CMUSHORT mcbData;
  /* variable length, max = 256 */
  CMBYTE mabData[CM_MAX_STRING_LEN];
  CMBYTE mabReserve[6];
} CMCPIO_PROTDATA;

/***** CMCPIO_SECRETDATA *****/
typedef struct __CMCPIO_SECRETDATA {
  CMUSHORT musExtType;
  CMUSHORT mcbTotal;
  CMUSHORT mcbData;
  CMUSHORT musReserve;
  /* variable length, max = 256 */
  CMBYTE mabData[CM_MAX_STRING_LEN];
} CMCPIO_SECRETDATA;

/***** CMCPIO_TEXT *****/
typedef struct __CMCPIO_TEXT {
  CMUSHORT mcchText;
  CMBYTE mabReserve[6];
  /* variable length, max = 256 */
  CMUSHORT mausText[CM_MAX_STRING_LEN];
} CMCPIO_TEXT;

/***** CMCPIO_USERDATA *****/
typedef struct __CMCPIO_USERDATA {
  CMUSHORT mcbData;
  CMBYTE mabReserve[6];
  /* variable length, max = 256 */
  CMBYTE mabData[CM_MAX_STRING_LEN];
} CMCPIO_USERDATA;

/***** CMPROGRAM_BOXCONTROL *****/
/*
  flags for mflCtrl in CMPROGRAM_BOXCONTROL
*/
#define CM_BC_ABSOLUTE                0x0000
#define CM_BC_SWITCH                  0x0001

typedef struct __CMPROGRAM_BOXCONTROL {
  CMULONG mflCtrl;
  CMUSHORT musIndicatorFlags;
  CMUSHORT musReserve;
} CMPROGRAM_BOXCONTROL;

/***** CMPROGRAM_BOXLOCK *****/
/*
  flags for mflCtrl in CMPROGRAM_BOXLOCK
*/
#define CM_BL_LOCK                    0x0000
#define CM_BL_UNLOCK                  0x0001

typedef struct __CMPROGRAM_BOXLOCK {
  CMULONG mflCtrl;
  CMULONG mulReserve;
  CMBYTE mabUserKey[CM_BLOCKCIPHER_KEY_LEN];
} CMPROGRAM_BOXLOCK;

/***** CMPROGRAM_USERKEY *****/
/*
  flags for mflCtrl in CMPROGRAM_USERKEY
*/
#define CM_UUK_USEUCK                 0x0000
#define CM_UUK_USEUIK                 0x0001
#define CM_UUK_USEMASK                0x0001
#define CM_UUK_SETUCK                 0x0000
#define CM_UUK_SETUIK                 0x0002
#define CM_UUK_SETMASK                0x0002

typedef struct __CMPROGRAM_USERKEY {
  /* plain members */
  CMUSHORT musReserved;
  CMUSHORT musCtrl;
  /* encrypted members */
  CMUSHORT musCtrlEncr;
  CMBYTE mabReserved[14];
  CMBYTE mabUserKey[CM_BLOCKCIPHER_KEY_LEN];
  CMBYTE mabTvb[CM_TVB_LEN];
} CMPROGRAM_USERKEY;

/***** CMPROGRAM_BOXPASSWORD *****/
typedef struct __CMPROGRAM_BOXPASSWORD {
  char mszOldPassword[CM_MAX_PASSWORD_LEN];
  char mszNewPassword[CM_MAX_PASSWORD_LEN];
} CMPROGRAM_BOXPASSWORD;

/***** CMVALIDATE_DELETEFI *****/
typedef struct __CMVALIDATE_DELETEFI {
  CMULONG mflFiCtrl;
  CMULONG mulFirmCode;
  CMULONG mulSystemTime;
  CMULONG mulFirmUpdateCounter;
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
} CMVALIDATE_DELETEFI;

#if defined(__sun__) || defined(__sun) || defined(sun)
  #pragma pack(1)
#elif defined(_WIN32) || defined(_WIN64) || (defined(__GNUC__) && __GNUC__ >= 3)
  /* set 1 Byte alignment, only for programming & enabling structures */
  #pragma pack(pop)
  #pragma pack(push, 1)
#elif defined(__GNUC__)
  #pragma pack(1)
#else
  #error "Undefined platform / Compiler. Packing not enabled!"
#endif

/***** CMPROGRAM_ADD_FIRMITEM *****/
/*
 global flags used for musFirmItemTypePlain
 & musFirmItemTypeEncrypted of CMPROGRAM_ADD_FIRMITEM
*/
typedef struct __CMPROGRAM_ADD_FIRMITEM {
  /* plain members */
  CMUSHORT musFirmItemReference;
  CMUSHORT musFirmItemTypePlain;
  CMBYTE mabPublicLicensorKey[CM_PUBLIC_KEY_LEN];
  /* encrypted members */
  CMUSHORT musFirmItemTypeEncrypted;
  CMUSHORT musFirmAccessCounter;
  CMULONG mulFirmUpdateCounter;
  CMULONG mulFirmPreciseTime;
  CMULONG mulReserved;
  CMBYTE mabFirmKey[CM_BLOCKCIPHER_KEY_LEN];
  CMBYTE mabSessionId[2 * CM_SESSIONID_LEN];
  /* plain members */
  CMUSHORT mcchText;
  /* variable length, max = 256 */
  CMUSHORT mausText[CM_MAX_STRING_LEN];
} CMPROGRAM_ADD_FIRMITEM;

/***** CMPROGRAM_UPDATE_FIRMITEM *****/
/*
  flags for musCtrl in CMPROGRAM_UPDATE_FIRMITEM
*/
#define CM_UFI_FAC                    0x0001
#define CM_UFI_FUC                    0x0002
#define CM_UFI_FPT                    0x0004
#define CM_UFI_TEXT                   0x0008
#define CM_UFI_MASK                   0x000f

typedef struct __CMPROGRAM_UPDATE_FIRMITEM {
  CMUSHORT musFirmItemReference;
  CMUSHORT musCtrl;
  CMUSHORT musFirmAccessCounter;
  CMULONG mulFirmUpdateCounter;
  CMULONG mulFirmPreciseTime;
  CMUSHORT mcchText;
  /* variable length, max = 256, expand structure on the stack */
  CMUSHORT mausText[1];
} CMPROGRAM_UPDATE_FIRMITEM;

/***** CMPROGRAM_DELETE_FIRMITEM *****/
typedef struct __CMPROGRAM_DELETE_FIRMITEM {
  CMUSHORT musFirmItemReference;
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
} CMPROGRAM_DELETE_FIRMITEM;

/***** CMPROGRAM_ADD_PRODUCTITEM *****/
typedef struct __CMPROGRAM_ADD_PRODUCTITEM {
  CMUSHORT musFirmItemReference;
  CMUSHORT musProductItemSuccessorReference;
  /* variable length, expand structure on the stack */
  CMBYTE mabProductItemOptionBuffer[1];
} CMPROGRAM_ADD_PRODUCTITEM;

/***** CMPROGRAM_UPDATE_PRODUCTITEM *****/
/*
  flags for musCtrl set internally!!!
*/
typedef struct __CMPROGRAM_UPDATE_PRODUCTITEM {
  CMUSHORT musFirmItemReference;
  CMUSHORT musProductItemReference;
  CMUSHORT musCtrl;
  /* variable length, expand structure on the stack */
  CMBYTE mabProductItemOptionBuffer[1];
} CMPROGRAM_UPDATE_PRODUCTITEM;

/***** CMPROGRAM_DELETE_PRODUCTITEM *****/
typedef struct __CMPROGRAM_DELETE_PRODUCTITEM {
  CMUSHORT musFirmItemReference;
  CMUSHORT musProductItemReference;
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
} CMPROGRAM_DELETE_PRODUCTITEM;

/***** CMACTIVATE_FIELDUPDATE *****/
typedef struct __CMACTIVATE_FIELDUPDATE {
  CMUSHORT musCtrl;
  CMBYTE mabFieldUpdateSeqHash[CM_BLOCKCIPHER_KEY_LEN];
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
  CMBYTE mabUserTrailingValidationBlock[CM_TVB_LEN];
} CMACTIVATE_FIELDUPDATE;

/*************************
 2.2.) Enabling Structures
**************************/

/***** CMENABLING_SIMPLEPIN *****/
typedef struct __CMENABLING_SIMPLEPIN {
  CMBYTE mcbEnableAccessCode;
  /* variable length, max = 16 */
  CMBYTE mabEnableAccessCode[CM_CHALLENGE_LEN];
} CMENABLING_SIMPLEPIN;

/***** CMENABLING_TIMEPIN *****/
typedef struct __CMENABLING_TIMEPIN {
  CMULONG mulDisableTime;
  CMBYTE mcbEnableAccessCode;
  CMUSHORT mcchText;
  /* variable length, AccessCode + Text added, expand structure on the stack */
  CMBYTE mabBuffer[1];
} CMENABLING_TIMEPIN;

/***** CMENABLING_SMARTCARD *****/
typedef struct __CMENABLING_SMARTCARD {
  CMULONG mulDisableTime;
  CMUSHORT musIdentity;
  CMBYTE mabEnableAccessCode[CM_CHALLENGE_LEN];
} CMENABLING_SMARTCARD;

/***** CMENABLING_FINGERPIN *****/
/* FingerPin has the same parameters as TimePin */
typedef CMENABLING_TIMEPIN CMENABLING_FINGERPIN;

/***** CMENABLING_SIMPLEPIN_LIST *****/
typedef struct __CMENABLING_SIMPLEPIN_LIST {
  CMBYTE mcbEnableAccessCode;
} CMENABLING_SIMPLEPIN_LIST;

/***** CMENABLING_TIMEPIN_LIST *****/
typedef struct __CMENABLING_TIMEPIN_LIST {
  CMULONG mulDisableTime;
  CMBYTE mcbEnableAccessCode;
  CMUSHORT mcchText;
  CMUSHORT mausText[CM_MAX_STRING_LEN];
} CMENABLING_TIMEPIN_LIST;

/***** CMENABLING_SMARTCARD_LIST *****/
typedef struct __CMENABLING_SMARTCARD_LIST {
  CMULONG mulDisableTime;
  CMUSHORT musIdentity;
} CMENABLING_SMARTCARD_LIST;

/***** CMENABLING_FINGERPINLIST *****/
/* FingerPin has the same parameters as TimePin */
typedef CMENABLING_TIMEPIN_LIST CMENABLING_FINGERPIN_LIST;

/***** CMENABLING_ENABLEBLOCKITEM *****/
/*
  flags for musOption in CMENABLING_ENABLEBLOCKITEM
*/
#define CM_EBI_VALID                  0x8000
#define CM_EBI_DISABLED               0x0001
#define CM_EBI_TEMPENABLED            0x0002

/*
  flags for mbType in CMENABLING_ENABLEBLOCKITEM
*/
#define CM_EBI_SIMPLEPIN              0x0000
#define CM_EBI_TIMEPIN                0x0010
#define CM_EBI_SMARTCARD              0x0020
#define CM_EBI_FINGERPIN              0x0030

typedef struct __CMENABLING_ENABLEBLOCKITEM {
  CMUSHORT musOption;
  CMBYTE mbIndex;
  CMBYTE mbType;
  union {
    /* variable length */
    CMENABLING_SIMPLEPIN mcmSimplePin;
    CMENABLING_TIMEPIN mcmTimePin;
    CMENABLING_SMARTCARD mcmSmartCard;
    CMENABLING_FINGERPIN mcmFingerPin;
  } mEnableType;
} CMENABLING_ENABLEBLOCKITEM;

/***** CMENABLING_ENABLEBLOCKITEM_LIST *****/
typedef struct __CMENABLING_ENABLEBLOCKITEM_LIST {
  CMUSHORT musOption;
  CMBYTE mbIndex;
  CMBYTE mbType;
  union {
    /* variable length */
    CMENABLING_SIMPLEPIN_LIST mcmSimplePin;
    CMENABLING_TIMEPIN_LIST mcmTimePin;
    CMENABLING_SMARTCARD_LIST mcmSmartCard;
    CMENABLING_FINGERPIN_LIST mcmFingerPin;
  } mEnableType;
} CMENABLING_ENABLEBLOCKITEM_LIST;

/***** CMENABLING_ENABLELOOKUPENTRY *****/
/*
  flags for musOption in CMENABLING_ENABLELOOKUPENTRY
*/
#define CM_ELUE_VALID                 0x8000
#define CM_ELUE_IFI                   0x0001
#define CM_ELUE_REQUIRED              0x0002

/*
  flags for mbEnableLevel & mbDisableLevel in CMENABLING_ENABLELOOKUPENTRY
*/
#define CM_ELUE_LOCATE                0x0000
#define CM_ELUE_READ                  0x0001
#define CM_ELUE_ENCRYPT               0x0003
#define CM_ELUE_UNITUSE               0x0005
#define CM_ELUE_MODIFY                0x0007
#define CM_ELUE_DETACH                0x00ff

typedef struct __CMENABLING_ENABLELOOKUPENTRY {
  CMUSHORT musOption;
  CMBYTE mbEnableBlockIndex;
  CMBYTE mbEnableLevel;
  CMBYTE mbDisableLevel;
  CMBYTE mabReserved[3];
} CMENABLING_ENABLELOOKUPENTRY;

/***** CMENABLING_WRITEADD *****/
typedef struct __CMENABLING_WRITEADD {
  /* plain members */
  CMULONG mcbLength;
  CMUSHORT musFirmItemReference;
  /* encrypted members */
  CMENABLING_ENABLEBLOCKITEM mcmEnableBlockItem;
} CMENABLING_WRITEADD;

/***** CMENABLING_WRITEUPDATE *****/
/*
  flags for mbEnableBlockIndex in CMENABLING_WRITEUPDATE
*/
#define CM_WU_FIRMKEY                 0x80
/*
  flags for musCtrl in CMENABLING_WRITEUPDATE
*/
#define CM_WU_DISABLED                0x0001
#define CM_WU_TEMPENABLED             0x0002
#define CM_WU_ACCESSCODE              0x0004
#define CM_WU_DISABLETIME             0x0008
#define CM_WU_TEXT                    0x0010
#define CM_WU_IDENTITY                0x0020
#define CM_WU_FULL                    0x003f

typedef struct __CMENABLING_WRITEUPDATE {
  /* plain members */
  CMBYTE mbEnableBlockIndex;
  /* encrypted members */
  CMUSHORT musCtrl;
  CMULONG mulFirmUpdateCounter;
  CMENABLING_ENABLEBLOCKITEM mcmEnableBlockItem;
} CMENABLING_WRITEUPDATE;

/***** CMENABLING_WRITEATTACHDETACH *****/
typedef struct __CMENABLING_WRITEATTACHDETACH {
  /* plain members */
  CMBYTE mbEnableBlockIndex;
  /* encrypted members (32 Bytes) */
  CMUSHORT musFirmItemReference;
  CMUSHORT musProductItemReference;
  CMULONG mulFirmUpdateCounter;
  CMENABLING_ENABLELOOKUPENTRY mcmEnableLookupEntry;
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
} CMENABLING_WRITEATTACHDETACH;

/***** CMENABLING_WRITEDELETE *****/
typedef struct __CMENABLING_WRITEDELETE {
  /* plain members */
  CMUSHORT musFirmItemReference;
  CMBYTE mbEnableBlockIndex;
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
} CMENABLING_WRITEDELETE;

#if defined(__sun__) || defined(__sun) || defined(sun)
  #pragma pack(8)
#elif defined(_WIN32) || defined(_WIN64) || (defined(__GNUC__) && __GNUC__ >= 3)
  /* reset to original alignment */
  #pragma pack(pop)
  #pragma pack(push, 8)
#elif defined(__GNUC__)
  #pragma pack(8)
#else
  #error "Undefined platform / Compiler. Packing not enabled!"
#endif

/***** CMENABLING_APPCONTEXT *****/
/*
  flags for mflCtrl in CMENABLING_APPCONTEXT
*/
#define CM_AC_VALID                   0x8000
#define CM_AC_DISABLED                0x0001
#define CM_AC_TEMPENABLED             0x0002

typedef struct __CMENABLING_APPCONTEXT {
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMBOXINFO mcmBoxInfo;
  CMULONG mflCtrl;
  CMUSHORT midEnableBlockItem;
  CMUSHORT musReserve;
  CMULONG mulFirmUpdateCounter;
  CMULONG midIdentity;
  char mszCompanyName[CM_MAX_COMPANY_LEN];
} CMENABLING_APPCONTEXT;

/***********************************
 2.3.) Remote Programming Structures
************************************/

/***** CMSERIAL *****/
typedef struct __CMSERIAL {
  CMUINT mulSerial;
  CMUSHORT musMask;
  CMUSHORT musReserved;
} CMSERIAL;

#if defined(__sun__) || defined(__sun) || defined(sun)
  /* no reset of alignment available */
#elif defined(_WIN32) || defined(_WIN64) || (defined(__GNUC__) && __GNUC__ >= 3)
  /* reset to original alignment */
  #pragma pack(pop)
#elif defined(__GNUC__)
  /* no reset of alignment available */
#else
  #error "Undefined platform / Compiler. Packing not enabled!"
#endif

/*****************************************************************************
 ************************* Functions predeclarations *************************
******************************************************************************/

#if defined(_CODEMETER_API_STATIC_LIB)

/* initialization routine for the static lib */
CODEMETER_API void CMAPIENTRY CmStaticLibOnInit();

/* cleanup routine for the static lib */
CODEMETER_API void CMAPIENTRY CmStaticLibOnExit();

#endif

/***************************************************
 1.) Functions for accessing to an entry of a CM-Box
****************************************************/

/*****
 definition of CmAccess and the belonging options (flCtrl)
*****/
#define CM_ACCESS_LOCAL               0x0000
#define CM_ACCESS_LAN                 0x0001
#define CM_ACCESS_LOCAL_LAN           0x0002
#define CM_ACCESS_LAN_LOCAL           0x0003
#define CM_ACCESS_CMDMASK             0x0003

CODEMETER_API HCMSysEntry CMAPIENTRY CmAccess(CMULONG flCtrl, CMACCESS *pcmAcc);

/*****
 definition of CmRelease 
*****/
CODEMETER_API int CMAPIENTRY CmRelease(HCMSysEntry hcmse);

/**********************
 2.) Security functions
***********************/

/*****
 definition of CmCheckBox
*****/
CODEMETER_API int CMAPIENTRY CmCheckBox(HCMSysEntry hcmse);

/*****
 definition of CmCheckEvents and the belonging events (flEvents)
*****/
#define  CM_CE_BOXREMOVED              0x0001
#define  CM_CE_BOXREPLACED             0x0002
#define  CM_CE_BOXADDED                0x0004
#define  CM_CE_NETWORKLOST             0x0008
#define  CM_CE_NETWORKREPLACED         0x0010
#define  CM_CE_ENTRYMODIFIED           0x0020
#define  CM_CE_THRESHOLD_UNITCOUNTER   0x0040
#define  CM_CE_THRESHOLD_EXPDATE       0x0080
#define  CM_CE_SERVER_TERMINATED       0x0100
#define  CM_CE_BOXENABLED              0x0200
#define  CM_CE_BOXDISABLED             0x0400

CODEMETER_API CMULONG CMAPIENTRY CmCheckEvents(CMULONG flEvents);

/*****
 definition of CmCrypt & CmCryptSim and the belonging options (flCtrl)
*****/
/* commands for direct encryption */
#define CM_CRYPT_DIRECT_ENC           0x0000
#define CM_CRYPT_DIRECT_DEC           0x0001
#define CM_CRYPT_CERTTIME_ENC         0x0002
/* commands for indirect encryption */
#define CM_CRYPT_STREAM               0x0003
#define CM_CRYPT_AES_ENC_ECB          0x0004
#define CM_CRYPT_AES_DEC_ECB          0x0005
#define CM_CRYPT_AES_ENC_CFB          0x0006
#define CM_CRYPT_AES_DEC_CFB          0x0007
#define CM_CRYPT_AES_ENC_CBC          0x0008
#define CM_CRYPT_AES_DEC_CBC          0x0009

/* command mask for extracting the upper commands from other flags */
#define CM_CRYPT_CMDMASK              0x000f
/* This flag can be or'ed with all upper commands for indirect encryption */
#define CM_CRYPT_AUTOKEY              0x0100

CODEMETER_API int CMAPIENTRY CmCrypt(HCMSysEntry hcmse, CMULONG flCtrl,
    CMCRYPT *pcmCrypt, void* pvDest, CMUINT cbDest);

CODEMETER_API int CMAPIENTRY CmCryptSim(HCMSysEntry hcmse, CMULONG flCtrl,
    CMCRYPTSIM *pcmCryptSim, void* pvDest, CMUINT cbDest);

/***** 
 definition of CmCryptEcies
*****/
CODEMETER_API int CMAPIENTRY CmCryptEcies(CMBYTE* pbPublicKey,
    CMUINT cbPublicKey, CMBYTE* pbData, CMUINT cbData);

/***** 
 definition of CmCalcualtePioCoreKey
*****/
CODEMETER_API int CMAPIENTRY CmCalculatePioCoreKey(HCMSysEntry hcmse,
    CMPIOCK *pcmPioCK, CMBYTE* pbPioCK, CMUINT cbPioCK);

/***** 
 definition of CmGetSecureData
*****/
CODEMETER_API int CMAPIENTRY CmGetSecureData(HCMSysEntry hcmse, CMSECUREDATA *pcmSecureData,
    CMENTRYDATA *pcmEntryData);

/***** 
 definition of CmGetSecureData
*****/
CODEMETER_API int CMAPIENTRY CmGetPioDataKey(
    CMBYTE* pabPiodkDest, CMUINT cbPiodkDest,
    CMBYTE* pabPioCk, CMUINT cbPioCk,
    CMBYTE* pabPioEk, CMUINT cbPioEk);

/***** 
 definition of CmDecryptPioData
*****/
CODEMETER_API int CMAPIENTRY CmDecryptPioData(CMBYTE* pabDest, CMUINT cbDest,
    CMBYTE* pabPiodkDest, CMUINT cbPiodkDest);

/***** 
 definition of CmAgreeTalkKey
*****/

CODEMETER_API int CMAPIENTRY CmAgreeTalkKey(CMBOXINFO *pcmBoxInfo,
    CMTALKKEYINPUT_LICENSOR *pcmTalkKeyInputLicensor, CMTALKKEY *pcmTalkKey);

/* 
 options for 
  * flCtrl in CmCreateTalkKeyInput()
  * flCtrl in CmGetTalkKey()
*/
#define CM_TK_FIRMKEY                 0x00000000
#define CM_TK_FIRMCOMMONKEY           0x00000100
#define CM_TK_HIDDENDATA              0x00000200
#define CM_TK_SECRETDATA              0x00000300
#define CM_TK_CMDMASK                 0x00000300

#define CM_TK_KEYMASK                 0x000000ff

/***** 
 definition of CmCreateTalkKeyInput
*****/

CODEMETER_API int CMAPIENTRY CmCreateTalkKeyInput(HCMSysEntry hcmse, CMULONG flCtrl, 
    CMTALKKEYINPUT_USER *pcmTalkKeyInputUser,
    CMTALKKEYINPUT_LICENSOR *pcmTalkKeyInputLicensor);

/***** 
 definition of CmGetTalkKey
*****/

CODEMETER_API int CMAPIENTRY CmGetTalkKey(HCMSysEntry hcmse, CMULONG flCtrl,
    CMTALKKEYINPUT_USER *pcmTalkKeyInput, CMTALKKEY *pcmTalkKey);

/***********************************
 3.) Functions for managing a CM-Box
************************************/

/*****
 definition of CmBoxIoControl
*****/
CODEMETER_API CMUINT CMAPIENTRY CmBoxIoControl(HCMSysEntry hcmse,
    CMBYTE *pbInBuffer, CMUINT cbInBuffer,
    CMBYTE *pbOutBuffer, CMUINT cbOutBuffer);

/*****
 definition of CmCheckVersion
*****/
CODEMETER_API int CMAPIENTRY CmCheckVersion(HCMSysEntry hcmse, CMVERSION *pcmver);

/*****
 definition of CmGetBoxContents and the belonging options (flCtrl)
*****/
#define  CM_GBC_ALLENTRIES             0x0000
#define  CM_GBC_FI                     0x0001
#define  CM_GBC_BOX                    0x0002

CODEMETER_API int CMAPIENTRY CmGetBoxContents(HCMSysEntry hcmse, CMULONG flCtrl,
    CMULONG ulFirmCode, CMBOXINFO *pcmBoxInfo,
    CMBOXENTRY *pcmBoxEntry, CMUINT cbBoxEntry);

/*****
 definition of CmGetBoxes and the belonging ports (idPort)
*****/
#define  CM_GB_ALLPORTS                0x0000
#define  CM_GB_USB                     0x0001
#define  CM_GB_SIM                     0x0002

CODEMETER_API int CMAPIENTRY CmGetBoxes(HCMSysEntry hcmse, CMULONG idPort,
    CMBOXINFO *pcmBoxInfo, CMUINT cbBoxInfo);

/*****
 definition of CmGetInfo and the belonging options (flCtrl)
*****/
#define CM_GEI_BOXCONTROL             0x0000
#define CM_GEI_BOXINFO                0x0001
#define CM_GEI_BOXSECURITY            0x0002
#define CM_GEI_BOXTIME                0x0003
#define CM_GEI_ENTRYDATA              0x0004
#define CM_GEI_ENTRYINFO              0x0005
#define CM_GEI_INTERNALENTRYINFO      0x0006
#define CM_GEI_MEMINFO                0x0007
#define CM_GEI_SIGNEDLIST             0x0008
#define CM_GEI_SIGNEDTIME             0x0009
#define CM_GEI_SYSTEM                 0x000a
#define CM_GEI_VERSION                0x000b
#define CM_GEI_ENABLEBLOCKITEMS       0x000c
#define CM_GEI_ENABLELOOKUPITEMS_FI   0x000d
#define CM_GEI_ENABLELOOKUPITEMS_PI   0x000e
#define CM_GEI_CHIPINFO               0x000f
#define CM_GEI_BOXSTATUS              0x0010
#define CM_GEI_USBCHIPINFO            0x0011
#define CM_GEI_NETINFO_CLUSTER        0x0012
#define CM_GEI_NETINFO_USER           0x0013
#define CM_GEI_CMDMASK                0x001f
/*****
 definition of the BoxStatus flags (CM_GEI_BOXSTATUS)
*****/
#define CM_BOXSTATUS_LOWMEMORY         0x00000001
#define CM_BOXSTATUS_REPLUG            0x00000002
#define CM_BOXSTATUS_HASFLASH          0x00000004

#define CM_BOXSTATUS_ENABLED           0x00000000
#define CM_BOXSTATUS_DISABLED          0x00010000
#define CM_BOXSTATUS_TEMPENABLED       0x00020000
#define CM_BOXSTATUS_MASK              0x00030000

CODEMETER_API int CMAPIENTRY CmGetInfo(HCMSysEntry hcmse, CMULONG flCtrl,
    void *pvDest, CMUINT cbDest);

/*****
 definition of CmGetServers 
*****/
CODEMETER_API int CMAPIENTRY CmGetLicenseInfo(HCMSysEntry hcmse,
    CMLICENSEINFO *pcmLicenseInfo, CMUINT cbNumberOfLicenses);

/*****
 definition of CmGetServers and the belonging options (flCtrl) 
*****/
#define CM_GS_LIST                    0x0000
#define CM_GS_SINGLE_FIRST            0x0001
#define CM_GS_SINGLE                  0x0002
#define CM_GS_CMDMASK                 0x0003

#define CM_GS_IPADDR                  0x0000
#define CM_GS_NAME                    0x0010

CODEMETER_API int CMAPIENTRY CmGetServers(CMULONG flCtrl, char *pszServer,
    CMUINT cbServer, CMUINT *pnNumberOfServers);

/*****
 definition of CmGetVersion 
*****/
CODEMETER_API CMULONG CMAPIENTRY CmGetVersion(HCMSysEntry hcmse);

/******************************
 4.) Error management functions
*******************************/

/*****
 definition of CmGetLastErrorCode
*****/
CODEMETER_API int CMAPIENTRY CmGetLastErrorCode();

/***** 
 definition of CmGetLastErrorText and the belonging options (flCtrl)
*****/
#define CM_GLET_ERRORTEXT             0x0000
#define CM_GLET_DIALOG                0x0010

CODEMETER_API int CMAPIENTRY CmGetLastErrorText(CMULONG flCtrl,
    char *pszErrorText, CMUINT cbErrorText);

/***** 
 definition of CmSetLastErrorCode
*****/
CODEMETER_API void CMAPIENTRY CmSetLastErrorCode(int idError);

/**************************************
 5.) Functions for programming a CM-Box
***************************************/

/***** 
 definition of CmReserveFirmItem
  * global flags used for flCtrl
*****/
CODEMETER_API int CMAPIENTRY CmReserveFirmItem(HCMSysEntry hcmse,
    CMULONG ulFirmCode, CMRESERVEFI *pcmReserveFi);

/*****
 definition of CmCreateProductItemOption and the belonging options (flCtrl) 
*****/
/* flags specifing the PIO to add, update or delete */
#define CM_CPIO_PRODUCTCODE           0x00000000
#define CM_CPIO_FEATUREMAP            0x00000001
#define CM_CPIO_EXPTIME               0x00000002
#define CM_CPIO_ACTTIME               0x00000003
#define CM_CPIO_UNITCOUNTER           0x00000004
#define CM_CPIO_PROTDATA              0x00000005
#define CM_CPIO_EXTPROTDATA           0x00000006
#define CM_CPIO_HIDDENDATA            0x00000007
#define CM_CPIO_SECRETDATA            0x00000008
#define CM_CPIO_USERDATA              0x00000009
#define CM_CPIO_TEXT                  0x0000000a
#define CM_CPIO_DELETEALL             0x0000000b
#define CM_CPIO_CHANGEDEPENDENCY      0x0000000c
#define CM_CPIO_USAGEPERIOD           0x0000000d
#define CM_CPIO_CMDMASK               0x0000000f
/* flags combined only with CM_CPIO_PRODUCTCODE */
#define CM_CPIO_ADD                   0x00010000
#define CM_CPIO_UPDATE                0x00020000
/* flag combined with any CM_CPIO_...-flag */
#define CM_CPIO_DELETE_PIO            0x00100000
#define CM_CPIO_RELATIVE              0x00200000
/* flag combined with any CM_CPIO_...-flag but only called once altogether */
#define CM_CPIO_TERMINATE             0x00400000

CODEMETER_API int CMAPIENTRY CmCreateProductItemOption(HCMSysEntry hcmse,
    CMULONG flCtrl, void *pvPio, CMUINT cbPio);

/***** 
 definition of CmCreateSequence
  * global flags used for flCtrl
*****/
CODEMETER_API int CMAPIENTRY CmCreateSequence(HCMSysEntry hcmse, CMULONG flCtrl,
    CMCREATEITEM *pcmCreateItem, void *pvCtrl, CMUINT cbCtrl);

/***** 
 definition of CmProgram
  * global flags used for flCtrl
*****/
#define CM_PROG_DEFRAGMEM             0x00010000
#define CM_PROG_MASTERPASSWORD        0x00020000

CODEMETER_API int CMAPIENTRY CmProgram(HCMSysEntry hcmse,
    CMULONG flCtrl, void *pvCtrl, CMUINT cbCtrl, 
    void *pvVerify, CMUINT cbVerify);

/***** 
 definition of CmValidateEntry
*****/
#define CM_VAL_SIGNEDLIST             0x00000000
#define CM_VAL_SIGNEDTIME             0x00000001
#define CM_VAL_DELETE_FI              0x00000002
#define CM_VAL_CMDMASK                0x00000003

CODEMETER_API int CMAPIENTRY CmValidateEntry(HCMSysEntry hcmse, CMULONG flCtrl,
    CMBOXINFO *pcmBoxInfo, void *pvValidate, CMUINT cbValidate); 

/***** 
 definition of CmCreateLicenseFile
*****/
CODEMETER_API int CMAPIENTRY CmCreateLicenseFile(HCMSysEntry hcmse,
    CMLICENSEINFO *pcmLicenseInfo, CMBYTE *pbFirmKey, CMUINT cbFirmKey);

/********************************
 6.) Remote-Programming functions
*********************************/

/***** 
 definition of CmGetRemoteContext
*****/
#define CM_RMT_OVERWRITE              0x00000000
#define CM_RMT_APPEND                 0x00000001

CODEMETER_API int CMAPIENTRY CmGetRemoteContext(const char *pszRacFile,
    CMUINT flCtrl, HCMSysEntry hcmBox, CMUINT *pulFirmCodes,
    CMUINT cbFirmCodes);

/***** 
 definition of CmSetRemoteUpdate
*****/
CODEMETER_API int CMAPIENTRY CmSetRemoteUpdate(const char *pszRauFile,
    HCMSysEntry hcmBox);

/***** 
 definition of CmListRemoteUpdate
*****/
CODEMETER_API int CMAPIENTRY CmListRemoteUpdate(const char *pszRauFile,
    CMSERIAL *pcmSerials, CMUINT cbSerials);

/***********************************
 7.) Functions for enabling a CM-Box 
************************************/

/***** 
 definition of CmEnablingWriteApplicationKey and the belonging options (flCtrl)
*****/
#define CM_EW_ADD                     0x0000
#define CM_EW_UPDATE                  0x0001
#define CM_EW_DELETE                  0x0002
#define CM_EW_ATTACH                  0x0003
#define CM_EW_DETACH                  0x0004
#define CM_EW_CMDMASK                 0x0007

CODEMETER_API int CMAPIENTRY CmEnablingWriteApplicationKey(HCMSysEntry hcmse,
    CMULONG flCtrl, void *pvCtrl, CMUINT cbCtrl);

/***** 
 definition of CmEnablingGetApplicationContext
*****/
CODEMETER_API int CMAPIENTRY CmEnablingGetApplicationContext(HCMSysEntry hcmse,
    CMENABLING_APPCONTEXT *pcmAppContext);

/***** 
 definition of CmEnablingGetChallenge
*****/
CODEMETER_API int CMAPIENTRY CmEnablingGetChallenge(HCMSysEntry hcmse,
    CMENABLING_APPCONTEXT *pcmAppContext, CMBYTE *pbChallenge,
    CMUINT cbChallenge);

/***** 
 definition of CmEnablingSendResponse
*****/
CODEMETER_API int CMAPIENTRY CmEnablingSendResponse(HCMSysEntry hcmse,
    CMENABLING_APPCONTEXT *pcmAppContext, CMBYTE *pbResponse,
    CMUINT cbResponse);

/***** 
 definition of CmEnablingWithdrawAccessRights
*****/
CODEMETER_API int CMAPIENTRY CmEnablingWithdrawAccessRights(HCMSysEntry hcmse,
    CMENABLING_APPCONTEXT *pcmAppContext);

/**************************************************
 8.) Functions for time and date update in a CM-Box 
***************************************************/

/***** 
 definition of CmSetCertifiedTimeUpdate
*****/
CODEMETER_API int CMAPIENTRY CmSetCertifiedTimeUpdate(HCMSysEntry hcmse, char *pszCtcsServer);

/****************************
 9.) Authentication functions 
*****************************/

/***** 
 definition of CmCalculateDigest
*****/
CODEMETER_API int CMAPIENTRY CmCalculateDigest(CMBYTE *pbInput, CMUINT cbInput,
    CMBYTE *pbDigest, CMUINT cbDigest);

/***** 
 definition of CmCalculateSignature
*****/
CODEMETER_API int CMAPIENTRY CmCalculateSignature(HCMSysEntry hcmse,
    CMAUTHENTICATE *pcmAuth, CMBYTE *pbSignature, CMUINT cbSignature);

/***** 
 definition of CmGetPublicKey
*****/
CODEMETER_API int CMAPIENTRY CmGetPublicKey(HCMSysEntry hcmse,
    CMAUTHENTICATE *pcmAuth, CMBYTE *pbPubKey, CMUINT cbPubKey);

/***** 
 definition of CmValidateSignature
*****/
CODEMETER_API int CMAPIENTRY CmValidateSignature(CMAUTHENTICATE *pcmAuth,
    CMBYTE *pbSignature, CMUINT cbSignature,
    CMBYTE *pbPubKey, CMUINT cbPubKey);


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* !defined(_CODEMETER_INCLUDED) */
