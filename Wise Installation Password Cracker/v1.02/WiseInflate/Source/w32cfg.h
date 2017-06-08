/*---------------------------------------------------------------------------
    Win32 specific configuration section:
  ---------------------------------------------------------------------------*/

#ifndef __w32cfg_h
#define __w32cfg_h

#include <sys/types.h>        /* off_t, time_t, dev_t, ... */
#include <sys/stat.h>
#include <io.h>               /* read(), open(), etc. */
#include <time.h>
#include <memory.h>
#ifndef __EMX__
#  include <direct.h>         /* mkdir() */
#endif
#include <fcntl.h>
#if (defined(MSC) || defined(__WATCOMC__))
#  include <sys/utime.h>
#else
#  include <utime.h>
#endif
#if defined(FILEIO_C)
#  include <conio.h>
#  include <windows.h>
#  ifdef __RSXNT__
#    include "win32/rsxntwin.h"
#  endif
#endif
#if (defined(ENVARGS_C) || defined(EXTRACT_C) || defined(UNZIP_C) || \
     defined(ZCRYPT_INTERNAL))
#  include <windows.h>
#  ifdef __RSXNT__
#    include "win32/rsxntwin.h"
#  endif
#endif

#if defined(MSC)
#  define DIR_END     '\\'   /* ZipInfo with VC++ 4.0 requires this */
#endif
#define DATE_FORMAT   DF_MDY
#define lenEOL        2
#define PutNativeEOL  {*q++ = native(CR); *q++ = native(LF);}

#ifndef NT_TZBUG_WORKAROUND
#  define NT_TZBUG_WORKAROUND
#endif
#define USE_EF_UT_TIME
#ifdef __RSXNT__
#  ifndef NO_NTSD_WITH_RSXNT
#    define NO_NTSD_WITH_RSXNT  /* RSXNT windows.h does not yet support NTSD */
#  endif
#else /* !__RSXNT__ */
#  if (!defined(NO_NTSD_EAS) && !defined(NTSD_EAS))
#    define NTSD_EAS    /* enable NTSD support unless explicitly suppressed */
#  endif
#endif /* ?__RSXNT__ */

/* handlers for OEM <--> ANSI string conversions */
#ifdef __RSXNT__
   /* RSXNT uses OEM coded strings in functions supplied by C RTL */
#  ifdef CRTL_CP_IS_ISO
#    undef CRTL_CP_IS_ISO
#  endif
#  ifndef CRTL_CP_IS_OEM
#    define CRTL_CP_IS_OEM
#  endif
#else
   /* "real" native WIN32 compilers use ANSI coded strings in C RTL calls */
#  ifndef CRTL_CP_IS_ISO
#    define CRTL_CP_IS_ISO
#  endif
#  ifdef CRTL_CP_IS_OEM
#    undef CRTL_CP_IS_OEM
#  endif
#endif

#ifdef CRTL_CP_IS_ISO
   /* C RTL's file system support assumes ANSI coded strings */
#  define ISO_TO_INTERN(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#  define OEM_TO_INTERN(src, dst)  OemToAnsi(src, dst)
#  define INTERN_TO_ISO(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#  define INTERN_TO_OEM(src, dst)  AnsiToOem(src, dst)
#endif /* CRTL_CP_IS_ISO */
#ifdef CRTL_CP_IS_OEM
   /* C RTL's file system support assumes OEM coded strings */
#  define ISO_TO_INTERN(src, dst)  AnsiToOem(src, dst)
#  define OEM_TO_INTERN(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#  define INTERN_TO_ISO(src, dst)  OemToAnsi(src, dst)
#  define INTERN_TO_OEM(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#endif /* CRTL_CP_IS_OEM */
#define _OEM_INTERN(str1) OEM_TO_INTERN(str1, str1)
#define _ISO_INTERN(str1) ISO_TO_INTERN(str1, str1)
#ifndef WINDLL
   /* Despite best intentions, for the command-line version UzpPassword()
    * could return either character set, depending on whether running under
    * Win95 (DOS-session) or WinNT (native WinNT command interpreter)! */
#  define STR_TO_CP2(dst, src)  (AnsiToOem(src, dst), dst)
#  define STR_TO_CP3(dst, src)  (OemToAnsi(src, dst), dst)
#else
   /* The WINDLL front end is known to supply ISO/ANSI-coded passwords! */
#  define STR_TO_CP2(dst, src)  (AnsiToOem(src, dst), dst)
#endif
/* dummy defines to disable these functions, they are not needed */
#define STR_TO_OEM
#define STR_TO_ISO

/* Static variables that we have to add to struct Globals: */
#define SYSTEM_SPECIFIC_GLOBALS \
    int created_dir, renamed_fullpath, fnlen;\
    unsigned nLabelDrive;\
    char *rootpath, *buildpathHPFS, *buildpathFAT, *endHPFS, *endFAT;\
    char *dirname, *wildname, matchname[FILNAMSIZ];\
    int rootlen, have_dirname, dirnamelen, notfirstcall;\
    zvoid *wild_dir;

/* created_dir, renamed_fullpath, fnlen, and nLabelDrive are used by   */
/*    both mapname() and checkdir().                                   */
/* rootlen, rootpath, buildpathHPFS, buildpathFAT, endHPFS, and endFAT */
/*    are used by checkdir().                                          */
/* wild_dir, dirname, wildname, matchname[], dirnamelen, have_dirname, */
/*    and notfirstcall are used by do_wild().                          */

#if (defined(_MSC_VER) && !defined(MSC))
#  define MSC
#endif

#ifdef __WATCOMC__
#  undef SSTAT
#  define SSTAT stat_bandaid
   int stat_bandaid(const char *path, struct stat *buf);

#  ifdef __386__
#    ifndef WATCOMC_386
#      define WATCOMC_386
#    endif
#    define __32BIT__
#    undef far
#    define far
#    undef near
#    define near

/* Get asm routines to link properly without using "__cdecl": */
#    ifndef USE_ZLIB
#      pragma aux crc32         "_*" parm caller [] value [eax] modify [eax]
#      pragma aux get_crc_table "_*" parm caller [] value [eax] \
                                      modify [eax ecx edx]
#    endif /* !USE_ZLIB */
#  endif /* __386__ */

#  ifndef EPIPE
#    define EPIPE -1
#  endif
#  define PIPE_ERROR (errno == EPIPE)
#endif /* __WATCOMC__ */

#endif /* !__w32cfg_h */
