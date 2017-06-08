/*---------------------------------------------------------------------------

  unzip.h (new)

  This header file contains the public macros and typedefs required by
  both the UnZip sources and by any application using the UnZip API.  If
  UNZIP_INTERNAL is defined, it includes unzpriv.h (containing includes,
  prototypes and extern variables used by the actual UnZip sources).

  ---------------------------------------------------------------------------*/

// conditional expression is constant
#pragma warning(disable:4127)

// old-style function declarations
#pragma warning(disable:4131)


#ifndef __unzip_h   /* prevent multiple inclusions */
#define __unzip_h


/*---------------------------------------------------------------------------
    Predefined, machine-specific macros.
  ---------------------------------------------------------------------------*/

#ifdef POCKET_UNZIP             /* WinCE port */
#  include "wince/punzip.h"     /* must appear before windows.h */
#endif

#ifdef WINDLL
#  include <windows.h>
#endif

#ifdef __GO32__                 /* MS-DOS extender:  NOT Unix */
#  ifdef unix
#    undef unix
#  endif
#  ifdef __unix
#    undef __unix
#  endif
#  ifdef __unix__
#    undef __unix__
#  endif
#endif

#if ((defined(__convex__) || defined(__convexc__)) && !defined(CONVEX))
#  define CONVEX
#endif

#if (defined(unix) || defined(__unix) || defined(__unix__))
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* unix || __unix || __unix__ */
#if (defined(M_XENIX) || defined(COHERENT) || defined(__hpux))
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* M_XENIX || COHERENT || __hpux */
#if (defined(CONVEX) || defined(MINIX) || defined(_AIX) || defined(__QNX__))
#  ifndef UNIX
#    define UNIX
#  endif
#endif /* CONVEX || MINIX || _AIX || __QNX__ */

#if (defined(VM_CMS) || defined(MVS))
#  define CMS_MVS
#endif

#if (defined(__OS2__) && !defined(OS2))
#  define OS2
#endif

#if (defined(__VMS) && !defined(VMS))
#  define VMS
#endif

#if (defined(__WIN32__) && !defined(WIN32))
#  define WIN32
#endif

#ifdef __COMPILER_KCC__
#  include <c-env.h>
#  ifdef SYS_T20
#    define TOPS20
#  endif
#endif /* __COMPILER_KCC__ */

/* Borland C does not define __TURBOC__ if compiling for a 32-bit platform */
#ifdef __BORLANDC__
#  ifndef __TURBOC__
#    define __TURBOC__
#  endif
#  if (!defined(__MSDOS__) && !defined(OS2) && !defined(WIN32))
#    define __MSDOS__
#  endif
#endif

/* define MSDOS for Turbo C (unless OS/2) and Power C as well as Microsoft C */
#ifdef __POWERC
#  define __TURBOC__
#  define MSDOS
#endif /* __POWERC */

#if (defined(__MSDOS__) && !defined(MSDOS))   /* just to make sure */
#  define MSDOS
#endif

#if (defined(linux) && !defined(LINUX))
#  define LINUX
#endif

#ifdef __riscos
#  define RISCOS
#endif

/* use prototypes and ANSI libraries if __STDC__, or Microsoft or Borland C, or
 * Silicon Graphics, or Convex?, or IBM C Set/2, or GNU gcc/emx, or Watcom C,
 * or Macintosh, or Windows NT, or Sequent, or Atari or IBM RS/6000.
 */
#if (defined(__STDC__) || defined(MSDOS) || defined(WIN32) || defined(__EMX__))
#  ifndef PROTO
#    define PROTO
#  endif
#  ifndef MODERN
#    define MODERN
#  endif
#endif
#if (defined(__IBMC__) || defined(__BORLANDC__) || defined(__WATCOMC__))
#  ifndef PROTO
#    define PROTO
#  endif
#  ifndef MODERN
#    define MODERN
#  endif
#endif
#if (defined(THINK_C) || defined(MPW) || defined(ATARI_ST) || defined(RISCOS))
#  ifndef PROTO
#    define PROTO
#  endif
#  ifndef MODERN
#    define MODERN
#  endif
#endif
/* Sequent running Dynix/ptx:  non-modern compiler */
#if (defined(_AIX) || defined(sgi) || (defined(_SEQUENT_) && !defined(PTX)))
#  ifndef PROTO
#    define PROTO
#  endif
#  ifndef MODERN
#    define MODERN
#  endif
#endif
#if (defined(CMS_MVS) || defined(__BEOS__))  /* || defined(CONVEX) */
#  ifndef PROTO
#    define PROTO
#  endif
#  ifndef MODERN
#    define MODERN
#  endif
#endif

/* turn off prototypes if requested */
#if (defined(NOPROTO) && defined(PROTO))
#  undef PROTO
#endif

/* used to remove arguments in function prototypes for non-ANSI C */
#ifdef PROTO
#  define OF(a) a
#else
#  define OF(a) ()
#endif

/* enable the "const" keyword only if MODERN and if not otherwise instructed */
#ifdef MODERN
#  if (!defined(ZCONST) && (defined(USE_CONST) || !defined(NO_CONST)))
#    define ZCONST const
#  endif
#endif

#ifndef ZCONST
#  define ZCONST
#endif

/* bad or (occasionally?) missing stddef.h: */
#if (defined(M_XENIX) || defined(DNIX))
#  define NO_STDDEF_H
#endif

#if (defined(M_XENIX) && !defined(M_UNIX))   /* SCO Xenix only, not SCO Unix */
#  define SCO_XENIX
#  define NO_LIMITS_H        /* no limits.h, but MODERN defined */
#  define NO_UID_GID         /* no uid_t/gid_t */
#  define size_t int
#endif

#ifdef realix   /* Modcomp Real/IX, real-time SysV.3 variant */
#  define SYSV
#  define NO_UID_GID         /* no uid_t/gid_t */
#endif

#if (defined(_AIX) && !defined(_ALL_SOURCE))
#  define _ALL_SOURCE
#endif

#if defined(apollo)          /* defines __STDC__ */
#    define NO_STDLIB_H
#endif

#ifdef DNIX
#  define SYSV
#  define SHORT_NAMES         /* 14-char limitation on path components */
/* #  define FILENAME_MAX  14 */
#  define FILENAME_MAX  NAME_MAX    /* GRR:  experiment */
#endif

#if (defined(SYSTEM_FIVE) || defined(__SYSTEM_FIVE))
#  ifndef SYSV
#    define SYSV
#  endif
#endif /* SYSTEM_FIVE || __SYSTEM_FIVE */
#if (defined(M_SYSV) || defined(M_SYS5))
#  ifndef SYSV
#    define SYSV
#  endif
#endif /* M_SYSV || M_SYS5 */
/* __SVR4 and __svr4__ catch Solaris on at least some combos of compiler+OS */
#if (defined(__SVR4) || defined(__svr4__) || defined(sgi) || defined(__hpux))
#  ifndef SYSV
#    define SYSV
#  endif
#endif /* __SVR4 || __svr4__ || sgi || __hpux */
#if (defined(LINUX) || defined(__QNX__))
#  ifndef SYSV
#    define SYSV
#  endif
#endif /* LINUX || __QNX__ */

#if (defined(ultrix) || defined(__ultrix) || defined(bsd4_2))
#  if (!defined(BSD) && !defined(SYSV))
#    define BSD
#  endif
#endif /* ultrix || __ultrix || bsd4_2 */
#if (defined(sun) || defined(pyr) || defined(CONVEX))
#  if (!defined(BSD) && !defined(SYSV))
#    define BSD
#  endif
#endif /* sun || pyr || CONVEX */

#ifdef pyr  /* Pyramid:  has BSD and AT&T "universes" */
#  ifdef BSD
#    define pyr_bsd
#    define USE_STRINGS_H  /* instead of more common string.h */
#    define ZMEM           /* ZMEM now uses bcopy/bzero: not in AT&T universe */
#  endif                   /* (AT&T memcpy claimed to be very slow, though) */
#  define DECLARE_ERRNO
#endif /* pyr */

/* stat() bug for Borland, VAX C (also GNU?), and Atari ST MiNT on TOS
 * filesystems:  returns 0 for wildcards!  (returns 0xffffffff on Minix
 * filesystem or `U:' drive under Atari MiNT.)  Watcom C was previously
 * included on this list; it would be good to know what version the problem
 * was fixed at, if it did exist.  Watcom 10.6 has a separate stat() problem:
 * it fails on "." when the current directory is a root.  This is covered by
 * giving it a separate definition of SSTAT in OS-specific header files. */
#if (defined(__TURBOC__) || defined(VMS) || defined(__MINT__))
#  define WILD_STAT_BUG
#endif

#ifdef WILD_STAT_BUG
#  define SSTAT(path,pbuf) (iswild(path) || stat(path,pbuf))
#else
#  define SSTAT stat
#endif

#ifdef REGULUS  /* returns the inode number on success(!)...argh argh argh */
#  define stat(p,s) zstat(p,s)
#endif

#define STRNICMP zstrnicmp


/*---------------------------------------------------------------------------
    OS-dependent includes
  ---------------------------------------------------------------------------*/

#ifdef EFT
#  define LONGINT off_t  /* Amdahl UTS nonsense ("extended file types") */
#else
#  define LONGINT long
#endif

#ifdef MODERN
#  ifndef NO_STDDEF_H
#    include <stddef.h>
#  endif
#  ifndef NO_STDLIB_H
#    include <stdlib.h>  /* standard library prototypes, malloc(), etc. */
#  endif
   typedef size_t extent;
   typedef void zvoid;
#else /* !MODERN */
#  ifndef AOS_VS         /* mostly modern? */
#    ifndef CMS_MVS
       LONGINT lseek();
#    endif
#    ifdef VAXC          /* not fully modern, but does have stdlib.h and void */
#      include <stdlib.h>
#    else
       char *malloc();
#      define void int
#    endif /* ?VAXC */
#  endif /* !AOS_VS */
   typedef unsigned int extent;
   typedef char zvoid;
#endif /* ?MODERN */


/*---------------------------------------------------------------------------
    Grab system-dependent definition of EXPENTRY for prototypes below.
  ---------------------------------------------------------------------------*/

#if 0
#if (defined(OS2) && !defined(FUNZIP))
#  ifdef UNZIP_INTERNAL
#    define INCL_NOPM
#    define INCL_DOSNLS
#    define INCL_DOSPROCESS
#    define INCL_DOSDEVICES
#    define INCL_DOSDEVIOCTL
#    define INCL_DOSERRORS
#    define INCL_DOSMISC
#    ifdef OS2DLL
#      define INCL_REXXSAA
#      include <rexxsaa.h>
#    endif
#  endif /* UNZIP_INTERNAL */
#  include <os2.h>
#  define UZ_EXP EXPENTRY
#endif /* OS2 && !FUNZIP */
#endif /* 0 */

#if (defined(OS2) && !defined(FUNZIP))
#  if defined(__IBMC__) || defined(__WATCOMC__)
#    define UZ_EXP  _System    /* compiler keyword */
#  else
#    define UZ_EXP
#  endif
#endif /* OS2 && !FUNZIP */

#ifdef WINDLL
#  ifndef EXPENTRY
#    define UZ_EXP WINAPI
#  else
#    define UZ_EXP EXPENTRY
#  endif
#endif

#ifndef UZ_EXP
#  define UZ_EXP
#endif


/*---------------------------------------------------------------------------
    Public typedefs.
  ---------------------------------------------------------------------------*/

typedef unsigned char   uch;    /* code assumes unsigned bytes; these type-  */
typedef unsigned short  ush;    /*  defs replace byte/UWORD/ULONG (which are */
typedef unsigned long   ulg;    /*  predefined on some systems) & match zip  */

/* InputFn is not yet used and is likely to change: */
#ifdef PROTO
   typedef int   (UZ_EXP MsgFn)     (zvoid *pG, uch *buf, ulg size, int flag);
   typedef int   (UZ_EXP InputFn)   (zvoid *pG, uch *buf, int *size, int flag);
   typedef void  (UZ_EXP PauseFn)   (zvoid *pG, ZCONST char *prompt, int flag);
   typedef int   (UZ_EXP PasswdFn)  (zvoid *pG, int *rcnt, char *pwbuf,
                                     int size, ZCONST char *zfn,
                                     ZCONST char *efn);
#ifdef WINDLL
   typedef int   (WINAPI ReplaceFn) (char *);
   typedef void  (WINAPI SoundFn)   (void);
#endif
#else /* !PROTO */
   typedef int   (UZ_EXP MsgFn)     ();
   typedef int   (UZ_EXP InputFn)   ();
   typedef void  (UZ_EXP PauseFn)   ();
   typedef int   (UZ_EXP PasswdFn)  ();
#ifdef WINDLL
   typedef int   (WINAPI ReplaceFn) ();
   typedef void  (WINAPI SoundFn)   ();
#endif
#endif /* ?PROTO */

typedef struct _UzpBuffer {    /* rxstr */
    ulg   strlength;           /* length of string */
    char  *strptr;             /* pointer to string */
} UzpBuffer;

typedef struct _UzpInit {
    ulg structlen;             /* length of the struct being passed */

    /* GRR: can we assume that each of these is a 32-bit pointer?  if not,
     * does it matter? add "far" keyword to make sure? */
    MsgFn *msgfn;
    InputFn *inputfn;
    PauseFn *pausefn;

    void (* UZ_EXP userfn)();  /* user init function to be called after */
                               /*  globals constructed and initialized */

    /* pointer to program's environment area or something? */
    /* hooks for performance testing? */
    /* hooks for extra unzip -v output? (detect CPU or other hardware?) */
    /* anything else?  let me (Greg) know... */
} UzpInit;

/* intended to be a private struct: */
typedef struct _ver {
    uch major;              /* e.g., integer 5 */
    uch minor;              /* e.g., 2 */
    uch patchlevel;         /* e.g., 0 */
    uch not_used;
} _version_type;

typedef struct _UzpVer {
    ulg structlen;          /* length of the struct being passed */
    ulg flag;               /* bit 0: is_beta   bit 1: uses_zlib */
    char *betalevel;        /* e.g., "g BETA" or "" */
    char *date;             /* e.g., "4 Sep 95" (beta) or "4 September 1995" */
    char *zlib_version;     /* e.g., "0.95" or NULL */
    _version_type unzip;
    _version_type zipinfo;
    _version_type os2dll;
    _version_type windll;
} UzpVer;

typedef struct central_directory_file_header { /* CENTRAL */
    uch version_made_by[2];
    uch version_needed_to_extract[2];
    ush general_purpose_bit_flag;
    ush compression_method;
    ush last_mod_file_time;
    ush last_mod_file_date;
    ulg crc32;
    ulg csize;
    ulg ucsize;
    ush filename_length;
    ush extra_field_length;
    ush file_comment_length;
    ush disk_number_start;
    ush internal_file_attributes;
    ulg external_file_attributes;
    ulg relative_offset_local_header;
} cdir_file_hdr;


#define UZPINIT_LEN   sizeof(UzpInit)
#define UZPVER_LEN    sizeof(UzpVer)
#define cbList(func)  int (* UZ_EXP func)(char *filename, cdir_file_hdr *crec)


/*---------------------------------------------------------------------------
    Return (and exit) values of the public UnZip API functions.
  ---------------------------------------------------------------------------*/

/* external return codes */
#define PK_OK              0   /* no error */
#define PK_COOL            0   /* no error */
#define PK_GNARLY          0   /* no error */
#define PK_WARN            1   /* warning error */
#define PK_ERR             2   /* error in zipfile */
#define PK_BADERR          3   /* severe error in zipfile */
#define PK_MEM             4   /* insufficient memory (during initialization) */
#define PK_MEM2            5   /* insufficient memory (password failure) */
#define PK_MEM3            6   /* insufficient memory (file decompression) */
#define PK_MEM4            7   /* insufficient memory (memory decompression) */
#define PK_MEM5            8   /* insufficient memory (not yet used) */
#define PK_NOZIP           9   /* zipfile not found */
#define PK_PARAM          10   /* bad or illegal parameters specified */
#define PK_FIND           11   /* no files found */
#define PK_DISK           50   /* disk full */
#define PK_EOF            51   /* unexpected EOF */

#define IZ_CTRLC          80   /* user hit ^C to terminate */
#define IZ_UNSUP          81   /* no files found: all unsup. compr/encrypt. */
#define IZ_BADPWD         82   /* no files found: all had bad password */

/* internal and DLL-only return codes */
#define IZ_DIR            76   /* potential zipfile is a directory */
#define IZ_CREATED_DIR    77   /* directory created: set time and permissions */
#define IZ_VOL_LABEL      78   /* volume label, but can't set on hard disk */
#define IZ_EF_TRUNC       79   /* local extra field truncated (PKZIP'd) */

/* return codes of password fetches (negative = user abort; positive = error) */
#define IZ_PW_ENTERED      0   /* got some password string; use/try it */
#define IZ_PW_CANCEL      -1   /* no password available (for this entry) */
#define IZ_PW_CANCELALL   -2   /* no password, skip any further pwd. request */
#define IZ_PW_ERROR        5   /* = PK_MEM2 : failure (no mem, no tty, ...) */


/*---------------------------------------------------------------------------
    Prototypes for public UnZip API (DLL) functions.
  ---------------------------------------------------------------------------*/

int      UZ_EXP UzpMain            OF((int argc, char **argv));
int      UZ_EXP UzpAltMain         OF((int argc, char **argv, UzpInit *init));
UzpVer * UZ_EXP UzpVersion         OF((void));
int      UZ_EXP UzpUnzipToMemory   OF((char *zip, char *file,
                                       UzpBuffer *retstr));
int      UZ_EXP UzpFileTree        OF((char *name, cbList(callBack),
                                       char *cpInclude[], char *cpExclude[]));

/* default I/O functions (can be swapped out via UzpAltMain() entry point): */

int      UZ_EXP UzpMessagePrnt   OF((zvoid *pG, uch *buf, ulg size, int flag));
int      UZ_EXP UzpMessageNull   OF((zvoid *pG, uch *buf, ulg size, int flag));
int      UZ_EXP UzpInput         OF((zvoid *pG, uch *buf, int *size, int flag));
void     UZ_EXP UzpMorePause     OF((zvoid *pG, ZCONST char *prompt, int flag));
int      UZ_EXP UzpPassword      OF((zvoid *pG, int *rcnt, char *pwbuf,
                                     int size, ZCONST char *zfn,
                                     ZCONST char *efn));


/*---------------------------------------------------------------------------
    Remaining private stuff for UnZip compilation.
  ---------------------------------------------------------------------------*/

#ifdef UNZIP_INTERNAL
#  include "unzpriv.h"
#endif


#endif /* !__unzip_h */
