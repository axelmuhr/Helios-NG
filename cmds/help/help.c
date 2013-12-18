/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   H E L P   S Y S T E M                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- help.c:          							--
--		Search a help database                        		--
--									--
--	Author:  MJT 28/07/92    					--
--                                                                      --
------------------------------------------------------------------------*/
#ifdef __TRAN
static char *rcsid = "$Id: help.c,v 1.13 1994/03/08 13:16:06 nickc Exp $";
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
#endif
#include "db.h"
#include "btree.h"

        /* weighting for indexed terms (higher number = greater weight) */

#define TOPIC		4
#define SUBJECT		3
#define PURPOSE		2
#define KEYWORD		1
#define NOINDEX		0

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

#define ENDLINE		(unsigned int) 0xff
#define SPACE		' '
#define TAB		'\t'

	/* tokens for text file manager */

#define TEXTWORD	(unsigned int) 0xe0
#define NEWPAGE		(unsigned int) 0xe1
#define EODOC		(unsigned int) 0xee

	/* internal page display flag */

#define INTERNAL	-1

        /* magic number at start of subject list */

#define SUBJECTMAGIC	0x18273645

	/* flag to indicate end of screen when displaying data */

#define END_OF_SCREEN	1

	/* flag bits for display of menu options */

#define PREV_PAGE		0x01
#define NEXT_PAGE		0x02
#define GOBACK			0x04
#define SEARCH			0x08

	/* states for top-level control. Note that the order of the first */
	/* three states are vitally important - they correspond to the    */
	/* order of the 3 selections on the opening title page            */

#define QUERY		0x00
#define TOC		0x01
#define TUTORIAL	0x02
#define PAGE_ONE	0x03
#define LIST		0x04
#define DOC		0x05
#define NOT_FOUND	0x06
#define TOC_SEARCH	0x10

	/* modes for combining search terms */

#define AND		1
#define OR		2

        /* maximum number of concurrent databases */

#define DBMAX		10

	/* pathname of database files */

#ifdef __HELIOS
#define DBPATH		"/helios/lib"
#define OPEN_R          "rb"
#else
#ifdef RS6000
#define DBPATH		"/pds/lib"
#define OPEN_R          "r"
#else
#define DBPATH		"/usr/local/lib"
#define OPEN_R          "r"
#endif
#endif
        /* maximum length of indexed words */

#define WORDMAX		63

        /* maximum number of search words */

#define HITMAX		2048

        /* maximum length of input buffer */

#define BUFFMAX		512

        /* maximum number of stacked states */

#define STATEMAX	10

        /* length and format of address string in LIST_ITEM */

#define ADDRESS_LENGTH	6
#define ADDRESS_FORMAT	"%06lx"

        /* length and format of document number string in search */

#define DOCNUM_LENGTH	6
#define DOCNUM_FORMAT	"%06u"

	/* columns for three-column, right and table display */

#define COL_ONE		4
#define COL_TWO		12
#define COL_THREE	28
#define COL_INC		12
#define COL_LEFT	4
#define COL_RIGHT	20
#define COL_TABLE	24

	/* format string for highlighting */

#define HIGHFORMAT_	"\033[7m_\033[0m%s"
#define HIGHFORMAT_C	"\033[7m%c\033[0m%s"
#define HIGHFORMAT_CS	"\033[7m%c\033[0m%s  "
#define HIGHFORMAT_S1	"\033[7m"
#define HIGHFORMAT_S2	"\033[0m"
#define MOVETO		"\033[%d;%dH"
#define CLEAR_TO_EOL    "\033[K"
#define CLEAR_TO_EOS    "\033[J"

	/* list of paragraph names for text output */

static char *pword[8] = {
	"Purpose: ", 	"", 		"Returns: ",		"Include: ",
	"Format: ",	"Errors: ",	"Description: ",	"Argument: "
};

        /* structure at end of text file for subject list */

static struct subject_header {
        long subject_magic;     /* magic number */
        long subject_pointer;   /* pointer to start of list */
        int subject_count;      /* number in list */
} subject_header;

	/* data for the xrefs on this page */

	/* NB first six elements of xref_data and menu_data are the same */

typedef struct xref_data {
	u_char word[WORDMAX+1];
	int x;
	int y;
	u_long addr;			/* text file address (if known) */
	long docn;			/* document number      "  "    */
	int fd;				/* text file number     "  "    */
	int screen;
};

	/* data for menu items on this page */

typedef struct menu_data {
	u_char word[WORDMAX+1];
	int x;
	int y;
	u_long addr;
	long docn;
	int fd;
	int action;
};

	/* data for title items on this page */

typedef struct title_data {
	u_char *word;
	u_long addr;
	int fd;
	long document_number;
};

	/* data for the screen positions */

typedef struct sp {
	int position;
	unsigned int token;
	int verbatim;
	int right;
	int column_two;
	int column_three;
};

	/* data for search hit list */

typedef struct hitlist {
	int fd;
	u_long addr;
	int weight;
};

static FILE *fdtxt[DBMAX];		/* stream for database text file(s)*/
static int maxdb = 0;			/* number of databases opened */
static char textfile[_POSIX_PATH_MAX]; 	/* name of database text file(.txt)*/
static char dictfile[_POSIX_PATH_MAX]; 	/* name of database dict file(.dct)*/
static int dbnumber;			/* pointer to '?' in above names */
static long doclen;			/* length of doc in text file */
static DB *t[DBMAX];			/* access routines for B-tree(s) */
static int screen_width = -1;		/* width of display */
static int screen_height = -1;		/* height of display */
static int number_to_display = -1;	/* maximum number of display lines */
static int highlight_words = 0;		/* word highlighting flag */
static unsigned int last_token;		/* spare token for next display */
static int cur_x;			/* current display column */
static int cur_y;			/* current display row */
static int verbatim = 0;		/* verbatim flag */
static int right = 0;			/* right indent flag */
static int three_col = 0;		/* three column flag */
static int column_two = COL_TWO;	/* position of column two */
static int column_three = COL_THREE;	/* position of column three */
static u_char *current_page = NULL;	/* pointer to current display page */
static u_char *page_pointer = NULL;	/* pointer to current page pos */
static u_char text_word[WORDMAX+1];	/* current word on page */
static int screen_no = 0;		/* no. of screen within page */
static struct sp screen_pos[256];	/* positions of screens */
static int xref_this = 0;		/* do we xref or not? */
static int xrefp = 0;			/* no. of xrefs on page */
static struct xref_data xref[HITMAX];	/* xref data for page */
static int menup = 0;			/* no. of menu items on page */
static struct menu_data menu[HITMAX+20];/* menu data for page */
static struct title_data title[HITMAX];	/* title data for page */
static u_char search_term[2*BUFFMAX];	/* current search term */
static int search_fd = 0;		/* current search file */
static u_long search_addr = 0;		/* current search address */
static long search_docn = 0;		/* current search docn */
static u_char *internal_page = (u_char *) NULL;	/* pointer to internal page*/
static long page_reqd = 0;		/* length of internal page */
static int state = PAGE_ONE;		/* current top-level state */
static int state_stack[STATEMAX];	/* state stack */
static int ssp = 0;			/* and pointer */
static u_long laddr = -1;		/* temporary save of text file addr*/
static int lfd = 0;			/* temporary save of text file addr*/
static struct hitlist *hit[HITMAX];	/* hit list data */
static int hitp;			/* pointers to above data */
static long document_number;		/* unique document identifier */
static int display_page_one = FALSE;	/* if we are showing the (special) p1 */
static int exact_match = FALSE;		/* if user wants exact search */
static int command_line_query = FALSE;	/* if user gives explicit query */
static int title_only = FALSE;		/* if user wants to check xrefs */
static int mode = OR;			/* search combination mode */
static int done_newline = 0;		/* set if we had to move to new line  */

	/* various macros to access the page data */

#define PEEK		(*page_pointer)
#define NEXT		(*page_pointer++)
#define LAST		(*--page_pointer)
#define PREVIOUS	(page_pointer--)
#define POSITION	(page_pointer - current_page)
#define PAGE_END	(POSITION == doclen)
#define CURRENT		-1

	/* macros to change states */

#define PUSH(c)		(state_stack[(ssp == STATEMAX) ? --ssp : ssp++] = c)
#define POP()		(state_stack[(ssp) ? --ssp : 0])

#ifdef __STDC__
#define fn(a, b) a b
#else
#define fn(a, b) a()
#endif

	/* function prototypes */

static void	fn( show_title, (void));
static int	fn( print_docs, (int field));
static int 	fn( do_search, (u_char *bb, int field));
static int 	fn( search_btree, (u_char *bb, int field));
static int 	fn( show_document, (int fd, u_long addr));
static void 	fn( initialise, (void));
static void 	fn( screen_size, (void));
static void 	fn( move_to, (int y, int x));
static void 	fn( clear_screen, (void));
static u_char *	fn( get_text_token, (unsigned int *token));
static int 	fn( wait_for_user, (int bits));
static int 	fn( tab, (void));
static void 	fn( skip_to_marker, (void));
static int 	fn( display_word, (u_char *p));
static int 	fn( newline, (void));
static u_char *	fn( getline, (u_char *p, int echo));
static int 	fn( space, (void));
static void 	fn( clean_up, (char *p));
static void 	fn( add_xref, (u_char *p, long docn));
static int 	fn( do_xrefs, (void));
static int 	fn( no_more, (void));
static void 	fn( check_three_col, (void));
static int 	fn( move_cursor, (u_char c, int *current_label));
static void 	fn( put_menu, (char *a, int action));
static int 	fn( sort_title, (const void *a, const void *b));
static int 	fn( sort_hit, (const void *a, const void *b));
static void 	fn( highlight_term, (int num));
static void 	fn( lowlight_term, (int num));
static int 	fn( page_one, (void));
static int 	fn( table_of_contents, (void));
static int 	fn( compare_string, (u_char *s1, u_char *s2));
static int 	fn( compare_string_nocase, (u_char *s1, u_char *s2));
static int 	fn( isvalid, (u_char c));
static void 	fn( word_to_lower, (u_char *p));
static u_long 	fn( swap, (u_long s));
static void 	fn( usage, (void));
static void 	fn( termend, (void));
extern u_char *	fn( stem, (u_char *wordptr));

#ifndef __HELIOS
static void 	fn( clean_exit, (int sig));
#endif

int main(argc, argv)
int argc;
char **argv;
{
	u_char buff[BUFFMAX];		/* buffer for user input */
	int status, db, dbn;		/* status & db numbers */
	char *dbpath;			/* path to database files */
	u_long taddr;			/* address of tutorial document */
	int field;			/* field to search in */

	BTREEINFO b;			/* start up info for B-tree */

			/* process arguments */

	search_term[0] = 0;		/* start at the start */

	while(--argc)
		{
		char *ac = *++argv;	/* next arg */

		switch(ac[0])
			{
			case '-':
				switch(ac[1])
					{
					case 'a':	/* refine search */
						mode = AND;
						break;

					case 'o':	/* expand search */
						mode = OR;
						break;

					case 'e':	/* title only */
						exact_match = TRUE;
						break;

					case 'T':	/* title count */
						title_only = TRUE;
						break;

					default:
						usage();
					}
				break;

			default:	/* must be a search term */
				if(strlen((char *)search_term) + WORDMAX +1 >
					            BUFFMAX)
					break;
				command_line_query = TRUE;
				if(strlen((char *)search_term))
					strcat((char *)search_term, " ");
				strncat((char *)search_term, ac, WORDMAX);
				break;
			}
		}

	if((title_only || exact_match) && !command_line_query)
		usage();

		/* if the user has the path in the environment */
		/* get it from there. Otherwise use default    */

	if((dbpath = getenv("HELP_DBPATH")) == (char *)NULL)
		dbpath = DBPATH;

	/* and form db names */

	strcpy(textfile, dbpath);
	strcat(textfile, "/help?");

	dbnumber = strlen(textfile) - 1;
	strcpy(dictfile, textfile);

	strcat(textfile, ".txt");
	strcat(dictfile, ".dct");

		/* set up the screen and signals */

	if(title_only)
		signal(SIGINT, SIG_IGN);
	else
		initialise();

		/* initialise B-tree */

	b.psize = 0;
	b.cachesize = 0;
	b.lorder = 0;
	b.flags = 0;			/* No dups */
	b.compare = compare_string;	/* use our own (for unsigned chars) */

	for(db = 0 ; db < DBMAX; db++)	/* zero data to start with */
		{
		fdtxt[db] = (FILE *)NULL;
		t[db] = (DB *)NULL;
		}

	for(db = dbn = 0 ; db < DBMAX; db++)	/* and open everything up */
		{

		textfile[dbnumber] = '0' + db;	/* db number */
		dictfile[dbnumber] = '0' + db;	/* db number */

		fdtxt[dbn] = fopen(textfile, OPEN_R);	/* open .txt file */
		if(fdtxt[dbn] == (FILE *)NULL)
			{
			if(db == 0)
				clean_up("failed to open textfile (check HELP_DBPATH)");
			else
				continue;
			}

			/* and open the B-tree */
	
		if((t[dbn] = btree_open(dictfile, O_RDONLY, 0666, &b)) ==
				(DB *) NULL)
			{
			if(db == 0)
				clean_up("failed to open dictfile (check HELP_DBPATH)");
			}
		else
			dbn++;
		}

	maxdb = dbn;			/* remember no. of dbs */

		/* and here we go .... */

	if(title_only)			/* special for Xref checking */
		{
		char Tbuff[BUFFMAX+1];

		strcpy(Tbuff, (char *)search_term);

		status = search_btree((u_char *)Tbuff, TOPIC);
		if(status == NOT_FOUND)
			{
			printf("No document with the title \"%s\"\n",
					search_term);
			exit(-1);
			}
		printf("%d documents match the title \"%s\"\n", hitp,
							search_term);
		exit(0);
		}

	while(1)
		{

		if(command_line_query)		/* query on command line */
			{
			command_line_query = FALSE;
			state = PAGE_ONE;
			}
		else
			state = page_one();

		screen_size();

		switch(state)
			{
			case TUTORIAL:		/* show tutorial doc         */
						/* always doc #2, so read    */
						/* length of doc #1 and skip */
						/* past it.                  */

				if(fseek(fdtxt[0], 1, 0))
					clean_up("corrupt text file (seek)");
				if(fread(&doclen, sizeof(long), 1, fdtxt[0])!=1)
					clean_up("corrupt text file (read)");
				doclen = swap(doclen);

				taddr=
				(u_long)doclen+ADDRESS_LENGTH+2*(sizeof(long)+1);

				show_document(0, taddr);
				continue;

			case TOC:		/* subject list */
			case TOC_SEARCH:
				state = TOC;
				status = table_of_contents();
				if(status == GOBACK)
					continue;

				state = TOC_SEARCH;
				break;

			case QUERY:		/* general query */
				move_to(CURRENT, cur_x+3);
				printf("Enter search term - ");
				fflush(stdout);

				if(getline(buff, TRUE) == (u_char *) NULL)
					clean_up((char *)0);

				if(!buff[0])
					continue;

				strncpy((char *)search_term, (char *)buff,
					WORDMAX);
				break;
			}

		PUSH(state);

		field = -1;

		if(state == PAGE_ONE && exact_match)
			field = TOPIC;

		status = search_btree(search_term, field);

		if(state == PAGE_ONE && status == NOT_FOUND)
			{
			termend();
			fprintf(stderr, "HELP: sorry, nothing found\r\n");
			fflush(stderr);
			exit(-1);
			}

		if(status != NOT_FOUND)
			status = print_docs(field);

		exact_match = FALSE;		/* command line only */

		while(state != PAGE_ONE)
			{
			switch(status)
				{
				case NOT_FOUND:	/* oh dear .. tell him */
					move_to(screen_height-1,1);
					printf(CLEAR_TO_EOL);
					printf("No documents found\n");
					printf(CLEAR_TO_EOL);
					printf("Press RETURN to continue ");
					fflush(stdout);
					if(getline(buff, FALSE)
						        == (u_char *) NULL)
						clean_up((char *)0);
					status = GOBACK;

				case GOBACK:	/* step back a state */
					state = POP();
					if(state == TOC || state == TOC_SEARCH)
						{
						state = TOC;
						status = table_of_contents();
						state = TOC_SEARCH;
						break;
						}

					if(state == LIST)
						{
						status =
						 show_document(0, INTERNAL);
						break;
						}
						
					break;

				case SEARCH:	/* user asked for a search */
						/* if search_addr is non-zero */
						/* then we already know the */
						/* correct doc - so show it */
					PUSH(state);
					if(search_addr)
					   {
					   status=show_document(search_fd,
						search_addr);
					   state = DOC;
					   }
					else	/* do a new search */
					   {
					   field = TOPIC;

					   if(state == TOC_SEARCH)
						field = -1;
					   status = search_btree(search_term,
					       field);
					   if(status == NOT_FOUND)
						break;
					   status = print_docs(field);
					   }
					break;
				}
			}

		if(internal_page != (u_char *) NULL)
			{
			free(internal_page);
			internal_page = (u_char *) NULL;
			}
		}
}

	/* format of B-tree data is :					     */

	/* length, textfile address, weight [, textfile address, weight ...] */

	/* where length and weight(s) are ints and address is a long         */

	/* the following macros allow us to calculate address,weight offsets */

#define ADDROFF		(sizeof(int))
#define WEIGHTOFF	(sizeof(int)+sizeof(long))

	/* given a term, look it up in the B-tree */

static int do_search(bb, field)
u_char *bb;
int field;
{
	DBT key;
	DBT data;
	DB *tt;
	int i, db, status, cnt, offset, weight;
	u_long addr;
	int got_some_docs = FALSE;

	key.data = bb;				/* set up search key */
	key.size = strlen((char *)bb) + 1;

	if(state == TOC_SEARCH)
		{
		move_to(1,1);
		printf("Searching ...");
		fflush(stdout);
		}

	if(field == -1)			/* free search */
		{
		word_to_lower(bb);	/* lower case */
		key.data = stem(bb);	/* look for word stem */
		key.size = strlen((char *)key.data) + 1;
		}

	for(db = 0 ; db < maxdb ; db++)		/* search all databases */
		{
		tt = t[db];			/* B-tree pointer */

			/* call the B-tree search routine with the given key */
			/* data (if any) is returned in (DBT) data           */

		status = (*(tt->get))(tt->internal, &key, &data, 0);

		switch(status)
			{
			case RET_SUCCESS:		/* got it */
				got_some_docs = TRUE;
				bcopy((char *)data.data, (char *)&cnt,
					sizeof(int));
				cnt = (int) swap(cnt);
				for(i = 0 ; i < cnt; i++)
					{
						/* get address and weight */

					offset = ADDROFF + i*WEIGHTOFF;
					bcopy((char *)&data.data[offset],
						(char *)&addr, sizeof(long));
					bcopy((char *)&data.data[offset+ADDROFF]
						,(char *)&weight, sizeof(int));

						/* make sure we want it */

					if(field==TOPIC && swap(weight)!=TOPIC)
						continue;

						/* save it away */

					hit[hitp] = (struct hitlist *)
						malloc(sizeof(struct hitlist));
					if(hit[hitp] == (struct hitlist *)NULL)
						break;
					hit[hitp]->addr = swap(addr);
					hit[hitp]->weight = (int) swap(weight);
					hit[hitp++]->fd = db;
					if(hitp == HITMAX)
						{
						hitp--;
						break;
						}
					}
				break;

			case RET_SPECIAL:		/* non-existent */
				continue;

			case RET_ERROR:			/* oops ... */
				clear_screen();
				fprintf(stderr, "errno = %d\n", errno);
				fflush(stderr);
				clean_up("B-tree error");
			}
		}

	return(got_some_docs ? QUERY : NOT_FOUND);
}

static int print_docs(field)
int field;
{
	int i, nextc, len, nextpos;
	int cnt = hitp;
	int status = 0;
	u_char buff[BUFFMAX+1], *p;

		/* get number of search hits */

	if(cnt > HITMAX)		/* too many to deal with */
		{
		for(i = HITMAX ; i < cnt ; i++)
			free(hit[i]);
		cnt = HITMAX;
		}

	if(cnt == 1)		/* display single doc */
		{
		status = show_document(hit[0]->fd, hit[0]->addr);
		free(hit[0]);
		state = DOC;
		return status;
		}

				/* if we are searching for a title e.g. Xref */
				/* and we know the document number concerned */
				/* then use that one (if found), otherwise   */
				/* use first one in the list                 */

	if(field == TOPIC && !exact_match)
		{
		for(i = 0 ; i < cnt; i++)
			{
			FILE *f = fdtxt[hit[i]->fd];
			if(fseek(f, hit[i]->addr, 0))	/* seek to docnum */
				clean_up("corrupt text file (seek)");
			if(fscanf(f, DOCNUM_FORMAT, &document_number) != 1)
				clean_up("corrupt text file (fscanf)");
			if(document_number == search_docn)	/* this one */
				break;
			}
		if(i == cnt)
			i = 0;
		status = show_document(hit[i]->fd, hit[i]->addr);
		for(i = 0 ; i < cnt ; i++)
			free(hit[i]);
		state = DOC;
		return status;
		}

		/* if we have more than one document, display a list */
		/* to do this, sort the titles into alphabetical order, */
		/* build a dummy document page, and let the display */
		/* routines go into action */

	page_reqd = 0;			/* number of bytes for display page */
	nextpos = 0;			/* number of docs accepted */

	for(i = 0 ; i < cnt; i++)
		{
		FILE *f = fdtxt[hit[i]->fd];
		int pos, j;

		if(field == TOPIC && hit[i]->weight != TOPIC)
			continue;

		if(fseek(f, hit[i]->addr, 0))	/* seek to docnum */
			clean_up("corrupt text file (seek)");
		if(fscanf(f, DOCNUM_FORMAT, &document_number) != 1)
			clean_up("corrupt text file (fscanf)");
		pos = nextpos;
		for(j = 0; j < i; j++)		/* check for overwrite */
			if(title[j].document_number == document_number)
				{
				pos = j;
				break;
				}

			/* get the title, but cater for very long lines */

		p = buff;
		while(getc(f) != TITLE);
       		while((p-buff < BUFFMAX) && ((nextc = getc(f))!=ENDLINE))
			{
       			*p++= nextc;
			if(nextc == '\\' && (p-buff > WORDMAX/2))
				{
				p = buff+WORDMAX/2;
				*p++ = nextc;
				}
			}

       		*p = 0;
		len = strlen((char *)buff) + 11;
		page_reqd += len + 8L;

			/* now store the title, textfile address and db no. */

		if(pos != nextpos)
			free(title[pos].word);
		title[pos].word = (u_char *)malloc(len);
		if(title[pos].word == (u_char *) NULL)
			clean_up("failed to malloc titles");

		title[pos].addr = hit[i]->addr;
		title[pos].fd = hit[i]->fd;
		title[pos].document_number = document_number;
		strcpy((char *)title[pos].word, (char *)buff);
		free(hit[i]);
		if(pos == nextpos)
			nextpos++;
		}

	if(nextpos == 1)		/* reduced to a single doc */
		{
		free(title[0].word);
		status = show_document(title[0].fd, title[0].addr);
		state = DOC;
		return status;
		}

	qsort(title, nextpos, sizeof(struct title_data), sort_title);

		/* now build dummy screen to display */

	p = internal_page = (u_char *)malloc((int)page_reqd + 15);
	if(internal_page == (u_char *) NULL)
		clean_up("failed to malloc internal page");

		/* as this is a list, show the total number of docs */

	*p++ = VERBATIM;		/* force a blank line */
	sprintf((char *)p, "%3d retrieved", nextpos);
	p += 13;
	*p++ = ENDLINE;

	for(i = 0 ; i < nextpos; i++)
		{
		*p++ = THREECOL;		    /* display in 3 columns */
		sprintf((char *)p, "%3d\\", i+1);   /* add index number */
		p += 4;				    /* and column marker */

		*p++ = LIST_ITEM;		    /* list marker */

		sprintf((char *)p, ADDRESS_FORMAT, title[i].addr); /* address */
		p += ADDRESS_LENGTH;

		*p++ = 1+title[i].fd;		    /* file number */

		strcpy((char *)p, (char *)title[i].word);	/* data */
		p += strlen((char *)title[i].word);

		*p++ = ENDLINE;			    /* end of line marker */

		free(title[i].word);		    /* tidy up */
		}

	*p++ = EODOC;

	/* Having built the page in memory, display it */

	state = LIST;
	status = show_document(0, INTERNAL);

	return status;
}

	/* Assuming that the screen is now clear, put the title and */
	/* subject lines, centring as we go */

static void show_title()
{
	int nextc, len;
	u_char buff[WORDMAX+1], *p;

	p = buff;

	move_to(2,1);

	while(1)
		{
		nextc = NEXT;

			/* '\' delimits the title and subject */
			/* but limit title to WORDMAX chars per line */

		if(p-buff < WORDMAX && nextc != '\\' && nextc != ENDLINE)
			{
			*p++ = nextc;
			continue;
			}

			/* if we overflow, skip back to previous word */
			/* and display current title line             */

		if(p-buff == WORDMAX)
			{
			p--;
			nextc = LAST;
			nextc = LAST;
			if(nextc == ' ' || nextc == ',')
				while(nextc == ' ' || nextc == ',')
					{
					nextc = LAST;
					p--;
					}
			else
				{
				while(nextc != ' ' && nextc != ',')
					{
					nextc = LAST;
					p--;
					}
				while(nextc == ' ' || nextc == ',')
					{
					nextc = LAST;
					p--;
					}
				}
			p++;
			nextc = NEXT;
			nextc = NEXT;
			nextc = NEXT;
			}

		*p = 0;
		len = strlen((char *)buff);
		if(len <= screen_width)	   /* it fits (phew ...) */
			move_to(CURRENT, (screen_width-len)/2);
		else
			move_to(CURRENT,1);
		display_word(buff);
		if(nextc != ENDLINE)
			newline();
		p = buff;
		if(nextc == ENDLINE)
			break;
		}
}

#ifdef __HELIOS

/*  This routine is responsible for setting the screen to raw input mode, */
/*  for the display and keyboard handling. Signals are looked after here. */

static void initialise(void)
{
	Attributes attr;

	if( ! (isatty(0) && isatty(1)) )
		{
		fputs("HELP: not running interactively.\r\n", stderr);
		exit(EXIT_FAILURE);
		}

	if (GetAttributes(fdstream(0), &attr) < Err_Null)
		{
		fputs("HELP: failed to get keyboard details.\r\n", stderr);
		exit(EXIT_FAILURE);
		}

	AddAttribute(&attr, ConsoleRawInput);
	RemoveAttribute(&attr, ConsoleEcho);
	RemoveAttribute(&attr, ConsoleRawOutput);
	RemoveAttribute(&attr, ConsoleBreakInterrupt);

	if (SetAttributes(fdstream(0), &attr) < Err_Null)
		{
		fputs("HELP: failed to initialise keyboard.\r\n", stderr);
		exit(EXIT_FAILURE);
		}

	screen_size();
}

	/* and reverse the attributes at termination */

static void termend(void)
{
	Attributes attr;

	if (GetAttributes(fdstream(0), &attr) < Err_Null)
		return;

	RemoveAttribute(&attr, ConsoleRawInput);
	AddAttribute(&attr, ConsoleEcho);
	AddAttribute(&attr, ConsoleRawOutput);
	AddAttribute(&attr, ConsoleBreakInterrupt);

	if (SetAttributes(fdstream(0), &attr) < Err_Null)
		return;
}

static void screen_size(void)
{
	Attributes attr;

	if (GetAttributes(fdstream(1), &attr) < Err_Null)
		{
		fputs("HELP: failed to determine screen size.\r\n", stderr);
		exit(EXIT_FAILURE);
		}

	if ((attr.Time == screen_width) && (attr.Min == screen_height))
		return;

	screen_width = attr.Time;
	screen_height = attr.Min;

	if ((screen_height < 17) || (screen_width < 40))
		{
		fputs("HELP: screen too small.\r\n", stderr);
		exit(EXIT_FAILURE);
		}

	/* Allow two lines at the bottom for the menu options. */

	number_to_display = screen_height - 2;
}

#else

#include <sys/ioctl.h>

#ifdef _INCLUDE_HPUX_SOURCE
#include <sgtty.h>
#endif

static int console;
static struct sgttyb normal, special;
 
static void initialise()
{
	if (! (isatty(0) && isatty(1)))
 		{
		fputs("HELP: not running interactively.\n", stderr);
   		exit(-1);
 		}
	console = fileno(stdin);
	ioctl(console, TIOCGETP, &normal);
	bcopy((char *)&normal, (char *)&special, sizeof(struct sgttyb));
	special.sg_flags &= ~ECHO;
#ifdef _INCLUDE_HPUX_SOURCE
	special.sg_flags |= RAW;
#else
	special.sg_flags |= CBREAK;
#endif
	ioctl(console, TIOCSETP, &special);
	signal(SIGINT, clean_exit);

	screen_size();
}

static void screen_size()
{
#ifdef TIOCGWINSZ
	struct winsize win;

	if (ioctl(console, TIOCGWINSZ, &win) >= 0)
		{
		screen_height = win.ws_row;
		screen_width = win.ws_col;
		}
#else
	screen_height = 24; screen_width = 80;
#endif
	number_to_display = screen_height - 2;

	if ((screen_height < 17) || (screen_width < 40))
		{
		fputs("HELP: screen too small.\n", stderr);
		termend();
		exit(-1);
		}
}
 
static void termend()
{
	ioctl(console, TIOCSETP, &normal);
}
#endif

	/* move the cursor to the given position. */

static void move_to(y,x )
int y;
int x;
{
	if(y == CURRENT)
		y = cur_y;
	if(x == CURRENT)
		x = cur_x;
	if(x > screen_width)
		x = screen_width;
	if(y > screen_height)
		y = number_to_display;

	printf(MOVETO, y, x);
	cur_x = x;
	cur_y = y;
}

	/* clear the screen */

static void clear_screen()
{
	cur_x = cur_y = 1;
	printf(MOVETO, 1, 1 );
	printf(CLEAR_TO_EOS);	
	putchar('\f');		/* XXX does not work on UNIX boxes */
	fflush(stdout);
}


	/* display a document (from address 'addr' in the database text file */
	/* This routine takes care of formatting the text, highlighting      */
	/* cross references, user input, etc.                                */

static int show_document(fd, addr)
int fd;
u_long addr;
{
	unsigned int token;
	int status;
	u_char *p;
	FILE *textfd = fdtxt[fd];

	if(addr != INTERNAL)		/* read from disk */
		{
			/* go to start of document */

		if(fseek(textfd, addr-sizeof(long), 0))
			clean_up("corrupt database");

		fread(&doclen, sizeof(long), 1, textfd);  /* get doc length */

		doclen = swap(doclen);

			/* get (unique) document number */

		fscanf(textfd, DOCNUM_FORMAT, &document_number);

		/* allocate enough space to read in the entire page */

		page_pointer = current_page = (u_char *)malloc((int)doclen + 1);
		if(current_page == (u_char *) NULL)
			clean_up("out of memory");

		/* and read it in - all operations on this page can */
		/* now be performed without re-reading the disk */

		fread(current_page, (int) doclen, 1, textfd);
		current_page[doclen] = EODOC;
		}
	else				/* already in memory */
		{
		if(internal_page== (u_char *) NULL)
			return GOBACK;
		page_pointer = current_page = internal_page;
		doclen = page_reqd;
		document_number = -1;
		}

		/* initialise page 1 data */

	screen_pos[0].position = 0;
	screen_pos[0].token = 0;
	screen_pos[0].verbatim = 0;
	screen_pos[0].right = 0;
	screen_pos[0].column_two = COL_TWO;
	screen_pos[0].column_three = COL_THREE;

	three_col = right = xrefp = last_token = screen_no = 0;

	clear_screen();

	move_to(1,1);			/* go to top left */

		/* now loop, displaying pages, until the user */
		/* wants to do something else */

	while(1)
		{
		p = get_text_token(&token);

		if(p == (u_char *) NULL)		/* end of doc */
			{
			status = 0;

			if(addr != INTERNAL && !display_page_one)
				status = do_xrefs();

			if(status == GOBACK || status == SEARCH)
				{
				if(addr != INTERNAL)
					{
					free(current_page);
					current_page = (u_char *)NULL;
					}
				return status;
				}

			if(status == 0)
				status = wait_for_user((screen_no > 0) ?
					PREV_PAGE : 0);

			if(status == GOBACK || status == SEARCH)
				{
				if(addr != INTERNAL)
					{
					free(current_page);
					current_page = (u_char *)NULL;
					}
				return status;
				}
			continue;
			}

		status = 0;
		switch(token)
			{
			case TITLE:		/* got a title */
				show_title();
				break;

			case PARAGRAPH:		/* some sort of paragraph */
				if((status = newline()) == END_OF_SCREEN)
					break;
				if((status = newline()) == END_OF_SCREEN)
					break;
				status = display_word(p);
				break;

			case XREF:		/* cross reference */
				xref_this = TRUE;
				break;

			case TEXTWORD:		/* a *real* word ! */
				if(highlight_words)	/* list item */
					{
					strcpy((char *)xref[xrefp].word,
						(char *)p);
					if(display_page_one)
						{
						xref[xrefp].x = cur_x;
						xref[xrefp].addr = xrefp;
						}
					else
						{
						xref[xrefp].x = column_two;
						xref[xrefp].addr = laddr;
						}
					xref[xrefp].y = cur_y;
					xref[xrefp].fd = lfd;
					xref[xrefp].screen = -1;
					status = display_word(p);
					if(status == END_OF_SCREEN)
						break;
					xref[xrefp].screen = screen_no;
					xrefp++;
					highlight_words = FALSE;
					break;
					}
				if(xref_this)
					{
					u_char tbuff[WORDMAX+1], *tp;
					long docn = 0;

					strcpy((char *)tbuff, (char *)p);
					for(tp = tbuff;
					   *tp && *tp != '\\'; tp++);

					if(*tp)
					    sscanf((char *)(tp+1),
						DOCNUM_FORMAT, &docn);

					*tp = 0;

					add_xref(tbuff, docn);
					}
				else
					status = display_word(p);
				break;

			case KWORD:	/* keyword */
			case QWORD:	/* permuted index */
				skip_to_marker();
				break;

			case THREECOL:	/* three column table */
				if(!right)
					{
					check_three_col();
					if((status = newline())==END_OF_SCREEN)
						break;
					}
				right = COL_ONE;
				status = newline();
				break;

			case RIGHT:	/* right side of two-column table */
				if(cur_x < COL_RIGHT)
					{
					move_to(CURRENT, COL_RIGHT);
					right = cur_x;
					}
				else
					{
					right = COL_RIGHT;
					status = newline();
					}
				break;

			case TABLE:	/* indented text */
				right = COL_TABLE;
				status = newline();
				break;

			case LEFT:	/* left side of two-column table */
				move_to(CURRENT, COL_LEFT);
				right = COL_LEFT;
				if((status = newline())==END_OF_SCREEN)
					break;
				if((status = newline())==END_OF_SCREEN)
					break;
				break;

			case NEWPAGE:	/* page break */
				status = END_OF_SCREEN;
				token = 0;
				break;

			case VERBATIM:	/* verbatim text */
				if(verbatim == 1)
					{
					if((status = newline())==END_OF_SCREEN)
						break;
					if((status = newline())==END_OF_SCREEN)
						break;
					}

				status = tab();
				break;

			case ENDLINE:	/* end of line marker */
				if(verbatim)
					{
					if(PEEK == VERBATIM)
						status = newline();
					break;
					}

			case SPACE:	/* (soft) space */
				status = space();
				break;

			case TAB:	/* horizontal tab */
				status = tab();
				break;

			default:
				break;
			}
		last_token = 0;
	
		if(status == END_OF_SCREEN)
			{
			int bits;

			bits = ((screen_no > 0) ? PREV_PAGE : 0);
			bits |= ((PAGE_END || no_more()) ? 0 : NEXT_PAGE);

			last_token = token;
			status = wait_for_user(bits);
			if(status == GOBACK || status == SEARCH)
				{
				if(addr != INTERNAL)
					{
					free(current_page);
					current_page = (u_char *)NULL;
					}
				return status;
				}
			}
		}
}

	/* get a word (or special token) from the current page data */

static u_char *get_text_token(token)
unsigned int *token;
{
	u_char *p;
	int c;

	p = text_word;

	if(last_token)
		{
		*token = last_token;
		return p;
		}

	while(1)
		{
		switch(c = NEXT)
			{
			case EODOC:	/* end of document */
				return (u_char *) NULL;

			case VERBATIM:
				verbatim++;
				xref_this = FALSE;
				three_col = right = 0;
				*token = c;
				return text_word;

			case THREECOL:
				three_col = 1;
				xref_this = FALSE;
				verbatim = 0;
				*token = c;
				return text_word;

			case LIST_ITEM:		/* list item - only used  */
						/* internally and on page */
						/* one			  */

				highlight_words = TRUE;

					/* save text file address */

				sscanf((char *) page_pointer, ADDRESS_FORMAT,
					&laddr);
				page_pointer += ADDRESS_LENGTH;
				lfd = NEXT - 1;
				continue;

			case PARAGRAPH:		/* paragraph marker - next */
						/* byte is paragraph type  */

				switch(c = NEXT)
					{
					case P_PARAGRAPH:
					case P_PURPOSE:
					case P_RETURNS:
					case P_INCLUDE:
					case P_FORMAT:
					case P_ERROR:
					case P_DESCRIPTION:
					case P_ARGUMENT:
						strcpy((char *)text_word,
							pword[c-1]);
						break;

					default: PREVIOUS;
					}
				c = PARAGRAPH;
			case TITLE:
			case KWORD:
			case QWORD:
			case LEFT:
			case RIGHT:
			case TABLE:
			case XREF:
				verbatim = 0;
				xref_this = FALSE;
				three_col = right = 0;
				*token = c;
				return text_word;

			case '\\':	/* backslash *might* be special */

				if(verbatim || xref_this)
					{
					*p++ = c;
					break;
					}
				if(!three_col)
					{
					if(PEEK != '\\')
						{
						*p++ = SPACE;
						*p++ = c;
						}
					break;
					}
				*p = 0;
				if(p == text_word)
					{
					three_col++;
					break;
					}

				PREVIOUS;
				*token = TEXTWORD;
				return text_word;

			case FORMATCHAR:	/* ignore bold, italic */
				c = NEXT;
				continue;

			case ENDLINE:
			case NEWPAGE:
			case TAB:
			case SPACE:
				*p = 0;
				if(p == text_word)
					{
					*token = c;
					return text_word;
					}

				PREVIOUS;
				*token =  TEXTWORD;
				return text_word;

			default:
				*p++ = c;
				break;
			}
		}
}

	/* put a newline on the available screen area */

static int newline()
{
	if(cur_y == number_to_display)
		return END_OF_SCREEN;

	if(cur_y == 1 && cur_x == 1)	/* top of screen - don't bother */
		return 0;

	cur_y++;
	cur_x = 1;
	move_to(cur_y, cur_x);
	return 0;
}

	/* display a word (with highlights, if necessary). If the current */
	/* word won't fit on the line, wait until the next line */

static int display_word(p)
u_char *p;
{
	int status = 0;
	u_char *q, *r, buff[WORDMAX+1];

	if(*p == 0)
		return 0;

	if((cur_x + strlen((char *)p) + highlight_words) > screen_width)
		{
		status = newline();
		done_newline = 1;
		}

	if(status)
		return END_OF_SCREEN;

	if(right && cur_x == 1)
		move_to(CURRENT, right);
	if(three_col)
		{
		if(three_col == 2)	right = column_two;
		if(three_col == 3)	right = column_three;
		if(right > cur_x)
			move_to(CURRENT, right);
		}
	if((cur_x + strlen((char *)p) + highlight_words) > screen_width)
		p[screen_width - cur_x] = 0;		/* truncate it */

		/* check for hard spaces */

	for(q = p , r = buff; (*r = *q) != (u_char)0 ; q++, r++)
		if(*q == (u_char)HARD_SPACE)
			*r = (u_char)' ';

	if(highlight_words)
		{
		cur_x++;
		printf(HIGHFORMAT_, buff);
		}
	else
		printf("%s", buff);
	cur_x += strlen((char *)buff);
	return 0;
}

	/* ignore text to next special marker */

static void skip_to_marker()
{
	while((NEXT & 0x80) == 0);
	PREVIOUS;
}

	/* put a space (but not if we are at left edge of screen) */

static int space()
{
	if(cur_x == screen_width)
		return(0);

	if(cur_x > 1)
		{
		putchar(SPACE);
		cur_x++;
		}
	return 0;
}

	/* put a tab (only used for left-edge indenting */

static int tab()
{
	cur_x++;

	while(cur_x % 8)
		cur_x++;

	if(cur_x >= screen_width)
		return(newline());

	move_to(cur_y, cur_x);
	return 0;
}

	/* the main keyboard handler - allow the user to move around the */
	/* screen to his heart's content. When he finally selects */
	/* something, do the appropriate thing. The 'bits' flags which */
	/* of the possible actions are available to him at this point. */
	/* Quit is always allowed, Goback is allowed on all but page one. */

static int wait_for_user(bits)
int bits;
{
	u_char c, *p;
	int action;
	int current_action = 0;
	int current_label = 0;
	char buff[BUFFMAX+1];

	menup = 0;

	for(action = 0 ; action < xrefp; action++)
		if(screen_no == xref[action].screen)	/* yes - add it */
			{
			bcopy((char *)&xref[action], (char *)&menu[menup],
				sizeof(struct menu_data));
			menu[menup++].action = SEARCH;
			}

	action = 0;
	move_to(screen_height, 1);
	put_menu((bits & NEXT_PAGE) ? "Forward" : (char *)NULL, 'f');
	put_menu((bits & PREV_PAGE) ? "Back" : (char *)NULL, 'b');
	put_menu((state == TOC) ? "Search" : (char *)NULL, 's');
	if(!display_page_one)
		put_menu("Go back", 'g');
	put_menu("Quit", 'q');

	highlight_term(action);		/* show the user */
	current_action = menu[action].action;

	action = 0;

	while(1)
		{
		if(action)
			c = action;
		else
#ifndef __HELIOS
			read(0, (char *)&c, 1);
#else
			Read(fdstream(0), (char *)&c, 1, -1);
#endif

		switch(tolower(c))
			{
			case 'q':		/* quit */
			case -1:
				clean_up((char *)0);

#ifdef __HELIOS
			case 0x03:
				clean_up("interrupted by user");
#endif

			case 'd':		/* print document number */
				if(document_number > 2)
					{
					int tx = cur_x;
					int ty = cur_y;

					move_to(screen_height -1,1);
					printf("Displaying document %06lu",
						document_number);
					move_to(ty, tx);
					fflush(stdout);
					}
				break;

			case 'f':		/* forward */
			case SPACE:
					/* if we are allowed forward, */
					/* save page state, in case we */
					/* ever come back */

				if(bits & NEXT_PAGE)
					{
					screen_pos[++screen_no].position
						= POSITION;
					if(last_token == TEXTWORD)
						screen_pos[screen_no].position
						-= strlen((char *)text_word);

					screen_pos[screen_no].token=last_token;
					screen_pos[screen_no].verbatim
						 = verbatim;
					screen_pos[screen_no].right = right;
					screen_pos[screen_no].column_two
						 = column_two|(three_col << 8);
					screen_pos[screen_no].column_three
						 = column_three;
					clear_screen();
					return NEXT_PAGE;
					}
				break;

			case 'b':		/* back */
					/* if we are allowed, restore */
					/* previous page state        */
				if(bits & PREV_PAGE)
					screen_no--;
				else
					break;

			case '\f':		/* re-draw screen */

				page_pointer = current_page +
				    screen_pos[screen_no].position;
				last_token=screen_pos[screen_no].token;
				verbatim=screen_pos[screen_no].verbatim;
				right=screen_pos[screen_no].right;
				column_two
				    =screen_pos[screen_no].column_two;
				three_col = column_two >> 8;
				column_two &= 0xff;
				column_three
				    =screen_pos[screen_no].column_three;
				screen_size();
				clear_screen();
				return PREV_PAGE;

			case 'g':		/* pop a state */
				if(!display_page_one)
					return GOBACK;
				action = 0;
				break;

#ifdef __HELIOS
			case 0x9b:		/* cursor movement */
				Read(fdstream(0), (char *)&c, 1, -1);
				c = move_cursor(c, &current_label);

				if(c == 0x03)
					clean_up("interrupted by user");

				if(c != 0)
					current_action = c;
				break;

			case '\033':		/* treat escape as go-back */
						/* specially for Paul ...  */
				action = 'g';
				break;

#else
			case '\033':		/* cursor movement */
				read(0, &c, 1);
				if(c == '[')
					read(0, &c, 1);
				c = move_cursor(c, &current_label);
				if(c != 0)
					current_action = c;
				break;
#endif

			case 's':		/* sub-search */
				if(state != TOC || current_action != SEARCH)
					break;
			case SEARCH:
				p = menu[current_label].word;
				while(!isvalid(*p))	p++;
				strcpy((char *)search_term, (char *)p);
				for(p = search_term; *p; p++);
				p--;
				while(!isvalid(*p))	p--;
				*++p = 0;
				if(c == 's')
					{
					move_to(CURRENT, cur_x+3);
					printf("Enter search term - ");
					fflush(stdout);

					if(getline((u_char *)buff, TRUE) ==
							(u_char *) NULL)
						clean_up((char *)0);

					if(buff[0])
						{
						strcat((char *)buff, " + ");
						strcat(buff,
						   (char *)search_term);
						strcpy((char *)search_term,
						    buff);
						search_addr = 0;
						return SEARCH;
						}
					else
						{
						action = '\f';
						break;
						}
					}

				search_docn = menu[current_label].docn;
				search_addr = menu[current_label].addr;
				search_fd = menu[current_label].fd;
				return SEARCH;

			case '\n':	/* do whatever the cursor is */
					/* pointing to.		     */
			case '\r':
				if(current_action != 's')
					action = current_action;
				else
					action = 0;
				break;
			}
		}
}

	/* get a line of data from the user. Take care of backspace, etc */

static u_char *getline(p, echo)
u_char *p;
int echo;
{
	u_char c;
	u_char lastc = 0;
	u_char *q = p;

#ifndef __HELIOS
	while(read(0, (char *)&c, 1))
#else
	while(Read(fdstream(0), (char *)&c, 1, -1))
#endif
		{
		c = (u_char)tolower((char)c);
		switch(c)
			{
#ifdef __HELIOS
			case 0x03:
				clean_up("interrupted by user");
#endif

			case '\033':		/* escape */
				*q = 0;
				return q;

			case '\n':
			case '\r':
				if(echo)
					{
					putchar('\n');
					fflush(stdout);
					}
				*p++ = 0;
				return p;

			case '\b':		/* backspace */
			case 0x7f:		/* or delete */
				if(!echo)
					break;
				if(p > q)
					{
					putchar('\b');
					putchar(SPACE);
					putchar('\b');
					fflush(stdout);
					p--;
					}
				break;

			case -1:		/* error or CTRL-d */
			case 0x04:
				return (u_char *) NULL;

			case ' ':
			case '\t':
				if(lastc == ' ' || lastc == '\t')
					break;
				c = ' ';
					
			default:
				if(!isprint((char)c))
					continue;
				if( p-q >= BUFFMAX)
					break;
				if(echo)
					{
					putchar(c);
					fflush(stdout);
					}
				*p++ = c;
			}
		lastc = c;
		}

	return (u_char *) NULL;
}

	/* and, finally ... */

static void clean_up(p)
char *p;
{
	int db;
	DB *tt;

	signal(SIGINT, SIG_IGN);

	if(current_page != (u_char *) NULL)
		free(current_page);
	if((current_page != internal_page) && internal_page != (u_char *) NULL)
		free(internal_page);
	for(db = 0 ; db < maxdb ; db++)
		{
		if(fdtxt[db] != (FILE *)NULL)
			fclose(fdtxt[db]);
		tt = t[db];
		if(tt != (DB *)NULL)
			(void) (*(tt->close))(tt->internal);
		}

	if(!title_only)
		termend();

	if(p)
		{
		clear_screen();
		fprintf(stderr, "HELP: %s\r\n", p);
		fflush(stderr);
		exit(-1);
		}

	clear_screen();
	exit(0);
}

#ifndef __HELIOS

static void clean_exit(sig)
int sig;
{
	sig = sig;
	clean_up("interrupted by user");
}

#endif

	/* add an xref to the current list - take care of duplicates */

static void add_xref(p, docn)
u_char *p;
long docn;
{
	int i;

	for(i = 0 ; i < xrefp ; i++)
		if(! compare_string(xref[i].word, p))
			return;

	strcpy((char *)xref[xrefp].word, (char *)p);
	xref[xrefp].docn = docn;
	xref[xrefp].addr = 0;
	xref[xrefp].fd = 0;
	xref[xrefp++].screen = -1;
}

	/* display xrefs at end of document */

static int do_xrefs()
{
	int i;
	int status;

	if(xrefp == 0)
		return 0;		/* no xrefs here */

	i = newline();
	if(i != END_OF_SCREEN)
		i = newline();

	if(i == END_OF_SCREEN)
		{
		status = wait_for_user(NEXT_PAGE
				| ((screen_no > 0) ? PREV_PAGE : 0));

		if(status == PREV_PAGE ||
		   status == GOBACK    ||
		   status == SEARCH)
			return status;
		}

	printf("See also: ");
	cur_x += 10;

	for(i = 0 ; i < xrefp ; i++)
		{
		xref[i].x = cur_x;
		xref[i].y = cur_y;
		xref[i].addr = 0;
		xref[i].fd = 0;
		xref[i].screen = -1;
		highlight_words = 1;
		done_newline = 0;
		if(display_word(xref[i].word) == END_OF_SCREEN)
			{
			highlight_words = 0;
			status = wait_for_user(NEXT_PAGE
					| ((screen_no > 0) ? PREV_PAGE : 0));

			if(status == PREV_PAGE ||
			   status == GOBACK    ||
			   status == SEARCH)
				return status;

			i--;
			continue;
			}
		if(done_newline)
			{
			xref[i].x = 1;
			xref[i].y++;
			}

		xref[i].screen = screen_no;

		if(i < xrefp -1)
			{
			highlight_words = 0;
			if(cur_x + 2 > screen_width)
				continue;
			if(display_word((u_char *)", ") == END_OF_SCREEN)
				continue;	 /* force next word to fail */
			}
		}
	highlight_words = 0;

	return 0;
}

	/* check to see if we have reached the end of the displayable text */

static int no_more()
{
	u_char *p = page_pointer;

	while((unsigned int) *p != EODOC)
		switch((unsigned int) *p)
			{
			case KWORD:
			case QWORD:
				while(((unsigned int) *++p & 0x80) == 0);
				break;

			case ENDLINE:
				p++;
				continue;

			default:
				return 0;
			}
	return 1;
}

	/* run through the three-column table and work out column positioning */

static void check_three_col()
{
	u_char *p = page_pointer;
	u_char *q, buf[BUFFMAX+1];
	int cnt, len1, len2;

	len1 = len2 = 0;

	while(1)
		{
		q = buf;
		while(*p != ENDLINE && q-buf < BUFFMAX)
			*q++ = *p++;
		*q = 0;
	
		for(cnt = 0, q = buf ; *q != '\\' && *q; q++, cnt++);
		if(cnt > len1)	len1 = cnt;
	
		if(*(q+1) == LIST_ITEM)
			q += ADDRESS_LENGTH + 1;

		for(cnt = 0, q++ ; *q != '\\' && *q; q++, cnt++);
		if(cnt > len2)	len2 = cnt;

		while(*p++ != ENDLINE);
	
		if(*p != THREECOL)
			break;
		}

	column_two = COL_ONE + len1 + 4;
		
	column_three = column_two + len2 + 4;

	if(column_two +10 > screen_width)
		column_two = screen_width - 10;

	if(column_three +5 > screen_width)
		column_three = screen_width - 5;
}

	/* move the cursor depending on cursor-key received */

#ifdef __STDC__
static int move_cursor(u_char c, int *current_label)
#else
static int move_cursor(c, current_label)
u_char c;
int *current_label;
#endif
{
	int label = *current_label;
	int tmp, row;

	switch(c)
		{
		case 'A':	/* up-arrow */
			row = menu[label].y;
			for(tmp = label-1; tmp >= 0; tmp--)
				if(menu[tmp].y < row)
					{
					label = tmp;
					break;
					}
			if(label == *current_label)
				return 0;
			break;

		case 'B':	/* down-arrow */
			row = menu[label].y;
			for(tmp = label+1; tmp <= menup; tmp++)
				if(menu[tmp].y > row)
					{
					label = tmp;
					break;
					}
			if(label == *current_label)
				return 0;
			break;

		case 'C':	/* right-arrow */
			if(++label == menup)
				return 0;
			break;
			
		case 'D':	/* left-arrow */
			if(--label < 0)
				return 0;
			break;
			
		}

		/* erase highlighting on old selection */

	lowlight_term(*current_label);

		/* highlight new selection */

	highlight_term(label);

	*current_label = label;
	return menu[label].action;
}

	/* display a menu-item on the screen */

static void put_menu(a, action)
char *a;
int action;
{
	if(a == (char *)NULL)
		return;

	menu[menup].x      = cur_x;
	menu[menup].y      = cur_y;
	menu[menup].action = action;
	menu[menup].addr = 0;
	menu[menup].fd = 0;
	strcpy((char *)menu[menup++].word, a);
	printf(HIGHFORMAT_CS, a[0], &a[1]);
	cur_x += strlen((char *)a)+2;
}

	/* routine used by qsort to sort document titles */
	/* only the title word is checked                */
	/* ignore case when doing comparisons            */

static int sort_title(a, b)
#ifdef __STDC__
const
#endif
void *a;
#ifdef __STDC__
const
#endif
void *b;
{
	struct title_data *aa = (struct title_data *)a;
	struct title_data *bb = (struct title_data *)b;
	u_char *p = aa->word;
	u_char *q = bb->word;

	return (compare_string_nocase(p,q));
}

	/* routine used by qsort to sort search hits     */

static int sort_hit(a, b)
#ifdef __STDC__
const
#endif
void *a;
#ifdef __STDC__
const
#endif
void *b;
{
	struct hitlist **aa = (struct hitlist **)a;
	struct hitlist **bb = (struct hitlist **)b;

	u_long val1 = ((u_long)aa[0]->fd * 1000000) + aa[0]->addr;
	u_long val2 = ((u_long)bb[0]->fd * 1000000) + bb[0]->addr;

	return ((int)(val1 - val2));
}

	/* highlight a searchable term */

static void highlight_term(num)
int num;
{
	int tright = right;
	int tcol = three_col;

	right = three_col = 0;

	move_to(menu[num].y, menu[num].x);
	if(menu[num].action == SEARCH)
		{
		putchar(' ');
		cur_x++;
		}
	printf(HIGHFORMAT_S1);
	display_word(menu[num].word);
	printf(HIGHFORMAT_S2);
	fflush(stdout);

	three_col = tcol;
	right = tright;
}

static void lowlight_term(num)
int num;
{
	int tright = right;
	int tcol = three_col;

	right = three_col = 0;

	move_to(menu[num].y, menu[num].x);
	if(menu[num].action == SEARCH)		 /* xref */
		{
		highlight_words = TRUE;
		display_word(menu[num].word);
		highlight_words = FALSE;
		}
	else
		{
		printf(HIGHFORMAT_C, menu[num].word[0], &menu[num].word[1]);
		cur_x += strlen((char *)menu[num].word);
		}

	three_col = tcol;
	right = tright;
}

	/* do the opening page bits and pieces */

static int page_one()
{
	int status;

	screen_size();

	state = PAGE_ONE;

	display_page_one = TRUE;
	status = show_document(0, 5);	/* document 1 is opening title page */
	display_page_one = FALSE;

	PUSH(state);

	return ((status == GOBACK) ? status : (int) search_addr);
}

	/* build internal page with subject list and display it */

static int table_of_contents()
{
	int nextc, i, j, len, db;
	int cnt = 0;
	int number_accepted;
	u_char buff[WORDMAX + 1], *p;

		/* At the end of each text file is the subject list */

	page_reqd = 0;

	for(db = 0 ; db < maxdb ; db++)
		{

		if(fseek(fdtxt[db], -sizeof(subject_header), 2))
			clean_up("corrupt text file");

		fread(&subject_header, sizeof(subject_header), 1, fdtxt[db]);
		if(swap(subject_header.subject_magic) != SUBJECTMAGIC)
			clean_up("corrupt text file");

			/* seek to start of subject list */

		if(fseek(fdtxt[db], swap(subject_header.subject_pointer), 0))
			clean_up("corrupt database");

		subject_header.subject_count =
				 (int) swap(subject_header.subject_count);

		number_accepted = 0;

		for(i = 0 ; i < subject_header.subject_count ; i++)
			{
			p = buff;
			while(((nextc = getc(fdtxt[db])) != SUBJECTBREAK)
					&& p-buff < WORDMAX)
				*p++= nextc;

			*p = 0;

				/* No duplicates */

			for(j = 0; j < cnt; j++)
				if(!compare_string(buff, title[j].word))
					break;
					
			if(j == cnt)		/* no dup */
				{
				len = strlen((char *)buff) + 11;
				page_reqd += len + 7L;

				title[number_accepted+cnt].word=(u_char *)malloc(len);
                		if(title[number_accepted+cnt].word == (u_char *) NULL)
                       			clean_up("failed to malloc titles");

       				strcpy((char *)title[number_accepted+cnt].word,
					(char *)buff);
				title[number_accepted+cnt].addr = 0;
				title[number_accepted+cnt].fd = 0;
				number_accepted++;
				}
			}

		cnt += number_accepted;
		}

        qsort(title, cnt, sizeof(struct title_data), sort_title);
 
                /* now build dummy screen to display */
 
	if(internal_page != (u_char *) NULL)	/* free old page */
		free(internal_page);

        p = internal_page = (u_char *)malloc((int)page_reqd + 2);
        if(internal_page == (u_char *) NULL)
                clean_up("failed to malloc internal page");
 
	*p++ = VERBATIM;		/* force a blank line */
	*p++ = ENDLINE;
	for(i = 0 ; i < cnt ; i++)
		{
		*p++ = THREECOL;
		*p++ = i + 'a';
		*p++ = '.';
		*p++ = '\\';
		*p++ = LIST_ITEM;
		sprintf((char *)p, ADDRESS_FORMAT, title[i].addr);
		p += ADDRESS_LENGTH;
		*p++ = 1;
                strcpy((char *)p, (char *)title[i].word);	/* data */
                p += strlen((char *)title[i].word);
                *p++ = ENDLINE;                 /* end of line marker */
                free(title[i].word);
 		}

        *p++ = EODOC;
 
        /* Having built the page in memory, display it */
 
        return (show_document(0, INTERNAL));
}

	/* compare unsigned character strings */

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

	/* compare unsigned character strings (ignoring case) */

static int compare_string_nocase(s1, s2)
u_char *s1, *s2;
{
	u_char c1,c2;

	while(1)
		{
		if ((c1 = tolower((char)*s1++)) != (c2 = tolower((char)*s2++)))
			return c1 - c2;
		if (c1 == 0)
			return 0;     /* no need to check c2 */
		}
}

	/* determine if the char is valid in a search term */

#ifdef __STDC__
static int isvalid(u_char c)
#else
static int isvalid(c)
u_char c;
#endif

{
        return(c == '_' || isalnum(c));
}

	/* convert word to lower case */

static void word_to_lower(p)
u_char *p;
{
        for(; *p ; *p = tolower((char)*p), p++);
}

	/* swap disk data if machine is not LITTLE_ENDIAN */

static u_long swap(s)
u_long s;
{
        if(BYTE_ORDER != LITTLE_ENDIAN)
                BLSWAP(s);

        return s;
}

static void usage()
{
	fprintf(stderr, "HELP: usage - \n\thelp [<options>] [<query>]\n");
	fprintf(stderr, "\nwhere <query> is :\n");
	fprintf(stderr,
		"\tthe word (or words) to be searched for\n");
	fprintf(stderr, "\nand <options> are :\n");
	fprintf(stderr, "\t-e\texact title match only (case sensitive)\n");
	fprintf(stderr, "\t-a\tlet multiple words refine the search\n");
	fprintf(stderr,
		"\t-o\tlet multiple words expand the search (default)\n");
	fflush(stdout);
	exit(-1);
}

		/* for each term in the search, search the B-tree and */
		/* merge hit lists depending on the prevailing mode */

static int search_btree(bb, field)
u_char *bb;
int field;
{
	int wordcnt, status, lasthitp;
	int i, j, k, l;
	char buff[BUFFMAX+1];
	char *p, *q;

	for(p = (char *)bb; isspace(*p) && *p ; p++);   /* skip spaces */
	for(q = (char *)(bb + strlen((char *)bb) -1);
			q >= p && isspace(*q);
				q--);

	if(q < p)
		return(NOT_FOUND);

	strncpy(buff, p, q-p+1);
	buff[q-p+1] = 0;

	for(p = buff, wordcnt = 1; *p; p++)	/* count words */
		if(*p == ' ')
			{
			*p = 0;
			wordcnt++;
			}

	hitp = 0;

	status = do_search((u_char *)buff, field);
	qsort(hit, hitp, sizeof(struct hitlist *), sort_hit);

	if(wordcnt == 1)			/* single term */
		return(hitp ? QUERY : NOT_FOUND);

	p = buff;				/* back to start */

	for(i = 1; i < wordcnt; i++)		/* multiple terms */
		{
		int tmode = mode;		/* temporary mode flag */

		while(*p++);			/* to start of next word */

		lasthitp = hitp;		/* remember old total */

		if(!strcmp(p, "+"))		/* forced and (TOC) */
			{
			tmode = AND;
			while(*p++);		/* next word */
			if(++i == wordcnt)
				break;
			}

		status = do_search((u_char *)p, field);     /* next */

		if(tmode == OR)			/* add new terms */
			{
			qsort(hit, hitp, sizeof(struct hitlist *), sort_hit);
			continue;
			}

			/* we have to merge the two sets */

		j = 0;
		k = lasthitp;
		l = hitp;

		while(j < lasthitp && k < hitp)
			{
			u_long val1=(hit[j]->fd * 1000000L) + hit[j]->addr;
			u_long val2=(hit[k]->fd * 1000000L) + hit[k]->addr;

			if(val1 < val2)
				{
				j++;
				continue;
				}

			if(val1 > val2)
				{
				k++;
				continue;
				}

				/* match - copy data */

			hit[l] = (struct hitlist *)
				malloc(sizeof(struct hitlist));
			if(hit[l] == (struct hitlist *)NULL)
				break;
			if(l == HITMAX)
				{
				l--;
				break;
				}

			bcopy((char *)hit[j], (char *)hit[l],
					sizeof(struct hitlist));

			j++;
			k++;
			l++;
			}

		/* free up unused elements */

		for(k = 0 ; k < hitp ; k++)
			free(hit[k]);

		/* now copy new pointers to start of list */

		for(j = 0, k = hitp; k < l ; j++, k++)
			hit[j] = hit[k];

		hitp = j;
		}

	return(hitp ? QUERY : NOT_FOUND);		
}
