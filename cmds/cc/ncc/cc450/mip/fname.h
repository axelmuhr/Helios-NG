/*
 * fname.h:
 * Copyright (C) Advanced RISC Machines Ltd., 1992.
 */

/*
 * The purpose of the fname module is:
 *
 * -  to make the manipulation of file names independent of host system;
 * -  to support the use of multiple file-naming conventions on one host.
 *
 * Host systems include Unix, DOS, RISC OS and Mac/MPW.
 *
 * In each environment fname can parse:
 *
 *  -  native file names;
 *  -  pseudo Unix file names;
 *  -  Unix file names;
 *
 * and can unparse a parsed file name into the native format.
 *
 * A pseudo Unix file name is one in the format:
 *
 *     <host-volume-name>:/rest-of-unix-file-name
 *
 * Determining how to parse a name is done heuristically. Heuristics are
 * applied in order as follows:
 *
 * 1/  A name starting with: volume-name:/ is a pseudo Unix name.
 *
 * 2/  Under RISC OS, a name starting with: {filing-system:}:mount, $ or & is a
 *     RISC OS name.
 *
 * 3/  A name containing a '/' is a Unix name.
 *
 * Otherwise, the name is a host name.
 *
 * Note that, under RISC OS, name.<extn> is recognised as a RISC OS name.
 * Extension inversion is performed separately and only for a specified
 * list of extensions.
 */

#ifndef __fname__h
#define __fname__h

#define  FNAME_ROOTED    8

#define  FNAME_AS_NAME   0
#define  FNAME_AS_PATH  16

typedef struct
{   /* The source string is parsed into volume, path, root and extension   */
    /* segments, described by the pairs:                                   */
    /*     <vol vlen>, <path, plen>, <root, rlen>, <extn, elen>.           */
    /* These fields and the flags field are notionally private to fname.   */

    const char *vol, *path, *root, *extn;
    unsigned char vlen, plen, rlen, elen;
    char flags;

    /* 'type' is set to the assumed type of the parsed string. If it is an */
    /* 'absolute' pathname then the FNAME_ROOTED bit is also set. This bit */
    /* is also maintained by fname_unparse, but will only be altered by it */
    /* if the <path, plen> pair has been changed between calling ..._parse */
    /* and ..._unparse. 'pathsep' is set to the path-separator char for    */
    /* filenames of type 'type'.                                           */

    char type, pathsep;

    /* un_pathlen is set by fname_unparse to the length of the path prefix */
    /* of the output name generated in the output style, which may be      */
    /* concatenated with a (non-rooted) file name in the same style, to    */
    /* generate a new file name in the same style. For Unix, DOS and Acorn */
    /* styles, the length includes the trailing path separator; in the MAC */
    /* style, the length excludes the path separator because the canonical */
    /* form of a non-rooted file name begins with a path separator.        */

    unsigned char un_pathlen;
} UnparsedName;

void fname_parse(const char s[], const char *suffixes, UnparsedName *un);
/*
 * Parse a presented file-name into a (mostly) system-independent
 * representation called an UnparsedName.
 *
 * NOTE: 'suffixes' is a space-separated, case-sensitive list of extensions
 *       which are to be recognised. This mostly controls the mapping of
 *       foo.c <--> c.foo, etc, under Acorn operating systems.
 *
 * NOTE: On exit, type has been set so that (type & FNAME_ROOTED) is non-0
 *       if the file name was rooted (e.g. /usr/... \x\... $.foo... &.foo...).
 *
 * NOTE: un_pathsep is set to the path-separator appropriate to 'type'.
 */

int fname_unparse(UnparsedName *un, int type, char *buffer, int maxlen);
/*
 * Generate a textual file-name for the host system from UnparsedName un.
 * The filename is put in 'buffer' which has length 'maxlen'.
 *
 * If 'type' is FNAME_AS_NAME then a complete file name is generated.
 * If 'type' is FNAME_AS_PATH then a path prefix is generated which is 
 * suitable for concatenation with a relative file name in the host's
 * canonical format.
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

#endif
