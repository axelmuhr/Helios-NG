/*
 * Header file for public domain tar (tape archive) program.
 *
 * @(#)tar.h 1.24 87/11/06	Public Domain.
 *
 * Created 25 August 1985 by John Gilmore, ihnp4!hoptoad!gnu.
 *
 * Helios port: 2-feb-90 CGS
 *
 * Actually this was created from the distributed "tar.h"
 * which is now in two parts viz.: <tar.h> and "tarpriv.h"
 *
 * $Header: /hsrc/cmds/gnu/tar/RCS/tar.h,v 1.1 1990/08/28 13:20:33 james Exp $
 *
 */

/*
 * Header block on tape.
 *
 * I'm going to use traditional DP naming conventions here.
 * A "block" is a big chunk of stuff that we do I/O on.
 * A "record" is a piece of info that we care about.
 * Typically many "record"s fit into a "block".
 */
#define	RECORDSIZE	512
#define	NAMSIZ	100
#define	TUNMLEN	32
#define	TGNMLEN	32

union hblock {
	char		charptr[RECORDSIZE];
	struct header {
		char	name[NAMSIZ];
		char	mode[8];
		char	uid[8];
		char	gid[8];
		char	size[12];
		char	mtime[12];
		char	chksum[8];
		char	linkflag;
		char	linkname[NAMSIZ];
		char	magic[6];
		char	version[2];
		char	uname[TUNMLEN];
		char	gname[TGNMLEN];
		char	devmajor[8];
		char	devminor[8];
	} header;
};

/* The magic field is filled with this if uname and gname are valid. */
#define	TMAGIC		"ustar"		/* 5 chars and a null */
#define TMAGLEN		6
#define TVERSION	"00"		/* 2 chars no null */
#define TVERSLEN	2

/* The linkflag defines the type of file */
#define	AREGTYPE	'\0'		/* Normal disk file, Unix compat */
#define	REGTYPE		'0'		/* Normal disk file */
#define	LNKTYPE		'1'		/* Link to previously dumped file */
#define	SYMTYPE		'2'		/* Symbolic link */
#define	CHRTYPE		'3'		/* Character special file */
#define	BLKTYPE		'4'		/* Block special file */
#define	DIRTYPE		'5'		/* Directory */
#define	FIFOTYPE	'6'		/* FIFO special file */
#define	CONTTYPE	'7'		/* Contiguous file */

/* Further link types may be defined later. */

#define TSUID   04000
#define TSGID   02000
#define TSVTX   01000

#define TUREAD	0400
#define TUWRITE	0200
#define TUEXEC	0100
#define TGREAD	0040
#define TGWRITE	0020
#define TGEXEC	0010
#define TOREAD	0004
#define TOWRITE	0002
#define TOEXEC	0001
