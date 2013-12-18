/*
 *	make [-f makefile] [-ins] [target(s) ...]
 *
 *	(Better than EON mk but not quite as good as UNIX make)
 *
 *	-f makefile name
 *	-i ignore exit status
 *	-n Pretend to make
 *	-p Print all macros & targets
 *	-q Question up-to-dateness of target.  Return exit status 1 if not
 *	-r Don't not use inbuilt rules
 *	-s Make silently
 *	-t Touch files instead of making them
 *	-m Change memory requirements (EON only)
 */

/* RCSID: $HEADER: /hsrc/cmds/make/RCS/main.c,v 1.2 1991/03/03 23:00:07 paul Exp bart $ */

/*
 * Revision history:
 * 21.7.93, V1.10	first numbered revision history (calling it 1.00
 *			after six years seemed a bit silly)
 */
static char *VersionNumber = "1.10";

#include <stdio.h>
#include "h.h"

#ifdef unix
#include <sys/errno.h>
#endif
#ifdef eon
#include <sys/err.h>
#endif
#ifdef os9
#include <errno.h>
#endif
#ifdef amiga
#include <errno.h>
#endif
#ifdef __HELIOS
#include <errno.h>
#endif

#ifdef eon
#define MEMSPACE	(16384)
#endif

#ifdef helios
extern char 	**environ;
#endif

char           *myname;
char           *makefile;	/* The make file  */
#ifdef eon
unsigned        memspace = MEMSPACE;
#endif

FILE           *ifd;		/* Input file desciptor  */
bool            domake = TRUE;	/* Go through the motions option  */
bool            ignore = FALSE;	/* Ignore exit status option  */
bool            silent = FALSE;	/* Silent option  */
bool            print = FALSE;	/* Print debuging information  */
bool            rules = TRUE;	/* Use inbuilt rules  */
bool            dotouch = FALSE;/* Touch files instead of making  */
bool            quest = FALSE;	/* Question up-to-dateness of file  */




#ifdef __STDC__
void
fatal(char * msg, ... )
{
  va_list	args;

  va_start( args, msg );
  
  fprintf(stderr, "%s: ", myname);
  vfprintf( stderr, msg, args );
  fputc('\n', stderr);

  va_end( args );
  
  exit(1);
}
#else
void
fatal(msg, a1, a2, a3, a4, a5, a6)
    char           *msg;
{
    fprintf(stderr, "%s: ", myname);
    fprintf(stderr, msg, a1, a2, a3, a4, a5, a6);
    fputc('\n', stderr);
    exit(1);
}
#endif

void
usage()
{
    fprintf(stderr, "Usage: %s [-f makefile] [-inpqrstv] [macro=val ...] [target(s) ...]\n", myname);
    exit(1);
}

int
main(argc, argv)
    int             argc;
    char          **argv;
{
    register char  *p;		/* For argument processing  */
    int             estat = 0;	/* For question  */
    register struct name *np;
    void            prt(), circh();

    myname = (argc-- < 1) ? "make" : *argv++;

    while ((argc > 0) && (**argv == '-')) {
	argc--;			/* One less to process  */
	p = *argv++;		/* Now processing this one  */

	while (*++p != '\0') {
	    switch (*p) {
	    case 'f':		/* Alternate file name  */
		if (*++p == '\0') {
		    if (argc-- <= 0)
			usage();
		    p = *argv++;
		}
		makefile = p;
		goto end_of_args;
#ifdef eon
	    case 'm':		/* Change space requirements  */
		if (*++p == '\0') {
		    if (argc-- <= 0)
			usage();
		    p = *argv++;
		}
		memspace = atoi(p);
		goto end_of_args;
#endif
	    case 'n':		/* Pretend mode  */
		domake = FALSE;
		break;
	    case 'i':		/* Ignore fault mode  */
		ignore = TRUE;
		break;
	    case 's':		/* Silent about commands  */
		silent = TRUE;
		break;
	    case 'p':
		print = TRUE;
		break;
	    case 'r':
		rules = FALSE;
		break;
	    case 't':
		dotouch = TRUE;
		break;
	    case 'q':
		quest = TRUE;
		break;
	    case 'v':
		fprintf(stderr, "make: version %s\n", VersionNumber);
	        exit(0);
	    default:		/* Wrong option  */
		usage();
	    }
	}
end_of_args:;
    }

#ifdef amiga
    if ((ifd = fopen("s:builtins.make", "r")) != (FILE *) 0) {
	input(ifd);
	fclose(ifd);
    } else
#endif
    makerules();

#ifdef eon
    if (initalloc(memspace) == 0xffff)	/* Must get memory for alloc  */
	fatal("Cannot initalloc memory");
#endif

    if (!makefile)
      {	/* If no file, then use default */
	if ((ifd = fopen(DEFN1, "r")) == (FILE *) 0)
	  {
#ifdef eon
	    if (errno != ER_NOTF)
		fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef unix
	    if (errno != ENOENT)
	      fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef amiga
	    if (errno != ENOENT)
	      fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef __HELIOS
	    if (errno != ENOENT)
	      fatal("Can't open %s; error %02x", DEFN1, errno);
#endif
#ifdef DEFN2
	    if ((ifd == (FILE *) 0)
		&& ((ifd = fopen(DEFN2, "r")) == (FILE *) 0))
	      fatal("Can't open %s", DEFN2);
#else
	    else
	      fatal("Can't open %s", DEFN1);
#endif
	  }
      }
    else if (strcmp(makefile, "-") == 0)	/* Can use stdin as makefile  */
	ifd = stdin;
    else if ((ifd = fopen(makefile, "r")) == (FILE *) 0)
	fatal("Can't open %s", makefile);

    setmacro("$", "$");

    while (argc && (p = index( *argv, '=')) != NULL)
      {
	char            c;

	c = *p;
	*p = '\0';
	setmacro(*argv, p + 1);
	*p = c;

	argv++;
	argc--;
    }

#ifdef helios
	{
		char **env = environ;
		while( *env != NULL )
		{
			char c;
			p = index(*env,'=');
			c = *p; *p = '\0';
			setmacro(*env,p+1);
			*p = c;
			env++;
		}
	}
#endif

    input(ifd);			/* Input all the gunga  */
    fclose(ifd);		/* Finished with makefile  */
    lineno = 0;			/* Any calls to error now print no line
				 * number */

    if (print)
	prt();			/* Print out structures  */

    np = newname(".SILENT");
    if (np->n_flag & N_TARG)
	silent = TRUE;

    np = newname(".IGNORE");
    if (np->n_flag & N_TARG)
	ignore = TRUE;

    precious();

    if (!firstname)
	fatal("No targets defined");

    circh();			/* Check circles in target definitions  */

    if (!argc)
	estat = make(firstname, 0);
    else
	while (argc--) {
	    if (!print && !silent && strcmp(*argv, "love") == 0)
		printf("Not war!\n");
	    estat |= make(newname(*argv++), 0);
	}

    if (quest)
	exit(estat);
    else
	exit(0);
}
