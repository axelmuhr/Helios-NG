/*
-- HP LaserJet filter. The output of this program should be piped to lpr, i.e.
-- /helios/local/bin/laserpri $* | /helios/bin/lpr -l
-- Arguments :
-- -l		suppress line numbers
-- -t		suppress title
-- -w		print sideways
-- -2		print sideways in 2 columns
-- -u		display usage
-- name ...	the name of the file to be printed. If not specified, the 
--		standard input, terminated by EOF, is used
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	V_PAGE_LGTH	93
#define	H_PAGE_LGTH	66
#define	LNS_PER_IN	8
#define	TOP_MGN		2
#define	LEFT_MGN	1

#define	V_TEXT_LGTH	84
#define	H_TEXT_LGTH	57

#define	COLUMN_WIDTH	86
#define	V_PAGE_WIDTH	125
#define	H_PAGE_WIDTH	182

#define	COLUMN_SPACE	10
#define	LHS		1
#define	RHS		2
#define	TAB		8
#define	TAB_SIZE(x) 	(TAB-((x)%TAB)) 
#define	UNDERLINE	0x80

/* printer initialization codes */
#define _RESET		"\033E"		/* reset */
#define _ORIENT		"\033&l1O"	/* landscape orientation */
#define _PAGE_LEN	"\033&l%dP"	/* landscape page length */
#define _FONT		"\033(s16.66H"	/* landscape font */
#define	_LEFT_MARGIN	"\033&a%dL"	/* left margin */
#define _LINES_PER_IN	"\033&l%dD"	/* lines per inch */
#define _TOP_MARGIN	"\033&l%dE"	/* top margin */
#define _LINES_PER_PAGE	"\033&l%dF"	/* lines of text per page */

#define	TWO_COL		0	/* for two columns, sideways... */
#define	NO_LINE		1	/* for no line numbers...	*/
#define WIDE		2	/* for wide, sideways..		*/
#define NO_TITLE	3	/* for no title...		*/
#define	MAX_OPT		4
int opt [MAX_OPT] ;

char time_str [30] ;
#define LEN_NUM		6
#define MAX_LINE_NUM	9999

char Page [V_TEXT_LGTH][H_PAGE_WIDTH] ;

int linenumber ;
int pagenumber ;
int pagewidth ;
int textlength ;

char *filename = "(stdin)" ;

int state = 0 ;

int main (
	  int argc ,
	  char *argv [] )
{
	int k = 1 ;
	char *cp ;
	FILE *fp ;
	time_t timer ;
	void usage (void) ;
	void init_printer (void) ;
	void init_page (void) ;
	void print (FILE *) ;

	time (&timer) ;
	strcpy (time_str, ctime (&timer)) ;
	time_str [strlen (time_str) - 1] = '\0' ; /* strip off new-line */

	for (k = 0 ; k < MAX_OPT ; k++) 
		opt [k] = 0 ;		
	for (k = 1 ; k < argc ; k++) 
	{
		cp = argv [k] ;
		if (*cp++ != '-') 
			break ;
		while (*cp != NULL) 
		{
			switch (*cp++) 
			{
				case 't':	opt [NO_TITLE]++ ;	break ;
				case 'l':	opt [NO_LINE]++ ;	break ;
				case 'w':	opt [WIDE]++ ;		break ;
				case '2':	opt [TWO_COL]++ ;		break ;
				case 'u':
				default:	usage () ;
			}
		}
	}
	if (opt [TWO_COL] && opt [WIDE]) 	/* maybe user needs help... */
		usage () ;
	if (k == argc) 
	{
		/* no files specified, so std input used */
		init_printer () ;
		init_page () ;
		print (NULL) ;
	}
	while (k < argc) 
	{
		if ((fp = fopen (argv [k], "r")) == NULL) 
		{
			fprintf (stderr, "Can't open file: %s\n", argv [k]) ;
			exit (1) ;
		}
		else
		{
			filename = argv [k] ;
			init_printer () ;
			init_page () ;
			print (fp) ;
			fclose (fp) ;
		}
		k++ ;
	}
}

void init_printer () 
{
	/* reset */
	printf (_RESET) ;
	if (opt [TWO_COL] || opt [WIDE]) 
	{
		/* landscape orientation */
		printf (_ORIENT) ;
		/* landscape page length */
		printf (_PAGE_LEN, H_PAGE_LGTH) ;
		/* landscape font */
		printf (_FONT) ;
	}
	else
	{
		/* portrait page length */
		printf (_PAGE_LEN, V_PAGE_LGTH) ;
		/* font selection */
		printf (_FONT) ;
	}
	/* left margin */
	printf (_LEFT_MARGIN, LEFT_MGN) ;
	/* lines per inch */
	printf (_LINES_PER_IN, LNS_PER_IN) ;
	/* top margin */
	printf (_TOP_MARGIN, TOP_MGN) ;
	/* lines of text per page */
	if (opt [TWO_COL]) 
	/* add 3 for title */
		printf (_LINES_PER_PAGE, H_TEXT_LGTH+3) ;	
	else
		printf (_LINES_PER_PAGE, V_TEXT_LGTH+3) ;
}

void init_page () 
{
	pagenumber = 1 ;
	linenumber = 1 ;
	state = 0 ;
	if (opt [TWO_COL] || opt [WIDE]) 
	{
		/* landscape (horizontal) */
		textlength = H_TEXT_LGTH ;
		if (opt [TWO_COL])
			pagewidth = COLUMN_WIDTH ;
		else
			pagewidth = H_PAGE_WIDTH ;
	}
	else
	{
		/* portrait (vertical) */
		textlength = V_TEXT_LGTH ;
		pagewidth = V_PAGE_WIDTH ;
	}
}

void print (FILE *file )
{
	int fill_page (FILE *, int) ;
	int print_page (void) ;
	if (file == 0) file = stdin ;
	while (state != EOF) 
	{
		state = fill_page (file, LHS) ;
		if (opt [TWO_COL] && state != EOF) 
			(void) fill_page (file, RHS) ;
		state = print_page () ;
	}
}

int fill_page (
	       FILE *file ,
	       int page_side )
{
	static int overflow = 0 ;
	int i = 0, c = 0 ;
	int spaces = 0 ;
	int line = 0, col = 0 ;
	int backspaces = 0 ;
	int lline = 0, ccol = 0, stopcol = 0 ;
	int margin = 0 ;
	char lineno [LEN_NUM + 1] ;
	for (line = 0 ; line < textlength ; line++) 
	{
		if (opt [TWO_COL]) 
		{
			if (page_side == RHS) 
			{
				col = COLUMN_WIDTH ;
				pagewidth = H_PAGE_WIDTH ;
				for (i = 0 ; i < COLUMN_SPACE ; i++) 
					Page [line][col++] = ' ' ;
			}
			else if (page_side == LHS) 
			{
				pagewidth = COLUMN_WIDTH ;
			}
		}
		if (!opt [NO_LINE] && !overflow) 
		{
			if (linenumber > MAX_LINE_NUM) 
				linenumber = 1 ;
			sprintf (lineno, "%4d  ", linenumber++) ;
		}
		else if (opt [NO_LINE] && overflow) 
			sprintf (lineno, " >>>  ") ;
		else 
			sprintf (lineno, "      ") ;
		stopcol = col ;
		for (i = 0 ; i < LEN_NUM ; i++) 
		{
			Page [line][col++] = lineno [i] ;
		}
		while (col < pagewidth) 
		{
			if ((c = getc (file)) == EOF) 
			{
				Page [line][stopcol] = '\0' ;
				overflow = 0 ;
				return EOF ;
			}
			else if (c == '\n') 
			{
				while (col < pagewidth) 
					Page [line][col++] = ' ' ;
				break ;
			}
			else if (c == '\t') 
			{
				if (page_side == RHS) 
					spaces = TAB_SIZE (col - ((12 + COLUMN_SPACE) % TAB)) ;
				else
					spaces = TAB_SIZE (col - 6) ;
				for (i = 0 ; i < spaces ; i++) 
				{
					Page [line][col++] = ' ' ;
					if (col >= pagewidth) break ;
				}
			}
			else if (c == '\f') 
			{
				Page [line][col++] = c ;
				while ((c = getc (file)) != '\n' && c != EOF) ;
				return (1) ;
			}
			else if (c == '\b' || c == '\r') 
			{
				if (page_side == RHS) 
					margin = LEN_NUM + COLUMN_WIDTH+COLUMN_SPACE ;
				else
					margin = LEN_NUM ;
				if (c == '\b') 
				{
					backspaces = 1 ;
					while ((c = getc (file)) == '\b') backspaces++ ;
					ungetc (c, file) ;
				}
				else
					backspaces = col - margin ;
				lline = line ;
				ccol = col ;
				for (i = backspaces ; i > 0 ; i--) 
				{
					c = getc (file) ;
					if (i > col - margin) 
					{
						if (lline > 0) 
						{
							lline = line - 1 ;
							ccol = pagewidth + (col-margin) ;
						}
						else
							continue ;
					}
					else
					{
						lline = line ;
						ccol = col ;
					}
					if (c == '\n' || c == '\f') 
					{
						ungetc (c, file) ;
						break ;
					}
					if (c != '_') 
					{
						if (Page [lline][ccol-i] == '_') 
						{
							Page [lline][ccol-i] = c ;
							Page [lline][ccol-i] |= UNDERLINE ;
						}
						else
							Page [lline][ccol-i] = c ;
					}
					else Page [lline][ccol-i] |= UNDERLINE ;
				}
			}
			else if (c >= ' ') 
			{	
				/* leave out unwanted control chars */
				Page [line][col++] = c ;
			}
		}
		if (c == '\n') overflow = 0 ;
		else
		{
			if ((c = getc (file)) != '\n') 
			{
				overflow = 1 ;
				ungetc (c, file) ;
			}
		}
		col = 0 ;
		backspaces = 0 ;
	}
	return (1) ;
}

int print_page () 
{
	int line, col = 0 ;
	int start = 0 ;
	int width = pagewidth ;
	int c = 0 ;
	int result = 1 ;
	void print_title_line (void) ;
	print_title_line () ;
	for (line = 0 ; line < textlength ; line++) 
	{
		for (col = 0 ; col < width ; col++) 
		{
			if (start == width) 
			{
				printf ("\r\n\f") ;
				return (1) ;
			}
			else while (col < start) 
			{ 
				putchar (' ') ; col++ ; 
			}
			if ((c = Page [line][col]) == '\0') 
			{
				if (state == EOF || start != 0) 
					return (EOF) ;
				else
				{
					result = EOF ;
					width = COLUMN_WIDTH ;
					break ;
				}
			}
			else if (c & UNDERLINE) 
			{
				putchar (c & ~UNDERLINE) ;
				putchar ('\b') ;
				putchar ('_') ;
			}
			else if (c == '\f') 
			{
				if (opt [TWO_COL]) 
				{
					putchar (' ') ;
					if (col < COLUMN_WIDTH) 
					{
						start = COLUMN_WIDTH ;
					}
					else
					{
						width = COLUMN_WIDTH ;
						break ;
					}
				}
				else
				{
					printf ("\r\n") ;
					putchar (c) ;
					return (1) ;
				}
			}
			else
				putchar (c) ;
		}
		if (result != EOF || line != textlength - 1) 
			printf ("\r\n") ;
	}
	return (result) ;
}

void print_title_line () 
{
	void print_title (int) ;
	if (!opt [NO_TITLE]) 
	{
		print_title (pagenumber++) ;
		if (opt [TWO_COL] && state != EOF) 
		{
			printf ("%*s", LEN_NUM, "") ;
			print_title (pagenumber++) ;
		}
	}
	printf ("\r\n\r\n\r\n") ;
}

void print_title (int pageno )
{
	const filler = 30 + 5 + 3 + 24 ; /* ugly, huh ? */
	int i, spaces, width ;
	/* format */
	width = pagewidth - 4 ;
	if (opt [TWO_COL]) 
	{
		spaces = 2 ;
		width = COLUMN_WIDTH - 3 ;
	}
	else if (opt [WIDE]) 
		spaces = 50 ;
	else
		spaces = 19 ;

	/* now to print the title */
	printf ("File:  ") ;
	if (strlen (filename) > 28) 
		*(filename + 28) = '\0' ;
	printf ("%-30s", filename) ;
	for (i = 0 ; i < spaces ; i++)
		putchar (' ') ;
	printf ("%5s%3d", "Page ", pageno) ;
	while ((filler + (spaces++)) < width) 
		putchar (' ') ;
	printf ("%-24s", time_str) ;
}

void usage () 
{
	fprintf (stderr, "Usage: laserp [-l] [-t] [-2] [-w] [-u] [[file] ...]\n") ;
	fprintf (stderr, "-l : suppress line numbers\n") ;
	fprintf (stderr, "-t : suppress title\n") ;
	fprintf (stderr, "-w : wide, sideways\n") ;
	fprintf (stderr, "-2 : two columns, sideways\n") ;
	fprintf (stderr, "-u : display this message\n") ;
	exit (1) ;
}
