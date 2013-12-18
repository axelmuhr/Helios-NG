/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   H E L P   S Y S T E M                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- dbbuild.c:       							--
--		Build a help database from a given input file		--
--									--
--	Author:  MJT 28/07/92    					--
--                                                                      --
--                                                                      --
-- From the input data (currently MUSCAT format), find all words that   --
-- need to be indexed and store them in a btree. Each word is stored    --
-- together with a pointers to the document(s) in which it appears.     --
-- Words are weighted according to the paragraph in which they appear;  --
-- duplicate words in a document (of a lower weight) are not stored.    --
--                                                                      --
-- At the same time, store the document text in the database text file, --
-- processing newlines, formatting characters, etc. appropriately. The  --
-- text file also contains a list of the valid subject types for this   --
-- database.                                                            --
--                                                                      --
-- Thus :                                                               --
--                                                                      --
--                                                                      --
--    ---------                        ---------------------            --
--    |       | ---------------------> | Dictionary file   |            --
--    | input |           |            | (btree with words |            --
--    |  data |           |            | + doc pointers)   |            --
--    |       |           |            ---------------------            --
--    |       |           |                                             --
--    ---------           |            ---------------------            --
--                        |            | Text file (data + |            --
--                        -----------> | subject list).    |            --
--                                     |                   |            --
--                                     ---------------------            --
--                                                                      --
------------------------------------------------------------------------*/

#ifdef __TRAN
static char *rcsid = "$Id: dbbuild.c,v 1.4 1994/03/08 13:52:52 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <signal.h>
#include <limits.h>

#ifdef __HELIOS
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#endif

#include "db.h"
#include "btree.h"

	/* general purpose error exit routine */

#define fatal(x) 	{ err(x, 1) ; clean_up(); }

	/* weighting for indexed terms (higher number = greater weight) */

#define TOPIC		4
#define SUBJECT		3
#define PURPOSE		2
#define KEYWORD		1
#define NOINDEX		0

	/* special character markers for database text file */

#define NEWPAGE		(unsigned int)0xe1
#define NEWDOC		(unsigned int)0xf0
#define TITLE		(unsigned int)0xf1
#define PARAGRAPH	(unsigned int)0xf2
#define		P_PURPOSE	(unsigned int)0x01
#define		P_PARAGRAPH	(unsigned int)0x02
#define		P_RETURNS	(unsigned int)0x03
#define		P_INCLUDE	(unsigned int)0x04
#define		P_FORMAT	(unsigned int)0x05
#define		P_ERROR		(unsigned int)0x06
#define		P_DESCRIPTION	(unsigned int)0x07
#define		P_ARGUMENT	(unsigned int)0x08
#define XREF		(unsigned int)0xf3
#define KWORD		(unsigned int)0xf4
#define QWORD		(unsigned int)0xf5
#define VERBATIM	(unsigned int)0xf6
#define LEFT		(unsigned int)0xf7
#define RIGHT		(unsigned int)0xf8
#define TABLE		(unsigned int)0xf9
#define THREECOL	(unsigned int)0xfa
#define SUBJECTBREAK	(unsigned int)0xfb
#define FORMATCHAR	(unsigned int)0xfc
#define HARD_SPACE	(unsigned int)0xfd
#define LIST_ITEM	(unsigned int)0xfe

#define ENDLINE		(unsigned int)0xff

	/* pathname and open modes of database files */

#ifdef __HELIOS
#define DBPATH		"/helios/lib"
#define OPEN_R		"rb"
#define OPEN_W		"wb"
#else
#define DBPATH		"/usr/local/lib"
#define OPEN_R		"r"
#define OPEN_W		"w"
#endif

	/* magic number at start of subject list */

#define SUBJECTMAGIC	0x18273645

	/* Maximum number of subjects the database can deal with */

#define SUBMAX		100

	/* Maximum length of indexed words */

#define WORDMAX		63

	/* Maximum number of stopwords */

#define STOPMAX		128

	/* Maximum length of input line */

#define BUFFMAX		512

	/* print format strings for docs processed message */

#define DOCSDONE	"\r%d documents processed"
#define DOCDONE		"1 document processed"

	/* structure for holding words as they are processed */

typedef struct indexword {
	u_char word[WORDMAX+1];	/* word */	
	long docnum;			/* position of document in text file */
	int  weight;			/* weight of search term */
	};

	/* structure written at end of text file for subject list */

struct subject_header {
	long subject_magic;	/* magic number */
	long subject_pointer;	/* pointer to start of list */
	int subject_count;	/* number in list */
} subject_header;

static FILE *fdin = (FILE *)NULL;	/* stream for input data file */
static FILE *fdtxt = (FILE *)NULL;	/* stream for database text file */
static char *ifnm;			/* name of input data file */
static char ofnm[_POSIX_PATH_MAX];	/* name of database */
static char textfile[_POSIX_PATH_MAX];	/* name of database text file (.txt) */
static char dictfile[_POSIX_PATH_MAX];	/* name of database dict file (.dct) */
static int lineno = 1;			/* current line in input data */
static int docnum = 0;			/* current document being processed */
static long curpos;			/* position of doc start in text file */
static DB *ttt = (DB *)NULL;		/* ptr to access routines for B-tree */
static u_char *subject[SUBMAX];		/* storage for subjects */
static int subcnt;			/* counter for above */
static int verbatim = FALSE;		/* verbatim flag */
static char *stoplist[STOPMAX];		/* stopword list */
static int stopcnt;			/* counter for above */
static int signal_caught = FALSE;	/* if we get interrupted */
static int we_have_started = FALSE;	/* if we have done some work */
static int no_warn = FALSE;		/* don't warn user */
static int wordlist = FALSE;		/* produce wordlist */
static int format_count = 0;		/* check format string match */

#ifdef __STDC__
#define fn(a, b) a b
#else
#define fn(a, b) a()
#endif

static int	fn( check_dups, (u_char *cur_data, struct indexword *new_data));
static void	fn( insert_btree, ( struct indexword *bb));
static void	fn( err, (char *p, int action));
static void	fn( usage, (void));
static int	fn( getline, (u_char *buff));
static void	fn( putword, (u_char *p));
static void	fn( putline, (int indexflag, u_char *p));
static void	fn( add_subject, (u_char *w));
static int	fn( isvalid, (u_char c));
static int	fn( compare_string, (u_char *s1, u_char *s2));
static void	fn( read_stoplist, (void));
static int	fn( in_stoplist, (char *wd));
static void	fn( word_to_lower, (u_char *p));
static u_long	fn( swap, (u_long s));
static void	fn( clean_up, (void));
static void	fn( initialise, (void));
static void	fn( catch_signal, (int sig));
static void	fn( check_title, (u_char *p));
static int	fn( check_three_col, (u_char *p));
extern u_char *	fn( stem, (u_char *wordptr));

int main(argc, argv)
int argc;
char **argv;
{
	u_char buff[BUFFMAX],c, *p, tc;
	int i, db, dbnumber;
	char *dbpath;

	int justgotdoc = FALSE;
	int had_a_title = FALSE;

	BTREEINFO b;

	long doclen, tmptell, number;

		/* simple arg processing - dbbuild [-w|-W] input dbnum */

	if(argc < 3)
		usage();

	if(argc == 4)
		{
		if(!strcmp("-W", argv[1]))	/* create wordlist */
			{
			no_warn = TRUE;
			wordlist = TRUE;
			argv++;
			}
		else
		    if(!strcmp("-w", argv[1]))	/* no warning messages */
			    {
			    no_warn = TRUE;
			    argv++;
			    }
		    else
			    usage();
		}
		
	subcnt = 0;

	ifnm = argv[1];

		/* set up database names */

        if((dbpath = getenv("HELP_DBPATH")) == (char *)NULL)
                dbpath = DBPATH;

        strcpy(textfile, dbpath);
        strcat(textfile, "/help?");

        dbnumber = strlen(textfile) - 1;
        strcpy(dictfile, textfile);

        strcat(textfile, ".txt");
        strcat(dictfile, ".dct");

	db = argv[2][0] - '0';
	if(db < 0 || db > 9 || argv[2][1])
		fatal("invalid database number")

	textfile[dbnumber] = db + '0';
	dictfile[dbnumber] = db + '0';

	strcpy(ofnm, textfile);
	ofnm[dbnumber+1] = 0;

	if((fdtxt = fopen(textfile, OPEN_R)) != (FILE *)NULL)	/* exists */
		fatal("database exists")

	if((fdin = fopen(dictfile, OPEN_R)) != (FILE *)NULL)	/* exists */
		fatal("database exists")

	if((fdin = fopen(argv[1], OPEN_R)) == (FILE *)NULL) /* doesn't exist */
		fatal("failed to open input file")

	if(getc(fdin) != '#')		/* must be start of file */
		fatal("No document marker")

	fseek(fdin, 0, 0);		/* back to start */
	
	if((fdtxt = fopen(textfile, OPEN_W)) == (FILE *)NULL) /* can't create */
		fatal("failed to create text file")

	b.psize = 4096;
	b.cachesize = 0;
	b.lorder = LITTLE_ENDIAN;	/* force db to be little-endian */
	b.flags = 0;			/* No dups */
	b.compare = compare_string;
	
	if((ttt = btree_open(dictfile, O_CREAT|O_TRUNC|O_RDWR, 0666, &b)) ==
			(DB *) NULL)
		fatal("failed to create dict file")

	read_stoplist();		/* read stopword list (if one exists) */

	initialise();			/* signals */

	while(getline(buff))
		{
		if(signal_caught)	/* safe to exit at this point */
			clean_up();

		c = buff[0];
		switch(c)
			{
			case '\0':		/* blank */
				break;

			case '#':		/* start of doc, may have */
						/* document number        */
				if(format_count)
					err("unmatched format string",0);

				format_count = 0;

				if(justgotdoc)
					break;

					/* if we have written a document, */
					/* record its length		  */

				if(docnum)
					{
					tmptell = ftell(fdtxt);
					doclen = swap(tmptell-curpos-6);
					fseek(fdtxt, curpos-sizeof(long), 0);
					fwrite(&doclen, sizeof(long), 1, fdtxt);
					fseek(fdtxt, tmptell, 0);
					if(!had_a_title && docnum > 2)
						fatal("no title")
					}

				docnum++;
				putc(NEWDOC, fdtxt);

				if((docnum % 10) == 0)
					{
					fprintf(stderr, DOCSDONE, docnum);
					fflush(stderr);
					}

					/* leave space for document size */

				fwrite("\0\0\0\0", sizeof(long), 1, fdtxt);
				curpos = ftell(fdtxt);

					/* and put document number */

				number = -1;
				for(p = &buff[1]; *p && *p == ' '; p++);
				if(*p)		/* given doc num */
					if(sscanf((char *)p, "%d",
							&number) != 1)
						err("invalid document number",0);

				if(number < 0)	/* create unique num */
					number = (db * 100000L) + docnum;
				if(number > 999999)
					fatal("document number out of range")

				fprintf(fdtxt, "%06lu", number);
				justgotdoc = TRUE;
				had_a_title = FALSE;
				break;

			case '*':	/* a special para marker */
				justgotdoc = FALSE;
				tc = 1;
				switch(buff[1])
					{
					case 'n':	/* title */
						if(had_a_title)
						      fatal("duplicate title")
						had_a_title = TRUE;
						check_title(&buff[2]);
						putc(TITLE,fdtxt);
						putline(SUBJECT, &buff[2]);
						break;

					case 'L':	/* list item */
						putc(LIST_ITEM, fdtxt);
						fprintf(fdtxt, "000000\001");
						for(p = &buff[2]; *p; p++)
						   if(*p == ' ')
							*p = HARD_SPACE;
						putline(NOINDEX, &buff[2]);
						break;

					case 'P':	/* purpose */
						putc(PARAGRAPH, fdtxt);
						putc(P_PURPOSE, fdtxt);
						putline(PURPOSE, &buff[2]);
						break;

					case 'A': tc++;		/* Argument */
					case 'D': tc++;		/* Description*/
					case 'E': tc++;		/* Error */
					case 'F': tc++;		/* Format */
					case 'I': tc++;		/* Include */
					case 'R': tc++;		/* Returns */
					case 'p': tc++;		/* normal */
						putc(PARAGRAPH, fdtxt);
						putc(tc, fdtxt);
						putline(NOINDEX, &buff[2]);
						break;

					case 'N':		/* new page */
						putc(NEWPAGE, fdtxt);
						break;

					case 'x':		/* x-ref */	
						putc(XREF, fdtxt);
						putword(&buff[2]);
						break;

					case 'k':		/* keyword */
						putc(KWORD, fdtxt);
						putline(KEYWORD, &buff[2]);
						break;
						
					case 'q':		/* index */
						putc(QWORD, fdtxt);
						putline(KEYWORD, &buff[2]);
						break;
						
					case 'c':		/* verbatim */
						if(buff[2] != '=')
							{
							err("invalid format",0);
							break;
							}
						putc(VERBATIM, fdtxt);
						putline(NOINDEX, &buff[3]);
						verbatim = FALSE;
						break;
						
					case 'l':		/* left */
						putc(LEFT, fdtxt);
						putline(NOINDEX, &buff[2]);
						break;
						
					case 't':		/* right */	
						putc(RIGHT, fdtxt);
						putline(NOINDEX, &buff[2]);
						break;

					case 'd':		/* table */	
						putc(TABLE, fdtxt);
						putline(NOINDEX, &buff[2]);
						break;
						
					case 'f':		/* three-col */
						putc(THREECOL, fdtxt);
						putline(NOINDEX, &buff[2]);
						if(check_three_col(&buff[2]))
						   err("column missing", 0);
						break;
						
					default:
						err("unknown control",0);
						putline(NOINDEX, buff);
					}
					break;

			default:
				justgotdoc = FALSE;
				putline(NOINDEX, buff);
				break;
						
			}
		lineno++;
		}

	/* record length of last document */

	tmptell = ftell(fdtxt);
	doclen = swap(tmptell-curpos);
	fseek(fdtxt, curpos-sizeof(long), 0);
	fwrite(&doclen, sizeof(long), 1, fdtxt);
	fseek(fdtxt, tmptell, 0);
	if(!had_a_title && docnum > 2)
		fatal("no title")

	/* finished massaging input data - now add subject list to end of */
	/* text file. Write out each string (with terminator), then string */
	/* count and pointer to start of list */

	subject_header.subject_pointer = swap(ftell(fdtxt));	/* file ptr */
	subject_header.subject_count   = (int) swap(subcnt);		/* count */
	subject_header.subject_magic   = swap(SUBJECTMAGIC);	/* magic */
	
	for(i = 0 ; i < subcnt; i++)
		{
		fwrite((void *)subject[i], (size_t)strlen((char *)subject[i])+1,
			(size_t)1, fdtxt);
		putc(SUBJECTBREAK, fdtxt);
		free(subject[i]);
		}

	i = (int) ftell(fdtxt);

	if(i%4)				/* not on word boundary */
		fwrite("   ", i%4, 1, fdtxt);
	
	fwrite(&subject_header, sizeof(subject_header), 1, fdtxt);

	fclose(fdin);		/* tidy up */
	fclose(fdtxt);

	(void) (*(ttt->close))(ttt->internal);		/*close btree */

	if(docnum > 1)
		fprintf(stderr, DOCSDONE, docnum);
	else
		fprintf(stderr, DOCDONE);
	putc('\n', stderr);
	fflush(stderr);

	exit(0);
}

	/* write a line to the database text file, indexing words as needed */

static void putline(indexflag, p)
int indexflag;
u_char *p;
{
	u_char *q, *r;
	struct indexword iw;
	u_char buff[2*BUFFMAX];

	bzero((char *)&iw, sizeof(struct indexword));

	if(!verbatim)
		while(*p && (isspace(*p) || *p == HARD_SPACE))
			p++;

	strcpy((char *)buff, (char *)p);
	if(indexflag == SUBJECT)	/* convert SPACE to HARD_SPACE */
		{
		for(q = buff ; *q && *q != '\\'; q++);
		for( ; *q ; q++)
			if(*q == ' ')
				*q = HARD_SPACE;

		for(r = q; *r == HARD_SPACE || !isvalid(*r); r--);

		*(r+1) = 0;
		}
	q = buff;

	fprintf(fdtxt, "%s", buff);
	putc(ENDLINE, fdtxt);

	if(indexflag != NOINDEX)
		while(*q)
			{
			int wordlen;

			for(p = q ; *p && !isvalid(*p); p++)
				{
				if(*p == '\\' && indexflag == SUBJECT)
					{
					for(q = p ; *q && !isvalid(*q); q++);
					wordlen = (strlen((char *)q) > WORDMAX)
						? WORDMAX : strlen((char *)q);
                			strncpy((char *)iw.word, (char *)q,
						 wordlen);
					iw.word[wordlen] = 0;
					iw.docnum = curpos;
					iw.weight = SUBJECT;
					add_subject(iw.word);
					word_to_lower(iw.word);
					q = stem(iw.word);
					strcpy((char *)iw.word, (char *)q);
					insert_btree((struct indexword *)&iw);
					return;
					}
				}
			if(!*p)
				return;
			for(r = p ; *r && (isvalid(*r) ||
				((indexflag == SUBJECT) &&
				 (*r == '.' || *r == '-'))); r++);
			wordlen = ((r-p) > WORDMAX) ?
					WORDMAX : (r-p);
                	strncpy((char *)iw.word, (char *)p, wordlen);
			iw.word[wordlen] = 0;
			iw.docnum = curpos;
			if(indexflag == SUBJECT)
				{
				iw.weight = TOPIC;
				insert_btree((struct indexword *)&iw);
				iw.weight = KEYWORD;
				}
			else
				iw.weight = indexflag;

			word_to_lower(iw.word);
			q = stem(iw.word);
			if(!in_stoplist((char *)q))
				{
				strcpy((char *)iw.word, (char *)q);
				insert_btree((struct indexword *)&iw);
				}
			q = r;
			}
}

static void putword(p)
u_char *p;
{
	u_char *q;

	while(*p && *p == ' ' || *p == '\t')
		p++;

	q = p;

		/* allow '.' and '-' (and special '\') into xrefs */

	while(*p && (isvalid(*p) || *p == '.' || *p == '-' || *p == '\\'))
		p++;
	*p = 0;

	fprintf(fdtxt, "%s", q);
	putc(ENDLINE, fdtxt);
}

static int getline(buff)
u_char *buff;
{
	u_char *p = buff;
	int c;
	int got_format = FALSE;

	while((c = getc(fdin)) != EOF)
		{
		if(p > buff + BUFFMAX -3)
			{
			err("line too long",0);
			*p++ = c;
			*p = 0;
			return 1;
			}

		switch(c)
			{
			case '\r':
				continue;

			case '\n':
				*p = 0;
				return 1;

			case '%':
				if(verbatim)
					{
					*p++ = (u_char)c;
					break;
					}
				if(got_format)
					{
					*p++ = '%';
					got_format = FALSE;
					}
				else
					got_format = TRUE;
				break;

			case '\\':
					/* lose any spaces */

				if(!verbatim)
					if(*(p+1) != '\\')
						{
						p--;
						while(p != buff && *p == ' ')
							p--;
						p++;
						}

				*p++ = (u_char) c;
				break;

			case ' ':
				if(verbatim || *(p-1) != '\\')
					*p++ = (u_char) c;
				break;

			case 'b':
			case 's':
				if(got_format)
					{
					*p++ = FORMATCHAR;
					got_format = FALSE;
					format_count = !format_count;
					}

			default:
				if((c < 32 || c > 126) && c != '\t')
					err("illegal character", 0);
				if(got_format)
					*p++ = '%';
				got_format = FALSE;
				*p++ = (u_char) c;

					/* check for verbatim */

				if(p-buff == 3)
					if(!strncmp((char *)buff, "*c=", 3))
						verbatim=TRUE;
			}

		}

	*p = 0;
	return 0;
}

static void err(p, action)
char *p;
int action;
{
	if(no_warn && !action)	/* user doesn't want to know */
		return;
	fprintf(stderr,
		"\nDBBUILD - %s%s at line %d (document number %d)\n",
		action ? "" : "(warning) ", p, lineno, docnum);
	if(action)
    	   fprintf(stderr, "          building %s -> %s\n", ifnm, ofnm);
	fflush(stderr);
}

static void usage()
{
	fprintf(stderr,
	    "DBBUILD : usage -\n\tdbbuild [-w] input_data_file db_number\n");
	fflush(stderr);
	exit(-1);
}

	/* insert a term into the B-tree. If the word exists, get its data */
	/* block, delete old version, update and store new version.        */

static void insert_btree(bb)
struct indexword *bb;
{
	DBT key;
	DBT data;
	int status, offset, newsize;
	int len = 1;
	long tdocnum;
	int tweight;
	u_char *tptr;

#define ADDROFF		(sizeof(int))
#define WEIGHTOFF	(sizeof(int)+sizeof(long))

			/* first lookup word in btree */
	
	key.data = bb->word;
	key.size = strlen((char *)bb->word) + 1;

	status = (*(ttt->get))(ttt->internal, &key, &data, 0);

	switch(status)
		{
		case RET_SUCCESS:		/* already entered */

				/* if duplicate for this doc - ignore */

			if(check_dups(data.data, bb))
				break;

				/* now we need to update the B-tree */
				/* firstly, save old data */

			newsize = data.size + WEIGHTOFF;
			tptr = (u_char *)malloc(newsize);
			if(tptr == (u_char *)NULL)
				fatal("malloc failure (replace)")

			bcopy((char *)data.data, (char *)tptr, data.size);
			bcopy((char *)data.data, (char *)&len, sizeof(int));
			len = (int) swap(len);

				/* now delete old record */

			status = (*(ttt->Delete))(ttt->internal, &key, 0);

			if(status == RET_ERROR)
				fatal("btree delete failure")

			offset = ADDROFF + len * WEIGHTOFF;
			len++;

				/* add new document details */

			len = (int) swap(len);
			bcopy((char *)&len, (char *)tptr, sizeof(int));
			tdocnum = swap(bb->docnum);
			bcopy((char *)&tdocnum, (char *)&tptr[offset],
					sizeof(long));
			tweight = (int) swap(bb->weight);
			bcopy((char *)&tweight,(char *)&tptr[offset+ADDROFF],
					sizeof(int));

			data.data = tptr;
			data.size = newsize;

				/* and put it back */

			status = (*(ttt->put))(ttt->internal,&key,&data, R_PUT);

			free(tptr);

			if(status == RET_ERROR)
				fatal("btree replace failure")

			if(wordlist)
				printf("%s (%ld)\n", key.data, swap(len));

			break;

		case RET_SPECIAL:		/* non-existent */
			data.size = 2*sizeof(int)+sizeof(long);
			data.data = (u_char *)malloc(data.size);
			if(data.data == (u_char *)NULL)
				fatal("malloc failure (put)")

				/* create initial record for B-tree */

			len = (int) swap(len);
			bcopy((char *)&len, (char *)data.data, sizeof(int));
			tdocnum = swap(bb->docnum);
			bcopy((char *)&tdocnum, (char *)&data.data[ADDROFF],
				 sizeof(long));
			tweight = (int) swap(bb->weight);
			bcopy((char *)&tweight,(char *)&data.data[WEIGHTOFF],
				 sizeof(int));

				/* and add the new record */

			status = (*(ttt->put))(ttt->internal, &key, &data, R_PUT);

			free(data.data);

			if(status == RET_ERROR)
				fatal("btree put failure")

			if(wordlist)
				printf("%s\n", key.data);

			break;

		case RET_ERROR:			/* oops ... */
			fatal("btree error")

		}
}

	/* check for multiple occurrences in a single document */

static int check_dups(cur_data, new_data)
u_char *cur_data;
struct indexword *new_data;
{
	int i,offset;
	long addr;
	int cnt;

	bcopy((char *)cur_data, (char *)&cnt, sizeof(int));
	cnt = (int) swap(cnt);

	for(i = 0 ; i < cnt; i++)
		{
		offset = ADDROFF + i*WEIGHTOFF;
		bcopy((char *)&cur_data[offset], (char *)&addr, sizeof(long));
		addr = swap(addr);
		if(addr == new_data->docnum)
			return(1);
		}
	return(0);
}

	/* record subject (used in table-of-contents) */

static void add_subject(w)
u_char *w;
{
	int i;
	u_char *p;

		/* check to see if we've had this one before */

	for(i = 0 ; i < subcnt; i++)
		if(!compare_string(w, subject[i]))
			return;

		/* no - add it to list */

	subject[subcnt] = (u_char *)malloc(strlen((char *)w) + 1);
	if(subject[subcnt] == (u_char *) NULL)
		fatal("malloc failure (sub)")
	strcpy((char *)subject[subcnt], (char *)w);

	for(p = subject[subcnt]; *p; p++)
		if(*p == ' ')
			*p = HARD_SPACE;

		/* if we overflow - throw away last one */

	if(++subcnt == SUBMAX)
		subcnt--;
}

#ifdef __STDC__
static int isvalid(u_char c)
#else
static int isvalid(c)
u_char c;
#endif
{
	return(c == '/' || c == '@' || c == '_' || isalnum(c));
}

static int compare_string(s1, s2)
u_char *s1, *s2;
{
	u_char c1,c2;

	while(1)
		{
		if ((c1 = *s1++) != (c2 = *s2++))
			return c1 - c2;
		if (c1 == 0)
			return 0;     /* no need to check c2 */
		}
}

	/* if we have a stopword list, then use it - these (truncated) */
	/* words will never be indexed */

static void read_stoplist()
{
	FILE *fdstop = fopen("stopword.lst", "r");
	char buff[WORDMAX+1];

	stopcnt = 0;

	if(fdstop == NULL)
		return;

	while(fgets(buff, WORDMAX, fdstop) != NULL)
		{
		buff[strlen(buff)-1] = 0;
		stoplist[stopcnt] = (char *) malloc(strlen(buff) + 1);
		if(stoplist[stopcnt] == (char *) NULL)
			break;

		strcpy(stoplist[stopcnt++], buff);
		}

	fclose(fdstop);
}

	/* check for (truncated) word in the stopword list */

static int in_stoplist(wd)
char *wd;
{
	int i, val;

	for(i = 0 ; i < stopcnt ; i++)
		{
		val = strcmp(stoplist[i], wd);
		if (! val)
			return TRUE;
		if (val > 0)
			break;
		}
	return FALSE;
}

static void word_to_lower(p)
u_char *p;
{
	for(; *p ; *p = tolower((char)*p), p++);
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
	if(fdtxt != (FILE *) NULL)
		fclose(fdtxt);
	if(fdin != (FILE *) NULL)
		fclose(fdin);

	if(ttt != (DB *) NULL)
		(void) (*(ttt->close))(ttt->internal);		/*close btree */

	if(we_have_started)
                { 
                unlink(textfile);
                unlink(dictfile);
                } 

	exit(-1);
}

static void initialise()
{
#ifdef __HELIOS
	struct sigaction act;
	act.sa_handler = catch_signal;
	act.sa_mask = 0;
	act.sa_flags = 0;
	(void) sigaction(SIGINT, &act, NULL);
#else
	signal(SIGINT, catch_signal);
#endif

	we_have_started = TRUE;

	fprintf(stderr,
		"Building database \"%s\" from \"%s\" (%d stopwords)\n\n",
		ofnm, ifnm, stopcnt);
	fflush(stderr);
}

static void catch_signal(sig)
int sig;
{
	sig = sig;
	signal(SIGINT, SIG_IGN);

	signal_caught = TRUE;
}

	/* make sure the document has a subject */

static void check_title(p)
u_char *p;
{
	u_char *q = p;

	while(*q)
		if(*q++ == '\\')
			return;

	fatal("no subject")
}

static int check_three_col(p)
u_char *p;
{
	int counter;

	for(counter = 0; *p; p++)
		if(*p == '\\')
			counter++;

	return(counter != 2);
}
