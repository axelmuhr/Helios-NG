
/*
 * imagesplit.c
 *
 *	Takes a binary image with a length in words, and splits
 * the individual bytes into separate files.
 *
 * Example:
 *
 *	file: image
 *
 *		byte0.byte1.byte2.byte3 | byte0.byte1.byte2.byte3
 *		  |     |     |     |       |     |     |     |
 *	image.0 <-+-----|-----|-----|-------+     |     |     |
 *	image.1 <-------+-----|-----|-------------+     |     |
 *	image.2 <-------------+-----|-------------------+     |
 *	image.3 <-------------------+-------------------------+
 *
 *
 * Usage:
 *
 *	split [<options>] <file> [[<options>] <file> ... [<options>] <file>]
 *
 * where <options> are of the form
 *
 *	-p <size>	The required size of the final image files.  <size>
 *			can be specified in dec or hex form, the latter
 *			being preceded by 0x.
 *			(The files are padded to this size if smaller).
 *
 *	-s <splits>	The number of files the image is to be split into.
 *				4 =>	byte0 - image0, etc
 *				2 =>	byte0 & byte1 - image0,
 *					byte2 & byte3 - image1
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define strequ_(s1,s2)		(strcmp (s1, s2) == 0)
#define strnequ_(s1,s2,n)	(strncmp (s1, s2, n) == 0)


#define debug_	if (Debug) printf

#define TRUE	1
#define FALSE	0

#define PAD_BYTE	0x00

char *	ProgName;

int	FailNo = 0;		/* FailNo is set to non-zero on an error */
#define FAIL_BADSPLITNUM	1

/*
 * Option Global Variables
 *
 *	Options are passed to the splitter function by way of the
 * following global variables.
 */

#define	DEFAULT_PAD	-1	/* => no padding */
#define DEFAULT_SPLIT	4	/* split into 4 files */

int		PadSize = DEFAULT_PAD;
int		Splits  = DEFAULT_SPLIT;

#ifdef DEBUG
int	Debug = 1;
#else
int	Debug = 0;
#endif

int	Verbose = FALSE;

void usage ()
{
	fprintf (stderr, "usage: %s [<options>] <file> [[<options>] <file> ... ]\n",
				ProgName);
	fprintf (stderr, "where <options> is - \n");
	fprintf (stderr, "	-p <size>	pad split files to <size>\n");
	fprintf (stderr, "	-s <splits>	number of image split files\n");
	fprintf (stderr, "	-v 		toggle verbose mode\n");
	fprintf (stderr, "	-d 		toggle debugging\n");

	exit (1);
}

int str2num (char *	str)
{
	int	num;

	if (strnequ_(str, "0x", 2))
	{
		/* found a hex number */
		debug_("	[%s is a hex number]\n", str);

		sscanf (str, "0x%lx", &num);
	}
	else
	{
		/* found a decimal number */
		debug_("	[%s is a dec number]\n", str);

		sscanf (str, "%d", &num);
	}

	debug_("	[returning %d (0x%x)]\n", num, num);

	return num;
}

int handle_option (char *	opt,
		   char *	arg)	/* possible argument to opt */
{
	int	ret_val;

	/* skip over '-' */
	opt++;

	while (*opt)
	{
		switch (*opt)
		{
		case 'p':
			if (*(opt + 1) != '\0')
			{
				/* pad size follows immediately */
				opt++;
				PadSize = str2num (opt);

				return 0;
			}
			else
			{
				/* pad size in separate argument */
				PadSize = str2num (arg);

				return 1;
			}

			break;

		case 's':
			if (*(opt + 1) != '\0')
			{
				opt++;
				Splits = str2num (opt);

				ret_val = 0;
			}
			else
			{
				Splits = str2num (arg);

				ret_val = 1;
			}

			if (Splits != 2 && Splits != 4)
			{
				fprintf (stderr, "error - %s: don't know how to split into %d files\n", 
					ProgName, Splits);

				FailNo = FAIL_BADSPLITNUM;
			}

			return ret_val;

			break;

		case 'd':
			/* toggle debugging */
			Debug = !Debug;

			break;

		case 'v':
			/* toggle verbose mode */
			Verbose = !Verbose;

			break;

		default:
			fprintf (stderr, "warning - %s: unknown option %c\n",
					ProgName, *opt);

			break;
		}

		opt++;
	}

	return 0;
}

void handle_file (char *	file)
{
	int	i;

	FILE *	image_fp;
	FILE *	split_fp[4];

	char 	split_file[4][256];

	unsigned char bytes[4];

	int	bytes_written = 0;

	if (Verbose)
	{
		printf ("Splitting file %s into %d parts\n", file, Splits);
	}

	debug_("	[Splits = %d]\n", Splits);

	/* attempt to open image file */
	if ((image_fp = fopen (file, "rb")) == NULL)
	{
		fprintf (stderr, "warning - %s: failed to open %s\n",
				ProgName, file);

		return;
	}

	debug_("	[opened image file %s]\n", file);

	/* set up split image files */
	for (i = 0; i < Splits; i++)
	{
		sprintf (split_file[i], "%s.%d", file, i);
	}

	debug_("	[created %d split image filenames]\n", Splits);

	/* attempt to open split image files */
	for (i = 0; i < Splits; i++)
	{
		if ((split_fp[i] = fopen (split_file[i], "wb")) == NULL)
		{
			fprintf (stderr, "warning - %s: failed to open image file %s\n",
				ProgName, split_file[i]);

			/* tidyup after the error */
			i--;
			while (i-- >= 0)
			{
				fclose (split_fp[i]);
			}
			fclose (image_fp);

			return;
		}
		else if (Verbose)
		{
			printf ("Opened split file %s\n", split_file[i]);
		}
	}

	debug_("	[opened %d split image files]\n", Splits);

	/* split the file */
	while (fread (bytes, 1, 4, image_fp) == 4)
	{
		if (Splits == 4)
		{
			for (i = 0; i < 4; i++)
			{
				putc (bytes[i], split_fp[i]);
			}
			bytes_written++;
		}
		else /* Splits == 2 */
		{
			putc (bytes[0], split_fp[0]);
			putc (bytes[1], split_fp[0]);
			putc (bytes[2], split_fp[1]);
			putc (bytes[3], split_fp[1]);

			bytes_written += 2;
		}
	}

	if (Verbose)
	{
		printf ("Wrote %d bytes to each file, %d in total\n", bytes_written, bytes_written * Splits);
	}

	debug_("	[split %s]\n", file);

	/* pad, if necessary */
	while (bytes_written < PadSize)
	{
		if (Verbose)
		{
			printf ("Padding with %d bytes\n", PadSize - bytes_written);
		}

		for (i = 0; i < Splits; i++)
		{
			putc (PAD_BYTE, split_fp[i]);
		}
		bytes_written++;
	}
		
	/* tidyup */
	for (i = 0; i < Splits; i++)
	{
		fclose (split_fp[i]);
	}
	fclose (image_fp);
}

int main (int		argc,
	  char *	argv[])
{
	int	i;
	char *	opt;

	int	found_a_file = FALSE;

	ProgName = argv[0];

	for (i = 1; i < argc; i++)
	{
		opt = argv[i];

		if (*opt == '-')
		{
			/* found an option */
			if (i == argc)
			{
				i += handle_option (opt, NULL);

				if (FailNo) break;
			}
			else
			{
				i += handle_option (opt, argv[i + 1]);

				if (FailNo) break;
			}
		}
		else
		{
			/* found a file */
			found_a_file = TRUE;

			handle_file (opt);

			if (FailNo) break;
		}
	}

	if (!found_a_file)
	{
		usage ();
	}
}
