/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- exec.c								--
--                                                                      --
--	Exec et al.							--
--                                                                      --
--	Author:  NHG 8/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: exec.c,v 1.16 1993/08/11 13:42:17 bart Exp $ */


#include <helios.h>	/* standard header */

#define __in_exec 1	/* flag that we are in this module */

#include <stdarg.h>

#include <posix.h>

#include "pposix.h"
#include <codes.h>
#include <process.h>

#define _Trace(a,b,c)

STATIC Semaphore forklock;
static fdentry *pfdvec;
STATIC PCB *childvec;
STATIC int inchild;
static int nextpid;
STATIC Port WaitPort;
STATIC int starttime;
STATIC int childtimes;
STATIC int childvecsize;

static char *pathbuf = NULL;

STATIC void (*_exitvector[EXIT_LIMIT])(void);
STATIC int number_of_exit_functions;
STATIC Environ MyEnv;

static void waiter(PCB *pcb);
static void int2hex(int n, char *s);
static int hex2int(char *s);

extern char **_posix_init( void )
{
	char **argv;
	Port myport;
	char *ids;
	sigset_t ignore = 0;
	sigset_t mask = 0;
	Capability cap;
			
	starttime  = (int) _cputime();
	childtimes = 0;

	InitSemaphore(&forklock,1);
	PausePort = NullPort;
	WaitPort = NewPort();
	
	childvec     = (PCB *)Malloc(childvecinc*sizeof(PCB));
	childvecsize = childvecinc;
	memset(childvec,0,childvecinc*sizeof(PCB));
	
	nextpid = 43;
	inchild = 0;

	childvec[0].pid = 42;	
	childvec[0].pgrp = 42;	
	
	TaskData(TD_Port, &myport);	
	GetEnv(myport, &MyEnv);
	
	argv = MyEnv.Argv;
	environ = MyEnv.Envv;

	UID = -1; GID = -1;
	if((ids = getenv("_UID")) != NULL ) UID = hex2int(ids);
	if((ids = getenv("_GID")) != NULL ) GID = hex2int(ids);
	if((ids = getenv("_SIGMASK")) != NULL ) mask = hex2int(ids);
	if((ids = getenv("_SIGIGNORE")) != NULL ) ignore = hex2int(ids);

	/* install default capability				*/
	((word *)&cap)[0] = -1;
	((word *)&cap)[1] = UID | ((long)GID<<16);
	DefaultCapability(&cap,NULL);
		
	init_signal(mask, ignore);
	
	init_fileio(&MyEnv);

	/* if the CDL environment variable is set, try TFM in execve */
	if( getenv("CDL") != NULL ) pflagword |= PE_RemExecute;

	return argv;	
}

static int hex2int(char *s)
{
	int n = 0;
	int i;
	for( i = 0; i < 8; i++ )
	{
		char c = s[i];
		if  ( '0' <= c && c <= '9' ) n = (n<<4) + c - '0';
		elif( 'a' <= c && c <= 'f' ) n = (n<<4) + c - 'a' + 10;
		else 			     n = (n<<4) + c - 'A' + 10;
	}
	return n;
}

static void int2hex(int n, char *s)
{
	int i;
	for( i = 7; i >= 0; i-- ) 
	{
		int d = n&0xf;
		if( d > 9 ) s[i] = 'A' + d - 10;
		else s[i] = '0' + d;
		n >>= 4;
	}
}

static int vfork_start(void)
{
	int i;
	int pcb = 0;

	for( i = 1; i < childvecsize ; i++ ) 
		if( childvec[i].pid == 0 ) { pcb = i; break; }
	
	/* if the childvec is full, re-allocate it */
	if( i == childvecsize )
	{
		PCB *oldvec = childvec;
		PCB *newvec = (PCB *)Malloc(((word)childvecsize+childvecinc)*sizeof(PCB));
		
		if( newvec != NULL )
		{
			memset(newvec,0,(childvecsize+childvecinc)*sizeof(PCB));
			memcpy(newvec,oldvec,childvecsize*sizeof(PCB));
			childvec = newvec;
			Free(oldvec);
			pcb = childvecsize;
			childvecsize += childvecinc;
		}
	}

	if( pcb == 0 )
	{
		Signal(&forklock);
		errno = EAGAIN; return -1;
	}

	/* save fdvec and increment refs on open files */
	
	if( (pfdvec = savefdv()) == NULL )
	{
		Signal(&forklock);
		errno = ENOMEM; return -1;
	}

	inchild = pcb;

	savesigmasks();

	childvec[pcb].status    = 0;
	childvec[pcb].pid       = nextpid++;
	childvec[pcb].pgrp      = childvec[0].pgrp;
	childvec[pcb].starttime = (int)_cputime();
	childvec[pcb].prog      = NULL;	
	childvec[pcb].stream    = NULL;
	
	return 0;
}

static int posix_exit(int status)
{
	int pid;

	_Trace(0xCCCC000F,MyTask,status);
	
	/* check for parent termination, if so, generate exit code */
	
	if( inchild == 0 )
	{
		Exit(((word)status&0xff)<<8);
	}

	/* else this is the termination of a vforked child, 		*/
	/* status will either be the failure code, or MinInt if the	*/
	/* child was successfully execed.				*/

	pid = childvec[inchild].pid;

	childvec[inchild].status = (int)(status==MinInt?MinInt:(((word)status&0xff)<<8));

	restorefdv(pfdvec);
	
	pfdvec = NULL;	
	
	inchild	= 0;

	restoresigmasks();	/* restore signal masks		*/

	Signal(&forklock);	/* allow more vforks		*/

	CHECKSIGS();		/* raise any pending signals	*/
	
	return pid;		/* return child pid to parent 	*/
}

static int _sigexit(int sig)
{
_Trace(0xCCCC000E,MyTask,sig);
#ifndef __TRAN
	/* make sure that exit code will not loop at a higher priority */
	/* waiting for lower priority code to abort - because it can't! */
	SetPriority(StandardPri);
#endif
	Exit(sig);
return 0;
}

extern int execl(char *path,...)
{
  char **a0 = (&path)+1;	/* XXX - BEWARE of assumption of descending stack !!! */
	
  return execve(path,a0,environ);
}

extern int execv(char *path,char **argv)
{
	return execve(path,argv,environ);
}

extern int execle(char *path,...)
{
	char **a0;
	char ***env;
	
	a0 = (&path)+1;
	env = (char ***)a0;
	
	while( *env++ );

	return execve(path,a0,*env);
}

extern int execlp(char *file,...)
{
	int r;
	char **a0;
	
	pathbuf = (char *)Malloc(PATH_MAX+1);
	
	if( pathbuf == NULL ) { errno = ENOMEM; return -1; }
	
	find_file(pathbuf,file);
	
	a0 = (&file)+1;
	
	r = execve(pathbuf,a0,environ);	
	
	return r;
}

extern int execvp(char *file, char **argv)
{
	int r;

	pathbuf = (char *)Malloc(PATH_MAX+1);
	
	if( pathbuf == NULL ) { errno = ENOMEM; return -1; }
	
	find_file(pathbuf,file);

	r = execve(pathbuf,argv,environ);	
	
	return r;
}

extern int execve(char *name, char **argv, char **envv)
{
	Object *  tfm    = NULL;
	Object *  source = NULL;
	Object *  code   = NULL;
	Object *  prog   = NULL;
	Stream *  stream = NULL;
	Environ * env    = (Environ *)Malloc(sizeof(Environ));
	Stream ** strv   = NULL;
	Object ** objv   = (Object **)Malloc(sizeof(Object *)*(OV_End+1));
	word      e = 0;
	char *    dummy = NULL;
	char	  uidbuf[14];	/* _UID=01234567 */
	char	  gidbuf[14];	/* _GID=01234567 */
	bool	  got_newenv	= FALSE;

	if( env == NULL || objv == NULL )
	{ e = ENOMEM; goto fail; }
	
	if ((source = Locate(cdobj(), name)) == NULL)
	{ e = posix_error(Result2(cdobj())); goto fail; }

	code = source;
	
	/* Check the POSIX extension flag for Remote_Execute */
	if( (pflagword & PE_RemExecute) == 0 || 
	    (tfm = UserTFM) == (Object *)MinInt 
	  )
	{
		if((code = Load(NULL,source)) == NULL)
		{ e = posix_error(Result2(source)); goto fail; }
		
		tfm = Locate(cdobj(),"/tasks");
	}

	if ( (prog = Execute(tfm, code) ) == NULL )
	{ e = posix_error(Result2(code));goto fail; }

	if ((stream = Open(prog, NULL, O_ReadWrite)) == NULL)
	{ e = posix_error(Result2(prog));goto fail; }

	objv[OV_Cdir]	= cdobj();		/* current directory		*/
	objv[OV_Task] 	= prog;			/* this program's /tasks entry	*/
	objv[OV_Code] 	= code;			/* its /loader entry		*/
	objv[OV_Source] = source;		/* original source file		*/
	objv[OV_Parent] = ThisTask;		/* pass my /tasks entry as parent */
	
	objv[OV_Home] 	= Home;			/* user's home directory	*/
	objv[OV_Console]= Console;		/* control console		*/
	objv[OV_CServer]= CServer;		/* console server		*/
	objv[OV_Session]= Session;		/* my session manager entry	*/
	objv[OV_TFM]	= UserTFM;		/* user's TFM			*/

	objv[OV_TForce]	= prog;			/* TFM entry for task force	*/
	objv[OV_End]	= NULL;

	if( (strv = marshalfdv()) == NULL ) {e = ENOMEM; goto fail; }

	if (envv == NULL) envv = MyEnv.Envv;;

		/* BLV - if an application uses a private set of environment	*/
		/* strings then it is necessary to make a copy of these and	*/
		/* add a further two for _UID and _GID, or there are very	*/
		/* strange effects on protection.				*/
		/* BLV - even worse, some programs change the environ variable !*/
	if (envv != MyEnv.Envv)
	 { int	count, i;
	   char	**newenv;

	   for (count = 0; envv[count] != NULL; count++);
	   newenv = (char **)Malloc(((long)count + 3) * sizeof(char *));
	   if (newenv == NULL)
	    { e = ENOMEM; goto fail; }
	   for (i = 0; i < count; i++)
	    newenv[i] = envv[i];
	   newenv[i] = NULL;

	   envv = environ;		/* Save the old environment */
	   environ = newenv;
	   if (getenv("_UID") == NULL)	/* Check that _UID is not already defined */
	    { strcpy(uidbuf, "_UID=00000000");
 	      newenv[i++] = uidbuf;
	      newenv[i]   = NULL;
	    }
	   if (getenv("_GID") == NULL)
	    { strcpy(gidbuf, "_GID=00000000");
	      newenv[i++] = gidbuf;
	      newenv[i]   = NULL;
	    }
	   environ = envv;	/* restore old environment		*/
	   envv	   = newenv;	/* and use the newly created environment*/
	   got_newenv = TRUE;
	 }

		/* Try to stop a hacker from overwriting the _UID and	*/
		/* _GID strings in the environment by resetting these 	*/
		/* using private variables.				*/
	{ char	**saveenv;
	  char	 *ids;

	  saveenv = environ;		/* so that getenv() can be used */
	  environ = envv;		/* possibly a no-op		*/

	  if((ids = getenv("_UID")) != NULL ) int2hex(UID,ids);
	  if((ids = getenv("_GID")) != NULL ) int2hex(GID,ids);
	  if((ids = getenv("_SIGMASK")) != NULL ) int2hex((int)mask.mask,ids);
	  if((ids = getenv("_SIGIGNORE")) != NULL ) int2hex((int)mask.ignore,ids);

	  environ = saveenv;
	}
	
	env->Argv = argv==NULL?&dummy:argv;
	env->Envv = envv==NULL?&dummy:envv;
	env->Objv = objv;
	env->Strv = strv;

	if( (e=SendEnv(stream->Server, env)) < 0 )
	{ e = posix_error(e); goto fail; }

	if( tfm != NULL && tfm != UserTFM ) Close(tfm);
	if( source != NULL && source != code ) Close(source);
	Close(code);
	if (got_newenv) Free(envv);
	Free(env);
	Free(strv);
	Free(objv);
	if( pathbuf != NULL ) Free(pathbuf),pathbuf=NULL;

	if( inchild == 0 )
	{
		Close(prog);		/* replace current */
		Close(stream);
		_exit(0);
	}
	else
	{
		childvec[inchild].prog = prog;	
		childvec[inchild].stream = stream;
		Fork(WaiterStack, waiter,4,&(childvec[inchild]));
		_exit((int)MinInt);
	}

fail:
	if( tfm != NULL && tfm != UserTFM ) Close(tfm);
	if( source != NULL && source != code ) Close(source);
	if( code != NULL ) Close(code);
	if( prog != NULL ) Close(prog);
	if( stream != NULL ) Close(stream);
	if( env != NULL ) Free(env);
	if( strv != NULL ) Free(strv);
	if( objv != NULL ) Free(objv);
	if( pathbuf != NULL ) Free(pathbuf),pathbuf=NULL;
	if (got_newenv) Free(envv);
	errno = (int)e;
	return -1;
}

static void waiter(PCB *pcb)
{
	extern void inner_raise(int sig, bool async, bool propagate);
	word svsize = InitProgramInfo(pcb->stream,PS_Terminate);
	word *sv    = (word *)Malloc(svsize * sizeof(word));
	word status;
	
_Trace(0xAAAA000a,MyTask,pcb->pid);	
	status = GetProgramInfo(pcb->stream,sv,-1);
_Trace(0xAAAA000b,MyTask,status);	
	Wait( &forklock );
_Trace(0xAAAA000c,MyTask,status);	
	pcb->status = (int)status;

	AbortPort(WaitPort,0);
_Trace(0xAAAA000d,MyTask,status);	
	Signal( &forklock );

	inner_raise(SIGCHLD,TRUE,FALSE);
	Free(sv);
_Trace(0xAAAA000e,MyTask,status);
}

extern pid_t wait(int *stat)
{
	return waitpid(-1,stat,0);
}

extern pid_t wait2(int *stat, int options)
{
	return waitpid(-1,stat,options);
}

extern pid_t wait3(int *stat, int options,void *rusage)
{
	return waitpid(-1,stat,options);
}

extern pid_t waitpid(pid_t wpid, int *stat,int options)
{
	int status = 0;
	int pcb;
	int pid = 0;
	pid_t pgrp = 0;
	MCB m;
_Trace(0xAAAA0001,MyTask,wpid);
	CHECKSIGS();
	Wait(&forklock);	/* ensure no forks happen during this */
_Trace(0xAAAA0005,MyTask,wpid);	

	if( wpid == 0 ) pgrp = childvec[inchild].pgrp;
	elif( wpid < -1 ) pgrp = -wpid;
	
	/* loop here forever until a child, if any, terminates	*/
	forever
	{
		int i;
		word e;
		int any = 0;
		pcb = 0;

		for( i = 1; i < childvecsize; i++ )
			if( (childvec[i].pid != 0)		&&
			    ((wpid == -1) 			||
			     (childvec[i].pid == wpid)		||
			     (childvec[i].pgrp == pgrp)
			    )
			  ) 
			{
				any++;
				if( (childvec[i].status != MinInt) )
					{ pcb = i; break; }
			}

		if( any == 0 ) { errno = ECHILD, pid = -1; goto done; }

		if( pcb != 0 ) break;
		
		if( (options & WNOHANG) != 0 ) goto done;
		
		InitMCB(&m,0,WaitPort,NullPort,0);
_Trace(0xAAAA0006,MyTask,wpid);
		/* release the lock while we wait for a child to finish	*/
		Signal( &forklock );
		e = GetMsg(&m);
		Wait( &forklock );		
_Trace(0xAAAA0004,MyTask,e);
	
		if( e == 123 ) { errno = EINTR; pid = -1; goto done; }
		
		if( e == EK_Timeout ) continue;
		
		FreePort(WaitPort);
		WaitPort = NewPort();
	}

	pid = childvec[pcb].pid;

	status = childvec[pcb].status;

	if( childvec[pcb].prog != NULL ) Close(childvec[pcb].prog);
	if( childvec[pcb].stream != NULL ) Close(childvec[pcb].stream);

	childtimes += (int)(_cputime() - childvec[ pcb ].starttime);

	childvec[pcb].pid = 0;
		
	if( stat != NULL ) *stat = status;

done:
	Signal(&forklock);
_Trace(0xAAAA0002,MyTask,pid|(errno<<16));	
	CHECKSIGS();
_Trace(0xAAAA0003,MyTask,pid|(errno<<16));
	return pid;
}

extern void find_file(char *path, char *file )
{
	char *env;
	char *f = file;
	
	while( *f ) 
		if( *f++ == '/' ) 
		{
			strcpy(path,file);
			return;
		}
	
	env = getenv("PATH");

	if( env != NULL ) 
	{
		while( *env != '\0' )
		{
			Object *o;
			char *p = path;
			char *f = file;
			while( *env != ':' && *env != '\0' ) *p++ = *env++;
			*p++ = '/';

			while( (*p++ = *f++) != '\0' );

			if( (o=Locate(cdobj(),path)) != NULL )
			{
				Close(o);
				return;
			}
			if( *env == ':' ) env++;
		}
	}

	strcpy(path,file);
	
	return;
}

/* This is not really part of Posix, but no self-respecting Unix look-	*/
/* alike should be without it.						*/

extern int system(const char *command)
{
	int stat;
	CHECKSIGS();
	if( vfork()==0 )
	{
	/* MJT 25/7/91 */

	/* close down streams with fd > 3 so that open pipes do not end */
	/* up with 4 ends !						*/

		int fd, maxfd = getdtablesize();

		for(fd = 4 ; fd < maxfd ; fd++)
			close(fd);

		execl("/helios/bin/shell", "shell", "-fc", command, NULL );
		_exit(20);
	}
	wait(&stat);
	CHECKSIGS();
	return stat;
}

/* It is very difficult to tell from the Posix spec whether these 	*/
/* functions are part of Posix or not. We have put them here because	*/
/* this is the most sensible place for them...				*/

extern int atexit(void (*func)(void))
{ 
	if (number_of_exit_functions >= EXIT_LIMIT) return 1;    /* failure */
		_exitvector[number_of_exit_functions++] = func;
	return 0;                                                /* success */
}

extern void exit(int n)
{
	while (number_of_exit_functions!=0)
		(*_exitvector[--number_of_exit_functions])();
	_exit(n);
}

extern void abort()
{
	raise(SIGABRT);
	_sigexit(0x80+SIGABRT);
}

extern Environ *getenviron(void)
{
	return &MyEnv;
}

/* end of exec.c */
