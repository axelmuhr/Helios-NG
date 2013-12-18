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
"@(#) Copyright (c) 1983, 1989 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lpr.c	5.5 (Berkeley) 12/21/88";
#endif /* not lint */
#elif defined __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/lpr.c,v 1.4 1994/03/17 17:07:59 nickc Exp $";
#endif

/*
 *      lpr -- off line print
 *
 * Allows multiple printers and printers on remote machines by
 * using information from a printer data base.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <ctype.h>
#include <syslog.h>

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#ifndef __HELIOS
#include "lp.local.h"
#else
#include "lp_local.h"
#include <stdarg.h>
#include <module.h>
#include "printcap.h"
#endif

char    *tfname;		/* tmp copy of cf before linking */
char    *cfname;		/* daemon control files, linked from tf's */
char    *dfname;		/* data files */

int	nact;			/* number of jobs to act on */
int	tfd;			/* control file descriptor */
int     mailflg;		/* send mail */
int	qflag;			/* q job, but don't exec daemon */
char	format = 'f';		/* format char for printing files */
int	rflag;			/* remove files upon completion */	
#ifndef __HELIOS
int	sflag;			/* symbolic link flag */
#endif
int	inchar;			/* location to increment char in file names */
int     ncopies = 1;		/* # of copies to make */
int	iflag;			/* indentation wanted */
int	indent;			/* amount to indent */
int	hdr = 1;		/* print header or not (default is yes) */
int     userid;			/* user id */
char	*person;		/* user name */
char	*title;			/* pr'ing title */
char	*fonts[4];		/* troff font names */
char	*width;			/* width for versatec printing */
char	host[32];		/* host name */
char	*Class = host;		/* class title on header page */
char    *jobname;		/* job name on header page */
char	*name;			/* program name */
char	*printer;		/* printer name */
struct	stat statb;

int	MX;			/* maximum number of blocks to copy */
int	MC;			/* maximum number of copies allowed */
int	DU;			/* daemon user-id */
char	*SD;			/* spool directory */
char	*LO;			/* lock file name */
char	*RG;			/* restrict group */
short	SC;			/* suppress multiple copies */

#ifndef __HELIOS
char	*getenv();
char	*rindex();
char	*linked();
int	cleanup();
#else
void	cleanup (void) ;
void	mktemps (void) ;
void	chkprinter (char *) ;
char	*itoa (register int) ;
int	nfile (char *) ;
void	card (register char, char *) ;
void	copy (int, char []) ;
int	test (char *) ;

extern int startdaemon (char *) ;
#endif

void fatal (char *, ...) ;

/*ARGSUSED*/
int main(int argc, char *argv[])
{
#ifndef __HELIOS
  extern struct passwd *getpwuid();
  struct group *gptr, *getgrnam();
  extern char *itoa();
  register char *arg, *cp;
  char buf[BUFSIZ];
  int i, f;
  struct stat stb;
#else
  struct passwd *pw;
  struct group *gptr;
  register char *arg ;
  char buf[BUFSIZ];
  int i, f;
#endif  

#ifndef __HELIOS
  if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
    signal(SIGHUP, cleanup);
  if (signal(SIGINT, SIG_IGN) != SIG_IGN)
    signal(SIGINT, cleanup);
  if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
    signal(SIGQUIT, cleanup);
  if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
    signal(SIGTERM, cleanup);
#else
  debugf ("using sigaction()") ;
  {
    struct sigaction act;
    act.sa_handler = cleanup;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    if (signal(SIGINT, SIG_IGN) != SIG_IGN)    
      (void) sigaction(SIGINT, &act, NULL);
    if (signal(SIGHUP, SIG_IGN) != SIG_IGN)    
      (void) sigaction(SIGHUP, &act, NULL);
    if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)    
      (void) sigaction(SIGQUIT, &act, NULL);
    if (signal(SIGTERM, SIG_IGN) != SIG_IGN)    
      (void) sigaction(SIGTERM, &act, NULL);
  }
#endif

  name = argv[0];
  gethostname(host, sizeof (host));
  openlog("lpd", 0, LOG_LPR);

  while (argc > 1 && argv[1][0] == '-') {
    argc--;
    arg = *++argv;
    switch (arg[1]) {

    case 'P':    /* specifiy printer name */
      if (arg[2])
        printer = &arg[2];
      else if (argc > 1) {
        argc--;
        printer = *++argv;
      }
      break;

    case 'C':    /* classification spec */
      hdr++;
      if (arg[2])
        Class = &arg[2];
      else if (argc > 1) {
        argc--;
        Class = *++argv;
      }
      break;

    case 'U':    /* user name */
      hdr++;
      if (arg[2])
        person = &arg[2];
      else if (argc > 1) {
        argc--;
        person = *++argv;
      }
      break;

    case 'J':    /* job name */
      hdr++;
      if (arg[2])
        jobname = &arg[2];
      else if (argc > 1) {
        argc--;
        jobname = *++argv;
      }
      break;

    case 'T':    /* pr's title line */
      if (arg[2])
        title = &arg[2];
      else if (argc > 1) {
        argc--;
        title = *++argv;
      }
      break;

    case 'l':    /* literal output */
#ifdef __HELIOS
      format = arg[1];
      break;
#endif

    case 'p':    /* print using ``pr'' */
    case 't':    /* print troff output (cat files) */
    case 'n':    /* print ditroff output */
    case 'd':    /* print tex output (dvi files) */
    case 'g':    /* print graph(1G) output */
    case 'c':    /* print cifplot output */
    case 'v':    /* print vplot output */
      format = arg[1];
#ifdef __HELIOS
    printf("%s: warning - remote filter specification for format '%c' assumed\n",
           name, format) ;
#endif
      break;

    case 'f':    /* print fortran output */
      format = 'r';
#ifdef __HELIOS
    printf("%s: warning - remote filter specification for format '%c' assumed\n",
           name, arg[1]) ;
#endif
      break;

    case '4':    /* troff fonts */
    case '3':
    case '2':
    case '1':
      if (argc > 1) {
        argc--;
        fonts[arg[1] - '1'] = *++argv;
#ifdef __HELIOS
        printf("%s: warning - font file (%s) assumed to reside on remote host\n",
               name, fonts[arg[1] - '1']) ;
#endif
      }
      break;

    case 'w':    /* versatec page width */
      width = arg+2;
      break;

    case 'r':    /* remove file when done */
      rflag++;
      break;

    case 'm':    /* send mail when done */
      mailflg++;
      break;

    case 'h':    /* toggle want of header page */
      hdr = !hdr;
      break;

    case 's':    /* try to link files */
#ifndef __HELIOS    
      sflag++;
#else
      debugf ("-s option not supported") ;
      printf("%s: -%c option ignored\n", name, arg[1]);
#endif      
      break;

    case 'q':    /* just q job */
      qflag++;
      break;

    case 'i':    /* indent output */
      iflag++;
      indent = arg[2] ? atoi(&arg[2]) : 8;
      break;

    case '#':    /* n copies */
      if (isdigit(arg[2])) {
        i = atoi(&arg[2]);
        if (i > 0)
          ncopies = i;
      }
    }
  }
  if (printer == NULL && (printer = getenv("PRINTER")) == NULL)
    printer = DEFLP;
  chkprinter(printer);
  if (SC && ncopies > 1)
    fatal("multiple copies are not allowed");
  if (MC > 0 && ncopies > MC)
    fatal("only %d copies are allowed", MC);
  /*
   * Get the identity of the person doing the lpr using the same
   * algorithm as lprm. 
   */
  userid = getuid();
  if (userid != DU || person == 0) {
    if ((pw = getpwuid(userid)) == NULL)
      fatal("Who are you?");
    person = pw->pw_name;
  }
  /*
   * Check for restricted group access.
   */
  if (RG != NULL && userid != DU) {
    if ((gptr = getgrnam(RG)) == NULL)
      fatal("Restricted group specified incorrectly");
    if (gptr->gr_gid != getgid()) {
      while (*gptr->gr_mem != NULL) {
        if ((strcmp(person, *gptr->gr_mem)) == 0)
          break;
        gptr->gr_mem++;
      }
      if (*gptr->gr_mem == NULL)
        fatal("Not a member of the restricted group");
    }
  }
  /*
   * Check to make sure queuing is enabled if userid is not root.
   */

#ifndef __HELIOS
  (void) sprintf(buf, "%s/%s", SD, LO);
  if (userid && stat(buf, &stb) == 0 && (stb.st_mode & 010))
    fatal("Printer queue is disabled");
#else
  debugf ("testing if queue disabled") ;
  (void) sprintf(buf, "%s/%s", SD, DEFMODE);
  {
    int lockmode = get_mode (buf) ;
    if ((userid) && (lockmode > 0) && (lockmode & 010))
      fatal("Printer queue is disabled");
  }
#endif

  /*
   * Initialize the control file.
   */
  mktemps();
  tfd = nfile(tfname);

#ifdef HAVE_FCHOWN
  (void) fchown(tfd, DU, getgid());/* owned by daemon for protection */
#else
  (void) chown(tfname, DU, getgid());
#endif
  debugf ("(chown()) (1)") ;

  card('H', host);
  card('P', person);
  if (hdr) {
    if (jobname == NULL) {
      if (argc == 1)
        jobname = "stdin";
      else
        jobname = ((arg = rindex(argv[1], '/')) != NULL) ? arg+1 : argv[1];
    }
    card('J', jobname);
    card('C', Class);
    card('L', person);
  }
  if (iflag)
    card('I', itoa(indent));
  if (mailflg)
    card('M', person);
  if (format == 't' || format == 'n' || format == 'd')
    for (i = 0; i < 4; i++)
      if (fonts[i] != NULL)
        card('1'+i, fonts[i]);
  if (width != NULL)
    card('W', width);

  /*
   * Read the files and spool them.
   */
  if (argc == 1)
    copy(0, " ");
  else while (--argc) {
    if ((f = test(arg = *++argv)) < 0)
      continue;  /* file unreasonable */

/*
-- crf : symbolic links not supported
*/
#ifndef __HELIOS
    if (sflag && (cp = linked(arg)) != NULL) {
      (void) sprintf(buf, "%d %d", statb.st_dev, statb.st_ino);
      card('S', buf);
      if (format == 'p')
        card('T', title ? title : arg);
      for (i = 0; i < ncopies; i++)
        card(format, &dfname[inchar-2]);
      card('U', &dfname[inchar-2]);
      if (f)
        card('U', cp);
      card('N', arg);
      dfname[inchar]++;
      nact++;
      continue;
    }
    if (sflag)
      printf("%s: %s: not linked, copying instead\n", name, arg);
#endif      
    if ((i = open(arg, O_RDONLY)) < 0) {
      printf("%s: cannot open %s\n", name, arg);
      continue;
    }
    copy(i, arg);
    (void) close(i);
    if (f && unlink(arg) < 0)
      printf("%s: %s: not removed\n", name, arg);
  }

  if (nact) {
    (void) close(tfd);
    tfname[inchar]--;
    /*
     * Touch the control file to fix position in the queue.
     */
    if ((tfd = open(tfname, O_RDWR)) >= 0) {
      char c;

      if (read(tfd, &c, 1) == 1 && lseek(tfd, 0, 0) == 0 &&
          write(tfd, &c, 1) != 1) {
        printf("%s: cannot touch %s\n", name, tfname);
        tfname[inchar]++;
        cleanup();
      }
      (void) close(tfd);
    }
#ifndef __HELIOS
    if (link(tfname, cfname) < 0) {
      printf("%s: cannot rename %s\n", name, cfname);
      tfname[inchar]++;
      cleanup();
    }
#else
    debugf ("rename used instead") ;
    if (rename(tfname, cfname) != 0) {
      printf("%s: cannot rename %s\n", name, cfname);
      tfname[inchar]++;
      cleanup();
    }
#endif
    unlink(tfname);
    if (qflag)    /* just q things up */
      exit(0);
    if (!startdaemon(printer))
      printf("jobs queued, but cannot start daemon.\n");
    exit(0);
  }
  cleanup();
  /* NOTREACHED */
}

/*
 * Create the file n and copy from file descriptor f.
 */
void copy(int f, char n[])
{
  register int fd, i, nr, nc;
  char buf[BUFSIZ];

  if (format == 'p')
    card('T', title ? title : n);
  for (i = 0; i < ncopies; i++)
    card(format, &dfname[inchar-2]);
  card('U', &dfname[inchar-2]);
  card('N', n);
  fd = nfile(dfname);
  nr = nc = 0;
  while ((i = read(f, buf, BUFSIZ)) > 0) {
    if (write(fd, buf, i) != i) {
      printf("%s: %s: temp file write error\n", name, n);
      break;
    }
    nc += i;
    if (nc >= BUFSIZ) {
      nc -= BUFSIZ;
      nr++;
      if (MX > 0 && nr > MX) {
        printf("%s: %s: copy file is too large\n", name, n);
        break;
      }
    }
  }
  (void) close(fd);
  if (nc==0 && nr==0) 
    printf("%s: %s: empty input file\n", name, f ? n : "stdin");
  else
    nact++;
}

/*
-- crf : not supported
*/
#ifndef __HELIOS
/*
 * Try and link the file to dfname. Return a pointer to the full
 * path name if successful.
 */
char *
linked(file)
  register char *file;
{
  register char *cp;
  static char buf[BUFSIZ];

  if (*file != '/') {
    if (getwd(buf) == NULL)
      return(NULL);
    while (file[0] == '.') {
      switch (file[1]) {
      case '/':
        file += 2;
        continue;
      case '.':
        if (file[2] == '/') {
          if ((cp = rindex(buf, '/')) != NULL)
            *cp = '\0';
          file += 3;
          continue;
        }
      }
      break;
    }
    strcat(buf, "/");
    strcat(buf, file);
    file = buf;
  }
  return(symlink(file, dfname) ? NULL : file);
}
#endif

/*
 * Put a line into the control file.
 */
void card (register char c, char *p2)
{
  char buf[BUFSIZ];
  register char *p1 = buf;
  register int len = 2;

  *p1++ = c;
  while ((c = *p2++) != '\0') {
    *p1++ = c;
    len++;
  }
  *p1++ = '\n';
  write(tfd, buf, len);
}

/*
 * Create a new file in the spool directory.
 */
int nfile(char *n)
{
  register f;
  int oldumask = umask(0);    /* should block signals */

  f = creat(n, FILMOD);
  (void) umask(oldumask);
  if (f < 0) {
    printf("%s: cannot create %s\n", name, n);
    cleanup();
  }
#ifdef HAVE_FCHOWN
  if (fchown(f, userid, -1) < 0) {
#else    
  if (chown(n, userid, getegid()) < 0) {
#endif
    debugf ("(chown()) (2)") ;
    printf("%s: cannot chown %s\n", name, n);
    cleanup();
  }
  if (++n[inchar] > 'z') {
    if (++n[inchar-2] == 't') {
      printf("too many files - break up the job\n");
      cleanup();
    }
    n[inchar] = 'A';
  } else if (n[inchar] == '[')
    n[inchar] = 'a';
  return(f);
}

/*
 * Cleanup after interrupts and errors.
 */
void cleanup()
{
  register i;

  signal(SIGHUP, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  i = inchar;
  if (tfname)
    do
      unlink(tfname);
    while (tfname[i]-- != 'A');
  if (cfname)
    do
      unlink(cfname);
    while (cfname[i]-- != 'A');
  if (dfname)
    do {
      do
        unlink(dfname);
      while (dfname[i]-- != 'A');
      dfname[i] = 'z';
    } while (dfname[i-2]-- != 'd');

#ifdef __HELIOS
  f_lock_exit() ;
#endif  

  exit(1);
}

/*
 * Test to see if this is a printable file.
 * Return -1 if it is not, 0 if its printable, and 1 if
 * we should remove it after printing.
 */
int test(char *file)
{
#ifndef __HELIOS
#ifdef mips
  struct aouthdr execb;
#else
  struct exec execb;
#endif
#else
  word exec_word ;
#endif
  register int fd;
  register char *cp;

  if (access(file, 4) < 0) {
    printf("%s: cannot access %s\n", name, file);
    return(-1);
  }
  if (stat(file, &statb) < 0) {
    printf("%s: cannot stat %s\n", name, file);
    return(-1);
  }
  if ((statb.st_mode & S_IFMT) == S_IFDIR) {
    printf("%s: %s is a directory\n", name, file);
    return(-1);
  }
  if (statb.st_size == 0) {
    printf("%s: %s is an empty file\n", name, file);
    return(-1);
   }
  if ((fd = open(file, O_RDONLY)) < 0) {
    printf("%s: cannot open %s\n", name, file);
    return(-1);
  }
#ifndef __HELIOS
  if (read(fd, &execb, sizeof(execb)) == sizeof(execb))
#ifdef mips
    switch(execb.magic) {
    case OMAGIC:
    case NMAGIC:
    case ZMAGIC:
      printf("%s: %s is an executable program", name, file);
      goto error1;

    case LIBMAGIC:
      printf("%s: %s is an archive file", name, file);
      goto error1;
    }
#else
    switch(execb.a_magic) {
    case A_MAGIC1:
    case A_MAGIC2:
    case A_MAGIC3:
#ifdef A_MAGIC4
    case A_MAGIC4:
#endif
      printf("%s: %s is an executable program", name, file);
      goto error1;

    case ARMAG:
      printf("%s: %s is an archive file", name, file);
      goto error1;
    }
#endif
#else
  debugf ("testing for executable ...") ;
  if (read (fd, (char *) &exec_word, sizeof(exec_word)) == sizeof(exec_word))
  switch(exec_word) 
  {
    case Image_Magic:
      printf("%s: %s is an executable program", name, file);
      goto error1;
    case TaskForce_Magic:
      printf("%s: %s is a CDL object file", name, file);
      goto error1;
  }
#endif
  (void) close(fd);
  if (rflag) {
    if ((cp = rindex(file, '/')) == NULL) {
      if (access(".", 2) == 0)
        return(1);
    } else {
      *cp = '\0';
      fd = access(file, 2);
      *cp = '/';
      if (fd == 0)
        return(1);
    }
    printf("%s: %s: is not removable by you\n", name, file);
  }
  return(0);

error1:
  printf(" and is unprintable\n");
  (void) close(fd);
  return(-1);
}

/*
 * itoa - integer to string conversion
 */
char *itoa(register int i)
{
  static char b[10] = "########";
  register char *p;

  p = &b[8];
  do
    *p-- = i%10 + '0';
  while (i /= 10);
  return(++p);
}

/*
 * Perform lookup for printer name or abbreviation --
 */
void chkprinter(char *s) 
{
  int status;
  char buf[BUFSIZ];
  static char pbuf[BUFSIZ/2];
  char *bp = pbuf;
#ifndef __HELIOS
  extern char *pgetstr();
#endif

  if ((status = pgetent(buf, s)) < 0)
    fatal("cannot open printer description file");
  else if (status == 0)
    fatal("%s: unknown printer", s);
  if ((SD = pgetstr("sd", &bp)) == NULL)
    SD = DEFSPOOL;
  if ((LO = pgetstr("lo", &bp)) == NULL)
    LO = DEFLOCK;
  RG = pgetstr("rg", &bp);
  if ((MX = pgetnum("mx")) < 0)
    MX = DEFMX;
  if ((MC = pgetnum("mc")) < 0)
    MC = DEFMAXCOPIES;
  if ((DU = pgetnum("du")) < 0)
    DU = DEFUID;
  SC = pgetflag("sc");
}

/*
 * Make a temp file name.
 */
char *maketemp(char *id, int num)
{
  register char *s;
  int	   len = strlen(SD) + strlen(host) + 8;

  if ((s = (char *) malloc(len)) == NULL)
    fatal("out of memory");
  (void) sprintf(s, "%s/%sA%03d%s", SD, id, num, host);
#ifdef __HELIOS
  /*
   * XXX - NC - 22/3/92 - under MS-DOS we can only have 8
   * character file names. Normally longer file names are
   * silently truncated, but using Novell, this is not so.
   * Ref bug no. 1030.
   */

  /*    spool directory /  cfA 000 hostname */
  len =   strlen(SD)  + 1 + 3 + 3 + 3;
  --len; /* we count from 0 not 1 */
  
  if (s[len] != '\0')
    {
      /* BEFORE:  spool_directory/cfA000hostname */
      
      s[ len     ] = '.';
      s[ len + 4 ] = '\0';

      /* AFTER:   spool_directory/cfA000ho.tna */
    }
#endif
  return(s);
}

/*
 * Make the temp files.
 */
void mktemps()
{
#ifndef __HELIOS
  register int c, len, fd, n;
#else
  register int len, fd, n;
#endif  
  register char *cp;
  char buf[BUFSIZ];
#ifndef __HELIOS
  char *mktemp();
#endif

#ifndef __HELIOS
  (void) sprintf(buf, "%s/.seq", SD);
#else
  (void) sprintf(buf, "%s/seq", SD);
#endif  
  if ((fd = open(buf, O_RDWR|O_CREAT, 0661)) < 0) {
    printf("%s: cannot create %s\n", name, buf);
    exit(1);
  }
#ifndef __HELIOS
  if (flock(fd, LOCK_EX))
#else
  if (f_lock(fd, LOCK_EX)) 
#endif  
    {
  printf("%s: cannot lock %s\n", name, buf);
    exit(1);
  }
  n = 0;
  if ((len = read(fd, buf, sizeof(buf))) > 0) {
    for (cp = buf; len--; ) {
      if (*cp < '0' || *cp > '9')
        break;
      n = n * 10 + (*cp++ - '0');
    }
  }
  tfname = maketemp("tf", n);
  cfname = maketemp("cf", n);
  dfname = maketemp("df", n);
  inchar = strlen(SD) + 3;
  n = (n + 1) % 1000;
  (void) lseek(fd, 0, 0);
  sprintf(buf, "%03d\n", n);
  (void) write(fd, buf, strlen(buf));
  (void) close(fd);  /* unlocks as well */
#ifdef __HELIOS
  (void) f_lock(fd, LOCK_UN) ;
#endif  
}


#ifndef __HELIOS
/*VARARGS1*/
fatal(msg, a1, a2, a3)
  char *msg;
{
  printf("%s: ", name);
  printf(msg, a1, a2, a3);
  putchar('\n');
  exit(1);
}
#else
void fatal (char *format, ...)
{
  va_list args;

  va_start (args, format);
  printf ("%s: ", name);
  vprintf (format, args);
  putchar('\n');
  va_end (args);
  exit(1);
}
#endif
