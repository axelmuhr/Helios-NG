/* $Id: signal.c,v 1.4 1993/07/09 12:59:02 nickc Exp $ */
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

typedef  void		(*VoidFnPtr)();
typedef  int		(*IntFnPtr)();

extern int errno;

extern int sigvec(int sig, struct sigvec *vec, struct sigvec *ovec)
{
	int e;
	struct sigaction New,old;
	
	if( vec )
	{
		New.sa_handler = (VoidFnPtr)(vec->sv_handler);
		New.sa_mask = vec->sv_mask;
		New.sa_flags = 0;
	}
	
	e = sigaction(sig,vec ? &New : NULL, ovec ? &old : NULL);
	
	if( e==0 && ovec )
	{
		ovec->sv_handler = (IntFnPtr)old.sa_handler;
		ovec->sv_mask = (int)old.sa_mask;
		ovec->sv_onstack = 0;
	}
	
	return e;
}

extern int sigblock(int mask)
{
	sigset_t set,oset;
	set = (sigset_t)mask;
	sigprocmask(SIG_BLOCK,&set,&oset);
	return (int)oset;
}

extern int sigsetmask(int mask)
{
	sigset_t set,oset;
	set = (sigset_t)mask;
	sigprocmask(SIG_SETMASK,&set,&oset);
	return (int)oset;
}

extern int sigpause(int mask)
{
	return sigsuspend((sigset_t *)&mask);
}

extern int sigstack(struct sigstack *ss,struct sigstack *oss)
{
	errno = EFAULT;
	return -1;
}
