/*	DOLOCK.C:	Machine specific code for File Locking
#include	"elang.h"
			for MicroEMACS
			(C)opyright 1987 by Daniel M Lawrence
*/

#include	"estruct.h"
#include	"etype.h"

#if	WMCS
/*	file locking for WMCS */

#include "sys$disk/sysincl.sys/sysequ.h"
#include <stdio.h>
#include <ctype.h>

char msg[] = TEXT35;
/*           "another user" */

char *dolock(fname)
char *fname;
{
	int lun,status;
	status = _open(fname,OPREADACC|OPWRITEACC|OPWRITELOCK,-1,&lun);
	if(status == 133 || status == 0 ) return(NULL);
	return(msg);
}

char *undolock(fname)
char *fname;
{
	int i,j,k,lun,status;
	char xname[95],c;
	
	for(lun=4; _getfnam(lun,xname) == 0; lun++) {
		for(i=0;i<strlen(xname);i++)	{
			k = i;
			for(j=0;j<strlen(fname);j++)  {
				c = fname[j];
				if(islower(c)) c = toupper(c);
				if(c == xname[k]) { ++k; continue; }
				if(c == '\0') break;
				break;
				}
			if(j == strlen(fname)) {
				_close(lun,0);
				return(NULL);
				}
			}
	}
	return(NULL);
}
#endif

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

#define LOCKMSG TEXT36
/*              "LOCK ERROR -- " */
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

	  if (tellall) printf(TEXT37,lockname);
/*                            "checking for existence of %s\n" */

	  if (stat(lockname,&statblk))
	  {
		 if (tellall) printf(TEXT38,lockname);
/*                                   "making directory %s\n" */
		 mkdir(lockname,0777); 
	  }

	  sprintf(lockname,"%s/%s",lockname,gtname(fname));

	  if (tellall) printf(TEXT37,lockname);
/*                            "checking for existence of %s\n" */

	  if (stat(lockname,&statblk))
	  {
makelock:  	if (tellall) printf(TEXT39,lockname);
/*                                  "creating %s\n" */

		if ((lf = fopen(lockname,FOP_TW)) == NULL)
		  LOCKERR(TEXT40)
/*                        "could not create lock file" */
	        else
	  	{
			if (parent)
			 pid = getppid();	/* parent pid */
			else
			 pid = getpid();	/* current pid */

			 if (tellall)
			  printf(TEXT41,pid); 
/*                               "pid is %ld\n" */

			 fprintf(lf,"%ld",pid); /* write pid to lock file */

			fclose(lf);
			oldumask = umask(oldumask);
			return(NULL);
		}
	  }
	  else
	  {
		if (tellall) printf(TEXT42,lockname);
/*                                  "reading lock file %s\n" */
		if ((lf = fopen(lockname,FOP_TR)) == NULL)
		  LOCKERR(TEXT43)
/*                        "could not read lock file" */
	        else
	  	{
			fscanf(lf,"%ld",&pid); /* contains current pid */
			fclose(lf);
			if (tellall)
			 printf(TEXT44,lockname, pid);
/*                              "pid in %s is %ld\n" */
			if (tellall)
			 printf(TEXT45, pid);
/*                              "signaling process %ld\n" */
			if (kill(pid,0))
				switch (errno)
				{
				  case ESRCH:	/* process not found */
						goto makelock;
						break;
				  case EPERM:	/* process exists, not yours */
			 			if (tellall) 
						 puts(TEXT46);
/*                                                    "process exists" */
						break;
				  default:
					LOCKERR(TEXT47)
/*                                              "kill was bad" */
					break;
				}
			else
			 if (tellall) puts(TEXT48);
/*                                         "kill was good; process exists" */
		}
		if ((pblk = getpwuid(statblk.st_uid)) == NULL)
		  sprintf(username,"uid %d",asc_int(statblk.st_uid));
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

	  if (tellall) printf(TEXT49,lockname);
/*                            "attempting to unlink %s\n" */

	  if (unlink(lockname))
	  { 
		strcat(lmsg,TEXT50); 
/*                          "could not remove lock file" */
		return(lmsg); 
	  }
	  else
	  	  return(NULL);
}

#endif

/******************
 * end dolock module
 *******************/

#else
dolhello()
{
}
#endif

