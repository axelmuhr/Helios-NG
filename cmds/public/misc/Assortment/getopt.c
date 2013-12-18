/*
**  GETOPT PROGRAM AND LIBRARY ROUTINE
**
**  I wrote main() and AT&T wrote getopt() and we both put our efforts into
**  the public domain via mod.sources.
**	Rich $alz
**	Mirror Systems
**	(mirror!rs, rs@mirror.TMC.COM)
**	August 10, 1986
**
**  John Fitch converted it into Helios in 1988 April 27
*/

#include <stdio.h>
#include <syslib.h>

char		*index(char *,char);
extern int	 optind;
extern char	*optarg;
int              getopt(int, char **,char *);


int main(ac, av)
    register int	 ac;
    register char 	*av[];
{
    register char 	*flags;
    register int	 c;

    /* Check usage. */
    if (ac < 2) {
	fprintf(stderr, "usage: %s flag-specification arg-list\n", av[0]);
	Exit(2);
    }

    /* Play games; remember the flags (first argument), then splice
       them out so it looks like a "standard" command vector. */
    flags = av[1];
    av[1] = av[0];
    av++;
    ac--;

    /* Print flags. */
    while ((c = getopt(ac, av, flags)) != EOF) {
	if (c == '?')
	    Exit(1);
	/* We assume that shells collapse multiple spaces in `` expansion. */
	printf("-%c %s ", c, index(flags, c)[1] == ':' ? optarg : "");
    }

    /* End of flags; print rest of options. */
    printf("-- ");
    for (av += optind; *av; av++)
	printf("%s ", *av);
    Exit(0);
}



/*
**  This is the public-domain AT&T getopt(3) code.  I added the
**  #ifndef stuff because I include <stdio.h> for the program;
**  getopt, per se, doesn't need it.  I also added the INDEX/index
**  hack (the original used strchr, of course). 
*/


#define ERR(s, c)	if(opterr){\
	(void) fputs(argv[0],stderr);\
	(void) fputs(s,stderr);\
	(void) fputc(c,stderr);\
	(void) fputc('\n',stderr);}

extern int strcmp(char *,char *);
char *index(char *, char);

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1) {
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=index(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

char *index(target,pat)
char *target;
char pat;
{
  for (; *target != '\0'; target++)
      if (*target == pat) return(target);
  return(NULL);
}
