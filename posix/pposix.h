
/* pposix.h : private header for posix library implementation	*/

/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: pposix.h,v 1.6 1993/08/11 13:42:36 bart Exp $ */

#ifndef __pposix_h
#define __pposix_h

#include <sys/types.h>

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>
#include <dirent.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <sys/wait.h>

#include <syslib.h>

typedef struct Pstream
  {
    word	type;
    word	refs;
    Stream *	stream;
  }
Pstream;

typedef struct fdentry
  {
    Pstream *	pstream;
    word	flags;
  }
fdentry;

typedef struct pcb
  {
    int		status;
    pid_t	pid;
    pid_t	pgrp;
    int		starttime;
    Object *	prog;
    Stream *	stream;
    int		spare;
  }
PCB;	

#define fdvecinc	20
#define childvecinc	20


#define EXIT_LIMIT 33

#define STATIC

#ifndef __in_fileio
extern int posix_error(word e);
extern void freefd(int fd);
extern fdentry *checkfd(int fd);
extern int findfd(int low);
#endif

struct sigmask
{
	sigset_t	pending;	/* set of pending signals */
	sigset_t	mask;		/* set of masked signals  */
	sigset_t	ignore;		/* signals to be ignored  */
	sigset_t	async;		/* signals which may be delivered asyncronously */
	sigset_t	propagate;	/* signals to pass on to children */	
};

#ifndef __in_signal
extern struct sigmask mask;
extern void raise_sync(void);
#define CHECKSIGS()	if(mask.pending) raise_sync()
#endif

#ifndef __in_exec
extern Environ MyEnv;
#endif

extern char **environ;

#define ThisTask	(MyEnv.Objv[OV_Task])
#define TaskCode	(MyEnv.Objv[OV_Code])
#define TaskSource	(MyEnv.Objv[OV_Source])
#define Parent		(MyEnv.Objv[OV_Parent])
#define Home		(MyEnv.Objv[OV_Home])
#define Console		(MyEnv.Objv[OV_Console])
#define CServer		(MyEnv.Objv[OV_CServer])
#define Session		(MyEnv.Objv[OV_Session])
#define UserTFM		(MyEnv.Objv[OV_TFM])
#define TaskForce	(MyEnv.Objv[OV_TForce])


#ifdef STACKEXTENSION
#define WaiterStack	 750
#else
#define WaiterStack	1000
#endif

#ifndef __in_signal
extern int	SysTimeout;
#endif

extern PCB *	childvec;
extern Port	PausePort;
extern Port	WaitPort;
extern gid_t	GID;
extern int	childtimes;
extern int	childvecsize;
extern int	inchild;
extern int	number_of_exit_functions;
extern int	pflagword;
extern int	starttime;
extern uid_t	UID;
extern void (*	_exitvector[])(void);
extern struct sigaction sigactions[];

#ifndef __in_fileio
extern Stream **marshalfdv(void);
extern fdentry *savefdv(void);
extern void	init_fileio(Environ *env);
extern void	abortfdv(void);
extern int	posix_error(word);
extern void	restorefdv(fdentry *sfdvec);
#endif

#ifndef __in_exec
extern void	_sigexit(int sig);
#endif

extern int	setuptimeout( void );
extern word	_cputime(void);
extern word	_ldtimer(word pri);
extern void	closedb(int mode);
extern int	getdtablesize(void);
extern int	opendb(char *name, int mode);
extern int	scandb(char *format, ... );
extern int	setuptimeout(void);
extern int	svopen(Stream *s, int fd);
extern void	resettimeout( void );
extern void	DefaultCapability( Capability *, void * );
extern void	find_file(char *path, char *file );
extern void	init_signal(sigset_t mask, sigset_t ignore);
extern void	resettimeout(void);
extern void	restoresigmasks(void);
extern void	savesigmasks(void);

#endif /* __pposix_h */
