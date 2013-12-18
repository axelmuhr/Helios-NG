/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   H E L P   S Y S T E M                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- dbdump.c:          							--
--		Dump a help database                        		--
--									--
--	Author:  MJT 28/07/92    					--
--                                                                      --
------------------------------------------------------------------------*/

#ifdef __TRAN
static char *rcsid = "$Id: dbdump.c,v 1.2 1994/05/12 13:46:06 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

#ifdef __HELIOS
#include <codes.h>
#include <attrib.h>
#include <syslib.h>
#include <nonansi.h>
#include <ctype.h>
#include <string.h>
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

#ifdef __STDC__
#define fn(a, b) a b
#else
#define fn(a, b) a()
#endif

	/* function prototypes */

static u_long 	fn( swap, (u_long s));

int main(argc, argv)
int argc;
char **argv;
{
	char *dbpath;			/* path to database files */
	int db;				/* database number */
	int c;
	int i;
	int cnt;
	int verbatim = 0;

			/* process arguments */

                /* if the user has the path in the environment */
                /* get it from there. Otherwise use default    */
 
        if((dbpath = getenv("HELP_DBPATH")) == (char *)NULL)
                dbpath = DBPATH;
 
	strcpy(textfile, dbpath);
	strcat(textfile, "/help?");

	dbnumber = strlen(textfile) - 1;

	strcat(textfile, ".txt");

        if(argc == 1)
		db = 0;
	else
		{
        	db = argv[1][0] - '0';
        	if(db < 0 || db > 9)
			{
                	fprintf(stderr, "invalid database number");
			exit(-1);
			}
		}
                   
        textfile[dbnumber] = db + '0';

	fdtxt = fopen(textfile, OPEN_R);	/* open .txt file */
	if(fdtxt == (FILE *)NULL)
		{
		fprintf(stderr, "failed to open textfile (check HELP_DBPATH)");
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

		/* and here we go .... */

	for(cnt = 0; cnt < subject_header.subject_pointer; cnt++)
		switch(c = getc(fdtxt))
			{
			case NEWPAGE:
				printf("*N\n");
				break;

			case NEWDOC:
				printf("#");
				for(i = 0 ; i < sizeof(long); i++)
					{cnt++; c = getc(fdtxt);}
				for(i = 0 ; i < 6; i++)
					{cnt++; putchar(getc(fdtxt));}
				putchar('\n');
				break;

			case TITLE:
				printf("*n ");
				break;

			case PARAGRAPH:
				putchar('*');
				cnt++;
				switch(getc(fdtxt))
					{
					case P_PURPOSE:
						putchar('P');
						break;

					case P_PARAGRAPH:
						putchar('p');
						break;

					case P_RETURNS:
						putchar('R');
						break;

					case P_INCLUDE:
						putchar('I');
						break;

					case P_FORMAT:
						putchar('F');
						break;

					case P_ERROR:
						putchar('E');
						break;

					case P_DESCRIPTION:
						putchar('D');
						break;

					case P_ARGUMENT:
						putchar('A');
						break;

					}
				putchar(' ');
				break;

			case XREF:
				printf("*x ");
				break;

			case KWORD:
				printf("*k ");
				break;

			case QWORD:
				printf("*q ");
				break;

			case VERBATIM:
				verbatim = 1;
				printf("*c=");
				break;

			case LEFT:
				printf("*l ");
				break;

			case RIGHT:
				printf("*t ");
				break;

			case TABLE:
				printf("*d ");
				break;

			case THREECOL:
				printf("*f ");
				break;

			case FORMATCHAR:
				printf("%%");
				break;

			case HARD_SPACE:
				printf(" ");
				break;

			case LIST_ITEM:
				printf("*L ");
				for(i = 0 ; i < 7; i++)
					{cnt++; c = getc(fdtxt);}
				break;

			case ENDLINE:
				printf("\n");
				verbatim = 0;
				break;

			case '%':
				if(!verbatim)
					putchar(c);
			default:
				putchar(c);
				break;
			}

	fclose(fdtxt);
}

static u_long swap(s)
u_long s;
{
	if(BYTE_ORDER != LITTLE_ENDIAN)
		BLSWAP(s);

	return s;
}

