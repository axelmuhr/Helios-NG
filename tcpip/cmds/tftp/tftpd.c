/*$Log: tftpd.c,v $
 * Revision 1.6  1991/10/04  10:03:32  nickc
 * fixed CHROOT option
 *
 * Revision 1.5  1991/04/16  11:26:31  nickc
 * changed more exit stati
 *
 * Revision 1.4  1991/04/16  11:15:13  nickc
 * *** empty log message ***
 *
 * Revision 1.3  1991/04/16  11:14:28  nickc
 * changed define dependencies from helios to __HELIOS
 *
 * Revision 1.2  1991/04/16  10:13:20  nickc
 * changed exit status upon success to 0
 *
 * Revision 1.1  1991/04/16  10:09:07  nickc
 * Initial revision
 *
 * Revision 1.6.1 89/06/30  16:40:14  16:40:14  ronw (Ron Williams)
 * fix 'put' comand
 * 	routine validate_access was bailing out when getting write request
 *	(mode=2) and the file did not exist. So... created file with
 *	ownership of 'ownership of tftpd' and access mode of 666 (less umask).
 *	Note that 'tftpd' should have the SUID and SGID bits set and be
 *      owned by the user/group that provides the appropriate access
 *	restrictions.
 *
 * Revision 1.5  88/01/11  15:40:07  15:40:07  tai (Tai Jin)
 * fix -t option
 * 
 * Revision 1.4  88/01/11  15:36:51  15:36:51  tai (Tai Jin)
 * fix timeout log message
 * 
 * Revision 1.3  88/01/11  14:37:37  14:37:37  tai (Tai Jin)
 * change default dir to /usr/pub
 * 
 * Revision 1.2  88/01/11  14:34:30  14:34:30  tai (Tai Jin)
 * fixed a few things
 * 
 * Revision 1.1  87/12/01  14:03:43  14:03:43  root (Nucleus)
 * Initial revision
 * 
 * Revision 1.7  86/10/23  11:45:51  11:45:51  tai (jin)
 * merged with released 4.3 code
 * 
 * Revision 1.6  86/08/16  23:34:55  23:34:55  jin (tai)
 * exit if recvfrom returns zero (client went home) instead of logging
 * a bogus address and doing unnecessary things
 * 
 * Revision 1.5  86/07/25  11:47:09  11:47:09  krishnan (N.K. Krishnan)
 * Added CHROOT option that changes directory and root to a directory
 * specified on the command line.  The default is /usr/spool/uucppublic.
 * Fixed an old bug that was looking for options in argv[0].
 * 
 * Revision 1.4  86/07/11  01:04:57  01:04:57  jin (Tai Jin)
 * no error message when we receive 0 bytes (assume client quit)
 * 
 * Revision 1.3  86/05/31  18:19:35  18:19:35  jin (tai)
 * *** empty log message ***
 * */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#ifndef	hpux
#include <sys/wait.h>
#else
#include <fcntl.h>
#endif
#include <sys/stat.h>

#include <netinet/in.h>

#include "tftp.h"

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <setjmp.h>
#ifdef	SYSLOG
#include <syslog.h>
#endif

#ifdef __HELIOS
#include <posix.h>
#include <fcntl.h>
#endif

#define	TIMEOUT		5

#define DEFAULTDIR	"/helios/tftpdir"

extern	int errno;
struct	sockaddr_in sin = { AF_INET };
int	peer;
int	rexmtval = TIMEOUT;
int	maxtimeout = 5*TIMEOUT;
#ifdef	CHROOT
char	rootat[BUFSIZ] = DEFAULTDIR;
#endif	CHROOT

#define	PKTSIZE	SEGSIZE+4
char	buf[PKTSIZE];
char	ackbuf[PKTSIZE];
struct	sockaddr_in from;
int	fromlen;

int	debug;
int	logging;


int
main( argc, argv )
  int 		argc;
  char **	argv;
{
  register struct tftphdr *	tp;
  register int 			n;
  register char *		cp;
  int 				on = 1;
  
#ifdef DEFAULTDIR
  /* set default directory */
  
  if (chdir( DEFAULTDIR ) < 0)
    chdir( "/helios" );
#endif DEFAULTDIR
  
  argc--;argv++;	/* get rid of the argv[0] before looking at options */
  
  while (argc > 0 && *argv[ 0 ] == '-')
    {
      for (cp = &argv[ 0 ][ 1 ]; *cp; cp++)
	switch (*cp)
	  {
	case 'd':
	  debug = 1;
	  break;
	  
	case 'l':
	  logging = 1;
	  break;
	  
	case 't':
	  if (++cp == '\0')
	    cp = *++argv;
	  
	  maxtimeout = atoi(cp);
	  
	  goto nextopt;
	  break;
	  
#ifdef	CHROOT
	case 'r':
	  if (++cp == '\0')
	    cp = *++argv;
	  strncpy(&rootat[0], cp, BUFSIZ);
	  goto nextopt;
	  break;
#endif	CHROOT
	  
	default:
	  fprintf(stderr, "tftpd: Unknown flag -%c ignored.\n", *cp);
	  break;
	}
    nextopt:
      argc--, argv++;
    }
  
#ifdef	SYSLOG
#ifdef	ULTRIX
  openlog("tftpd", LOG_PID);
#else
  openlog("tftpd", LOG_PID, LOG_DAEMON);
#endif
#endif /* SYSLOG */
  
#ifdef	CHROOT
  if (chdir(rootat) < 0)
    {
#ifdef	SYSLOG
      syslog(LOG_ERR, "chdir %s failed because %m", rootat);
#endif	/* SYSLOG */
      exit(1);
    }
#ifndef __HELIOS  
  if (chroot(rootat) == -1)
    {
#ifdef	SYSLOG
      syslog(LOG_ERR, "chroot %s failed because %m", rootat);
#endif	/* SYSLOG */
      exit(1);
    }
#endif /* __HELIOS */
#endif	/* CHROOT */
  
  if (ioctl(0, FIONBIO, &on) < 0)
    {
#ifdef	SYSLOG
      syslog(LOG_ERR, "ioctl(FIONBIO): %m\n");
#endif
      exit(1);
    }
  
  fromlen = sizeof (from);
  
  n = recvfrom(0, buf, sizeof (buf), 0, (caddr_t)&from, &fromlen);
  
  if (n < 0)
    {
#ifdef	SYSLOG
      if (errno != EWOULDBLOCK)
	syslog(LOG_ERR, "recvfrom: %m\n");
#endif
      exit(1);
    }
  
  /*
   * Now that we have read the message out of the UDP
   * socket, we fork and exit.  Thus, inetd will go back
   * to listening to the tftp port, and the next request
   * to come in will start up a new instance of tftpd.
   *
   * We do this so that inetd can run tftpd in "wait" mode.
   * The problem with tftpd running in "nowait" mode is that
   * inetd may get one or more successful "selects" on the
   * tftp port before we do our receive, so more than one
   * instance of tftpd may be started up.  Worse, if tftpd
   * break before doing the above "recvfrom", inetd would
   * spawn endless instances, clogging the system.
   */
  
    {
      int pid;
      int i, j;
      
      for (i = 1; i < 20; i++)
	{
#ifdef __HELIOS
	  /*
	   * XXX - NC
	   *
	   * yuck, yuck, yuck
	   *
	   * for now I'll just make inetd wait for the 'child'
	   * process to finish ...
	   */
	  
	  pid = 0;
#else
	  pid = fork();
#endif
	  if (pid < 0)
	    {
	      sleep(i);
	      
	      /*
	       * flush out to most recently sent request.
	       *
	       * This may drop some request, but those
	       * will be resent by the clients when
	       * they timeout.  The positive effect of
	       * this flush is to (try to) prevent more
	       * than one tftpd being started up to service
	       * a single request from a single client.
	       */
	      
	      j = sizeof from;
	      
	      i = recvfrom(0, buf, sizeof (buf), 0, (caddr_t)&from, &j);
	      
	      if (i > 0)
		{
		  n = i;
		  fromlen = j;
		}
	    }
	  else
	    {
	      break;
	    }
	}
      
      if (pid < 0)
	{
#ifdef	SYSLOG
	  syslog(LOG_ERR, "fork: %m\n");
#endif
	  exit(1);
	}
      else if (pid != 0)
	{
	  exit(0);
	}
    }
  
  from.sin_family = AF_INET;
  
  alarm(0);
  close(0);
  close(1);
  
  peer = socket(AF_INET, SOCK_DGRAM, 0);
  
  if (peer < 0)
    {
#ifdef	SYSLOG
      syslog(LOG_ERR, "socket: %m");
#endif
      exit(1);
    }
  
  if (bind( peer, (caddr_t)&sin, sizeof( sin ) ) < 0)
    {
#ifdef	SYSLOG
      syslog(LOG_ERR, "bind: %m");
#endif
      exit(1);
    }
  
  if (connect( peer, (caddr_t)&from, sizeof( from ) ) < 0)
    {
#ifdef	SYSLOG
      syslog(LOG_ERR, "connect: %m");
#endif
      exit(1);
    }
  
#ifdef SYSLOG
  if (debug || logging)
    syslog(LOG_INFO, "requestor(%s)", inet_ntoa(from.sin_addr.s_addr));
#endif SYSLOG
  
  tp = (struct tftphdr *)buf;
  
  tp->th_opcode = ntohs( tp->th_opcode );

  if (tp->th_opcode == RRQ || tp->th_opcode == WRQ)
    tftp( tp, n );
#ifdef	SYSLOG
  else if (n > 0 && tp->th_opcode != ACK)
    syslog( LOG_ERR, "bad opcode = %x, packet length = %d", tp->th_opcode, n );
#endif
  
  exit(0);
}


int	validate_access();
void	sendfile(), recvfile();

struct formats
  {
	char	*f_mode;
	int	(*f_validate)();
	int	(*f_send)();
	int	(*f_recv)();
	int	f_convert;
} formats[] = {
	{ "netascii",	validate_access,	sendfile,	recvfile, 1 },
/*
	{ "ascii",	validate_access,	sendfile,	recvfile },
*/
	{ "octet",	validate_access,	sendfile,	recvfile, 0 },
/*
	{ "binary",	validate_access,	sendfile,	recvfile },
*/
#ifdef notdef
	{ "mail",	validate_user,		sendmail,	recvmail, 1 },
#endif
	{ 0 }
};

/*
 * Handle initial connection protocol.
 */

tftp( tp, size )
	struct tftphdr *tp;
	int size;
{
	register char *cp;
	int first = 1, ecode;
	register struct formats *pf;
	char *filename, *mode;

#ifdef __HELIOS
	filename = cp = (char *)&(tp->th_field);
#else
	filename = cp = tp->th_stuff;
#endif
#ifdef __HELIOS
	/* XXX - NC - cope with the fact that we cannot do chroot() */
	if (*filename == '/')
		filename++;
/*	IOdebug( "tftpd: filename = %s", cp ); */
#endif
	
again:
	while (cp < buf + size)
	{
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
#ifdef SYSLOG
		if (logging)
			syslog(LOG_INFO, "filename not null terminated");
#endif SYSLOG
		nak(EBADOP);
		exit(1);
	}
	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}
#ifdef SYSLOG
	if (debug) {
		switch (tp->th_opcode) {
		case RRQ:
			cp = "read"; break;
		case WRQ:
			cp = "write"; break;
		}
		syslog(LOG_DEBUG, "request(%s) file(%s)", cp, filename);
	}
#endif SYSLOG
	for (cp = mode; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	for (pf = formats; pf->f_mode; pf++)
		if (strcmp(pf->f_mode, mode) == 0)
			break;
	if (pf->f_mode == 0) {
#ifdef SYSLOG
		if (logging)
			syslog(LOG_INFO, "transfer format not supported");
#endif SYSLOG
		nak(EBADOP);
		exit(1);
	}
	ecode = (*pf->f_validate)(filename, tp->th_opcode);

	if (ecode)
	{
#ifdef SYSLOG
		if (logging)
		{
			switch (ecode)
			{
			case ENOTFOUND:
				cp = "file not found"; break;
			case EACCESS:
				cp = "access violation"; break;
			case ENOSPACE:
				cp = "disk full or allocation exceeded"; break;
			case EBADID:
				cp = "unknown transfer ID"; break;
			case EEXISTS:
				cp = "file already exists"; break;
			case ENOUSER:
				cp = "no such user"; break;
			default:
				cp = "unknown"; break;
			}
			syslog(LOG_INFO, "validation failed, file(%s) error(%s)",
				filename, cp);
		}
#endif SYSLOG
		nak(ecode);
		exit(0);
	}
	if (tp->th_opcode == WRQ)
		(*pf->f_recv)(pf);
	else
		(*pf->f_send)(pf);
	exit(0);
}


FILE *file;

/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable.
 * Note also, full path name must be
 * given as we have no login directory.
 */
validate_access(filename, mode)
	char *filename;
	int mode;
{
	struct stat stbuf;
	int	fd;

/* 	IOdebug( "tftpd: validating %s (mode = %d)", filename, mode ); */

#ifndef DEFAULTDIR
#ifndef	CHROOT
	if (*filename != '/')
		return (EACCESS);
#endif	!CHROOT
#endif

/* Modification 06/30/89 ronw@hpuflfa to fix 'put' command */
statagain:
	if (stat(filename, &stbuf) < 0)
	{
/*		IOdebug( "tftpd: stat failed, errno = %d", errno );  */
		
		if ( mode == WRQ && errno == ENOENT )
		{
			if ( fd=creat(filename,S_IRUSR
		 	                     | S_IWUSR
		 	                     | S_IRGRP
		 	                     | S_IWGRP
		 	                     | S_IROTH
		 	                     | S_IWOTH) > 0 )
		 	{
				close(fd);
				goto statagain;
			}
			else
				return (EACCESS);
		}
		else
			return (errno == ENOENT ? ENOTFOUND : EACCESS);
	}
/* end of 06/30/89 'put' fix */

	if (mode == RRQ)
	{
#ifdef __HELIOS
		if (((stbuf.st_mode & S_IFREG) != S_IFREG) ||
			((stbuf.st_mode & S_IRUSR) != S_IRUSR))
#else
		if ((stbuf.st_mode & (S_IREAD >> 6)) == 0)
#endif
		{
/*			IOdebug( "st_mode = %x", stbuf.st_mode ); */
			
			return (EACCESS);
		}
	}
	else
	{
#ifdef __HELIOS
		if (((stbuf.st_mode & S_IFREG) != S_IFREG) ||
			((stbuf.st_mode & S_IWUSR) != S_IWUSR))
#else
		if ((stbuf.st_mode & (S_IWRITE >> 6)) == 0)
#endif
			return (EACCESS);
	}

/*	IOdebug( "tftpd: trying to open file" ); */
	
	fd = open( filename, mode == RRQ ? O_RDONLY : O_WRONLY );
	
	if (fd < 0)
	{
/* 		IOdebug( "tftpd: open failed, errno = %d", errno ); */
		
		return (errno + 100);
	}

	file = fdopen( fd, (mode == RRQ)? "rb":"wb" );
	
	if (file == NULL)
	{
/*		IOdebug( "tftpd: fopen failed, errno = %d", errno ); */
		
		return errno+100;
	}
	
/*	IOdebug( "tftpd: validate succeded" );  */
	
	return (0);
}

int	timeout;
jmp_buf	timeoutbuf;

timer()
{

	timeout += rexmtval;
	if (timeout >= maxtimeout) {
#ifdef SYSLOG
		if (debug)
			syslog(LOG_DEBUG, "timed out after %d attempts",
				timeout/rexmtval);
#endif SYSLOG
		exit(0);
	}
#ifdef	hpux
	signal(SIGALRM, timer);
#endif
	longjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */

void
sendfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *r_init();
	register struct tftphdr *ap;    /* ack packet */
	register int block = 1, size, n;

/*	IOdebug( "tftpd: sendfile: called" );  */
	
	signal(SIGALRM, timer);
	
	dp = r_init();
	ap = (struct tftphdr *)ackbuf;
	
	do
	{
		size = readit(file, &dp, pf->f_convert);
		
		if (size < 0)
		{
/*			IOdebug( "tftpd: read failed, errno = %d", errno ); */
			
			nak(errno + 100);
			goto abort;
		}
		
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;
		(void) setjmp(timeoutbuf);

send_data:
/* 		IOdebug( "tftpd: sending %d bytes of data", size + 4 ); */
		
		if (send(peer, dp, size + 4, 0) != size + 4)
		{
/*			IOdebug( "tftpd: send failed, errno = %d", errno ); */
#ifdef	SYSLOG
			syslog(LOG_ERR, "tftpd: write: %m");
#endif
			goto abort;
		}
		
/*		IOdebug( "tftpd: data sent, block = %d", block ); */
		
		read_ahead( file, pf->f_convert );
		
		for ( ; ; )
		{
			alarm( rexmtval );        /* read the ack */
			
/*			IOdebug( "tftpd: receiving from peer" ); */
			
			oserr = 0;
			
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			
			alarm(0);
			
/* 			IOdebug( "tftpd: received %d bytes from peer", n );  */
			
			if (n < 0)
			{
/*	IOdebug( "tftpd: receive aborted, errno = %d, oserr = %x",
		errno, oserr ); */
#ifdef	SYSLOG
				syslog(LOG_ERR, "tftpd: read: %m");
#endif
				goto abort;
			}
			
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block  = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == ERROR)
				goto abort;
			
			if (ap->th_opcode == ACK)
			{
				if (ap->th_block == block)
				{
					break;
				}				

				/* Re-synchronize with the other side */
				
				(void) synchnet( peer );
				
				if (ap->th_block == (block -1))
				{
					goto send_data;
				}
			}
		}

		block++;
		
	} while (size == SEGSIZE);
abort:
/*	IOdebug( "tftpd: send finished" );  */
	
	(void) fclose(file);
}

justquit()
{
	exit(0);
}


/*
 * Receive a file.
 */
void
recvfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *w_init();
	register struct tftphdr *ap;    /* ack buffer */
	register int block = 0, n, size;

	signal(SIGALRM, timer);
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	do {
		timeout = 0;
		ap->th_opcode = htons((u_short)ACK);
		ap->th_block = htons((u_short)block);
		block++;
		(void) setjmp(timeoutbuf);
send_ack:
		if (send(peer, ackbuf, 4, 0) != 4) {
#ifdef	SYSLOG
			syslog(LOG_ERR, "tftpd: write: %m");
#endif
			goto abort;
		}
		write_behind(file, pf->f_convert);
		for ( ; ; ) {
			alarm(rexmtval);
			n = recv(peer, dp, PKTSIZE, 0);
			alarm(0);
			if (n < 0) {            /* really? */
#ifdef	SYSLOG
				syslog(LOG_ERR, "tftpd: read: %m");
#endif
				goto abort;
			}
			dp->th_opcode = ntohs((u_short)dp->th_opcode);
			dp->th_block = ntohs((u_short)dp->th_block);
			if (dp->th_opcode == ERROR)
				goto abort;
			if (dp->th_opcode == DATA) {
				if (dp->th_block == block) {
					break;   /* normal */
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (dp->th_block == (block-1))
					goto send_ack;          /* rexmit */
			}
		}
		/*  size = write(file, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, pf->f_convert);
		if (size != (n-4)) {                    /* ahem */
			if (size < 0) nak(errno + 100);
			else nak(ENOSPACE);
			goto abort;
		}
	} while (size == SEGSIZE);
	write_behind(file, pf->f_convert);
	(void) fclose(file);            /* close data file */

	ap->th_opcode = htons((u_short)ACK);    /* send the "final" ack */
	ap->th_block = htons((u_short)(block));
	(void) send(peer, ackbuf, 4, 0);

	signal(SIGALRM, justquit);      /* just quit on timeout */
	alarm(rexmtval);
	n = recv(peer, buf, sizeof (buf), 0); /* normally times out and quits */
	alarm(0);
	if (n >= 4 &&                   /* if read some data */
	    dp->th_opcode == DATA &&    /* and got a data block */
	    block == dp->th_block) {	/* then my last ack was lost */
		(void) send(peer, ackbuf, 4, 0);     /* resend final ack */
	}
abort:
	return;
}

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ -1,		0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
	extern char *sys_errlist[];

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = sys_errlist[error - 100];
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (send(peer, buf, length, 0) != length)
#ifdef	SYSLOG
		syslog(LOG_ERR, "nak: %m");
#else
		;
#endif
}
