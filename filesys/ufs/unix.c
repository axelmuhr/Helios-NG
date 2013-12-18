/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 29 July 1992                                                 */
/* File: unix.c                                                       */
/*                                                                    */
/* This file contains the "faked" UNIX routines to allow the UNIX     */
/* kernel to run on top of Helios.                                    */
/*                                                                    */
/* This file also contains HELIOS routines for debugging.             */
/*                                                                    */
/* $Id: unix.c,v 1.4 1992/12/09 17:36:36 al Exp $ */
/* $Log: unix.c,v $
 * Revision 1.4  1992/12/09  17:36:36  al
 * Changed procname to myprocname and back_trace to my_back_trace.
 * I must update these files later to use the proper helios calls.
 *
 * Revision 1.3  1992/10/19  08:15:12  al
 * Added support for no output for use when ufs in nucleus.
 *
 * Revision 1.2  1992/10/13  11:47:03  al
 * Syntax fixed to compile for C40
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 * */

#include "helios.h"
#include "stddef.h"
#include "module.h"
#include "param.h"
#include "proc.h"
#include "user.h"
#include "malloc.h"
#include "buf.h"
#include "sem.h"
#include "queue.h"
#include "filedesc.h"
#include "gsp.h"

#include "backtrace.c"

extern char *progname;

/* Helios variables used to help emulate unix */
Semaphore kernel;

/************************************************************************
 *                           FAKE UNIX ROUTINES                         *
 ************************************************************************/
#define alex_stub(name,msg)	name() { \
					panic(msg); \
					return(1); \
				}

/* Generic null routine. */
int nullop()
{
  return(0);
}

/* Find a process given its arguments */
struct proc *pfind(int args)
{
	/* Returns the current running process */
	IOdebug("%s: PANIC; pfind called by %s",
		progname,myprocname(returnlink_(args)));
	return(curproc);
}

/* Virtual memory routine to unallocate regions mapped to the given file */
int munmapfd(p,fd)
struct proc *p;
int fd;
{
	p->p_fd->fd_ofileflags[fd] &= ~UF_MAPPED;
	IOdebug("%s: VM panic; munmapfd called by %s",
		progname,myprocname(returnlink_(p)));
}

extern struct timeval time;
extern struct plimit limit0;

/* Initialise the time of day */
inittodr(base)
time_t base;
{
	time.tv_sec = GetDate();
	time.tv_usec = 0;
}

/* The stack error call */
void _stack_error(Proc *p)
{	int i;
		
	int *sp = (int *)&p;
	int *d;
	char *pp = (char *) myprocname(returnlink_(p));

	IOdebug("ufs: stack error in %s (%s)0x%x at %x  return 0x%x",p->Name,myprocname(returnlink_(p)),myprocname(returnlink_(p)),&p,returnlink_(p));
	IOdebug("ufs: stack error; p %x : %x %x %x %x %x",p,*p);
	sp -= 2;
	IOdebug("ufs: stack error; sp %x : %",sp);
	for( i = 0; i < 8 ; i++ ) IOdebug("ufs: stack error; %x %",sp[i]);
	IOdebug("");
	d = (int *)sp[5];
	IOdebug("ufs: stack error; d  %x : %x %x",d,d[0],d[1]);
	for( i = 0; i < 64; i++ )
	{
		if( (i % 16 ) == 0 ) IOdebug("ufs: stack error; 0x%x: ",pp+i);
		IOdebug("ufs: stack error; %x %",pp[i]);
	}
	IOdebug("");
	my_back_trace(i); 
}

int uiomove(caddr_t p, int len, struct uio *uio)
{
	while( len )
	{
		struct iovec *iov = uio->uio_iov;
		int tfr = len;
		
		if (tfr > iov->iov_len) tfr = iov->iov_len;
		
		if( uio->uio_rw == UIO_READ ) 
			memcpy(iov->iov_base,p,tfr);
		else 	memcpy(p,iov->iov_base,tfr);
		
		len -= tfr;
		iov->iov_base += tfr;
		iov->iov_len -= tfr;
		uio->uio_resid -= tfr;	
		uio->uio_offset += tfr;
		p += tfr;
		if( iov->iov_len <= 0 ) {
			uio->uio_iov = iov+1;
			uio->uio_iovcnt--;
		}
	}
	return 0;
}

/* Output routines */
int panic(char *s)
{
	IOdebug("UNIX panic: %s",s);
}

void log( int fd, char *fmt, ... )
{
	int *a = (int *)&fmt;
	a++;
	IOdebug(fmt,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]);
}

/*
 * A reduced version of printf to print I/O to the startup environment.
 */
extern Environ env;
extern char *shortprogname;
extern int debug2io;
void printf( char *fmt, ... )
{
	unsigned int *a = (unsigned int *)&fmt;
	static char format[512];
	static char buf[64];
	int i,j, fred;

	a++;
	strcpy(format,shortprogname);
	strcat(format,": ");
	i = strlen(format);

	/* Parse into fmt */
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
			case '%':
				format[i++] = '%';
				break;
			case 's':
				format[i] = '\0';
				strcat(format,*(a++));
				i = strlen(format);
				break;
			case 'c':
				format[i++] = *(a++);
				break;
			case 'd':
			case 'i':
			case 'u':
				fred = *a;
				if (fred == 0) {
					format[i++] = '0';
					a++;
					break;
				}
				if (fred < 0) {
					format[i++] = '-';
					fred = - (fred);
				}

				/* Create string backwards */
				j = 0;
				while (fred) {
					buf[j++] = '0' + (fred % 10);
					fred /= 10;
				}

				/* Move into position */
				while (j) {
					format[i++] = buf[--j];
				}

				a++;
				break;
			case 'x':
			case 'X':
				if (*a == 0) {
					format[i++] = '0';
					a++;
					break;
				}

				/* Create string backwards */
				j = 0;
				while (*a) {
					fred = *a % 16;
					if (fred < 10)
						buf[j++] = '0' + fred;
					else
						buf[j++] = 'a' + fred - 10;
					*a /= 16;
				}

				/* Move into position */
				while (j) {
					format[i++] = buf[--j];
				}

				a++;
				break;
			case 'o':
				if (*a == 0) {
					format[i++] = '0';
					a++;
					break;
				}

				/* Create string backwards */
				j = 0;
				while (*a) {
					buf[j++] = '0' + (*a % 8);
					*a /= 8;
				}

				/* Move into position */
				while (j) {
					format[i++] = buf[--j];
				}

				a++;
				break;
			case '\0':	
				goto done;
			default:
				break;
			}
			fmt++;
		} else {
			format[i++] = *(fmt++);
		}
	}
done:
	format[i] = '\0';

	switch (debug2io) {
		case 0:
			Write(env.Strv[1],format,i,-1);
			break;
		case 1:
			strcat(format,"%");
			IOdebug(format);
			break;
		case 2:
		default:
			/* No output at all */
			break;
	}
}

/* Wake up the specified process */
unsleep(p)
struct proc *p;
{
	panic("FATAL: 'unsleep' called");
	IOdebug("unsleep called by %s",myprocname(returnlink_(p)));
}

/* Send the specified signal to the specified process */
void psignal(p,sig)
struct proc *p;
int sig;
{
	panic("FATAL: 'psignal' called by %s");
	IOdebug("psignal called by %s",myprocname(returnlink_(p)));
}

/* Set the process to runable. */
void setrun (p)
struct proc *p;
{
	panic("FATAL: 'setrun' called");
	IOdebug("setrun called by %s",myprocname(returnlink_(p)));
}

/* Helios has no vm */
vnode_pager_uncache(struct vnode *vp)
{ return(TRUE); }

vnode_pager_setsize(struct vnode *vp, u_long size)
{ return(0); }

vnode_pager_umount(struct mount *mp)
{ return(0); }


/************************************************************************
 *             HELIOS GUARDS TO/FROM THE UNIX KERNEL                    *
 ************************************************************************/

#ifdef DIAGNOSTIC
char *kernellname[2];
int *kernelstack;
void tokernel(char *dummy) {
	int i;
	
	Wait(&kernel);
	if (TestSemaphore(&kernel) > 0)
		IOdebug("kernel %s:%s got through when already in use by %s:%s",myprocname(returnlink_(dummy)),dummy,kernellname[0],kernellname[1]);
	kernellname[0] = myprocname(returnlink_(dummy));
	kernellname[1] = dummy;
	kernelstack = &i;
}
#else
void tokernel(void) {
	Wait(&kernel);
}
#endif

#ifdef DIAGNOSTIC
void fromkernel(char *str) {
	if (TestSemaphore(&kernel) > 0)
		IOdebug("fromkernel called by %s (%s) without tokernel being called",myprocname(returnlink_(str)),str);
	Signal(&kernel);
	kernellname[0] = NULL;
	kernellname[1] = NULL;
	kernelstack = NULL;
}
#else
void fromkernel(void) {
	Signal(&kernel);
}
#endif

/************************************************************************
 *                  UNIX SLEEP/WAKEUP IMPLEMENTATION                    *
 ************************************************************************/

extern int hz;
#define SleepHz 10
extern struct timeval time;

List sleepq;
static int Now;
static int CurTime = 1;

void init_timer(void);

#ifdef MONITOR
Semaphore qlok;
#endif

struct zombie
{
	Node		node;
	struct proc	*p;
	caddr_t		chan;
	int		pri;
	word		wakemeat;
	bool		timeout;
	Semaphore	wait;
#ifdef MONITOR
	char		*pname;
#endif
};

void sleep(caddr_t chan, unsigned int pri)
{
	struct zombie s;
#ifdef ALEX
	IOdebug("sleep called on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif
#ifdef SHOWCALLS
	IOdebug("sleep called on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif

	/* Initialise */
	s.chan = chan;
	s.pri = pri;
	s.p = curproc;
	s.timeout = FALSE;
	s.wakemeat = 0;		/* Never timeout (for the moment anyway) */
	InitSemaphore(&s.wait,0);
#ifdef MONITOR
	s.pname = myprocname(returnlink_(chan));
#endif

	/* Stick me on the list of sleepers, at the back (not timeouts) */
#ifdef MONITOR
	Wait(&qlok);
#endif
	AddTail(&sleepq,&s.node);
#ifdef MONITOR
	Signal(&qlok);
#endif

	/* Release kernel and go to sleep */
#ifdef DIAGNOSTIC
	fromkernel(myprocname(returnlink_(chan)));
#else
	fromkernel();
#endif

	/* Sleep until wakeup issued */
	Wait(&s.wait);

	/* Yawn, woken up so lock the kernel */
#ifdef DIAGNOSTIC
	tokernel(myprocname(returnlink_(chan)));
#else
	tokernel();
#endif
	curproc = s.p;			/* Restore process info */

#ifdef SHOWCALLS
	IOdebug("sleep woken up on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif
#ifdef ALEX
	IOdebug("sleep woken up on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif
}

int tsleepinsert(struct zombie *next, struct zombie *s)
{
	if((next->wakemeat == 0) || (next->wakemeat > s->wakemeat))
	{
		PreInsert(&next->node,&s->node);
		return TRUE;
	}
	return FALSE;
}

int tsleep(caddr_t chan, int pri, char *msg, int timeout)
{
	struct zombie s;
	int error = 0;

#ifdef SHOWCALLS
	IOdebug("tsleep called on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif

	/* Initialise */
	s.chan = chan;
	s.pri = pri;
	s.p = curproc;
	s.timeout = FALSE;
	InitSemaphore(&s.wait,0);
	if((pri < PZERO) || (timeout == 0)) s.wakemeat = 0;
	else s.wakemeat = CurTime+timeout*(hz/SleepHz);
	
	/* Stick me on the list of sleepers, at the front for timeouts */
	if (s.wakemeat) 
	{	/* This should always insert if the queue is not empty */
		if (SearchList(&sleepq,(WordFnPtr)tsleepinsert,&s) == NULL)
			AddHead(&sleepq,&s.node); /* Empty, add it */
	}
	else AddTail(&sleepq,&s.node);

	/* Release kernel and go to sleep */
#ifdef DIAGNOSTIC
	fromkernel(myprocname(returnlink_(chan)));
#else
	fromkernel();
#endif

	/* Sleep unitl wakeup or timeout */
	Wait(&s.wait);

	/* Yawn, woken up so lock the kernel */
#ifdef DIAGNOSTIC
	tokernel(myprocname(returnlink_(chan)));
#else
	tokernel();
#endif
	curproc = s.p;	/* Restore process info */

	/* If woken up by a timout, return the corresponding error */
	if ( s.timeout ) {
		IOdebug("tsleep timeout: %s",msg);
		error = ETIMEDOUT;  /* Yuc, fix later -AMS */
	}

#ifdef SHOWCALLS
	IOdebug("tsleep called on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif

	return(error);
}

static int do_wakeup(struct zombie *s, caddr_t chan)
{
	if( s->chan == chan )
	{
		Remove(&s->node);	/* Throw him off the list */
		Signal(&s->wait);	/* Wake him up	*/
	}
	return 0;
}

void wakeup(caddr_t chan)	/* Run down list waking up */
{					/* where necessary. */
#ifdef ALEX
	IOdebug("wakeup called on 0x%x by '%s'",chan,myprocname(returnlink_(chan)));
#endif
#ifdef MONITOR
	Wait(&qlok);
#endif
	WalkList(&sleepq,(WordFnPtr)do_wakeup,chan);
#ifdef MONITOR
	Signal(&qlok);
#endif
}

#ifdef MONITOR
static int do_showwaiting(struct zombie *s)
{
	IOdebug("process %s waiting on 0x%x (stack index 0x%x)",s->pname,s->chan,s);
}
#endif

/* 
 * This checks the sleep list, waking up the processes which have timed
 * out.
 */
void sleep_timer(void) 
{	struct zombie *z;
#ifdef MONITOR
	struct buf *buf;
	struct timeval mytime;
	mytime.tv_sec = 0;
#endif		
	for (;;) {
		Delay(OneSec/SleepHz);
		time.tv_sec = GetDate();
		CurTime++;
		
#ifdef MONITOR
		if ((time.tv_sec - mytime.tv_sec) > 60) {
extern Semaphore hd_numbufs;
			IOdebug("sleep_timer: minute check");
			mytime.tv_sec = GetDate();
			Wait(&qlok);
			if (!EmptyList_(sleepq))
				WalkList(&sleepq,do_showwaiting);
			Signal(&qlok);
			IOdebug("%d disk requests",TestSemaphore(&hd_numbufs));
		}
		while (!TestWait(&kernel)) {
extern Semaphore hd_numbufs;
			if (GetDate() - mytime.tv_sec > 5) {
#ifdef DIAGNOSTIC
				IOdebug("sleep_timer: lockout check; locked by '%s|%s'  stack=0x%x",kernellname[0],kernellname[1],kernelstack);
#else
				IOdebug("sleep_timer: lockout check; locked");
#endif
				mytime.tv_sec = GetDate();
				Wait(&qlok);
				if (!EmptyList_(sleepq))
					WalkList(&sleepq,do_showwaiting);
				Signal(&qlok);
					
				IOdebug("%d disk requests",TestSemaphore(&hd_numbufs));
			}
		}
#else 
#ifdef DIAGNOSTIC
		tokernel("sleep timer monitor");
#else
		tokernel();
#endif
#endif
		/* Walk down, waking up as we go (until no more wakeups). */
		if( !EmptyList_(sleepq) )
		{
			z = Head_(struct zombie,sleepq);
		
			while((z != NULL) && (z->wakemeat) && (z->wakemeat <= CurTime ))
			{	/* Throw him off and wake him up */
				Remove(&z->node);	
				Signal(&z->wait);	

				/* Next Zombie to wakeup ? */
				z = Head_(struct zombie,sleepq);
			}
		}
#ifdef DIAGNOSTIC
		Signal(&kernel);
#else
		fromkernel();
#endif
	}
}

/*
 * This process sets up the helios variables used by the unix emulation
 * routines above.
 */
extern struct filedesc0 filedesc0;
extern struct pcred cred0;
extern struct vnode *vfreeh, **vfreet;
extern long numvnodes;
extern void KMeminit(void);
extern void bufinit();

int init_unix(void) 
{	struct filedesc0 *fdp;
	int i;

	/* Intialise memory routines. */
	KMeminit();

	/* Intialise the buffer space (nbuf must be setup in conf.c) */
	buffers = (char *)malloc(nbuf*sizeof(struct buf),M_MBUF,M_WAITOK);
	buf = (struct buf *)buffers;
	bufinit();

	/* set up system process 0, its credentials and file descriptor table */
	curproc = &proc0;
	cred0.p_refcnt = 1;
	proc0.p_cred = &cred0;
	proc0.p_ucred = crget();
	proc0.p_ucred->cr_ngroups = 1;		/* group 0 */
	proc0.p_flag = SLOAD;			/* in core */
	proc0.p_pid = 0;			/* Process 0 */
	fdp = &filedesc0;
	proc0.p_fd = &fdp->fd_fd;
	fdp->fd_fd.fd_refcnt = 1;
	fdp->fd_fd.fd_cmask = CMASK;
	fdp->fd_fd.fd_ofiles = fdp->fd_dfiles;
	fdp->fd_fd.fd_ofileflags = fdp->fd_dfileflags;
	fdp->fd_fd.fd_nfiles = NDFILE;

	/* Initial limits for proc0 */
	proc0.p_limit = &limit0;
	for (i = 0; i < sizeof(proc0.p_rlimit)/sizeof(proc0.p_rlimit[0]); i++)
		limit0.pl_rlimit[i].rlim_cur =
		    limit0.pl_rlimit[i].rlim_max = RLIM_INFINITY;
	limit0.pl_rlimit[RLIMIT_OFILE].rlim_cur = NOFILE;
	limit0.pl_rlimit[RLIMIT_NPROC].rlim_cur = MAXUPRC;
	limit0.p_refcnt = 1;

	/* Setup the time */
	time.tv_sec = GetDate();
	time.tv_usec = 0;
	
	/* The vnodes list */
	vfreeh = NULL;
	vfreet = &vfreeh;
	numvnodes = 0;
	
	/* The queues and kernel variables */
	InitList(&sleepq);
	InitSemaphore(&kernel,1);
#ifdef MONITOR
	InitSemaphore(&qlok,1);
#endif

	return(!Fork(5000,sleep_timer,0));	
}


/************************************************************************
 *                            HELIOS ROUTINES                           *
 ************************************************************************/

int memcmp(const void *a, const void *b, size_t n)
{   const unsigned char *ac = a, *bc = b;
    while (n-- > 0)
    {   unsigned char c1,c2;   /* unsigned cmp seems more intuitive */
        if ((c1 = *ac++) != (c2 = *bc++)) return c1 - c2;
    }
    return 0;
}

/* Code to return procedure name (for debugging) */
int myprocname(void (*fn)())
{
	word *x = (word *)fn;
	
	if( (int *)fn == NULL ) return ((int)"<NULL fn>");
	
	x = (word *) (((word) x) & ~3);
	
	while( (*x & T_Mask) != T_Valid ) x--;
	
	switch( *x )
	{
	case T_Proc:
		return (int)((Proc *)x)->Name;
	case T_Module:
	case T_Program:
		return (int)((Module *)x)->Name;
	}
}

