/*
 * Name:	MG 2a
 *
 * 		Very early-on system-specific initialization for whatever's
 *		necessary.
 * Last edit:	05-May-88, Stephen Walton, swalton@solar.stanford.edu
 * Created:	Stephen Walton, 3-Dec-87.
 *	
 */
#include <libraries/dosextens.h>
#include "sysdef.h"

#undef	FALSE
#undef	TRUE
#define	TRUE	1
#define	FALSE	0

#ifdef USE_ARP
struct Library	*ArpBase;
extern struct	Library *OpenLibrary();
#endif

#ifndef	NO_DIR
extern struct	Task *FindTask();
static BPTR	StartLock;
char		MyDirName[MAXPATH];
extern BPTR	DupLock(), CurrentDir();
#endif NO_DIR

sysinit()
{
	long len;
	BPTR MyDirLock;

#ifdef USE_ARP
	if (!(ArpBase = OpenLibrary("arp.library", 0L)))
		panic("Compiled with USE_ARP, but arp.library not found");
#endif
#ifndef NO_DIR
	/*
	 * The following attempt to be clever assigns the external StartLock
	 * to the lock on the current directory, then switches our CurrentDir
	 * to a duplicate of that lock so we can restore the original lock
	 * on exit.
	 */

	StartLock = ((struct Process *)FindTask(0L))->pr_CurrentDir;
	(void) CurrentDir(MyDirLock = DupLock(StartLock));
	len = PathName(MyDirLock, MyDirName, MAXPATH/31L);
#endif NO_DIR
}

/*
 * System dependent cleanup for the Amiga.
 */
syscleanup()
{
	UnLock(CurrentDir(StartLock));	/* restore startup directory	*/
#ifdef USE_ARP
	if (ArpBase)
		CloseLibrary(ArpBase);
#endif
}
