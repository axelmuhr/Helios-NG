/*
 * @(#)open3.h 1.4 87/11/11	Public Domain.
 *
 * open3.h -- #defines for the various flags for the Sys V style 3-argument
 * open() call.  On BSD or System 5, the system already has this in an
 * include file.  This file is needed for V7 and MINIX systems for the
 * benefit of open3() in port.c, a routine that emulates the 3-argument
 * call using system calls available on V7/MINIX. 
 *
 * This file is needed by PD tar even if we aren't using the
 * emulator, since the #defines for O_WRONLY, etc. are used in
 * a couple of places besides the open() calls, (e.g. in the assignment
 * to openflag in extract.c).  We just #include this rather than
 * #ifdef them out.
 *
 * Written 6/10/87 by rmtodd@uokmax (Richard Todd).
 *
 * The names have been changed by John Gilmore, 31 July 1987, since
 * Richard called it "bsdopen", and really this change was introduced in
 * AT&T Unix systems before BSD picked it up.
 $Header: /hsrc/cmds/gnu/tar/RCS/open3.h,v 1.1 1990/08/28 13:18:12 james Exp $ */

/* Only one of the next three should be specified */
#define O_RDONLY	 0 /* only allow read */
#define	O_WRONLY	 1 /* only allow write */
#define	O_RDWR		 2 /* both are allowed */

/* The rest of these can be OR-ed in to the above. */
/*
 * O_NDELAY isn't implemented by the emulator.  It's only useful (to tar) on
 * systems that have named pipes anyway; it prevents tar's hanging by
 * opening a named pipe.  We #ifndef it because some systems already have
 * it defined.
 */
#ifndef O_NDELAY
#define O_NDELAY	 4 /* don't block on opening devices that would
			    * block on open -- ignored by emulator. */
#endif
#define O_CREAT		 8 /* create file if needed */
#define O_EXCL		16 /* file cannot already exist */
#define O_TRUNC		32 /* truncate file on open */
#define O_APPEND	64 /* always write at end of file -- ignored by emul */

#ifdef EMUL_OPEN3
/*
 * make emulation transparent to rest of file -- redirect all open() calls
 * to our routine
 */
#define open	open3
#endif
