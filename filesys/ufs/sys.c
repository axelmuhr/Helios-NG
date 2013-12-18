/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 29 July 1992                                                 */
/* File: sys.c                                                        */
/*                                                                    */
/* This file contains the variable definitions which were made extern */
/* in the header files of include/sys in order to prevent             */
/* duplicate definition errors while linking under helios compiler.   */
/* This is therefore grouping of all these externs.                   */
/*                                                                    */
/* This file also contains the system variables which are initialised */
/* here.                                                              */
/*                                                                    */
/* $Id: sys.c,v 1.1 1992/09/16 09:29:06 al Exp $ */
/* $Log: sys.c,v $
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 * */

#include "param.h"
#include "systm.h"
#include "buf.h"
#include "mount.h"
#include "proc.h"
#include "specdev.h"
#include "vnode.h"
#include "filedesc.h"
#ifdef __HELIOS
#include "resourcvar.h"
#else
#include "resourcevar.h"
#endif

/* From include/sys/buf.h */
struct	buf *buf;		/* the buffer pool itself */
char	*buffers;
extern int nbuf;		/* number of buffer headers  (defined in conf.c) */
int	bufpages;		/* number of memory pages in the buffer pool */
struct	buf *swbuf;		/* swap I/O headers */
int	nswbuf;
struct	bufhd bufhash[BUFHSZ];	/* heads of hash lists */
struct	buf bfreelist[BQUEUES];	/* heads of available lists */
struct	buf bswlist;		/* head of free swap header list */
struct	buf *bclnlist;		/* head of cleaned page list */

/* From include/sys/filedesc.h */
struct filedesc0 filedesc0;

/* From include/sys/kernel.h */
long hostid;
char hostname[MAXHOSTNAMELEN];
int hostnamelen;
struct timeval boottime;
struct timeval time;
struct timezone tz;			/* XXX */
int hz = 100;				/* clock frequency */
int phz = 100;				/* alternate clock's frequency */
int tick;
int lbolt;				/* once a second sleep address */
fixpt_t	averunnable[3];

/* From include/sys/namei.h */
u_long	nextvnodeid;

/* From include/sys/proc.h */
struct	proc *zombproc, *allproc;	/* lists of procs in various states */
struct	proc *initproc, *pageproc;	/* process slots for init, pager */
int	whichqs;		/* bit mask summarizing non-empty qs's */
struct	prochd qs[NQS];
struct pcred cred0;

/* From include/sys/resourcevar.h */
struct plimit limit0;			/* Limits for process 0 */

/* From include/sys/socketvar.h */
u_long	sb_max;

/* From include/sys/specdev.h */
struct vnode *speclisth[SPECHSZ];

/* From include/sys/vnode.h */
struct vnode *rootvp;
struct vnode *vfreeh, **vfreet;
int numvnodes;

/* SYSTEM VARIABLES */
struct proc proc0;
struct proc *curproc = &proc0;		/* Process 0 */
u_long nextgennumber = 0;		/* Next generation number to assign */


