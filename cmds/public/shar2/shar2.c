/*
** cshar.c
*/

#ifndef lint
#ifdef RCSIDENT
static char *rcsid[] =
{
	"$Header:$",
	"$Locker:$"
};
#endif RCSIDENT
#endif lint
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 Shar puts readable text files together in a package
from which they are easy to extract.  The original version
was a shell script posted to the net, shown below:
	#Date: Mon Oct 18 11:08:34 1982
	#From: decvax!microsof!uw-beave!jim (James Gosling at CMU)
	AR=$1
	shift
	for i do
		echo a - $i
		echo "echo x - $i" >>$AR
		echo "cat >$i <<'!Funky!Stuff!'" >>$AR
		cat $i >>$AR
		echo "!Funky!Stuff!" >>$AR
	done
I rewrote this version in C to provide better diagnostics
and to run faster.  The main difference is that my version
does not affect any files because it prints to the standard
output.  Mine also has a -v (verbose) option.
*/
/*
 *	I have made several mods to this program:
 *
 *	1) the -----Cut Here-----... now preceds the script.
 *	2) the cat has been changed to a sed which removes a prefix
 *	character from the beginning of each line of the extracted
 *	file, this prefix character is added to each line of the archived
 *	files and is not the same as the first character of the
 *	file delimeter.
 *	3) added several options:
 *		-c	- add the -----Cut Here-----... line.
 *		-d'del' - change the file delimeter to del.
 *		-s	- cause the resulting script to print the sum of
 *			  the orignal file and the sum of the extracted
 *			  file.
 *
 *				Michael A. Thompson
 *				Dalhousie University
 *				Halifax, N.S., Canada.
 */

/*
 *	I, too, have been hacking this code. This is the version on sixhub
 *		bill davidsen (davidsen@sixhub.uucp)
 *
 *	- added support for binary files
 *	- automatic creation of limited size multiple file archives,
 *	  each of which may be unpacked separately, and with sequence
 *	  checking.
 *	- support for mixed text and binary files
 *	- preserve file permissions
 *	- restore to filename rather than pathname
 *
 */
/*
 *	Yet another hacker (rnovak@mips) Robert E. Novak
 *	-p when restoring full pathnames, do not emit mkdir
 */

static char *SCCSid =
"@(#)cshar, mips version 1.23, last change 5/21/89";

#define	DELIM           "SHAR_EOF"/* put after each file */
#define PREFIX1	        'X'	/* goes infront of each line */
#define PREFIX2		'Y'	/* goes infront of each line if Delim
starts with PREFIX1 */
#define PREFIX		(Delim[0] == PREFIX1 ? PREFIX2 : PREFIX1)
#define	SHAR            "shar2"	/* the name of this program */
#define	READ_PERMISSION 4	/* access permission */
#define SUM	        "wc -c"

int  Verbose = 0;		/* option to provide append/extract
				   feedback */
int  Sum = 0;			/* option to provide sum checking */
char *Delim = DELIM;		/* pointer to delimiter string */
int  Cut = 0;			/* option to provide cut mark */
int  Binary = 0;		/* flag for binary files */
int  Mixed = 0;			/* mixed text and binary files */
int  Detail = 0;		/* extra detail in shar header */
int  eXists = 0;		/* check if file exists */
int  PosParam = 0;		/* allow positional parameters */
int  FileStrip = 0;		/* strip directories from filenames */
int  Pathname = 1;		/* emit mkdirs by default */
#ifdef	DEBUG
int de_bug = 0;			/* switch for debugging on */
#define DeBug(f,v) if (de_bug) printf(f, v)
#else	/* normal compile */
#define DeBug(f,v)		/* do nothing */
#endif

FILE *popen ();
FILE *outfile = stdout, *fopen ();
char *Rname();			/* file restore name */
unsigned  limit = 0;
long  ftell ();
long  TypePos;			/* position for archive type message */
char  outname[50],		/* base for output filename */
filename[50];		/* actual output filename */
int  filenum = 0;		/* output file # */
struct stat  filestat;		/* check file type, access */

main (argc, argv)
char **argv;
{
	int  status = 0;
	char *oname;

	if (argc<2) {
		helpuser() ;
		exit(2) ;
	}
	while (argv[1][0] == '-')
	{
		switch (argv[1][1])
		{
		case 'v':
			Verbose = 1;
			break;
		case 's':
			Sum = 1;
			break;
		case 'D': /* add detail */
			Detail = 1;
			break;
		case 'd':
			if (argv[1][2])
				Delim = &argv[1][2];
			break;
		case 'b': /* binary files */
			Binary = 1;
			break;
		case 't': /* text mode */
			Binary = 0;
			break;
		case 'x': /* does the file exist */
			eXists = 1;
			if (limit) exit_incompat();
			break;
		case 'c':
			Cut = 1;
			break;
		case 'f': /* filenames only */
			FileStrip = 1;
			break;
		case 'm': /* no mkdirs */
			Pathname = 0;
			break;
		case 'M': /* mixed text and binary */
			Mixed = 1;
			break;
		case 'p': /* allow positional parameters */
			PosParam = 1;
			break;
		case 'l': /* size limit in k */
			limit = atoi (argv[1] + 2) - 1;
			if (eXists) exit_incompat();
			DeBug("Limit %dk\n", limit);
			break;
		case 'o': /* specify output file */
			if (argv[1][2])
				oname = &argv[1][2];
			else
			{ /* use the next arg */
				if (argc > 1 && argv[2][0] != '-')
				{ /* if the next one isn't an option */
					oname = argv[2];
					argc--;
					argv++;
				}
				else
				{ /* format error in the -o option */
					fprintf (stderr, "Format error in -o option\n");
					exit (1);
				}
			}
			strcpy (outname, oname);
			filenum = 1;
			sprintf (filename, "%s01", outname);
			outfile = fopen (filename, "w");
			if (outfile == NULL)
			{ /* creation error */
				perror ("can't create output file");
				exit (1);
			}
			break;
#ifdef	DEBUG
		case '$': /* totally undocumented $ option, debug on */
			de_bug = 1;
			break;
#endif
		default: /* invalid option */
			fprintf (stderr, "%s: invalid argument: %s\n", SHAR, argv[1]);
		case 'h': /* help */
			helpuser ();
			break;
		}
		argc--;
		argv++;
	}
	if (argc == 1)
	{
		fprintf (stderr, "%s: No input files\n", SHAR);
		helpuser ();
		exit (1);
	}
	if (header (argc, argv))
		exit (2);
	while (--argc)
	{ /* process positional parameters and files */
		argv++;
		if (PosParam)
		{ /* allow -b and -t inline */
			if (strcmp (*argv, "-b") == 0)
			{ /* set binary */
				Binary = 1;
				continue;
			}
			if (strcmp (*argv, "-t") == 0)
			{ /* set mode text */
				Binary = 0;
				continue;
			}
		}
		status += shar (*argv);
	}

	/* delete the sequence file, if any */
	if (limit && filenum > 1)
	{
		fputs ("rm -f s2_seq_.tmp\n", outfile);
		fputs ("echo \"You have unpacked the last part\"\n", outfile);
		if (!Verbose)
			fprintf (stderr, "Created %d files\n", filenum);
	}
	fputs ("exit 0\n", outfile);
	exit (status);
}

header (argc, argv)
char **argv;
{
	int  i;
	int  status;
	FILE *ptemp;		/* pipe temp */

	/* see if any conflicting options */
	if (limit && !filenum)
	{ /* can't rename what you don't have */
		fprintf (stderr, "Can't use -l option without -o\n");
		helpuser ();
	}

	for (i = 1; i < argc; i++)
	{ /* skip positional parameters */
		if (PosParam && (strcmp (argv[i], "-b") == 0 ||
		    strcmp (argv[i], "-t") == 0))
			continue;

		/* see if access and correct type */
		if (access (argv[i], READ_PERMISSION))
		{
			fprintf (stderr, "%s: Can't access %s\n", SHAR, argv[i]);
			return 1;
		}

		/* get file type */
		stat (argv[i], &filestat);
		status = filestat.st_mode & S_IFMT;

		/* at this point I check to see that this is a regular file */
		if (status != S_IFREG)
		{ /* this is not a regular file */
			fprintf (stderr, "%s: %s is not a file\n", SHAR, argv[i]);
			return 1;
		}
	}

	if (Cut)
		fputs ("---- Cut Here and unpack ----\n", outfile);
	fputs ("#!/bin/sh\n", outfile);
	fprintf (outfile, "# %s:	Shell Archiver  (v1.22)\n", SHAR);
	if (Detail)
	{
		char  dateout[50],	/* pipe output, who */
		whoout[50];	/* and who am i */

		ptemp = popen ("date; who am i; pwd", "r");
		fgets (dateout, 50, ptemp);
		dateout[strlen (dateout) - 1] = 0;
		fscanf (ptemp, "%s", whoout);
		fprintf (outfile, "#\tPacked %s by %s\n", dateout, whoout);
		while (getc (ptemp) != '\n');
		fgets (dateout, 50, ptemp);
		fprintf (outfile, "#\tfrom directory %s", dateout);

		pclose (ptemp);
	}

	if (limit)
	{ /* may be split, explain */
		fprintf(outfile, "#\n");
		TypePos = ftell(outfile);
		fprintf (outfile, "%-75s\n%-75s\n", "#", "#");
	}

	fputs ("#\n#\tRun the following text with /bin/sh to create:\n", outfile);
	for (i = 1; i < argc; i++)
	{ /* output names of files but not parameters */
		if (PosParam && (strcmp (argv[i], "-b") == 0 ||
		    strcmp (argv[i], "-t") == 0))
			continue;
		fprintf (outfile, "#\t  %s\n", Rname(argv[i]));
	}
	fputs ("#\n", outfile);

	if (limit)
	{ /* now check the sequence */
		fprintf (outfile, "%s%s%s%s",
		    "if test -r s2_seq_.tmp\n",
		    "then echo \"Must unpack archives in sequence!\"\n",
		    "     next=`cat s2_seq_.tmp`; echo \"Please unpack part $next next\"\n",
		    "     exit 1; fi\n");
	}
	return (0);
}

shar (file)
char *file;
{
	char  line[BUFSIZ];
	FILE *ioptr, *fdopen ();
	long  cursize, remaining, ftell ();
	int  split = 0;		/* file split flag */
	char *filetype;		/* text or binary */
	char *RstrName;		/* name for restore */

	/* if limit set, get the current output length */
	if (limit)
	{
		cursize = ftell (outfile);
		remaining = (limit * 1024L) - cursize;
		DeBug("In shar: remaining size %ld\n", remaining);
	}

	/* determine the name to use for restore */
	RstrName = Rname(file);

	/* if pathname contains '/' then emit a series of mkdirs */
	if (PathName (RstrName))
	{
		EmitPath(RstrName) ;
	}

	/* if mixed, determine the file type */
	if (Mixed)
	{
		int  count;
		sprintf (line, "file %s | egrep -c \"text|shell|script\"", file);
		ioptr = popen (line, "r");
		fscanf (ioptr, "%d", &count);
		pclose (ioptr);
		Binary = (count != 1);
	}

	if (Binary)
	{ /* fork a uuencode process */
		static int  pid, pipex[2];

		pipe (pipex);
		fflush (outfile);

		if (pid = fork ())
		{ /* parent, create a file to read */
			close (pipex[1]);
			ioptr = fdopen (pipex[0], "r");
		}
		else
		{ /* start writing the pipe with encodes */
			FILE *outptr;

			ioptr = fopen (file, "rb");
			outptr = fdopen (pipex[1], "w");
			fprintf (outptr, "begin 600 %s\n", RstrName);
			encode (ioptr, outptr);
			fprintf (outptr, "end\n");
			exit (0);
		}
		filetype = "Binary";
	}
	else
	{
		ioptr = fopen (file, "r");
		filetype = "Text";
	}
	
	    if (ioptr != NULL)
	{
		/* protect existing files */
		if (eXists)
			fprintf (outfile,
			    "if test -f %s; then echo \"File %s exists\"; else\n",
			    RstrName, RstrName);

		if (Verbose)
		{ /* info on archive and unpack */
			fprintf (stderr, "%s: saving %s (%s)\n",
			    SHAR, file, filetype);
			fprintf (outfile, "echo \"x - extracting %s (%s)\"\n",
			    RstrName, filetype);
		}
		if (Binary)
		{ /* run sed through uudecode via temp file */
			fprintf (outfile, "sed 's/^%c//' << '%s' > s2_temp_.tmp &&\n",
			    PREFIX, Delim);
		}
		else
		{ /* just run it into the file */
			fprintf (outfile, "sed 's/^%c//' << '%s' > %s &&\n",
			    PREFIX, Delim, RstrName);
		}
		while (fgets (line, BUFSIZ, ioptr))
		{ /* output a line and test the length */
			fprintf (outfile, "%c%s", PREFIX, line);
			if (limit && (remaining -= strlen (line) + 2) < 0)
			{ /* change to another file */
				DeBug("Newfile, remaining %ld, ", remaining);
				DeBug("limit still %d\n", limit);
				fprintf (outfile, "%s\n", Delim);
				if (Verbose)
				{ /* output some reassurance */
					fprintf (outfile,
					    "echo \"End of part %d\"\n", filenum);
					fprintf (outfile,
					    "echo \"File %s is continued in part %d\"\n",
					    RstrName, filenum + 1);
				}
				else
					fprintf (outfile,
					    "echo \"End of part %d, continue with part %d\"\n",
					    filenum, filenum + 1);
				fprintf (outfile, "echo \"%d\" > s2_seq_.tmp\n", filenum + 1);
				fprintf (outfile, "exit 0\n");

				if (filenum == 1)
				{ /* rewrite the info lines on the firstheader */
					fseek(outfile, TypePos, 0);
					fprintf (outfile, "%-75s\n%-75s\n",
					    "# This is part 1 of a multipart archive",
					    "# do not concatenate these parts, unpack them in order with /bin/sh");
				}
				fclose (outfile);

				/* form the next filename */
				sprintf (filename, "%s%02d", outname, ++filenum);
				outfile = fopen (filename, "w");
				if (Cut)
					fputs ("---- Cut Here and unpack ----\n", outfile);
				fprintf (outfile, "#!/bin/sh\n");
				fprintf (outfile,
				    "# this is part %d of a multipart archive\n%s%s",
				    filenum, "# do not concatenate these parts, ",
				    "unpack them in order with /bin/sh\n");
				fprintf (outfile, "# file %s continued\n#\n", RstrName);
				fprintf (outfile, "CurArch=%d\n", filenum);
				fprintf (outfile, "%s%s%s%s%s%s%s%s%s",
				    "if test ! -r s2_seq_.tmp\n",
				    "then echo \"Please unpack part 1 first!\"\n",
				    "     exit 1; fi\n",
				    "( read Scheck\n",
				    "  if test \"$Scheck\" != $CurArch\n",
				    "  then echo \"Please unpack part $Scheck next!\"\n",
				    "       exit 1;\n",
				    "  else exit 0; fi\n",
				    ") < s2_seq_.tmp || exit 1\n");

				if (Verbose)
				{ /* keep everybody informed */
					fprintf (stderr, "Starting file %s\n", filename);
					fprintf (outfile,
					    "echo \"x - Continuing file %s\"\n", RstrName);
				}
				fprintf (outfile,
				    "sed 's/^%c//' << '%s' >> %s\n",
				    PREFIX, Delim, (Binary ? "s2_temp_.tmp" : RstrName));
				remaining = limit * 1024L;
				split = 1;
			}
		}
		
		    (void) fclose (ioptr);
		fprintf (outfile, "%s\n", Delim);
		if (split && Verbose)
			fprintf (outfile,
			    "echo \"File %s is complete\"\n", RstrName);

		/* if this file was uuencoded, decode it and drop the temp */
		if (Binary)
		{
			if (Verbose)
				fprintf (outfile, "echo \"uudecoding file %s\"\n", RstrName);
			fprintf (outfile,
			    "uudecode < s2_temp_.tmp && rm -f s2_temp_.tmp &&\n");
		}

		/* set the permissions as they were */
		stat (file, &filestat);
		fprintf (outfile,
		    "chmod %04o %s || echo \"restore of %s fails\"\n",
		    filestat.st_mode & 07777, RstrName, RstrName);

		if (Sum)
		{ /* validate the transferred file */
			FILE *pfp;
			char  command[BUFSIZ];

			sprintf (command, "%s %s", SUM, file);
			if ((pfp = popen (command, "r")))
			{
				char  sum[BUFSIZ];

				fscanf (pfp, "%s", sum);
				fprintf (outfile, "set `%s %s`;Sum=$1\n",
				    SUM, RstrName);
				fprintf (outfile,
				    "if test \"$Sum\" != \"%s\"\n", sum);
				fprintf (outfile,
				    "then echo original size %s, current size $Sum;fi\n",
				    sum);
				pclose (pfp);
			}
		}

		/* if the exists option is in place close the if */
		if (eXists)
			fprintf (outfile, "fi\n");

		return (0);
	}
	else
	{
		fprintf (stderr, "%s: Can't open %s\n", SHAR, file);
		return (1);
	}
}

char *Rname(file)
register char *file;
{
	register char *RstrName;

	if (FileStrip)
	{ /* use just the filename */
		RstrName = file+strlen(file);
		while (RstrName > file && *RstrName != '/')
			RstrName--;
		if (*RstrName == '/') RstrName++;
	}
	else RstrName = file;
	return (RstrName);
}

#define EOS '\0'
int
PathName(file)
register char *file;
{
	char *	strptr ;

	strptr = file ;
	while (*strptr != EOS) {
		if (*strptr == '/') {
			return(1) ;
		}
		strptr++ ;
	}
	return(0) ;
}

EmitPath(file)
char *	file ;
{
	char *	strptr ;
	char *	pathname ;
	int		pathlength ;
	char *	malloc() ;

	strptr = file ;
	while (*strptr != EOS) {
		if (*strptr == '/') {
			pathlength = (int)((int)strptr - (int)file );
			pathname = malloc(pathlength) + 1 ;
			strncpy(pathname,file,(pathlength)) ;
			pathname[pathlength]=EOS ;
			if (strcmp(pathname,".") != 0) {
				fprintf(outfile,
				    "if [ ! -d %s ]; then\n", pathname) ;
				if (Verbose) { /* info on archive and unpack */
					fprintf (outfile, 
					    "\techo \"x - making Directory %s for %s\"\n",
					    pathname, file);
				}
				fprintf(outfile,
				    "\tmkdir %s\nfi\n",
				    pathname) ;
			}
		}
		strptr++ ;
	}
}
/*****************************************************************
 |  exit_incompat - incompatible options
 ****************************************************************/

exit_incompat()
{
	fprintf(stderr, "%s%s",
	    "The -x and -l options are currently incompatible\n",
	    "Please reenter the command without one option\n");
	exit(1);
}

helpuser ()
{				/* output a command format message */
	register char **ptr;
	static char *helpinfo[] =
	{
		"-v\tverbose messages while executing",
		"-s\tdo checking with 'sum' after unpack",
		"-x\tdon't overwrite existing files",
		"-b\ttreat all files as binary, use uuencode",
		"-t\ttreat all files as text (default)",
		"-p\tallow positional parameter options. The options \"-b\"",
		"\tand \"-t\" may be embedded, and files to the right of the",
		"\toption will be processed in the specified mode",
		"-m\tmkdirs are disabled for restoral",
		"-M\tmixed mode. Determine if the files are text or",
		"\tbinary and archive correctly.",
		"-D\toutput date, user, and directory comments to the archive",
		"-c\tstart the shar with 'cut here'",
		"-f\trestore by filename only, rather than path",
		"-dXXX\tuse XXX to delimit the files in the shar",
		"-oXXX\t(or -o XXX) output to file XXX01 thru XXXnn",
		"-lXX\tlimit output file size to XXk bytes",
		"\nThe 'o' option is required if the 'l' option is used",
		NULL
	};
	fprintf (stderr,
	    "\nFormat:\n  %s [ options ] file [ file1 ... ] ]\n",
	    SHAR);
	for (ptr = helpinfo; *ptr; ptr++)
		fprintf (stderr, "%s\n", *ptr);

	exit (1);
}
