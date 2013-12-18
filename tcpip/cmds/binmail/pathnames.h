/*
 * Copyright (c) 1990 The Regents of the University of California.
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
 *
 *	@(#)pathnames.h	5.1 (Berkeley) 4/29/90
 */

/*
static char *rcsid = "$Header: /hsrc/tcpip/cmds/binmail/RCS/pathnames.h,v 1.3 1992/02/06 14:53:31 craig Exp $";
*/

#if defined(__hpux)
#include "flock.h"
#endif /* __hpux */
#include <sys/param.h>

#ifndef __HELIOS
#ifdef BSD4_4
#define _PATH_SENDMAIL	"/usr/sbin/sendmail"
#define _PATH_TMP	"/tmp/maXXXXX"
#define _PATH_MAILDIR	"/var/mail/"
#else /* !BSD4_4 */
#define _PATH_SENDMAIL	"/usr/lib/sendmail"
#define _PATH_TMP	"/tmp/maXXXXX"
#define _PATH_MAILDIR	"/usr/spool/mail/"
#endif /* BSD4_4 */
#else
#define _PATH_SENDMAIL	"/helios/lib/sendmail"
#define _PATH_TMP	"/helios/tmp/maXXXXX"
#define _PATH_MAILDIR	"/helios/local/spool/mail/"
#define _PATH_MORE	"/helios/bin/more"
#define DEAD_LETTER	"dead.letter"
#endif
