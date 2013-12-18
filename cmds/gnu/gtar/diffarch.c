/*
 * Diff files from a tar archive.
 *
 * Written 30 April 1987 by John Gilmore, ihnp4!hoptoad!gnu.
 *
 * @(#) diffarch.c 1.10 87/11/11 Public Domain - gnu
 */

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/tar/RCS/diffarch.c,v 1.1 1990/08/28 13:11:54 james Exp $";

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

extern int errno;			/* From libc.a */
extern char *valloc();			/* From libc.a */

#include "tarpriv.h"
#include "port.h"

/* Some systems don't have these #define's -- we fake it here. */
#ifndef O_RDONLY
#define	O_RDONLY	0
#endif
#ifndef	O_NDELAY
#define	O_NDELAY	0
#endif

extern union record *head;		/* Points to current tape header */
extern struct stat hstat;		/* Stat struct corresponding */
extern int head_standard;		/* Tape header is in ANSI format */

extern void print_header();
extern void skip_file();

char *filedata;				/* Pointer to area for reading
					   file contents into */

/*
 * Initialize for a diff operation
 */
diff_init()
{

	/*NOSTRICT*/
	filedata = (char *) valloc((unsigned)blocksize);
	if (!filedata) {
		fprintf(stderr,
		"tar: could not allocate memory for diff buffer of %d bytes\n",
			blocking);
		exit(EX_ARGSBAD);
	}
}

/*
 * Diff a file against the archive.
 */
void
diff_archive()
{
	register char *data;
	int fd, check, namelen, written;
	int err, firsttime;
	long size;
	struct stat filestat;
	char linkbuf[NAMSIZ+3];

	errno = EPIPE;			/* FIXME, remove perrors */

	saverec(&head);			/* Make sure it sticks around */
	userec(head);			/* And go past it in the archive */
	decode_header(head, &hstat, &head_standard, 1);	/* Snarf fields */

	/* Print the record from 'head' and 'hstat' */
	if (f_verbose)
		print_header(stdout);

	switch (head->header.linkflag) {

	default:
		annofile(stderr, tar);
		fprintf(stderr,
		   "Unknown file type '%c' for %s, diffed as normal file\n",
			head->header.linkflag, head->header.name);
		/* FALL THRU */

	case LF_OLDNORMAL:
	case LF_NORMAL:
	case LF_CONTIG:
		/*
		 * Appears to be a file.
		 * See if it's really a directory.
		 */
		namelen = strlen(head->header.name)-1;
		if (head->header.name[namelen] == '/')
			goto really_dir;

		fd = open(head->header.name, O_NDELAY|O_RDONLY);

		if (fd < 0) {
			if (errno == ENOENT) {
				/* Expected error -- to stdout */
				annofile(stdout, (char *)NULL);
				fprintf(stdout, "%s: does not exist\n",
					head->header.name);
			} else {
				annofile(stderr, (char *)NULL);
				perror(head->header.name);
			}
			skip_file((long)hstat.st_size);
			goto quit;
		}

		err = fstat(fd, &filestat);
		if (err < 0) {
			annofile(stdout, (char *)NULL);
			fprintf(stdout, "Cannot fstat file ");
			perror(head->header.name);
			skip_file((long)hstat.st_size);
			goto qclose;
		}

		if ((filestat.st_mode & S_IFMT) != S_IFREG) {
			annofile(stdout, (char *)NULL);
			fprintf(stdout, "%s: not a regular file\n",
				head->header.name);
			skip_file((long)hstat.st_size);
			goto qclose;
		}

		filestat.st_mode &= ~S_IFMT;
		if (filestat.st_mode != hstat.st_mode)
			sigh("mode");
		if (filestat.st_uid  != hstat.st_uid)
			sigh("uid");
		if (filestat.st_gid  != hstat.st_gid)
			sigh("gid");
		if (filestat.st_size != hstat.st_size) {
			sigh("size");
			skip_file((long)hstat.st_size);
			goto qclose;
		}
		if (filestat.st_mtime != hstat.st_mtime)
			sigh("mod time");

		firsttime = 0;

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
			check = read (fd, filedata, written);
			/*
			 * The following is in violation of strict
			 * typing, since the arg to userec
			 * should be a struct rec *.  FIXME.
			 */
			userec(data + written - 1);
			if (check == written) {
				/* The read worked, now compare the data */
				if (bcmp(data, filedata, check) == 0)
					continue;	/* It compares */
				if (firsttime++) {
					annofile(stdout, (char *)NULL);
					fprintf(stdout, "%s: data differs\n",
						head->header.name);
				}
			}

			/*
			 * Error in reading from file.
			 * Print it, skip to next file in archive.
			 */
			annofile(stderr, tar);
			fprintf(stderr,
	"Tried to read %d bytes from file, could only read %d:\n",
				written, check);
			perror(head->header.name);
			skip_file((long)(size - written));
			break;	/* Still do the close, mod time, chmod, etc */
		}

	qclose:
		check = close(fd);
		if (check < 0) {
			annofile(stderr, tar);
			fprintf(stderr, "Error while closing ");
			perror(head->header.name);
		}
		
	quit:
		break;

	case LF_LINK:
		check = 1;	/* FIXME deal with this */
		/* check = link (head->header.linkname,
			      head->header.name); */
		/* FIXME, don't worry uid, gid, etc... */
		if (check == 0)
			break;
		annofile(stderr, tar);
		fprintf(stderr, "Could not link %s to ",
			head->header.name);
		perror(head->header.linkname);
		break;

#ifdef S_IFLNK
	case LF_SYMLINK:
		check = readlink(head->header.name, linkbuf,
				 (sizeof linkbuf)-1);
		
		if (check < 0) {
			if (errno == ENOENT) {
				annofile(stdout, (char *)NULL);
				fprintf(stdout,
					"%s: no such file or directory\n",
					head->header.name);
			} else {
				annofile(stderr, tar);
				fprintf(stderr, "Could not read link");
				perror(head->header.name);
			}
			break;
		}

		linkbuf[check] = '\0';	/* Null-terminate it */
		if (strncmp(head->header.linkname, linkbuf, check) != 0) {
			annofile(stdout, (char *)NULL);
			fprintf(stdout, "%s: symlink differs\n",
				head->header.linkname);
		}
		break;
#endif

#ifdef S_IFCHR
	case LF_CHR:
		hstat.st_mode |= S_IFCHR;
		goto make_node;
#endif

#ifdef S_IFBLK
	/* If local system doesn't support block devices, use default case */
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
		/* FIXME, deal with umask */
		
		check = 1; /* FIXME, implement this */
		/* check = mknod(head->header.name, (int) hstat.st_mode,
			(int) hstat.st_rdev); */
		if (check != 0) {
			annofile(stderr, tar);
			fprintf(stderr, "Could not make ");
			perror(head->header.name);
			break;
		};
		break;

	case LF_DIR:
		/* Check for trailing / */
		namelen = strlen(head->header.name)-1;
	really_dir:
		while (namelen && head->header.name[namelen] == '/')
			head->header.name[namelen--] = '\0';	/* Zap / */
		
		check = 1; /* FIXME, implement this */
/*		check = mkdir(head->header.name, S_IXUSR|S_IWUSR | (int)hstat.st_mode); */
		if (check != 0) {
			annofile(stderr, tar);
			fprintf(stderr, "Could not make directory ");
			perror(head->header.name);
			break;
		}
		
		break;

	}

	/* We don't need to save it any longer. */
	saverec((union record **) 0);	/* Unsave it */
}

/*
 * Sigh about something that differs.
 */
sigh(what)
	char *what;
{

	annofile(stdout, (char *)NULL);
	fprintf(stdout, "%s: %s differs\n",
		head->header.name, what);
}
