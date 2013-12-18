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
static char sccsid[] = "@(#)printjob.c	5.6 (Berkeley) 6/30/88";
#endif /* not lint */
#else
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/printjob.c,v 1.11 1994/03/17 16:59:15 nickc Exp $";
#endif
#endif

/*
 * printjob -- print jobs in the queue.
 *
 *	NOTE: the lock file is used to pass information to lpq and lprm.
 *	it does not need to be removed because file locks are dynamic.
 */

#include "lp.h"
#include <stdarg.h>
#include <syslib.h>
#include "printcap.h"

#define DORETURN	0	/* absorb fork error */
#define DOABORT		1	/* abort if dofork fails */

/*
 * Error tokens
 */
#define REPRINT		-2
#define ERROR		-1
#define	OK		0
#define	FATALERR	1
#define	NOACCT		2
#ifndef __HELIOS
#define	FILTERERR	3
#define	ACCESS		4
#endif

char	title[80];		/* ``pr'' title */
FILE	*cfp;			/* control file */
int	pfd;			/* printer file descriptor */
int	ofd;			/* output filter file descriptor */
int	lfd;			/* lock file descriptor */
#ifndef __HELIOS
int	pid;			/* pid of lpd process */
#endif
int	prchild;		/* id of pr process */
#ifndef __HELIOS
int	child;			/* id of any filters */
int	ofilter;		/* id of output filter, if any */
#endif
int	tof;			/* true if at top of form */
int	remote;			/* true if sending files to remote */
dev_t	fdev;			/* device of file pointed to by symlink */
ino_t	fino;			/* inode of file pointed to by symlink */

char	fromhost[32];		/* user's host machine */
char	logname[32];		/* user's login name */
char	jobname[100];		/* job or file name */
char	Class[32];		/* classification field */
char	width[10] = "-w";	/* page width in characters */
char	length[10] = "-l";	/* page length in lines */
char	pxwidth[10] = "-x";	/* page width in pixels */
char	pxlength[10] = "-y";	/* page length in pixels */
char	indent[10] = "-i0";	/* indentation size in characters */
#ifndef __HELIOS
char	*tempfile = "errsXXXXXX"; /* file name for filter output */
#endif

#ifdef __HELIOS
void status (char *, ...) ;
void print_fatal (char *, ...) ;
int LF_fd = -1 ;
#endif

void printjob (char *lpd_aux_name)
{
  struct stat stb;
  register struct queue *q, **qp;
  struct queue **queue;
  register int i, nitems;
  long pidoff;
  int count = 0;
#ifndef __HELIOS
  extern int abortpr();
#else
  void init(void) ;
  void abortpr(void) ;
  void openpr(void) ;
  int sendit(char *) ; 
  void check_filter (char *, char *) ;
#endif

#ifdef __HELIOS
  {
    struct sigaction act;
    act.sa_handler = abortpr;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    (void) sigaction(SIGINT, &act, NULL);
    (void) sigaction(SIGHUP, &act, NULL);
    (void) sigaction(SIGQUIT, &act, NULL);
    (void) sigaction(SIGTERM, &act, NULL);
  }
#endif

  init();          /* set up capabilities */

  (void) write(1, "", 1);      /* ack that daemon is started */

  (void) close(2);      /* set up log file */

#ifndef __HELIOS
  if (open(LF, O_WRONLY|O_APPEND, 0664) < 0) {
    syslog(LOG_ERR, "%s: %m", LF);
    (void) open("/dev/null", O_WRONLY);
  }
#else
/*
-- crf : if LF doesn't exist it should be created ... but (obviously) not if
-- LF is the controlling terminal
*/
  if (strcmp (LF, DEFLOGF))
  {
    struct stat LF_stb;
    if ((stat (LF, &LF_stb) < 1) && (errno == ENOENT))
    {
      debugf ("printjob: creating LF") ;
      if ((LF_fd = creat (LF, 0664)) < 0) 
        debugf ("printjob: LF creat() error") ;
      (void) close (LF_fd) ;
    }
  }

  if ((LF_fd = open (LF, O_WRONLY|O_APPEND, 0664)) < 0) 
  {
    syslog(LOG_ERR, "%s: %m", LF);
    (void) open("/null", O_WRONLY);
  }
  debugf ("LF_fd = %d", LF_fd) ;

  debugf ("LP = %s", LP) ; 
  debugf ("RM = %s", RM) ; 

/*
-- crf: there are a number of printcap options that are meaningless/senseless
-- in this system - in particular, the filter options. I think it is a good
-- idea to warn the user if these options are used (I am restricting these
-- warning to filter-related options).
*/
  check_filter (OF, "of");
  check_filter (IF, "if");
  check_filter (RF, "rf");
  check_filter (TF, "tf");
  check_filter (NF, "nf");
  check_filter (DF, "df");
  check_filter (GF, "gf");
  check_filter (VF, "vf");
  check_filter (CF, "cf");
#endif

#ifndef __HELIOS
  setgid(getegid());
  pid = getpid();        /* for use with lprm */
#ifdef USG
  setpgrp();
#else
  setpgrp(0, pid);
#endif
#else
  debugf ("printjob: setgid(), setpgrp() skipped") ;
#endif

#ifndef __HELIOS
  signal(SIGHUP, abortpr);
  signal(SIGINT, abortpr);
  signal(SIGQUIT, abortpr);
  signal(SIGTERM, abortpr);
#endif

#ifndef __HELIOS
  (void) mktemp(tempfile);
#endif

  /*
   * uses short form file names
   */
  if (chdir(SD) < 0) {
    syslog(LOG_ERR, "%s: %m", SD);
    exit(1);
  }
#ifndef __HELIOS
  if (stat(LO, &stb) == 0 && (stb.st_mode & 0100))
    exit(0);    /* printing disabled */
#else
  debugf ("printjob: checking printer status") ;
  {
    int lockmode ;
    if (
         ((lockmode = get_mode (DEFMODE)) > 0) && 
         (lockmode & 0100)
       )
    {
      debugf ("printjob: %s is disabled", printer);
      exit(0);    /* printing disabled */
    }
  }
#endif

#ifndef __HELIOS    
  lfd = open(LO, O_WRONLY|O_CREAT, 0644);
#else
/*
-- crf : I need to read this file to get the name of the prev. daemon
*/
  lfd = open(LO, O_RDWR|O_CREAT, 0644);
#endif  

  if (lfd < 0) {
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
#ifdef __HELIOS
  if (set_mode (DEFMODE, 0644) < 0)
  {
    syslog(LOG_ERR, "%s: %s: %m", printer, DEFMODE);
    exit(1);
  }
#endif

#ifndef __HELIOS
  if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
    if (errno == EWOULDBLOCK)  /* active deamon present */
      exit(0);
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
#else
/*
-- crf: take note - I am not unlocking this file anywhere ... this would
-- have to be done at every instance where the program exits. Instead, I am
-- relying on the atexit function (f_lock_exit()) to handle this.
*/
  if (f_lock(lfd, LOCK_EX) < 0) 
  {
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
/*
-- crf : if there is a daemon running, its name will be in the first line
-- of the lock file.
*/  
  debugf ("printjob: testing for presence of daemon") ;
  {
    char prev_daemon [256] ;

    if (read (lfd, prev_daemon, 256) <= 0)
      debugf ("printjob: could not read lock file") ;
    else
    {
      debugf ("printjob: prev daemon name = %s", prev_daemon) ;
      if (Locate (NULL, prev_daemon) != NULL)
      {
        syslog (LOG_INFO, "active daemon (%s) present", prev_daemon);
        exit(0);  /* active daemon present */
      }
    }      
  }
#endif

#ifndef __HELIOS
  ftruncate(lfd, 0);
#else
  debugf ("printjob: cludging ftruncate") ;
  close (lfd) ;
  lfd = open(LO, O_WRONLY|O_TRUNC, 0644);
  if (lfd < 0) 
  {
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
#endif

  /*
   * write process id for others to know
   */
#ifndef __HELIOS
  sprintf(line, "%u\n", pid);
  pidoff = i = strlen(line);
  if (write(lfd, line, i) != i) {
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
#else
  pidoff = i = strlen (lpd_aux_name);
  if (write (lfd, lpd_aux_name, i) != i) {
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
#endif  
  /*
   * search the spool directory for work and sort by queue order.
   */
  if ((nitems = getq( (void *)&queue)) < 0) {
    syslog(LOG_ERR, "%s: can't scan %s", printer, SD);
    exit(1);
  }
  if (nitems == 0)    /* no work to do */
  {
#ifdef MEM_CHECK
    IOdebug ("exiting printjob: Bytes free : %d  Heap size : %d", 
             Malloc(-1), Malloc(-3));
#endif
    exit(0);
  }
  
  debugf ("printjob: Not happy here - appear to be missing a stat ?") ;

#ifndef __HELIOS
  if (stb.st_mode & 01) {    /* reset queue flag */
#ifdef HAVE_FCHMOD
    if (fchmod(lfd, stb.st_mode & 0776) < 0)
#else
    if (chmod(LO, stb.st_mode & 0776) < 0)
#endif
      syslog(LOG_ERR, "%s: %s: %m", printer, LO);
  }
#else
{
  int lockmode = get_mode (DEFMODE) ;
  if (lockmode & 01) {    /* reset queue flag */
    if (set_mode (DEFMODE, lockmode & 0776) < 0)
      syslog(LOG_ERR, "%s: %s: %m", printer, DEFMODE);
  }
}
#endif

  openpr();      /* open printer or remote */
again:
  /*
   * we found something to do now do it --
   *    write the name of the current control file into the lock file
   *    so the spool queue program can tell what we're working on
   */
  for (qp = queue; nitems--; free((char *) q)) {
    q = *qp++;
    if (stat(q->q_name, &stb) < 0)
      continue;
  restart:
    (void) lseek(lfd, pidoff, 0);
    (void) sprintf(line, "%s\n", q->q_name);
    i = strlen(line);
    if (write(lfd, line, i) != i)
      syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    if (!remote)
#ifndef __HELIOS    
      i = printit(q->q_name);
#else
      print_fatal ("invalid output device: %s", LP) ;
#endif
    else
      i = sendit(q->q_name);
    /*
     * Check to see if we are supposed to stop printing or
     * if we are to rebuild the queue.
     */

    debugf ("printjob: caution - wrong filemode will cause termination") ;

#ifndef __HELIOS
    if (fstat(lfd, &stb) == 0) {
      /* stop printing before starting next job? */
      if (stb.st_mode & 0100)
        goto done;
      /* rebuild queue (after lpc topq) */
      if (stb.st_mode & 01) {
        for (free((char *) q); nitems--; free((char *) q))
          q = *qp++;
#ifdef HAVE_FCHMOD
        if (fchmod(lfd, stb.st_mode & 0776) < 0)
#else
        if (chmod(LO, stb.st_mode & 0776) < 0)
#endif
          syslog(LOG_WARNING, "%s: %s: %m",
            printer, LO);
        break;
      }
    }
#else
  {
    int lockmode ;
    if (
         (fstat (lfd, &stb) >= 0) && 
         ((lockmode = get_mode (DEFMODE)) > 0)
       ) 
    {
      /* stop printing before starting next job? */
      if (lockmode & 0100)
        goto done;
      /* rebuild queue (after lpc topq) */
      if (lockmode & 01) {
        for (free((char *) q); nitems--; free((char *) q))
          q = *qp++;
        if (set_mode (DEFMODE, lockmode & 0776) < 0)
          syslog(LOG_WARNING, "%s: %s: %m",
            printer, DEFMODE);
        break;
      }
    }
  }
#endif

    if (i == OK)    /* file ok and printed */
      count++;
    else if (i == REPRINT) { /* try reprinting the job */
      syslog(LOG_INFO, "restarting %s", printer);
#ifndef __HELIOS
      if (ofilter > 0) {
        kill(ofilter, SIGCONT);  /* to be sure */
        (void) close(ofd);
        while ((i = wait(0)) > 0 && i != ofilter)
          ;
        ofilter = 0;
      }
#endif      
      (void) close(pfd);  /* close printer */
#ifndef __HELIOS      
      if (ftruncate(lfd, pidoff) < 0)
        syslog(LOG_WARNING, "%s: %s: %m", printer, LO);
#else
/*
-- crf : can't ftruncate ... the first pidoff bytes in the file *should*
-- be the daemon name (i.e. lpd_aux_name). To be safe, I will try to close
-- the file, open it for reading and get the first pidoff bytes. Then, I'll
-- close it again, open it for writing (and truncate the size to zero), and
-- then write the bytes back again. There must be an easier, more elegant
-- way of doing this ???
*/
  debugf ("printjob: BIG ftruncate() cludge") ;
  (void) close (lfd) ;
  if ((lfd = open (LO, O_RDONLY, 0644)) < 0)
  {
    syslog(LOG_ERR, "%s: %s: %m", printer, LO);
    exit(1);
  }
  
  {
    char LO_buff [256] ;
    if (read (lfd, LO_buff, (int)pidoff) != pidoff)
    {
      syslog(LOG_ERR, "%s: %s: %m", printer, LO);
      exit(1);
    }
    if (!(strcmp (LO_buff, lpd_aux_name)) || (pidoff != strlen (lpd_aux_name)))
    {
      /* 
      -- crf : would be surprised if this happens ...
      */
      syslog (LOG_WARNING, "inconsistent entry in %s", LO) ;
    }

    close (lfd) ;
    lfd = open(LO, O_WRONLY|O_TRUNC, 0644);
    if (lfd < 0) 
    {
      syslog(LOG_ERR, "%s: %s: %m", printer, LO);
      exit(1);
    }
    if (write (lfd, LO_buff, (int) pidoff) != pidoff) 
    {
      syslog(LOG_ERR, "%s: %s: %m", printer, LO);
      exit(1);
    }
  }
#endif        
      openpr();    /* try to reopen printer */
      goto restart;
    }
  }
  free((char *) queue);
  /*
   * search the spool directory for more work.
   */
  if ((nitems = getq((void *)&queue)) < 0) {
    syslog(LOG_ERR, "%s: can't scan %s", printer, SD);
    exit(1);
  }
  if (nitems == 0) {    /* no more work to do */
  done:
    if (count > 0) {  /* Files actually printed */
      if (!SF && !tof)
        (void) write(ofd, FF, strlen(FF));
      if (TR != NULL)    /* output trailer */
        (void) write(ofd, TR, strlen(TR));
    }
#ifndef __HELIOS
    (void) unlink(tempfile);
#endif    
#ifdef MEM_CHECK
    IOdebug ("exiting printjob: Bytes free : %d  Heap size : %d", 
             Malloc(-1), Malloc(-3));
#endif
    exit(0);
  }
  goto again;
}

#ifndef __HELIOS
char  fonts[4][50];  /* fonts for troff */

char ifonts[4][18] = {
  "/usr/lib/vfont/R",
  "/usr/lib/vfont/I",
  "/usr/lib/vfont/B",
  "/usr/lib/vfont/S"
};

/*
 * The remaining part is the reading of the control file (cf)
 * and performing the various actions.
 */
printit(file)
  char *file;
{
  register int i;
  char *cp;
  int bombed = OK;

  /*
   * open control file; ignore if no longer there.
   */
  if ((cfp = fopen(file, "r")) == NULL) {
    syslog(LOG_INFO, "%s: %s: %m", printer, file);
    return(OK);
  }
  /*
   * Reset troff fonts.
   */
  for (i = 0; i < 4; i++)
    strcpy(fonts[i], ifonts[i]);
  strcpy(width+2, "0");
  strcpy(indent+2, "0");

  /*
   *      read the control file for work to do
   *
   *      file format -- first character in the line is a command
   *      rest of the line is the argument.
   *      valid commands are:
   *
   *    S -- "stat info" for symbolic link protection
   *    J -- "job name" on banner page
   *    C -- "class name" on banner page
   *              L -- "literal" user's name to print on banner
   *    T -- "title" for pr
   *    H -- "host name" of machine where lpr was done
   *              P -- "person" user's login name
   *              I -- "indent" amount to indent output
   *              f -- "file name" name of text file to print
   *    l -- "file name" text file with control chars
   *    p -- "file name" text file to print with pr(1)
   *    t -- "file name" troff(1) file to print
   *    n -- "file name" ditroff(1) file to print
   *    d -- "file name" dvi file to print
   *    g -- "file name" plot(1G) file to print
   *    v -- "file name" plain raster file to print
   *    c -- "file name" cifplot file to print
   *    1 -- "R font file" for troff
   *    2 -- "I font file" for troff
   *    3 -- "B font file" for troff
   *    4 -- "S font file" for troff
   *    N -- "name" of file (used by lpq)
   *              U -- "unlink" name of file to remove
   *                    (after we print it. (Pass 2 only)).
   *    M -- "mail" to user when done printing
   *
   *      getline reads a line and expands tabs to blanks
   */

  /* pass 1 */

  while (getline(cfp))
    switch (line[0]) {
    case 'H':
      strcpy(fromhost, line+1);
      if (Class[0] == '\0')
        strncpy(Class, line+1, sizeof(Class)-1);
      continue;

    case 'P':
      strncpy(logname, line+1, sizeof(logname)-1);
      if (RS) {      /* restricted */
        if (getpwnam(logname) == (struct passwd *)0) {
          bombed = NOACCT;
          sendmail(line+1, bombed);
          goto pass2;
        }
      }
      continue;

    case 'S':
      cp = line+1;
      i = 0;
      while (*cp >= '0' && *cp <= '9')
        i = i * 10 + (*cp++ - '0');
      fdev = i;
      cp++;
      i = 0;
      while (*cp >= '0' && *cp <= '9')
        i = i * 10 + (*cp++ - '0');
      fino = i;
      continue;

    case 'J':
      if (line[1] != '\0')
        strncpy(jobname, line+1, sizeof(jobname)-1);
      else
        strcpy(jobname, " ");
      continue;

    case 'C':
      if (line[1] != '\0')
        strncpy(Class, line+1, sizeof(Class)-1);
      else if (Class[0] == '\0')
        gethostname(Class, sizeof(Class));
      continue;

    case 'T':  /* header title for pr */
      strncpy(title, line+1, sizeof(title)-1);
      continue;

    case 'L':  /* identification line */
      if (!SH && !HL)
        banner(line+1, jobname);
      continue;

    case '1':  /* troff fonts */
    case '2':
    case '3':
    case '4':
      if (line[1] != '\0')
        strcpy(fonts[line[0]-'1'], line+1);
      continue;

    case 'W':  /* page width */
      strncpy(width+2, line+1, sizeof(width)-3);
      continue;

    case 'I':  /* indent amount */
      strncpy(indent+2, line+1, sizeof(indent)-3);
      continue;

    default:  /* some file to print */
      switch (i = print(line[0], line+1)) {
      case ERROR:
        if (bombed == OK)
          bombed = FATALERR;
        break;
      case REPRINT:
        (void) fclose(cfp);
        return(REPRINT);
      case FILTERERR:
      case ACCESS:
        bombed = i;
        sendmail(logname, bombed);
      }
      title[0] = '\0';
      continue;

    case 'N':
    case 'U':
    case 'M':
      continue;
    }

  /* pass 2 */

pass2:
  fseek(cfp, 0L, 0);
  while (getline(cfp))
    switch (line[0]) {
    case 'L':  /* identification line */
      if (!SH && HL)
        banner(line+1, jobname);
      continue;

    case 'M':
      if (bombed < NOACCT)  /* already sent if >= NOACCT */
        sendmail(line+1, bombed);
      continue;

    case 'U':
      (void) unlink(line+1);
    }
  /*
   * clean-up in case another control file exists
   */
  (void) fclose(cfp);
  (void) unlink(file);
  return(bombed == OK ? OK : ERROR);
}

/*
 * Print a file.
 * Set up the chain [ PR [ | {IF, OF} ] ] or {IF, RF, TF, NF, DF, CF, VF}.
 * Return -1 if a non-recoverable error occured,
 * 2 if the filter detected some errors (but printed the job anyway),
 * 1 if we should try to reprint this job and
 * 0 if all is well.
 * Note: all filters take stdin as the file, stdout as the printer,
 * stderr as the log file, and must not ignore SIGINT.
 */
print(format, file)
  int format;
  char *file;
{
  register int n;
  register char *prog;
  int fi, fo;
  char *av[15], buf[BUFSIZ];
  int pid, p[2], stopped = 0;
  union wait status;
  struct stat stb;

  if (lstat(file, &stb) < 0 || (fi = open(file, O_RDONLY)) < 0)
    return(ERROR);
  /*
   * Check to see if data file is a symbolic link. If so, it should
   * still point to the same file or someone is trying to print
   * something he shouldn't.
   */
  if ((stb.st_mode & S_IFMT) == S_IFLNK && fstat(fi, &stb) == 0 &&
      (stb.st_dev != fdev || stb.st_ino != fino))
    return(ACCESS);
  if (!SF && !tof) {    /* start on a fresh page */
    (void) write(ofd, FF, strlen(FF));
    tof = 1;
  }
  if (IF == NULL && (format == 'f' || format == 'l')) {
    tof = 0;
    while ((n = read(fi, buf, BUFSIZ)) > 0)
      if (write(ofd, buf, n) != n) {
        (void) close(fi);
        return(REPRINT);
      }
    (void) close(fi);
    return(OK);
  }
  switch (format) {
  case 'p':  /* print file using 'pr' */
    if (IF == NULL) {  /* use output filter */
      prog = PR;
      av[0] = "pr";
      av[1] = width;
      av[2] = length;
      av[3] = "-h";
      av[4] = *title ? title : " ";
      av[5] = 0;
      fo = ofd;
      goto start;
    }
    pipe(p);
    if ((prchild = dofork(DORETURN)) == 0) {  /* child */
      dup2(fi, 0);    /* file is stdin */
      dup2(p[1], 1);    /* pipe is stdout */
      for (n = 3; n < NOFILE; n++)
        (void) close(n);
      execl(PR, "pr", width, length, "-h", *title ? title : " ", 0);
      syslog(LOG_ERR, "cannot execl %s", PR);
      exit(2);
    }
    (void) close(p[1]);    /* close output side */
    (void) close(fi);
    if (prchild < 0) {
      prchild = 0;
      (void) close(p[0]);
      return(ERROR);
    }
    fi = p[0];      /* use pipe for input */
  case 'f':  /* print plain text file */
    prog = IF;
    av[1] = width;
    av[2] = length;
    av[3] = indent;
    n = 4;
    break;
  case 'l':  /* like 'f' but pass control characters */
    prog = IF;
    av[1] = "-c";
    av[2] = width;
    av[3] = length;
    av[4] = indent;
    n = 5;
    break;
  case 'r':  /* print a fortran text file */
    prog = RF;
    av[1] = width;
    av[2] = length;
    n = 3;
    break;
  case 't':  /* print troff output */
  case 'n':  /* print ditroff output */
  case 'd':  /* print tex output */
    (void) unlink(".railmag");
    if ((fo = creat(".railmag", FILMOD)) < 0) {
      syslog(LOG_ERR, "%s: cannot create .railmag", printer);
      (void) unlink(".railmag");
    } else {
      for (n = 0; n < 4; n++) {
        if (fonts[n][0] != '/')
          (void) write(fo, "/usr/lib/vfont/", 15);
        (void) write(fo, fonts[n], strlen(fonts[n]));
        (void) write(fo, "\n", 1);
      }
      (void) close(fo);
    }
    prog = (format == 't') ? TF : (format == 'n') ? NF : DF;
    av[1] = pxwidth;
    av[2] = pxlength;
    n = 3;
    break;
  case 'c':  /* print cifplot output */
    prog = CF;
    av[1] = pxwidth;
    av[2] = pxlength;
    n = 3;
    break;
  case 'g':  /* print plot(1G) output */
    prog = GF;
    av[1] = pxwidth;
    av[2] = pxlength;
    n = 3;
    break;
  case 'v':  /* print raster output */
    prog = VF;
    av[1] = pxwidth;
    av[2] = pxlength;
    n = 3;
    break;
  default:
    (void) close(fi);
    syslog(LOG_ERR, "%s: illegal format character '%c'",
      printer, format);
    return(ERROR);
  }
  if ((av[0] = rindex(prog, '/')) != NULL)
    av[0]++;
  else
    av[0] = prog;
  av[n++] = "-n";
  av[n++] = logname;
  av[n++] = "-h";
  av[n++] = fromhost;
  av[n++] = AF;
  av[n] = 0;
  fo = pfd;
  if (ofilter > 0) {    /* stop output filter */
    write(ofd, "\031\1", 2);
    while ((pid = wait3(&status, WUNTRACED, 0)) > 0 && pid != ofilter)
      ;
    if (status.w_stopval != WSTOPPED) {
      (void) close(fi);
      syslog(LOG_WARNING, "%s: output filter died (%d)",
        printer, status.w_retcode);
      return(REPRINT);
    }
    stopped++;
  }
start:
  if ((child = dofork(DORETURN)) == 0) {  /* child */
    dup2(fi, 0);
    dup2(fo, 1);
    n = open(tempfile, O_WRONLY|O_CREAT|O_TRUNC, 0664);
    if (n >= 0)
      dup2(n, 2);
    for (n = 3; n < NOFILE; n++)
      (void) close(n);
    execv(prog, av);
    syslog(LOG_ERR, "cannot execv %s", prog);
    exit(2);
  }
  (void) close(fi);
  if (child < 0)
    status.w_retcode = 100;
  else
    while ((pid = wait(&status)) > 0 && pid != child)
      ;
  child = 0;
  prchild = 0;
  if (stopped) {    /* restart output filter */
    if (kill(ofilter, SIGCONT) < 0) {
      syslog(LOG_ERR, "cannot restart output filter");
      exit(1);
    }
  }
  tof = 0;
  if (!WIFEXITED(status)) {
    syslog(LOG_WARNING, "%s: Daemon filter '%c' terminated (%d)",
      printer, format, status.w_termsig);
    return(ERROR);
  }
  switch (status.w_retcode) {
  case 0:
    tof = 1;
    return(OK);
  case 1:
    return(REPRINT);
  default:
    syslog(LOG_WARNING, "%s: Daemon filter '%c' exited (%d)",
      printer, format, status.w_retcode);
  case 2:
    return(ERROR);
  }
}
#endif

/*
 * Send the daemon control file (cf) and any data files.
 * Return -1 if a non-recoverable error occured, 1 if a recoverable error and
 * 0 if all is well.
 */
int sendit(char *file) 
{
  register int i, err = OK;
  char *cp, last[BUFSIZ];
  int sendfile(char, char *) ;

  /*
   * open control file
   */
  if ((cfp = fopen(file, "r")) == NULL)
    return(OK);
  /*
   *      read the control file for work to do
   *
   *      file format -- first character in the line is a command
   *      rest of the line is the argument.
   *      commands of interest are:
   *
   *            a-z -- "file name" name of file to print
   *              U -- "unlink" name of file to remove
   *                    (after we print it. (Pass 2 only)).
   */

  /*
   * pass 1
   */
  while (getline(cfp)) {
  again:
    if (line[0] == 'S') {
      cp = line+1;
      i = 0;
      while (*cp >= '0' && *cp <= '9')
        i = i * 10 + (*cp++ - '0');
      fdev = i;
      cp++;
      i = 0;
      while (*cp >= '0' && *cp <= '9')
        i = i * 10 + (*cp++ - '0');
      fino = i;
      continue;
    }
    if (line[0] >= 'a' && line[0] <= 'z') {
      strcpy(last, line);
      while ((i = getline(cfp)) != NULL)
        if (strcmp(last, line))
          break;
      switch (sendfile('\3', last+1)) {
      case OK:
        if (i)
          goto again;
        break;
      case REPRINT:
        (void) fclose(cfp);
        return(REPRINT);
#ifndef __HELIOS      
      case ACCESS:
        sendmail(logname, ACCESS);
#endif
      case ERROR:
        err = ERROR;
      }
      break;
    }
  }
  if (err == OK && sendfile('\2', file) > 0) {
    (void) fclose(cfp);
    return(REPRINT);
  }
  /*
   * pass 2
   */
  fseek(cfp, 0L, 0);
  while (getline(cfp))
    if (line[0] == 'U')
      (void) unlink(line+1);
  /*
   * clean-up in case another control file exists
   */
  (void) fclose(cfp);
  (void) unlink(file);
  return(err);
}

/*
 * Send a data file to the remote machine and spool it.
 * Return positive if we should try resending.
 */
int sendfile(char type, char *file)
{
  register int f, i, amt;
  struct stat stb;
  char buf[BUFSIZ];
  int sizerr, resp;
  
  char response(void) ;

  if (lstat(file, &stb) < 0 || (f = open(file, O_RDONLY)) < 0)
    return(ERROR);
#ifndef __HELIOS
  /*
   * Check to see if data file is a symbolic link. If so, it should
   * still point to the same file or someone is trying to print something
   * he shouldn't.
   */
  if ((stb.st_mode & S_IFMT) == S_IFLNK && fstat(f, &stb) == 0 &&
      (stb.st_dev != fdev || stb.st_ino != fino))
    return(ACCESS);
#else
  debugf ("printjob: skipped symbolic stuff") ;
#endif    
  (void) sprintf(buf, "%c%ld %s\n", type, stb.st_size, file);
  amt = strlen(buf);
  for (i = 0;  ; i++) {
    if (write(pfd, buf, amt) != amt ||
        (resp = response()) < 0 || resp == '\1') {
      (void) close(f);
      return(REPRINT);
    } else if (resp == '\0')
      break;
    if (i == 0)
      status("no space on remote; waiting for queue to drain");
    if (i == 10)
      syslog(LOG_ALERT, "%s: can't send to %s; queue full",
        printer, RM);
    sleep(5 * 60);
  }
  if (i)
    status("sending to %s", RM);
  sizerr = 0;
  for (i = 0; i < stb.st_size; i += BUFSIZ) {
/*
-- crf : buf contains junk at end ... NULLify it
*/
    memset (buf, (char) NULL, sizeof buf) ;
    
    amt = BUFSIZ;
    if (i + amt > (int) stb.st_size)
      amt = (int) stb.st_size - i;
    if (sizerr == 0 && read(f, buf, amt) != amt)
      sizerr = 1;
    if (write(pfd, buf, amt) != amt) {
      (void) close(f);
      return(REPRINT);
    }
  }
  (void) close(f);
  if (sizerr) {
    syslog(LOG_INFO, "%s: %s: changed size", printer, file);
    /* tell recvjob to ignore this file */
    (void) write(pfd, "\1", 1);
    return(ERROR);
  }
  if (write(pfd, "", 1) != 1 || response())
    return(REPRINT);
  return(OK);
}

/*
 * Check to make sure there have been no errors and that both programs
 * are in sync with eachother.
 * Return non-zero if the connection was lost.
 */
char response()
{
  char resp;

  if (read(pfd, &resp, 1) != 1) {
    syslog(LOG_INFO, "%s: lost connection", printer);
    return(255);
  }
  return(resp);
}

#ifndef __HELIOS
/*
 * Banner printing stuff
 */
banner(name1, name2)
  char *name1, *name2;
{
  time_t tvec;
  extern char *ctime();

  time(&tvec);
  if (!SF && !tof)
    (void) write(ofd, FF, strlen(FF));
  if (SB) {  /* short banner only */
    if (Class[0]) {
      (void) write(ofd, Class, strlen(Class));
      (void) write(ofd, ":", 1);
    }
    (void) write(ofd, name1, strlen(name1));
    (void) write(ofd, "  Job: ", 7);
    (void) write(ofd, name2, strlen(name2));
    (void) write(ofd, "  Date: ", 8);
    (void) write(ofd, ctime(&tvec), 24);
    (void) write(ofd, "\n", 1);
  } else {  /* normal banner */
    (void) write(ofd, "\n\n\n", 3);
    scan_out(ofd, name1, '\0');
    (void) write(ofd, "\n\n", 2);
    scan_out(ofd, name2, '\0');
    if (Class[0]) {
      (void) write(ofd,"\n\n\n",3);
      scan_out(ofd, Class, '\0');
    }
    (void) write(ofd, "\n\n\n\n\t\t\t\t\tJob:  ", 15);
    (void) write(ofd, name2, strlen(name2));
    (void) write(ofd, "\n\t\t\t\t\tDate: ", 12);
    (void) write(ofd, ctime(&tvec), 24);
    (void) write(ofd, "\n", 1);
  }
  if (!SF)
    (void) write(ofd, FF, strlen(FF));
  tof = 1;
}

char *
scnline(key, p, c)
  register char key, *p;
  char c;
{
  register scnwidth;

  for (scnwidth = WIDTH; --scnwidth;) {
    key <<= 1;
    *p++ = key & 0200 ? c : BACKGND;
  }
  return (p);
}

#define TRC(q)  (((q)-' ')&0177)

scan_out(scfd, scsp, dlm)
  int scfd;
  char *scsp, dlm;
{
  register char *strp;
  register nchrs, j;
  char outbuf[LINELEN+1], *sp, c, cc;
  int d, scnhgt;
  extern char scnkey[][HEIGHT];  /* in lpdchar.c */

  for (scnhgt = 0; scnhgt++ < HEIGHT+DROP; ) {
    strp = &outbuf[0];
    sp = scsp;
    for (nchrs = 0; ; ) {
      d = dropit(c = TRC(cc = *sp++));
      if ((!d && scnhgt > HEIGHT) || (scnhgt <= DROP && d))
        for (j = WIDTH; --j;)
          *strp++ = BACKGND;
      else
        strp = scnline(scnkey[c][scnhgt-1-d], strp, cc);
      if (*sp == dlm || *sp == '\0' || nchrs++ >= PW/(WIDTH+1)-1)
        break;
      *strp++ = BACKGND;
      *strp++ = BACKGND;
    }
    while (*--strp == BACKGND && strp >= outbuf)
      ;
    strp++;
    *strp++ = '\n';  
    (void) write(scfd, outbuf, strp-outbuf);
  }
}

dropit(c)
  char c;
{
  switch(c) {

  case TRC('_'):
  case TRC(';'):
  case TRC(','):
  case TRC('g'):
  case TRC('j'):
  case TRC('p'):
  case TRC('q'):
  case TRC('y'):
    return (DROP);

  default:
    return (0);
  }
}

/*
 * sendmail ---
 *   tell people about job completion
 */
sendmail(user, bombed)
  char *user;
  int bombed;
{
  register int i;
  int p[2], s;
  register char *cp;
  char buf[100];
  struct stat stb;
  FILE *fp;

  pipe(p);
  if ((s = dofork(DORETURN)) == 0) {    /* child */
    dup2(p[0], 0);
    for (i = 3; i < NOFILE; i++)
      (void) close(i);
    if ((cp = rindex(MAIL, '/')) != NULL)
      cp++;
    else
      cp = MAIL;
    sprintf(buf, "%s@%s", user, fromhost);
    execl(MAIL, cp, buf, 0);
    exit(0);
  } else if (s > 0) {        /* parent */
    dup2(p[1], 1);
    printf("To: %s@%s\n", user, fromhost);
    printf("Subject: printer job\n\n");
    printf("Your printer job ");
    if (*jobname)
      printf("(%s) ", jobname);
    switch (bombed) {
    case OK:
      printf("\ncompleted successfully\n");
      break;
    default:
    case FATALERR:
      printf("\ncould not be printed\n");
      break;
    case NOACCT:
      printf("\ncould not be printed without an account on %s\n", host);
      break;
    case FILTERERR:
      if (stat(tempfile, &stb) < 0 || stb.st_size == 0 ||
          (fp = fopen(tempfile, "r")) == NULL) {
        printf("\nwas printed but had some errors\n");
        break;
      }
      printf("\nwas printed but had the following errors:\n");
      while ((i = getc(fp)) != EOF)
        putchar(i);
      (void) fclose(fp);
      break;
    case ACCESS:
      printf("\nwas not printed because it was not linked to the original file\n");
    }
    fflush(stdout);
    (void) close(1);
  }
  (void) close(p[0]);
  (void) close(p[1]);
  wait(&s);
}

/*
 * dofork - fork with retries on failure
 */
dofork(action)
  int action;
{
  register int i, pid;

  for (i = 0; i < 20; i++) {
    if ((pid = fork()) < 0) {
      sleep((unsigned)(i*i));
      continue;
    }
    /*
     * Child should run as daemon instead of root
     */
    if (pid == 0)
      setuid(DU);
    return(pid);
  }
  syslog(LOG_ERR, "can't fork");

  switch (action) {
  case DORETURN:
    return (-1);
  default:
    syslog(LOG_ERR, "bad action (%d) to dofork", action);
    /*FALL THRU*/
  case DOABORT:
    exit(1);
  }
  /*NOTREACHED*/
}
#endif 

/*
 * Kill child processes to abort current job.
 */
void abortpr()
{
  debugf ("printjob: entering abortpr()") ;
#ifndef __HELIOS
  (void) unlink(tempfile);
  kill(0, SIGINT);
  if (ofilter > 0)
    kill(ofilter, SIGCONT);
#endif    
  while (wait(0) > 0)
    ;
  debugf ("printjob: exiting abortpr()") ;
  
#ifdef __HELIOS
  f_lock_exit() ;
#endif

  exit(0);
}

void init()
{
  int status;

  if ((status = pgetent(line, printer)) < 0) {
    syslog(LOG_ERR, "can't open printer description file");
    exit(1);
  } else if (status == 0) {
    syslog(LOG_ERR, "unknown printer: %s", printer);
    exit(1);
  }
  if ((LP = pgetstr("lp", &bp)) == NULL)
    LP = DEFDEVLP;
  if ((RP = pgetstr("rp", &bp)) == NULL)
    RP = DEFLP;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  if ((ST = pgetstr("st", &bp)) == NULL)
    ST = DEFSTAT;
  if ((LF = pgetstr("lf", &bp)) == NULL)
    LF = DEFLOGF;
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((DU = pgetnum("du")) < 0)
    DU = DEFUID;
  if ((FF = pgetstr("ff", &bp)) == NULL)
    FF = DEFFF;
  if ((PW = pgetnum("pw")) < 0)
    PW = DEFWIDTH;
  sprintf(&width[2], "%d", PW);
  if ((PL = pgetnum("pl")) < 0)
    PL = DEFLENGTH;
  sprintf(&length[2], "%d", PL);
  if ((PX = pgetnum("px")) < 0)
    PX = 0;
  sprintf(&pxwidth[2], "%d", PX);
  if ((PY = pgetnum("py")) < 0)
    PY = 0;
  sprintf(&pxlength[2], "%d", PY);
  RM = pgetstr("rm", &bp);

  debugf ("printjob: RM = %s", RM) ;

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
        syslog(LOG_ERR,
#ifndef __HELIOS
      "unable to get network name for local machine %s",
      name);
#else
      "%s %s", ERR_LOCAL_NAME, name);
#endif
        goto localcheck_done;
    } else strcpy(name, hp->h_name);

    /* get the standard network name of RM */
    hp = gethostbyname(RM);
    if (hp == (struct hostent *) NULL) {
        syslog(LOG_ERR,
#ifndef __HELIOS
      "unable to get hostname for remote machine %s", RM);
#else
      "%s %s", ERR_REMOTE_NAME, RM);
#endif
        goto localcheck_done;
    }

    /* if printer is not on local machine, ignore LP */
    if (strcmp(name, hp->h_name) != 0) *LP = '\0';
  }

localcheck_done:

  AF = pgetstr("af", &bp);
  OF = pgetstr("of", &bp);
  IF = pgetstr("if", &bp);
  RF = pgetstr("rf", &bp);
  TF = pgetstr("tf", &bp);
  NF = pgetstr("nf", &bp);
  DF = pgetstr("df", &bp);
  GF = pgetstr("gf", &bp);
  VF = pgetstr("vf", &bp);
  CF = pgetstr("cf", &bp);
  TR = pgetstr("tr", &bp);
  RS = pgetflag("rs");
  SF = pgetflag("sf");
  SH = pgetflag("sh");
  SB = pgetflag("sb");
  HL = pgetflag("hl");
  RW = pgetflag("rw");
  BR = pgetnum("br");
  if ((FC = pgetnum("fc")) < 0)
    FC = 0;
  if ((FS = pgetnum("fs")) < 0)
    FS = 0;
  if ((XC = pgetnum("xc")) < 0)
    XC = 0;
  if ((XS = pgetnum("xs")) < 0)
    XS = 0;
  tof = !pgetflag("fo");
}

/*
 * Acquire line printer or remote connection.
 */
void openpr()
{
  register int i, n;
  int resp;

  if (*LP) {
#ifndef __HELIOS
    for (i = 1; ; i = i < 32 ? i << 1 : i) {
      pfd = open(LP, RW ? O_RDWR : O_WRONLY);
      if (pfd >= 0)
        break;
      if (errno == ENOENT) {
        syslog(LOG_ERR, "%s: %m", LP);
        exit(1);
      }
      if (i == 1)
        status("waiting for %s to become ready (offline ?)", printer);
      sleep(i);
    }
    if (isatty(pfd))
      setty();
    status("%s is ready and printing", printer);
#else
    print_fatal ("invalid output device: %s", LP) ;
#endif    
  }
#ifndef __HELIOS
  } else if (RM != NULL) {
#else
  if (RM != NULL) {
#endif    
    for (i = 1; ; i = i < 256 ? i << 1 : i) {
      resp = -1;
      pfd = getport(RM);
      if (pfd >= 0) {
        (void) sprintf(line, "\2%s\n", RP);
        n = strlen(line);
        if (write(pfd, line, n) == n &&
            (resp = response()) == '\0')
          break;
        (void) close(pfd);
      }
      if (i == 1) {
        if (resp < 0)
          status("waiting for %s to come up", RM);
        else {
          status("waiting for queue to be enabled on %s", RM);
#ifndef __HELIOS
          i = 256;
#else
          syslog(LOG_ERR, "%s unable to access remote printer", from) ;
          exit(1);
#endif
        }
      }
      sleep(i);
    }
    status("sending to %s", RM);
    remote = 1;
  } else {
    syslog(LOG_ERR, "%s: no line printer device or host name",
      printer);
    exit(1);
  }

  /*
   * Start up an output filter, if needed.
   */
#ifndef __HELIOS   
  if (OF) {
    int p[2];
    char *cp;

    pipe(p);
    if ((ofilter = dofork(DOABORT)) == 0) {  /* child */
      dup2(p[0], 0);    /* pipe is std in */
      dup2(pfd, 1);    /* printer is std out */
      for (i = 3; i < NOFILE; i++)
        (void) close(i);
      if ((cp = rindex(OF, '/')) == NULL)
        cp = OF;
      else
        cp++;
      execl(OF, cp, width, length, 0);
      syslog(LOG_ERR, "%s: %s: %m", printer, OF);
      exit(1);
    }
    (void) close(p[0]);    /* close input side */
    ofd = p[1];      /* use pipe for output */
  } else {
    ofd = pfd;
    ofilter = 0;
  }
#else
  if (OF)
    debugf ("printjob: output filters NOT implemented") ;
  ofd = pfd;
#endif    
}

#ifndef __HELIOS
struct bauds {
  int  baud;
  int  speed;
} bauds[] = {
  50,  B50,
  75,  B75,
  110,  B110,
  134,  B134,
  150,  B150,
  200,  B200,
  300,  B300,
  600,  B600,
  1200,  B1200,
  1800,  B1800,
  2400,  B2400,
  4800,  B4800,
  9600,  B9600,
  19200,  EXTA,
  38400,  EXTB,
  0,  0
};

/*
 * setup tty lines.
 */
setty()
{
  register struct bauds *bp;

#ifdef USG
  struct termio tt;

  /*
   * set terminal modes; (THIS HAS NOT BEEN TESTED)
   *
   * should simulate BSD sgtty and local flags (32 in all), to
   * facilitate use of BSD printcap files.  But as we don't use
   * locally attatched printers on any of our SGI boxes, I just
   * stuck in the modes set by transcript -- your mileage may
   * vary.
   */

  if (ioctl(pfd, TCGETA, (char *)&tt) < 0) {
    syslog(LOG_ERR, "%s: ioctl(TCGETA): %m", printer);
    exit(1);
  }

  tt.c_iflag &= ~(IGNBRK|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IUCLC|IXANY);
  tt.c_iflag |= BRKINT|INPCK|IXON;

  tt.c_oflag |= OPOST;

  tt.c_lflag &= ~(ISIG|ICANON|XCASE);

  tt.c_cflag &= ~(CSIZE|CLOCAL);
  tt.c_cflag |= CS8|CREAD;

  tt.c_cc[VMIN] = 1;
  tt.c_cc[VTIME] = 4;

  if (BR > 0) {
    for (bp = bauds; bp->baud; bp++)
      if (BR == bp->baud)
        break;
    if (!bp->baud) {
      syslog(LOG_ERR, "%s: illegal baud rate %d", printer, BR);
      exit(1);
    }
    tt.c_cflag &= ~CBAUD;
    tt.c_cflag |= bp->speed;
  }
  if (ioctl(pfd, TCSETA, (char *)&tt) < 0) {
    syslog(LOG_ERR, "%s: ioctl(TCSETA): %m", printer);
    exit(1);
  }

#else
  struct sgttyb ttybuf;

  if (ioctl(pfd, TIOCEXCL, (char *)0) < 0) {
    syslog(LOG_ERR, "%s: ioctl(TIOCEXCL): %m", printer);
    exit(1);
  }
  if (ioctl(pfd, TIOCGETP, (char *)&ttybuf) < 0) {
    syslog(LOG_ERR, "%s: ioctl(TIOCGETP): %m", printer);
    exit(1);
  }
  if (BR > 0) {
    for (bp = bauds; bp->baud; bp++)
      if (BR == bp->baud)
        break;
    if (!bp->baud) {
      syslog(LOG_ERR, "%s: illegal baud rate %d", printer, BR);
      exit(1);
    }
    ttybuf.sg_ispeed = ttybuf.sg_ospeed = bp->speed;
  }
  ttybuf.sg_flags &= ~FC;
  ttybuf.sg_flags |= FS;
  if (ioctl(pfd, TIOCSETP, (char *)&ttybuf) < 0) {
    syslog(LOG_ERR, "%s: ioctl(TIOCSETP): %m", printer);
    exit(1);
  }
#ifdef TIOCSETD
  if (XC || XS) {
    int ldisc = NTTYDISC;

    if (ioctl(pfd, TIOCSETD, &ldisc) < 0) {
      syslog(LOG_ERR, "%s: ioctl(TIOCSETD): %m", printer);
      exit(1);
    }
  }
#endif
#ifdef TIOCLBIC
  if (XC) {
    if (ioctl(pfd, TIOCLBIC, &XC) < 0) {
      syslog(LOG_ERR, "%s: ioctl(TIOCLBIC): %m", printer);
      exit(1);
    }
  }
#endif
#ifdef TIOCLBIS
  if (XS) {
    if (ioctl(pfd, TIOCLBIS, &XS) < 0) {
      syslog(LOG_ERR, "%s: ioctl(TIOCLBIS): %m", printer);
      exit(1);
    }
  }
#endif
#endif
}
#endif 

#ifndef __HELIOS
/*VARARGS1*/
status(msg, a1, a2, a3)
  char *msg;
{
  register int fd;
  char buf[BUFSIZ];

  umask(0);
  fd = open(ST, O_WRONLY|O_CREAT, 0664);
  if (fd < 0 || flock(fd, LOCK_EX) < 0) {
    syslog(LOG_ERR, "%s: %s: %m", printer, ST);
    exit(1);
  }
  ftruncate(fd, 0);
  sprintf(buf, msg, a1, a2, a3);
  strcat(buf, "\n");
  (void) write(fd, buf, strlen(buf));
  (void) close(fd);
}
#else
void status (char *format, ...)
{
  va_list args;

  register int fd;
  char buf[BUFSIZ];

  va_start (args, format);

  umask(0);

#ifndef __HELIOS
  fd = open(ST, O_WRONLY|O_CREAT, 0664);
  if (fd < 0 || flock(fd, LOCK_EX) < 0) {
#else  
  fd = open(ST, O_WRONLY|O_CREAT|O_TRUNC, 0664);
  if (fd < 0 || f_lock(fd, LOCK_EX) < 0) {
#endif  
    syslog(LOG_ERR, "%s: %s: %m", printer, ST);
    exit(1);
  }

#ifndef __HELIOS
  ftruncate(fd, 0);
#else
  debugf ("printjob: implicit ftruncate()") ;
#endif  
  vsprintf (buf, format, args);
  strcat(buf, "\n");
  (void) write(fd, buf, strlen(buf));
  (void) close(fd);

#ifdef __HELIOS
  (void) f_lock(fd, LOCK_UN) ;
#endif

  va_end (args);
}
#endif

#ifdef __HELIOS
/*
-- crf: this is getting a bit messy. I only want to lock the error file if
-- 1. it is NOT the controlling terminal (i.e. default)
-- 2. it was successfully created (fd = 2)
-- Then, I only want to unlock the file if it was locked in the first place
-- I'm not entirely happy with the way error logging is handled in general ...
*/
int locked ;
#define F_LOCK_LF \
if ((locked = (strcmp (LF, DEFLOGF)) && (LF_fd == 2)) != 0) \
  if (f_lock(LF_fd, LOCK_EX) < 0) \
    syslog (LOG_ERR, "failed to lock error file: %s", LF)

#define F_UNLOCK_LF \
if (locked) \
  if (f_lock(LF_fd, LOCK_UN) < 0) \
    syslog(LOG_ERR, "failed to unlock error file: %s", LF)

void check_filter (char *filter_value, char *filter_name)
{
  if (filter_value)
  {
    F_LOCK_LF ;
    fprintf (stderr, "%s: ", name);
    if (printer) fprintf (stderr, "%s: ", printer);
    fprintf (stderr, "printcap filter specification (%s=%s) ignored\n", 
             filter_name, filter_value) ;
    F_UNLOCK_LF ;
    fflush(stderr);
  }
}
#endif

#ifdef __HELIOS
/*
-- crf: Note - this is a modification of the fatal() in common.c
*/
void print_fatal (char *format, ...)
{
  va_list args;
  
  debugf ("printjob: entering print_fatal()") ;
  va_start (args, format);
  
  F_LOCK_LF ;
  if (from != host) fprintf (stderr, "%s: ", host);
  fprintf (stderr, "%s: ", name);
  if (printer) fprintf (stderr, "%s: ", printer);
  vfprintf (stderr, format, args);
  fputc('\n', stderr);
  va_end (args);
  F_UNLOCK_LF ;
  fflush(stderr);
  exit(1);
}
#endif
