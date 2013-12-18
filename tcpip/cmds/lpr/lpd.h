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
 *
 *	@(#)lp.h	5.3 (Berkeley) 6/30/88
 */

/*
 * Global definitions for the line printer system.
 */

/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/lpd.h,v 1.1 1992/01/16 18:03:52 craig Exp $";
*/

#include <stdio.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/socket.h>

#ifndef __HELIOS
#ifdef HAVE_UNSOCK
#include <sys/un.h>
#endif
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/wait.h>
#include <sgtty.h>
#include <ctype.h>
#include <errno.h>

#ifndef __HELIOS
#include "lp.local.h"
#else
#include "lp_local.h"
#include <stdlib.h>
#include <string.h>
#endif
