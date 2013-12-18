/*
 * Header file for public domain tar (tape archive) program.
 *
 * @(#)tarpriv.h 1.24 87/11/06	Public Domain.
 *
 * Created 25 August 1985 by John Gilmore, ihnp4!hoptoad!gnu.
 * Actually created from distributed tar.h which is now
 * chopped up into <tar.h> and "tarpriv.h"
 *
 * $Header: /hsrc/cmds/gnu/tar/RCS/tarpriv.h,v 1.2 1992/06/27 12:16:32 bart Exp $
 *
 */

/*
 * Kludge for handling systems that can't cope with multiple
 * external definitions of a variable.  In ONE routine (tar.c),
 * we #define TAR_EXTERN to null; here, we set it to "extern" if
 * it is not already set.
 */
#ifndef TAR_EXTERN
#define TAR_EXTERN extern
#endif

#include "tar.h"

#define record hblock

#ifdef POSIX
#include <fcntl.h>
#include <sys/stat.h>
#endif

#ifdef __HELIOS
#define symlink(a,b) link(a,b)
#endif

/* The checksum field is filled with this while the checksum is computed. */
#define	CHKBLANKS	"        "	/* 8 blanks, no null */

/* The linkflag defines the type of file */
#define	LF_OLDNORMAL	AREGTYPE	/* Normal disk file, Unix compat */
#define	LF_NORMAL	REGTYPE		/* Normal disk file */
#define	LF_LINK		LNKTYPE		/* Link to previously dumped file */
#define	LF_SYMLINK	SYMTYPE		/* Symbolic link */
#define	LF_CHR		CHRTYPE		/* Character special file */
#define	LF_BLK		BLKTYPE		/* Block special file */
#define	LF_DIR		DIRTYPE		/* Directory */
#define	LF_FIFO		FIFOTYPE	/* FIFO special file */
#define	LF_CONTIG	CONTTYPE	/* Contiguous file */
/* Further link types may be defined later. */

/*
 * Exit codes from the "tar" program
 */
#define	EX_SUCCESS	0		/* success! */
#define	EX_ARGSBAD	1		/* invalid args */
#define	EX_BADFILE	2		/* invalid filename */
#define	EX_BADARCH	3		/* bad archive */
#define	EX_SYSTEM	4		/* system gave unexpected error */


/*
 * Global variables
 */
TAR_EXTERN union record	*ar_block;	/* Start of block of archive */
TAR_EXTERN union record	*ar_record;	/* Current record of archive */
TAR_EXTERN union record	*ar_last;	/* Last+1 record of archive block */
TAR_EXTERN char		ar_reading;	/* 0 writing, !0 reading archive */
TAR_EXTERN int		blocking;	/* Size of each block, in records */
TAR_EXTERN int		blocksize;	/* Size of each block, in bytes */
TAR_EXTERN char		*ar_file;	/* File containing archive */
TAR_EXTERN char		*name_file;	/* File containing names to work on */
TAR_EXTERN char		*tar;		/* Name of this program */

/*
 * Flags from the command line
 */
TAR_EXTERN char	f_reblock;		/* -B */
TAR_EXTERN char	f_create;		/* -c */
TAR_EXTERN char	f_diff;			/* -d */
TAR_EXTERN char	f_dironly;		/* -D */
TAR_EXTERN char	f_follow_links;		/* -h */
TAR_EXTERN char	f_ignorez;		/* -i */
TAR_EXTERN char	f_keep;			/* -k */
TAR_EXTERN char f_local_filesys;	/* -l */
TAR_EXTERN char	f_modified;		/* -m */
TAR_EXTERN char	f_oldarch;		/* -o */
TAR_EXTERN char	f_use_protection;	/* -p */
TAR_EXTERN char	f_sayblock;		/* -R */
TAR_EXTERN char	f_sorted_names;		/* -s */
TAR_EXTERN char	f_list;			/* -t */
TAR_EXTERN char	f_namefile;		/* -T */
TAR_EXTERN char	f_verbose;		/* -v */
TAR_EXTERN char	f_extract;		/* -x */
TAR_EXTERN char	f_compress;		/* -z */

/*
 * We now default to Unix Standard format rather than 4.2BSD tar format.
 * The code can actually produce all three:
 *	f_standard	ANSI standard
 *	f_oldarch	V7
 *	neither		4.2BSD
 * but we don't bother, since 4.2BSD can read ANSI standard format anyway.
 * The only advantage to the "neither" option is that we can cmp(1) our
 * output to the output of 4.2BSD tar, for debugging.
 */
#define		f_standard		(!f_oldarch)

/*
 * Structure for keeping track of filenames and lists thereof.
 */
struct name {
	struct name	*next;
	short		length;		/* cached strlen(name) */
	char		found;		/* A matching file has been found */
	char		firstch;	/* First char is literally matched */
	char		regexp;		/* This name is a regexp, not literal */
	char		name[NAMSIZ+1];
};

TAR_EXTERN struct name	*namelist;	/* Points to first name in list */
TAR_EXTERN struct name	*namelast;	/* Points to last name in list */

TAR_EXTERN int		archive;	/* File descriptor for archive file */
TAR_EXTERN int		errors;		/* # of files in error */

/*
 *
 * Due to the next struct declaration, each routine that includes
 * "tar.h" must also include <sys/types.h>.  I tried to make it automatic,
 * but System V has no defines in <sys/types.h>, so there is no way of
 * knowing when it has been included.  In addition, it cannot be included
 * twice, but must be included exactly once.  Argghh!
 *
 * Thanks, typedef.  Thanks, USG.
 */
struct link {
	struct link	*next;
	dev_t		dev;
	ino_t		ino;
	short		linkcount;
	char		name[NAMSIZ+1];
};

TAR_EXTERN struct link	*linklist;	/* Points to first link in list */


/*
 * Error recovery stuff
 */
TAR_EXTERN char		read_error_flag;


/*
 * Declarations of functions available to the world.
 */
union record *findrec();
void userec();
union record *endofrecs();
void anno();
#define	 annorec(stream, msg)	anno(stream, msg, 0)	/* Cur rec */
#define	annofile(stream, msg)	anno(stream, msg, 1)	/* Saved rec */

