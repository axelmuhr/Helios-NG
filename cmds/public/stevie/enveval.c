/*
 *	Evaluate a string, expanding environment variables
 *	where encountered.
 *	We'll use the UNIX convention for representing environment
 *	variables: $xxx, where xxx is the shortest string that
 *	matches some environment variable.
 */

#include <stdio.h>
#ifdef BSD
#include <strings.h>
#define strchr index
#else
#include <string.h>
#endif
char	*getenv();

int
EnvEval (s, len)
  char *s;
  int   len;
/*------------------------------------------------------------------
 *  s=	Pointer to buffer, currently containing string.  It will be
 *	expanded in-place in the buffer.
 *  len=Maximum allowable length of the buffer.  (In this version, we
 *	use a static buffer of 256 bytes internally.)
 *
 * RETURNS:
 *	 0	on success.
 *	-1	on failure.  In this case, s may contain a partially
 *			converted string, but it won't contain a partial
 *			string.  It will be the FULL string, with as
 *			many substitutions as we could find.
 */

{
#define	LEN	256
	char	buf [LEN];
	char	*s1, *s2;
	char	*b1;
	int	done=0;

	if (len > LEN)
		return (-1);

	s1 = s;

	/*  Check for '$', and expand when we find one.  */
	while (!done) {
		if ((s1 = strchr (s1, '$')) == NULL)
			done = 1;
		else {
			/*
			 *  Here's where the real work gets done.
			 *  We'll find the env.var., and convert
			 *  it into buf, then copy back into s
			 *  and continue.
			 */
			char	c;
			int	need, got;

			/* Test successively longer strings, to see
			 * if they're env.vars.
			 */
			for (s2=++s1+1;	; s2++) {
				c = *s2;	/* save it */
				*s2 = '\0';
				b1 = getenv (s1);
				*s2 = c;		/* restore it */
				if (b1) 		/* found it */
					break;
				if (!*s2)		/* nothing to try */
					goto Failed;
			}
			--s1;	/* Back to the '$' */

			/* OK, we've found one (between s1 & s2,
			 * non-inclusive).  Its value is in b1.
			 * Do the substitution into bufp,
			 * and copy back into s.
			 */
			need = strlen(b1) + strlen(s2) + 1;
			got  = len - (s1-s);
			if (need > got)
				goto Failed;
			strcpy (buf, b1);
			strcat (buf, s2);
			strcpy (s1, buf);
		}
	}

	/*  If we get here, the converted value is in s  */
	return (0);

   Failed:
	return (-1);
}


/* #define SAMPLE */
#ifdef SAMPLE  /***************************************************/

main (int argc, char **argv)
{
	int	i, ret;

	for (i=1; i<argc; i++) {
		printf ("Convert  %s  to", argv [i]);
		ret = EnvEval (argv [i], 80);
		printf ("  %s", argv [i]);
		if (ret)
			printf ("  -  Failed");
		putchar ('\n');
	}
}

#endif
