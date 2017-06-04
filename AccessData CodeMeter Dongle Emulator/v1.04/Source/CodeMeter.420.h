/* ---- CodeMeter.h ----------------------------------------------------------
 * CodeMeter Development-Kit ANSI/Microsoft C header file.
 * (C) Copyright 2002-2010 by WIBU-SYSTEMS AG
 * D-76137 Karlsruhe, Germany
 * Version 4.20a of 2010May12
 * ---------------------------------------------------------------------------
 */

#if !defined(_CODEMETER_INCLUDED)
#define _CODEMETER_INCLUDED

/* Define Version for this header file as MAJOR * 100 + MINOR */
/* since 4.00 */
#define CODEMETER_VERSION 420

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
  /* library string concatination macros */
  #define CM_PPCX(s1, s2) s1##s2
  #define CM_PPCAT(s1, s2) CM_PPCX(s1, s2)
  #if defined(_WIN32) || defined(_WIN64)
    /* platform suffix */
    #if defined(_WIN32_WCE)
      #if defined(_X86_)
        #define CODEMETER_ARCH_SUFFIX "32ceX86"
      #elif defined(_ARM_)&& defined(ARMV4I)
        #define CODEMETER_ARCH_SUFFIX "32ceARMV4I"
      #elif defined(_ARM_)
        #define CODEMETER_ARCH_SUFFIX "32ceARMV4"
      #else
        #error Unsupported Windows CE environment
      #endif
    #elif defined(_WIN64)
      #define CODEMETER_ARCH_SUFFIX	       "64"
    #else
      /* standard Win32 has no suffix */
      #define CODEMETER_ARCH_SUFFIX        "32"
    #endif
    #if defined(_CODEMETER_API_STATIC_LIB) && !defined(_DEBUG) && defined(_MSC_VER) && _MSC_VER>=1200
      /* link static lib, only possible when release is wanted */
        /* compiler version variant */
        #if _MSC_VER >= 1600
          #define CODEMETER_LIB_VARIANT_SUB "StaticV10"
        #elif _MSC_VER >= 1500
          #define CODEMETER_LIB_VARIANT_SUB "StaticV9"
        #elif _MSC_VER >= 1400
          #define CODEMETER_LIB_VARIANT_SUB "StaticV8"
        #elif _MSC_VER >= 1300
          #define CODEMETER_LIB_VARIANT_SUB "StaticV7"
        #elif _MSC_VER >= 1200
          #define CODEMETER_LIB_VARIANT_SUB "StaticV6"
        #endif
        /* C/C++ runtime in DLL or static */
        #if defined(_DLL)
          /* link MFC based variant */ 
          #if defined(_UNICODE) && defined(_WIN32_WCE)
            /* with Unicode support */
            #define CODEMETER_LIB_VARIANT CM_PPCAT(CODEMETER_LIB_VARIANT_SUB, "MDU")
          #else
            #define CODEMETER_LIB_VARIANT CM_PPCAT(CODEMETER_LIB_VARIANT_SUB, "MD")
          #endif
        #else
            /* link Standard C based variant */ 
          #if defined(_UNICODE) && defined(_WIN32_WCE)
            /* with Unicode support */
            #define CODEMETER_LIB_VARIANT CM_PPCAT(CODEMETER_LIB_VARIANT_SUB, "MTU")
          #else
            #define CODEMETER_LIB_VARIANT CM_PPCAT(CODEMETER_LIB_VARIANT_SUB, "MT")
          #endif
        #endif
    #else
      /* link dynamic lib/dll */
      #define CODEMETER_LIB_VARIANT  ""
    #endif
  #endif
  #if !defined(__GNUC__)
    /* main name */
    #define CODEMETER_LIB_NAME  CM_PPCAT("WibuCm", CM_PPCAT(CODEMETER_ARCH_SUFFIX, CODEMETER_LIB_VARIANT))
    /* pass library name to linker */
    #if defined(__BORLANDC__)
      /* Borland C++ Builder 3 doesn't accept macros in pragma */
      #pragma comment(lib,"WibuCm32BC.lib")
    #else
      #pragma comment(lib,CODEMETER_LIB_NAME)
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
#define CMERROR_BOXLOCKED                       73
#define CMERROR_BOX_DAMAGED                     74
#define CMERROR_BOX_LOCKED_TO                   75

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
#define CMERROR_FULA_UPDATEEB_REACHED           90
#define CMERROR_FULA_ENCRCOMM_REACHED           91
#define CMERROR_UNFRAMED_COMMAND_REJECTED       92
#define CMERROR_COMMAND_PARTIALLY_REJECTED      93
#define CMERROR_COMMAND_PARTIALLY_UNSUPPORTED   94

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
/* replaced by CMERROR_LIBRARY_NOT_FOUND in version 4.00:
  #define CMERROR_SUBSYSTEM_NOT_AVAILABLE         126
*/
#define CMERROR_LIBRARY_NOT_FOUND               126

/* Internal box communication specific error codes */
#define CMERROR_BOX_COMMUNICATION_176           176
#define CMERROR_BOX_COMMUNICATION_177           177
#define CMERROR_BOX_COMMUNICATION_178           178
#define CMERROR_BOX_COMMUNICATION_179           179
#define CMERROR_BOX_COMMUNICATION_180           180
#define CMERROR_BOX_COMMUNICATION_181           181
#define CMERROR_BOX_COMMUNICATION_182           182
#define CMERROR_BOX_COMMUNICATION_183           183
#define CMERROR_BOX_COMMUNICATION_184           184
#define CMERROR_BOX_COMMUNICATION_185           185
#define CMERROR_BOX_COMMUNICATION_186           186
#define CMERROR_BOX_COMMUNICATION_187           187
#define CMERROR_BOX_COMMUNICATION_188           188
#define CMERROR_BOX_COMMUNICATION_189           189
#define CMERROR_BOX_COMMUNICATION_190           190
#define CMERROR_BOX_COMMUNICATION_191           191

/* I2C error codes */
#define CMERROR_I2C_TIMEOUT_REQUEST             192
#define CMERROR_I2C_TIMEOUT_ANSWER              193
#define CMERROR_I2C_ANSWER_CRC                  194
#define CMERROR_I2C_TWCOMMAND_INVALID           195
#define CMERROR_I2C_PROTOCOL_VERSION            196 
#define CMERROR_I2C_COMMUNICATION               197 
#define CMERROR_I2C_REQUEST_CRC                 198 
#define CMERROR_I2C_ANSWER_BEFORE_REQUEST       199 

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
#define CMERROR_WRONG_BOX_VERSION               224
#define CMERROR_INVALID_LICENSE_PARAMETER       225
#define CMERROR_BORROW_LICENSE_FAILED           226
#define CMERROR_BORROW_LICENSE_ISENABLED        227
#define CMERROR_UPDATE_TOO_NEW                  228
#define CMERROR_UPDATE_TOO_OLD                  229
#define CMERROR_SEVERAL_REASONS                 230
#define CMERROR_ACCESS_DENIED                   231
#define CMERROR_BORROW_TIMEDIFFTOOLARGE         232
#define CMERROR_UPDATE_FAILED                   233
#define CMERROR_UPDATE_ABORTED                  234

#define CMERROR_INTERNAL                        254
#define CMERROR_UNKNOWN                         255

/* CmAct related error codes */
#define CMERROR_CMACT_SUBSYSTEM_FAILED              260
#define CMERROR_CMACT_LICENSE_NOT_ACTIVATED         261
#define CMERROR_CMACT_LICENSE_ALREADY_ACTIVATED     262
#define CMERROR_CMACT_LICENSE_REACTIVATION_REQUIRED 263
#define CMERROR_CMACT_LICENSE_INVALID               264
#define CMERROR_CMACT_COMMAND_NOT_SUPPORTED         265
#define CMERROR_CMACT_LICENSE_ALREADY_EXISTS        266
#define CMERROR_CMACT_RUN_AS_SERVICE_REQUIRED       267
#define CMERROR_CMACT_BINDING_NOT_POSSIBLE          268
#define CMERROR_CMACT_OPEN_FIRMITEM_TEMPLATE_FAILED 269
#define CMERROR_CMACT_OSVERSION_NOT_PERMITTED       270
#define CMERROR_CMACT_HOST_ID_NOT_AVAILABLE         271
#define CMERROR_CMACT_PLUGIN_MALFUNCTION            272
#define CMERROR_CMACT_VIRTUAL_MACHINE_NOT_ALLOWED   273

/* basic error codes */
#define CMERROR_ERRORTEXT_NOT_FOUND             300
#define CMERROR_SEND_PROTECTION_FAILED          301
#define CMERROR_ANALYSING_DETECTED              302
#define CMERROR_COMMAND_EXECUTION_FAILED        303
#define CMERROR_FUNCTION_NOT_IMPLEMENTED        304
#define CMERROR_OBSOLETE_FUNCTION               305

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
#define CM_SYM_KEY_LEN                16
#define CM_SIGNATURE_LEN              64
#define CM_EPHEMERAL_POINT_LEN        64
#define CM_BORROW_SERVERID_LEN        8

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
#define CM_GF_SET_BOXPASSWORD         0x002f
#define CM_GF_CHANGE_BOXPASSWORD      0x003f
#define CM_GF_RESET_BOXPASSWORD       0x004f
#define CM_GF_UPDATECHECKOUT_BORROW   0x005f
#define CM_GF_SET_BOXPASSWORD2        0x006f

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
#define CM_GF_LICENSEQUANTITY         0x1000
#define CM_GF_BORROWCLIENT            0x2000
#define CM_GF_BORROWSERVER            0x4000
#define CM_GF_COLI                    0x8000
#define CM_GF_PIO_MASK                0x7fff

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

/*
  flags for midCps in all CMBORROWXXX structures
*/
#define CM_CPS_CODEMETER              0x00000000
#define CM_CPS_CODEMETERACT           0x00000001

/***** CMBORROWCLIENT *****/
typedef struct __CMBORROWCLIENT {
  CMULONG mulStatus;
  /* data from EPD 130 */
  CMUSHORT midEnableBlock;
  CMUSHORT midCps;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMBYTE mabUpdateProgSeq[32];
  CMBYTE mabServerID[CM_BORROW_SERVERID_LEN];
  CMBYTE mabReserved1[12];
  /* data from EPD 131 */
  CMUSHORT musReserved;
  CMUSHORT musServerBoxMask;
  CMULONG mulServerBoxSerial;
  char mszServerName[128];
  CMBYTE mabReserved2[12];
} CMBORROWCLIENT;

/***** CMBORROWDATA *****/
typedef struct __CMBORROWDATA {
  CMULONG mulStatus;
  CMUSHORT midEnableBlock;
  CMUSHORT midCps;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulFeatureMap;
  CMUSHORT musReserved;
  CMUSHORT musBoxMask;
  CMULONG mulBoxSerial;
  CMBYTE mabSerialPublicKey[CM_PUBLIC_KEY_LEN];
  CMBYTE mabServerID[CM_BORROW_SERVERID_LEN];
  char mszClientName[128];
  CMBYTE mabTrailingValidationBlock[CM_TVB_LEN];
  CMULONG mulClientRequestTime;
  CMBYTE mabReserved[8];
} CMBORROWDATA;

/***** CMBORROWITEM *****/
typedef struct __CMBORROWITEM {
  /* data from EPD 132 */
  CMUSHORT midCps;
  CMUSHORT musReserved;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulFeatureMap;
  CMULONG mcLicenseQuantity;
  CMULONG mulCheckoutDuration;
  CMBYTE mabServerID[CM_BORROW_SERVERID_LEN];
  CMBYTE mabReserved[28];
  CMUSHORT musUsedLicenses;
  CMUSHORT musFreeLicenses;
} CMBORROWITEM;

/***** CMBORROWMANAGE *****/
typedef struct __CMBORROWMANAGE {
  /* data from EPD 133 */
  CMBYTE mabUpdateProgSeq[32];
  CMULONG mcBorrowedLicenses;
  CMBYTE mabHashBorrowedLicenses[CM_TVB_LEN];
  CMBYTE mabReserved[12];
} CMBORROWMANAGE;

/***** CMACCESS *****/
/*
  flags for mflCtrl in CMACCESS
*/
/* flags for kind of access */
#define CM_ACCESS_USERLIMIT           0x00000000
#define CM_ACCESS_NOUSERLIMIT         0x00000100
#define CM_ACCESS_EXCLUSIVE           0x00000200
#define CM_ACCESS_STATIONSHARE        0x00000300
#define CM_ACCESS_CONVENIENT          0x00000400
/* mask for the access modes */
#define CM_ACCESS_STRUCTMASK          0x00000700

/* no validation check of the entry data */
#define CM_ACCESS_FORCE               0x00010000
/* constant for searching a fitting FSB entry */
#define CM_ACCESS_CHECK_FSB           0x00020000
/* constant for searching a fitting CTSB entry */
#define CM_ACCESS_CHECK_CTSB          0x00040000
/* allow normal subsystem access if no CmStick is found */
#define CM_ACCESS_SUBSYSTEM           0x00080000
/* force FI access to prevent access to a FC:PC=x:0 */
#define CM_ACCESS_FIRMITEM            0x00100000

/* flag access to borrow license */
#define CM_ACCESS_BORROW_ACCESS       0x01000000
/* flag release to borrow license */
#define CM_ACCESS_BORROW_RELEASE      0x02000000
/* flag check borrowed license */
#define CM_ACCESS_BORROW_VALIDATE     0x04000000
/* flag ignore entry state for release to borrow license */
#define CM_ACCESS_BORROW_IGNORESTATE  0x08000000

/* These option constants are used in Feature Codes which are FSB-internally
   created for a specific FSB operation (mulFeatureCode). */

/* Feature Code for the <FsbEncryptAddFi> FSB operation */
#define CM_FSBFEATURECODE_ADDFI            0x00000001
/* Feature Code for the <FsbHashBoxAlg::DeleteFi> FSB operation */
#define CM_FSBFEATURECODE_DELETEFI         0x00000002
/* Feature Code for the <FsbHashBoxAlg::UpdateFi> FSB operation */
#define CM_FSBFEATURECODE_UPDATEFI         0x00000004
/* Feature Code for the <FsbHashBoxAlg::AddPi> FSB operation */
#define CM_FSBFEATURECODE_ADDPI            0x00000008
/* Feature Code for the <FsbHashBoxAlg::DeletePi> FSB operation */
#define CM_FSBFEATURECODE_DELETEPI         0x00000010
/* Feature Code for the <FsbHashBoxAlg::UpdatePi> FSB operation */
#define CM_FSBFEATURECODE_UPDATEPI         0x00000020
/* Feature Code for the <FsbHashBoxAlg::AddEb> FSB operation */
#define CM_FSBFEATURECODE_ADDEB            0x00000040
/* Feature Code for the <FsbHashBoxAlg::DeleteEb> FSB operation */
#define CM_FSBFEATURECODE_DELETEEB         0x00000080
/* Feature Code for the <FsbHashBoxAlg::UpdateEb> FSB operation */
#define CM_FSBFEATURECODE_UPDATEEB        0x00000100
/* Feature Code for the <FsbHashBoxAlg::AttachEb> FSB operation */
#define CM_FSBFEATURECODE_ATTACHEB         0x00000200
/* Feature Code for the <FsbHashBoxAlg::GetTalkKeyInput> FSB operation */
#define CM_FSBFEATURECODE_GETTALKKEYINPUT  0x00000400
/* Feature Code for the <FsbHashBoxAlg::GetBoxTimeSign> FSB operation */
#define CM_FSBFEATURECODE_GETBOXTIMESIGN   0x00002000
/* Feature Code for the <FsbHashBoxAlg::DeleteFiSign> FSB operation */
#define CM_FSBFEATURECODE_DELETEFISIGN     0x00004000
/* Feature Code for the <FsbHashBoxAlg::ListSign> FSB operation */
#define CM_FSBFEATURECODE_LISTSIGN         0x00008000
/* Feature Code for the <FsbHashBoxAlg::Encrypt> FSB operation */
#define CM_FSBFEATURECODE_ENCRYPT          0x00010000
/* Feature Code for the <FsbPriceDeduct> FSB operation */
#define CM_FSBFEATURECODE_PRICEDEDUCT      0x00020000
/* Feature Code for storing the CmAct serial numbers */
#define CMACT_FSBFEATURECODE_SERIALNUMBERS 0x00040000

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



/* These option constants are used in CMCREDENTIAL in combination 
   with a CmGetInfo and CM_GEI_CREDENTIAL or CM_GEI_NETINFO_USER_EXT
   request and are informal (mulAccessCtrl). */

/* Identicate that an access is based on CM_ACCESS_LOCAL */
#define CM_CRED_LICENSE_LOCATION_LOCAL    0x0001 
/* Identicate that an access is based on CM_ACCESS_LAN */
#define CM_CRED_LICENSE_LOCATION_LAN      0x0002 
#define CM_CRED_LICENSE_LOCATION_MASK     0x0007

/* Identicate that a handle is based on a Entry Access */
#define CM_CRED_HANDLE_ACCESS_ENTRY       0x0008 
/* Identicate that a handle is based on a FI Access */
#define CM_CRED_HANDLE_ACCESS_FI          0x0010
/* Identicate that a handle is based on a Box Access */
#define CM_CRED_HANDLE_ACCESS_BOX         0x0018
/* Identicate that a handle is based on a SubSystem Access */
#define CM_CRED_HANDLE_ACCESS_SUBSYSTEM   0x0020
#define CM_CRED_HANDLE_ACCESS_MASK        0x0038 

/***** CMCREDENTIAL *****/
typedef struct __CMCREDENTIAL {
  CMULONG mulPID;
  CMULONG mulSession;
  CMULONG mulCleanupTime;
  CMULONG mulMaxLifeTime;
  CMULONG mulCreationTime;
  CMULONG mulAccessCtrl;
  CMULONG mulExpirationTime; 
  CMULONG mulUserDefinedID;
  char mszUserDefinedText[128];
  char mszUsername[32];
  CMULONG mulOtherBorrowFirmCode;
  CMULONG mulOtherBorrowProductCode;
  CMULONG mulOtherBorrowFeatureMap;
  CMULONG mulOtherBorrowSerial;
  CMUSHORT musOtherBorrowMask;
  char mabReserved2[30];
} CMCREDENTIAL;


/***** CMACCESS2 *****/
typedef struct __CMACCESS2 {
  CMULONG mflCtrl;
  CMULONG mulFirmCode;
  CMULONG mulProductCode;
  CMULONG mulFeatureCode; 
  CMULONG mulUsedRuntimeVersion; 
  CMULONG mulReserved1;
  CMULONG mulProductItemReference;
  CMULONG mulReserved2; 
  CMBYTE  mbMinBoxMajorVersion;
  CMBYTE  mbMinBoxMinorVersion;
  CMUSHORT musBoxMask;
  CMULONG mulSerialNumber;
  CMULONG mulReserved3; 
  CMULONG mulReserved4; 
  char    mszServername[128]; 
  char    mabReserved[32];
  CMBORROWDATA mcmBorrowData;
  CMCREDENTIAL mcmCredential;
} CMACCESS2;


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
  CMBYTE mbReserve;
  CMUSHORT musProductItemRef;
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
/* CmStick encryption algorithms */
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

/***** CMBASECRYPT *****/
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
  CMUINT mulCreationTime;
  char mszClientIP[32];
} CMNETINFOUSER;


/***** CMNETINFOUSER_EXT *****/
typedef struct __CMNETINFOUSER_EXT {
  CMUSHORT musMask;
  CMUSHORT mflCtrl;
  CMULONG mulSerial;
  CMULONG mulProductItemRef;
  CMULONG mulFirmCode;
  
  CMULONG mulProductCode;
  CMULONG mulFeatureMap;
  CMULONG mulID;
  CMULONG mulAccessMode;
  
  CMULONG mulLastAccessTime;
  CMULONG mulReserved2;
  CMULONG mulReserved3;
  CMULONG mulReserved4;
  
  char mszClientAddress[64];
  CMCREDENTIAL mcmCredential;
} CMNETINFOUSER_EXT;


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


/***** CMMEMINFO2 *****/
typedef struct __CMMEMINFO2 {
  CMUSHORT mcFree4ByteBlock;
  CMUSHORT mcFree8ByteBlock;
  CMUSHORT mcFree16ByteBlock;
  CMUSHORT mcFree32ByteBlock;
  CMUSHORT mcFree64ByteBlock;
  CMUSHORT mcFree128ByteBlock;
  CMUSHORT mcFree256ByteBlock;
  CMUSHORT mcFree512ByteBlock;
  CMULONG  mulReserved1;
  CMULONG  mulReserved2;
  CMULONG  mulReserved3;
  CMULONG  mulReserved4;
  CMULONG  mulTotalBytesFree;
  CMULONG  mulCapacity;
} CMMEMINFO2;


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
  flags for midPlatform in CMSYSTEM
*/
#define CM_SYSTEM_W95                 0x00000000
#define CM_SYSTEM_W98                 0x00000001
#define CM_SYSTEM_WME                 0x00000002
#define CM_SYSTEM_NT4                 0x00000003
#define CM_SYSTEM_W2K                 0x00000004
#define CM_SYSTEM_WXP                 0x00000005
#define CM_SYSTEM_W2003               0x00000006
#define CM_SYSTEM_VISTA               0x00000007
#define CM_SYSTEM_W2008               0x00000008
#define CM_SYSTEM_W7                  0x00000009

#define CM_SYSTEM_WINDOWS             0x00000000
#define CM_SYSTEM_MACOSX              0x00000100
#define CM_SYSTEM_SOLARIS             0x00000200
#define CM_SYSTEM_WIN_CE              0x00001000
#define CM_SYSTEM_LINUX               0x00010000

#define CM_SYSTEM_PLATFORM_MASK       0x000FFF00
#define CM_SYSTEM_VERSION_MASK        0x000000FF
#define CM_SYSTEM_INVALID_PLATFORM    0xFFFFFFFF

#define CM_SYSTEM_LITTLE_ENDIAN       0x00100000
#define CM_SYSTEM_BIG_ENDIAN          0x00200000
#define CM_SYSTEM_ENDIAN_ORDER_MASK   0x00300000


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

/***** CMSECURITYVERSION *****/
typedef struct __CMSECURITYVERSION {
  CMUSHORT musVersion;
  CMUSHORT musSubVersion;
  CMUSHORT musBuild;
  CMUSHORT musCount;
  CMUSHORT musYear;
  CMUSHORT musMonth;
  CMUSHORT musDay;
  CMUSHORT musType;
  CMUSHORT musReserved1;
  CMUSHORT musReserved2;
  CMULONG  mulReserved1;
  CMULONG  mulReserved2;
  CMULONG  mulReserved3;
} CMSECURITYVERSION;

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
  CMBYTE mbMajorVersion;
  CMBYTE mbMinorVersion;
  CMUSHORT musReserve;
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
#define CM_EBI_ENABLED                0x0000
#define CM_EBI_DISABLED               0x0001
#define CM_EBI_TEMPENABLED            0x0002
#define CM_EBI_ENABLING_MASK          0x0003
#define CM_EBI_EXPIRED                0x0100

/*
  flags for mbType in CMENABLING_ENABLEBLOCKITEM
*/
#define CM_EBI_SIMPLEPIN              0x0000
#define CM_EBI_TIMEPIN                0x0010
#define CM_EBI_SMARTCARD              0x0020
#define CM_EBI_FINGERPIN              0x0030
/*
  constants for mulDisableTime
*/
#define CM_EBI_NODISABLETIME          0xFFFFFFFF


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


/***********************************
 3.0.) CodeMeterAct Structures
************************************/

/*****
 definition of the mulPreferredActivationMethod flags (CMACTLICENSEINFO)
*****/
#define CMACT_ACTIVATION_INVALID      0
#define CMACT_ACTIVATION_PHONE        1
#define CMACT_ACTIVATION_EMAIL        2
#define CMACT_ACTIVATION_PORTAL       3
#define CMACT_ACTIVATION_SERVICE      4
#define CMACT_ACTIVATION_FILE         5

#define CMACT_LO_ALLOW_VM             0x00000001

/*****
 for CmActLicenseControl( CM_GF_ACT_REMOVE, ... )
*****/
#define CM_GF_ACT_UNLOAD_LICENSE      0
#define CM_GF_ACT_REMOVE_LICENSE      1

#define CMACT_MAX_LEN_PRODUCTNAME     64
#define CMACT_MAX_LEN_VENDORTEXT      128
#define CMACT_MAX_LEN_ACTIVATION_CODE 64
#define CMACT_MAX_LEN_LICENSORDATA    4096
#define CMACT_MAX_LEN_PLUGIN_NAME     32

#define CMACT_LEN_PRODUCTIDMAJ        5
#define CMACT_LEN_CLIENTSECKEY        6

#define CMACT_MAX_NUM_PRODUCT_DESCS   2000
#define CMACT_MAX_NUM_DIGESTS         4

#define CMACT_MAX_PLATFORMS           10

/***** CMACTPRODUCTDESCRIPTION (CmAct) *****/
typedef struct __CMACTPRODUCTDESCRIPTION {
  CMULONG mulProductCode;
  CMULONG mulFeatureMask;
} CMACTPRODUCTDESCRIPTION;

/***** CMACTLICENSEINFO *****/
typedef struct __CMACTLICENSEINFO {
  CMUSHORT musBoxMask;
  CMUSHORT musReserved1;
  CMULONG  mulSerialNumber;
  CMULONG  mulFirmCode;
  CMULONG  mulStatus;
  CMULONG  mflLicenseOptions;
  CMULONG  mulReserved2;
  CMULONG  mulReserved3;
  // PHONE = 1, EMAIL = 2, PORTAL = 3, SERVICE = 4, FILE = 5
  CMULONG  mulPreferredActivationMethod; 
  char mszProductName[CMACT_MAX_LEN_PRODUCTNAME];
  char mszProductNameExt[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorName[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorPhone[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorEMail[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorWebPortal[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorWebService[CMACT_MAX_LEN_VENDORTEXT];
  char mszPluginName[CMACT_MAX_LEN_PLUGIN_NAME];
  CMBYTE mabReserved3[96];
  CMULONG mulProductIdMinor;
  CMULONG mulReserved4;
  char mszProductIdMajor[CMACT_LEN_PRODUCTIDMAJ];
  CMBYTE mbReserved1;
  CMUSHORT musNumberOfProductDescriptions;
  CMACTPRODUCTDESCRIPTION maProductDescriptions[CMACT_MAX_NUM_PRODUCT_DESCS];
  char mszProductSerialNumber[CMACT_MAX_LEN_VENDORTEXT];
} CMACTLICENSEINFO;


/***** CMACTLICENSEDATA (CmAct) *****/

typedef struct __CMACTLICENSEDATA {
  CMULONG mflCtrl;
  CMUSHORT musBoxMask;
  CMUSHORT musReserved1;
  CMULONG mulSerialNumber;
  CMULONG mulFirmCode;
  CMULONG mulBindingFlags;
  CMULONG mnNumberOfRequiredBindingFlags;
  CMULONG mflLicenseOptions;
  CMULONG mulGeneration;
  CMULONG mulReplacesGeneration;
  CMBYTE mabSymKeyPhone[CM_SYM_KEY_LEN];
  CMBYTE mabLicensorPubKey[CM_PUBLIC_KEY_LEN];
  CMBYTE mabSigLicensorPubKey[CM_SIGNATURE_LEN];
  char mszProductName[CMACT_MAX_LEN_PRODUCTNAME];
  CMULONG mulProductIdMinor;
  char mszProductIdMajor[CMACT_LEN_PRODUCTIDMAJ];
  CMBYTE mbReserved2;
  CMUSHORT musReserved4;
  CMULONG mulPreferredActivationMethod;
  char mszProductNameExt[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorName[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorPhone[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorEMail[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorWebPortal[CMACT_MAX_LEN_VENDORTEXT];
  char mszVendorWebService[CMACT_MAX_LEN_VENDORTEXT];
  char mszPluginName[CMACT_MAX_LEN_PLUGIN_NAME];
  CMBYTE mabReserved6[96];
  CMUSHORT musNumberOfProductDescriptions;
  CMUSHORT musReserved3;
  CMACTPRODUCTDESCRIPTION maProductDescriptions[CMACT_MAX_NUM_PRODUCT_DESCS];
  CMBYTE mabBoxTemplateKey[CM_SYM_KEY_LEN];
  CMBYTE mabSymKeyBoxMemory[CM_SYM_KEY_LEN];
  CMBYTE mabSymKeyDynData[CM_SYM_KEY_LEN];
  CMULONG mulNumberOfPermittedPlatforms;
  CMULONG maulPermittedPlatforms[CMACT_MAX_PLATFORMS]; // see flags for midPlatform in CMSYSTEM
  CMULONG mulReserved5;
} CMACTLICENSEDATA;

/***** CMACTDIGEST (CmAct) *****/
typedef struct __CMACTDIGEST {
  CMUSHORT musPaPuCount;
  CMUSHORT musDigestCount; //!< no of digest contains at mabDigest
  CMULONG mulReserved1;
  CMULONG mulReserved2;
  CMULONG mulReserved3;
  CMBYTE mabDigest[CMACT_MAX_NUM_DIGESTS][CM_DIGEST_LEN];  //!< one or more digest
} CMACTDIGEST;


/***** CMACTSIGNATURE (CmAct) *****/
typedef struct __CMACTSIGNATURE {
  CMUSHORT musPaPuCount;
  CMUSHORT musSignatureCount;  //!< no of signatures contains at mabSignature
  CMULONG mulReserved1;
  CMULONG mulReserved2;
  CMULONG mulReserved3;
  CMBYTE  mabSignature[CMACT_MAX_NUM_DIGESTS][CM_SIGNATURE_LEN]; //!< one or more signatures
} CMACTSIGNATURE;


/***** CMACTACTIVATION (CmAct) *****/
typedef struct __CMACTACTIVATION {
  CMULONG mflCtrl;
  char mszInstallationId[CMACT_MAX_LEN_ACTIVATION_CODE];
  char mszProductSerialNumber[CMACT_MAX_LEN_VENDORTEXT];
  CMBYTE mabSymKeyPhone[CM_SYM_KEY_LEN];
  CMBYTE mabClientSecKey[CMACT_LEN_CLIENTSECKEY];
  CMUSHORT musReserved2;
  CMUSHORT musBoxMask;
  CMUSHORT musReserved1;
  CMULONG mulSerialNumber;
  CMULONG mulProductIdMinor;
  CMULONG mulGeneration;
  CMUSHORT musPaPuCount;
  CMBYTE mabReserved[34];
} CMACTACTIVATION;

/***** CMACTREQUEST *****/
typedef struct __CMACTREQUEST {
  char mszProductSerialNumber[CMACT_MAX_LEN_VENDORTEXT];
  CMULONG mulProductSubType;
  CMULONG mulReserved1;
  CMBYTE mabLicensorData[CMACT_MAX_LEN_LICENSORDATA];
} CMACTREQUEST;


/***** CMACTSYMKEY (CmAct) *****/

typedef struct __CMACTSYMKEY {
  CMBYTE mabSymKey[CM_SYM_KEY_LEN];
  CMBYTE mabEphemeralPoint[CM_EPHEMERAL_POINT_LEN];
} CMACTSYMKEY;

/*****
 definition of the mulFileType flags (CMACTFILEINFO)
*****/
#define CMACT_FILETYPE_LICENSEINFORMATIONFILE           1
#define CMACT_FILETYPE_LICENSEINFORMATIONFILE_BOXMEMORY 2 
#define CMACT_FILETYPE_LICENSEREQUESTFILE               3
#define CMACT_FILETYPE_LICENSEACTIVATIONFILE            4
#define CMACT_FILETYPE_LICENSEFILE                      5
#define CMACT_FILETYPE_LICENSEREQUESTFILE_BOXMEMORY     6

/***** CMACTFILEINFO (CmAct) *****/
typedef struct __CMACTFILEINFO {
  CMULONG mulFileType;
  CMULONG mulFirmCode;
  char mszProductName[CMACT_MAX_LEN_PRODUCTNAME];
  CMUSHORT musBoxMask;
  CMUSHORT musReserved1;
  CMULONG mulSerialNumber;
  CMULONG mulProductIdMinor;
  char mszProductIdMajor[CMACT_LEN_PRODUCTIDMAJ]; 
  CMBYTE mbReserved2;
  CMUSHORT musReserved3;
  CMULONG mulGeneration;
  char mszProductSerialNumber[CMACT_MAX_LEN_VENDORTEXT];
  CMBYTE mabLicensorInfo[CMACT_MAX_LEN_LICENSORDATA];
  CMULONG mulStatus;
  char mszPluginName[CMACT_MAX_LEN_PLUGIN_NAME];
  CMULONG mflLicenseOptions;
  CMULONG mulPlatform;
  CMUSHORT musNumberOfProductDescriptions;
  CMUSHORT musReserved5;
  CMACTPRODUCTDESCRIPTION maProductDescriptions[CMACT_MAX_NUM_PRODUCT_DESCS];
  CMACTSYMKEY mSymKeyBoxMemory;
  CMACTSYMKEY mSymKeyDynData;
} CMACTFILEINFO;

/***** CMACTERRORINFO *****/
typedef struct __CMACTERRORINFO {
  CMULONG mulErrorCode;
  CMULONG mulReserved1;
  char mszAdditionalInfo[512];
} CMACTERRORINFO;

/***********************************
 4.0.) Remote Update Structures
************************************/

/*
** for CMFAS.mflSelect
*/

#define CMFAS_SELECT_MASK             0x0000000F
#define CMFAS_SELECT_WILDCARD         0x00000000
#define CMFAS_SELECT_SERIALNUMBER     0x00000001
#define CMFAS_SELECT_FIRMCODE         0x00000002

#define CMFAS_OPT_MASK                0x10000000
#define CMFAS_OPT_IGNORE              0x10000000

/*
** for CMFAS.mulItemType
*/

#define CMFAS_ITEMTYPE_LICENSE           0x00000001
#define CMFAS_ITEMTYPE_CMACT_LICENSE     0x80000001
#define CMFAS_ITEMTYPE_CMHW_LICENSE      0x40000001
#define CMFAS_ITEMTYPE_CMFIRM_WBC        0x00000002
#define CMFAS_ITEMTYPE_CMACT_FI_TEMPLATE 0x80000004

/***** CMFAS *****/
typedef struct __CMFAS
{
  CMULONG mflSelect;
  CMULONG mulItemType;

  // Serial number (CMFAS_CTL_SELECT_SERIALNUMBER)
  CMULONG mulSerial;
  CMUSHORT musMask;
  CMUSHORT musReserved1;

  // FC (CMFAS_CTL_SELECT_FIRMCODE)
  CMULONG mulFirmCode;

  CMULONG maulReserved3[20];

  // result
  CMULONG mulResult;

  CMULONG maulReserved4[10];
} CMFAS;

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

/****************************************************
 1.) Functions for accessing to an entry of a CmStick
*****************************************************/

/*****
 definition of CmAccess and the belonging options (flCtrl)
*****/
#define CM_ACCESS_LOCAL               0x0000
#define CM_ACCESS_LAN                 0x0001
#define CM_ACCESS_LOCAL_LAN           0x0002
#define CM_ACCESS_LAN_LOCAL           0x0003
#define CM_ACCESS_CMDMASK             0x0003

CODEMETER_API HCMSysEntry CMAPIENTRY CmAccess(CMULONG flCtrl, CMACCESS *pcmAcc);

CODEMETER_API HCMSysEntry CMAPIENTRY CmAccess2(CMULONG flCtrl, CMACCESS2 *pcmAcc);

/*****
 definition of CmRelease 
*****/
CODEMETER_API int CMAPIENTRY CmRelease(HCMSysEntry hcmse);

/**********************
 2.) Security functions
***********************/


/*****
 definition of CmCheckEvents and the belonging events (flEvents)
*****/
#define  CM_CE_BOXREMOVED              0x000001
#define  CM_CE_BOXREPLACED             0x000002
#define  CM_CE_BOXADDED                0x000004
#define  CM_CE_NETWORKLOST             0x000008
#define  CM_CE_NETWORKREPLACED         0x000010
#define  CM_CE_ENTRYMODIFIED           0x000020
#define  CM_CE_THRESHOLD_UNITCOUNTER   0x000040
#define  CM_CE_THRESHOLD_EXPDATE       0x000080
#define  CM_CE_SERVER_TERMINATED       0x000100
#define  CM_CE_BOXENABLED              0x000200
#define  CM_CE_BOXDISABLED             0x000400
#define  CM_CE_RELEASE_EVENT           0x800000

CODEMETER_API CMULONG CMAPIENTRY CmCheckEvents(CMULONG flEvents);

/*****
 definition of CmCrypt & CmCryptSim and the belonging options (flCtrl)
*****/
/* commands for direct encryption */
#define CM_CRYPT_DIRECT_ENC           0x0000
#define CM_CRYPT_DIRECT_DEC           0x0001
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

/************************************
 3.) Functions for managing a CmStick
*************************************/

/*****
 definition of CmGetBoxContents and the belonging options (flCtrl)
*****/
#define  CM_GBC_ALLENTRIES             0x0000
#define  CM_GBC_FI                     0x0001
#define  CM_GBC_BOX                    0x0002
#define  CM_GBC_USELOCALTIME           0x1000

CODEMETER_API int CMAPIENTRY CmGetBoxContents(HCMSysEntry hcmse, CMULONG flCtrl,
    CMULONG ulFirmCode, CMBOXINFO *pcmBoxInfo,
    CMBOXENTRY *pcmBoxEntry, CMUINT cbBoxEntry);

/*****
 definition of CmGetBoxes and the belonging ports (idPort)
*****/
#define  CM_GB_ALLPORTS                0x0000
#define  CM_GB_USB                     0x0001
#define  CM_GB_SIM                     0x0002
#define  CM_GB_ACT                     0x0004
#define  CM_GB_PORT_MASK               0x000F

CODEMETER_API int CMAPIENTRY CmGetBoxes(HCMSysEntry hcmse, CMULONG idPort,
    CMBOXINFO *pcmBoxInfo, CMUINT cbBoxInfo);


#define CM_SEC_USER     1
#define CM_SEC_LICENSOR 2
#define CM_SEC_WIBU     3


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
#define CM_GEI_CREDENTIAL             0x0014
#define CM_GEI_SECURITYVERSION        0x0015
#define CM_GEI_NETINFO_USER_EXT       0x0016
#define CM_GEI_MEMINFO2               0x0017
#define CM_GEI_ACT_LICENSE_INFO       0x0018
#define CM_GEI_ACT_ERROR_INFO         0x0019
#define CM_GEI_CMACTVERSION           0x0020
#define CM_GEI_BORROWCLIENT           0x0030
#define CM_GEI_BORROWDATA             0x0031
#define CM_GEI_BORROWITEMS            0x0032
#define CM_GEI_CMDMASK                0x00ff
#define CM_GEI_USELOCALTIME           0x1000
#define CM_GEI_INDEXMASK              0xff000000


/*****
 definition of the BoxStatus flags (CM_GEI_BOXSTATUS)
*****/
#define CM_BOXSTATUS_LOWMEMORY         0x00000001
#define CM_BOXSTATUS_REPLUG            0x00000002
#define CM_BOXSTATUS_HASFLASH          0x00000004
#define CM_BOXSTATUS_ISCMACT           0x00000008
#define CM_BOXSTATUS_REMOVABLE         0x00000010
#define CM_BOXSTATUS_FEATURE_MASK      0x000000FF

#define CM_ACTSTATUS_FILE              0x00000100
#define CM_ACTSTATUS_LOADED            0x00000200
#define CM_ACTSTATUS_ACTIVATE_BY_CODE  0x00000400
#define CM_ACTSTATUS_ACTIVATE_BY_FILE  0x00000800
#define CM_ACTSTATUS_ACTIVE            0x00001000
#define CM_ACTSTATUS_PROGRAMMABLE      0x00002000
#define CM_ACTSTATUS_INVALID           0x00004000
#define CM_ACTSTATUS_BROKEN            0x00008000
#define CM_ACTSTATUS_PSN_REQUIRED      0x02000000
#define CM_ACTSTATUS_VM_DETECTED       0x04000000 
  /* CM_ACTSTATUS_VM_DETECTED has two meanings: When returned by
     CM_GEI_BOXSTATUS it's an error condition; when returned by
     CM_GF_ACT_GETFILEINFO, the file was created in a VM (which
     was permitted) */
#define CM_BOXSTATUS_CMACT_MASK        0x0F00FF00

#define CM_BOXSTATUS_ENABLED           0x00000000
#define CM_BOXSTATUS_DISABLED          0x00010000
#define CM_BOXSTATUS_TEMPENABLED       0x00020000
#define CM_BOXSTATUS_MASK              0x00030000


CODEMETER_API int CMAPIENTRY CmGetInfo(HCMSysEntry hcmse, CMULONG flCtrl,
    void *pvDest, CMUINT cbDest);

/*****
 definition of CmGetLicenseInfo 
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
#define CM_GS_ALL_SERVERS             0x0020
#define CM_GS_REMOTE_SERVERS_ONLY     0x0040
#define CM_GS_IPADDR_IPv6MAPPED       0x0080

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
 definition of CmGetLastErrorText/CmGetLastErrorText2 and the belonging options (flCtrl)
*****/
#define CM_GLET_ERRORTEXT             0x0000
#define CM_GLET_DIALOG                0x0010
#define CM_GLET_MASK                  0x00FF
#define CM_GLET_LOCAL_ENCODING        0x0000
#define CM_GLET_UTF8_ENCODING         0x0100
#define CM_GLET_WCHAR_ENCODING        0x0200
#define CM_GLET_ENCODING_MASK         0x0F00

CODEMETER_API int CMAPIENTRY CmGetLastErrorText(CMULONG flCtrl,
    char *pszErrorText, CMUINT cbErrorText);

CODEMETER_API int CMAPIENTRY CmGetLastErrorText2(CMULONG flCtrl,
    void *pvErrorText, CMUINT cbErrorText);

/***** 
 definition of CmSetLastErrorCode
*****/
CODEMETER_API void CMAPIENTRY CmSetLastErrorCode(int idError);

/***************************************
 5.) Functions for programming a CmStick
****************************************/

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
 definition of CmGetRemoteContext/CmGetRemoteContext2/CmGetRemoteContextBuffer
*****/
#define CM_RMT_OVERWRITE              0x00000000
#define CM_RMT_APPEND                 0x00000001
#define CM_RMT_MASK                   0x000000ff
#define CM_RMT_LOCAL_ENCODING         0x00000000
#define CM_RMT_UTF8_ENCODING          0x00000100
#define CM_RMT_WCHAR_ENCODING         0x00000200
#define CM_RMT_ENCODING_MASK          0x00000f00

CODEMETER_API int CMAPIENTRY CmGetRemoteContext(const char *pszRacFile,
    CMUINT flCtrl, HCMSysEntry hcmBox, CMUINT *pulFirmCodes,
    CMUINT cbFirmCodes);

CODEMETER_API int CMAPIENTRY CmGetRemoteContext2(HCMSysEntry hcmBox, 
    CMUINT flCtrl, void *pvRacFile, CMUINT *pulFirmCodes,
    CMUINT cbFirmCodes);

CODEMETER_API int CMAPIENTRY CmGetRemoteContextBuffer(HCMSysEntry hcmBox, 
    CMUINT flCtrl, CMUINT *pulFirmCodes, CMUINT cbFirmCodes,
    void* pvMemBuffer, CMUINT cbMemBuffer, CMUINT* pcbWritten);

/***** 
 definition of CmSetRemoteUpdate/CmSetRemoteUpdate2/CmSetRemoteUpdateBuffer
*****/
CODEMETER_API int CMAPIENTRY CmSetRemoteUpdate(const char *pszRauFile,
    HCMSysEntry hcmBox);

CODEMETER_API int CMAPIENTRY CmSetRemoteUpdate2(HCMSysEntry hcmBox,
    CMUINT flCtrl, void *pvRauFile);

CODEMETER_API int CMAPIENTRY CmSetRemoteUpdateBuffer(HCMSysEntry hcmBox,
    CMUINT flCtrl, void* pvMemBuffer, CMUINT cbMemBuffer);

/***** 
 definition of CmListRemoteUpdate/CmListRemoteUpdate2/CmListRemoteUpdateBuffer
*****/
CODEMETER_API int CMAPIENTRY CmListRemoteUpdate(const char *pszRauFile,
    CMSERIAL *pcmSerials, CMUINT cbSerials);

CODEMETER_API int CMAPIENTRY CmListRemoteUpdate2(CMUINT flCtrl,
    void *pvRauFile, CMSERIAL *pcmSerials, CMUINT cbSerials);

CODEMETER_API int CMAPIENTRY CmListRemoteUpdateBuffer(CMUINT flCtrl, 
    void* pvMemBuffer, CMUINT cbMemBuffer, CMSERIAL *pcmSerials, CMUINT cbSerials);

/***** 
 definition of CmExecuteRemoteUpdate
*****/

/*
** For flCtrl
*/

#define CMFAS_CTL_CMD_MASK            0x00000003
#define CMFAS_CTL_CMD_LIST            0x00000000
#define CMFAS_CTL_CMD_UPDATE          0x00000001
#define CMFAS_CTL_CMD_UPDATE_EXISTING 0x00000002
#define CMFAS_CTL_CMD_GET_COUNT       0x00000003

#define CMFAS_CTL_OPT_MASK            0x00000010
#define CMFAS_CTL_OPT_ABORT_ON_ERROR  0x00000010

CODEMETER_API int CMAPIENTRY CmExecuteRemoteUpdate(
  HCMSysEntry hcmSubSystem,
  CMULONG flCtrl, void* pMemBuffer, CMUINT cbMemBuffer,
  CMFAS * pCmFasSelect, CMUINT cbCmFasSelect,
  CMFAS * pCmFasResult, CMUINT cbCmFasResult,
  CMUINT *pcbCmFasResultCount
  );

/************************************
 7.) Functions for enabling a CmStick 
*************************************/

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

/***************************************************
 8.) Functions for time and date update in a CmStick 
****************************************************/

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


/****************************
10.) Helper functions 
*****************************/

/***** 
 definition of CmConvertString and the belonging options (flCtrl)
*****/
#define CM_CONVERT_UTF8_TO_WCHAR        0x0001
#define CM_CONVERT_UTF8_TO_UCS2LE       0x0002
#define CM_CONVERT_UTF8_TO_ASCII        0x0003
#define CM_CONVERT_UTF8_TO_UTF16        0x0004
#define CM_CONVERT_UTF8_TO_LOCAL        0x0005

#define CM_CONVERT_WCHAR_TO_UCS2LE      0x0010
#define CM_CONVERT_WCHAR_TO_UTF8        0x0011
#define CM_CONVERT_WCHAR_TO_ASCII       0x0012
#define CM_CONVERT_WCHAR_TO_UTF16       0x0013
#define CM_CONVERT_WCHAR_TO_LOCAL       0x0014

#define CM_CONVERT_UCS2LE_TO_WCHAR      0x0020
#define CM_CONVERT_UCS2LE_TO_UTF8       0x0021
#define CM_CONVERT_UCS2LE_TO_ASCII      0x0022
#define CM_CONVERT_UCS2LE_TO_UTF16      0x0023
#define CM_CONVERT_UCS2LE_TO_LOCAL      0x0024

#define CM_CONVERT_ASCII_TO_WCHAR       0x0030
#define CM_CONVERT_ASCII_TO_UCS2LE      0x0031
#define CM_CONVERT_ASCII_TO_UTF8        0x0032
#define CM_CONVERT_ASCII_TO_UTF16       0x0033
#define CM_CONVERT_ASCII_TO_LOCAL       0x0034

#define CM_CONVERT_UTF16_TO_UTF8        0x0040
#define CM_CONVERT_UTF16_TO_UCS2LE      0x0041
#define CM_CONVERT_UTF16_TO_WCHAR       0x0042
#define CM_CONVERT_UTF16_TO_ASCII       0x0043
#define CM_CONVERT_UTF16_TO_LOCAL       0x0044

#define CM_CONVERT_LOCAL_TO_UTF8        0x0050
#define CM_CONVERT_LOCAL_TO_UCS2LE      0x0051
#define CM_CONVERT_LOCAL_TO_WCHAR       0x0052
#define CM_CONVERT_LOCAL_TO_ASCII       0x0053
#define CM_CONVERT_LOCAL_TO_UTF16       0x0054

#define CM_CONVERT_MASK                 0x00FF

CODEMETER_API int CMAPIENTRY CmConvertString(CMUINT flCtrl,
    const void *pszSrc, void *pszDst, CMUINT cbDst);


/****************************
11.) CmAct License managment
*****************************/

#define CM_GF_ACT_CREATE_LICENSE_INFO     0x0011
#define CM_GF_ACT_SETUP_LICENSE           0x0012
#define CM_GF_ACT_SETUP_LICENSE_STEP1     0x0013
#define CM_GF_ACT_SETUP_LICENSE_STEP2     0x0014
#define CM_GF_ACT_REGISTER                0x0015
#define CM_GF_ACT_UPDATE_LICENSE_DATA     0x0016
#define CM_GF_ACT_ACTIVATION_CODE         0x0017
#define CM_GF_ACT_LICENSE_BY_CODE         0x0018
#define CM_GF_ACT_REQUEST                 0x0019
#define CM_GF_ACT_CREATE_ACTIVATION_STEP1 0x001a
#define CM_GF_ACT_CREATE_ACTIVATION_STEP2 0x001b
#define CM_GF_ACT_CREATE_ACTIVATION_STEP3 0x001c
#define CM_GF_ACT_REMOVE                  0x001d
#define CM_GF_ACT_GETFILEINFO             0x001e
#define CM_GF_ACT_GET_INSTALLATION_ID     0x001f
#define CM_GF_ACT_SPLIT_INSTALLATION_ID   0x0020
#define CM_GF_ACT_GET_TEMPLATE_KEY        0x0021

#define CM_GF_ACT_MASK                    0x007f

CODEMETER_API int CMAPIENTRY CmActLicenseControl(HCMSysEntry hcmse,
    CMULONG flCtrl, const void *pvData, CMUINT cbData,
    void *pvReturn, CMUINT cbReturn);


/****************************
12.) License Borrowing
*****************************/

CODEMETER_API int CMAPIENTRY CmBorrow(HCMSysEntry hcmse, 
    CMULONG flCtrl, const char *pszServer);

/****************************
13.) Revalidation of boxes and licenses
*****************************/

CODEMETER_API int CMAPIENTRY CmRevalidateBox( HCMSysEntry hcmse, 
    CMULONG flCtrl );

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif /* !defined(_CODEMETER_INCLUDED) */
