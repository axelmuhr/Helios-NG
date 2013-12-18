/*
 * $XConsortium: main.c,v 1.21 89/02/09 15:59:07 jim Exp $
 */
#include "def.h"
#ifdef hpux
#define sigvec sigvector
#endif /* hpux */
#include	<sys/signal.h>

#ifdef DEBUG
int	debug;
#endif

char	*directives[] = {
	"if",
	"ifdef",
	"ifndef",
	"else",
	"endif",
	"define",
	"undef",
	"include",
	"line",
	"pragma",
	"elif",
	"eject",
	NULL
};

struct symtab	predefs[] = {
#ifdef apollo
	{"apollo", NULL},
#endif
#ifdef ibm032
	{"ibm032", NULL},
#endif
#ifdef sun
	{"sun", NULL},
#endif
#ifdef hpux
	{"hpux", NULL},
#endif
#ifdef vax
	{"vax", NULL},
#endif
#ifdef VMS
	{"VMS", NULL},
#endif
#ifdef cray
	{"cray", NULL},
#endif
#ifdef CRAY
	{"CRAY", NULL},
#endif
#ifdef mips
	{"mips", NULL},
#endif
#ifdef ultrix
	{"ultrix", NULL},
#endif
	{NULL, NULL}
};

struct symtab	deflist[ MAXDEFINES ];
struct	inclist inclist[ MAXFILES ],
		*inclistp = inclist;

char	*filelist[ MAXFILES ];
char	*includedirs[ MAXDIRS ];
char	*notdotdot[ MAXDIRS ];
char	*objfile = ".o";
char	*startat = "# DO NOT DELETE THIS LINE -- make depend depends on it.";
int	width = 78;
boolean	printed = FALSE;
boolean	verbose = FALSE;
boolean	show_where_not = FALSE;
#if defined (mips) && defined (SYSTYPE_SYSV)
void  catch();
#else /* !(mips && SYSTYPE_SYSV) */
#ifdef ultrix
void  catch();
#else
int   catch();
#endif

struct sigvec sig_vec = {
	catch,
	 (1<<(SIGINT -1))
	|(1<<(SIGQUIT-1))
	|(1<<(SIGBUS-1))
	|(1<<(SIGILL-1))
	|(1<<(SIGSEGV-1))
	|(1<<(SIGHUP-1))
	|(1<<(SIGPIPE-1))
	|(1<<(SIGSYS-1)),
	0
};
#endif /* mips && SYSTYPE_SYSV */

main(argc, argv)
	int	argc;
	char	**argv;
{
	register struct symtab	*symp = deflist;
	register char	**fp = filelist;
	register char	**incp = includedirs;
	register char	*p;
	register struct inclist	*ip;
	char	*makefile = NULL;
	struct filepointer	*filecontent;
	struct symtab *psymp = predefs;
	char *endmarker = NULL;

	while (psymp->s_name)
	    *symp++ = *psymp++;
	for(argc--, argv++; argc; argc--, argv++) {
	    	/* if looking for endmarker then check before parsing */
		if (endmarker && strcmp (endmarker, *argv) == 0) {
		    endmarker = NULL;
		    continue;
		}
		if (**argv != '-') {
			*fp++ = argv[0];
			continue;
		}
		switch(argv[0][1]) {
		case '-':
			endmarker = &argv[0][2];
			if (endmarker[0] == '\0') endmarker = "--";
			break;
		case 'D':
			symp->s_name = argv[0]+2;
			if (*symp->s_name == '\0') {
				symp->s_name = *(++argv);
				argc--;
			}
			for (p=symp->s_name; *p ; p++)
				if (*p == '=') {
					*p++ = '\0';
					break;
				}
			symp->s_value = p;
			symp++;
			break;
		case 'I':
			*incp++ = argv[0]+2;
			if (**(incp-1) == '\0') {
				*(incp-1) = *(++argv);
				argc--;
			}
			break;
		/* do not use if endmarker processing */
		case 'w':
			if (endmarker) break;
			if (argv[0][2] == '\0') {
				argv++;
				argc--;
				width = atoi(argv[0]);
			} else
				width = atoi(argv[0]+2);
			break;
		case 'o':
			if (endmarker) break;
			if (argv[0][2] == '\0') {
				argv++;
				argc--;
				objfile = argv[0];
			} else
				objfile = argv[0]+2;
			break;
		case 'v':
			if (endmarker) break;
			verbose = TRUE;
#ifdef DEBUG
			if (argv[0][2])
				debug = atoi(argv[0]+2);
#endif
			break;
		case 's':
			if (endmarker) break;
			startat = argv[0]+2;
			if (*startat == '\0') {
				startat = *(++argv);
				argc--;
			}
			if (*startat != '#')
				log_fatal("-s flag's value should start %s\n",
					"with '#'.");
			break;
		case 'f':
			if (endmarker) break;
			makefile = argv[0]+2;
			if (*makefile == '\0') {
				makefile = *(++argv);
				argc--;
			}
			break;
		
		/* Ignore -O, -g so we can just pass ${CFLAGS} to
		   makedepend
		 */
		case 'O':
		case 'g':
			break;
		default:
			if (endmarker) break;
	/*		log_fatal("unknown opt = %s\n", argv[0]); */
			log("ignoring option %s\n", argv[0]);
		}
	}
	*incp++ = INCLUDEDIR;

	redirect(startat, makefile);

	/*
	 * catch signals.
	 */
#if defined (mips) && defined (SYSTYPE_SYSV)
/*  should really reset SIGINT to SIG_IGN if it was.  */
	signal (SIGHUP, catch);
	signal (SIGINT, catch);
	signal (SIGQUIT, catch);
	signal (SIGILL, catch);
	signal (SIGBUS, catch);
	signal (SIGSEGV, catch);
	signal (SIGSYS, catch);
#else /* not (mips && SYSTYPE_SYSV) */
	sigvec(SIGHUP, &sig_vec, (struct sigvec *)0);
	sigvec(SIGINT, &sig_vec, (struct sigvec *)0);
	sigvec(SIGQUIT, &sig_vec, (struct sigvec *)0);
	sigvec(SIGILL, &sig_vec, (struct sigvec *)0);
	sigvec(SIGBUS, &sig_vec, (struct sigvec *)0);
	sigvec(SIGSEGV, &sig_vec, (struct sigvec *)0);
	sigvec(SIGSYS, &sig_vec, (struct sigvec *)0);
#endif /* mips && SYSTYPE_SYSV */

	/*
	 * now peruse through the list of files.
	 */
	for(fp=filelist; *fp; fp++) {
		filecontent = getfile(*fp);
		ip = newinclude(*fp, (char *)NULL);

		find_includes(filecontent, ip, ip, 0);
		freefile(filecontent);
		recursive_pr_include(ip, ip->i_file, basename(*fp));
		inc_clean();
	}
	if (printed)
		printf("\n");
	exit(0);
}

struct filepointer *getfile(file)
	char	*file;
{
	register int	fd;
	struct filepointer	*content;
	struct stat	st;

	content = (struct filepointer *)malloc(sizeof(struct filepointer));
	if ((fd = open(file, O_RDONLY)) < 0) {
		log("cannot open \"%s\"\n", file);
		content->f_p = content->f_base = content->f_end = malloc(1);
		*content->f_p = '\0';
		return(content);
	}
	fstat(fd, &st);
	content->f_len = st.st_size+1;
	content->f_base = malloc(content->f_len);
	if (content->f_base == NULL)
		log_fatal("cannot allocate mem\n");
	if (read(fd, content->f_base, st.st_size) != st.st_size)
		log_fatal("cannot read all of %s\n", file);
	close(fd);
	content->f_p = content->f_base;
	content->f_end = content->f_base + st.st_size;
	*content->f_end = '\0';
	content->f_line = 0;
	return(content);
}

freefile(fp)
	struct filepointer	*fp;
{
	free(fp->f_base);
	free(fp);
}

/*VARARGS*/
log_fatal(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
{
	log(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
	exit (1);
}

/*VARARGS0*/
log(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
{
	fprintf(stderr, x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);
}

char *copy(str)
	register char	*str;
{
	register char	*p = malloc(strlen(str) + 1);

	strcpy(p, str);
	return(p);
}

match(str, list)
	register char	*str, **list;
{
	register int	i;

	for (i=0; *list; i++, list++)
		if (strcmp(str, *list) == 0)
			return(i);
	return(-1);
}

/*
 * Get the next line.  We only return lines beginning with '#' since that
 * is all this program is ever interested in.
 */
char *getline(filep)
	register struct filepointer	*filep;
{
	register char	*p,	/* walking pointer */
			*eof,	/* end of file pointer */
			*bol;	/* beginning of line pointer */
	register	lineno;	/* line number */

	p = filep->f_p;
	eof = filep->f_end;
	if (p >= eof)
		return((char *)NULL);
	lineno = filep->f_line;

	for(bol = p--; ++p < eof; ) {
		if (*p == '/' && *(p+1) == '*') { /* consume comments */
			*p++ = ' ', *p++ = ' ';
			while (*p) {
				if (*p == '*' && *(p+1) == '/') {
					*p++ = ' ', *p = ' ';
					break;
				}
				else if (*p == '\n')
					lineno++;
				*p++ = ' ';
			}
			continue;
		}
		else if (*p == '\n') {
			lineno++;
			if (*bol == '#') {
				*p++ = '\0';
				goto done;
			}
			bol = p+1;
		}
	}
	if (*bol != '#')
		bol = NULL;
done:
	filep->f_p = p;
	filep->f_line = lineno;
	return(bol);
}

char *basename(file)
	register char	*file;
{
	register char	*p;

	for (p=file+strlen(file); p>file && *p != '/'; p--) ;

	if (*p == '/')
		p++;

	file = copy(p);
	for(p=file+strlen(file); p>file && *p != '.'; p--) ;

	if (*p == '.')
		*p = '\0';
	return(file);
}

redirect(line, makefile)
	char	*line,
		*makefile;
{
	struct stat	st;
	FILE	*fdin, *fdout;
	char	backup[ BUFSIZ ],
		buf[ BUFSIZ ];
	boolean	found = FALSE;
	int	len;

	/*
	 * if makefile is "-" then let it pour onto stdout.
	 */
	if (makefile && *makefile == '-' && *(makefile+1) == '\0')
		return;

	/*
	 * use a default makefile is not specified.
	 */
	if (!makefile) {
		if (stat("makefile", &st) == 0)
			makefile = "makefile";
		else if (stat("Makefile", &st) == 0)
			makefile = "Makefile";
		else
			log_fatal("[mM]akefile is not present\n");
	}
	else
	    stat(makefile, &st);
	if ((fdin = fopen(makefile, "r")) == NULL)
		log_fatal("cannot open \"%s\"\n", makefile);
	sprintf(backup, "%s.bak", makefile);
	unlink(backup);
	if (rename(makefile, backup) < 0)
		log_fatal("cannot rename %s to %s\n", makefile, backup);
	if ((fdout = freopen(makefile, "w", stdout)) == NULL)
		log_fatal("cannot open \"%s\"\n", backup);
	len = strlen(line);
	while (fgets(buf, BUFSIZ, fdin) && !found) {
		if (*buf == '#' && strncmp(line, buf, len) == 0)
			found = TRUE;
		fputs(buf, fdout);
	}
	if (!found) {
		log("Adding new delimiting line \"%s\"\nAdding dependencies...\n",
			line);
		puts(line); /* same as fputs(fdout); but with newline */
	}
	fflush(fdout);
#if defined (mips) && defined (SYSTYPE_SYSV)
	chmod(makefile, st.st_mode);
#else /* not (mips && SYSTYPE_SYSV) */
        fchmod(fileno(fdout), st.st_mode);
#endif /* mips && SYSTYPE_SYSV */
}
