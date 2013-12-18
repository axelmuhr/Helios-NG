/*
 * vms_misc.c -- sustitute code for missing/different run-time library routines.
 */

/* 
 * Copyright (C) 1991 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"        /* really "../awk.h" */
#include <ssdef.h>
#include <stsdef.h>

    /*
     * VMS uses a completely different status scheme (odd => success,
     * even => failure), so we'll trap calls to exit() and alter the
     * exit status code.  [VAXC can't handle this as a macro.]
     */
#ifdef exit
# undef exit
#endif
void
vms_exit( int errno )		/* note: local override of global 'errno' */
{
    exit(errno == 0 ? SS$_NORMAL : (SS$_ABORT | STS$M_INHIB_MSG));
}
#define exit(v) vms_exit(v)

    /*
     * In VMS's VAXCRTL, strerror() takes an optional second argument.
     *  #define strerror(errnum) strerror(errnum,vaxc$errno)
     * is all that's needed, but VAXC can't handle that (gcc can).
     * [The 2nd arg is used iff errnum == EVMSERR.]
     */
#ifdef strerror
# undef strerror
#endif
/* vms_strerror() -- convert numeric error code into text string */
char *
vms_strerror( int errnum )
{
    extern char *strerror( /* int, ... */ );
    return ( errnum != EVMSERR ? strerror(errnum)
			       : strerror(EVMSERR, vaxc$errno) );
}
# define strerror(v) vms_strerror(v)

    /*
     * Miscellaneous utility routine, not part of the run-time library.
     */
/* vms_strdup() - allocate some new memory and copy a string into it */
char *
vms_strdup( const char *str )
{
    char *result;
    int len = strlen(str);

    emalloc(result, char *, len+1, "strdup");
    return strcpy(result, str);
}

    /*
     * VAXCRTL does not contain unlink().  This replacement has limited
     * functionality which is sufficient for GAWK's needs.  It works as
     * desired even when we have the file open.
     */
/* unlink(file) -- delete a file (ignore soft links) */
int
unlink( const char *file_spec ) {
    char tmp[255+1];			/*(should use alloca(len+2+1)) */
    extern int delete(const char *);

    strcpy(tmp, file_spec);		/* copy file name */
    if (strchr(tmp, ';') == NULL)
	strcat(tmp, ";0");		/* append version number */
    return delete(tmp);
}

    /*
     * Check for attempt to (re-)open known file.
     */
/* vms_devopen() - check for "SYS$INPUT" or "SYS$OUTPUT" or "SYS$ERROR" */
int
vms_devopen( const char *name )
{
    FILE *file = NULL;

    if (strncasecmp(name, "SYS$", 4) == 0) {
	name += 4;		/* skip "SYS$" */
	if (strncasecmp(name, "INPUT", 5) == 0)
	    file = stdin,  name += 5;
	else if (strncasecmp(name, "OUTPUT", 6) == 0)
	    file = stdout,  name += 6;
	else if (strncasecmp(name, "ERROR", 5) == 0)
	    file = stderr,  name += 5;
	if (*name == ':')  name++;	/* treat trailing colon as optional */
    }
    /* note: VAXCRTL stdio has extra level of indirection (*file) */
    return (file && *file && *name == '\0') ? fileno(file) : -1;
}

    /*
     * VMS has no timezone support.
     */
/* these are global for use by missing/strftime.c */
char   *tzname[2] = { "local", "" };
int     daylight = 0;

/* dummy to satisfy linker */
void tzset()
{
    return;
}

#ifndef __GNUC__
# ifdef bcopy
#  undef bcopy
# endif
void bcopy( char *src, char *dst, int len )
{
    (void) OTS$MOVE3(len, src, dst);
}
#endif	/*!__GNUC__*/

/*----------------------------------------------------------------------*/
#ifdef NO_VMS_ARGS      /* real code is in "vms/vms_args.c" */
void vms_arg_fixup( int *argc, char ***argv ) { return; }	/* dummy */
#endif

#ifdef NO_VMS_PIPES     /* real code is in "vms/vms_popen.c" */
FILE *popen( const char *command, const char *mode ) {
    fatal(" Cannot open pipe `%s' (not implemented)", command);
    return NULL;
}
int pclose( FILE *current ) {
    fatal(" Cannot close pipe #%d (not implemented)", fileno(current));
    return -1;
}
int fork( void ) {
    fatal(" Cannot fork process (not implemented)");
    return -1;
}
#endif /*NO_VMS_PIPES*/
