/* $Header: /usr/perihelion/Helios/cmds/emacs/RCS/dolock.c,v 1.1 90/08/23 15:13:49 jon Exp $ */

#if	0
/*	dolock:	MDBS specific Unix 4.2BSD file locking mechinism
		this is not to be distributed generally		*/

#include	<mdbs.h>
#include	<mdbsio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

/* included by port.h: mdbs.h, mdbsio.h, sys/types.h, sys/stat.h */


#ifndef bsdunix
char *dolock(){return(NULL);}
char *undolock(){return(NULL);}
#else

#include <pwd.h>
#include <errno.h>

extern int errno;

#define LOCKDIR ".xlk"

#define LOCKMSG "LOCK ERROR -- "
#define LOCKMSZ sizeof(LOCKMSG)
#define LOCKERR(s) { strcat(lmsg,s); oldumask = umask(oldumask); return(lmsg); }

/**********************
 *
 * dolock -- lock the file fname
 *
 * if successful, returns NULL 
 * if file locked, returns username of person locking the file
 * if other error, returns "LOCK ERROR: explanation"
 *
 * Jon Reid, 2/19/86
 *
 *********************/

BOOL parent = FALSE;
BOOL tellall = FALSE;

char *gtname(filespec)		/* get name component of unix-style filespec */
char *filespec;
{
	char *rname, *rindex();

	rname = rindex(filespec,'/');

	if (rname != NULL)
		return(rname);
	else
		return(filespec);
}

char *getpath(filespec)
char *filespec;
{
	char rbuff[LFILEN];
	char *rname, *rindex();

	strcpy(rbuff,filespec);
	rname = rindex(rbuff,'/');

	if (rname == NULL)
		return(NULL);
	else
	{
		*(++rname) = '\0';
		return(rbuff);
	}

}

char *dolock(fname)
	char *fname;
{
	static char lockname[LFILEN] = LOCKDIR;
	static char username[12];
	static char lmsg[40] = LOCKMSG;
	char *pathfmt;
	struct stat statblk;
	struct passwd *pblk;
	long pid, getpid();
	FILE *lf, *fopen();
	int oldumask;

	oldumask = umask(0);	/* maximum access allowed to lock files */


	  if (*fname != '/')
	   pathfmt = "./%s%s";
	  else
	   pathfmt = "%s/%s";
	  sprintf(lockname,pathfmt,getpath(fname), LOCKDIR);

	  if (tellall) printf("checking for existence of %s\n",lockname);

	  if (stat(lockname,&statblk))
	  {
		 if (tellall) printf("making directory %s\n",lockname);
		 mkdir(lockname,0777); 
	  }

	  sprintf(lockname,"%s/%s",lockname,gtname(fname));

	  if (tellall) printf("checking for existence of %s\n",lockname);

	  if (stat(lockname,&statblk))
	  {
makelock:  	if (tellall) printf("creating %s\n",lockname);

		if ((lf = fopen(lockname,FOP_TW)) == NULL)
		  LOCKERR("could not create lock file")
	        else
	  	{
			if (parent)
			 pid = getppid();	/* parent pid */
			else
			 pid = getpid();	/* current pid */

			 if (tellall)
			  printf("pid is %ld\n",pid); 

			 fprintf(lf,"%ld",pid); /* write pid to lock file */

			fclose(lf);
			oldumask = umask(oldumask);
			return(NULL);
		}
	  }
	  else
	  {
		if (tellall) printf("reading lock file %s\n",lockname);
		if ((lf = fopen(lockname,FOP_TR)) == NULL)
		  LOCKERR("could not read lock file")
	        else
	  	{
			fscanf(lf,"%ld",&pid); /* contains current pid */
			fclose(lf);
			if (tellall)
			 printf("pid in %s is %ld\n",lockname, pid);
			if (tellall)
			 printf("signaling process %ld\n", pid);
			if (kill(pid,0))
				switch (errno)
				{
				  case ESRCH:	/* process not found */
						goto makelock;
						break;
				  case EPERM:	/* process exists, not yours */
			 			if (tellall) 
						 puts("process exists");
						break;
				  default:
					LOCKERR("kill was bad")
					break;
				}
			else
			 if (tellall) puts("kill was good; process exists");
		}
		if ((pblk = getpwuid(statblk.st_uid)) == NULL)
		  sprintf(username,"uid %d",atoi(statblk.st_uid));
		else
		  strcpy(username,pblk->pw_name);

		oldumask = umask(oldumask);
		return(username);
	  }
}

/**********************
 *
 * undolock -- unlock the file fname
 *
 * if successful, returns NULL 
 * if other error, returns "LOCK ERROR: explanation"
 *
 * Jon Reid, 2/19/86
 *
 *********************/

char *undolock(fname)
	char *fname;
{
	static char lockname[LFILEN] = LOCKDIR;
	static char lmsg[40] = LOCKMSG;
	char *pathfmt;

	  if (*fname != '/')
	   pathfmt = "./%s%s";
	  else
	   pathfmt = "%s/%s";
	  sprintf(lockname,pathfmt,getpath(fname), LOCKDIR);

	  sprintf(lockname,"%s/%s",lockname,gtname(fname));

	  if (tellall) printf("attempting to unlink %s\n",lockname);

	  if (unlink(lockname))
	  { 
		strcat(lmsg,"could not remove lock file"); 
		return(lmsg); 
	  }
	  else
	  	  return(NULL);
}

#endif bsdunix

/******************
 * end dolock module
 *******************/

#else
dolhello()
{
}
#endif

