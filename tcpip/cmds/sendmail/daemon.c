/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef __HELIOS
#include <syslib.h>
#include <nonansi.h>
#endif

#include "sendmail.h"
#include <errno.h>
#include <sys/signal.h>

#ifndef __HELIOS
#ifndef lint
# ifdef DAEMON
static char sccsid[] = "@(#)daemon.c	5.36 (Berkeley) 6/1/90 (with daemon mode)";
# else /* !DAEMON */
static char sccsid[] = "@(#)daemon.c	5.36 (Berkeley) 6/1/90 (without daemon mode)";
# endif /* DAEMON */
#endif /* not lint */

int la;	/* load average */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/daemon.c,v 1.5 1993/02/26 14:55:28 paul Exp $";
#endif

#ifdef DAEMON

#ifndef __HELIOS
# include <netdb.h>
#else
#include <arpa/inet.h>
#endif

# include <sys/wait.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <sys/stat.h>
# ifdef NAMED_BIND
#  include <arpa/nameser.h>
#  include <resolv.h>
# endif /* NAMED_BIND */

/*
**  DAEMON.C -- routines to use when running as a daemon.
**
**	This entire file is highly dependent on the 4.2 BSD
**	interprocess communication primitives.  No attempt has
**	been made to make this file portable to Version 7,
**	Version 6, MPX files, etc.  If you should try such a
**	thing yourself, I recommend chucking the entire file
**	and starting from scratch.  Basic semantics are:
**
**	getrequests()
**		Opens a port and initiates a connection.
**		Returns in a child.  Must set InChannel and
**		OutChannel appropriately.
**	clrdaemon()
**		Close any open files associated with getting
**		the connection; this is used when running the queue,
**		etc., to avoid having extra file descriptors during
**		the queue run and to avoid confusing the network
**		code (if it cares).
**	makeconnection(host, port, outfile, infile)
**		Make a connection to the named host on the given
**		port.  Set *outfile and *infile to the files
**		appropriate for communication.  Returns zero on
**		success, else an exit status describing the
**		error.
**	maphostname(hbuf, hbufsize)
**		Convert the entry in hbuf into a canonical form.  It
**		may not be larger than hbufsize.
**
**	mapinit(c)
**		Open and initialize a dbm database.  Reopen if our current
**		file descriptor is out of date.
**
**	mapkey(c, key, argval, argsiz)
**		Search a database for a match to the given key, sprintf'ing
**		the argument through the result if found.
*/

/* new page */
/*
**  GETREQUESTS -- open mail IPC port and get requests.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Waits until some interesting activity occurs.  When
**		it does, a child is created to process it, and the
**		parent waits for completion.  Return from this
**		routine is always in the child.  The file pointers
**		"InChannel" and "OutChannel" should be set to point
**		to the communication channel.
*/

struct sockaddr_in	SendmailAddress; /* internet address of sendmail */
#ifdef __HELIOS
/*
-- crf: this used to be in "sendmail.h"
*/
struct sockaddr_in RealHostAddr;	/* address of host we are talking to */
#endif

#ifdef __HELIOS
static
#endif
int	DaemonSocket	= -1;		/* fd describing socket */
char	*NetName;			/* name of home (local?) network */

#ifndef __HELIOS
void
#else
int
#endif
getrequests()
{
	int t;
	register struct servent *sp;
	int on = 1;

#ifdef __HELIOS
	if (DaemonSocket == -1)
	{
#endif
	/*
	**  Set up the address for the mailer.
	*/

	sp = getservbyname("smtp", "tcp");
	if (sp == NULL)
	{
		syserr("server \"smtp\" unknown");
		goto severe;
	}
	SendmailAddress.sin_family = AF_INET;
	SendmailAddress.sin_addr.s_addr = INADDR_ANY;
	SendmailAddress.sin_port = sp->s_port;

	/*
	**  Try to actually open the connection.
	*/

#ifndef __HELIOS
	if (tTd(15, 1))
		printf("getrequests: port 0x%x\n", SendmailAddress.sin_port);
#endif

	/* get a socket for the SMTP connection */
	DaemonSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (DaemonSocket < 0)
	{
		/* probably another daemon already */
		syserr("getrequests: can't create socket");
	  severe:
# ifdef LOG
		if (LogLevel > 0)
			syslog(LOG_ALERT, "cannot get connection");
# endif /* LOG */
		finis();
	}

	/* turn on network debugging? */
#ifndef __HELIOS
	if (tTd(15, 15))
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
#endif	

	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	if (bind(DaemonSocket,
	    (struct sockaddr *) &SendmailAddress, sizeof SendmailAddress) < 0)
	{
		syserr("getrequests: cannot bind");
		(void) close(DaemonSocket);
		goto severe;
	}
	if (listen(DaemonSocket, 10) < 0)
	{
		syserr("getrequests: cannot listen");
		(void) close(DaemonSocket);
		goto severe;
	}

#ifndef __HELIOS
# ifdef SIGCHLD
	(void) signal(SIGCHLD, reapchild);
# endif /* SIGCHLD */

	if (tTd(15, 1))
		printf("getrequests: %d\n", DaemonSocket);

# ifdef _PATH_SENDMAILPID
	(void) WritePid();
# endif /* _PATH_SENDMAILPID */
#endif	

#ifdef __HELIOS
	}
#endif

	for (;;)
	{
		auto int lotherend;
#ifndef __HELIOS
		register int pid;
		extern int RefuseLA;

		/*
		 * see if we are rejecting connections
		 */
		while ((la = getla()) > RefuseLA)
		{
			setproctitle("rejecting connections: load average: %.2f", (double)la);
			Xsleep (5);
		}
		setproctitle("Waiting for connection");

		do
#endif		
		{
#ifdef __HELIOS
			int nfds ; 
			int readfds = 1 << DaemonSocket ;
#endif

			errno = 0;
			lotherend = sizeof RealHostAddr;
#ifndef __HELIOS
			t = accept(DaemonSocket,
			    (struct sockaddr *) &RealHostAddr, &lotherend);
#else
/*
-- crf: problems with killing a task waiting on an accept - use select instead
*/
			nfds = select(DaemonSocket + 1, &readfds, 0, 0, 0);
			if (nfds <= 0) 
			{
				if (nfds < 0 && errno != EINTR)
					syslog(LOG_WARNING, "select: %m");
				continue;
			}
			if (readfds & (1 << DaemonSocket))
				t = accept(DaemonSocket,
				    (struct sockaddr *) &RealHostAddr, &lotherend);
			else
			{
				syserr ("select: invalid readfds") ;
				(void) close(DaemonSocket);
				goto severe;
			}
#endif

#ifndef __HELIOS
		} while (t < 0 && errno == EINTR);
#else		
		}
#endif

		if (t < 0)
		{
			syserr("getrequests: accept");
#ifndef __HELIOS
			Xsleep(5);
#else
			Delay (OneSec * 5) ;
#endif			
			continue;
		}

#ifndef __HELIOS
		/*
		**  Create a subprocess to process the mail.
		*/

		if (tTd(15, 2))
			printf("getrequests: forking (fd = %d)\n", t);

		pid = fork();
		if (pid > 0 && tTd(4, 2))
			printf("getrequests: forking (pid = %d)\n", pid);
		if (pid < 0)
		{
			syserr("daemon: cannot fork");
			Xsleep(10);
			(void) close(t);
			continue;
		}

		if (pid == 0)
#endif		
		{
			register struct hostent *hp;
			char buf[MAXNAME];

			/*
			**  CHILD -- return to caller.
			**	Collect verified idea of sending host.
			**	Verify calling user id if possible here.
			*/

# ifdef SIGCHLD
			(void) signal(SIGCHLD, SIG_DFL);
# endif /* SIGCHLD */

			/* determine host name */
			hp = gethostbyaddr((char *) &RealHostAddr.sin_addr, sizeof RealHostAddr.sin_addr, AF_INET);
			if (hp != NULL)
				(void) strcpy(buf, hp->h_name);
			else
			{
				/* produce a dotted quad */
				(void) sprintf(buf, "[%s]",
					inet_ntoa(RealHostAddr.sin_addr));
			}

			/* should we check for illegal connection here? XXX */

#ifndef __HELIOS
			RealHostName = newstr(buf);
#else
			strcpy (RealHostName, buf);
#endif			

#ifndef __HELIOS
			(void) close(DaemonSocket);

			InChannel = fdopen(t, "r");
			OutChannel = fdopen(dup(t), "w");
			if (tTd(15, 2))
				printf("getreq: returning\n");
# ifdef LOG
			if (LogLevel > 11)
				syslog(LOG_DEBUG, "connected, pid=%d", getpid());
# endif /* LOG */
			return;
		}
		/* close the port so that others will hang (for a while) */
		(void) close(t);
	}
	/*NOTREACHED*/
#else
		return (t) ;
		} /* (pid == 0) */
	} /* for (;;) */
#endif
}

#ifndef __HELIOS
/* new page */
/*
**  CLRDAEMON -- reset the daemon connection
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		releases any resources used by the passive daemon.
*/

void
clrdaemon()
{
	if (DaemonSocket >= 0)
		(void) close(DaemonSocket);
	DaemonSocket = -1;
}
#endif

/* new page */
/*
**  MAKECONNECTION -- make a connection to an SMTP socket on another machine.
**
**	Parameters:
**		host -- the name of the host.
**		port -- the port number to connect to.
**		outfile -- a pointer to a place to put the outfile
**			descriptor.
**		infile -- ditto for infile.
**
**	Returns:
**		An exit code telling whether the connection could be
**			made and if not why not.
**
**	Side Effects:
**		none.
*/

#if defined __C40 || defined __ARM
int
makeconnection(
	       const char *	host,
	       u_short		port,
	       FILE **		outfile,
	       FILE **		infile )
#else
int makeconnection(host, port, outfile, infile)
	const char *host;
	u_short port;
	FILE **outfile;
	FILE **infile;
#endif
{
	register int i = 0;
	register int s;
	register struct hostent *hp = (struct hostent *)NULL;
	int sav_errno;

#ifdef __HELIOS
/*
-- crf: host must be mailhost
--      port must be 0
*/

	if (strcmp (host, MAIL_HOST))
	{
		syserr ("makeconnection: invalid host: %s", host) ;
		return (EX_SOFTWARE) ;
	}
        if (port != 0)
	{
		syserr ("makeconnection: invalid port: %d", port) ;
		return (EX_SOFTWARE) ;
	}
#endif

# ifdef NAMED_BIND
	extern int h_errno;

	/*
	**  Don't do recursive domain searches.  An example why not:
	**  Machine cerl.cecer.army.mil has a UUCP connection to
	**  osiris.cso.uiuc.edu.  Also at UIUC is a machine called
	**  uinova.cerl.uiuc.edu that accepts mail for the its parent domain
	**  cerl.uiuc.edu.  Sending mail to cerl!user with recursion on
	**  will select cerl.uiuc.edu which maps to uinova.cerl.uiuc.edu.
	**  We leave RES_DEFNAMES on so single names in the current domain
	**  still work.
	**
	**  Paul Pomes, CSO, UIUC	17-Oct-88
	*/

	_res.options &= (~RES_DNSRCH & 0xffff);

	/*
	**  Set up the address for the mailer.
	**	Accept "[a.b.c.d]" syntax for host name.
	*/

	h_errno = 0;
# endif /* NAMED_BIND */
	errno = 0;

#ifndef __HELIOS
	if (host[0] == '[')
	{
		long hid = -1;
		register char *p = index(host, ']');

		if (p != NULL)
		{
			*p = '\0';
			hid = inet_addr(&host[1]);
			*p = ']';
		}
		if (p == NULL || hid == -1)
		{
			usrerr("Invalid numeric domain spec \"%s\"", host);
			return (EX_NOHOST);
		}
		SendmailAddress.sin_addr.s_addr = hid;
	}
	else
#endif	
	{
		hp = gethostbyname((char *)host);
		if (hp == NULL)
		{
# ifdef NAMED_BIND
			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN)
				return (EX_TEMPFAIL);

			/* if name server is specified, assume temp fail */
			if (errno == ECONNREFUSED && UseNameServer)
				return (EX_TEMPFAIL);
# endif /* NAMED_BIND */

			/*
			**  XXX Should look for mail forwarder record here
			**  XXX if (h_errno == NO_ADDRESS).
			*/

			return (EX_NOHOST);
		}
		bcopy(hp->h_addr, (char *) &SendmailAddress.sin_addr, hp->h_length);
		i = 1;
	}

	/*
	**  Determine the port number.
	*/

	if (port != 0)
		SendmailAddress.sin_port = htons(port);
	else
	{
		register struct servent *sp = getservbyname("smtp", "tcp");

		if (sp == NULL)
		{
			syserr("makeconnection: server \"smtp\" unknown");
			return (EX_OSFILE);
		}
		SendmailAddress.sin_port = sp->s_port;
	}

	/*
	**  Try to actually open the connection.
	*/

#ifndef __HELIOS
again:
	if (tTd(16, 1))
		printf("makeconnection (%s [%s])\n", host,
		    inet_ntoa(SendmailAddress.sin_addr));
#endif

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		syserr("makeconnection: no socket");
		sav_errno = errno;
		goto failure;
	}

#ifndef __HELIOS
	if (tTd(16, 1))
		printf("makeconnection: %d\n", s);

	/* turn on network debugging? */
	if (tTd(16, 14))
	{
		int on = 1;
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
	}
	if (CurEnv->e_xfp != NULL)
		(void) fflush(CurEnv->e_xfp);		/* for debugging */
#endif

	errno = 0;					/* for debugging */
	SendmailAddress.sin_family = AF_INET;
	if (connect(s,
	    (struct sockaddr *) &SendmailAddress, sizeof SendmailAddress) < 0)
	{
		sav_errno = errno;
		(void) close(s);
# ifdef NAMED_BIND
		if (hp && hp->h_addr_list[i])
		{
			bcopy(hp->h_addr_list[i++],
			    (char *)&SendmailAddress.sin_addr, hp->h_length);
			goto again;
		}
# endif /* NAMED_BIND */

		/* failure, decide if temporary or not */
	failure:
		errno = sav_errno;
		switch (errno)
		{
		  case EISCONN:
		  case ETIMEDOUT:
		  case EINPROGRESS:
		  case EALREADY:
		  case EADDRINUSE:
		  case EHOSTDOWN:
		  case ENETDOWN:
		  case ENETRESET:
		  case ENOBUFS:
		  case ECONNREFUSED:
		  case ECONNRESET:
		  case EHOSTUNREACH:
		  case ENETUNREACH:
			/* there are others, I'm sure..... */
			return (EX_TEMPFAIL);

		  case EPERM:
			/* why is this happening? */
			syserr("makeconnection: funny failure, addr=%lx, port=%x",
				SendmailAddress.sin_addr.s_addr, SendmailAddress.sin_port);
			return (EX_TEMPFAIL);

		  default:
			{
				message(Arpa_Info, "%s", errstring(sav_errno));
				return (EX_UNAVAILABLE);
			}
		}
	}

	/* connection ok, put it into canonical form */
	*outfile = fdopen(s, "w");
	*infile = fdopen(s, "r");

	/* Necessary to catch leaks */
	if (*outfile == NULL || *infile == NULL)
	{
		syserr("makeconnection: *outfile or *infile NULL");
		return (EX_TEMPFAIL);
	}
	else
		return (EX_OK);
}

/* new page */
/*
**  MYHOSTNAME -- return the name of this host.
**
**	Parameters:
**		hostbuf -- a place to return the name of this host.
**		size -- the size of hostbuf.
**
**	Returns:
**		A list of aliases for this host.
**
**	Side Effects:
**		none.
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	struct hostent *hp;

	if (gethostname(hostbuf, size) < 0)
	{
		(void) strcpy(hostbuf, "localhost");
#ifdef __HELIOS
/*
-- crf: if this fails, the program should not continue - if it does, there
-- will be problems later on when an attempt is made to match a receipient
-- address against the name of the machine.
*/
		syserr ("failed to get host name") ;
		exit (EX_NOHOST) ;
#endif
	}
# ifdef NAMED_BIND
	while ((hp = gethostbyname(hostbuf)) == NULL)
	{
		setproctitle("gethostbyname(%s) failed, sleeping", hostbuf);
		Xsleep(10);
	}
# else /* !NAMED_BIND */
	hp = gethostbyname(hostbuf);
# endif /* NAMED_BIND */
	if (hp != NULL)
	{
		(void) strcpy(hostbuf, hp->h_name);
		return (hp->h_aliases);
	}
	else
		return (NULL);
}

#ifndef __HELIOS
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		An exit code telling if the hostname was found and
**		canonicalized.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
**/
bool
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	register struct hostent *hp;
	static char tmphbuf[MAXNAME];

	/*
	 * If first character is a bracket, then it is an address
	 * lookup.  Address is copied into a temporary buffer to
	 * strip the brackets and to preserve hbuf if address is
	 * unknown.
	 */
	if (*hbuf == '[')
	{
		u_long in_addr;

		(void) strncpy(tmphbuf, hbuf+1, strlen(hbuf)-2);
		tmphbuf[strlen(hbuf)-2]='\0';
		in_addr = inet_addr(tmphbuf);
		hp = gethostbyaddr((char *) &in_addr, sizeof(struct in_addr), AF_INET);
	}
	else
	{
		register int ret;

# ifdef NAMED_BIND
		/*
		** See note in makeconnection() above for why we disable
		** recursive domain matching.  -pbp
		*/
		_res.options &= (~RES_DNSRCH & 0xffff);

		/*
		** But make sure default domain qualification is enabled -
		** it may have been disabled in deliver.c.  -nr
		*/
		_res.options |= RES_DEFNAMES ;
# endif /* NAMED_BIND */

		hp = gethostbyname(hbuf);
		if (hp == NULL)
		{
			/* try lowercase version */
			(void) strcpy(tmphbuf, hbuf);
			(void) makelower(tmphbuf);
			/* Could be just an MX record; look for anything */
			ret = getcanonname(tmphbuf,sizeof(tmphbuf));
			if (ret != TRUE)
			{
				if (tTd(9, 1))
					printf("maphostname(%s, %d) => %.*s\n",
						hbuf, hbsize, hbsize-1,
						hp ? hp->h_name : "NOT_FOUND");
				return FALSE;
			}
			strcpy(hbuf,tmphbuf);
			return TRUE;
		}
	}
	if (tTd(9, 1))
		printf("maphostname(%s, %d) => %.*s\n",
			hbuf, hbsize, hbsize-1, hp ? hp->h_name : "NOT_FOUND");
	if (hp == NULL)
		return FALSE;

	if (strlen(hp->h_name) >= hbsize)
		hp->h_name[hbsize - 1] = '\0';
	(void) strcpy(hbuf, hp->h_name);
	return TRUE;
}
#endif

#else /* DAEMON */
/* new page */
/* code for systems without sophisticated networking */

/*
**  MYHOSTNAME -- stub version for case of no daemon code.
**
**	Can't convert to upper case here because might be a UUCP name.
**
**	Mark, you can change this to be anything you want......
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	register FILE *f;

	hostbuf[0] = '\0';
	f = fopen("/usr/include/whoami", "r");
	if (f != NULL)
	{
		(void) fgets(hostbuf, size, f);
		fixcrlf(hostbuf, TRUE);
		(void) fclose(f);
	}
	return (NULL);
}

/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		An exit code telling if the hostname was found and
**		canonicalized.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

/*ARGSUSED*/
bool
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	return (FALSE);
}

#endif /* DAEMON */

#ifndef __HELIOS

/* new page */
/*
**  MAPINIT -- Open and (re)initialize a dbm database
**
**	Parameters:
**		c -- the (one character) name of the database
**
**	Returns:
**		An exit code telling if we could open the database.
**
*/
#if defined(NDBM) || defined(OTHERDBM)
bool
mapinit(c)
	char c;
{
	struct stat stb;
	struct dbm_table *db;
	int k;
	char buf[MAXNAME];

	db = &DbmTab[c & 0177];

	if (db->db_name == NULL)
	{
		syserr("database '%c' has not been defined", c);
		return FALSE;
	}

# ifdef YPMARK
	/*
	 * Yellow pages are always supposed to be ready
	 */
	if (db->db_name[0] == YPMARK)
		return TRUE;
# endif /* YPMARK */

	/*
	 * Have we already (unsuccessfully) tried to open it?
	 */
	if (db->db_dbm == DB_NOSUCHFILE)
	{
		if (tTd(60, 1))
			printf("mapinit(%c) => NO_FILE\n", c);
		return FALSE;
	}

	/*
	 * If it already is open, check if it has been changed.
	 */
	(void) sprintf(buf, "%s%s", db->db_name, DB_DIREXT);
	if (db->db_dbm != DB_NOTYETOPEN)
	{
		if (stat(buf, &stb) < 0 && (Xsleep(30), stat(buf, &stb) < 0))
		{
			syserr("somebody removed %s for db '%c'", buf, c);
			db->db_dbm = DB_NOSUCHFILE;
			if (tTd(60, 1))
				printf("mapinit(%c) => FILE_REMOVED\n", c);
			return FALSE;
		}
		if (db->db_mtime != stb.st_mtime)
		{
			if (tTd(60, 1))
				printf("database '%c' [%s] has changed; reopening it\n",
				    c, db->db_name);
			(void) dbm_close(db->db_dbm);
			db->db_dbm = DB_NOTYETOPEN;
		}
	}

	/*
	 * Initialize database if not already open (r/w for aliases iff init)
	 */
	if (db->db_dbm == DB_NOTYETOPEN)
	{
		db->db_dbm = dbm_open(db->db_name,
		    (OpMode == MD_INITALIAS && c == DB_ALIAS)
		    ? O_RDWR : O_RDONLY, 0);
		if (db->db_dbm == DB_NOSUCHFILE)
		{
			/* try once more */
			Xsleep(30);
			db->db_dbm = dbm_open(db->db_name,
			    (OpMode == MD_INITALIAS && c == DB_ALIAS)
			    ? O_RDWR : O_RDONLY, 0);
		}
		if (db->db_dbm == DB_NOSUCHFILE)
		{
			syserr("can't open database '%c' [%s]", c, db->db_name);
			if (tTd(60, 1))
				printf("mapinit(%c) => CAN'T OPEN %s\n",
				    c, db->db_name);
			return FALSE;
		}
		if (stat(buf, &stb) < 0 && (Xsleep(30), stat(buf, &stb) < 0))
		{
			syserr("can't stat %s", buf);
			if (tTd(60, 1))
				printf("mapinit(%c) => FILE_REMOVED\n", c);
			return FALSE;
		}
		db->db_mtime = stb.st_mtime;

# ifndef GDBM
		/*
		 * Make sure the database isn't being updated
		 */
		if (flock(dbm_dirfno(db->db_dbm),
		    ((OpMode == MD_INITALIAS && c == DB_ALIAS) ? LOCK_EX : LOCK_SH) | LOCK_NB) < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				if (tTd(60, 1))
					printf("%s%s is locked, waiting...\n",
					    db->db_name, DB_DIREXT);
				(void) flock(dbm_dirfno(db->db_dbm),
					    ((OpMode == MD_INITALIAS && c == DB_ALIAS) ? LOCK_EX : LOCK_SH));
			}
			else
				syserr("flock failed for db %c [%s], fd %d",
				    c, db->db_name, dbm_dirfno(db->db_dbm));
		}
		(void) flock(dbm_dirfno(db->db_dbm), LOCK_UN);
# endif /* !GDBM */
	}
	return TRUE;
}
/* new page */
/*
**  MAPKEY -- Search a dbm database.
**
**	Search the named database using the given key.  If
**	a result is found, sprintf the argument through the
**	result back into the key and return TRUE;
**	otherwise return FALSE and do nothing.
**
**	Keysize may also be given as zero, in which case the
**	sprintf'ed result is returned if the key matched.
**
**	Parameters:
**		c -- the database
**		key -- search string
**		keysiz -- size of key
**		arg -- sprintf argument
**
**	Returns:
**		An exit code telling if there was a match.
**
**	Side Effects:
**		The key is rewritten to reflect what was found
**		in the database.
*/

char *
mapkey(c, key, keysiz, arg)
	char c;
	const char *arg;
	char *key;
	int keysiz;
{
	struct dbm_table *db;
	XDATUM dkey, result;
	static char lowkey[MAXLINE+1];
# ifdef YPMARK
	static char *yp_domain = NULL;
# endif /* YPMARK */

	db = &DbmTab[c & 0177];

	if (tTd(60, 1))
		printf("mapkey('%c', \"%s\", \"%s\") => ",
		    c, key, arg ? arg : "--");

	/*
	 * Init the database; return if failure
	 */
	if (!mapinit(c))
		return NULL;

	/*
	 * Normalize key (ie turn it to lowercase)
	 */
	(void) strcpy(lowkey, key);
	(void) makelower(lowkey);

# ifdef YPMARK
	/*
	 * Test for yellow page database first
	 */
	if (db->db_name[0] == YPMARK)
	{
		if (yp_domain == NULL)
			(void) yp_get_default_domain(&yp_domain);

	/*
	 * We include the null after the string, but Sun doesn't
	 */
		if (yp_match(yp_domain, &db->db_name[1], lowkey,
		    strlen(key)+1, &result.dptr, &result.dsize) != 0 &&
		    yp_match(yp_domain, &db->db_name[1], lowkey,
		    strlen(key), &result.dptr, &result.dsize) != 0)
			result.dptr = NULL;
		else
			/* smash newline */
			result.dptr[result.dsize] = '\0';
	}
	else
	{
# endif /* YPMARK */
		/*
		 * Go look for matching dbm entry
		 */
		dkey.dptr = lowkey;
		dkey.dsize = strlen(dkey.dptr) + 1;
		result = dbm_fetch(db->db_dbm, dkey);
# ifdef YPMARK
	}
# endif /* YPMARK */

	/*
	 * Well, were we successful?
	 */
	if (result.dptr == NULL)
	{
		if (tTd(60, 1))
			printf("NOT_FOUND\n");
		return NULL;
	}

	/*
	 * Yes, rewrite result if sprintf arg was given.
	 */
	if (strlen(result.dptr) > MAXLINE)
	{
		syserr("mapkey: strlen(result.dptr) (%d) > %d\n",
			strlen(result.dptr), MAXLINE);
# ifdef LOG
		syslog(LOG_ALERT, "mapkey: strlen(result.dptr) (%d) > %d\n",
			strlen(result.dptr), MAXLINE);
# endif /* LOG */
	}
	if (arg == NULL)
		(void) strcpy(lowkey, result.dptr);
	else
		(void) sprintf(lowkey, result.dptr, arg);
	if (strlen(lowkey) > MAXLINE)
	{
		syserr("mapkey: strlen(lowkey) (%d) > %d\n",
			strlen(lowkey), MAXLINE);
# ifdef LOG
		syslog(LOG_ALERT, "mapkey: strlen(lowkey) (%d) > %d\n",
			strlen(lowkey), MAXLINE);
# endif /* LOG */
	}

	/*
	 * if keysiz is zero, that means we should return a string from
	 * the heap
	 */
	if (keysiz == 0)
		key = newstr(lowkey);
	else
	{
		if (strlen(lowkey)+1 > keysiz)
		{
			syserr("mapkey: result \"%s\" too long after expansion\n",
			    lowkey);
			lowkey[keysiz-1] = '\0';
		}
		(void) strcpy(key, lowkey);
	}
	if (tTd(60, 1))
		printf("%s\n", key);

	return (key);
}

#else /* !NDBM && !OTHERDBM */

/* should really read the table into the stab instead */
/*ARGSUSED*/
char *
mapkey(db, key, keysiz, arg)
	char db;
	const char *arg;
	char *key;
	int keysiz;
{
	return NULL;
}
#endif /* NDBM || OTHERDBM */

#endif
