/*
 *  CMP for Helios
 * Written by John Fitch  1988 May 02
 * Fixed by PAB
 * Based very losely on the public domain DIFF
 * Debugged PRH aka paulh	13/8/90
 */

static char *rcsid ="$Header: /hsrc/cmds/com/RCS/cmp.c,v 1.5 1993/07/12 10:50:37 nickc Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslib.h>

				/* Forward references */
void error(char *,int);
void cant(char *,char *,int);

FILE  *infd[2] = { NULL, NULL };  /* Input file identifiers  */

/*
 * Global variables
 */

int  	lflag  = FALSE;  /* Print byte number and differing byte  */
int  	sflag  = FALSE;  /* Silence for different files           */

int     byteno = 0;		/* Count of the byte in question */
int     lineno = 1;		/* Line number */
int     byte0;			/* From first file */
int     byte1;			/* From second file */
int     retcode = 0;		/* Return code */


int main(
int  	argc,
char  	**argv )
/*
 * Cmp main program
 */
{
  register int  i;
  register char  *ap;

#ifdef  TIMING
  sectiontime = time(&totaltime);
#endif
  while (argc > 1 && *(ap = argv[1]) == '-' && *++ap != '\0') {
  	while (*ap != '\0') {
  	  switch ((*ap++)) {
  	  case 'l':
  	  	  lflag++; /* long listing */
  	  	  break;

  	  case 's':
  	  	  sflag++; /* silent - nothing printed - just a ret code */
  	  	  break;

  	  default:
  	  	  fprintf(stderr,
  	  	  	"Warning, bad option '%c'\n",
  	  	  	ap[-1]);
  	  	  break;
  	  }
  	}
  	argc--;
  	argv++;
  }

  if (argc != 3)
  	error("Usage: cmp [-ls] file1 file2",0);
  if (lflag && sflag) {
  	fprintf(stderr,
  	  "Warning, -l and -s are incompatible, -s supressed.\n");
  	sflag = FALSE;
  }
  argv++;
  for (i = 0; i <= 1; i++) {
  	if (argv[i][0] == '-' && argv[i][1] == '\0') infd[i] = stdin;
  	else {
          infd[i] = fopen(argv[i], "rb");
	  if (!infd[i]) cant(argv[i], "input", 2);	/* Fatal error */
  	}
  }

  if (infd[0] == stdin && infd[1] == stdin)
	error("Can't cmp two things both on standard input.",0);

  if (infd[0] == NULL && infd[1] == NULL) {
  	cant(argv[0], "input", 0);
  	cant(argv[1], "input", 1);
  }

  byte0 = getc(infd[0]);
  byte1 = getc(infd[1]);

  while (byte0 != EOF &&  byte1 != EOF) {
    byteno++;			/* Count the byte */
    if (byte0 != byte1) {
      retcode=1;
      if (sflag) exit(1);
      if (lflag) fprintf(stdout,"Offset %6d       char %c/%c byte %2x/%2x\n",byteno,isprint(byte0)?byte0:'?',isprint(byte1)?byte1:'?',byte0,byte1);
      else {
	fprintf(stdout,"%s %s differ: char %d, line %d\n",
		        argv[0],argv[1],   byteno,  lineno);
	exit(1);
      }
    }
    if (byte0 == 0x0a) lineno++; /* check for lf */

  byte0 = getc(infd[0]);
  byte1 = getc(infd[1]);
  }
 

  if (byte0 != byte1) {
    retcode = 1;
    if (byte0 != EOF) {
      if (!sflag) fprintf(stderr,"cmp: EOF on %s\n",argv[1]);
    }
    else {
      if (!sflag) fprintf(stderr,"cmp: EOF on %s\n",argv[0]);
    }
  }
  else
    if (!(sflag || retcode))
    	fprintf(stderr,"cmp: Files are identical\n");

  exit(retcode);
}


void cant(
char  	*filename,
char  	*what,
int  	fatalflag )
/*
 * Can't open file message
 */
{
  fprintf(stderr, "Can't open %s file \"%s\": ", what, filename);
  perror((char *)NULL);
  if (fatalflag) {
  	exit(fatalflag);
  }
}

void error(
char  	*format,
int	arg )
/*
 * Error message before retiring.
 */
{
  fprintf(stderr, format, arg);
  putc('\n', stderr);
  exit(2);
}

