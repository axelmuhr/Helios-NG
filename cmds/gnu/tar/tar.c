/*
 * A public domain tar(1) program.
 * 
 * Written by John Gilmore, ihnp4!hoptoad!gnu, starting 25 Aug 85.
 *
 * @(#)tar.c 1.34 11/6/87 Public Domain - gnu
 */

#include <stdio.h>
#include <sys/types.h>		/* Needed for typedefs in tar.h */

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/tar/RCS/tar.c,v 1.1 1990/08/28 13:20:07 james Exp $";

extern char 	*malloc();
extern char 	*getenv();
extern char	*strncpy();
extern char	*optarg;	/* Pointer to argument */
extern int	optind;		/* Global argv index from getopt */

/*
 * The following causes "tar.h" to produce definitions of all the
 * global variables, rather than just "extern" declarations of them.
 */
#define TAR_EXTERN /**/
#include "tarpriv.h"

/*
 * We should use a conversion routine that does reasonable error
 * checking -- atoi doesn't.  For now, punt.  FIXME.
 */
#define intconv	atoi
extern int	getoldopt();
extern void	read_and();
extern void	list_archive();
extern void	extract_archive();
extern void	diff_archive();
extern void	create_archive();

static FILE	*namef;		/* File to read names from */
static char	**n_argv;	/* Argv used by name routines */
static int	n_argc;	/* Argc used by name routines */
				/* They also use "optind" from getopt(). */

void	describe();


/*
 * Main routine for tar.
 */
main(argc, argv)
	int	argc;
	char	**argv;
{

	/* Uncomment this message in particularly buggy versions...
	fprintf(stderr,
	 "tar: You are running an experimental PD tar, maybe use /bin/tar.\n");
	 */

	tar = "tar";		/* Set program name */

	options(argc, argv);

	name_init(argc, argv);

	if (f_create) {
		if (f_extract || f_list || f_diff) goto dupflags;
		create_archive();
	} else if (f_extract) {
		if (f_list || f_diff) goto dupflags;
		extr_init();
		read_and(extract_archive);
	} else if (f_list) {
		if (f_diff) goto dupflags;
		read_and(list_archive);
	} else if (f_diff) {
		diff_init();
		read_and(diff_archive);
	} else {
dupflags:
		fprintf (stderr,
"tar: you must specify exactly one of the c, t, x, or d options\n");
		describe();
		exit(EX_ARGSBAD);
	}
	exit(0);
	/* NOTREACHED */
}


/*
 * Parse the options for tar.
 */
int
options(argc, argv)
	int	argc;
	char	**argv;
{
	register int	c;		/* Option letter */

	/* Set default option values */
	blocking = DEFBLOCKING;		/* From Makefile */
	ar_file = getenv("TAPE");	/* From environment, or */
	if (ar_file == 0)
		ar_file = DEF_AR_FILE;	/* From Makefile */

	/* Parse options */
	while ((c = getoldopt(argc, argv, "b:BcdDf:hiklmopRstT:vxzZ")
		) != EOF) {
		switch (c) {

		case 'b':
			blocking = intconv(optarg);
			break;

		case 'B':
			f_reblock++;		/* For reading 4.2BSD pipes */
			break;

		case 'c':
			f_create++;
			break;

		case 'd':
			f_diff++;		/* Find difference tape/disk */
			break;

		case 'D':
			f_dironly++;		/* Dump dir, not contents */
			break;

		case 'f':
			ar_file = optarg;
			break;

		case 'h':
			f_follow_links++;	/* follow symbolic links */
			break;

		case 'i':
			f_ignorez++;		/* Ignore zero records (eofs) */
			/*
			 * This can't be the default, because Unix tar
			 * writes two records of zeros, then pads out the
			 * block with garbage.
			 */
			break;

		case 'k':			/* Don't overwrite files */
#ifdef NO_OPEN3
			fprintf(stderr,
				"tar: can't do -k option on this system\n");
			exit(EX_ARGSBAD);
#else
			f_keep++;
#endif
			break;

		case 'l':
			f_local_filesys++;
			break;

		case 'm':
			f_modified++;
			break;

		case 'o':			/* Generate old archive */
			f_oldarch++;
			break;

		case 'p':
			f_use_protection++;
			break;

		case 'R':
			f_sayblock++;		/* Print block #s for debug */
			break;			/* of bad tar archives */

		case 's':
			f_sorted_names++;	/* Names to extr are sorted */
			break;

		case 't':
			f_list++;
			f_verbose++;		/* "t" output == "cv" or "xv" */
			break;

		case 'T':
			name_file = optarg;
			f_namefile++;
			break;

		case 'v':
			f_verbose++;
			break;

		case 'x':
			f_extract++;
			break;

		case 'z':		/* Easy to type */
		case 'Z':		/* Like the filename extension .Z */
			f_compress++;
			break;

		case '?':
			describe();
			exit(EX_ARGSBAD);

		}
	}

	blocksize = blocking * RECORDSIZE;
}


/* 
 * Print as much help as the user's gonna get.
 *
 * We have to sprinkle in the KLUDGE lines because too many compilers
 * cannot handle character strings longer than about 512 bytes.  Yuk!
 * In particular, MSDOS MSC 4.0 (and 5.0) and PDP-11 V7 Unix have this
 * problem.
 */
void
describe()
{

	fputs("\
tar: valid options:\n\
-b N	blocking factor N (block size = Nx512 bytes)\n\
-B	reblock as we read (for reading 4.2BSD pipes)\n\
-c	create an archive\n\
-d	find differences between archive and file system\n\
-D	don't dump the contents of directories, just the directory\n\
", stderr); /* KLUDGE */ fputs("\
-f F	read/write archive from file or device F (or hostname:/ForD)\n\
-h	don't dump symbolic links; dump the files they point to\n\
-i	ignore blocks of zeros in the archive, which normally mean EOF\n\
-k	keep existing files, don't overwrite them from the archive\n\
-l	stay in the local file system (like dump(8)) when creating an archive\n\
", stderr); /* KLUDGE */ fputs("\
-m	don't extract file modified time\n\
-o	write an old V7 format archive, rather than ANSI [draft 6] format\n\
-p	do extract all protection information\n\
-R	dump record number within archive with each message\n\
-s	list of names to extract is sorted to match the archive\n\
-t	list a table of contents of an archive\n\
", stderr); /* KLUDGE */ fputs("\
-T F	get names to extract or create from file F\n\
-v	verbosely list what files we process\n\
-x	extract files from an archive\n\
-z or Z	run the archive through compress(1)\n\
", stderr);
}


/*
 * Set up to gather file names for tar.
 *
 * They can either come from stdin or from argv.
 */
name_init(argc, argv)
	int	argc;
	char	**argv;
{

	if (f_namefile) {
		if (optind < argc) {
			fprintf(stderr, "tar: too many args with -T option\n");
			exit(EX_ARGSBAD);
		}
		if (!strcmp(name_file, "-")) {
			namef = stdin;
		} else {
			namef = fopen(name_file, "r");
			if (namef == NULL) {
				fprintf(stderr, "tar: ");
				perror(name_file);
				exit(EX_BADFILE);
			}
		}
	} else {
		/* Get file names from argv, after options. */
		n_argc = argc;
		n_argv = argv;
	}
}

/*
 * Get the next name from argv or the name file.
 *
 * Result is in static storage and can't be relied upon across two calls.
 */
char *
name_next()
{
	static char	buffer[NAMSIZ+2];	/* Holding pattern */
	register char	*p;
	register char	*q;

	if (namef == NULL) {
		/* Names come from argv, after options */
		if (optind < n_argc)
			return n_argv[optind++];
		return (char *)NULL;
	}
	for (;;) {
		p = fgets(buffer, NAMSIZ+1 /*nl*/, namef);
		if (p == NULL) return p;	/* End of file */
		q = p+strlen(p)-1;		/* Find the newline */
		if (q <= p) continue;		/* Ignore empty lines */
		*q-- = '\0';			/* Zap the newline */
		while (q > p && *q == '/')  *q-- = '\0'; /* Zap trailing /s */
		return p;
	}
	/* NOTREACHED */
}


/*
 * Close the name file, if any.
 */
name_close()
{

	if (namef != NULL && namef != stdin) fclose(namef);
}


/*
 * Gather names in a list for scanning.
 * Could hash them later if we really care.
 *
 * If the names are already sorted to match the archive, we just
 * read them one by one.  name_gather reads the first one, and it
 * is called by name_match as appropriate to read the next ones.
 * At EOF, the last name read is just left in the buffer.
 * This option lets users of small machines extract an arbitrary
 * number of files by doing "tar t" and editing down the list of files.
 */
name_gather()
{
	register char *p;
	static struct name namebuf[1];	/* One-name buffer */

	if (f_sorted_names) {
		p = name_next();
		if (p) {
			namebuf[0].length = strlen(p);
			if (namebuf[0].length >= sizeof namebuf[0].name) {
				fprintf(stderr, "Argument name too long: %s\n",
					p);
				namebuf[0].length = (sizeof namebuf[0].name) - 1;
			}
			strncpy(namebuf[0].name, p, namebuf[0].length);
			namebuf[0].name[ namebuf[0].length ] = 0;
			namebuf[0].next = (struct name *)NULL;
			namebuf[0].found = 0;
			namelist = namebuf;
			namelast = namelist;
		}
		return;
	}

	/* Non sorted names -- read them all in */
	while (NULL != (p = name_next())) {
		addname(p);
	}
}

/*
 * Add a name to the namelist.
 */
addname(name)
	char	*name;			/* pointer to name */
{
	register int	i;		/* Length of string */
	register struct name	*p;	/* Current struct pointer */

	i = strlen(name);
	/*NOSTRICT*/
	p = (struct name *)
		malloc((unsigned)(i + sizeof(struct name) - NAMSIZ));
	if (!p) {
		fprintf(stderr,"tar: cannot allocate mem for namelist entry\n");
		exit(EX_SYSTEM);
	}
	p->next = (struct name *)NULL;
	p->length = i;
	strncpy(p->name, name, i);
	p->name[i] = '\0';	/* Null term */
	p->found = 0;
	p->regexp = 0;		/* Assume not a regular expression */
	p->firstch = 1;		/* Assume first char is literal */
	if (index(name, '*') || index(name, '[') || index(name, '?')) {
		p->regexp = 1;	/* No, it's a regexp */
		if (name[0] == '*' || name[0] == '[' || name[0] == '?')
			p->firstch = 0;		/* Not even 1st char literal */
	}

	if (namelast) namelast->next = p;
	namelast = p;
	if (!namelist) namelist = p;
}


/*
 * Match a name from an archive, p, with a name from the namelist.
 */
name_match(p)
	register char *p;
{
	register struct name	*nlp;
	register int		len;

again:
	if (0 == (nlp = namelist))	/* Empty namelist is easy */
		return 1;
	len = strlen(p);
	for (; nlp != 0; nlp = nlp->next) {
		/* If first chars don't match, quick skip */
		if (nlp->firstch && nlp->name[0] != p[0])
			continue;

		/* Regular expressions */
		if (nlp->regexp) {
			if (wildmat(p, nlp->name)) {
				nlp->found = 1;	/* Remember it matched */
				return 1;	/* We got a match */
			}
			continue;
		}

		/* Plain Old Strings */
		if (nlp->length <= len		/* Archive len >= specified */
		 && (p[nlp->length] == '\0' || p[nlp->length] == '/')
						/* Full match on file/dirname */
		 && strncmp(p, nlp->name, nlp->length) == 0) /* Name compare */
		{
			nlp->found = 1;		/* Remember it matched */
			return 1;		/* We got a match */
		}
	}

	/*
	 * Filename from archive not found in namelist.
	 * If we have the whole namelist here, just return 0.
	 * Otherwise, read the next name in and compare it.
	 * If this was the last name, namelist->found will remain on.
	 * If not, we loop to compare the newly read name.
	 */
	if (f_sorted_names && namelist->found) {
		name_gather();		/* Read one more */
		if (!namelist->found) goto again;
	}
	return 0;
}


/*
 * Print the names of things in the namelist that were not matched.
 */
names_notfound()
{
	register struct name	*nlp;
	register char		*p;

	for (nlp = namelist; nlp != 0; nlp = nlp->next) {
		if (!nlp->found) {
			fprintf(stderr, "tar: %s not found in archive\n",
				nlp->name);
		}
		/*
		 * We could free() the list, but the process is about
		 * to die anyway, so save some CPU time.  Amigas and
		 * other similarly broken software will need to waste
		 * the time, though.
		 */
#ifndef unix
		if (!f_sorted_names)
			free(nlp);
#endif
	}
	namelist = (struct name *)NULL;
	namelast = (struct name *)NULL;

	if (f_sorted_names) {
		while (0 != (p = name_next()))
			fprintf(stderr, "tar: %s not found in archive\n", p);
	}
}
