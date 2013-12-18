/*
* BUG !!!
* not a bug, but to make xlatecr run faster, system("mv... should be
* replaced with posix rename() - WHEN THIS RENAME CAN HANDLE ABS PATHS.
*/

/*
 * xlatecr convert cr/lf to lf and vis/versa
 *
 * If no input files exist, stdin is read with the output going to stdout
 *
 * PAB 4/7/88
 */
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/xlatecr.c,v 1.4 1994/05/12 13:39:44 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <limits.h>
#include <posix.h>
#include <string.h>

int reverse = FALSE;

int
xlatecrlf(	/* xlate cr/lf sequences to just lf */
	  FILE *in,
	  FILE *out )
{
	int c, cr = FALSE;

	while ((c = fgetc(in)) != EOF)
	{
		if (c == '\r')
		{
			if (cr == TRUE)
				fputc(c, out);
			else
				cr = TRUE;
		}
		else
		{
			if (cr == TRUE)
			{
				if (c == '\n') /* if we have cr/lf seq. */
				{
					fputc(c, out);
					cr = FALSE;
				}
				else
				{
					fputc('\r', out);
					fputc(c, out);
				}
			}
			else
				fputc(c, out);
		}

	}
	if (cr == TRUE)
		fputc('\r', out);

	return(0);
}


int
xlatelf(	/* xlate lf to cr/lf sequence */
	FILE *in,
	FILE *out )
{
	int c;

	while ((c = fgetc(in)) != EOF)
	{
		if ( c == '\n')
			fputc('\r', out);
		fputc(c, out);
	}

	return(0);
}


int
main(
     int argc,
     char **argv )
{
	int s = 0;
	FILE *fpi, *fpo;

	if (argv[1] != NULL && strcmp(argv[1], "-r") == 0)
	{
		argv++;
		argc--;
		reverse = TRUE;
	}

	if (argc < 2)
	{
	if (reverse)
		xlatelf(stdin,stdout);
	else
		xlatecrlf(stdin,stdout);
	}
	else
	{
		static char tmpf[PATH_MAX];
		static char sys[PATH_MAX + PATH_MAX];
/*** JMP changed above variables to static to prevent stackoverflow ***/	
		while (--argc >= 1)
		{
			if ((fpi = fopen(argv[argc],"rb")) == NULL)
				{
					perror("xlatecr inputfile");
					exit(1);
				}
			tmpnam(tmpf);
			if ((fpo = fopen(tmpf,"wb")) == NULL)
				{
					perror("xlatecr outputfile");
					exit(1);
				}
			if (reverse)
				s = xlatelf(fpi,fpo);
			else
				s = xlatecrlf(fpi,fpo);

			fclose(fpi);
			fclose(fpo);

			if (s)
			{
				fprintf(stderr,"xlatecr: Error in translation, translation abandoned\n");
				unlink(tmpf);
				exit(1);
			}
			else
			{
				if (unlink(argv[argc]))
					{
						perror("xlatecr");
						exit(1);
					}

#ifndef naff
				sprintf(sys,"mv %s %s", tmpf, argv[argc]);
				if (system(sys))
					{
						fprintf(stderr, "xlatecr: Couldnt rename tmpfile");
						unlink(tmpf);
						exit(1);
					}
#else
				if (rename(tmpf, argv[argc]))
					{
						perror("xlatecr");
						exit(1);
					}
#endif
			}
		}
	}
	exit(0);
}
