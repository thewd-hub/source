/*---------------------------------------------------------------------------

  globals.c

  Routines to allocate and initialize globals, with or without threads.

  Contents:  registerGlobalPointer()
             deregisterGlobalPointer()
             getGlobalPointer()
             globalsCtor()

  ---------------------------------------------------------------------------*/

#define FUNZIP
#define UNZIP_INTERNAL
#include "unzip.h"

char *fnames[2] = {"*", NULL};   /* default filenames vector */

#ifndef REENTRANT
	struct Globals G;
#else /* REENTRANT */
#ifndef USETHREADID
	struct Globals *GG;
#else /* USETHREADID */

#define THREADID_ENTRIES  0x40

int lastScan;
struct Globals *threadPtrTable[THREADID_ENTRIES];
ulg threadIdTable [THREADID_ENTRIES] =
{
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,    /* Make sure there are */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,    /* THREADID_ENTRIES 0s */
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

static char Far TooManyThreads[] =
"error:  more than %d simultaneous threads.\n\
Some threads are probably not calling DESTROYTHREAD()\n";
static char Far EntryNotFound[] =
"error:  couldn't find global pointer in table.\n\
Maybe somebody accidentally called DESTROYTHREAD() twice.\n";
static char Far GlobalPointerMismatch[] =
"error:  global pointer in table does not match pointer passed as\
parameter\n";

static void registerGlobalPointer OF((__GPRO));

static void registerGlobalPointer(__G)
__GDEF
{
	int scan=0;
	ulg tid = GetThreadId();

	while (threadIdTable[scan] && scan < THREADID_ENTRIES)
		scan++;

	if (scan == THREADID_ENTRIES)
	{
		char *tooMany = LoadFarString(TooManyThreads);
		Info(slide, 0x421, ((char *)slide, tooMany, THREADID_ENTRIES));
		free(pG);
		EXIT(PK_MEM);   /* essentially memory error before we've started */
	}

	threadIdTable [scan] = tid;
	threadPtrTable[scan] = pG;
	lastScan = scan;
}

void deregisterGlobalPointer(__G)
__GDEF
{
	int scan=0;
	ulg tid = GetThreadId();

	while (threadIdTable[scan] != tid && scan < THREADID_ENTRIES)
		scan++;

	/*---------------------------------------------------------------------------
	There are two things we can do if we can't find the entry:  ignore it or
	scream.  The most likely reason for it not to be here is the user calling
	this routine twice.  Since this could cause BIG problems if any globals
	are accessed after the first call, we'd better scream.
	---------------------------------------------------------------------------*/

	if (scan == THREADID_ENTRIES || threadPtrTable[scan] != pG)
	{
		char *noEntry;
		if (scan == THREADID_ENTRIES)
			noEntry = LoadFarString(EntryNotFound);
		else
			noEntry = LoadFarString(GlobalPointerMismatch);
		Info(slide, 0x421, ((char *)slide, noEntry));
		EXIT(PK_WARN);   /* programming error, but after we're all done */
	}

	threadIdTable [scan] = 0;
	lastScan = scan;
	free(threadPtrTable[scan]);
}

struct Globals *getGlobalPointer()
{
	int scan=0;
	ulg tid = GetThreadId();

	while (threadIdTable[scan] != tid && scan < THREADID_ENTRIES)
		scan++;

	/*---------------------------------------------------------------------------
	There are two things we can do if we can't find the entry:  ignore it or
	scream.  The most likely reason for it not to be here is the user calling
	this routine twice.  Since this could cause BIG problems if any globals
	are accessed after the first call, we'd better scream.
	---------------------------------------------------------------------------*/

	if (scan == THREADID_ENTRIES)
	{
		char *noEntry;
		noEntry = LoadFarString(EntryNotFound);
		fprintf(stderr, noEntry);  /* can't use Info w/o a global pointer */
		EXIT(PK_ERR);   /* programming error while still working */
	}

	return threadPtrTable[scan];
}

#endif /* ?USETHREADID */
#endif /* ?REENTRANT */

struct Globals *globalsCtor()
{
	#ifdef REENTRANT
	struct Globals *pG = (struct Globals *)malloc(sizeof(struct Globals));

	if (!pG)
		return (struct Globals *)NULL;
#endif /* REENTRANT */
/* for REENTRANT version, G is defined as (*pG) */

memzero(&G, sizeof(struct Globals));

#ifdef CMS_MVS
	G.aflag=1;
	G.C_flag=1;
#endif

	G.lflag=(-1);
	G.wildzipfn = "";
	G.pfnames = fnames;
	G.pxnames = &fnames[1];
	G.pInfo = G.info;
	G.sol = TRUE;          /* at start of line */

#ifndef FUNZIP
	G.message = UzpMessagePrnt;
	G.input = UzpInput;           /* not used by anyone at the moment... */
#if defined(WINDLL) || defined(MACOS)
	G.mpause = NULL;              /* has scrollbars:  no need for pausing */
#else
	G.mpause = UzpMorePause;
#endif
	G.decr_passwd = UzpPassword;
#endif /* !FUNZIP */

#if (!defined(DOS_H68_OS2_W32) && !defined(AMIGA) && !defined(RISCOS))
#if (!defined(MACOS) && !defined(ATARI) && !defined(VMS))
	G.echofd = -1;
#endif /* !(MACOS || ATARI || VMS) */
#endif /* !(DOS_H68_OS2_W32 || AMIGA || RISCOS) */

#ifdef SYSTEM_SPECIFIC_CTOR
	SYSTEM_SPECIFIC_CTOR(__G);
#endif

#ifdef REENTRANT
#ifdef USETHREADID
	registerGlobalPointer(__G);
#else
	GG = &G;
#endif /* ?USETHREADID */
#endif /* REENTRANT */

	return &G;
}