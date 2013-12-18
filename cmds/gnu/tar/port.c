/*
 * @(#)port.c 1.15	87/11/05	Public Domain, by John Gilmore, 1986
 *
 * These are routines not available in all environments.
 *
 * I know this introduces an extra level of subroutine calls and is
 * slightly slower.  Frankly, my dear, I don't give a damn.  Let the
 * Missed-Em Vee losers suffer a little.  This software is proud to
 * have been written on a BSD system.
 */

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/tar/RCS/port.c,v 1.2 1992/06/27 12:16:16 bart Exp $";

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#ifdef	MSDOS
#include <fcntl.h>
#else
# ifndef POSIX
#  include <sys/file.h>
# endif
#endif

#include "tarpriv.h"

extern char **environ;

#ifndef NULL
#define NULL 0
#endif

/*
 * Some people (e.g. V7) don't have a #define for these.
 */
#ifndef	O_BINARY
#define	O_BINARY	0
#endif
#ifndef	O_RDONLY
#define	O_RDONLY	0
#endif


#include "port.h"


/*
 * Some computers are not so crass as to align themselves into the BSD
 * or USG camps.  If a system supplies all of the routines we fake here,
 * add it to the list in the #ifndefs below and it'll all be skipped.
 * Make sure to add a matching #endif at the end of the file!
 *
 * We are avoiding #if defined() here for the sake of Minix, which comes
 * with the severely broken Amsterdam Compiler Kit.  Thanks, Andy!
 */
#ifndef mc300
#ifndef mc500
#ifndef mc700


#ifndef BSD42

#ifndef __HELIOS
/*
 * lstat() is a stat() which does not follow symbolic links.
 * If there are no symbolic links, just use stat().
 */
int
lstat (path, buf)
	char *path;
	struct stat *buf;
{
	extern int stat ();
	return (stat (path, buf));
}
#endif

/*
 * valloc	() does a malloc() on a page boundary.  On some systems,
 * this can make large block I/O more efficient.
 */
char *
valloc (size)
	unsigned size;
{
	extern char *malloc ();
	return (malloc (size));
}

/*
 *				NMKDIR.C
 *
 * Written by Robert Rother, Mariah Corporation, August 1985. 
 *
 * I wrote this out of shear disgust with myself because I couldn't
 * figure out how to do this in /bin/sh.
 *
 * If you want it, it's yours.  All I ask in return is that if you
 * figure out how to do this in a Bourne Shell script you send me
 * a copy.
 *					sdcsvax!rmr or rmr@uscd
*
* Severely hacked over by John Gilmore to make a 4.2BSD compatible
* subroutine.	11Mar86; hoptoad!gnu
*
* Modified by rmtodd@uokmax 6-28-87 -- when making an already existing dir,
* subroutine didn't return EEXIST.  It does now.
*/

/*
 * Make a directory.  Compatible with the mkdir() system call on 4.2BSD.
 */
#ifndef POSIX
#ifndef	MSDOS
int
mkdir(dpath, dmode)
	char *dpath;
	int dmode;
{
	int cpid, status;
	struct stat statbuf;
	extern int errno;

	if (stat(dpath,&statbuf) == 0) {
		errno = EEXIST;		/* Stat worked, so it already exists */
		return -1;
	}

	/* If stat fails for a reason other than non-existence, return error */
	if (errno != ENOENT) return -1; 

	switch (cpid = ()) {

	case -1:			/* Error in fork() */
		return(-1);		/* Errno is set already */

	case 0:				/* Child process */
		/*
		 * Cheap hack to set mode of new directory.  Since this
		 * child process is going away anyway, we zap its umask.
		 * FIXME, this won't suffice to set SUID, SGID, etc. on this
		 * directory.  Does anybody care?
		 */
		status = umask(0);	/* Get current umask */
		status = umask(status | (0777 & ~dmode)); /* Set for mkdir */
		execl("/bin/mkdir", "mkdir", dpath, (char *)0);
		_exit(-1);		/* Can't exec /bin/mkdir */
	
	default:			/* Parent process */
		while (cpid != wait(&status)) ;	/* Wait for kid to finish */
	}

	if (TERM_SIGNAL(status) != 0 || TERM_VALUE(status) != 0) {
		errno = EIO;		/* We don't know why, but */
		return -1;		/* /bin/mkdir failed */
	}

	return 0;
}
#endif	/* MSDOS */
#endif /* POSIX */
#endif

/* This next bit is called "doing OR on Minix cpp", e.g. without defined(). */
#undef WANTSTRING
#ifdef USG
#define WANTSTRING
#endif
#ifdef MSDOS
#define WANTSTRING
#endif
#if defined(__STDC__) && !defined(__HELIOS)
#define WANTSTRING
#endif

#ifdef WANTSTRING
/*
 * Translate V7 style into Sys V style.
 */
#include <string.h>
#include <memory.h>

char *
index (s, c)
	char *s;
	int c;
{
	return (strchr (s, c));
}

void
bcopy (s1, s2, n)
	char *s1, *s2;
	int n;
{
	(void) memcpy (s2, s1, n);
}

void
bzero (s1, n)
	char *s1;
	int n;
{
	(void) memset(s1, 0, n);
}

int
bcmp(s1, s2, n)
	char	*s1, *s2;
	int	n;
{
	return memcmp(s1, s2, n);
}
#endif

#ifdef MINIX
/* Minix has bcopy but not bzero, and no memset.  Thanks, Andy. */
void
bzero (s1, n)
	register char *s1;
	register int n;
{
	while (n--) *s1++ = '\0';
}

/* It also has no bcmp() */
int
bcmp (s1, s2, n) 
	register char *s1,*s2;
	register int n;
{
	for ( ; n-- ; ++s1, ++s2) {
		if (*s1 != *s2) return *s1 - *s2;
	}
	return 0;
}

/*
 * Groan, Minix doesn't have execlp either!
 *
 * execlp(file,arg0,arg1...argn,(char *)NULL)
 * exec a program, automatically searching for the program through
 * all the directories on the PATH.
 *
 * This version is naive about variable argument lists, it assumes
 * a straightforward C calling sequence.  If your system has odd stacks
 * *and* doesn't have execlp, YOU get to fix it.
 */
int
execlp(filename, arg0)
	char *filename, *arg0;
{
	extern char *malloc(), *getenv(), *index();
	extern int errno;
	register char *p, *path;    
	char **argstart = &arg0;
	register char *fnbuffer;
	struct stat statbuf;

	if ((p = getenv("PATH")) == NULL) {
		/* couldn't find path variable -- try to exec given filename */
		return execve(filename, argstart, environ);
	}

	/*
	 * make a place to build the filename.  We malloc larger than we
	 * need, but we know it will fit in this.
	 */
	fnbuffer = malloc( strlen(p) + 1 + strlen(filename) );
	if (fnbuffer == NULL) {
		errno = ENOMEM;
		return -1;
	}

	/*
	 * try each component of the path to see if the file's there
	 * and executable.
	 */
	for (path = p ; path ; path = p) {
		/* construct full path name to try */
		if ((p = index(path,':')) == NULL) {
			strcpy(fnbuffer, path);
		} else {
			strncpy(fnbuffer, path, p-path);
			fnbuffer[p-path] = '\0';
			p++;		/* Skip : for next time */
		}
		if (strlen(fnbuffer) != 0)
			strcat(fnbuffer,"/");
		strcat(fnbuffer,filename);

		/* check to see if file is there and is a normal file */
		if (stat(fnbuffer, &statbuf) < 0) {
			if (errno == ENOENT)
				continue; /* file not there,keep on looking */
			else
				goto fail; /* failed for some reason, return */
		}
		if ( (statbuf.st_mode & S_IFMT) != S_IFREG) continue;

		if (execve(fnbuffer, argstart, environ) < 0
		    && errno != ENOENT
		    && errno != ENOEXEC) {
			/* failed, for some other reason besides "file
			 * not found" or "not a.out format"
			 */
			goto fail;
		}

		/*
		 * If we got error ENOEXEC, the file is executable but is
		 * not an object file.  Try to execute it as a shell script,
		 * returning error if we can't execute /bin/sh.
		 *
		 * FIXME, this code is broken in several ways.  Shell
		 * scripts should not in general be executed by the user's
		 * SHELL variable program.  On more mature systems, the
		 * script can specify with #!/bin/whatever.  Also, this
		 * code clobbers argstart[-1] if the exec of the shell
		 * fails.
		 */
		if (errno == ENOEXEC) {
			char *shell;

			/* Try to execute command "sh arg0 arg1 ..." */
			if ((shell = getenv("SHELL")) == NULL)
				shell = "/bin/sh";
			argstart[-1] = shell;
			argstart[0] = fnbuffer;
			execve(shell, &argstart[-1], environ);
			goto fail;	/* Exec didn't work */
		}

		/* 
		 * If we succeeded, the execve() doesn't return, so we
		 * can only be here is if the file hasn't been found yet.
		 * Try the next place on the path.
		 */
	}

	/* all attempts failed to locate the file.  Give up. */
	errno = ENOENT;

fail:
	free(fnbuffer);
	return -1;
}
#endif /* MINIX */


#ifdef EMUL_OPEN3
#include "open3.h"
/*
 * open3 -- routine to emulate the 3-argument open system
 * call that is present in most modern Unix systems.
 * This version attempts to support all the flag bits except for O_NDELAY
 * and O_APPEND, which are silently ignored.  The emulation is not as efficient
 * as the real thing (at worst, 4 system calls instead of one), but there's
 * not much I can do about that.
 *
 * Written 6/10/87 by rmtodd@uokmax
 *
 * open3(path, flag, mode)
 * Attempts to open the file specified by
 * the given pathname.  The following flag bits (#defined in tar.h)
 * specify options to the routine:
 *	O_RDONLY	file open for read only
 *	O_WRONLY	file open for write only
 *	O_RDWR		file open for both read & write
 * (Needless to say, you should only specify one of the above).
 * 	O_CREAT		file is created with specified mode if it needs to be.
 *	O_TRUNC		if file exists, it is truncated to 0 bytes
 *	O_EXCL		used with O_CREAT--routine returns error if file exists
 * Function returns file descriptor if successful, -1 and errno if not.
 */

/*
 * array to give arguments to access for various modes
 * FIXME, this table depends on the specific integer values of O_XXX,
 * and also contains integers (args to 'access') that should be #define's.
 */
static int modes[] =
	{
		04, /* O_RDONLY */
		02, /* O_WRONLY */
		06, /* O_RDWR */
		06, /* invalid but we'd better cope -- O_WRONLY+O_RDWR */
	};

/* Shut off the automatic emulation of open(), we'll need it. */
#undef open

int
open3(path, flags, mode)
char *path;
int flags, mode;
{
	extern int errno;
	int exists = 1;
	int call_creat = 0;
	int fd;
	/*
	 * We actually do the work by calling the open() or creat() system
	 * call, depending on the flags.  Call_creat is true if we will use 
	 * creat(), false if we will use open().
	 */

	/*
	 * See if the file exists and is accessible in the requested mode. 
	 *
	 * Strictly speaking we shouldn't be using access, since access checks
	 * against real uid, and the open call should check against euid.
	 * Most cases real uid == euid, so it won't matter.   FIXME.
	 * FIXME, the construction "flags & 3" and the modes table depends
	 * on the specific integer values of the O_XXX #define's.  Foo!
	 */
	if (access(path,modes[flags & 3]) < 0) {
		if (errno == ENOENT) {
			/* the file does not exist */
			exists = 0;
		} else {
			/* probably permission violation */
			if (flags & O_EXCL) {
				/* Oops, the file exists, we didn't want it. */
				/* No matter what the error, claim EEXIST. */
				errno = EEXIST;
			}
			return -1;
		}
	}

	/* if we have the O_CREAT bit set, check for O_EXCL */
	if (flags & O_CREAT) {
		if ((flags & O_EXCL) && exists) {
			/* Oops, the file exists and we didn't want it to. */
			errno = EEXIST;
			return -1;
		}
		/*
		 * If the file doesn't exist, be sure to call creat() so that
		 * it will be created with the proper mode.
		 */
		if (!exists) call_creat = 1;
	} else {
		/* If O_CREAT isn't set and the file doesn't exist, error. */
		if (!exists) {
			errno = ENOENT;
			return -1;
		}
	}

	/*
	 * If the O_TRUNC flag is set and the file exists, we want to call
	 * creat() anyway, since creat() guarantees that the file will be
	 * truncated and open()-for-writing doesn't.
	 * (If the file doesn't exist, we're calling creat() anyway and the
	 * file will be created with zero length.)
	 */
	if ((flags & O_TRUNC) && exists) call_creat = 1;
	/* actually do the call */
	if (call_creat) {
		/*
		 * call creat.  May have to close and reopen the file if we
		 * want O_RDONLY or O_RDWR access -- creat() only gives
		 * O_WRONLY.
		 */
		fd = creat(path,mode);
		if (fd < 0 || (flags & O_WRONLY)) return fd;
		if (close(fd) < 0) return -1;
		/* Fall out to reopen the file we've created */
	}

	/*
	 * calling old open, we strip most of the new flags just in case.
	 */
	return open(path, flags & (O_RDONLY|O_WRONLY|O_RDWR|O_BINARY));
}
#endif

#endif /* MASSCOMP mc700 */
#endif /* MASSCOMP mc500 */
#endif /* MASSCOMP mc300 */



#ifdef	MSDOS
/* Fake mknod by complaining */
int
mknod(path, mode, dev)
	char		*path;
	unsigned short	mode;
	dev_t		dev;
{
	extern int	errno;
	int		fd;
	
	errno = ENXIO;		/* No such device or address */
	return -1;		/* Just give an error */
}

/* Fake links by copying */
int
link(path1, path2)
	char		*path1;
	char		*path2;
{
	char	buf[256];
	int	ifd, ofd;
	int	nrbytes;
	int	nwbytes;

	fprintf(stderr, "%s: %s: cannot link to %s, copying instead\n",
		tar, path1, path2);
	if ((ifd = open(path1, O_RDONLY|O_BINARY)) < 0)
		return -1;
	if ((ofd = creat(path2, 0666)) < 0)
		return -1;
	setmode(ofd, O_BINARY);
	while ((nrbytes = read(ifd, buf, sizeof(buf))) > 0) {
		if ((nwbytes = write(ofd, buf, nrbytes)) != nrbytes) {
			nrbytes = -1;
			break;
		}
	}
	/* Note use of "|" rather than "||" below: we want to close
	 * the files even if an error occurs.
	 */
	if ((nrbytes < 0) | (0 != close(ifd)) | (0 != close(ofd))) {
		unlink(path2);
		return -1;
	}
	return 0;
}

/* everyone owns everything on MS-DOS (or is it no one owns anything?) */
int
chown(path, uid, gid)
	char	*path;
	int	uid;
	int	gid;
{
	return 0;
}

int
geteuid()
{
	return 0;
}
#endif	/* MSDOS */
