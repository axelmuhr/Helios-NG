/*
 * ctags - first cut at a UNIX ctags re-implementation
 */

/*
 * Caveats:
 *
 * Only simple function declarations are recognized, as in:
 *
 * type
 * fname(...)
 *
 * where "fname" is the name of the function and must come at the beginning
 * of a line. This is the form I always use, so the limitation doesn't
 * bother me.
 *
 * Macros (with or without parameters) of the following form are also detected:
 *
 * "#" [white space] "define" [white space] NAME
 *
 * No sorting or detection of duplicate functions is done.
 *
 * If there are no arguments, a list of filenames to be searched is read
 * from the standard input. Otherwise, all arguments are assumed to be
 * file names.
 *
 * Tony Andrews
 * August 1987
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

int	ac;
char	**av;

main(argc, argv)
int	argc;
char	*argv[];
{
	char	*fname, *nextfile();
	FILE	*tp, *fopen();

	ac = argc;
	av = argv;

	if ((tp = fopen("tags", "w")) == NULL) {
		fprintf(stderr, "Can't create tags file\n");
		exit(1);
	}

	while ((fname = nextfile()) != NULL)
		dofile(fname, tp);

	fclose(tp);
	exit(0);
}

char *
nextfile()	/* returns ptr to next file to be searched, null at end */
{
	static	char	buf[128];
	static	int	ap = 1;
	char	*gets();

	if (ac <= 1) {		/* read from stdin */
		if (feof(stdin))
			return (char *) NULL;
		return (gets(buf));
	} else {
		if (ap < ac)
			return av[ap++];
		else
			return (char *) NULL;
	}
}

#define	LSIZE	512	/* max. size of a line */

#define	BEGID(c)	(isalpha(c) || (c) == '_')
#define	MIDID(c)	(isalpha(c) || isdigit(c) || (c) == '_')

dofile(fn, tp)
char	*fn;
FILE	*tp;
{
	FILE	*fp, *fopen();
	char	*cp, *strchr();
	char	lbuf[LSIZE];
	char	func[LSIZE];
	int	i, j;

	if ((fp = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "Can't open file '%s' - skipping\n", fn);
		return;
	}

	while (fgets(lbuf, LSIZE, fp) != NULL) {

		lbuf[strlen(lbuf)-1] = '\0';	/* bag the newline */

		if (BEGID(lbuf[0])) {		/* could be a function */
			for (i=0; MIDID(lbuf[i]) ;i++)	/* grab the name */
				func[i] = lbuf[i];

			func[i] = '\0';		/* null terminate the name */

			/*
			 * We've skipped to the end of what may be a function
			 * name. Check to see if the next non-whitespace
			 * char is a paren,
			 * and make sure the closing paren is here too.
			 */
			while (lbuf[i]==' ' || lbuf[i]=='\t') i++;
			if (lbuf[i]=='(' && (((cp = strchr(lbuf,')'))!=NULL))) {
				*++cp = '\0';
				fprintf(tp, "%s\t%s\t/^%s$/\n", func,fn,lbuf);
			}

		} else if (lbuf[0] == '#') {	/* could be a define */
			for (i=1; isspace(lbuf[i]) ;i++)
				;
			if (strncmp(&lbuf[i], "define", 6) != 0)
				continue;

			i += 6;			/* skip "define" */

			for (; isspace(lbuf[i]) ;i++)
				;

			if (!BEGID(lbuf[i]))
				continue;

			for (j=0; MIDID(lbuf[i]) ;i++, j++)
				func[j] = lbuf[i];

			func[j] = '\0';		/* null terminate the name */
			lbuf[i] = '\0';
			fprintf(tp, "%s\t%s\t/^%s/\n", func, fn, lbuf);
		}
	}
	fclose(fp);
}
