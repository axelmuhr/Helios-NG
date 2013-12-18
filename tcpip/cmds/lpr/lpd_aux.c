/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __HELIOS
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lpd.c	5.6 (Berkeley) 6/30/88";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/lpd_aux.c,v 1.2 1992/02/08 12:14:20 craig Exp $";
#endif 

/*
 * lpd -- line printer daemon.
 *
 * Listen for a connection and perform the requested operation.
 * Operations are:
 *	\1printer\n
 *		check the queue for jobs and print any found.
 *	\2printer\n
 *		receive a job from another machine and queue it.
 *	\3printer [users ...] [jobs ...]\n
 *		return the current state of the queue (short form).
 *	\4printer [users ...] [jobs ...]\n
 *		return the current state of the queue (long form).
 *	\5printer person [users ...] [jobs ...]\n
 *		remove jobs from the queue.
 *
 * Strategy to maintain protected spooling area:
 *	1. Spooling area is writable only by daemon and spooling group
 *	2. lpr runs setuid root and setgrp spooling group; it uses
 *	   root to access any file it wants (verifying things before
 *	   with an access call) and group id to know how it should
 *	   set up ownership of files in the spooling area.
 *	3. Files in spooling area are owned by root, group spooling
 *	   group, with mode 660.
 *	4. lpd, lpq and lprm run setuid daemon and setgrp spooling group to
 *	   access files and printer.  Users can't get to anything
 *	   w/o help of lpq and lprm programs.
 */

#include <syslib.h>
#include "lp.h"
#include <unistd.h>

int  lflag;        /* log requests flag */

char lpd_aux_name [MAX_NAME_LEN] ;

extern void printjob (char *) ;
extern char **environ ;

void doit (void) ;

int main(int argc, char **argv)
{

  int command = atoi (argv [1]) ;
  name = argv [0] ;
  strcpy (host, argv [2]) ;
  lflag = atoi (argv [3]) ;

#ifdef MEM_CHECK
  IOdebug ("entering lpd_aux: Bytes free : %d  Heap size : %d", 
           Malloc(-1), Malloc(-3));
#endif

  (void) openlog ("lpd_aux", LOG_PID, LOG_LPR) ;

  {
    Environ *env = getenviron () ;
    sprintf(lpd_aux_name, "%s", env->Objv [OV_Task]->Name) ;

    if (lflag)
      syslog(LOG_INFO, "(%s) starting", lpd_aux_name);

    debugf ("lpd_aux_name = %s", lpd_aux_name) ;    
    strcat (lpd_aux_name, "\n") ;
  }

  switch (command)
  {
    case CALL_DOIT :
      doit () ;    
      break ;
    case CALL_PRINTJOB :
      printer = argv [4] ;
      printjob (lpd_aux_name) ;
      break ;
    default :
      fatal ("unknown auxiliary request") ;
  } 
}

/*
 * Stuff for handling job specifications
 */
char  *user[MAXUSERS];  /* users to process */
int  users;      /* # of users in user array */
int  requ[MAXREQUESTS];  /* job number of spool entries */
int  requests;    /* # of spool requests */
char  *person;    /* name of person doing lprm */

char  fromb[32];  /* buffer for client's machine name */
char  cbuf[BUFSIZ];  /* command line buffer */
char  *cmdnames[] = {
  "null",
  "printjob",
  "recvjob",
  "displayq short",
  "displayq long",
  "rmjob"
};

void doit()
{
  register char *cp;
  register int n;

  for (;;) {
    cp = cbuf;
    do {
      if (cp >= &cbuf[sizeof(cbuf) - 1])
        fatal("Command line too long");
      if ((n = read(1, cp, 1)) != 1) {
        if (n < 0)
          fatal("Lost connection");
        return;
      }
    } while (*cp++ != '\n');
    *--cp = '\0';
    cp = cbuf;
    if (lflag) {
      if (*cp >= '\1' && *cp <= '\5')
        syslog(LOG_INFO, "%s requests %s %s",
          from, cmdnames[*cp], cp+1);
      else
        syslog(LOG_INFO, "bad request (%d) from %s",
          *cp, from);
    }
    switch (*cp++) {
    case '\1':  /* check the queue and print any jobs there */
      printer = cp;
#ifndef __HELIOS      
      printjob();
#else
      printjob(lpd_aux_name);
#endif      
      break;
#ifndef __HELIOS
    case '\2':  /* receive files to be queued */
      printer = cp;
      recvjob();
      break;
    case '\3':  /* display the queue (short form) */
    case '\4':  /* display the queue (long form) */
      printer = cp;
      while (*cp) {
        if (*cp != ' ') {
          cp++;
          continue;
        }
        *cp++ = '\0';
        while (isspace(*cp))
          cp++;
        if (*cp == '\0')
          break;
        if (isdigit(*cp)) {
          if (requests >= MAXREQUESTS)
            fatal("Too many requests");
          requ[requests++] = atoi(cp);
        } else {
          if (users >= MAXUSERS)
            fatal("Too many users");
          user[users++] = cp;
        }
      }
      displayq(cbuf[0] - '\3');
      exit(0);
    case '\5':  /* remove a job from the queue */
      printer = cp;
      while (*cp && *cp != ' ')
        cp++;
      if (!*cp)
        break;
      *cp++ = '\0';
      person = cp;
      while (*cp) {
        if (*cp != ' ') {
          cp++;
          continue;
        }
        *cp++ = '\0';
        while (isspace(*cp))
          cp++;
        if (*cp == '\0')
          break;
        if (isdigit(*cp)) {
          if (requests >= MAXREQUESTS)
            fatal("Too many requests");
          requ[requests++] = atoi(cp);
        } else {
          if (users >= MAXUSERS)
            fatal("Too many users");
          user[users++] = cp;
        }
      }
      rmjob();
      break;
#endif      
    }
    fatal("Illegal service request");
  }
}

