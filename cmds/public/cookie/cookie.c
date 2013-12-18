/* cookie - print out an entry from the sayings file
 * by Karl Lehenbauer (karl@sugar.uu.net, uunet!sugar!karl)
 *  cookie.c  1.1  1/12/89
 */

#include <stdio.h>
#include <unistd.h>
#include <nonansi.h>
#include "cookie.h"

#ifdef __STDC__
# include <stdlib.h>
#else
# define RAND_MAX	((1 << 15) - 1)
#endif

#ifdef NEVER
#define ENTSIZE 8L		/* @@@ = 6 chars + CR + NL	*/
#else
#define ENTSIZE 7		/* @@@ = 6 chars + NL		*/
#endif

#define METACHAR '%'
#define YES 1
#define NO 0

char *sccs_id = "@(#) fortune cookie program 1.1 1/12/89 by K. Lehenbauer";
char *cookiename = COOKIEFILE;
char *hashname = HASHFILE;

/* really_random - insure a good random return for a range, unlike an arbitrary
 * random() % n, thanks to Ken Arnold, Unix Review, October 1987
 * ...likely needs a little hacking to run under Berkely
 */

int
really_random(int my_range)
{
	int max_multiple, rnum;

	max_multiple = RAND_MAX / my_range;
	max_multiple *= my_range;
	while ((rnum = rand()) >= max_multiple)
		continue;
	return (rnum % my_range);
}

int
main(
  int 	argc,
  char *argv[] )
{
	int nentries, oneiwant, c, sawmeta = 0;
	FILE *hashf, *cookief;
	long cookiepos;

	/* if we got exactly three arguments, use the cookie and hash
	 * files specified
	 */
	if (argc == 3)
	{
		cookiename = argv[1];
		hashname = argv[2];
	}
	/* otherwise if argc isn't one (no arguments, specifying the
	 * default cookie file), barf
	 */
	else if (argc != 1)
	{
		fputs("usage: cookie cookiefile hashfile\n",stderr);
		exit(1);
	}

	/* open the cookie file for read */
	if ((cookief = fopen(cookiename,"r")) == NULL)
	{
		perror(cookiename);
		exit(2);
	}

	/* open the hash file for read */
	if ((hashf = fopen(hashname,"r")) == NULL)
	{
		perror(hashname);
		exit(2);
	}

	/* compute number of cookie addresses in the hash file by
	 * dividing the file length by the size of a cookie address
	 */
	if (fseek(hashf,0L,2) != 0)
	{
		perror(hashname);
		exit(3);
	}
	nentries = (int)ftell(hashf) / ENTSIZE;

	/* seed the random number generator with time in seconds plus
	 * the program's process ID - it yields a pretty good seed
	 * again, thanks to Ken Arnold
	 */
	srand((int)_cputime());

	/* generate a not really random number */
	oneiwant = really_random(nentries);

	/* locate the one I want in the hash file and read the
	 * address found there
	 */
	fseek(hashf,(long)oneiwant * ENTSIZE, 0);
	fscanf(hashf,"%lx",&cookiepos);

	/* seek cookie file to cookie starting at address read from hash */
	fseek(cookief,cookiepos,0);

	/* get characters from the cookie file and write them out
	 * until finding the end-of-fortune sequence, '%%'
	 */
	while ((c = fgetc(cookief)) != EOF && sawmeta < 2)
	{
		if (c != METACHAR)
		{
			if (sawmeta)
				putchar(METACHAR);
			putchar(c);
			sawmeta = 0;
		}
		else
			sawmeta++;
	}
	exit(0);
}

/* end of cookie.c */

