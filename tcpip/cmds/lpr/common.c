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
static char sccsid[] = "@(#)common.c	5.4 (Berkeley) 6/30/88";
#endif /* not lint */
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/common.c,v 1.3 1994/03/17 17:01:23 nickc Exp $";
#endif
#endif

/*
 * Routines and data common to all the line printer functions.
 */

#include "lp.h"
#include <stdarg.h>

int	DU;		/* daeomon user-id */
int	MX;		/* maximum number of blocks to copy */
int	MC;		/* maximum number of copies allowed */
char	*LP;		/* line printer device name */
char	*RM;		/* remote machine name */
char	*RP;		/* remote printer name */
char	*LO;		/* lock file name */
char	*ST;		/* status file name */
char	*SD;		/* spool directory */
char	*AF;		/* accounting file */
char	*LF;		/* log file for error messages */
char	*OF;		/* name of output filter (created once) */
char	*IF;		/* name of input filter (created per job) */
char	*RF;		/* name of fortran text filter (per job) */
char	*TF;		/* name of troff filter (per job) */
char	*NF;		/* name of ditroff filter (per job) */
char	*DF;		/* name of tex filter (per job) */
char	*GF;		/* name of graph(1G) filter (per job) */
char	*VF;		/* name of vplot filter (per job) */
char	*CF;		/* name of cifplot filter (per job) */
char	*PF;		/* name of vrast filter (per job) */
char	*FF;		/* form feed string */
char	*TR;		/* trailer string to be output when Q empties */
short	SC;		/* suppress multiple copies */
short	SF;		/* suppress FF on each print job */
short	SH;		/* suppress header page */
short	SB;		/* short banner instead of normal header */
short	HL;		/* print header last */
short	RW;		/* open LP for reading and writing */
short	PW;		/* page width */
short	PL;		/* page length */
short	PX;		/* page width in pixels */
short	PY;		/* page length in pixels */
short	BR;		/* baud rate if lp is a tty */
int	FC;		/* flags to clear if lp is a tty */
int	FS;		/* flags to set if lp is a tty */
int	XC;		/* flags to clear for local mode */
int	XS;		/* flags to set for local mode */
short	RS;		/* restricted to those with local accounts */

char	line[BUFSIZ];
char	pbuf[BUFSIZ/2];	/* buffer for printcap strings */
char	*bp = pbuf;	/* pointer into pbuf for pgetent() */
char	*name;		/* program name */
char	*printer;	/* printer name */
char	host[32];	/* host machine name */
char	*from = host;	/* client's machine name */

void fatal (char *, ...) ;

/*
 * Create a connection to the remote printer server.
 * Most of this code comes from rcmd.c.
 */
int getport(char *rhost) 
{
  struct hostent *hp;
  struct servent *sp;
  struct sockaddr_in sin;
  int s, timo = 1, lport = IPPORT_RESERVED - 1;
  int err;

  /*
   * Get the host address and port number to connect to.
   */
  if (rhost == NULL)
    fatal("no remote host to connect to");
  hp = gethostbyname(rhost);
  if (hp == NULL)
    fatal("unknown host %s", rhost);
  sp = getservbyname("printer", "tcp");
  if (sp == NULL)
    fatal("printer/tcp: unknown service");
  bzero((char *)&sin, sizeof(sin));
  bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
  sin.sin_family = hp->h_addrtype;
  sin.sin_port = sp->s_port;

  /*
   * Try connecting to the server.
   */
retry:
  s = rresvport(&lport);
  if (s < 0)
    return(-1);

#ifndef __HELIOS
  if (connect(s, (caddr_t)&sin, sizeof(sin), 0) < 0) {
#else
  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
#endif    
    err = errno;
    (void) close(s);
    errno = err;
    if (errno == EADDRINUSE) {
      lport--;
      goto retry;
    }
    if (errno == ECONNREFUSED && timo <= 16) {
      sleep(timo);
      timo *= 2;
      goto retry;
    }
    return(-1);
  }
  return(s);
}

/*
 * Getline reads a line from the control file cfp, removes tabs, converts
 *  new-line to null and leaves it in line.
 * Returns 0 at EOF or the number of characters read.
 */
int getline(FILE *cfp)
{
  register int linel = 0;
  register char *lp = line;
  register c;

  while ((c = getc(cfp)) != '\n') {
    if (c == EOF)
      return(0);
    if (c == '\t') {
      do {
        *lp++ = ' ';
        linel++;
      } while ((linel & 07) != 0);
      continue;
    }
    *lp++ = c;
    linel++;
  }
  *lp++ = '\0';
  return(linel);
}

/*
 * Scan the current directory and make a list of daemon files sorted by
 * creation time.
 * Return the number of entries and a pointer to the list.
 */
int getq( void * arg )
{
  struct queue ***namelist;
  register struct direct *d;
  register struct queue *q, **queue;
  register int nitems;
  struct stat stbuf;
#ifndef __HELIOS
  int arraysz, compar();
#else
        int compar (struct queue **, struct queue **) ;
        int iscf (struct direct *) ; /* crf : used to be in rmjob () */
#endif  

  int arraysz = 0 ;

  DIR *dirp;

  namelist = (struct queue ***) arg;
  
  if ((dirp = opendir(SD)) == NULL)
    return(-1);
  if (fstat(dirp->dd_fd, &stbuf) < 0)
    goto errdone;

  /*
   * Estimate the array size by taking the size of the directory file
   * and dividing it by a multiple of the minimum size entry. 
   */
#ifndef __HELIOS
  arraysz = (stbuf.st_size / 24);
#endif
/*
-- crf : confirm the best way to do this ... why not just count the number 
-- of cfXXXXX files in the directory (in a similar way to the way I do it 
-- in rmjob())
*/
  debugf ("common: using alternative strategy for arraysz ...") ;

  d = readdir (dirp) ;
  while (d)
  {
    if (iscf (d))
      arraysz ++ ;
    d = readdir (dirp) ;
  }
  debugf ("found %d ctrl files", arraysz) ;
  rewinddir (dirp) ;
/*
-- crf (end of count ctrl files)
*/

  queue = (struct queue **)malloc(arraysz * sizeof(struct queue *));
  if (queue == NULL)
    goto errdone;

  nitems = 0;
  while ((d = readdir(dirp)) != NULL) {
#ifndef __HELIOS
    if (d->d_name[0] != 'c' || d->d_name[1] != 'f')
#else
    if (!iscf (d))
#endif    
      continue;  /* daemon control files only */
    if (stat(d->d_name, &stbuf) < 0)
      continue;  /* Doesn't exist */
    q = (struct queue *)malloc(sizeof(time_t)+strlen(d->d_name)+1);
    if (q == NULL)
      goto errdone;
    q->q_time = stbuf.st_mtime;
    strcpy(q->q_name, d->d_name);
    
    /*
     * Check to make sure the array has space left and
     * realloc the maximum size.
     */
/*
-- crf : this should now be redundant ...
*/     
    if (++nitems > arraysz) {
      queue = (struct queue **)realloc( (char *)queue,
        ((int)stbuf.st_size / 12) * sizeof (struct queue *));
      if (queue == NULL)
        goto errdone;
    }
    queue[nitems-1] = q;
  }

  if (closedir (dirp) != 0)
    return -1 ; /* cannot close directory */

  if (nitems)
    qsort(queue, nitems, sizeof(struct queue *), (int(*)())compar);
    
  *namelist = queue;
  return(nitems);

errdone:
  closedir(dirp);
  return(-1);
}

/*
 * Compare modification times.
 */
int compar (register struct queue **p1, 
            register struct queue **p2)
{
  if ((*p1)->q_time < (*p2)->q_time)
    return(-1);
  if ((*p1)->q_time > (*p2)->q_time)
    return(1);
  return(0);
}

#ifndef __HELIOS
/*VARARGS1*/
fatal(msg, a1, a2, a3)
  char *msg;
{
  if (from != host)
    printf("%s: ", host);
  printf("%s: ", name);
  if (printer)
    printf("%s: ", printer);
  printf(msg, a1, a2, a3);
  putchar('\n');
  exit(1);
}
#else
void fatal (char *format, ...)
{
  va_list args;

  va_start (args, format);
  if (from != host)
    printf ("%s: ", host);
  printf ("%s: ", name);
  if (printer)
    printf ("%s: ", printer);
  vprintf (format, args);
  putchar('\n');
  va_end (args);
  exit(1);
}
#endif

/*
-- crf : iscf() used to be in rmjob.c
*/
/*
 * Return 1 if the filename begins with 'cf'
 */
int iscf(struct direct *d)
{
  return(d->d_name[0] == 'c' && d->d_name[1] == 'f');
}
