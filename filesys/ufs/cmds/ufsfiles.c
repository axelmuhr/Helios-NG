/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 20 August 1992                                               */
/* File: ufsfiles.c                                                   */
/*                                                                    */
/* 
 * This program is used to list open files in a ufs file system.
 *
 * Usage: ufsfiles filesystem [-acnv]
 *        ufsfiles filesystem [-cnv] [-f filename ...] [-p pid ...] [-u uid]
 *
 *	where 	a is for all open files
 *		c is to close the open file
 *		n is do not confirm (if close and > 1 occurence)
 *		v is for verbose
 *
 *	and	filename	is a filename (all occurences)
 *		pid		is the unique file identifier
 *		uid		is a user identifier
 */
/* $Id: ufsfiles.c,v 1.1 1992/09/16 10:01:43 al Exp $ */
/* $Log: ufsfiles.c,v $
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 * */

#include "param.h"
#include "mount.h"
#include "socket.h"

#include <helios.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <sys/errno.h>

#define UFSOPENFILE
#include "../private.h"

char *progname;
struct UfsOpenFile **getopenfiles(char *dir, int *max);
int forceopenclosed(char *path, pid_t pid);

/* The system settings */
int verbose = FALSE;
int closefiles = FALSE;
int allfiles = FALSE;
int confirm = TRUE;
int npid = 0;
int nuid = 0;
int nfname = 0;
#define MAXOPTS	100
pid_t	pids[MAXOPTS];
uid_t	uids[MAXOPTS];
char *fnames[MAXOPTS];

/*
 * This will return TRUE if the openfile struct passed is a match
 * (specified on the command line).
 */
int match(struct UfsOpenFile *of)
{ int i;

  if (allfiles) return(TRUE);	/* Always a match */
	
  /* Is there a PID match */
  for (i=0; i<npid; i++)
	if (pids[i] == of->pid) return(TRUE);

  /* Is there a UID match */
  for (i=0; i<nuid; i++)
	if (uids[i] == of->uid) return(TRUE);

  /* Is there a FILENAME match */
  for (i=0; i<nfname; i++)
	if (!strcmp(fnames[i],of->name)) return(TRUE);

  return(FALSE);
}

void usage()
{
  fprintf(stderr,"usage: %s filesystem [-acnv]\n",progname);
  fprintf(stderr,"usage: %s filesystem %s\n", progname,
    "[-cnv] [-f filename ...] [-p pid ...] [-u uid ...]");
  Exit(1);
}

void showclose(char *str, struct UfsOpenFile *of)
{ char *buf = "                    "; /* 20 spaces */

  if (strlen(of->name) < 20)
	buf += 20-strlen(of->name);

  printf("%s file %s%s   PID %6d   UID %3d\n",
	str,buf,of->name,of->pid,of->uid);
}

int main(int argc, char *argv[])
{
  struct UfsOpenFile **ofs;
  struct UfsOpenFile *of;
  char *path, *opt;
  int max, index, numclosed;

  /* Get the program name and the path to the filesystem */
  progname = argv[0] + strlen(argv[0]) - 1;
  while ((*progname != '/') && (progname >= argv[0]))
	progname--;
  progname++;
  argc--;	argv++;
  if (argc < 1) usage();
  path = *argv;
  argc--;	argv++;
  
  /* Parse the arguments */
  while ((**argv == '-') && (argc)) {
	opt = *argv + 1;
moreopts:
	switch (*(opt++)) {
	case 'c':
		closefiles = TRUE;
		if (*opt) goto moreopts;
		argc--;	argv++;
		break;
	case 'a':
		allfiles = TRUE;
		if (*opt) goto moreopts;
		argc--;	argv++;
		break;
	case 'n':
		confirm = FALSE;
		if (*opt) goto moreopts;
		argc--;	argv++;
		break;
	case 'v':
		verbose = TRUE;
		if (*opt) goto moreopts;
		argc--;	argv++;
		break;
	case 'p':
		/* Add to the list of pids */
		argc--;	argv++;
		while ((**argv != '-') && (argc)) {
			pids[npid++] = atoi(*argv);
			argc--;	argv++;
		}
		break;
	case 'u':
		/* Add to the list of uids */
		argc--;	argv++;
		while ((**argv != '-') && (argc)) {
			uids[nuid++] = atoi(*argv);
			argc--;	argv++;
		}
		break;
	case 'f':
		/* Add to the list of filenames */
		argc--;	argv++;
		while ((**argv != '-') && (argc)) {
			fnames[nfname++] = *argv;
			argc--;	argv++;
		}
		break;
	default:
		fprintf(stderr,"%s: Invalid option %s\n",progname,*argv);
		usage();
		break;
	}
  }

  /* If any args left, then error */
  if (argc)
	usage();

  /* Allfile option is not valid with pid/uid/filename */
  if ((npid || nuid || nfname) && allfiles) {
	fprintf(stderr,"%s -a option not valid with -f -p or -u\n",progname);
	usage();
  }

  /* If closing, ensure that something is selected */
  if (closefiles && !(npid || nuid || nfname || allfiles)) {
	fprintf(stderr,"%s Must specify files to close\n",progname);
	usage();
  }

  /* Get the open files */
  if ((ofs = getopenfiles(path,&max)) == NULL) {
	if (!closefiles || verbose)
		printf("%s: No files are open\n",progname);
	Exit(0);
  }
 
  /* Display the files ?*/
  if (!closefiles) {
	int onedone = FALSE;

	/* If nothing listed, then list everything */
	if (!(npid || nuid || nfname)) allfiles = TRUE;

	printf("     PID  UID   Filename\n");
	for (index=0; ofs[index]; index++) {
		of = ofs[index];
		if (match(of)) {
			printf("%8d  %3d   %s\n",of->pid,of->uid,of->name);
			onedone = TRUE;
		}
		Free(of);
	}
	Free(ofs);
	if (!onedone) printf("No files open matching specifications\n");
	Exit(0);
  }

  /* If interactive, then also verbose */
  if (confirm)
	verbose = TRUE;

  /* Now run through the list of open files */
  numclosed = 0;
  for (index=0;  ofs[index]; index++) {
	of = ofs[index];
	if (match(of)) {
	    /* This must be forcibly closed */
	    if (confirm) {
		fprintf(stderr,"Force closure of %s   PID %d   UID %d ? ",
			of->name,of->pid,of->uid);
		fflush(stderr);
		if (tolower(getchar()) != 'y') {
			while (getchar() != '\n');
			continue;
		}
		while (getchar() != '\n');
	    }

	    if (forceopenclosed(path,of->pid)) {
		/* Error in close */
		if (verbose) {
			char buffer[128];
	
			sprintf(buffer,"Posix error %d in closing",errno);
			showclose(buffer,of);
		}
	    } else {
		numclosed++;
		if (verbose)
			showclose("Closed",of);		
	    }
	}
	Free(of);
  }
  Free(ofs);
  printf("%d files were closed\n",numclosed);  
  Exit(0);
}
