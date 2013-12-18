/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- env.c								--
--                                                                      --
--	Process environment routines.					--
--                                                                      --
--	Author:  NHG 8/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 @(#)env.c	1.8	18/9/89 Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: env.c,v 1.13 1993/07/12 10:24:40 nickc Exp $ */

#include <helios.h>	/* standard header */

#define __in_env 1	/* flag that we are in this module */

#include <string.h>

#include "pposix.h"

#include <root.h>		/* for gettimeofday */
#include <sys/time.h>
#include <sys/socket.h>		/* for gethostname() */

STATIC char nodename[50];
STATIC int pflagword = 0;

STATIC uid_t	UID;
STATIC gid_t	GID;


extern int getpid(void) { return childvec[inchild].pid; }
extern int getppid(void) { return inchild==0 ? childvec[0].pid-1 : childvec[0].pid; }

extern uid_t getuid(void) { return UID; }
extern uid_t geteuid(void) { return UID; }
extern uid_t getgid(void) { return GID; }
extern uid_t getegid(void) { return GID; }

extern void DefaultCapability( Capability *, void * );
  
extern int setuid(uid_t uid) 
{
	Capability cap;
	UID = uid;
	/* install default capability				*/
	((word *)&cap)[0] = -1;
	((word *)&cap)[1] = (word)UID | ((word)GID<<16);
	DefaultCapability(&cap,NULL);	
	return 0;
}

extern int setgid(uid_t gid) 
{
	Capability cap;	
	GID = gid;
	/* install default capability				*/
	((word *)&cap)[0] = -1;
	((word *)&cap)[1] = (word)UID | ((word)GID<<16);
	DefaultCapability(&cap,NULL);
	return 0;
}

extern int getgroups(int setsize, uid_t *list) { setsize=*list; return 0; }

extern char *getlogin(void)
{
	CHECKSIGS();
	return getenv("USER");
}

extern char *cuserid(char *s)
{
 	char *name = getlogin();
	if( name != NULL ) strcpy(s,name);
	CHECKSIGS();
	return s;
}

extern pid_t getpgrp(void) { return childvec[inchild].pgrp; }

extern pid_t setsid(void) 
{ 
	childvec[inchild].pgrp = childvec[inchild].pid;
	return 0; 
}

extern int setpgid(pid_t pid, pid_t pgid)
{
	int i;
	
	if( pid == 0 ) pid = childvec[inchild].pid;
	if( pgid == 0 ) pgid = pid;
	
	for(i = 0; i < childvecsize; i++ )
		if( childvec[i].pid == pid ) 
		{
			childvec[i].pgrp = pgid;
			return 0;
		}
		
	errno = EINVAL;
	return -1;
}

extern int uname(struct utsname *name)
{
	char *mcname;
	switch( MachineType() )
	{
#ifdef __TRAN
	case 800:	mcname = "T800"; break;
	case 414:	mcname = "T414"; break;
	case 425:	mcname = "T425"; break;
	default:	mcname = "TRAN"; break;
#endif
#ifdef __ARM
	case 0xa6:	mcname = "ARM6"; break;
	case 0xa3:	mcname = "ARM3"; break;
	default:	mcname = "ARM"; break;
#endif
#ifdef __C40
	case 0xc40:
	default:	mcname = "TMS320C40"; break;
#endif
	}
	gethostname(name->nodename,32);
	strcpy(name->sysname,"Helios");
	strcpy(name->release,"1.3");
	strcpy(name->version,"1");
	strcpy(name->machine,mcname);
	CHECKSIGS();
	return 0;
}

extern time_t time(time_t *tloc)
{
	CHECKSIGS();
	if( tloc != NULL ) return (*tloc = GetDate());
	else return GetDate();
}

extern clock_t times(struct tms *t)
{
	if( t != NULL )
	{
		t->tms_utime  = (int)(_cputime() - starttime);
		t->tms_stime  = 0;
		t->tms_cutime = childtimes;
		t->tms_cstime = 0;
	}
	CHECKSIGS();
	return (clock_t) _cputime();
}

extern int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	RootStruct *root = GetRoot();
	word systime = 0;
	
	CHECKSIGS();
	
	if( tz != NULL )
	{
		tz->tz_minuteswest = 0;
		tz->tz_dsttime = 3;  /* western europe */
	}
	
	while( systime != root->Timer )
	{
		word ptime = _ldtimer(0);
		unsigned long diff;

		systime = root->Timer;
		
		diff = (unsigned long)ptime - (unsigned long)systime;
		
		tv->tv_sec = root->Time;
		tv->tv_usec = diff;
	}

	return 0; /* always succeeds */
}

extern char *getenv(const char *name)
{ 
	int namelen = strlen(name);
	char **currenv = environ;

	CHECKSIGS();
	while (*currenv != (char *) NULL)
	{ 
		if ( (strncmp(name, *currenv, namelen) == 0) &&
			((*currenv)[namelen] == '='))
		return(*currenv + namelen + 1);
		else currenv++;
	}

	return((char *) NULL);
}

static char ctermname[50];

extern char *ctermid(char *s)
{
	CHECKSIGS();

	if( s == NULL ) s = ctermname;

	if( Console != (Object *)MinInt ) 
	{
		DecodeCapability(s,&Console->Access);
		strcat(s,Console->Name);
	}
	else
	{
		strcpy(s,getenv("CONSOLE"));
		
	}
	
	return s;
}

extern char *ttyname(int fd)
{
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return NULL;
	
	return f->pstream->stream->Name;
}

extern int isatty(int fd)
{
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return NULL;
	
	return (int)(f->pstream->stream->Flags & Flags_Interactive);
}

extern int sysconf(int name)
{
	CHECKSIGS();
	name = name;
	switch( name )
	{
	case _SC_ARG_MAX:		return ARG_MAX;
	case _SC_CHILD_MAX:		return LONG_MAX;
	case _SC_CLK_TCK:		return CLK_TCK;
	case _SC_NGROUPS_MAX:		return NGROUPS_MAX;
	case _SC_OPEN_MAX:		return LONG_MAX;
	case _SC_JOB_CONTROL:		return -1;
	case _SC_SAVED_IDS:		return -1;
	case _SC_VERSION:		return (int)_POSIX_VERSION;
	}
	errno = EINVAL;
	return -1;
}




/* _posixflags() : This is a Helios extension to the POSIX library.   	    */
/*		Modifies the global posix flag word. This flag word defines */
/*		various extensions to POSIX e.g remote Execute etc.	    */


extern int _posixflags(int how, int set )
{
	CHECKSIGS();
	switch (how)
	{

	case (PE_BLOCK):
		{
		pflagword |= set;
		break;
		}
	case (PE_UNBLOCK):
		{
		pflagword &= (~set);
		break;
		}
	case (PE_SETMASK):
		{
		pflagword = set;
		break;
		}
	default:
		{
		return(-1);
		}
	}

	return(pflagword);
}

	
/* end of env.c */

