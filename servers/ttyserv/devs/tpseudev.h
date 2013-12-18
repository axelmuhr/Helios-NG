#include <helios.h>			/* standard header		*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <servlib.h>
#include <string.h>
#include <root.h>
#include <message.h>
#include <link.h>
#include <task.h>
#include <sem.h>
#include <protect.h>
#include <codes.h>
#include <signal.h>
#include <gsp.h>
#ifdef __TRAN
#include <process.h>
#else
#include <thread.h>
#endif
#include <attrib.h>
#include <ioevents.h>
#include <nonansi.h>
#include <device.h>
#include <queue.h>
#include <posix.h>

#include "ttydev.h"

#define RINGSIZE 1048
#define min(a,b) 		(a) > (b) ? (b) : (a)
#define max(a,b) 		(a) > (b) ? (a) : (b)

typedef struct Ring {
	Semaphore	lock;
	char		*buf;
	int		size;
	int		fp;
	int		ep;
	bool		empty;
} Ring;


typedef struct Pty {
	ObjNode		ObjNode;
	Semaphore	ReadLock;
	Semaphore	WriteLock;
	Semaphore	SelectLock;
	Port		SelectPort;
	word		SelectMode;
	bool		terminating;	/* to signal termination	*/
	Ring		iring;		/* input ring			*/
	Ring		oring;		/* output ring			*/
	char		ibuf[RINGSIZE]; /* input buffer			*/
	char		obuf[RINGSIZE]; /* output buffer		*/
	word		Flags;
#define	PF_RCOLL	0x01
#define	PF_WCOLL	0x02
#define	PF_NBIO		0x04
#define	PF_PKT		0x08		/* packet mode */
#define	PF_STOPPED	0x10		/* user told stopped */
#define	PF_REMOTE	0x20		/* remote and flow controlled input */
#define	PF_NOSTOP	0x40
#define PF_UCNTL	0x80		/* user control mode */
	TermInfoReq	*setinfo;
	TermInfoReq	*getinfo;
	DCB		*dcb;	
	Semaphore	Forever;
} Pty;


typedef struct TermDCB {
	DCB		dcb;
	Semaphore	DispTerm;
	Object		*nte;		/* Pty's root Name table entry	*/
	DirNode		*PtyRoot;	/* Pty's root node		*/
	NameInfo	*PtyName;	/* Info structure to create 	*/
					/* the name table entry		*/
	DispatchInfo	*PtyInfo;	/* Info for servlib.Dispatch ()	*/
	Pty		*Pty;		/* Pty itself			*/
	char		McName[100];	/* processor name		*/
} TermDCB;
