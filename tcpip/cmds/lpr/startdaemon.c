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
static char sccsid[] = "@(#)startdaemon.c	5.3 (Berkeley) 6/30/88";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/startdaemon.c,v 1.1 1992/01/16 17:36:22 craig Exp $";
#endif

/*
 * Tell the printer daemon that there are new files in the spool directory.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <sys/hel.h>
#include <string.h>
#include <posix.h>

#ifndef __HELIOS
#ifdef HAVE_UNSOCK
#include <sys/un.h>
#else
#include <netinet/in.h>
#endif
#endif

#ifndef __HELIOS
#include "lp.local.h"
#else
#include "lp_local.h"
#endif

void perr(char *) ;

int startdaemon (char *printer)
{
#ifndef __HELIOS
#ifdef HAVE_UNSOCK
  struct sockaddr_un sun;
  register int s, n;
  char buf[BUFSIZ];

  s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s < 0) {
    perr("socket");
    return(0);
  }
  sun.sun_family = AF_UNIX;
  strcpy(sun.sun_path, SOCKETNAME);
  if (connect(s, &sun, strlen(sun.sun_path) + 2) < 0) {
    perr("connect");
    (void) close(s);
    return(0);
  }
#else
  struct sockaddr_in sin;
  struct hostent *hp;
  struct servent *sp;
  register int s, n;
  char buf[BUFSIZ];

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perr("socket");
    return(0);
  }

  if ((sin.sin_addr.s_addr = gethostid()) < 0) {
    perr("inet_addr");
    close(s);
    return(0);
  }

  if ((sp = getservbyname("local_printer", "tcp")) == NULL) {
      perr("local_printer/tcp: unknown service");
      return(0);
      }
  sin.sin_port = sp->s_port;

  if (connect(s, &sin, sizeof(sin)) < 0) {
    perr("connect");
    (void) close(s);
    return(0);
  }

#endif
#else
  struct sockaddr_hel socket_addr;
  register int s, n;
  char buf[BUFSIZ];

  s = socket(AF_HELIOS, SOCK_STREAM, 0);
  if (s < 0) {
    perr("socket");
    return(0);
  }

  socket_addr.sh_family = AF_HELIOS;
  strcpy(socket_addr.sh_path, SOCKETNAME);
  debugf ("startdaemon: connecting ...") ;
  if (connect(s, (struct sockaddr *) &socket_addr, 
      sizeof (socket_addr)) < 0) {
    perr("connect");
    (void) close(s);
    return(0);
  }
#endif
  (void) sprintf(buf, "\1%s\n", printer);
  n = strlen(buf);
  if (write(s, buf, n) != n) {
    perr("write");
    (void) close(s);
    return(0);
  }
  if (read(s, buf, 1) == 1) {
    if (buf[0] == '\0') {    /* everything is OK */
      (void) close(s);
      return(1);
    }
    putchar(buf[0]);
  }
  while ((n = read(s, buf, sizeof(buf))) > 0)
    fwrite(buf, 1, n, stdout);
  (void) close(s);
  return(0);
}

void perr(char *msg)
{
  extern char *name;
  extern int sys_nerr;
  extern char *sys_errlist[];
  extern int errno;

  printf("%s: %s: ", name, msg);
  fputs(errno < sys_nerr ? sys_errlist[errno] : "Unknown error" , stdout);
  putchar('\n');
}
