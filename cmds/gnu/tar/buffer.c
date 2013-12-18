/*
 * Buffer management for public domain tar.
 *
 * Written by John Gilmore, ihnp4!hoptoad!gnu, on 25 August 1985.
 *
 * @(#) buffer.c 1.28 11/6/87 Public Domain - gnu
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>		/* For non-Berkeley systems */
#include <sys/stat.h>
#include <signal.h>

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/tar/RCS/buffer.c,v 1.1 1990/08/28 13:10:24 james Exp $";

#ifdef	MSDOS
# include <fcntl.h>
#else
# ifdef XENIX
#  include <sys/inode.h>
# else
#  ifndef POSIX
#   include <sys/file.h>
#  endif
# endif
#endif

extern char	*index(), *strcat();

#include "tarpriv.h"
#include "port.h"

#define	STDIN	0		/* Standard input  file descriptor */
#define	STDOUT	1		/* Standard output file descriptor */

#define	PREAD	0		/* Read  file descriptor from pipe() */
#define	PWRITE	1		/* Write file descriptor from pipe() */

extern char	*valloc();

/*
 * V7 doesn't have a #define for this.
 */
#ifndef O_RDONLY
#define	O_RDONLY	0
#endif

#define	MAGIC_STAT	105	/* Magic status returned by child, if
				   it can't exec.  We hope compress/sh
				   never return this status! */
/*
 * The record pointed to by save_rec should not be overlaid
 * when reading in a new tape block.  Copy it to record_save_area first, and
 * change the pointer in *save_rec to point to record_save_area.
 * Saved_recno records the record number at the time of the save.
 * This is used by annofile() to print the record number of a file's
 * header record.
 */
static union record **save_rec;
static union record record_save_area;
static long	    saved_recno;

/*
 * PID of child program, if f_compress or remote archive access.
 */
static int	childpid = 0;

/*
 * Record number of the start of this block of records
 */
static long	baserec;

/*
 * Error recovery stuff
 */
static int	r_error_count;

/*
 * Have we hit EOF yet?
 */
static int	eof;


/*
 * Return the location of the next available input or output record.
 * Return NULL for EOF.  Once we have returned NULL, we just keep returning
 * it, to avoid accidentally going on to the next file on the "tape".
 */
union record *
findrec()
{
	if (ar_record == ar_last) {
		if (eof)
			return (union record *)NULL;	/* EOF */
		flush_archive();
		if (ar_record == ar_last) {
			eof++;
			return (union record *)NULL;	/* EOF */
		}
	}
	return ar_record;
}


/*
 * Indicate that we have used all records up thru the argument.
 * (should the arg have an off-by-1? XXX FIXME)
 */
void
userec(rec)
	union record *rec;
{
	while(rec >= ar_record)
		ar_record++;
	/*
	 * Do NOT flush the archive here.  If we do, the same
	 * argument to userec() could mean the next record (if the
	 * input block is exactly one record long), which is not what
	 * is intended.
	 */
	if (ar_record > ar_last)
		abort();
}


/*
 * Return a pointer to the end of the current records buffer.
 * All the space between findrec() and endofrecs() is available
 * for filling with data, or taking data from.
 */
union record *
endofrecs()
{
	return ar_last;
}


/* 
 * Duplicate a file descriptor into a certain slot.
 * Equivalent to BSD "dup2" with error reporting.
 */
void
dupto(from, to, msg)
	int from, to;
	char *msg;
{
	int err;

	if (from != to) {
		(void) close(to);
		err = dup(from);
		if (err != to) {
			fprintf(stderr, "tar: cannot dup ");
			perror(msg);
			exit(EX_SYSTEM);
		}
		(void) close(from);
	}
}


/*
 * Fork a child to deal with remote files or compression.
 * If rem_host is zero, we are called only for compression.
 */
void
child_open(rem_host, rem_file)
	char *rem_host, *rem_file;
{

#ifdef	MSDOS
	fprintf(stderr,
	  "MSDOS %s cannot deal with compressed or remote archives\n", tar);
	exit(EX_ARGSBAD);
#else
#ifdef	HELIOS
	fprintf(stderr,
	  "HELIOS %s cannot deal with compressed or remote archives (yet)\n", tar);
	exit(EX_ARGSBAD);
#else
	
	int pipes[2];
	int err;
	struct stat arstat;
	char cmdbuf[1000];		/* For big file and host names */

	/* Create a pipe to talk to the child over */
	err = pipe(pipes);
	if (err < 0) {
		perror ("tar: cannot create pipe to child");
		exit(EX_SYSTEM);
	}
	
	/* Fork child process */
	childpid = fork();
	if (childpid < 0) {
		perror("tar: cannot fork");
		exit(EX_SYSTEM);
	}

	/*
	 * Parent process.  Clean up.
	 *
	 * We always close the archive file (stdin, stdout, or opened file)
	 * since the child will end up reading or writing that for us.
	 * Note that this may leave standard input closed.
	 * We close the child's end of the pipe since they will handle
	 * that too; and we set <archive> to the other end of the pipe.
	 *
	 * If reading, we set f_reblock since reading pipes or network
	 * sockets produces odd length data.
	 */
	if (childpid > 0) {
		(void) close (archive);	
		if (ar_reading) {
			(void) close (pipes[PWRITE]);
			archive = pipes[PREAD];
			f_reblock++;
		} else {
			(void) close (pipes[PREAD]);
			archive = pipes[PWRITE];
		}
		return;
	}

	/*
	 * Child process.
	 */
	if (ar_reading) {
		/*
		 * Reading from the child...
		 *
		 * Close the read-side of the pipe, which our parent will use.
		 * Move the write-side of pipe to stdout,
		 * If local, move archive input to child's stdin,
		 * then run the child.
		 */
		(void) close (pipes[PREAD]);
		dupto(pipes[PWRITE], STDOUT, "to stdout");
		if (rem_host) {
			(void) close (STDIN);	/* rsh abuses stdin */
			if (STDIN != open("/dev/null"))
				perror("Can't open /dev/null");
			sprintf(cmdbuf,
				"rsh '%s' dd '<%s' bs=%db",
				rem_host, rem_file, blocking);
			if (f_compress)
				strcat(cmdbuf, "| compress -d");
#ifdef DEBUG
			fprintf(stderr, "Exec-ing: %s\n", cmdbuf);
#endif
			execlp("sh", "sh", "-c", cmdbuf, (char *)0);
			perror("tar: cannot exec sh");
		} else {
			/*
			 * If we are reading a disk file, compress is OK;
			 * otherwise, we have to reblock the input in case it's
			 * coming from a tape drive.  This is an optimization.
			 */
			dupto(archive, STDIN, "to stdin");
			err = fstat(STDIN, &arstat);
			if (err != 0) {
				perror("tar: can't fstat archive");
				exit(EX_SYSTEM);
			}
			if ((arstat.st_mode & S_IFMT) == S_IFREG) {
				execlp("compress", "compress", "-d", (char *)0);
				perror("tar: cannot exec compress");
			} else {
				/* Non-regular file needs dd before compress */
				sprintf(cmdbuf,
					"dd bs=%db | compress -d",
					blocking);
#ifdef DEBUG
				fprintf(stderr, "Exec-ing: %s\n", cmdbuf);
#endif
				execlp("sh", "sh", "-c", cmdbuf, (char *)0);
				perror("tar: cannot exec sh");
			}
		}
		exit(MAGIC_STAT);
	} else {
		/*
		 * Writing archive to the child.
		 * It would like to run either:
		 *	compress
		 *	compress |            dd obs=20b
		 *		   rsh 'host' dd obs=20b '>foo'
		 * or	compress | rsh 'host' dd obs=20b '>foo'
		 *
		 * We need the dd to reblock the output to the
		 * user's specs, if writing to a device or over
		 * the net.  However, it produces a stupid
		 * message about how many blocks it processed.
		 * Because the shell on the remote end could be just
		 * about any shell, we can't depend on it to do
		 * redirect stderr properly for us -- the csh
		 * doesn't use the same syntax as the Bourne shell.
		 * On the other hand, if we just ignore stderr on
		 * this end, we won't see errors from rsh, or from
		 * the inability of "dd" to write its output file.
		 * The combination of the local sh, the rsh, the
		 * remote csh, and maybe a remote sh conspires to mess
		 * up any possible quoting method, so grumble! we
		 * punt and just accept the fucking "xxx blocks"
		 * messages.  The real fix would be a "dd" that
		 * would shut up.
		 * 
		 * Close the write-side of the pipe, which our parent will use.
		 * Move the read-side of the pipe to stdin,
		 * If local, move archive output to the child's stdout.
		 * then run the child.
		 */
		(void) close (pipes[PWRITE]);
		dupto(pipes[PREAD], STDIN, "to stdin");
		if (!rem_host)
			dupto(archive, STDOUT, "to stdout");

		cmdbuf[0] = '\0';
		if (f_compress) {
			if (!rem_host) {
				err = fstat(STDOUT, &arstat);
				if (err != 0) {
					perror("tar: can't fstat archive");
					exit(EX_SYSTEM);
				}
				if ((arstat.st_mode & S_IFMT) == S_IFREG) {
					execlp("compress", "compress", (char *)0);
					perror("tar: cannot exec compress");
				}
			}
			strcat(cmdbuf, "compress | ");
		}
		if (rem_host) {
			sprintf(cmdbuf+strlen(cmdbuf),
			  "rsh '%s' dd obs=%db '>%s'",
				 rem_host, blocking, rem_file);
		} else {
			sprintf(cmdbuf+strlen(cmdbuf),
				"dd obs=%db",
				blocking);
		}
#ifdef DEBUG
		fprintf(stderr, "Exec-ing: %s\n", cmdbuf);
#endif
		execlp("sh", "sh", "-c", cmdbuf, (char *)0);
		perror("tar: cannot exec sh");
		exit(MAGIC_STAT);
	}
#endif  /* HELIOS */
#endif	/* MSDOS */
}


/*
 * Open an archive file.  The argument specifies whether we are
 * reading or writing.
 */
open_archive(read)
	int read;
{
	char *colon, *slash;
	char *rem_host = 0, *rem_file;

	colon = index(ar_file, ':');
	if (colon) {
		slash = index(ar_file, '/');
		if (slash && slash > colon) {
			/*
			 * Remote file specified.  Parse out separately,
			 * and don't try to open it on the local system.
			 */
			rem_file = colon + 1;
			rem_host = ar_file;
			*colon = '\0';
			goto gotit;
		}
	}

	if (ar_file[0] == '-' && ar_file[1] == '\0') {
		f_reblock++;	/* Could be a pipe, be safe */
		if (read)	archive = STDIN;
		else		archive = STDOUT;
	} else if (read) {
		archive = open(ar_file, O_RDONLY);
	} else {
		archive = creat(ar_file, S_IRUSR|S_IWUSR|
					 S_IRGRP|S_IWGRP|
					 S_IROTH|S_IWOTH);
	}

	if (archive < 0) {
		perror(ar_file);
		exit(EX_BADARCH);
	}

#ifdef	MSDOS
	setmode(archive, O_BINARY);
#endif

gotit:
	if (blocksize == 0) {
		fprintf(stderr, "tar: invalid value for blocksize\n");
		exit(EX_ARGSBAD);
	}

	/*NOSTRICT*/
	ar_block = (union record *) valloc((unsigned)blocksize);
	if (!ar_block) {
		fprintf(stderr,
		"tar: could not allocate memory for blocking factor %d\n",
			blocking);
		exit(EX_ARGSBAD);
	}

	ar_record = ar_block;
	ar_last   = ar_block + blocking;
	ar_reading = read;

	if (f_compress || rem_host) 
		child_open(rem_host, rem_file);

	if (read) {
		ar_last = ar_block;		/* Set up for 1st block = # 0 */
		(void) findrec();		/* Read it in, check for EOF */
	}
}


/*
 * Remember a union record * as pointing to something that we
 * need to keep when reading onward in the file.  Only one such
 * thing can be remembered at once, and it only works when reading
 * an archive.
 *
 * We calculate "offset" then add it because some compilers end up
 * adding (baserec+ar_record), doing a 9-bit shift of baserec, then
 * subtracting ar_block from that, shifting it back, losing the top 9 bits.
 */
saverec(pointer)
	union record **pointer;
{
	long offset;

	save_rec = pointer;
	offset = ar_record - ar_block;
	saved_recno = baserec + offset;
}

/*
 * Perform a write to flush the buffer.
 */
fl_write()
{
	int err;

	err = write(archive, ar_block->charptr, blocksize);
	if (err == blocksize) return;
	/* FIXME, multi volume support on write goes here */
	if (err < 0)
		perror(ar_file);
	else
		fprintf(stderr, "tar: %s: write failed, short %d bytes\n",
			ar_file, blocksize - err);
	exit(EX_BADARCH);
}


/*
 * Handle read errors on the archive.
 *
 * If the read should be retried, readerror() returns to the caller.
 */
void
readerror()
{
#	define	READ_ERROR_MAX	10

	read_error_flag++;		/* Tell callers */

	annorec(stderr, tar);
	fprintf(stderr, "Read error on ");
	perror(ar_file);

	if (baserec == 0) {
		/* First block of tape.  Probably stupidity error */
		exit(EX_BADARCH);
	}	

	/*
	 * Read error in mid archive.  We retry up to READ_ERROR_MAX times
	 * and then give up on reading the archive.  We set read_error_flag
	 * for our callers, so they can cope if they want.
	 */
	if (r_error_count++ > READ_ERROR_MAX) {
		annorec(stderr, tar);
		fprintf(stderr, "Too many errors, quitting.\n");
		exit(EX_BADARCH);
	}
	return;
}


/*
 * Perform a read to flush the buffer.
 */
fl_read()
{
	int err;		/* Result from system call */
	int left;		/* Bytes left */
	char *more;		/* Pointer to next byte to read */

	/*
	 * Clear the count of errors.  This only applies to a single
	 * call to fl_read.  We leave read_error_flag alone; it is
	 * only turned off by higher level software.
	 */
	r_error_count = 0;	/* Clear error count */

	/*
	 * If we are about to wipe out a record that
	 * somebody needs to keep, copy it out to a holding
	 * area and adjust somebody's pointer to it.
	 */
	if (save_rec &&
	    *save_rec >= ar_record &&
	    *save_rec < ar_last) {
		record_save_area = **save_rec;
		*save_rec = &record_save_area;
	}
error_loop:
	err = read(archive, ar_block->charptr, blocksize);
	if (err == blocksize) return;
	if (err < 0) {
		readerror();
		goto error_loop;	/* Try again */
	}

	more = ar_block->charptr + err;
	left = blocksize - err;

again:
	if (0 == (((unsigned)left) % RECORDSIZE)) {
		/* FIXME, for size=0, multi vol support */
		/* On the first block, warn about the problem */
		if (!f_reblock && baserec == 0 && f_verbose && err > 0) {
			annorec(stderr, tar);
			fprintf(stderr, "Blocksize = %d record%s\n",
				err / RECORDSIZE, (err > RECORDSIZE)? "s": "");
		}
		ar_last = ar_block + ((unsigned)(blocksize - left))/RECORDSIZE;
		return;
	}
	if (f_reblock) {
		/*
		 * User warned us about this.  Fix up.
		 */
		if (left > 0) {
error2loop:
			err = read(archive, more, left);
			if (err < 0) {
				readerror();
				goto error2loop;	/* Try again */
			}
			if (err == 0) {
				annorec(stderr, tar);
				fprintf(stderr,
		"%s: eof not on block boundary, strange...\n",
					ar_file);
				exit(EX_BADARCH);
			}
			left -= err;
			more += err;
			goto again;
		}
	} else {
		annorec(stderr, tar);
		fprintf(stderr, "%s: read %d bytes, strange...\n",
			ar_file, err);
		exit(EX_BADARCH);
	}
}


/*
 * Flush the current buffer to/from the archive.
 */
flush_archive()
{

	baserec += ar_last - ar_block;	/* Keep track of block #s */
	ar_record = ar_block;		/* Restore pointer to start */
	ar_last = ar_block + blocking;	/* Restore pointer to end */

	if (!ar_reading) 
		fl_write();
	else
		fl_read();
}

/*
 * Close the archive file.
 */
close_archive()
{
	int child;
	int status;

	if (!ar_reading) flush_archive();
	(void) close(archive);

#ifndef	MSDOS
	if (childpid) {
		/*
		 * Loop waiting for the right child to die, or for
		 * no more kids.
		 */
		while (((child = wait(&status)) != childpid) && child != -1)
			;

		if (child != -1) {
			switch (TERM_SIGNAL(status)) {
			case 0:
				/* Child voluntarily terminated  -- but why? */
				if (TERM_VALUE(status) == MAGIC_STAT) {
					exit(EX_SYSTEM);/* Child had trouble */
				}
				if (TERM_VALUE(status) == (SIGPIPE + 128)) {
					/*
					 * /bin/sh returns this if its child
					 * dies with SIGPIPE.  'Sok.
					 */
					break;
				} else if (TERM_VALUE(status))
					fprintf(stderr,
				  "tar: child returned status %d\n",
						TERM_VALUE(status));
			case SIGPIPE:
				break;		/* This is OK. */

			default:
				fprintf(stderr,
				 "tar: child died with signal %d%s\n",
				 TERM_SIGNAL(status),
				 TERM_COREDUMP(status)? " (core dumped)": "");
			}
		}
	}
#endif	/* MSDOS */
}


/*
 * Message management.
 *
 * anno writes a message prefix on stream (eg stdout, stderr).
 *
 * The specified prefix is normally output followed by a colon and a space.
 * However, if other command line options are set, more output can come
 * out, such as the record # within the archive.
 *
 * If the specified prefix is NULL, no output is produced unless the
 * command line option(s) are set.
 *
 * If the third argument is 1, the "saved" record # is used; if 0, the
 * "current" record # is used.
 */
void
anno(stream, prefix, savedp)
	FILE	*stream;
	char	*prefix;
	int	savedp;
{
#	define	MAXANNO	50
	char	buffer[MAXANNO];	/* Holds annorecment */
#	define	ANNOWIDTH 13
	int	space;
	long	offset;

	/* Make sure previous output gets out in sequence */
	if (stream == stderr)
		fflush(stdout);
	if (f_sayblock) {
		if (prefix) {
			fputs(prefix, stream);
			putc(' ', stream);
		}
		offset = ar_record - ar_block;
		sprintf(buffer, "rec %d: ",
			savedp?	saved_recno:
				baserec + offset);
		fputs(buffer, stream);
		space = ANNOWIDTH - strlen(buffer);
		if (space > 0) {
			fprintf(stream, "%*s", space, "");
		}
	} else if (prefix) {
		fputs(prefix, stream);
		fputs(": ", stream);
	}
}
