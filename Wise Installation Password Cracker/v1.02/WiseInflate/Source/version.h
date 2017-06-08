/*
   version.h (for UnZip) by Info-ZIP.

   This header file is not copyrighted and may be distributed without
   restriction.  (That's a little geek humor, heh heh.)
 */

#ifndef __version_h   /* don't include more than once */
#define __version_h

#ifdef BETA
#  define BETALEVEL         "d BETA"
#  define VERSION_DATE      "31 May 97"         /* internal beta version */
#else
#  define BETALEVEL         ""
#  define VERSION_DATE      "31 May 1997"     /* official release version */
#  define RELEASE
#endif

#define UZ_MAJORVER    5   /* UnZip */
#define UZ_MINORVER    3

#define ZI_MAJORVER    2   /* ZipInfo */
#define ZI_MINORVER    2

#define PATCHLEVEL     1


/* these are obsolete but remain for backward compatibility: */
#define D2_MAJORVER    UZ_MAJORVER   /* DLL for OS/2 */
#define D2_MINORVER    UZ_MINORVER
#define D2_PATCHLEVEL  PATCHLEVEL

#define DW_MAJORVER    UZ_MAJORVER   /* DLL for MS Windows */
#define DW_MINORVER    UZ_MINORVER
#define DW_PATCHLEVEL  PATCHLEVEL

#define WIN_VERSION_DATE  VERSION_DATE

#endif /* !__version_h */
