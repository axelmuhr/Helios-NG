/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   H E L P   S Y S T E M                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- dbprint.c:          							--
--		Dump a help database in TeX format          		--
--									--
--	Author:  MJT 28/07/92    					--
--                                                                      --
------------------------------------------------------------------------*/
#ifdef __TRAN
static char *rcsid = "$Id: dbprint.c,v 1.2 1994/03/14 16:44:39 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <string.h>

#ifdef __HELIOS
#include <codes.h>
#include <attrib.h>
#include <syslib.h>
#include <nonansi.h>
#include <ctype.h>
#endif

#include "db.h"

        /* special character markers for database text file */

#define NEWDOC		(unsigned int) 0xf0
#define TITLE		(unsigned int) 0xf1
#define PARAGRAPH	(unsigned int) 0xf2
#define         P_PURPOSE       (unsigned int)0x01
#define         P_PARAGRAPH     (unsigned int)0x02
#define         P_RETURNS       (unsigned int)0x03
#define         P_INCLUDE       (unsigned int)0x04
#define         P_FORMAT        (unsigned int)0x05
#define         P_ERROR         (unsigned int)0x06
#define         P_DESCRIPTION   (unsigned int)0x07
#define         P_ARGUMENT      (unsigned int)0x08
#define XREF		(unsigned int) 0xf3
#define KWORD		(unsigned int) 0xf4
#define QWORD		(unsigned int) 0xf5
#define VERBATIM	(unsigned int) 0xf6
#define LEFT		(unsigned int) 0xf7
#define RIGHT		(unsigned int) 0xf8
#define TABLE		(unsigned int) 0xf9
#define THREECOL	(unsigned int) 0xfa
#define SUBJECTBREAK	(unsigned int) 0xfb
#define FORMATCHAR	(unsigned int) 0xfc
#define HARD_SPACE	(unsigned int) 0xfd
#define LIST_ITEM	(unsigned int) 0xfe
#define NEWPAGE		(unsigned int) 0xe1

#define ENDLINE		(unsigned int) 0xff
#define SPACE		' '
#define TAB		'\t'

        /* magic number at start of subject list */

#define SUBJECTMAGIC	0x18273645

#ifdef __HELIOS
#define DBPATH		"/helios/lib"
#define OPEN_R          "rb"
#else
#define DBPATH		"/usr/local/lib"
#define OPEN_R          "r"
#endif

        /* structure at end of text file for subject list */

static struct subject_header {
        long subject_magic;     /* magic number */
        long subject_pointer;   /* pointer to start of list */
        int subject_count;      /* number in list */
} subject_header;


static FILE *fdtxt;			/* stream for database text file */
static char textfile[_POSIX_PATH_MAX]; 	/* name of database text file(.txt)*/
static int dbnumber;			/* pointer to '?' in above names */
static int got_a_doc = 0;
static int verbatim = 0;
static int left = 0;
static int xref = 0;
static int three_col = 0;
static int formatchar = 0;
static int italic = 0;
static long docnum = 0;
static long onedoc = -1;
static long botdoc = 0;
static long topdoc = 999999;
static char *width1 = "3.0cm";
static char *width2 = "3.0cm";
static char *width3 = "4.2cm";
static char *width4 = "3.0cm";
static char *width5 = "7.2cm";

#ifdef __STDC__
#define fn(a, b) a b
#else
#define fn(a, b) a()
#endif

	/* function prototypes */

static u_long 	fn( swap, (u_long s));
static void 	fn( clean_up, (void));
static void 	fn( usage, (void));
static void 	fn( print_string, (u_char *p));
static void 	fn( print_char, (u_char c));

int main(argc, argv)
int argc;
char **argv;
{
	char *dbpath;			/* path to database files */
	int db = 0;			/* database number */
	int c;
	int i;
	char *a;
	u_char tbuff[512], *t;
	int tt = 0;

                /* if the user has the path in the environment */
                /* get it from there. Otherwise use default    */
 
        if((dbpath = getenv("HELP_DBPATH")) == (char *)NULL)
                dbpath = DBPATH;
 
	strcpy(textfile, dbpath);
	strcat(textfile, "/help?");

	dbnumber = strlen(textfile) - 1;

	strcat(textfile, ".txt");

		/* widths can also be obtained from the environment */

	if((a = getenv("TWO_COL_WIDTH")) != (char *)NULL)
		{
		width4 = strtok(a, ", ");
		width5 = strtok(NULL, ", ");
		}

	if((a = getenv("THREE_COL_WIDTH")) != (char *)NULL)
		{
		width1 = strtok(a, ", ");
		width2 = strtok(NULL, ", ");
		width3 = strtok(NULL, ", ");
		}
	fprintf(stderr, "dbprinting (%s %s) (%s %s %s)\n",
			width4, width5, width1, width2, width3);
	fflush(stdout);

			/* process arguments */

	switch(argc)
		{
		case 1:
			db = 0;
			botdoc = 0;
			topdoc = 99999;
			break;

		case 3:
			a = argv[2];

			i = sscanf(a, "%ld-%ld", &botdoc, &topdoc);
			switch(i)
				{
				case 0:
                			fprintf(stderr, "invalid document\n");
					usage();
			
				case 1:
					onedoc = botdoc;
					botdoc = topdoc = -1;
					break;
				}

		case 2:
        		db = argv[1][0] - '0';
        		if(db < 0 || db > 9 || argv[1][1])
				{
                		fprintf(stderr, "invalid database number\n");
				usage();
				}
			break;

		default:
			usage();
		}

	if(topdoc < botdoc && onedoc < 0)
		exit(0);

        textfile[dbnumber] = db + '0';

	fdtxt = fopen(textfile, OPEN_R);	/* open .txt file */
	if(fdtxt == (FILE *)NULL)
		{
		fprintf(stderr,"failed to open textfile (check HELP_DBPATH)\n");
		exit(-1);
		}

        if(fseek(fdtxt, -sizeof(subject_header), 2))
                {
		fprintf(stderr, "corrupt text file\n");
		exit(-1);
		}

        fread(&subject_header, sizeof(subject_header), 1, fdtxt);
        if(swap(subject_header.subject_magic) != SUBJECTMAGIC)
                {
		fprintf(stderr, "corrupt text file\n");
		exit(-1);
		}

                /* seek to start of subject list */

        subject_header.subject_pointer = swap(subject_header.subject_pointer);
	
	fseek(fdtxt, 0, 0);


	printf("\\documentstyle[a5,times,fancyheadings]{psl}\n");
	printf("\\pagestyle{fancy}\n");
	printf("\\lhead{}\n\\setlength{\\headrulewidth}{0pt}\n");
	printf("\\begin{document}\n");

		/* and here we go .... */

	while(ftell(fdtxt) != subject_header.subject_pointer)
		switch(c = getc(fdtxt))
			{
			case -1:
				clean_up();

			case NEWPAGE:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				xref = 0;
				three_col = 0;
				left = 0;
				printf("\\newpage\n");
				break;

			case NEWDOC:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				left = 0;
				three_col = 0;
				xref = 0;
				for(i = 0 ; i < sizeof(long); i++)
					c = getc(fdtxt);
				for(docnum = 0, i = 0; i < 6; i++)
					docnum = docnum * 10 + ((long)getc(fdtxt)-'0');
				if((onedoc > 0 && got_a_doc) ||
					(botdoc > 0 && docnum > topdoc))
					    clean_up();

				if(((onedoc != docnum) &&
					(docnum < botdoc || docnum > topdoc)) ||
					(docnum < 3 && db == 0))
					while(1)
						{
						int d = getc(fdtxt);
						if(d == -1)
							clean_up();

						if(d == NEWDOC)
							{
							fseek(fdtxt, -1, 1);
							break;
							}
						}
				else
					{
					printf("\\newpage\n");
					printf("\\setcounter{page}{1}\n");
					printf("\\pagenumbering{arabic}\n");
					got_a_doc = 1;
					}
				break;

			case TITLE:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				left = 0;
				three_col = 0;
				t = tbuff;
				xref = 0;
				while((c = getc(fdtxt)) != ENDLINE)
					{
					if(c == HARD_SPACE)
						c = ' ';
					if(c == '\\')
						tt = t-tbuff;
					*t++ = c;
					}
				*t = 0;
				tbuff[tt] = 0;
				printf("\\cfoot{");
				print_string(tbuff);
				printf(" - \\thepage}\n");
				printf("\\begin{center}\n\n{{\\LARGE \\bf ");
				print_string(tbuff);
				printf("}}\n\\vspace{0.5cm}\n\n");
				printf("{\\large ");
				print_string(&tbuff[tt+1]);
				printf(" }\n\\vspace{0.25cm}");
				printf("\n\n\\end{center}\n");
				break;

			case PARAGRAPH:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				printf("\\vspace{0.25cm}\n");
				left = 0;
				three_col = 0;
				xref = 0;
				printf("\n\\noindent {\\bf");
				switch(c = getc(fdtxt))
					{
					case P_PURPOSE:
						printf(" Purpose:} ");
						break;

					case P_PARAGRAPH:
						printf("} ");
						break;

					case P_RETURNS:
						printf(" Returns:} ");
						break;

					case P_INCLUDE:
						printf(" Include:} ");
						printf("{\\it ");
						italic = 1;
						break;

					case P_FORMAT:
						printf(" Format:} ");
						printf("{\\it ");
						italic = 1;
						break;

					case P_ERROR:
						printf(" Error:} ");
						break;

					case P_DESCRIPTION:
						printf(" Description:} ");
						break;

					case P_ARGUMENT:
						printf(" Argument:} ");
						break;

					}
				break;

			case XREF:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				left = 0;
				if(!xref)
					{
					printf("\\vspace{0.5cm}\n");
					printf("\n\\noindent {\\bf See Also:} ");
					}
				else
					print_string((u_char *)", ");
				xref = 1;
				three_col = 0;
				break;

			case KWORD:
			case QWORD:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				left = 0;
				while(getc(fdtxt) != ENDLINE);
				xref = 0;
				three_col = 0;
				break;

			case VERBATIM:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(left || three_col)
					printf("\\end{tabular}\n");
				left = 0;
				three_col = 0;
				xref = 0;
				if(!verbatim)
					{
					printf("\n\\vspace{0.25cm}\n");
					printf("\\footnotesize\n");
					printf("\\begin{verbatim}\n");
					verbatim = 1;
					}
				printf("    ");
				break;

			case LEFT:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(three_col || left)
					{
					printf("\\end{tabular}\n");
					left = 0;
					}
				if(!left)
					{
					printf("\n\\vspace{0.25cm}\n");
					printf("\\begin{tabular}");
					printf("{p{%s}p{%s}}\n", width4,width5);
					}
				if(left == 2)
					printf("\\\\\n\\\\\n");
				left = 1;
				xref = 0;
				three_col = 0;
				break;

			case RIGHT:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				if(three_col)
					printf("\\end{tabular}\n");
				if(!left)
					{
					printf("\n\\vspace{0.25cm}\n");
					printf("\\begin{tabular}");
					printf("{p{%s}p{%s}}\n", width4,width5);
					}
				if(left == 2)
					printf("\\\\\n\\\\\n");
				left = 2;
				printf("& ");
				xref = 0;
				three_col = 0;
				break;

			case TABLE:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}
				printf("\n\\vspace{0.25cm}\n");
				if(left || three_col)
					printf("\\\\&");
				printf("\\hspace{1cm} ");
				xref = 0;
				break;

			case THREECOL:
				if(italic)
					{
					printf("}\n");
					italic = 0;
					}
				if(verbatim)
					{
					printf("\\end{verbatim}");
					printf("\n\\normalsize\n");
					verbatim = 0;
					}

				if(left || three_col)
					printf("\\end{tabular}\n");
				printf("\n\\vspace{0.125cm}\n");
				printf("\\begin{tabular}");
				printf("{p{%s}p{%s}p{%s}}\n",
					width1, width2, width3);
				three_col = 1;
				left = 0;
				xref = 0;
				break;

			case ENDLINE:
				formatchar = 0;
				if(!left)
					print_char('\n');
				else
					putchar(' ');
				break;

			case HARD_SPACE:
				print_char(' ');
				break;

			case FORMATCHAR:
				formatchar = !formatchar;
				c = getc(fdtxt);
				if(formatchar)
					switch(c)
						{
						case 's':
							printf("{\\em ");
							break;
	
						case 'b':
							printf("{\\bf ");
							break;
						}
				else
					putchar('}');

				break;

			case '%':
				if(!verbatim)
					print_char(c);
			default:
				if(xref && c == '\\')
					{
					while(getc(fdtxt) != ENDLINE);
					print_char('\n');
					break;
					}
				print_char(c);
				break;
			}

	clean_up();
}

static u_long swap(s)
u_long s;
{
	if(BYTE_ORDER != LITTLE_ENDIAN)
		BLSWAP(s);

	return s;
}

static void clean_up()
{
	printf("\\end{document}\n");
	fclose(fdtxt);
	exit(0);
}

static void print_string(p)
u_char *p;
{
	while(*p)
		print_char(*p++);
}

#ifdef __STDC__
static void print_char(u_char c)
#else
static void print_char(c)
u_char(c);
#endif
{
	char *backchars = "#$%&_{}";
	char *starchars = "<>[]";

	if(!verbatim)
		{
		if(c == '\\' && three_col)
			{
			putchar('&');
			return;
			}
		if(c == '\\')
			{
			printf(" \\verb+\\+");
			return;
			}
		if(c == '^')
			{
			printf("\\verb+^+");
			return;
			}
		if(strchr(starchars, (char)c) != (char *)NULL)
			{
			printf("$%c$", c);
			return;
			}
		if(strchr(backchars, (char)c) != (char *)NULL)
			putchar('\\');
		}
	putchar(c);
}

static void usage()
{

	fprintf(stderr, "\nUsage -\n\tdbprint [dbnum | dbnum range]\n");
	fprintf(stderr, "\te.g.\tdbprint          - print all docs in db 0\n");
	fprintf(stderr, "\t\tdbprint 1        - print all docs in db 1\n");
	fprintf(stderr, "\t\tdbprint 0 56     - print doc #56 in db 0\n");
	fprintf(stderr, "\t\tdbprint 2 23-107 - print docs #23 through");
	fprintf(stderr, " #107 in db 2\n");
	fflush(stderr);
	exit(-1);
}
