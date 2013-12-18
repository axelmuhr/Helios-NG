/*
 * [.vms]vms_popen.c -- substitute routines for missing pipe calls.
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

#ifndef NO_VMS_PIPES

#include "awk.h"	/* really "../awk.h" */
#include <stdio.h>

#ifndef PIPES_SIMULATED

FILE *
popen( const char *command, const char *mode )
{
    fatal(" Cannot open pipe `%s' (not implemented)", command);
    /* NOT REACHED */
    return 0;
}

int
pclose( FILE *current )
{
    fatal(" Internal error ('pclose' not implemented)");
    /* NOT REACHED */
    return -1;
}

int
fork()
{
    fatal(" Internal error ('fork' not implemented)");
    /* NOT REACHED */
    return -1;
}

#else	PIPES_SIMULATED
	/*
	 * Simulate pipes using temporary files; hope that the user
	 * doesn't expect pipe i/o to be interleaved with other i/o ;-}.
	 *
	 * This is essentially the same as the MSDOS version.  The
	 * difference is that redirection is handled using LIB$SPAWN
	 * rather than constructing a command for system() which uses
	 * '<' or '>'.
	 */
#include "vms.h"
#include <errno.h>

typedef enum { unopened = 0, reading, writing } pipemode;
static
struct {
    char *command;
    char *name;
    pipemode pmode;
} pipes[_NFILE];

FILE *
popen( const char *command, const char *mode )
{
    FILE *current;
    char *name, *mktemp();
    int   cur, strcmp();
    pipemode curmode;

    if (strcmp(mode, "r") == 0)
	curmode = reading;
    else if (strcmp(mode, "w") == 0)
	curmode = writing;
    else
	return NULL;

    /* make a name for the temporary file */
    if ((name = mktemp(strdup("sys$scratch:pipe_XXXX.tmp"))) == 0)
	return NULL;

    if (curmode == reading) {
	/* an input pipe reads a temporary file created by the command */
	vms_execute(command, (char *)0, name);	/* 'command >tempfile' */
    }
    if ((current = fopen(name, mode)) == NULL) {
	free(name);
	return NULL;
    }
    cur = fileno(current);
    pipes[cur].name = name;
    pipes[cur].pmode = curmode;
    pipes[cur].command = strdup(command);
    return current;
}

int
pclose( FILE *current )
{
    int rval, cur = fileno(current);

    if (pipes[cur].pmode == unopened)
	return -1;	/* should never happen */

    rval = fclose(current);	/* close temp file; if reading, we're done */
    if (pipes[cur].pmode == writing) {
	/* an output pipe feeds the temporary file to the other program */
	rval = vms_execute(pipes[cur].command, pipes[cur].name, (char *)0);
    }
    /* clean up */
    unlink(pipes[cur].name);	/* get rid of the temporary file */
    pipes[cur].pmode = unopened;
    free(pipes[cur].name),  pipes[cur].name = 0;
    free(pipes[cur].command),  pipes[cur].command = 0;
    return rval;
}

    /*
     * Create a process and execute a command in it.  This is essentially
     * the same as system() but allows us to specify SYS$INPUT (stdin)
     * and/or SYS$OUTPUT (stdout) for the process.
     * [With more work it could truly simulate a pipe using mailboxes.]
     */
int
vms_execute( const char *command, const char *input, const char *output )
{
    Dsc cmd, in, out, *in_p, *out_p;
    u_long sts, cmpltn_sts, LIB$SPAWN();

    cmd.len = strlen(cmd.adr = (char *)command);
    if (input)
	in.len = strlen(in.adr = (char *)input),  in_p = &in;
    else
	in_p = 0;
    if (output)
	out.len = strlen(out.adr = (char *)output),  out_p = &out;
    else
	out_p = 0;

    sts = LIB$SPAWN(&cmd, in_p, out_p, (long *)0,
		    (Dsc *)0, (u_long *)0, &cmpltn_sts);

    if (vmswork(sts) && vmsfail(cmpltn_sts))  sts = cmpltn_sts;
    if (vmsfail(sts)) {
	errno = EVMSERR,  vaxc$errno = sts;
	return -1;
    } else
	return 0;
}

#endif	/* PIPES_SIMULATED */

#endif	/*!NO_VMS_PIPES*/
