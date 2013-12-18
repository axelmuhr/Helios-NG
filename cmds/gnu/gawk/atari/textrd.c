/*
 * From gnulib Atari ST sources - with a fix for a bug causing
 * a premature EOF when only characters read are CR's.
 * Written by Eric R. Smith and placed in the public domain.
 * Fix - Michal Jaegermann, June 1991.
 */
#include <stdio.h>
#include <unistd.h>
int
_text_read(fd, buf, nbytes)
	int fd;
	char *buf;
	int nbytes;
{
	char *to, *from;
	int  r;
	do {
		r = read(fd, buf, nbytes);
		if (r <= 0)		/* if EOF or read error - return */
			return r;
		to = from = buf;
		do {
			if (*from == '\r')
				from++;
			else
				*to++ = *from++;
		} while (--r);
	} while (buf == to);	/* only '\r's? - try to read next nbytes */
	return (to - buf);
}
