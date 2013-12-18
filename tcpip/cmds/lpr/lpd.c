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
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/tcpip/cmds/lpr/RCS/lpd.c,v 1.5 1994/03/17 16:53:39 nickc Exp $";
#endif
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


#ifndef __HELIOS
#include "lp.h"
#else
#include <syslib.h>
#include "lpd.h"
#include <sys/hel.h>
#include <unistd.h>
#include <nonansi.h>
#include <stdarg.h>
#include "printcap.h"
#endif

int  lflag = 0;        /* log requests flag */
char  lpd_name [MAX_NAME_LEN] ;
char *name ;
char host [32] ;
char printer [MAX_NAME_LEN] ;

#ifndef __HELIOS
int  reapchild();
int  mcleanup();
#else
void reapchild (void) ;
void mcleanup (void) ;
void fatal (char *, ...) ;
#endif

#ifdef __HELIOS
void wait_for_aux (int) ;
/*
-- crf: execve arguments
*/
char cmd_str [2]   = 
  { '2', (char) NULL } ; /* initialise to invalid command */
char lflag_str [2] = 
  { '0', (char) NULL } ;
char *aux_argv [6] =
  { AUX_FILENAME, cmd_str, host, lflag_str, printer, NULL } ;
#endif

int main(int argc, char **argv)
{

#ifndef __HELIOS
  int f, flocal, finet, options, defreadfds, fromlen;
#ifdef HAVE_UNSOCK
  struct sockaddr_un sun, fromlocal;
#else
  struct sockaddr_in slo, fromlocal;
#endif
  struct sockaddr_in sin, frominet;
  int omask, lfd;
#else
  int f, fhelios, defreadfds, fromlen;
  struct sockaddr_hel socket_addr, fromhelios;
  int lfd ;
  void startup (void) ;
  extern char Version[] ;
#endif

#ifdef MEM_CHECK
  IOdebug ("entering lpd: Bytes free : %d  Heap size : %d", 
            Malloc(-1), Malloc(-3));
#endif

  openlog("lpd", LOG_PID, LOG_LPR);

#ifndef __HELIOS
  gethostname(host, sizeof(host));
#else
/*
-- crf: 09/03/93
-- Exit if gethostname() fails
*/
  if (gethostname(host, sizeof(host)))
  {
    syslog(LOG_ERR, "%s: %m", "gethostname");
    exit(1);
  }
#endif
  name = argv[0];

  while (--argc > 0) {
    argv++;
    if (argv[0][0] == '-')
      switch (argv[0][1]) {
#ifndef __HELIOS
/*
-- crf: not much point in this ...
*/
      case 'd':
        options |= SO_DEBUG;
        break;
#endif        
      case 'l':
        lflag++;
#ifdef __HELIOS
        lflag_str [0] = lflag + '0' ;
#endif        
        break;
      }
  }

#ifndef __HELIOS
#ifndef DEBUG
  /*
   * Set up standard environment by detaching from the parent.
   */
  if (fork())
    exit(0);
  for (f = 0; f < 5; f++)
    (void) close(f);
  (void) open("/dev/null", O_RDONLY);
  (void) open("/dev/null", O_WRONLY);
  (void) dup(1);
  f = open("/dev/tty", O_RDWR);
  if (f > 0) {
    ioctl(f, TIOCNOTTY, 0);
    (void) close(f);
  }
#endif /* DEBUG */
#else
  for (f = 0; f < 5; f++)
    (void) close(f);
  (void) open("/null", O_RDONLY);
  (void) open("/null", O_WRONLY);
  (void) dup(1);
#endif
  (void) umask(0);

#ifdef __HELIOS
/*
-- crf: get task id
*/
  {
    Environ *env = getenviron () ;
    sprintf(lpd_name, "%s", env->Objv [OV_Task]->Name) ;
  }
  syslog(LOG_INFO, "%s (%s) starting", Version, lpd_name);

/*
-- crf: check existing lock file
*/
  {
    int temp_lfd;
    char prev_lpd_name [MAX_NAME_LEN] ;

    if ((temp_lfd = open (MASTERLOCK, O_RDONLY)) >= 0)
    {
      debugf ("lock file exists") ;
      if (read (temp_lfd, prev_lpd_name, MAX_NAME_LEN) >= 0) 
      {
/*
-- crf: get rid of '\n' at end of name
*/      	
#define LAST_CHAR prev_lpd_name [strlen (prev_lpd_name) - 1]
	if (LAST_CHAR == '\n')
          LAST_CHAR = (char) NULL ;
        debugf ("prev. daemon name =  %s", prev_lpd_name) ;
/*
-- crf: it is possible that the current task name will be identical to the 
-- previous one. In this case, the daemon will never start up (i.e. it will 
-- Locate itself and exit) !. Therefore, the current and previous names must 
-- be compared ...
*/        
        if ((prev_lpd_name[0] == '/') && /* valid name ? */
            (strcmp (lpd_name, prev_lpd_name) != 0))
        {
          if (Locate (NULL, prev_lpd_name) != NULL) 
          {
            syslog (LOG_ERR, "already exists (%s)", prev_lpd_name);
            (void) close(temp_lfd);
            exit (0) ; /* daemon already present */
          }
        }
      }
    (void) close(temp_lfd);
    }
  }
#endif

#ifndef __HELIOS
  lfd = open(MASTERLOCK, O_WRONLY|O_CREAT, 0644);
#else  
  lfd = open(MASTERLOCK, O_WRONLY|O_CREAT|O_TRUNC, 0644);
#endif  
  if (lfd < 0) {
    syslog(LOG_ERR, "%s: %m", MASTERLOCK);
    exit(1);
  }

#ifndef __HELIOS
  if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
    if (errno == EWOULDBLOCK)  /* active deamon present */
      exit(0);
    syslog(LOG_ERR, "%s: %m", MASTERLOCK);
    exit(1);
  }
#else
  debugf ("flock() not done (used Locate instead)") ;
#endif  

#ifndef __HELIOS
  ftruncate(lfd, 0);
#else
  debugf ("implicit ftruncate()") ;    
#endif  

  /*
   * write process id for others to know
   */
#ifndef __HELIOS
  sprintf(line, "%u\n", getpid());
  f = strlen(line);
  if (write(lfd, line, f) != f) {
    syslog(LOG_ERR, "%s: %m", MASTERLOCK);
    exit(1);
  }
#else
    strcat (lpd_name, "\n") ;
    f = strlen (lpd_name);
    debugf ("lpd name = %s", lpd_name) ;    
    if (write (lfd, lpd_name, f) != f) {
      syslog (LOG_ERR, "%s: %m", MASTERLOCK);
      exit (1);
    }
#endif

#ifdef __HELIOS
/* 
-- crf: I don't need MASTERLOCK anymore (i.e. I don't depend on the lock
-- status to ascertain whether a daemon is active or not) ... so close it !
-- If I can't close it, its not really serious - so, I'm not generating any 
-- fancy error message.
*/
  if (close (lfd) < 0)
    debugf ("cannot close: %s", MASTERLOCK) ;
#endif

#ifndef __HELIOS
  signal(SIGCHLD, reapchild);
#else
  {
    struct sigaction act;
    act.sa_handler = reapchild;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    (void) sigaction(SIGCHLD, &act, NULL);
  }
#endif

  /*
   * Restart all the printers.
   */
  startup();

/*
-- crf: no longer need printer name
*/
  printer[0] = (char) NULL ;

#ifdef __HELIOS
/*
#ifdef HAVE_UNSOCK
*/
  (void) unlink(SOCKETNAME);
#ifndef __HELIOS
  flocal = socket(AF_UNIX, SOCK_STREAM, 0);
  if (flocal < 0) {
    syslog(LOG_ERR, "socket: %m");
    exit(1);
  }
#else
  fhelios = socket(AF_HELIOS, SOCK_STREAM, 0);
  if (fhelios < 0) {
    syslog(LOG_ERR, "socket: %m");
    exit(1);
  }
#endif
  
#ifndef __HELIOS  
#define  mask(s)  (1 << ((s) - 1))
  omask = sigblock(mask(SIGHUP)|mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
  signal(SIGHUP, mcleanup);
  signal(SIGINT, mcleanup);
  signal(SIGQUIT, mcleanup);
  signal(SIGTERM, mcleanup);
#else
  {
    struct sigaction act;
    act.sa_handler = mcleanup;
    act.sa_mask = 0;
    act.sa_flags = SA_ASYNC;
    (void) sigaction(SIGINT, &act, NULL);
    (void) sigaction(SIGHUP, &act, NULL);
    (void) sigaction(SIGQUIT, &act, NULL);
    (void) sigaction(SIGTERM, &act, NULL);
  }
#endif

#ifndef __HELIOS
  slo.sun_family = AF_UNIX;
  strcpy(slo.sun_path, SOCKETNAME);
  if (bind(flocal, &slo, strlen(slo.sun_path) + 2) < 0) {
    syslog(LOG_ERR, "ubind: %m");
    exit(1);
  }
  sigsetmask(omask);
  defreadfds = 1 << flocal;
  listen(flocal, 5);
#else
  socket_addr.sh_family = AF_HELIOS;
  strcpy(socket_addr.sh_path, SOCKETNAME);
  if (bind(fhelios, (struct sockaddr *) &socket_addr, 
      sizeof (socket_addr)) < 0) {
    syslog(LOG_ERR, "ubind: %m");
    exit(1);
  }
  
#ifndef __HELIOS  
  sigsetmask(omask);
#else
  debugf ("sigsetmask() NOT done") ;
#endif
  defreadfds = 1 << fhelios;
  listen(fhelios, 5);
#endif

#else /* __HELIOS (HAVE_UNSOCK) */
  signal(SIGHUP, mcleanup);
  signal(SIGINT, mcleanup);
  signal(SIGQUIT, mcleanup);
  signal(SIGTERM, mcleanup);

  flocal = socket(AF_INET, SOCK_STREAM, 0);
  if (flocal >= 0) {
    struct servent *sp;

    if (options & SO_DEBUG)
      if (setsockopt(flocal, SOL_SOCKET, SO_DEBUG, 0, 0) < 0) {
        syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
        mcleanup();
      }
    sp = getservbyname("local_printer", "tcp");
    if (sp == NULL) {
      syslog(LOG_ERR, "local_printer/tcp: unknown service");
      mcleanup();
    }
    sin.sin_family = AF_INET;
    sin.sin_port = sp->s_port;
    if (bind(flocal, &sin, sizeof(sin), 0) < 0) {
      syslog(LOG_ERR, "bind: %m");
      mcleanup();
    }
    defreadfds |= 1 << flocal;
    listen(flocal, 5);
  }
#endif /* else HAVE_UNSOCK */

#ifndef __HELIOS
  finet = socket(AF_INET, SOCK_STREAM, 0);
  if (finet >= 0) {
    struct servent *sp;

    if (options & SO_DEBUG)
      if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, 0, 0) < 0) {
        syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
        mcleanup();
      }
    sp = getservbyname("printer", "tcp");
    if (sp == NULL) {
      syslog(LOG_ERR, "printer/tcp: unknown service");
      mcleanup();
    }
    sin.sin_family = AF_INET;
    sin.sin_port = sp->s_port;
    if (bind(finet, &sin, sizeof(sin), 0) < 0) {
      syslog(LOG_ERR, "bind: %m");
      mcleanup();
    }
    defreadfds |= 1 << finet;
    listen(finet, 5);
  }
#endif
  
  /*
   * Main loop: accept, do a request, continue.
   */
  for (;;) {
    int domain, nfds, s, readfds = defreadfds;

#ifdef __HELIOS
/*
-- crf: id of child process (vfork)
*/
    int aux_pid ;
#endif

    nfds = select(20, &readfds, 0, 0, 0);
    if (nfds <= 0) {
      if (nfds < 0 && errno != EINTR)
        syslog(LOG_WARNING, "select: %m");
      continue;
    }
#ifndef __HELIOS
    if (readfds & (1 << flocal)) {
#ifdef HAVE_UNSOCK
      domain = AF_UNIX, fromlen = sizeof(fromlocal);
#else
      domain = AF_INET, fromlen = sizeof(fromlocal);
#endif
      s = accept(flocal, &fromlocal, &fromlen);
    } else if (readfds & (1 << finet)) {
      domain = AF_INET, fromlen = sizeof(frominet);
      s = accept(finet, &frominet, &fromlen);
    }
#else
    if (readfds & (1 << fhelios)) {
      domain = AF_HELIOS, fromlen = sizeof(fromhelios);
      s = accept (fhelios, (struct sockaddr *)&fromhelios, 
                  &fromlen);
    }
#endif      
    if (s < 0) {
      if (errno != EINTR)
        syslog(LOG_WARNING, "accept: %m");
      continue;
    }
#ifndef __HELIOS    
    if (fork() == 0) {
      signal(SIGCHLD, SIG_IGN);
      signal(SIGHUP, SIG_IGN);
      signal(SIGINT, SIG_IGN);
      signal(SIGQUIT, SIG_IGN);
      signal(SIGTERM, SIG_IGN);
      (void) close(flocal);
      (void) close(finet);
      dup2(s, 1);
      (void) close(s);
      if ((domain == AF_INET) && (readfds & (1 << finet)))
        chkhost(&frominet);
      doit();
      exit(0);
    }
#else
    if ((aux_pid = vfork()) == 0) 
    {
/*
-- crf: SIG_IGN doesn't work properly with vfork (affects parent ...)
*/
      (void) close(fhelios);

      dup2(s, 1);
      (void) close(s);

      cmd_str [0] = CALL_DOIT + '0' ;
      if (execve (AUX_PATHNAME, aux_argv, environ) < 0)
      {
        syslog (LOG_ERR, 
                "cannot start printer daemon: %m") ;
      }
#ifndef __HELIOS
      exit(0) ;
#else
      _exit(0) ;
#endif      
    }
#ifdef __HELIOS    
/*
-- crf: problems with SIGCHLD ...
*/
    else
    {
      if (Fork (1000, wait_for_aux, sizeof (aux_pid), aux_pid) == 0)
        fatal ("insufficient memory for wait_for_aux()") ;
    }
#endif    
#endif    
    (void) close(s);
  }
}

void wait_for_aux (int pid)
{
  int status ;
  debugf ("entering wait_for_aux()") ;  
  waitpid (pid, &status, 0) ;
  debugf ("exiting wait_for_aux()") ;  
}

void reapchild()
{
#ifndef __HELIOS
  union wait status;
#else
  int status;
#endif  
  int pid;

  debugf ("entering reapchild()") ;
  while ((pid = wait3(&status, WNOHANG, NULL)) > 0)
#ifdef __HELIOS
                        ;
#else
#ifdef DEBUG
        fprintf(stderr, "wait3; pid=%d; stat=%o", pid, status.w_status);
#else
                        ;
#endif
#endif

#ifdef USG
  signal(SIGCHLD, reapchild);
#endif
  debugf ("exiting reapchild()") ;
}

void mcleanup()
{
  debugf ("entering mcleanup()") ;  
  syslog(LOG_INFO, "terminated");

#ifdef HAVE_UNSOCK
  unlink(SOCKETNAME);
#endif 
  debugf ("exiting mcleanup()") ;  
#ifdef MEM_CHECK
  IOdebug ("exiting lpd: Bytes free : %d  Heap size : %d", 
            Malloc(-1), Malloc(-3));
#endif
  exit(0);
}

/*
 * Make a pass through the printcap database and start printing any
 * files left from the last time the machine went down.
 */
void startup()
{
  char buf[BUFSIZ];
  register char *cp;
  int pid;

#ifndef __HELIOS
  printer = buf;
#else
#define MAX_PRINTERS 50
  char *prn_name [MAX_PRINTERS] ;
  int prn_id = 0 ;
  int i = 0 ;
#endif

  /*
   * Restart the daemons.
   */
  while (getprent(buf) > 0) {
    for (cp = buf; *cp; cp++)
      if (*cp == '|' || *cp == ':') {
        *cp = '\0';
        break;
      }

#ifndef __HELIOS
    if ((pid = fork()) < 0) {
      syslog(LOG_WARNING, "startup: cannot fork");
      mcleanup();
    }
    if (!pid) {
      endprent();
      printjob();
    }
  }
}
#elif defined OLDCODE
    if ((pid = vfork()) < 0) {
      syslog(LOG_WARNING, "startup: cannot fork");
      mcleanup();
    }
    if (!pid) 
    {
      endprent(); /* crf: suspect ! */
      SET_AUX_ARGS (CALL_PRINTJOB, printer) ;
      if (execve (AUX_PATHNAME, aux_argv, environ) < 0)
      {
        syslog (LOG_ERR, "execve (1) : errno = %d", errno) ;
      }
    }
  }
}
#else
    debugf ("starting printer daemons") ;

    if (prn_id >= MAX_PRINTERS)
      syslog (LOG_WARNING, ": too many printers: %s daemon not started", 
              buf) ;
    else
    {
      prn_name [prn_id] = (char *) Malloc ((long)strlen (buf) + 1) ;
      if (!prn_name [prn_id])
              fatal ("out of memory") ;
      strcpy (prn_name [prn_id], buf) ;
      debugf ("Malloc'd printer : %s OK", prn_name [prn_id]) ;
      prn_id ++ ; 
    }
  }

/* 
-- crf: probably not necessary (should have been closed by getprent()) 
*/
  endprent(); 

  for (i = 0 ; i < prn_id ; i ++)
  {
    strcpy (printer, prn_name [i]) ;
    debugf ("freeing printer : %s", prn_name [i]) ;
    if (Free (prn_name [i]) != 0)
      fatal ("error freeing memory") ;

    if ((pid = vfork()) < 0) {
      syslog(LOG_WARNING, "startup: cannot fork: %m");
      mcleanup();
    }
    if (!pid) 
    {
      cmd_str [0] = CALL_PRINTJOB + '0' ;
      debugf ("execve : %s", printer) ;
      if (execve (AUX_PATHNAME, aux_argv, environ) < 0)
      {
        syslog (LOG_ERR, 
                "startup: cannot start printer daemon: %m") ;
      }
      _exit(0) ;
    }
  }
}
#endif

#ifndef __HELIOS
#define DUMMY ":nobody::"

/*
 * Check to see if the from host has access to the line printer.
 */
chkhost(f)
  struct sockaddr_in *f;
{
  register struct hostent *hp;
  register FILE *hostf;
  register char *cp, *sp;
  char ahost[50];
  int first = 1;
  extern char *inet_ntoa();
  int baselen = -1;

  f->sin_port = ntohs(f->sin_port);
  if (f->sin_family != AF_INET || f->sin_port >= IPPORT_RESERVED)
    fatal("Malformed from address");
  hp = gethostbyaddr(&f->sin_addr, sizeof(struct in_addr), f->sin_family);
  if (hp == 0)
    fatal("Host name for your address (%s) unknown",
      inet_ntoa(f->sin_addr));

  strcpy(fromb, hp->h_name);
  from = fromb;
  if (!strcmp(from, host))
    return;

  sp = fromb;
  cp = ahost;
  while (*sp) {
    if (*sp == '.') {
      if (baselen == -1)
        baselen = sp - fromb;
      *cp++ = *sp++;
    } else {
      *cp++ = isupper(*sp) ? tolower(*sp++) : *sp++;
    }
  }
  *cp = '\0';
  hostf = fopen("/etc/hosts.equiv", "r");
again:
  if (hostf) {
    if (!_validuser(hostf, ahost, DUMMY, DUMMY, baselen)) {
      (void) fclose(hostf);
      return;
    }
    (void) fclose(hostf);
  }
  if (first == 1) {
    first = 0;
    hostf = fopen("/etc/hosts.lpd", "r");
    goto again;
  }
  fatal("Your host does not have line printer access");
}
#endif

#ifdef __HELIOS
void fatal (char *format, ...)
{
  va_list args;
  char err_msg [256] ;
  
  va_start (args, format);
  vsprintf (err_msg, format, args) ;    
  syslog (LOG_ERR, "FATAL : %s", err_msg);
  if (printer)
    syslog (LOG_ERR, "printer = %s", printer);
  va_end (args);
  exit(1);
}
#endif
