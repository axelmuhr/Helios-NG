

/* routines to fake UNIX kernel environment for TCP/IP code */

#include <sys/types.h>
#include <queue.h>
#include <sem.h>
#include "user.h"
#include "param.h"

void init_timer(void);

struct user u;
struct timeval SYSTIME;
int selwait;

Semaphore kernel;

List sleepq;

static int Now;

struct sleeper
{
	Node		node;
	caddr_t		chan;
	int		wakeup;
	Semaphore	sem;
};

extern int MemAlloced;
extern void *MMalloc(int size);
extern void FFree(void *v);

void init_unix()
{
	InitList(&sleepq);
	InitSemaphore(&kernel,1);
	
	init_timer();
}


int (splx)(int level) { return 0; }
int (splnet)(void) { return splx(0); }
int (splimp)(void) { return splx(0); }
int (suser)(void) { return 1;}

void panic(char *s)
{
	extern void clean_exit (void) ;
	IOdebug("UNIX panic: %s",s);
/*
-- crf: 17/08/92 - Bugs 956, 999, 1002
-- At this point, the server should attempt to terminate gracefully. 
-- clean_exit() calls CleanUp() and Exits.
*/
	clean_exit() ;
}

void log( int fd, char *fmt, ... )
{
	int *a = (int *)fmt;
	
	a++;
	
	IOdebug(fmt,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]);
}

void printf( char *fmt, ... )
{
	int *a = (int *)&fmt;
	static char format[512];
	
	strcpy(format,fmt);
	strcat(format,"%");

	a++;

	IOdebug(format,a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]);
}

extern void insque(struct qelem *elem, struct qelem *pred)
{
	elem->q_back         = pred;
	elem->q_forw         = pred->q_forw;
	pred->q_forw->q_back = elem;
	pred->q_forw         = elem;
}

extern void remque(struct qelem *elem)
{
	elem->q_back->q_forw = elem->q_forw;
	elem->q_forw->q_back = elem->q_back;
}
#if 0
struct node
{
	struct node *next;
	struct node *prev;
};

void insque(struct node *p, struct node *prev)
{
	p->prev = prev;
	p->next = prev->next;
	prev->next->prev = p;
	prev->next = p;
}

void remque(struct node *p)
{
	p->prev->next = p->next;
	p->next->prev = p->prev;
}
#endif
/* timer system */

struct zz
{
	Node		node;
	int		time;
	void		(* proc)();
	caddr_t		arg;
};

List timerq;
Semaphore timerlock;
int hz;

void timerproc(void);

void init_timer(void)
{
	Now = 0;
	hz = 100;
	InitList(&timerq);
	InitSemaphore(&timerlock,1);
	Fork(5000,timerproc,0);
}

word insertzz(struct zz *next, struct zz *z)
{
	if( next->time > z->time )
	{
#ifdef SYSDEB
	  z->node.Next = z->node.Prev = &z->node;
#endif
		PreInsert(&next->node,&z->node);
		return TRUE;
	}
	return FALSE;
}

void timeout(void (*proc)(), caddr_t arg, int time)
{
	struct zz *z;

	z = (struct zz *)MMalloc(sizeof(struct zz));
	if( z == 0 ) panic("No more memory!");

	z->proc = proc;
	z->arg = arg;
	
	Wait(&timerlock);

	z->time = Now+time;

	if( SearchList(&timerq,insertzz,z) == NULL )
	{
		AddTail(&timerq,&z->node);
	}
	Signal(&timerlock);
}

void timerproc(void)
{
	for(;;)
	{
		struct zz *z;

		Delay(OneSec/hz);
		
		Wait(&timerlock);
		
		Now++;

		if( !EmptyList_(timerq) )
		{
			z = Head_(struct zz,timerq);
		
			while( z->time <= Now )
			{
				struct zz *nz = Next_(struct zz,z);
				void (* proc)() = (*z->proc);
				caddr_t arg = z->arg;

				Remove(&z->node);
				FFree(z);

				Signal(&timerlock);
				tokernel();

				proc(arg);
				
				fromkernel();
				Wait(&timerlock);

				z = nz;
			}
		}
		Signal(&timerlock);

	}
}

void microtime(struct timeval *tv)
{
	tv->tv_usec = 0;
	tv->tv_sec = GetDate();
}

void ovbcopy(char *a, char *b, int size)
{
	while( size-- ) b[size] = a[size];
}

void (mtpr)(int a, int b) { /*IOdebug("mtpr: netisr %x",netisr);*/ }

int uiomove(caddr_t p, int len, int mode, struct uio *uio)
{
	while( len )
	{
		struct iovec *iov = uio->uio_iov;
		int tfr = len;
		
		if( tfr > iov->iov_len ) tfr = iov->iov_len;
		
		if( mode == UIO_READ ) 
			memcpy(iov->iov_base,p,tfr);
		else 	memcpy(p,iov->iov_base,tfr);
		
		len -= tfr;
		iov->iov_base += tfr;
		iov->iov_len -= tfr;
		uio->uio_resid -= tfr;	
		p += tfr;
		if( iov->iov_len == 0 ) uio->uio_iov = iov+1,uio->uio_iovcnt--;
	}
	return 0;
}

void (gsignal)(int grp, int sig) { return; }

/* memory stuff */

int MemAlloced = 0;

#ifndef MMalloc
void *MMalloc(int size)
{
	void *v = Malloc(size);
	if( v == NULL )
	{
		IOdebug("tcpip Malloc failure");
		return NULL;
	}
	size = ((int *)v)[-2];
	if( size < 0 ) size = -size;
	MemAlloced += size;
	return v;
}

void FFree(void *v)
{
	int *vv = (int *)v;
	int size = ((int *)v)[-2];
	if( size < 0 ) size = -size;
	MemAlloced -= size;
	Free(v);
}
#endif
