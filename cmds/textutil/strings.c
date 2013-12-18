/*
 * Strings:
 *
 * Finds and prints any ascii strings in a binary file.
 * A string is a sequence of 4 or more printable characters ending with a
 * newline or a null. If no input files exist, stdin is read.
 *
 * PAB 28/9/90
 *
 * RcsId:	$Id: strings.c,v 1.6 1993/03/10 14:13:04 nickc Exp $	(C) Copyright 1990, Perihelion Software Ltd.
 *		$Log: strings.c,v $
 * Revision 1.6  1993/03/10  14:13:04  nickc
 * fixed bug 1075 (strings not work on text files) plus two other undiscovered bugs (corrupts memory with strings longer than 255 characters, and fails to display an unterminated string at eof)
 *
 * Revision 1.5  1992/09/25  10:45:09  martyn
 * changed error  message
 *
 * Revision 1.4  1990/11/20  11:50:48  martyn
 * added -a option after much discussion with PB
 *
 * Revision 1.1  90/08/30  15:01:48  james
 * Initial revision
 * 
 */

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/textutil/RCS/strings.c,v 1.6 1993/03/10 14:13:04 nickc Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TRUE 		1
#define FALSE		0
#define MAX_STRING	255

int offsetopt	    = FALSE;
int all_terminators = FALSE;
int length	    = 4;

#define show_string   			\
  string[ seq ] = '\0';			\
  \
  if (offsetopt)			\
    printf("%8x %s\n",strstart,string); \
  else					\
    puts(string);			\
  \
  seq = 0


void
strings( FILE * file )
{
  char	string[ MAX_STRING ];
  int	c;
  int	strstart = 0;
  int	offset   = 0;
  int	seq      = 0;

  
  while ((c = fgetc( file )) != EOF)
    {
      if (isprint( c ))
	{
	  if (seq == 0)
	    strstart = offset;

	  if (seq >= MAX_STRING - 1)
	    {
	      show_string;
	    }

	  string[ seq++ ] = c;
	}
      else
	{
	  if ((c == '\n' || c == '\r' || c == '\0' || all_terminators)
	       && seq >= length)
	    {
	      show_string;
	    }
      
	  seq = 0;
	}
      
      offset++;
    }

  if (seq >= length)
    {
      show_string;
    }
  
  return;
  
} /* strings */


int
main(
     int 	argc,
     char **	argv )
{
  FILE *	fpi;

  
  while (argv[1][0] == '-')
    {
      switch (argv[1][1])
	{
	case 'o':
	  offsetopt = TRUE;
	  break;
	  
	case 'a':
	  all_terminators = TRUE;
	  break;
	  
	case 0:		/* lone '-' */
	  break;	/* for compatibility with unix! */
	  
	default:
	  if (!isdigit(argv[1][1]))
	    {
	      fprintf(stderr,"strings: unknown option '%s'", argv[1]);
	      exit(1);
	    }
	  
	  if ((length = atoi(argv[1] + 1)) >= MAX_STRING)
	    {
	      fprintf(stderr,"strings: -n too big");
	      exit(2);
	    }
	  
	  break;
	}
      
      argc--;
      argv++;
    }
  
  if (argc < 2)
    {
      strings(stdin);
    }
  else
    {
      while (--argc >= 1)
	{
	  if ((fpi = fopen(argv[argc],"rb")) == NULL)
	    {
	      fprintf(stderr, "strings: %s ", argv[argc]);
	      perror("");
	      exit(1);
	    }
	  
	  strings(fpi);
	  fclose(fpi);
	}
    }
  
  return 0;

} /* main */

