/*
 * Plug-compatible replacement for getopt() for parsing tar-like
 * arguments.  If the first argument begins with "-", it uses getopt;
 * otherwise, it uses the old rules used by tar, dump, and ps.
 *
 * Written 25 August 1985 by John Gilmore (ihnp4!hoptoad!gnu) and placed
 * in the Pubic Domain for your edification and enjoyment.
 *
 * @(#)getoldopt.c 1.4 2/4/86 Public Domain - gnu
 */

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/tar/RCS/getoldop.c,v 1.1 1990/08/28 13:13:10 james Exp $";

#include <stdio.h>

extern char	*optarg;	/* Points to next arg */
extern int	optind;		/* Global argv index */

extern char	*index();

int
getoldopt(argc, argv, optstring)
	int	argc;
	char	**argv;
	char	*optstring;
{
	static char	*key;		/* Points to next keyletter */
	static char	use_getopt;	/* !=0 if argv[1][0] was '-' */
	char		c;
	char		*place;

	optarg = NULL;
	
	if (key == NULL) {		/* First time */
		if (argc < 2) return EOF;
		key = argv[1];
		if (*key == '-')
			use_getopt++;
		else
			optind = 2;
	}

	if (use_getopt)
		return getopt(argc, argv, optstring);

	c = *key++;
	if (c == '\0') {
		key--;
		return EOF;
	}
	place = index(optstring, c);

	if (place == NULL || c == ':') {
		fprintf(stderr, "%s: unknown option %c\n", argv[0], c);
		return('?');
	}

	place++;
	if (*place == ':') {
		if (optind < argc) {
			optarg = argv[optind];
			optind++;
		} else {
			fprintf(stderr, "%s: %c argument missing\n",
				argv[0], c);
			return('?');
		}
	}

	return(c);
}
