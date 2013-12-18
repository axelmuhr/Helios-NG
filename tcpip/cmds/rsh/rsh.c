/*
 * Copyright (c) 1983 The Regents of the University of California.
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

#ifdef lint
char copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifdef lint
static char sccsid[] = "@(#)rsh.c	5.7 (Berkeley) 9/20/88";
#endif /* not lint */

  #include <stdio.h>
  #include <string.h>
  #include <stdlib.h>
  #ifdef __HELIOS
  #include <helios.h>
  #include <syslib.h>
  #endif
  
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/wait.h>
  
#if 0
#include <task.h>
#endif

#include <netinet/in.h>
#include <sgtty.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

#define ctrl( a )	(a - 0x40)
  
  extern word ioctl(int fd, int code, caddr_t data);
  extern int gtty(int fd, struct sgttyb *buf);
  extern int stty(int fd, struct sgttyb *buf);
  extern int rcmd( char ** ahost, u_short rport, char *  locuser, char *  remuser, char *  cmd, int *   fd2p );
  
  
/*
 * rsh - remote shell
 */

int	options;
int	rfd2;
int	nflag;

int send_signal = -1;

#define	mask(s)	(1 << ((s) - 1))

void
sendsig(int signo )
{
#if 0
	MyTask->Flags |= Task_Flags_stream;
#endif
	send_signal = signo;
}

int
main(
     int 	argc,
     char **	argv0 )
{
  int 			rem;
  char *		host;
  char *		cp;
  char **		ap;
  char *		args;
  char **		argv = argv0;
  char *		user = 0;
  static char		buf[  BUFSIZ     ];
  static char		buf2[ BUFSIZ * 2 ];
  register int		cc;
  int			asrsh = 0;
  struct passwd *	pwd;
  int			readfrom, ready;
  int			one = 1;
  struct servent *	sp;

  
  host = rindex(argv[0], '/');
  
  if (host)
    host++;
  else
    host = argv[0];
  
  argv++, --argc;
  
  if (!strcmp(host, "rsh"))
    {
      host = *argv++, --argc;
      asrsh = 1;
    }
  
another:
  
  if (argc > 0 && !strcmp(*argv, "-l"))
    {
      argv++, argc--;
      if (argc > 0)
	user = *argv++, argc--;
      goto another;
    }
  
  if (argc > 0 && !strcmp(*argv, "-n"))
    {
      argv++, argc--;
      nflag++;
      goto another;
    }
  
  if (argc > 0 && !strcmp(*argv, "-d"))
    {
      argv++, argc--;
      options |= SO_DEBUG;
      goto another;
    }
  
  /*
   * Ignore the -L, -w, -e and -8 flags to allow aliases with rlogin
   * to work
   *
   * There must be a better way to do this! -jmb
   */
  
  if (argc > 0 && !strncmp(*argv, "-L", 2))
    {
      argv++, argc--;
      goto another;
    }
  
  if (argc > 0 && !strncmp(*argv, "-w", 2))
    {
      argv++, argc--;
      goto another;
    }
  
  if (argc > 0 && !strncmp(*argv, "-e", 2))
    {
      argv++, argc--;
      goto another;
    }
  
  if (argc > 0 && !strncmp(*argv, "-8", 2))
    {
      argv++, argc--;
      goto another;
    }
  
  if (host == 0)
    goto usage;
  
  if (argv[0] == 0)
    {
      if (asrsh)
	*argv0 = "rlogin";
      
      if ( vfork() == 0 )
	{
	  execv("/helios/bin/rlogin", argv0);
	  perror("rlogin");
	}
      
      wait(0);
      exit(1);
    }
  
  pwd = getpwuid(getuid());
  
  if (pwd == 0)
    {
      fprintf(stderr, "who are you?\n");
      exit(1);
    }
  
  cc = 0;
  
  for (ap = argv; *ap; ap++)
    cc += strlen(*ap) + 1;
  
  cp = args = (char *) malloc(cc);
  
  for (ap = argv; *ap; ap++)
    {
      (void) strcpy(cp, *ap);
      while (*cp)
	cp++;
      if (ap[1])
	*cp++ = ' ';
    }
  
  sp = getservbyname( "shell", "tcp" );
  
  if (sp == 0)
    {
      fprintf(stderr, "rsh: shell/tcp: unknown service\n");
      exit(1);
    }
  
  rem = rcmd( &host, sp->s_port, pwd->pw_name, user ? user : pwd->pw_name, args, &rfd2 );
  
  if (rem < 0)
    exit(1);
  
  if (rfd2 < 0)
    {
      fprintf(stderr, "rsh: can't establish stderr\n");
      exit(2);
    }
  
  if (options & SO_DEBUG)
    {
      if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, (char *)&one, sizeof (one)) < 0)
	perror("setsockopt (stdin)");
      
      if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, (char *)&one, sizeof (one)) < 0)
	perror("setsockopt (stderr)");
    }
  
  (void) setuid(getuid());
  
    {
      struct sigaction sa;

      
      sa.sa_handler = (void(*)())sendsig;
      sa.sa_mask    = 0;
      sa.sa_flags   = SA_ASYNC;
      
      if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	sigaction(SIGINT, &sa, NULL);
      
      if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
	sigaction(SIGQUIT, &sa, NULL);
      
      if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
	sigaction(SIGTERM, &sa, NULL);
    }
  
  ioctl( rfd2, FIONBIO, (caddr_t)&one );
  ioctl( rem,  FIONBIO, (caddr_t)&one );
  
  if ( nflag == 0 )
    {
      struct sgttyb sg;
      gtty(0,&sg);
      sg.sg_flags |= RAW;
      stty(0,&sg);
    }	
  
  readfrom = (1<<rfd2) | (1<<rem) | (1<<0);
  
  do
    {
      int r;

      
      if ( send_signal != -1 )
	{
	  r = write(rfd2, (char *)&send_signal, 1);
	  send_signal = -1;
	}
      
      ready = readfrom;
      
      if ((r = select(16, &ready, 0, 0, 0)) < 0)
	{
	  if (errno != EINTR)
	    {
	      perror("select");
	      exit(1);
	    }
	  continue;
	}
      
      if (ready & 1)
	{
	  cc = read( 0, buf, sizeof buf );
	  
	  if (cc <= 0)
	    {
	      if (errno == EINTR)
		continue;
	      
	      if (errno != EWOULDBLOCK)
		{
		  /*
		   * XXX - NC - 5/5/93
		   *
		   * There are two problems here.  Firstly, the process connected to
		   * stdin has finished sending data.  Ideally we should pass this
		   * information on to the remote process, which would send any final
		   * messages of its own, and then close down.  Unfortunately we only
		   * have one file descriptor (rem) for both sending data to the
		   * remote process, and receiving data from the remote process.  (This
		   * is a bug in the RSH protocol).  If we close down 'rem' then we
		   * cannot receive any more data from the remote process, and if we
		   * do not close it down, then the remote process does not know that
		   * we have finished sending data, (assuming that it is reading from
		   * its stdin), and so it will not terminate.
		   *   The correct solution is to fix the RSH protocol.  Failing that
		   * the solution ought to be to use shutdown() on 'rem', (since it is
		   * probably a socket).  Unfortunately this does not work, so in the
		   * interest of program termination, even at the expense of loosing
		   * data, I close the file descriptor below.
		   */
#if 1
		  close( rem ); 
		  readfrom &= ~(1 << rem);
#else
		  shutdown( rem, 1 );
#endif		  
		  readfrom &= ~(1);
		}
	    }
	  else
	    {
	      write( rem, buf, cc );
	      
	      if (nflag == 0)
		{
		  if (buf[ cc - 1 ] == ctrl( 'C' ))
		    {
		      /* we have been interrupted */

		      exit( 0 );
		    }
		  else if (buf[ cc - 1 ] == ctrl( 'D' ))
		    {
		      close( rem ); 
		      readfrom &= ~(1<<rem);
		    }
		}
	    }
	}
      
      if (ready & (1 << rfd2))
	{
	  errno = 0;
	  
	  cc = read( rfd2, buf, sizeof buf );
	  
	  if (cc <= 0)
	    {
	      if ( errno == EINTR )
		continue;
	      
	      if (errno != EWOULDBLOCK)
		readfrom &= ~(1<<rfd2);
	    }
	  else
	    {
#if 1
	      /* Without select on non-RAW streams in IO */
	      /* server, we must do LF->CRLF here.	   */
	      int cc2 = 0;
	      int i;
	      
	      for ( i = 0; i < cc; i++ )
		{
		  if ( buf[i] == '\n' )
		    {
		      buf2[cc2++] = '\r';
		    }
		  
		  buf2[cc2++] = buf[i];
		}
	      
	      (void) write(2, buf2, cc2);
#else
	      (void) write(2, buf, cc);
#endif
	    }
	}
      
      if (ready & (1<<rem))
	{
	  errno = 0;
	  cc = read(rem, buf, sizeof buf);
	  if (cc <= 0)
	    {
	      if ( errno == EINTR ) continue;
	      if (errno != EWOULDBLOCK)
		readfrom &= ~(1<<rem);
	    } else
	      {
#if 1
		/* Without select on non-RAW streams in IO */
		/* server, we must do LF->CRLF here.	   */
		int cc2 = 0;
		int i;
		for ( i = 0; i < cc; i++ )
		  {
		    if ( buf[i] == '\n' )
		      {
			buf2[cc2++] = '\r';
		      }
		    buf2[cc2++] = buf[i];
		  }
		(void) write(1, buf2, cc2);
#else
		(void) write(1, buf, cc);
#endif
	      }
	}
    }
  while (readfrom & ~1);
  
  close(0);
  Exit(0);
 usage:
  fprintf(stderr,
	  "usage: rsh host [ -l login-name ] [ -n (use cooked input) ] command\n");
  exit(1);
}

