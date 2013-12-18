/****************************************************************
 * unshar.c: Unpackage one or more shell archive files
 *
 * Usage:	unshar [ -d directory ] [ file ] ...
 *
 * Description:	unshar is a filter which removes the front part
 *		of a file and passes the rest to the 'sh' command.
 *		It understands phrases like "cut here", and also
 *		knows about shell comment characters and the Unix
 *		commands "echo", "cat", and "sed".
 *
 * HISTORY
 *  1-Feb-85  Guido van Rossum (guido@mcvax) at CWI, Amsterdam
 *	Added missing 'quit' routine;
 *	added -d flag to change to directory first;
 *	added filter mode (read stdin when no arguments);
 *	added 'getopt' to get flags (makes it self-contained).
 * 29-Jan-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created.
 ****************************************************************/

# include <stdio.h>
# define EOL '\n'

extern char *optarg;
extern int optind;

main (argc, argv)
int argc;
char *argv[];
{ int i, ch;
  FILE *in;

  /* Process options */

  while ((ch = getopt (argc, argv, "d:")) != EOF) {
    switch (ch) {
    case 'd':
      if (chdir (optarg) == -1) {
	fprintf (stderr, "unshar: cannot chdir to '%s'\n", optarg);
	exit(2);
      }
      break;
    default:
      quit (2, "Usage: unshar [-d directory] [file] ...\n");
    }
  }

  if (optind < argc)
  { for (i= optind; i < argc; ++i)
    { if ((in = fopen (argv[i], "r")) == NULL)
      { fprintf (stderr, "unshar: file '%s' not found\n", argv[i]);
        exit (1);
      }
      process (argv[i], in);
      fclose (in);
    }
  }
  else
    process ("standard input", stdin);

  exit (0);
}


process (name, in)
char *name;
FILE *in;
{ char ch;
  FILE *shpr, *popen();

   if (position (name, in))
    { printf ("%s:\n", name);
      if ((shpr = popen ("sh", "w")) == NULL)
	quit (1, "unshar: cannot open 'sh' process\n");

	while ((ch = fgetc (in)) != EOF)
	  fputc (ch, shpr);

	pclose (shpr);
    }
}

/****************************************************************
 * position: position 'fil' at the start of the shell command
 * portion of a shell archive file.
 ****************************************************************/

position (fn, fil)
char *fn;
FILE *fil;
{ char buf[BUFSIZ];
  long pos, ftell ();

  /* Results from star matcher */
  static char res1[BUFSIZ], res2[BUFSIZ], res3[BUFSIZ], res4[BUFSIZ];
  static char *result[] = { res1, res2, res3, res4 };

  rewind (fil);

  while (1)
  { /* Record position of the start of this line */
    pos = ftell (fil);

    /* Read next line, fail if no more */
    if (fgets (buf, BUFSIZ, fil) == NULL)
    { fprintf (stderr, "unshar: found no shell commands in %s\n", fn);
      return (0);
    }

    /* Bail out if we see C preprocessor commands or C comments */
    if (stlmatch (buf, "#include")	|| stlmatch (buf, "# include") ||
	stlmatch (buf, "#define")	|| stlmatch (buf, "# define") ||
	stlmatch (buf, "#ifdef")	|| stlmatch (buf, "# ifdef") ||
	stlmatch (buf, "#ifndef")	|| stlmatch (buf, "# ifndef") ||
	stlmatch (buf, "/*"))
    { fprintf (stderr,
	       "unshar: %s looks like raw C code, not a shell archive\n", fn);
      return (0);
    }

    /* Does this line start with a shell command or comment */
    if (stlmatch (buf, "#")	|| stlmatch (buf, ":") ||
	stlmatch (buf, "echo ")	|| stlmatch (buf, "sed ") ||
	stlmatch (buf, "cat "))
    { fseek (fil, pos, 0); return (1); }

    /* Does this line say "Cut here" */
    if (smatch (buf, "*CUT*HERE*", result) ||
	smatch (buf, "*cut*here*", result) ||
	smatch (buf, "*TEAR*HERE*", result) ||
	smatch (buf, "*tear*here*", result) ||
	smatch (buf, "*CUT*CUT*", result) ||
	smatch (buf, "*cut*cut*", result))
    {
      /* Read next line after "cut here", skipping blank lines */
      while (1)
      { pos = ftell (fil);

        if (fgets (buf, BUFSIZ, fil) == NULL)
	{ fprintf (stderr,
		"unshar: found no shell commands after 'cut' in %s\n", fn);
	  return (0);
	}
	
	if (*buf != '\n') break;
      }

      /* Win if line starts with a comment character of lower case letter */
      if (*buf == '#' || *buf == ':' || (('a' <= *buf) && ('z' >= *buf)))
      { fseek (fil, pos, 0);
	return (1);
      }

      /* Cut here message lied to us */      
      fprintf (stderr, "unshar: %s is probably not a shell archive,\n", fn);
      fprintf (stderr, "        the 'cut' line was followed by: %s", buf);
      return (0);
    }
  }
}

/*****************************************************************
 * stlmatch  --  match leftmost part of string
 *
 * Usage:  i = stlmatch (big,small)
 *	int i;
 *	char *small, *big;
 *
 * Returns 1 iff initial characters of big match small exactly;
 * else 0.
 *
 * HISTORY
 * 18-May-82 Michael Mauldin (mlm) at Carnegie-Mellon University
 *      Ripped out of CMU lib for Rog-O-Matic portability
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Rewritten for VAX from Ken Greer's routine.
 *
 *  Originally from klg (Ken Greer) on IUS/SUS UNIX
 *****************************************************************/

int   stlmatch (big, small)
char *small, *big;
{ register char *s, *b;
  s = small;
  b = big;
  do
  { if (*s == '\0')
      return (1);
  }
  while (*s++ == *b++);
  return (0);
}

/*****************************************************************
 * smatch: Given a data string and a pattern containing one or
 * more embedded stars (*) (which match any number of characters)
 * return true if the match succeeds, and set res[i] to the
 * characters matched by the 'i'th *.
 *****************************************************************/

smatch (dat, pat, res)
register char *dat, *pat, **res;
{ register char *star = 0, *starend, *resp;
  int nres = 0;

  while (1)
  { if (*pat == '*')
    { star = ++pat; 			     /* Pattern after * */
      starend = dat; 			     /* Data after * match */
      resp = res[nres++]; 		     /* Result string */
      *resp = '\0'; 			     /* Initially null */
    }
    else if (*dat == *pat) 		     /* Characters match */
    { if (*pat == '\0') 		     /* Pattern matches */
	return (1);
      pat++; 				     /* Try next position */
      dat++;
    }
    else
    { if (*dat == '\0') 		     /* Pattern fails - no more */
	return (0); 			     /* data */
      if (star == 0) 			     /* Pattern fails - no * to */
	return (0); 			     /* adjust */
      pat = star; 			     /* Restart pattern after * */
      *resp++ = *starend; 		     /* Copy character to result */
      *resp = '\0'; 			     /* null terminate */
      dat = ++starend; 			     /* Rescan after copied char */
    }
  }
}

/*****************************************************************
 * Addendum: quit subroutine (print a message and exit)
 *****************************************************************/

quit (status, message)
int status;
char *message;
{
  fprintf(stderr, message);
  exit(status);
}

/*****************************************************************
 * Public Domain getopt routine
 *****************************************************************/

/*
 * get option letter from argument vector
 */
int	opterr = 1,		/* useless, never set or used */
	optind = 1,		/* index into parent argv vector */
	optopt;			/* character checked for validity */
char	*optarg;		/* argument associated with option */

#define BADCH	(int)'?'
#define EMSG	""
#define tell(s)	fputs(*nargv,stderr);fputs(s,stderr); \
		fputc(optopt,stderr);fputc('\n',stderr);return(BADCH);

getopt(nargc,nargv,ostr)
int	nargc;
char	**nargv,
	*ostr;
{
	static char	*place = EMSG;	/* option letter processing */
	register char	*oli;		/* option letter list index */
	char	*index();

	if(!*place) {			/* update scanning pointer */
		if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place) return(EOF);
		if (*place == '-') {	/* found "--" */
			++optind;
			return(EOF);
		}
	}				/* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
		if(!*place) ++optind;
		tell(": illegal option -- ");
	}
	if (*++oli != ':') {		/* don't need argument */
		optarg = NULL;
		if (!*place) ++optind;
	}
	else {				/* need an argument */
		if (*place) optarg = place;	/* no white space */
		else if (nargc <= ++optind) {	/* no arg */
			place = EMSG;
			tell(": option requires an argument -- ");
		}
	 	else optarg = nargv[optind];	/* white space */
		place = EMSG;
		++optind;
	}
	return(optopt);			/* dump back option letter */
}
