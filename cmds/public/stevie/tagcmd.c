/* #Header: /?????
 *
 * Routines to implement tags, and, especially, tag stacking.
 * Added by Dave Tutelman - 12/89.
 * The dotag() routine is a modification of the posted
 * version by Tony Andrews.
 * The untag() routine is new.
 */

#include "stevie.h"

#define	LSIZE	256	/* max. size of a line in the tags file */

#ifdef TAGSTACK
/*  We build a stack of file records, on which we push info about
 *  current file when dotag() is called.
 */
#define	TAGSTACKSIZE	12		/* how many tag calls can we stack? */
static	struct filerecord {
		char	*name;		/* file name pointer */
		int	linenum;	/* line number when we left */
} tagstack [TAGSTACKSIZE];
static	int	stackindex = 0;		/* here's how we keep track */
#endif

extern	char	**files;
extern	int	curfile;
static	void	pushtags(), poptags();


/*
 * dotag(tag, force) - goto tag.  If force=TRUE, dump pending changes.
 */
void
dotag(tag, force)
char	*tag;
bool_t	force;
{
	FILE	*tp, *fopen();
	char	lbuf[LSIZE];		/* line buffer */
	char	pbuf[LSIZE];		/* search pattern buffer */
	register char	*fname, *str;
	register char	*p;

	if ((tp = fopen("tags", "r")) == NULL) {
		emsg("Can't open tags file");
		return;
	}

	while (fgets(lbuf, LSIZE, tp) != NULL) {
	
		if ((fname = strchr(lbuf, TAB)) == NULL) {
			emsg("Format error in tags file");
			return;
		}
		*fname++ = '\0';
		if ((str = strchr(fname, TAB)) == NULL) {
			emsg("Format error in tags file");
			return;
		}
		*str++ = '\0';

		if (strcmp(lbuf, tag) == 0) {

			/*
			 * Scan through the search string. If we see a magic
			 * char, we have to quote it. This lets us use "real"
			 * implementations of ctags.
			 */
			p = pbuf;
			*p++ = *str++;		/* copy the '/' or '?' */
			*p++ = *str++;		/* copy the '^' */

			for (; *str != NUL ;str++) {
				if (*str == '\\') {
					*p++ = *str++;
					*p++ = *str;
				} else if (strchr("/?", *str) != NULL) {
					if (str[1] != '\n') {
						*p++ = '\\';
						*p++ = *str;
					} else
						*p++ = *str;
				} else if (strchr("^()*.", *str) != NULL) {
					*p++ = '\\';
					*p++ = *str;
				} else
					*p++ = *str;
			}
			*p = NUL;

#ifdef TAGSTACK
			/* Push current position onto stack, unless this
			 * is a startup call using '-t' option. */
			if (Filename!=NULL && *Filename!='\0')
				pushtags ();
#endif

			/*
			 * This looks out of order, but by calling stuffin()
			 * before doecmd() we keep an extra screen update
			 * from occuring. This stuffins() have no effect
			 * until we get back to the main loop, anyway.
			 */

			stuffin(pbuf);		/* str has \n at end */
			stuffin("\007");	/* CTRL('g') */

			if (doecmd(fname, force)) {
				fclose(tp);
				return;
			} else {
#ifdef TAGSTACK
				poptags ();	/* cancel stack entry */
#endif
				stuffin(NULL);	/* clear the input */
			}
		}
	}
	emsg("tag not found");
	fclose(tp);
}

/*
 * dountag (spec) - undo the last ':ta' command, popping the tag stack.
 *	spec is the appended character, giving specifics:
 *	  '!'	dump pending changes.
 *	  'e'	came from K_CCIRCM "shortcut".  do :e# if stack empty.
 *	  ' '	do normal untag.
 *	  else	bad command.
 */

void
dountag (spec)
  char spec;
{
#ifndef TAGSTACK
	badcmd();	/* complain & return */
}
#else

	char	force=0, shortcut=0;
	char	*newfile;
	char	buf [LSIZE];

	switch (spec) {
	  case '!':
		force++;
		break;
	  case 'e':
		shortcut++;
		break;
	  case ' ':
	  case '\n':
	  case '\r':
	  case '\0':
		break;
	  default:
		badcmd();
		return;
	}

	/* Check the stack.  If empty, don't pop */
	if (!stackindex) {
		if (shortcut)		/* just edit altfile */
			stuffin(":e #\n");
		else
			emsg("Tags stack empty");
		return;
	}

	/* Get the top of the stack, and do the implied edit.
	 * If it succeeds, switch; if not, back off */
	newfile = tagstack [stackindex-1].name;
	if (doecmd (newfile, force)) {
		sprintf (buf, "%dG:f\n", tagstack [stackindex-1].linenum);
		stuffin(buf);
		poptags ();
		return;
	}
	else
		stuffin(NULL);
}

/*
 * pushtags () - push the current state onto the tagstack.
 */

static void
pushtags ()
{
	int	i;

	/* If stack full, throw away oldest and push the rest.
	 * This is clearly the best and most transparent way to behave,
	 * much preferable to either complaining or losing new entry.
	 */
	if (stackindex >= TAGSTACKSIZE) {
		for (i=0; i<TAGSTACKSIZE-1; i++) {
			tagstack[i].name    = tagstack[i+1].name;
			tagstack[i].linenum = tagstack[i+1].linenum;
		}
		stackindex--;
	}

	/* Get current state, and put it in stack.
	 * Right now, the state is file name & line number.
	 * This is less than perfect, in that line numbers may change if
	 * you edit elsewhere in the file.  Eventually, I'd like to base
	 * it on "hidden" marks, if I can implement them.  DMT
	 */
	tagstack [stackindex].name    = strsave (Filename);
	tagstack [stackindex].linenum = cntllines (Filemem, Curschar);
	stackindex++;
}

/*
 * poptags () - pop the tag stack.
 */

static void
poptags ()
{
	if (!stackindex) {
		emsg("Tags stack empty");
		return;
	}

	stackindex--;
	free (tagstack [stackindex].name);
}

#endif


