/*
 * Copyright (c) 1985 Regents of the University of California.
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
 *
 *	@(#)ftp_var.h	5.5 (Berkeley) 6/29/88
 */

#ifndef EXTERN
#define EXTERN extern
#endif

/*
 * FTP global variables.
 */

/*
 * Options and other state info.
 */
EXTERN int	trace;			/* trace packets exchanged */
EXTERN int	hash;			/* print # for each buffer transferred */
extern int	sendport;		/* use PORT cmd for each data connection */
EXTERN int	verbose;		/* print messages coming back from server */
extern int	connected;		/* connected to server */
EXTERN int	fromatty;		/* input is from a terminal */
EXTERN int	interactive;		/* interactively prompt on m* cmds */
EXTERN int	debug;			/* debugging level */
EXTERN int	bell;			/* ring bell on cmd completion */
EXTERN int	doglob;			/* glob local file names */
EXTERN int	autologin;		/* establish user account on connection */
EXTERN int	proxy;			/* proxy server connection active */
EXTERN int	proxflag;		/* proxy connection exists */
EXTERN int	sunique;		/* store files on server with unique name */
EXTERN int	runique;		/* store local files with unique name */
EXTERN int	mcase;			/* map upper to lower case for mget names */
EXTERN int	ntflag;			/* use ntin ntout tables for name translation */
EXTERN int	mapflag;		/* use mapin mapout templates on file names */
EXTERN int	code;			/* return/reply code for ftp command */
EXTERN int	crflag;			/* if 1, strip car. rets. on ascii gets */
EXTERN char	pasv[64];		/* passive port for proxy data connection */
EXTERN char	*altarg;		/* argv[1] with no shell-like preprocessing  */
EXTERN char	ntin[17];		/* input translation table */
EXTERN char	ntout[17];		/* output translation table */
#include <sys/param.h>
EXTERN char	mapin[MAXPATHLEN];	/* input map template */
EXTERN char	mapout[MAXPATHLEN];	/* output map template */
EXTERN char	typename[32];		/* name of file transfer type */
EXTERN int	type;			/* file transfer type */
EXTERN char	structname[32];		/* name of file transfer structure */
EXTERN int	stru;			/* file transfer structure */
EXTERN char	formname[32];		/* name of file transfer format */
EXTERN int	form;			/* file transfer format */
EXTERN char	modename[32];		/* name of file transfer mode */
EXTERN int	mode;			/* file transfer mode */
EXTERN char	bytename[32];		/* local byte size in ascii */
EXTERN int	bytesize;		/* local byte size in binary */

EXTERN char	*hostname;		/* name of host connected to */

EXTERN struct	servent *sp;		/* service spec for tcp/ftp */

#include <setjmp.h>
EXTERN jmp_buf	toplevel;		/* non-local goto stuff for cmd scanner */

EXTERN char	line[200];		/* input line buffer */
EXTERN char	*stringbase;		/* current scan point in line buffer */
EXTERN char	argbuf[200];		/* argument storage buffer */
EXTERN char	*argbase;		/* current storage point in arg buffer */
EXTERN int	margc;			/* count of arguments on input line */
EXTERN char	*margv[20];		/* args parsed from input line */
EXTERN int     cpend;                  /* flag: if != 0, then pending server reply */
EXTERN int	mflag;			/* flag: if != 0, then active multi command */

EXTERN int	options;		/* used during socket creation */

/*
 * Format of command table.
 */
struct cmd {
	char	*c_name;	/* name of command */
	char	*c_help;	/* help string */
	char	c_bell;		/* give bell when command completes */
	char	c_conn;		/* must be connected to use command */
	char	c_proxy;	/* proxy server may execute */
	int	(*c_handler)();	/* function to call */
};

struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

EXTERN int macnum;			/* number of defined macros */
EXTERN struct macel macros[16];
EXTERN char macbuf[4096];

#ifndef __HELIOS
EXTERN	char *tail();
EXTERN	char *index();
EXTERN	char *rindex();
EXTERN	char *remglob();
EXTERN	int errno;
EXTERN	char *mktemp();
EXTERN	char *strncpy();
EXTERN	char *strncat();
EXTERN	char *strcat();
EXTERN	char *strcpy();
#endif
