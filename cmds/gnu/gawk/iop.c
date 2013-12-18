/*
 * iop.c - do i/o related things.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"

#ifndef atarist
#define INVALID_HANDLE (-1)
#else
#include <stddef.h>
#include <fcntl.h>
#define INVALID_HANDLE  (__SMALLEST_VALID_HANDLE - 1)
#endif  /* atarist */


#ifdef TEST
int bufsize = 8192;
#endif

int
optimal_bufsize(fd)
int fd;
{
#ifdef VMS
/* don't even bother trying [fstat() fails across DECnet] */
	return BUFSIZ;
#else
	struct stat stb;

	/*
	 * System V doesn't have the file system block size in the
	 * stat structure. So we have to make some sort of reasonable
	 * guess. We use stdio's BUFSIZ, since that is what it was
	 * meant for in the first place.
	 */
#ifdef BLKSIZE_MISSING
#define	DEFBLKSIZE	BUFSIZ
#else
#define DEFBLKSIZE	(stb.st_blksize ? stb.st_blksize : BUFSIZ)
#endif

#ifdef TEST
	return bufsize;
#endif
#ifndef atarist
	if (isatty(fd))
#else
	/*
	 * On ST redirected stdin does not have a name attached
	 * (this could be hard to do to) and fstat would fail
	 */
	if (0 == fd || isatty(fd))
#endif  /*atarist */
		return BUFSIZ;
	if (fstat(fd, &stb) == -1)
		fatal("can't stat fd %d (%s)", fd, strerror(errno));
	if (lseek(fd, 0L, 0) == -1)
		return DEFBLKSIZE;
	return (stb.st_size < DEFBLKSIZE ? stb.st_size : DEFBLKSIZE);
#endif	/*! VMS */
}

IOBUF *
iop_alloc(fd)
int fd;
{
	IOBUF *iop;

	if (fd == INVALID_HANDLE)
		return NULL;
	emalloc(iop, IOBUF *, sizeof(IOBUF), "iop_alloc");
	iop->flag = 0;
	if (isatty(fd))
		iop->flag |= IOP_IS_TTY;
	iop->size = optimal_bufsize(fd);
	errno = 0;
	iop->fd = fd;
	emalloc(iop->buf, char *, iop->size + 2, "iop_alloc");
	iop->end = iop->off = iop->buf;
	iop->secsiz = iop->size < BUFSIZ ? iop->size : BUFSIZ;
	emalloc(iop->secbuf, char *, iop->secsiz+2, "iop_alloc");
	iop->cnt = -1;
	return iop;
}

int
get_a_record(out, iop, rs)
char **out;
IOBUF *iop;
register int rs;
{
	register char *bp = iop->off;
	register char *end_data = iop->end;	/* end of current data read */
	char *end_buf = iop->buf + iop->size;	/* end of input buffer */
	char *start = iop->off;			/* beginning of record */
	char *offset = iop->secbuf;		/* end of data in secbuf */
	size_t size;

	if (iop->cnt == 0)
		return EOF;

	/* set up sentinels */
	if (rs == 0) {
		*end_data = *(end_data+1) = '\n';
		*end_buf = *(end_buf+1) = '\n';
	} else
		*end_data = *end_buf = rs;

	for (;;) {	/* break on end of record, read error or EOF */

		if (bp == end_data) {
			if (bp == end_buf) {	/* record spans buffer end */
#ifdef atarist
#define P_DIFF ptrdiff_t
#else
#define P_DIFF int
#endif
#define	COPY_TO_SECBUF	{ \
				P_DIFF oldlen = offset - iop->secbuf; \
				P_DIFF newlen = bp - start; \
 								\
				if (iop->secsiz < oldlen + newlen) { \
					erealloc(iop->secbuf, char *, \
						oldlen+newlen, "get_record"); \
					offset = iop->secbuf + oldlen; \
				} \
				memcpy(offset, start, newlen); \
				offset += newlen; \
			}
				COPY_TO_SECBUF
				start = bp = iop->buf;
				size = iop->size;
			} else
				size = end_buf - bp;
			iop->cnt = read(iop->fd, bp, size);
			if (iop->cnt == -1)
				fatal("error reading input");
			else if (iop->cnt == 0) {
				break;
			} else {
				end_data = bp + iop->cnt;
				if (rs == 0 && *bp == '\n'
				    && offset > iop->secbuf
				    && *(offset-1) == '\n') {
					bp++;
					break;
				}
				if (rs == 0) {
					*end_data = *(end_data+1) = '\n';
					*end_buf = *(end_buf+1) = '\n';
				} else
					*end_data = rs;
			}
		}
		if (rs == 0) {
			for (;;) {
				if (*bp++ == '\n' && *bp == '\n') {
					bp++;
					break;
				}
			}
		} else
			while (*bp++ != rs)
				;
		if (bp <= end_data)	/* end of record */
			break;
		bp = end_data;
	}
	if (offset == iop->secbuf && start == bp && iop->cnt == 0) {
		*out = start;
		return EOF;
	}
	iop->off = bp;
	iop->end = end_data;
	if (offset != iop->secbuf) {
		if (start != bp)
			COPY_TO_SECBUF
		start = iop->secbuf;
		bp = offset;
	}
	if (rs == 0) {
		if (*--bp == '\n') {
			*bp = '\0';
			if (*--bp == '\n')
				*bp = '\0';
			else
				bp++;
		} else
			bp++;
	} else if (*--bp == rs)
		;
	else
		bp++;
	*bp = '\0';
	*out = start;
	return bp - start;
}

#ifdef TEST
main(argc, argv)
int argc;
char *argv[];
{
	IOBUF *iop;
	char *out;
	int cnt;

	if (argc > 1)
		bufsize = atoi(argv[1]);
	iop = iop_alloc(0);
	while ((cnt = get_a_record(&out, iop, 0)) > 0) {
		fwrite(out, 1, cnt, stdout);
		fwrite("\n", 1, 1, stdout);
	}
}
#endif
