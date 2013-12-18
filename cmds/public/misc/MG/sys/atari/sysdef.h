/* sysdef.h --  MG header file for Atari ST
 *
 * author :  Sandra Loosemore
 * date   :  24 Oct 1987
 *
 */

#define	NULL	0L			/* So who needs stdio.h?	*/
#include	<osbind.h>

/* This definition might be missing from your osbind.h */

#ifndef Supexec
#define Supexec(a)  xbios(38,a)
#endif


#define	KBLOCK		512		/* Kill grow.			*/
#define	GOOD		0		/* Good exit status.		*/
#define NO_VOID_TYPE	1		/* "void" is not a builtin type */
#ifdef MWC
#undef	NO_VOID_TYPE
#define	ZEROARRAY			/* See def.h			*/
    /*
     * Using the MWC shell, ARGV is always the last environment entry
     * (if ARGV is not defined, the program was not started from the
     * MWC shell).  The last environment variable is followed by
     * the string pointed to by argv[0], then argv[1], etc.  This
     * means that the non-MWC getenv() in misc.c will take command line
     * args as environment variables.  It also means that if spawn()
     * (misc.c) runs the MWC shell, the shell takes this program's args
     * as its own.  So here we truncate the environment by zeroing out
     * the first character of argv[0], thus working around both problems.
     */
#define SYSINIT (*(argv[0]) = 0)	/* run in main() 		*/
#endif /* MWC */

#define MALLOCROUND(m) ((m)+=1,(m)&=~1)	/* 2-byte blocks (see alloc.c)	*/
#define LOCAL_VARARGS	1		/* For echo.c			*/
#define RSIZE long			/* Type for file/region sizes   */
#define KCHAR int                       /* 16 bits for keystrokes       */


/* Enable various things */

#define DO_METAKEY	1		/* Meta key code */
#define METABIT		0x200
#define FKEYS		1		/* Enable fkey code */
#define GOSMACS 	1		/* Goslings compatibility functions */
#define PREFIXREGION	1		/* Enable prefix-region function */
#define BSMAP		FALSE		/* BSMAP code, default to off */

/* Disable some features for now. */

#define NO_BACKUP	1


/* Alcyon thinks subtracting two pointers gives a long.  Cast it to int.
 */

#define OFFSET(type,member) ((int)((char *)&(((type *)0)->member)-(char *)((type *)0)))
#ifdef MWC
#undef	OFFSET
#endif /* MWC */

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */

#define	BDC1	'\\'			/* Buffer names.		*/
#define	BDC2	':'


#define fncmp strcmp			/* All filenames are lowercased */
extern char *strncpy();

