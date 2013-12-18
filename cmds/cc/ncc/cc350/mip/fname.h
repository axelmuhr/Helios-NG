#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * fname.h:
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:01:50 $
 * Revising $Author: nickc $
 */

/*
 * This module provides support for interpreting a 'logical' file name and
 * generating a 'real' file name from it. Logical file names are assumed to
 * be in Acorn, Unix, or MS-DOS format. Depending on which host this code
 * is notionally running on (determined by the 'type' argument to fname_parse),
 * an attempt is made to interpret this logical filename as folows:-
 *
 * 1.  Matching proceeds using longer and longer 'tails' of the logical name
 *     (i.e. starting with the last character, then the last 2, ...).
 *
 * 2.  For each tail, we try to match it with a number of candidate types
 *     until either we get a definite match or until the whole filename
 *     fails to match definitely.
 *
 * 3.  The default order of matching is:
 *         under Acorn:  Acorn-like, then Unix-like, then MS-DOS-like.
 *         under MS-DOS: MS-DOS-like, then Unix-like, then Acorn-like.
 *         under Unix:   Unix-like.
 *
 * 4.  The type of an indefinite match defaults to the type passed to
 *     fname_parse.
 *
 * 5.  Under Acorn, certain suffixes are recognised for 'inversion' (i.e.
 *     foo.c <--> c.foo). The 'suffixes' argument to fname_parse is a case-
 *     sensitive list of the extensions you are prepared to invert. 
 */

#ifndef __fname__h
#define __fname__h

#define  FNAME_DFLT      0
#define  FNAME_ACORN     1
#define  FNAME_MSDOS     2
#define  FNAME_UNIX      3
#define  FNAME_TYPEMASK  7
#define  FNAME_ROOTED  256

typedef struct
{   /* The source string is parsed into path, root and extension segments, */
    /* described by the <path, plen>, <root, rlen>, <extn, elen> pairs.    */

    const char *path, *root, *extn;
    int plen, rlen, elen;

    /* 'type' is set to the assumed type of the parsed string. If it is an */
    /* 'absolute' pathname then the FNAME_ROOTED bit is also set. This bit */
    /* is also maintained by fname_unparse, but will only be altered by it */
    /* if the <path, plen> pair has been changed between calling ..._parse */
    /* and ..._unparse. 'pathsep' is set to the path-separator char for    */
    /* filenames of type 'type'.                                           */

    int type;
    int pathsep;

    /* un_pathlen is set by fname_unparse to the length of the path prefix */
    /* (including trailing path separator) generated in the output style.  */

    int un_pathlen;
} UnparsedName;

int fname_parse(const char s[], int type, const char *suffixes,
                UnparsedName *un);
/*
 * Parse a presented file-name into a (mostly) system-independent
 * representation called an UnparsedName.
 *
 * NOTE: 'suffixes' is a space-separated, case-sensitive list of extensions
 *       which are to be recognised. This mostly controls the mapping of
 *       foo.c <--> c.foo under Acorn and other BBC systems.
 *
 * NOTE: On entry, 'type' is the type of the system under which fname is
 *       notionally running: FNAME_ACORN, FNAME_MSDOS, or FNAME_UNIX. Any
 *       other value causes the type to be set to FNAME_DFLT.
 *
 *       On exit, type has been updated so that (type & FNAME_TYPEMASK) gives
 *       the type of the recognised file name. (type & FNAME_ROOTED) is non-0
 *       if the file name was rooted (e.g. /usr/... \x\... $.foo... &.foo...).
 *
 * NOTE: un_pathsep is set to the path-separator appropriate to 'type'.
 *
 * Returns: The type assumed for the presented file-name or FNAME_DFLT.
 */

int fname_unparse(UnparsedName *un, int type, char *buffer, int maxlen);
/*
 * Generate a textual file-name for system 'type' from UnparsedName un.
 * The filename is put in 'buffer' which has length 'maxlen'.
 *
 * If the buffer overflows, unparse returns -1, otherwise the number of
 * characters deposited in the buffer.
 *
 * 'un.un_pathlen' is set to the length of the path prefix, including
 * its terminating path separator (or 0, if there is no path prefix).
 *
 * If the generated filename is rooted, then the 'type' field of 'un'
 * has its FNAME_ROOTED bit set, otherwise cleared.
 */

#ifdef FNAME_SET_TRY_ORDER

extern int fname_set_try_order(int what, int try1, int try2, int try3);
/*
 * Set the order in which recognition ties are broken.
 * 'what' is the system under which fname is notionally running.
 * 'try1', 'try2', and 'try3' (if non-zero) should be an ordering
 * amongst FNAME_ACORN, FNAME_UNIX, and FNAME_MSDOS.
 * The default tie-breaking order is as follows:-
 *   under Acorn:   Acorn, then Unix, then MS-Dos
 *   under MS-Dos:  MS-Dos, then Unix, then Acorn
 *   under Unix:    Unix.
 * Here, MS-Dos should be interpreted as SpringBoard.
 */

#endif /* FNAME_SET_TRY_ORDER */

#endif
