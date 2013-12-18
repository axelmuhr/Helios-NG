#ifdef __HELIOS
/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/bsd.h,v 1.1 1992/01/16 18:02:57 craig Exp $";
*/

#ifndef _types_h
#include <sys/types.h>
#endif

extern int openlog (char *, int, int) ;
extern int syslog (int, char *, ... ) ;
extern char *mktemp (char *) ;
extern int fchmod (int, mode_t) ;
extern int getopt (int, char *[], char *) ;
extern int rresvport (int *) ;
extern int utimes (char *, struct timeval *) ;

#endif
