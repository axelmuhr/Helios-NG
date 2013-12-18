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
static char sccsid[] = "@(#)displayq.c	5.8 (Berkeley) 6/30/88";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/displayq.c,v 1.3 1992/02/25 10:55:12 craig Exp $";
#endif

/*
 * Routines to display the state of the queue.
 */

#include "lp.h"

#include <syslib.h>
#include "printcap.h"

#define JOBCOL	40		/* column for job # in -l format */
#define OWNCOL	7		/* start of Owner column in normal */
#define SIZCOL	62		/* start of Size column in normal */

/*
 * Stuff for handling job specifications
 */
extern char	*user[];	/* users to process */
extern int	users;		/* # of users in user array */
extern int	requ[];		/* job number of spool entries */
extern int	requests;	/* # of spool requests */

#ifndef __HELIOS
int	lflag;		/* long output option */
#else
int	long_flag;	/* long output option */
#endif

#ifndef __HELIOS
char	current[40];	/* current file being printed */
#else
char	current [256];	/* current file being printed */
#endif

int	garbage;	/* # of garbage cf files */
int	rank;		/* order to be printed (-1=none, 0=active) */
long	totsize;	/* total print job size in bytes */
int	first;		/* first file in ``files'' column? */
int	col;		/* column on screen */
#ifndef __HELIOS
int	sendtorem;	/* are we sending to a remote? */
#else
int	sendtorem = TRUE;	/* are we sending to a remote? */
#endif
char	file[132];	/* print file name */

char	*head0 = "Rank   Owner      Job  Files";
char	*head1 = "Total Size\n";

void warn (void) ;
void header (void) ;
void inform (char *) ;
int inlist (char *, char *) ;
void show (char *, char *, int) ;
void blankfill (int) ;
void dump (char *, char *, int) ; 
void ldump (char *, char *, int) ;
void prank (int) ;

/*
 * Display the current state of the queue. Format = 1 if long format.
 */
void displayq (int format)
{
  register struct queue *q;
  register int i, nitems, fd;
  register char  *cp;
  struct queue **queue;
  struct stat statb;
  FILE *fp;
#ifndef __HELIOS
  char c;
  lflag = format;
#else
  long_flag = format;
#endif  
  
#ifdef __HELIOS
  {
    struct sigaction act;
    act.sa_handler = f_lock_exit;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    (void) sigaction(SIGINT, &act, NULL);
    (void) sigaction(SIGHUP, &act, NULL);
    (void) sigaction(SIGQUIT, &act, NULL);
    (void) sigaction(SIGTERM, &act, NULL);
  }
#endif

  totsize = 0;
  rank = -1;

  if ((i = pgetent(line, printer)) < 0)
    fatal("cannot open printer description file");
  else if (i == 0)
    fatal("unknown printer");
  if ((LP = pgetstr("lp", &bp)) == NULL)
    LP = DEFDEVLP;
  if ((RP = pgetstr("rp", &bp)) == NULL)
    RP = DEFLP;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  if ((ST = pgetstr("st", &bp)) == NULL)
    ST = DEFSTAT;
  RM = pgetstr("rm", &bp);

  /*
   * Figure out whether the local machine is the same as the remote 
   * machine entry (if it exists).  If not, then ignore the local
   * queue information.
   */
   if (RM != (char *) NULL) {
    char name[256];
    struct hostent *hp;

    /* get the standard network name of the local host */
    gethostname(name, sizeof(name));
    name[sizeof(name)-1] = '\0';
    hp = gethostbyname(name);
    if (hp == (struct hostent *) NULL) {
#ifndef __HELIOS
        printf("unable to get network name for local machine %s\n",
      name);
#else
        printf("%s %s\n", ERR_LOCAL_NAME, name);
#endif
        goto localcheck_done;
    } else (void) strcpy(name, hp->h_name);

    /* get the network standard name of RM */
    hp = gethostbyname(RM);
    if (hp == (struct hostent *) NULL) {
#ifndef __HELIOS
        printf("unable to get hostname for remote machine %s\n",
      RM);
#else
        printf("%s %s\n", ERR_REMOTE_NAME, RM);
#endif
        goto localcheck_done;
    }

    /* if printer is not on local machine, ignore LP */
    if (strcmp(name, hp->h_name)) {
      *LP = '\0';
      ++sendtorem;
    }
  }
localcheck_done:

  /*
   * Print out local queue
   * Find all the control files in the spooling directory
   */
  if (chdir(SD) < 0)
    fatal("cannot chdir to spooling directory");
  if ((nitems = getq(&queue)) < 0)
    fatal("cannot examine spooling area\n");

#ifndef __HELIOS    
  if (stat(LO, &statb) >= 0) {
    if (statb.st_mode & 0100) {
      if (sendtorem)
        printf("%s: ", host);
      printf("Warning: %s is down: ", printer);
      fd = open(ST, O_RDONLY);
      if (fd >= 0) {
        (void) flock(fd, LOCK_SH);
        while ((i = read(fd, line, sizeof(line))) > 0)
          (void) fwrite(line, 1, i, stdout);
        (void) close(fd);  /* unlocks as well */
      } else
        putchar('\n');
    }
    if (statb.st_mode & 010) {
      if (sendtorem)
        printf("%s: ", host);
      printf("Warning: %s queue is turned off\n", printer);
    }
  }
#else
  debugf ("displayq: testing for printer status") ;
  {
    int lockmode ;
    if (
         (stat (LO, &statb) >= 0) && 
         ((lockmode = get_mode (DEFMODE)) > 0)
       ) 
    {
      if (lockmode & 0100) {
        if (sendtorem)
          printf("%s: ", host);
        printf("Warning: %s is down: ", printer);
        fd = open (ST, O_RDONLY);
        if (fd >= 0) {
#ifndef __HELIOS        
          (void) flock(fd, LOCK_SH);
#else
          (void) f_lock(fd, LOCK_SH);
#endif
          while ((i = read(fd, line, sizeof(line))) > 0)
            (void) fwrite(line, 1, i, stdout);
          (void) close(fd);  /* unlocks as well */
#ifdef __HELIOS
          (void) f_lock (fd, LOCK_UN) ;
#endif          
        } else
          putchar('\n');
      }

      if (lockmode & 010) {
        if (sendtorem)
          printf("%s: ", host);
        printf("Warning: %s queue is turned off\n", printer);
      }
    }
  }
#endif

  if (nitems) {
    fp = fopen(LO, "r");
    if (fp == NULL)
      warn();
    else {
      register char *cp;
      register c;

      /* get daemon pid */
      cp = current;
      while ((c = getc(fp)) != EOF && c != '\n')
        *cp++ = c;
      *cp = '\0';
#ifndef __HELIOS
      i = atoi(current);
      if (i <= 0 || kill(i, 0) < 0)
#else
      debugf ("displayq: lpd name = %s", current) ;
      if ((*current == NULL) || 
          (Locate (NULL, current) == NULL))
        warn();
#endif      
      else {
        /* read current file name */
        cp = current;
        while ((c = getc(fp)) != EOF && c != '\n')
          *cp++ = c;
        *cp = '\0';

        /*
         * Print the status file.
         */
        if (sendtorem)
          printf("%s: ", host);
        fd = open(ST, O_RDONLY);
        if (fd >= 0) {
#ifndef __HELIOS          
          (void) flock(fd, LOCK_SH);
#else
          (void) f_lock(fd, LOCK_SH);
#endif
          while ((i = read(fd, line, sizeof(line))) > 0)
            (void) fwrite(line, 1, i, stdout);
          (void) close(fd);  /* unlocks as well */
#ifdef __HELIOS
          (void) f_lock (fd, LOCK_UN) ;
#endif          
        } else
          putchar('\n');
      }
      (void) fclose(fp);
    }
    /*
     * Now, examine the control files and print out the jobs to
     * be done for each user.
     */
#ifndef __HELIOS
    if (!lflag)
#else
    if (!long_flag)
#endif    
      header();
    for (i = 0; i < nitems; i++) {
      q = queue[i];
      inform(q->q_name);
      free(q);
    }
    free(queue);
  }
  if (!sendtorem) {
#ifndef __HELIOS
    if (nitems == 0)
      puts("no entries");
    return;
#else
/*
-- crf: this *should* never happen ... sendtorem is initialized to TRUE
*/
    debugf ("displayq: FATAL - (!sendtorem)") ;
#endif    
  }

  /*
   * Print foreign queue
   * Note that a file in transit may show up in either queue.
   */
  if (nitems)
    putchar('\n');
  (void) sprintf(line, "%c%s", format + '\3', RP);
  cp = line;
  for (i = 0; i < requests; i++) {
    cp += strlen(cp);
    (void) sprintf(cp, " %d", requ[i]);
  }
  for (i = 0; i < users; i++) {
    cp += strlen(cp);
    *cp++ = ' ';
    (void) strcpy(cp, user[i]);
  }
  strcat(line, "\n");
  fd = getport(RM);
  if (fd < 0) {
    if (from != host)
      printf("%s: ", host);
    printf("connection to %s is down\n", RM);
  }
  else {
    i = strlen(line);
    if (write(fd, line, i) != i)
      fatal("Lost connection");
    while ((i = read(fd, line, sizeof(line))) > 0)
      (void) fwrite(line, 1, i, stdout);
    (void) close(fd);
  }
}

/*
 * Print a warning message if there is no daemon present.
 */
void warn()
{
  if (sendtorem)
    printf("\n%s: ", host);
  puts("Warning: no daemon present");
  current[0] = '\0';
}

/*
 * Print the header for the short listing format
 */
void header()
{
  printf(head0);
  col = strlen(head0)+1;
  blankfill(SIZCOL);
  printf(head1);
}

void inform(char *cf)
{
#ifndef __HELIOS
  register int j, k;
  register char *cp;
#else
  register int j;
#endif  
  FILE *cfp;

  /*
   * There's a chance the control file has gone away
   * in the meantime; if this is the case just keep going
   */
  if ((cfp = fopen(cf, "r")) == NULL)
    return;

  if (rank < 0)
    rank = 0;
  if (sendtorem || garbage || strcmp(cf, current))
    rank++;
  j = 0;
  while (getline(cfp)) {
    switch (line[0]) {
    case 'P': /* Was this file specified in the user's list? */
      if (!inlist(line+1, cf)) {
        fclose(cfp);
        return;
      }
#ifndef __HELIOS
      if (lflag) {
#else
      if (long_flag) {
#endif        
        printf("\n%s: ", line+1);
        col = strlen(line+1) + 2;
        prank(rank);
        blankfill(JOBCOL);
        printf(" [job %s]\n", cf+3);
      } else {
        col = 0;
        prank(rank);
        blankfill(OWNCOL);
        printf("%-10s %-3d  ", line+1, atoi(cf+3));
        col += 16;
        first = 1;
      }
      continue;
    default: /* some format specifer and file name? */
      if (line[0] < 'a' || line[0] > 'z')
        continue;
      if (j == 0 || strcmp(file, line+1) != 0)
        (void) strcpy(file, line+1);
      j++;
      continue;
    case 'N':
      show(line+1, file, j);
      file[0] = '\0';
      j = 0;
    }
  }
  fclose(cfp);
#ifndef __HELIOS
  if (!lflag) {
#else
  if (!long_flag) {
#endif    
    blankfill(SIZCOL);
    printf("%ld bytes\n", totsize);
    totsize = 0;
  }
}

int inlist(char *name, char *file)
{
  register int *r, n;
  register char **u, *cp;

  if (users == 0 && requests == 0)
    return(1);
  /*
   * Check to see if it's in the user list
   */
  for (u = user; u < &user[users]; u++)
    if (!strcmp(*u, name))
      return(1);
  /*
   * Check the request list
   */
  for (n = 0, cp = file+3; isdigit(*cp); )
    n = n * 10 + (*cp++ - '0');
  for (r = requ; r < &requ[requests]; r++)
#ifndef __HELIOS
    if (*r == n && !strcmp(cp, from))
#else
    if (*r == n && !strncmp(cp, from, 2))
#endif    
      return(1);
  return(0);
}

void show (register char *nfile, register char *file, int copies)
{
  if (strcmp(nfile, " ") == 0)
    nfile = "(standard input)";
#ifndef __HELIOS
  if (lflag)
#else
  if (long_flag)
#endif  
    ldump(nfile, file, copies);
  else
    dump(nfile, file, copies);
}

/*
 * Fill the line with blanks to the specified column
 */
void blankfill(register int n)
{
  while (col++ < n)
    putchar(' ');
}

/*
 * Give the abbreviated dump of the file names
 */
void dump (char *nfile, char *file, int copies) 
{
  register short n, fill;
  struct stat lbuf;

  /*
   * Print as many files as will fit
   *  (leaving room for the total size)
   */
   fill = first ? 0 : 2;  /* fill space for ``, '' */
   if (((n = strlen(nfile)) + col + fill) >= SIZCOL-4) {
    if (col < SIZCOL) {
      printf(" ..."), col += 4;
      blankfill(SIZCOL);
    }
  } else {
    if (first)
      first = 0;
    else
      printf(", ");
    printf("%s", nfile);
    col += n+fill;
  }
  if (*file && !stat(file, &lbuf))
    totsize += copies * lbuf.st_size;
}

/*
 * Print the long info about the file
 */
void ldump(char *nfile, char *file, int copies)
{
  struct stat lbuf;

  putchar('\t');
  if (copies > 1)
    printf("%-2d copies of %-19s", copies, nfile);
  else
    printf("%-32s", nfile);
  if (*file && !stat(file, &lbuf))
    printf(" %ld bytes", lbuf.st_size);
  else
    printf(" ??? bytes");
  putchar('\n');
}

/*
 * Print the job's rank in the queue,
 *   update col for screen management
 */
void prank(int n)
{
  char line[100];
  static char *r[] = {
    "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"
  };

  if (n == 0) {
    printf("active");
    col += 6;
    return;
  }
  if ((n/10) == 1)
    (void) sprintf(line, "%dth", n);
  else
    (void) sprintf(line, "%d%s", n, r[n%10]);
  col += strlen(line);
  printf("%s", line);
}
