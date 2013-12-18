/*
 * Extract files from a tar archive.
 *
 * Written 19 Nov 1985 by John Gilmore, ihnp4!hoptoad!gnu.
 *
 * @(#) extract.c 1.32 87/11/11 Public Domain - gnu
 */

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/tar/RCS/extract.c,v 1.1 1990/08/28 13:12:32 james Exp $";

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef BSD42
#include <sys/file.h>
#endif

#ifdef USG
#include <fcntl.h>
#endif

#ifdef POSIX
#include <fcntl.h>
#include <sys/types.h>
#include <utime.h>
#define S_ISVTX 0
#endif

#ifdef	MSDOS
#include <fcntl.h>
#endif	/* MSDOS */

/*
 * Some people don't have a #define for these.
 */
#ifndef	O_BINARY
#define	O_BINARY	0
#endif
#ifndef O_NDELAY
#define	O_NDELAY	0
#endif

#ifdef NO_OPEN3
/* We need the #define's even though we don't use them. */
#include "open3.h"
#endif

#ifdef EMUL_OPEN3
/* Simulated 3-argument open for systems that don't have it */
#include "open3.h"
#endif

extern char *index();			/* From libc.a or port.c */

extern int errno;			/* From libc.a */
extern time_t time();			/* From libc.a */

#include "tarpriv.h"
#include "port.h"

extern union record *head;		/* Points to current tape header */
extern struct stat hstat;		/* Stat struct corresponding */
extern int head_standard;		/* Tape header is in ANSI format */

extern void print_header();
extern void skip_file();
extern void pr_mkdir();

int make_dirs();			/* Makes required directories */

static time_t now = 0;			/* Current time */
static we_are_root = 0;			/* True if our effective uid == 0 */
static int notumask = ~0;		/* Masks out bits user doesn't want */

/*
 * Set up to extract files.
 */
extr_init()
{
	int ourmask;

	now = time((time_t *)0);
	if (geteuid() == 0)
		we_are_root = 1;

	/*
	 * We need to know our umask.  But if f_use_protection is set,
	 * leave our kernel umask at 0, and our "notumask" at ~0.
	 */
	ourmask = umask(0);		/* Read it */
	if (!f_use_protection) {
		(void) umask (ourmask);	/* Set it back how it was */
		notumask = ~ourmask;	/* Make umask override permissions */
	}
}


/*
 * Extract a file from the archive.
 */
void
extract_archive()
{
	register char *data;
	int fd, check, namelen, written, openflag;
	long size;
#ifdef POSIX
	struct utimbuf acc_upd_times;
#else
	time_t acc_upd_times[2];
#endif
	register int skipcrud;

	saverec(&head);			/* Make sure it sticks around */
	userec(head);			/* And go past it in the archive */
	decode_header(head, &hstat, &head_standard, 1);	/* Snarf fields */

	/* Print the record from 'head' and 'hstat' */
	if (f_verbose)
		print_header(stdout);

	/*
	 * Check for fully specified pathnames and other atrocities.
	 *
	 * Note, we can't just make a pointer to the new file name,
	 * since saverec() might move the header and adjust "head".
	 * We have to start from "head" every time we want to touch
	 * the header record.
	 */
	skipcrud = 0;
	while ('/' == head->header.name[skipcrud]) {
		static int warned_once = 0;

		skipcrud++;	/* Force relative path */
		if (!warned_once++) {
			annorec(stderr, tar);
			fprintf(stderr,
	"Removing leading / from absolute path names in the archive.\n");
		}
	}

	switch (head->header.linkflag) {

	default:
		annofile(stderr, tar);
		fprintf(stderr,
		   "Unknown file type '%c' for %s, extracted as normal file\n",
			head->header.linkflag, skipcrud+head->header.name);
		/* FALL THRU */

	case LF_OLDNORMAL:
	case LF_NORMAL:
	case LF_CONTIG:
		/*
		 * Appears to be a file.
		 * See if it's really a directory.
		 */
		namelen = strlen(skipcrud+head->header.name)-1;
		if (head->header.name[skipcrud+namelen] == '/')
			goto really_dir;

		/* FIXME, deal with protection issues */
	again_file:
		openflag = f_keep?
			O_BINARY|O_NDELAY|O_WRONLY|O_APPEND|O_CREAT|O_EXCL:
			O_BINARY|O_NDELAY|O_WRONLY|O_APPEND|O_CREAT|O_TRUNC;
#ifdef O_CTG
		/*
		 * Contiguous files (on the Masscomp) have to specify
		 * the size in the open call that creates them.
		 */
		if (head->header.lnkflag == LF_CONTIG)
			fd = open(skipcrud+head->header.name, openflag | O_CTG,
				hstat.st_mode, hstat.st_size);
		else
#endif
		{
#ifdef NO_OPEN3
			/*
			 * On raw V7 we won't let them specify -k (f_keep), but
			 * we just bull ahead and create the files.
			 */
			fd = creat(skipcrud+head->header.name, 
				hstat.st_mode);
#else
			/*
			 * With 3-arg open(), we can do this up right.
			 */
			fd = open(skipcrud+head->header.name, openflag,
				hstat.st_mode);
#endif
		}

		if (fd < 0) {
			if (make_dirs(skipcrud+head->header.name))
				goto again_file;
			annofile(stderr, tar);
			fprintf(stderr, "Could not make file ");
			perror(skipcrud+head->header.name);
			skip_file((long)hstat.st_size);
			goto quit;
		}

		for (size = hstat.st_size;
		     size > 0;
		     size -= written) {
			/*
			 * Locate data, determine max length
			 * writeable, write it, record that
			 * we have used the data, then check
			 * if the write worked.
			 */
			data = findrec()->charptr;
			if (data == NULL) {	/* Check it... */
				annorec(stderr, tar);
				fprintf(stderr, "Unexpected EOF on archive file\n");
				break;
			}
			written = endofrecs()->charptr - data;
			if (written > size) written = size;
			errno = 0;
			check = write(fd, data, written);
			/*
			 * The following is in violation of strict
			 * typing, since the arg to userec
			 * should be a struct rec *.  FIXME.
			 */
			userec(data + written - 1);
			if (check == written) continue;
			/*
			 * Error in writing to file.
			 * Print it, skip to next file in archive.
			 */
			annofile(stderr, tar);
			fprintf(stderr,
	"Tried to write %d bytes to file, could only write %d:\n",
				written, check);
			perror(skipcrud+head->header.name);
			skip_file((long)(size - written));
			break;	/* Still do the close, mod time, chmod, etc */
		}

		check = close(fd);
		if (check < 0) {
			annofile(stderr, tar);
			fprintf(stderr, "Error while closing ");
			perror(skipcrud+head->header.name);
		}
		
	set_filestat:
		/*
		 * Set the modified time of the file.
		 * 
		 * Note that we set the accessed time to "now", which
		 * is really "the time we started extracting files".
		 */
		if (!f_modified) {
			int r;
#ifdef POSIX
#ifdef HELIOS
			acc_upd_times.ctime = 0; /* Unaffected */
#endif
			acc_upd_times.actime = now;	         /* Accessed now */
			acc_upd_times.modtime = hstat.st_mtime; /* Mod'd */
			r = utime(skipcrud+head->header.name,
			    &acc_upd_times);
#else
			acc_upd_times[0] = now;	         /* Accessed now */
			acc_upd_times[1] = hstat.st_mtime; /* Mod'd */
			r = utime(skipcrud+head->header.name,
			    acc_upd_times);
#endif
			if( r < 0 )
			{
				annofile(stderr, tar);
				perror(skipcrud+head->header.name);
			}
		}

		/*
		 * If we are root, set the owner and group of the extracted
		 * file.  This does what is wanted both on real Unix and on
		 * System V.  If we are running as a user, we extract as that
		 * user; if running as root, we extract as the original owner.
		 */
		if (we_are_root) {
			if (chown(skipcrud+head->header.name, hstat.st_uid,
				  hstat.st_gid) < 0) {
				annofile(stderr, tar);
				perror(skipcrud+head->header.name);
			}
		}

		/*
		 * If '-k' is not set, open() or creat() could have saved
		 * the permission bits from a previously created file,
		 * ignoring the ones we specified.
		 * Even if -k is set, if the file has abnormal
		 * mode bits, we must chmod since writing or chown() has
		 * probably reset them.
		 *
		 * If -k is set, we know *we* created this file, so the mode
		 * bits were set by our open().   If the file is "normal", we
		 * skip the chmod.  This works because we did umask(0) if -p
		 * is set, so umask will have left the specified mode alone.
		 */
		if ((!f_keep)
		    || (hstat.st_mode & (S_ISUID|S_ISGID|S_ISVTX))) {
			if (chmod(skipcrud+head->header.name,
				  notumask & (int)hstat.st_mode) < 0) {
				annofile(stderr, tar);
				perror(skipcrud+head->header.name);
			}
		}

	quit:
		break;

	case LF_LINK:
	again_link:
		check = link (head->header.linkname,
			      skipcrud+head->header.name);
		if (check == 0)
			break;
		if (make_dirs(skipcrud+head->header.name))
			goto again_link;
		annofile(stderr, tar);
		fprintf(stderr, "Could not link %s to ",
			skipcrud+head->header.name);
		perror(head->header.linkname);
		break;

#ifdef S_IFLNK
	case LF_SYMLINK:
	again_symlink:
		check = symlink(head->header.linkname,
			        skipcrud+head->header.name);
		/* FIXME, don't worry uid, gid, etc... */
		if (check == 0)
			break;
		if (make_dirs(skipcrud+head->header.name))
			goto again_symlink;
		annofile(stderr, tar);
		fprintf(stderr, "Could not create symlink ");
		perror(head->header.linkname);
		break;
#endif

#ifdef S_IFCHR
	case LF_CHR:
		hstat.st_mode |= S_IFCHR;
		goto make_node;
#endif

#ifdef S_IFBLK
	case LF_BLK:
		hstat.st_mode |= S_IFBLK;
		goto make_node;
#endif

#ifdef S_IFIFO
	/* If local system doesn't support FIFOs, use default case */
	case LF_FIFO:
		hstat.st_mode |= S_IFIFO;
		hstat.st_rdev = 0;		/* FIXME, do we need this? */
		goto make_node;
#endif

	make_node:
#ifdef POSIX
		if( hstat.st_mode & S_IFIFO )
			check = mkfifo(skipcrud+head->header.name, hstat.st_mode );
		else
			check = mkdir(skipcrud+head->header.name, hstat.st_mode );
#else
		check = mknod(skipcrud+head->header.name,
			      (int) hstat.st_mode, (int) hstat.st_rdev);
#endif
		if (check != 0) {
			if (make_dirs(skipcrud+head->header.name))
				goto make_node;
			annofile(stderr, tar);
			fprintf(stderr, "Could not make ");
			perror(skipcrud+head->header.name);
			break;
		};
		goto set_filestat;

	case LF_DIR:
		namelen = strlen(skipcrud+head->header.name)-1;
	really_dir:
		/* Check for trailing /, and zap as many as we find. */
		while (namelen && head->header.name[skipcrud+namelen] == '/')
			head->header.name[skipcrud+namelen--] = '\0';
		
	again_dir:
		check = mkdir(skipcrud+head->header.name,
			      (S_IXUSR|S_IWUSR) | (int)hstat.st_mode);
		if (check != 0) {
			if (make_dirs(skipcrud+head->header.name))
				goto again_dir;
			/* If we're trying to create '.', let it be. */
			if (head->header.name[skipcrud+namelen] == '.' && 
			    (namelen==0 ||
			     head->header.name[skipcrud+namelen-1]=='/'))
				goto check_perms;
			annofile(stderr, tar);
			fprintf(stderr, "Could not make directory ");
			perror(skipcrud+head->header.name);
			break;
		}
		
	check_perms:
		if ((S_IXUSR|S_IWUSR) != 
		    ((S_IXUSR|S_IWUSR) & (int) hstat.st_mode)) {
			hstat.st_mode |= S_IXUSR|S_IWUSR;
			annofile(stderr, tar);
			fprintf(stderr,
			  "Added write & execute permission to directory %s\n",
			  skipcrud+head->header.name);
		}

		goto set_filestat;
		/* FIXME, Remember timestamps for after files created? */
		/* FIXME, change mode after files created (if was R/O dir) */

	}

	/* We don't need to save it any longer. */
	saverec((union record **) 0);	/* Unsave it */
}

/*
 * After a file/link/symlink/dir creation has failed, see if
 * it's because some required directory was not present, and if
 * so, create all required dirs.
 */
int
make_dirs(pathname)
	char *pathname;
{
	char *p;			/* Points into path */
	int madeone = 0;		/* Did we do anything yet? */
	int save_errno = errno;		/* Remember caller's errno */
	int check;

	if (errno != ENOENT)
		return 0;		/* Not our problem */

	for (p = index(pathname, '/'); p != NULL; p = index(p+1, '/')) {
		/* Avoid mkdir of empty string, if leading or double '/' */
		if (p == pathname || p[-1] == '/')
			continue;
		/* Avoid mkdir where last part of path is '.' */
		if (p[-1] == '.' && (p == pathname+1 || p[-2] == '/'))
			continue;
		*p = 0;				/* Truncate the path there */
		check = mkdir (pathname, 0777);	/* Try to create it as a dir */
		if (check == 0) {
			/* Fix ownership */
			if (we_are_root) {
				if (chown(pathname, hstat.st_uid,
					  hstat.st_gid) < 0) {
					annofile(stderr, tar);
					perror(pathname);
				}
			}
			pr_mkdir(pathname, p-pathname, notumask&0777, stdout);
			madeone++;		/* Remember if we made one */
			*p = '/';
			continue;
		}
		*p = '/';
		if (errno == EEXIST)		/* Directory already exists */
			continue;
		/*
		 * Some other error in the mkdir.  We return to the caller.
		 */
		break;
	}

	errno = save_errno;		/* Restore caller's errno */
	return madeone;			/* Tell them to retry if we made one */
}
