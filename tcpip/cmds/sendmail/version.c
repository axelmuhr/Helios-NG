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

#ifndef __HELIOS
#ifndef lint
static char sccsid[] = "@(#)version.c	5.65 (Berkeley) 8/29/90";
#endif /* not lint */

#include "conf.h"

#ifdef MAIL11V3
char	Version[] = "5.65b+";
#else /* !MAIL11V3 */
char	Version[] = "5.65b";
#endif /* MAIL11V3 */

#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/version.c,v 1.12 1992/03/01 18:21:12 craig Exp $";
#include <stdio.h>
#include "conf.h"
#define NAME	"Helios Sendmail"
/*
-- Alpha
-- 1.01 : main.c
        : Close socket descriptor after processing request
-- 1.02 : daemon.c, smtp.c
        : return error code or exit after calling syserr()
-- 1.03 : daemon.c
        : if gethostname() fails, then exit(EX_NOHOST)
-- 1.04 : deliver.c
        : tidied up error handling
-- 1.05 : changed name (above): "Helios Mail" to "Helios Sendmail"
-- 1.06 : main.c, deliver.c
	: tidied up SYSLOG messages (execve)
-- 1.07 : deliver.c
	: tidied up exit procedure when 'mailhost' is not defined
-- Beta (ha ha)
-- 1.00 : Tidied up sendmail.hf - code is the same as Version 1.07 Alpha
-- 1.01 : deliver.c
--      : If connection to mailhost fails, give error message
-- V1.00
--      : Identical to V1.01 Beta
*/
char	Version[] = "V1.00" ;
char *Version_ID ()
{
	static char id [MAXNAME] ;
	(void) sprintf (id, "%s %s", NAME, Version) ;
	return id ;
}
#endif
