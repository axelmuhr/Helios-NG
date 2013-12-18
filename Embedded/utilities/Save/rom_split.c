
/*
 * imagesplit.c
 *
 *	Splits an image into 4 separate files along byte boundaries.
 * In each word, byte0 will be placed into image0, byte1 will be placed
 * in image1, etc.
 *
 * Splitting a file called "image" containing two words into 4 would result in -
 *
 *		byte0.byte1.byte2.byte3 | byte0.byte1.byte2.byte3
 *		  |     |     |     |        |    |     |     |
 *	image.0 <-+-----|-----|-----|--------+    |     |     |
 *	image.1 <-------+-----|-----|-------------+     |     |
 *	image.2 <-------------+-----|-------------------+     |
 *	image.3 <-------------------+-------------------------+
 *
 * The same file split into 2 would result in -
 *
 *		byte0.byte1.byte2.byte3 | byte0.byte1.byte2.byte3
 *		  |     |     |     |        |    |     |     |
 *	image.0 <-+-----+-----|-----|--------+----+     |     |
 *	image.1 <-------------+-----+-------------------+-----+
 *
 *	An option to the utility allows the image files to be padded up
 * to a specified size.  Pad byte is 0.
 *
 * Possible extensions -
 *
 * o	Byte swapping ???
 * o	Split into two files for 16bit wide images.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#define strequ_(s1,s2)		(strcmp (s1, s2) == 0)
#define strnequ_(s1,s2,n)	(strncmp (s1, s2, n) == 0)

#define is_dec_digit_(d)	('0' <= (d) && (d) <= '9')
#define is_hex_digit_(d)	(   ('0' <= (d) && (d) <= '9') \
				 || ('a' <= (d) && (d) <= 'f') \
				 || ('A' <= (d) && (d) <= 'F'))

#define TRUE	1
#define FALSE	0

#define MAX_FILE_NAME_LEN	64

#define OK			0
#define FAIL_BADNUMBER		1
#define FAIL_BADFILE		2
#define FAIL_BADREAD		3
#define FAIL_NOTALIGNED		4
#define FAIL_NOPADSIZE		5
#define FAIL_NOSPLITVAL		6
#define FAIL_BADSPLITVAL	7
#define FAIL_NOINDEX		8

typedef unsigned char	uchar;
typedef uchar		byte;

#define PAD_BYTE	(byte)0x00

char *	ProgName;

int	FailNo;

/*
 * Options used when determining how to split the file.
 */
int	SplitImageSize;		/* how big the final split images should be (-p option) */
int	NumberOfSplits;		/* currently always 4 */
int	IndexStart;

void usage (void)
{
	fprintf (stderr, "usage: %s [<options>] <image-file> [[<options>] <image-file> ...]\n\n", ProgName);

	fprintf (stderr, "where <options> can be any combination of ...\n");
	fprintf (stderr, "	-p <n>	pad the split files to size <n> in bytes\n");
	fprintf (stderr, "	-s <n>	split the image into <n> files (only 2 or 4 supported)\n");
	fprintf (stderr, "	-n <n>	index of created split files starts at <n>\n\n");

	fprintf (stderr, "	<n> can be decimal or hexadecimal (hex form preceded by \"0x\"\n\n");
	fprintf (stderr, "Options last for all following files, unless overridden.\n");

	exit (1);
}

int check_dec (char *	number)
{
	while (*number != '\0')
	{
		if (!is_dec_digit_(*number))
		{
			FailNo = FAIL_BADNUMBER;

			return FailNo;
		}

		number++;
	}

	return OK;
}

int check_hex (char *	number)
{
	while (*number != '\0')
	{
		if (!is_hex_digit_(*number))
		{
			FailNo = FAIL_BADNUMBER;

			return FailNo;
		}

		number++;
	}

	return OK;
}

int str2num (char *	str,
	     int *	num)
{
	if (strnequ_(str, "0x", 2))
	{
		/* found a Hex number */
		if (check_hex (str) != OK)
		{
			fprintf (stderr, "%s - error: bad hex number %s found\n",
					ProgName, str);

			return FailNo;
		}

		sscanf (str, "0x%x", num);
	}
	else
	{
		if (check_dec (str) != OK)
		{
			fprintf (stderr, "%s - error: bad dec number %s found\n",
					ProgName, str);

			return FailNo;
		}

		sscanf (str, "%d", num);
	}

	return OK;
}

void split_file (char *	image_name)
{
	struct stat	statbuf;

	FILE *	image_fp;

	int	image_size = 0;	/* size of the split image files */

	int	read_count;

	int	i;

	char 	split_files[4][MAX_FILE_NAME_LEN];
	FILE *	split_fp[4];

	byte	buffer[4];

	fprintf (stderr, "	[split_file (%s)]\n", image_name);

	/* check to see that the image file is present and has a word aligned length */
	if (stat (image_name, &statbuf))
	{
		fprintf (stderr, "%s - error: failed to find image file %s\n", 
				ProgName, image_name);

		FailNo = FAIL_BADFILE;

		return;
	}

	if (statbuf.st_size % 4 != 0)
	{
		fprintf (stderr, "%s - error: %s is not word aligned\n", ProgName, image_name);

		FailNo = FAIL_NOTALIGNED;

		return;
	}	

	/* open the image file */
	if ((image_fp = fopen (image_name, "r")) == NULL)
	{
		fprintf (stderr, "%s - error: failed to open %s\n", ProgName, image_name);
	
		FailNo = FAIL_BADFILE;

		return;
	}

	/* now set up the split file names */
	for (i = 0; i < NumberOfSplits; i++)
	{
		sprintf (split_files[i], "%s.%d", image_name, i + IndexStart);
	}

	printf ("Split files are ... \n");
	for (i = 0; i < NumberOfSplits; i++)
	{
		printf ("\t%s\n", split_files[i]);
	}

	/* open the split files */
	for (i = 0; i < NumberOfSplits; i++)
	{
		if ((split_fp[i] = fopen (split_files[i], "w")) == NULL)
		{
			fprintf (stderr, "%s - error: failed to open split file %s\n", ProgName, split_files[i]);

			/* close any open file pointers */
			fclose (image_fp);
			while (i-- != 0)
			{
				fclose (split_fp[i]);
			}

			FailNo = FAIL_BADFILE;

			return;
		}
	}

	/* now we can split the file */
	while ((read_count = fread (buffer, 1, 4, image_fp)) != 0)
	{
		if (read_count != 4)
		{
			fprintf (stderr, "%s - error: failed to read word from %s\n", ProgName, image_name);

			FailNo = FAIL_BADREAD;

			return;
		}

		switch (NumberOfSplits)
		{
		case 4:

			for (i = 0; i < NumberOfSplits; i++)
			{
				putc (buffer[i], split_fp[i]);
			}

			image_size++;

			break;

		case 2:
			putc (buffer[0], split_fp[0]); putc (buffer[1], split_fp[0]); 
			putc (buffer[2], split_fp[1]); putc (buffer[3], split_fp[1]); 

			image_size += 2;

			break;
		}
	}

	/* finally pad out the image files */
	while (image_size < SplitImageSize)
	{
		for (i = 0; i < 4; i++)
		{
			putc (PAD_BYTE, split_fp[i]);
		}

		image_size++;
	}

	/* the end */
	fclose (image_fp);
	for (i = 0; i < NumberOfSplits; i++)
	{
		fclose (split_fp[i]);
	}

	FailNo = OK;

	return;
}

int setup (int		argc,
	   char *	argv[],
	   int		curr_opt)
{

	/* option */
	char * opt = argv[curr_opt];

	opt++;	/* skip over '-' */

	while (*opt != '\0')
	{
		fprintf (stderr, "	[*opt = '%c']\n", *opt);

		switch (*opt)
		{
		case 'n':
			opt++;
			if (*opt == '\0')
			{
				curr_opt++;
				if (curr_opt == argc)
				{
					fprintf (stderr, "%s - error: no index given\n", ProgName);

					FailNo = FAIL_NOINDEX;
				}
				opt = argv[curr_opt];
			}

			str2num (opt, &IndexStart);

			return curr_opt;
			
		case 'p':
			opt++;
			if (*opt == '\0')
			{
				curr_opt++;
				if (curr_opt == argc)
				{
					fprintf (stderr, "%s - error: no pad size given\n", ProgName);

					FailNo = FAIL_NOPADSIZE;
				}
				opt = argv[curr_opt];
			}

			str2num (opt, &SplitImageSize);

			return curr_opt;

		case 's':
			opt++;
			if (*opt == '\0')
			{
				curr_opt++;
				if (curr_opt == argc)
				{
					fprintf (stderr, "%s - error: no split value given\n", ProgName);

					FailNo = FAIL_NOSPLITVAL;

					return 0;
				}
				opt = argv[curr_opt];
			}

			str2num (opt, &NumberOfSplits);

			if (NumberOfSplits != 2 && NumberOfSplits != 4)
			{
				fprintf (stderr, "%s - error: cannot split an image into %d files\n",
						ProgName, NumberOfSplits);

				FailNo = FAIL_BADSPLITVAL;

				return 0;
			}

			return curr_opt;

		default:
			fprintf (stderr, "%s - warning: unknown option '%c'\n", ProgName, *opt);

			break;
		}

		opt++;
	}

	return curr_opt;
}

int main (int		argc,
	  char *	argv[])
{
	char *	opt;
	int	i;

	int	found_file = FALSE;

	ProgName = argv[0];

	FailNo = OK;

	NumberOfSplits = 4;
	SplitImageSize = -1;
	IndexStart = 0;

	for (i = 1; i < argc; i++)
	{
		opt = argv[i];

		fprintf (stderr, "	[opt = %s]\n", opt);

		if (*opt == '-')
		{
			i = setup (argc, argv, i);

			if (FailNo)
			{
				return FailNo;
			}
		}
		else
		{
			found_file = TRUE;

			split_file (opt);

			if (FailNo)
			{
				return FailNo;
			}
		}
	}

	if (!found_file)
	{
		usage ();
	}
}
