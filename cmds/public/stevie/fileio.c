/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/fileio.c,v 1.1 1993/08/06 15:17:14 nickc Exp tony $
 *
 * Basic file I/O routines.
 */

#include <sys/types.h>		/* For stat() and chmod() */
#include <sys/stat.h>		/* Ditto */
#include "stevie.h"

void
filemess(s)
char	*s;
{
	smsg("\"%s\" %s", (Filename == NULL) ? "" : Filename, s);
	flushbuf();
}

void
renum()
{
	LPTR	*p;
	unsigned long l = 0;

	for (p = Filemem; p != NULL ;p = nextline(p), l += LINEINC)
		p->linep->num = l;

	Fileend->linep->num = 0xffffffff;
}

#define	MAXLINE	256	/* maximum size of a line */

bool_t
readfile(fname,fromp,nochangename)
/*-------------------------------------------------
 * Note that this will try to expand the file name using environment
 * variables.  For this reason, we copy it into an 80-byte buffer,
 * so that there's room to expand it.
 *
 * It uses the environment-variable convention of UNIX, even
 * under systems with other conventions.  That is, your home directory
 * would be called $HOME (even in DOS, where you might want to say %HOME%)
 *-----------------------------------------------------*/
char	*fname;
LPTR	*fromp;
bool_t	nochangename;	/* if TRUE, don't change the Filename */
{
	FILE	*f, *fopen();
	register LINE	*curr;
	char	buff[MAXLINE], buf2[80];
	char	namebuf[80];
	register int	i, c;
	register long	nchars = 0;
	int	linecnt = 0;
	bool_t	wasempty = bufempty();
	int	nonascii = 0;		/* count garbage characters */
	int	nulls = 0;		/* count nulls */
	bool_t	incomplete = FALSE;	/* was the last line incomplete? */
	bool_t	toolong = FALSE;	/* a line was too long */

	curr = fromp->linep;

	strncpy (namebuf, fname, 80);
	EnvEval (namebuf, 80);

	if ( ! nochangename )
		Filename = strsave(namebuf);

	if ( (f=fopen(fixname(namebuf),"r")) == NULL )
		return TRUE;

	filemess("");

	i = 0;
	do {
		c = getc(f);

		if (c == EOF) {
			if (i == 0)	/* normal loop termination */
				break;

			/*
			 * If we get EOF in the middle of a line, note the
			 * fact and complete the line ourselves.
			 */
			incomplete = TRUE;
			c = NL;
		}

		/*
		 * Abort if we get an interrupt, but finished reading the
		 * current line first.
		 */
		if (got_int && i == 0)
			break;

		if (c >= 0x80) {
			c -= 0x80;
			nonascii++;
		}

		/*
		 * If we reached the end of the line, OR we ran out of
		 * space for it, then process the complete line.
		 */
		if (c == NL || i == (MAXLINE-1)) {
			LINE	*lp;

			if (c != NL)
				toolong = TRUE;

			buff[i] = '\0';
			if ((lp = newline(strlen(buff))) == NULL)
				exit(1);

			strcpy(lp->s, buff);

			curr->next->prev = lp;	/* new line to next one */
			lp->next = curr->next;

			curr->next = lp;	/* new line to prior one */
			lp->prev = curr;

			curr = lp;		/* new line becomes current */
			i = 0;
			linecnt++;

		} else if (c == NUL)
			nulls++;		/* count and ignore nulls */
		else {
			buff[i++] = c;		/* normal character */
		}

		nchars++;

	} while (!incomplete && !toolong);

	fclose(f);

	/*
	 * If the buffer was empty when we started, we have to go back
	 * and remove the "dummy" line at Filemem and patch up the ptrs.
	 */
	if (wasempty && nchars != 0) {
		LINE	*dummy = Filemem->linep;	/* dummy line ptr */

		free(dummy->s);				/* free string space */
		Filemem->linep = Filemem->linep->next;
		free((char *)dummy);			/* free LINE struct */
		Filemem->linep->prev = Filetop->linep;
		Filetop->linep->next = Filemem->linep;

		Curschar->linep = Filemem->linep;
		Topchar->linep  = Filemem->linep;
	}

	renum();

	if (got_int) {
		smsg("\"%s\" Interrupt", namebuf);
		got_int = FALSE;
		return FALSE;		/* an interrupt isn't really an error */
	}

	if (toolong) {
		smsg("\"%s\" Line too long", namebuf);
		return FALSE;
	}

	sprintf(buff, "\"%s\" %s%d line%s, %ld character%s",
		namebuf,
		incomplete ? "[Incomplete last line] " : "",
		linecnt, (linecnt != 1) ? "s" : "",
		nchars, (nchars != 1) ? "s" : "");

	buf2[0] = NUL;

	if (nonascii || nulls) {
		if (nonascii) {
			if (nulls)
				sprintf(buf2, " (%d null, %d non-ASCII)",
					nulls, nonascii);
			else
				sprintf(buf2, " (%d non-ASCII)", nonascii);
		} else
			sprintf(buf2, " (%d null)", nulls);
	}
	strcat(buff, buf2);
	msg(buff);

	return FALSE;
}


/*
 * writeit - write to file 'fname' lines 'start' through 'end'
 *
 * If either 'start' or 'end' contain null line pointers, the default
 * is to use the start or end of the file respectively.
 */
bool_t
writeit(fname, start, end)
char	*fname;
LPTR	*start, *end;
{
	FILE	*f, *fopen();
	FILE	*fopenb();		/* open in binary mode, where needed */
	char	*backup;
	register char	*s;
	register long	nchars;
	register int	lines;
	register LPTR	*p;
	struct stat statbuf;
	int	    statres;

	smsg("\"%s\"", fname);

	/* Expand any environment variables left in the name.
	 * fname better be in a variable big enough to handle the
	 * expansion (80 bytes).
	 */
	EnvEval (fname, 80);

	/* If the file already exists, get what we need to know
	 * (like current mode).
	 */
	statres = stat (fname, &statbuf);

	/*
	 * Form the backup file name - change foo.* to foo.bak
	 */
	backup = alloc((unsigned) (strlen(fname) + 5));
	if (backup == NULL) {
		emsg("Can't open file for writing!");
		return FALSE;
	}

	strcpy(backup, fname);
	for (s = backup; *s && *s != '.' ;s++)
		;
	*s = NUL;
	strcat(backup, ".bak");

	/*
	 * Delete any existing backup and move the current version
	 * to the backup. For safety, we don't remove the backup
	 * until the write has finished successfully. And if the
	 * 'backup' option is set, leave it around.
	 */
	rename(fname, backup);


	f = P(P_CR) ? fopen(fixname(fname), "w") : fopenb(fixname(fname), "w");

	if (f == NULL) {
		emsg("Can't open file for writing!");
		free(backup);
		return FALSE;
	}

	/*
	 * If we were given a bound, start there. Otherwise just
	 * start at the beginning of the file.
	 */
	if (start == NULL || start->linep == NULL)
		p = Filemem;
	else
		p = start;

	lines = nchars = 0;
	do {
		fprintf(f, "%s\n", p->linep->s);
		nchars += strlen(p->linep->s) + 1;
		lines++;

		/*
		 * If we were given an upper bound, and we just did that
		 * line, then bag it now.
		 */
		if (end != NULL && end->linep != NULL) {
			if (end->linep == p->linep)
				break;
		}

	} while ((p = nextline(p)) != NULL);

	fclose(f);
	smsg("\"%s\" %d line%s, %ld character%s", fname,
		lines, (lines > 1) ? "s" : "",
		nchars, (nchars > 1) ? "s" : "");

	UNCHANGED;

	/*
	 * Remove the backup unless they want it left around
	 */
	if (!P(P_BK))
		remove(backup);

	free(backup);

	/*
	 * Set the mode of the new file to agree with the old.
	 */
	if (statres==0)
		chmod (fname, statbuf.st_mode);

	return TRUE;
}
