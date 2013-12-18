/**
*
* Title:  Helios Shell - Global Data.
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/data.c,v 1.6 1990/12/11 12:04:54 martyn Exp $
*
**/
#include "shell.h"

/* Flags */

/*BOOL debugging = FALSE;	*/
/*BOOL debugging = TRUE;	*/            /* */

#if 0  /* DEBUGGING */
BOOL debugging = TRUE ;
#else
BOOL debugging = FALSE;      /* */
#endif /* DEBUGGING */

BOOL interactive = FALSE;	/* The Input is Interactive */
BOOL singleline = FALSE;	/* Only Execute a single Line */
BOOL exitonerror = FALSE;	/* Do we exit on an Error ? */
BOOL fast = FALSE;		/* Is it a Fast Shell ? */
BOOL login = FALSE;		/* Is it a Login Shell ? */
BOOL usingcdl = FALSE;          /* Are we using CDL ? */
BOOL exitflag = FALSE;          /* Time to exit ? */
BOOL backgroundtask = FALSE;    /* Run in background ? */
int innewfile = 0;              /* doing "source" ? */
int fdssaved = 0;               /* number of times thru runbuiltin loop */


char *filename;			/* Name of Input file */
long lineposition;		/* */
int eventnumber = 1;		/* History EventNumber number */
jmp_buf home;			/* Safety */
FILE *inputfile;		/* */

CMD *globcmd = NULL;
ARGV globargv = NULL;
ARGV wordlist = NULL;
char *currentword;
BOOL lastword = FALSE;
TOKEN token;
int parencount = 0;
BOOL parsingerror;
int waitwrpid = 0;
int shellpid;
BOOL breakflag = FALSE;

BOOL memorychecking = TRUE;
int totalmemory = 0;
int sfds[3];
